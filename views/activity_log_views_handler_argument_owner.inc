<?php

/**
 * @file
 *   Extends views_handler_filter_numeric to allow selection of stream type
 */

class activity_log_views_handler_argument_owner extends views_handler_argument_numeric {
  
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
      '#default_value' => (empty($this->options['stream_type']) ? 'user' : $this->options['stream_type']),
      //'#default_value' => $this->options['type'],
      //'#options' => $options,
      '#required' => TRUE,
    );
  }
  
  /*
   * Adds a where clause to the query in such a way that it takes the stream
   * type into account.
   */
  function query() {
    $this->ensure_my_table();

    if (!empty($this->options['break_phrase'])) {
      views_break_phrase($this->argument, $this);
    }
    else {
      $this->value = array($this->argument);
    }
    
    if(!empty($this->options['stream_type']) && $this->options['stream_type'] != 'all'){
      $type = "AND {$this->table_alias}.stream_owner_type = \"%s\"";
    }else{
      $type = '';
    }

    if (count($this->value) > 1) {
      $operator = empty($this->options['not']) ? 'IN' : 'NOT IN';
      $placeholders = implode(', ', array_fill(0, sizeof($this->value), '%d'));
      $this->query->add_where(0, "$this->table_alias.$this->real_field $operator ($placeholders) $type", $this->value, $this->options['stream_type']);
    }
    else {
      $operator = empty($this->options['not']) ? '=' : '!=';
      $this->query->add_where(0, "$this->table_alias.$this->real_field $operator %d $type", $this->argument, $this->options['stream_type']);
    }
    
  }

}
