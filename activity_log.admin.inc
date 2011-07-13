<?php

/**
 * @file
 *   Administrative settings for the Activity Log module.
 */

/**
 * The administrative settings form.
 */
function activity_log_admin($form_state) {
  $form = array();
  $form['info'] = array(
    '#value' => t('This page allows you to configure global settings for the Activity Log module.') .' '.
      t('In order to set up events to be logged to activity streams, you should <a href="!path">create a Rule</a> that uses the "Log Activity" action.', array(
        '!path' => 'admin/rules/trigger',
      )),
  );
  $form['activity_log_max_age'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of days to store an activity record'),
    '#description' => t('Activity records older than this many days will be deleted when cron runs.'),
    '#default_value' => variable_get('activity_log_max_age', 30),
    '#maxlength' => 6,
    '#size' => 4,
  );
  $form['clear_cache'] = array(
    '#type' => 'submit',
    '#value' => t('Clear all message caches'),
    '#submit' => array('activity_log_admin_clear_caches'),
    '#description' => t('Clear all activity message caches.') .' '.
      t('<strong>Warning:</strong> viewing activity messages will be slow after this until the cache is rebuilt.'),
  );
  return system_settings_form($form);
}

/**
 * Submit callback for the "clear caches" button.
 */
function activity_log_admin_clear_caches() {
  drupal_set_message(t('Message caches cleared.'));
  activity_log_clear_message_cache();
}
