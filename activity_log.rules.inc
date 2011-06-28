<?php

/**
 * @file
 *   Rules integration for the Activity Log module.
 */

//==================
// ADMIN UI: STEP 1
//==================

/**
 * Implementation of hook_form_alter().
 *
 * Do some magic to the Rules Activity Log action form so we can record all the
 * data we need about the template.
 *
 * We can put this in our rules.inc instead of .module file because the two
 * forms we need to alter only exist when all rules.inc files have already been
 * loaded.
 */
function activity_log_form_alter(&$form, &$form_state, $form_id) {
  if ($form_id == 'rules_admin_form_edit' || $form_id == 'rules_admin_form_add') {
    if ($form_state['storage']['element']['#name'] == 'activity_log_log_action') {
      // Run our custom submit hooks.
      array_unshift($form['submit']['#submit'], 'activity_log_rules_admin_form_submit_before');
      if ($form_id == 'rules_admin_form_add') {
        array_push($form['submit']['#submit'], 'activity_log_rules_admin_form_submit_eval_after');
      }
      array_push($form['submit']['#submit'], 'activity_log_rules_admin_form_submit_label_after');
      // Undo the prefix to the template value that we added so the input evaluator knows this is the template.
      if (isset($form['settings']['template']['#default_value'])) {
        $form['settings']['template']['#default_value'] = drupal_substr($form['settings']['template']['#default_value'], drupal_strlen(ACTIVITY_LOG_DELIMITER));
      }
      $form['#old_stream_owner'] = empty($form['settings']['stream_owner']['#default_value']) ? '' : $form['settings']['stream_owner']['#default_value'];
    }
    // Remove the help for our input evaluator (because it's internal, not user-facing).
    unset($form['input_help']['activity_log_input_evaluator_process']);
  }
  elseif ($form_id == 'rules_admin_form_delete') {
    if (!empty($form['#parameters'][3]) && !empty($form['#parameters'][3]['#name']) && $form['#parameters'][3]['#name'] == 'activity_log_log_action') {
      $form['#submit'][] = 'activity_log_rules_admin_form_delete';
    }
  }
}

/**
 * Submit callback for the Rules action edit form.
 *
 * Record all of the information we need about a template.
 * This has to run before the Rules hooks so we can modify the settings values
 * before they get saved.
 */
