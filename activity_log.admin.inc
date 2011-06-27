<?php // $Id$

/*
Admin settings menu callback
*/

function activity_log_admin($form_state){
  $form['global'] = array(
    '#type' => 'fieldset',
    '#title' => t('Activity Log global Settings'),
    '#weight' => -5,
    '#collapsible' => False,
    '#collapsed' => FALSE,
  );
  $form['global']['activity_log_maxage'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of days to store an activity log'),
    '#default_value' => variable_get('activity_log_maxage', 30),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}


function activity_log_admin_submit($form, &$form_state) {
  if ($form_state['values']['maxage'] != variable_get('activity_log_maxage', 30)){
    variable_set('activity_log_maxage', $form_state['values']['activity_log_maxage']);
  }
  drupal_set_message(t('Activity log settings updated.'));
}
