<?php
  
/**
 * @file
 *   Adds an option to validate arguments as stream owner types.
 */

/**
 * Validate whether an argument is a valid stream owner type.
 */
class activity_log_stream_owner_type_argument_validate extends views_plugin_argument_validate {
  var $option_name = 'validate_argument_activity_log_stream_owner_type';
  function validate_argument($argument) {
    
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
    
    return in_array($argument, array_keys($options));
    
  }
}