function activity_log_rules_admin_form_submit_before($form, &$form_state) {
  $rule = $form_state['proxy']->_rule_name;
  $tid = db_result(db_query("
    SELECT tid FROM {activity_log_templates} WHERE rule = '%s' AND stream_owner_id_template = '%s'
  ", $rule, $form['#old_stream_owner']));
  $name = new stdClass();
  $pid = 0;
  if (!empty($tid)) {
    $name = db_fetch_object(db_query("
      SELECT n.*
      FROM {activity_log_templates} t
      LEFT JOIN {activity_log_action_names} n
        ON t.pid = n.pid
      WHERE tid = %d
    ", $tid));
    $pid = $name->pid;
  }
  // If this is a new action or we changed the public name, check if the new name exists. If it doesn't, save the new value.
  if (empty($name) || $name->public_name != $form_state['values']['settings']['public_name']) {
    $pid = db_result(db_query("SELECT pid FROM {activity_log_action_names} WHERE public_name = '%s'", $form_state['values']['settings']['public_name']));
    if (empty($pid) && !empty($form_state['values']['settings']['public_name'])) {
      $rec = (object) array('public_name' => $form_state['values']['settings']['public_name']);
      drupal_write_record('activity_log_action_names', $rec);
      $pid = $rec->pid;
    }
  }
  $record = (object) array(
    'rule' => $rule,
    'template' => $form_state['values']['settings']['template'],
    'stream_owner_id_template' => $form_state['values']['settings']['stream_owner'],
    'pid' => $pid,
    'eval_input' => empty($form_state['element']['#settings']['#eval input']) ? '' : serialize($form_state['element']['#settings']['#eval input']),
  );
  if (!empty($tid)) {
    $record->tid = $tid;
    drupal_write_record('activity_log_templates', $record, array('tid'));
  }
  else {
    drupal_write_record('activity_log_templates', $record);
  }
  $form_state['values']['settings']['tid'] = $record->tid;
  // If switching values means that we've now abandoned an action name, remove it.
  if (isset($name->pid) && $name->pid != $pid) {
    db_query("
      DELETE {activity_log_action_names}
      FROM {activity_log_action_names}
      LEFT JOIN {activity_log_templates}
        ON {activity_log_action_names}.pid = {activity_log_templates}.pid
      WHERE {activity_log_templates}.tid IS NULL
    ");
  }
  // Designate that the template should be processed later in activity_log_input_evaluator_process_apply().
  $form_state['values']['settings']['template'] = ACTIVITY_LOG_DELIMITER . $form_state['values']['settings']['template'];
}

/**
 * Submit callback for the Rules action edit form.
 *
 * Set the eval input field when the action is first created.
 * We have to do this after the Rules submit callbacks run so that eval input
 * has already been computed.
 */
function activity_log_rules_admin_form_submit_eval_after($form, &$form_state) {
  db_query("
    UPDATE {activity_log_templates} SET eval_input = '%s' WHERE tid = %d
  ", serialize($form_state['element']['#settings']['#eval input']), $form_state['values']['settings']['tid']);
}

/**
 * Submit callback for the Rules action edit form.
 *
 * The label of the form may have changed between the first submit callback and
 * now if the label wasn't customized and a label callback was used.
 */
function activity_log_rules_admin_form_submit_label_after($form, $form_state) {
  if ($form_state['#action_label'] != $form_state['element']['#info']['label']) {
    db_query("
      UPDATE {activity_log_templates} SET action_label = '%s' WHERE tid = %d
    ", $form_state['element']['#info']['label'], $form_state['values']['settings']['tid']);
  }
}

/**
 * Delete callback for the Rules action edit form.
 *
 * Clean up when our Activity Log action is deleted by deleting the associated
 * template.
 */
function activity_log_rules_admin_form_delete($form, $form_state) {
  if (!empty($form['#parameters'][3])) {
    db_query("
      DELETE FROM {activity_log_templates} WHERE rule = '%s' AND stream_owner_id_template = '%s'
    ", $form_state['proxy']->_rule_name, $form['#parameters'][3]['#settings']['stream_owner']);
  }
}

//=============================
// RECORDING THE EVENT: STEP 2
//=============================

// INPUT EVALUATION: STEP 2 PART 1

/**
 * Implementation of hook_rules_evaluator().
 *
 * We use a heavy weight so that we run last. We want to run last because we
 * add text to the end of the string that should not be evaluated by other
 * input evaluators.
 */
function activity_log_rules_evaluator() {
  return array(
    'activity_log_input_evaluator_process' => array(
      'label' => t('Activity Log'),
      'weight' => 100,
    ),
  );
}

/**
 * Implementation of callback_prepare().
 */
function activity_log_input_evaluator_process_prepare($string, $variables) {
  return TRUE;
}

/**
 * Implementation of callback_apply().
 *
 * Get information about all the variables that should be available when
 * evaluating input and make it available somewhere we have access to in the
 * action callback.
 */
function activity_log_input_evaluator_process_apply($string, $data = NULL, &$state, $return_output = TRUE) {
  // Only process the Log Activity action's Template setting.
  if (strpos($string, ACTIVITY_LOG_DELIMITER) !== 0) {
    return $string;
  }
  else {
    $string = drupal_substr($string, drupal_strlen(ACTIVITY_LOG_DELIMITER));
  }
  // Get information about all the variables that should be available when evaluating input.
  $map = array();
  foreach ($state['variables'] as $name => $info) {
    if (!empty($info->data)) {
      $map[$name] = array(
        'id' => activity_log_get_id($info->data->_data),
        'type' => $info->info['type'],
        'class' => (empty($info->data->_data->class) ? 'rules_data_type_'. $info->info['type'] : $info->data->_data->class),
      );
    }
  }
  $map['state'] = $state;
  $serialized = serialize($map);
  // Remove any instances of our delimiting string so we don't get anything mixed up.
  if (strpos($serialized, ACTIVITY_LOG_DELIMITER) !== FALSE) {
    str_replace(ACTIVITY_LOG_DELIMITER, substr_replace(ACTIVITY_LOG_DELIMITER, '#! ', 0, 2), $serialized);
  }
  // Store the variable information so we have it available in the action callback.
  return $string . ACTIVITY_LOG_DELIMITER . $serialized;
}

/**
 * Attempt to extract the ID of an object.
 *
 * It is sad that we have to do it this way but unfortunately there is no
 * widely implemented, standard way to do this. Rules would need to add an
 * extract_id($object) property to its rules_data_type class.
 */
function activity_log_get_id($data) {
  // See if any of a list of pre-determined IDs exists as a property of $the object.
  foreach (array('tid', 'cid', 'nid', 'sid', 'vid', 'fid', 'rid', 'aid', 'eid', 'oid', 'uid') as $key) {
    if (isset($data->$key)) {
      return $data->$key;
    }
  }
  // Look for a property that seems like it could be an ID.
  foreach ($data as $key => $value) {
    if ((drupal_strlen($key) < 5 && drupal_substr($key, -2) == 'id') || drupal_substr($key, -3) == '_id') {
      return $value;
    }
  }
}


/**
 * Implementation of callback_help().
 */
function activity_log_input_evaluator_process_help($variables) {
  // Our evaluator is for internal processing. The user doesn't need to know anything about it.
  return array();
}

// EXECUTE ACTION: STEP 2 PART 2

/**
 * Implementation of hook_rules_action_info().
 */
function activity_log_rules_action_info() {
  return array(
    'activity_log_log_action' => array(
      'label' => t('Log activity'),
      'module' => 'Activity Log',
      'eval input' => array('template', 'stream_owner'),
    ),
  );
}

/**
 * The form callback for the activity logging action.
 */
function activity_log_log_action_form($settings, &$form) {
  $settings += array('template' => '', 'stream_owner' => '', 'type' => 'user', 'public_name' => '');
  $form['settings']['template'] = array(
    '#type' => 'textarea',
    '#title' => t('Activity message'),
    '#default_value' => $settings['template'],
    '#rows' => 3,
    '#required' => TRUE,
  );
  $form['settings']['stream_owner'] = array(
    '#type' => 'textfield',
    '#title' => t('Stream owner'),
    '#description' => t('Enter the ID of the entity in whose activity stream the activity records generated from this template should appear.'),
    '#default_value' => $settings['stream_owner'],
    '#required' => TRUE,
  );
  // Get a list of Rules data types we know about. @see _rules_admin_form_argument()
  $options = rules_extract_property(array_filter(rules_get_data_types(), 'rules_admin_element_filter'), 'label');
  $options = array_map('drupal_ucfirst', $options);
  // It only makes sense to offer options that have their own unique pages.
  unset(
    $options['comment'],
    $options['date'],
    $options['fbsmp'],
    $options['fbss_comment'],
    $options['flag'],
    $options['number'],
    $options['string'],
    $options['taxonomy_vocab'],
    $options['boolean'],
    $options['global']
  );
  asort($options);
  $form['settings']['type'] = array(
    '#type' => 'select',
    '#title' => t('Stream type'),
    '#description' => t('The type of entity on whose stream the activity record will appear.'),
    '#default_value' => $settings['type'],
    '#options' => $options,
    '#required' => TRUE,
  );
  $form['settings']['public_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Public name of this activity type'),
    '#description' => t('The name of the activity as shown to users who have permission to disable certain activity types in their stream.') .' '.
      t('Leave this field blank if you do not want to allow users to disable viewing messages about this activity type.') .' '.
      t('This field does not accept tokens.'),
    '#default_value' => $settings['public_name'],
    '#autocomplete_path' => 'activity_log/autocomplete/public_name',
  );
}

/**
 * Log activity.
 */
function activity_log_log_action($settings) {
  $time = time();
  // Get the value of the settings set via the UI.
  $pos = strrpos($settings['template'], ACTIVITY_LOG_DELIMITER);
  $message = drupal_substr($settings['template'], 0, $pos);
  $map = drupal_substr($settings['template'], $pos + drupal_strlen(ACTIVITY_LOG_DELIMITER)); // serialized
  // Store the event record.
  $record = (object) array(
    'tid' => $settings['tid'],
    'created' => $time,
    'stream_owner_id' => $settings['stream_owner'],
    'stream_owner_type' => $settings['type'],
    'id_map' => $map,
  );
  drupal_write_record('activity_log_events', $record);
  module_invoke_all('activity_log_event', $record, $settings);
}
