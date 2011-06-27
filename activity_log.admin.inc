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
  $form['activity_log_max_age'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of days to store an activity record'),
    '#description' => t('Activity records older than this many days will be deleted when cron runs.'),
    '#default_value' => variable_get('activity_log_max_age', 30),
  );
  return system_settings_form($form);
}
