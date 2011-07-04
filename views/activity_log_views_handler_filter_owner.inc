<?php

/**
 * @file
 *   Extends views_handler_field with the ability to theme activity log messages
 */

class activity_log_views_handler_filter_owner extends views_handler_filter_numeric {
  
  /*
   * Modified options form to include the option to select stream type
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    
    /* TODO: Find a way to get this functionality (from activity_log.rules.inc)
             working in this context.
    
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
    
    */
    $form['stream_type'] = array(
      '#type' => 'textfield',
      '#title' => t('Stream type'),
      '#description' => t('The type of entity on whose stream the activity is intended to appear.'),
      '#default_value' => (empty($this->options['type']) ? 'user' : $this->options['type']),
      //'#default_value' => $this->options['type'],
      //'#options' => $options,
      '#required' => TRUE,
    );
  }
}
