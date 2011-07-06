<?php
  
/**
 * @file
 *   Adds an option to set the default argument from known stream owner types.
 */

/**
 * Provide a select box containing valid stream owner types.
 */
class activity_log_stream_owner_type_argument_default extends views_plugin_argument_default {
  var $option_name = 'default_argument_activity_log_stream_owner_type';
  
  function argument_form(&$form, &$form_state){
    
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
    
    $form[$this->option_name] = array(
      '#type' => 'select',
      '#title' => t('Stream type'),
      '#description' => t('The type of entity on whose stream the activity is intended to appear.'),
      '#default_value' => $this->options['type'],
      '#options' => $options,
      '#required' => TRUE,
    );
    
  }
  
  function get_argument(){
    if (!empty($this->argument->options[$this->option_name])){
     return $this->argument->options[$this->option_name];
    }
  }

}
