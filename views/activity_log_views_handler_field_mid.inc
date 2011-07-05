<?php

/**
 * @file
 *   Extends views_handler_field with the ability to theme activity log messages
 */

class activity_log_views_handler_field_mid extends views_handler_field {
  
  /*
   * Modified options form to include the option to show the message or the mid
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['display_as'] = array(
      '#type' => 'radios',
      '#title' => t('Display as'),
      '#options' => array(
        'mid' => t('Message ID'),
        'message' => t('Themed Message'),
      ),
      '#description' => t('Select "Themed Message" to convert the MID into the actual message for this event.'),
      '#default_value' => (empty($this->options['display_as']) ? 'message' : $this->options['display_as']),
      '#required' => TRUE,
    );
  }

  /**
   * Render the field.
   *
   * @param $values
   *   The values retrieved from the database.
   *
   * @return
   *   The themed activity message, or the mid, depending on settings
   */
  function render($values) {
    $value = $values->{$this->field_alias};
    if ($this->options['display_as'] == 'message'){
      return theme('activity_log_item', activity_log_load($value));
    } else {
      return $value;
    }
  }
}
