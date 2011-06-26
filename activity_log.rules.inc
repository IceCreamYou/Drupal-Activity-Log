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
      // Run our custom submit hook.
      array_unshift($form['submit']['#submit'], 'activity_log_rules_admin_form_submit');
      // Undo the prefix to the template value that we added so the input evaluator knows this is the template.
      if (isset($form['settings']['template']['#default_value'])) {
        $form['settings']['template']['#default_value'] = drupal_substr($form['settings']['template']['#default_value'], drupal_strlen(ACTIVITY_LOG_DELIMITER));
      }
      $form['settings']['old_recipients'] = array(
        '#type' => '#value',
        '#value' => empty($form['settings']['recipients']['#default_value']) ? '' : $form['settings']['recipients']['#default_value'],
      );
    }
    // Remove the help for our input evaluator (because it's internal, not user-facing).
    unset($form['input_help']['activity_log_input_evaluator_process']);
  }
}

/**
 * Submit callback for the Rules action edit form.
 *
 * Record all of the information we need about a template.
 */
function activity_log_rules_admin_form_submit($form, &$form_state) {
  $rule = $form_state['proxy']->_rule_name;
  $template = $form_state['values']['settings']['template'];
  $recipient = $form_state['values']['settings']['recipient'];
  $eval_input = $form_state['element']['#settings']['#eval input'];
  $tid = db_result(db_query("
    SELECT tid FROM {activity_log_templates} WHERE rule = '%s' AND recipients = '%s'
  ", $rule, $form_state['values']['settings']['old_recipients']));
  $record = (object) array(
    'rule' => $rule,
    'template' => $template,
    'recipients' => $recipient,
    'eval_input' => serialize($eval_input),
  );
  if (!empty($tid)) {
    $record->tid = $tid;
    drupal_write_record('activity_log_templates', $record, array('tid'));
  }
  else {
    drupal_write_record('activity_log_templates', $record);
  }
  // Designate that the template should be processed later in activity_log_input_evaluator_process_apply().
  $form_state['values']['settings']['template'] = ACTIVITY_LOG_DELIMITER . $form_state['values']['settings']['template'];
  $form_state['values']['settings']['tid'] = $record->tid;
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
  // This could happen on the recipient field when the input is PHP and we want to return an array of recipients.
  if (is_array($string)) {
    return $string;
  }
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
      'module' => 'Rules Activity Logging',
      'eval input' => array('template', 'recipient'),
    ),
  );
}

/**
 * The form callback for the activity logging action.
 */
function activity_log_log_action_form($settings, &$form) {
  $settings += array('template' => '', 'recipient' => '', 'type' => 'user');
  $form['settings']['template'] = array(
    '#type' => 'textarea',
    '#title' => t('Activity message'),
    '#default_value' => $settings['template'],
    '#rows' => 3,
    '#required' => TRUE,
  );
  $form['settings']['recipient'] = array(
    '#type' => 'textarea',
    '#title' => t('Recipient(s)'),
    '#description' => t('Activity records generated from this template will appear in the stream of the entities specified here.') .' '.
      t('Enter an ID or use PHP to return multiple IDs if you want this action to appear in multiple streams.'),
    '#default_value' => $settings['recipient'],
    '#rows' => 2,
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
    '#title' => t('Recipient type'),
    '#description' => t('The type of entity on whose stream the activity record will appear.'),
    '#default_value' => $settings['type'],
    '#options' => $options,
    '#required' => TRUE,
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
  $recipient = $settings['recipient'];
  // Store the event record.
  $record = (object) array(
    'tid' => $settings['tid'],
    'created' => $time,
    'id_map' => $map,
  );
  drupal_write_record('activity_log_events', $record);
  // Store the record for each recipient. We could have multiple recipients if there was PHP in the recipient field that returned an array of IDs.
  $recipient = (array) $recipient;
  foreach ($recipient as $re) {
    $r = (object) array(
      'aid' => $record->aid,
      'recipient' => $re,
      'type' => $settings['type'],
    );
    drupal_write_record('activity_log_recipients', $r);
  }
}
