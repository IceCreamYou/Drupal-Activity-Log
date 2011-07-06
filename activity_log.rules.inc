<?php

/**
 * @file
 *   Rules integration for the Activity Log module.
 */

//==================
// ADMIN UI: STEP 1
//==================

/**
 * The form callback for the activity logging action.
 */
function activity_log_log_action_form($settings, &$form) {
  drupal_add_js(drupal_get_path('module', 'activity_log') .'/activity_log.admin.js');
  $settings += array(
    'public_name' => '',
    'grouping' => array(
      'group_method' => 'target_action',
      'group_interval' => 86400,
      'group_max' => 100,
      'group_summary' => '',
      'collapse_method' => 'activity_log_collection_inline',
      'group_template' => '',
    ),
    'template' => '',
    'stream_owner' => '',
    'type' => 'user',
    'acting_user' => '',
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
  $form['settings']['grouping'] = array(
    '#type' => 'fieldset',
    '#title' => t('Group similar messages'),
    '#collapsible' => TRUE,
    '#collapsed' => empty($settings['grouping']['group_template']),
    '#description' => '<p>'. t('Filling in these settings will allow grouping activity messages of this type in activity streams.') .' '.
      t('That way, instead of saying e.g. "Jack likes this" and "Jill likes this" separately, only one message can appear saying "Jack and Jill like this."') .' '.
      t('Messages will be grouped if there are at least two similar messages.') .' '.
      t('<strong>Warning:</strong> displaying grouped messages is slower than displaying ungrouped messages.') .'</p>',
  );
  $form['settings']['grouping']['group_method'] = array(
    '#type' => 'radios',
    '#title' => t('Group by'),
    '#description' =>
      t('Grouping by the target entity shows multiple users who took the same action on an object, like "Jane and Joe commented on this node."') .' '.
      t('You may want to group by action in cases where the result is trivial, such as changing profile pictures.') .' '.
      t('Group by the acting user when the fact that the user took action is more important than the action taken.') .' '.
      t('For example, you might group by the acting user on that user\'s profile for a "joined a group" action in order to get a message like "John joined the groups Drupal and Web Development."'),
    '#required' => TRUE,
    '#default_value' => $settings['grouping']['group_method'],
    '#options' => array(
      'target_action' => t('Group by action and the entity on which the action was performed'),
      'action' => t('Group by just the action'),
      'user_action' => t('Group by the acting user and the action performed'),
    ),
  );
  $form['settings']['grouping']['group_interval'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum duration between activities to allow grouping'),
    '#description' => t('The oldest an activity record can be (in seconds) and still be grouped together with a new activity.'),
    '#field_suffix' => t('seconds'),
    '#default_value' => $settings['grouping']['group_interval'],
  );
  $form['settings']['grouping']['group_max'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum number of items allowed in a grouped message'),
    '#description' => t('Do not allow combining more than this many similar messages into the same message.'),
    '#default_value' => $settings['grouping']['group_max'],
  );
  $form['settings']['grouping']['group_summary'] = array(
    '#type' => 'textarea',
    '#title' => t('Grouped item name'),
    '#description' => t('A summary of one of the grouped items. For example, if you grouped by user on the "add node" action, you might enter "[node:title]" in this field.'),
    '#default_value' => $settings['grouping']['group_summary'],
    '#rows' => 1,
  );
  $options = module_invoke_all('activity_log_collapse_methods');
  $form['settings']['grouping']['collapse_method'] = array(
    '#type' => 'select',
    '#title' => t('Collection format'),
    '#description' => t('The format used to evaluate the [collection] token in the grouped message template below.'),
    '#required' => TRUE,
    '#default_value' => $settings['grouping']['collapse_method'],
    '#options' => $options,
  );
  $form['settings']['grouping']['group_template'] = array(
    '#type' => 'textarea',
    '#title' => t('Grouped message template'),
    '#description' => '<p>'. t('The format of the grouped message.') .' '.
      t('The token <em>[names]</em> will be replaced with the names of the users who took this action.') .' '.
      '<span id="activity-log-admin-description">'.
        t('Additionally, the token <em>[collection]</em> will be replaced with summaries of the target objects formatted according to the "Grouped item name" setting above.')
      .'</span> '.
      t('For example, the template "[names] like the [node:type] [node:title]" might produce something like "Jack and Jill like the page Slippery Slopes."') .' '.
      t('<strong>Leave this blank if you do not want this type of message to be grouped.</strong>') .'</p>',
    '#default_value' => $settings['grouping']['group_template'],
    '#rows' => 3,
  );
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
  $form['settings']['acting_user'] = array(
    '#type' => 'textfield',
    '#title' => t('Acting user'),
    '#description' => t('Enter the ID of the user taking the action. If not specified, defaults to the current user.'),
    '#default_value' => $settings['acting_user'],
  );
}

/**
 * The form validation callback for the activity logging action.
 */
function activity_log_log_action_validate($form, $form_state) {
  $v = $form_state['values']['settings']['grouping'];
  if (!empty($v['group_template'])) {
    if (!empty($v['group_interval']) && (!is_numeric($v['group_interval']) || $v['group_interval'] <= 0)) {
      form_set_error('settings][grouping][group_interval', t('The maximum duration between activities to allow grouping must be a positive number.'));
    }
    if (!empty($v['group_max']) && (!is_numeric($v['group_max']) && $v['group_max'] <= 0)) {
      form_set_error('settings][grouping][group_max', t('The maximum number of items allowed in a grouped message must be a positive number.'));
    }
    if ($v['group_method'] == 'action' || $v['group_method'] == 'user_action') {
      if (empty($v['group_interval'])) {
        form_set_error('settings][grouping][group_interval', t('You must enter a maximum duration between activities to allow grouping if you do not group by the target entity.'));
      }
      if (empty($v['group_max'])) {
        form_set_error('settings][grouping][group_max', t('You must enter a maximum number of items allowed in a grouped message if you do not group by the target entity.'));
      }
    }
  }
}

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
    'group_template' => $form_state['values']['settings']['grouping']['group_template'],
    'group_summary' => $form_state['values']['settings']['grouping']['group_summary'],
    'collapse_method' => $form_state['values']['settings']['grouping']['collapse_method'],
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
      DELETE {activity_log_disabled_types}
      FROM {activity_log_disabled_types}
      LEFT JOIN {activity_log_templates}
        ON {activity_log_disabled_types}.pid = {activity_log_templates}.pid
      WHERE {activity_log_templates}.tid IS NULL
    ");
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
      DELETE FROM {activity_log_events} WHERE tid IN (SELECT tid FROM {activity_log_templates} WHERE rule = '%s' AND stream_owner_id_template = '%s')
    ", $form_state['proxy']->_rule_name, $form['#parameters'][3]['#settings']['stream_owner']);
    db_query("
      DELETE FROM {activity_log_messages} WHERE tid IN (SELECT tid FROM {activity_log_templates} WHERE rule = '%s' AND stream_owner_id_template = '%s')
    ", $form_state['proxy']->_rule_name, $form['#parameters'][3]['#settings']['stream_owner']);
    db_query("
      DELETE FROM {activity_log_templates} WHERE rule = '%s' AND stream_owner_id_template = '%s'
    ", $form_state['proxy']->_rule_name, $form['#parameters'][3]['#settings']['stream_owner']);
  }
}

/**
 * Autocomplete callback for the Rules action public name.
 */
function activity_log_autocomplete_public_name($string = '') {
  $matches = array();
  if ($string) {
    $result = db_query_range("SELECT public_name FROM {activity_log_action_names} WHERE LOWER(public_name) LIKE LOWER('%s%%')", $string, 0, 10);
    while ($row = db_fetch_object($result)) {
      $matches[$row->public_name] = check_plain($row->public_name);
    }
  }
  drupal_json($matches);
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
  drupal_alter('activity_log_id_map', $map);
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
      'eval input' => array('grouping|group_summary', 'grouping|group_template', 'template', 'stream_owner', 'acting_user'),
    ),
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
  $map_serialized = drupal_substr($settings['template'], $pos + drupal_strlen(ACTIVITY_LOG_DELIMITER));
  $map = unserialize($map_serialized);
  $target = reset($map); // returns the first element
  if (empty($settings['acting_user'])) {
    $settings['acting_user'] = $GLOBALS['user']->uid;
  }
  // Store the event record.
  $record = (object) array(
    'tid' => $settings['tid'],
    'created' => $time,
    'acting_uid' => $settings['acting_user'],
    'stream_owner_id' => $settings['stream_owner'],
    'stream_owner_type' => $settings['type'],
    'target_id' => $target['id'],
    'target_type' => $target['type'],
    'target_class' => $target['class'],
    'id_map' => $map_serialized,
  );
  drupal_write_record('activity_log_events', $record);
  if (!empty($settings['grouping']['group_template'])) {
    if ($settings['grouping']['group_method'] == 'target_action') {
      $mid = db_result(db_query_range("
        SELECT mid
        FROM {activity_log_messages}
        WHERE
          tid = %d AND                 /* group by activity type */
          stream_owner_id = %d AND     /* group by stream the message appears in */
          stream_owner_type = '%s' AND
          target_id = %d AND           /* group by target entity */
          target_type = '%s'
        ORDER BY mid DESC
      ", $record->tid, $record->stream_owner_id, $record->stream_owner_type, $record->target_id, $record->target_type, 0, 1));
    }
    elseif ($settings['grouping']['group_method'] == 'action') {
      $mid = db_result(db_query_range("
        SELECT mid
        FROM {activity_log_messages}
        WHERE
          tid = %d AND                 /* group by activity type */
          stream_owner_id = %d AND     /* group by stream the message appears in */
          stream_owner_type = '%s' AND
          created > %d AND             /* filter to groups that are new enough */
          LENGTH(aids) - LENGTH(REPLACE(aids, ',', '')) - 2 < %d /* filter to groups that don't have too many items in them already */
        ORDER BY mid DESC
      ", $record->tid, $record->stream_owner_id, $record->stream_owner_type, $time - $settings['grouping']['group_interval'], $settings['grouping']['group_max'], 0, 1));
    }
    elseif ($settings['grouping']['group_method'] == 'user_action') {
      $mid = db_result(db_query_range("
        SELECT mid
        FROM {activity_log_messages}
        WHERE
          tid = %d AND                 /* group by activity type */
          stream_owner_id = %d AND     /* group by stream the message appears in */
          stream_owner_type = '%s' AND
          acting_uid = %d AND          /* filter by action user */
          created > %d AND             /* filter to groups that are new enough */
          LENGTH(aids) - LENGTH(REPLACE(aids, ',', '')) - 2 < %d /* filter to groups that don't have too many items in them already */
        ORDER BY mid DESC
      ", $record->tid, $record->stream_owner_id, $record->stream_owner_type, $record->acting_uid, $time - $settings['grouping']['group_interval'], $settings['grouping']['group_max'], 0, 1));
    }
  }
  if (!empty($mid)) {
    db_query("UPDATE {activity_log_messages} SET aids = CONCAT(aids, %d, ','), last_updated = %d WHERE mid = %d", $record->aid, $record->created, $mid);
    $group = db_fetch_object(db_query("SELECT * FROM {activity_log_messages} WHERE mid = %d", $mid));
  }
  else {
    $group = (object) array(
      'tid' => $record->tid,
      'aids' => ','. $record->aid .',',
      'created' => $record->created,
      'last_updated' => $record->created,
      'stream_owner_id' => $record->stream_owner_id,
      'stream_owner_type' => $record->stream_owner_type,
      'target_id' => $record->target_id,
      'target_type' => $record->target_type,
      'acting_uid' => $record->acting_uid,
    );
    drupal_write_record('activity_log_messages', $group);
  }
  module_invoke_all('activity_log_event', $record, $group, $settings);
}
