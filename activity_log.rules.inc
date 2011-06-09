<?php

/**
 * @file
 *   Rules integration for the Rules Activity Log module.
 */

// ADMIN UI: STEP 1

/**
 * Implementation of hook_form_FORM_ID_alter().
 *
 * Do some magic to the Rules action form so we can record all the data we need
 * about the template.
 */
function activity_log_form_rules_admin_form_edit_alter($form, $form_state) {
  // If this is the Activity Log action:
  if ($form_state['storage']['element']['#name'] == 'activity_log_log_action') {
    // Remove the help for our input evaluator. It's internal, not user-facing.
    unset($form['input_help']['activity_log_input_evaluator_process']);
    // Put the rule name in the settings so we have access to it when the action callback runs.
    $form_state['storage']['element']['#settings']['rule'] = $form_state['proxy']->_rule_name;
    // Run our custom submit hook.
    $form['submit']['#submit'][] = 'activity_log_form_rules_admin_form_edit_submit';
    $form['#submit'][] = 'activity_log_form_rules_admin_form_edit_submit';
  }
}

/**
 * Submit callback for the Rules action form.
 */
function activity_log_form_rules_admin_form_edit_submit($form, $form_state) {
  $rule = $form_state['proxy']->_rule_name;
  $template = $form_state['values']['settings']['template'];
  $recipient = $form_state['values']['settings']['recipient'];
  $eval_input = $form_state['element']['#settings']['#eval input'];
  $tid = db_result(db_query("SELECT tid FROM {activity_log_templates} WHERE rule = '%s'", $rule));
  $record = (object) array(
    'rule' => $rule,
    'template' => $template,
    'recipients' => $recipient,
    'eval_input' => serialize($eval_input),
  );
  if ($tid) {
    $record->tid = $tid;
    drupal_write_record('activity_log_templates', $record, array('tid'));
  }
  else {
    drupal_write_record('activity_log_templates', $record);
  }
}

// RECORDING THE EVENT: STEP 2

// INPUT EVALUATION: STEP 2 PART 1

/**
 * Implementation of hook_rules_evaluator().
 */
function activity_log_rules_evaluator() {
  return array(
    'activity_log_input_evaluator_process' => array(
      'label' => t('Rules Activity Log'),
      'weight' => 0, // Run after Token (-5).
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
 */
function activity_log_input_evaluator_process_apply($string, $data = NULL, &$state, $return_output = TRUE) {
  $map = array();
  foreach ($state['variables'] as $name => $info) {
    $class = $info->data->_data->class;
    $map[$name] = array(
      'id' => activity_log_get_id($info->data->_data),
      'type' => $info->info['type'],
      'class' => (empty($class) ? 'rules_data_type_'. $info->info['type'] : $class),
    );
  }
  $map['state'] = $state;
  return $string .'#activity_log_id_map:'. serialize($map);
}

/**
 * Attempt to extract the ID of an object.
 *
 * It is sad that we have to do it this way but unfortunately there is no
 * widely implemented, standard way to do this. Rules would need to add an
 * extract_id($object) property to its rules_data_type class.
 */
function activity_log_get_id($data) {
  foreach (array('nid', 'sid', 'cid', 'tid', 'vid', 'fid', 'rid', 'aid', 'eid', 'oid', 'uid') as $key) {
    if (isset($data->$key)) {
      return $data->$key;
    }
  }
  foreach ($data as $key => $value) {
    if (drupal_strlen($key) < 5 && drupal_substr($key, -2) == 'id') {
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
      'eval input' => array('template', 'recipient', 'type'),
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
      t('Enter an ID or use PHP to return multiple IDs.'),
    '#default_value' => $settings['recipient'],
    '#rows' => 3,
    '#required' => TRUE,
  );
  // @see _rules_admin_form_argument()
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
  $recipient = $settings['recipient'];
  $pos = strrpos($settings['template'], '#activity_log_id_map:');
  $message = drupal_substr($settings['template'], 0, $pos);
  $map = drupal_substr($settings['template'], $pos + drupal_strlen('#activity_log_id_map:')); // serialized
  $record = (object) array(
    'rule' => $settings['rule'],
    'type' => $settings['type'],
    'created' => time(),
    'id_map' => $map,
  );
  if (is_array($recipient)) {
    foreach ($recipient as $re) {
      $record->recipient = $re;
      drupal_write_record('activity_log_events', $record);
      cache_set('aid '. $record->aid, $message, 'cache_activity_log', CACHE_TEMPORARY);
    }
  }
  else {
    $record->recipient = $recipient;
    drupal_write_record('activity_log_events', $record);
    cache_set('aid '. $record->aid, $message, 'cache_activity_log', CACHE_TEMPORARY);
  }
}
