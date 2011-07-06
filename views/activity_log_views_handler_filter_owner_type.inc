<?php

/**
 * @file
 *   Extends views_handler_filter_many_to_one with support for activity log 
 *  stream types
 */

class activity_log_views_handler_filter_owner_type extends views_handler_filter {
  
  /*
   * Override the value_form funtion to use a select box of strean types
   */
  function value_form(&$form, &$form_state) {

    require_once(drupal_get_path('module', 'rules_admin') . '/rules_admin.inc');
    
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
    
    $form['value'] = array(
      '#type' => 'select',
      '#title' => t('Stream type'),
      '#description' => t('The type of entity on whose stream the activity is intended to appear.'),
      '#default_value' => $this->options['type'],
      '#options' => $options,
      '#required' => TRUE,
    );
  }
}
