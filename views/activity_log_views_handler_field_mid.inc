<?php

/**
 * @file
 *   Extends views_handler_field with the ability to theme activity log messages
 */

class activity_log_views_handler_field_mid extends views_handler_field {
  /**
   * Render the field.
   *
   * @param $values
   *   The values retrieved from the database.
   *
   * @return
   *   The themed activity message
   */
  function render($values) {
    $value = $values->{$this->field_alias};
    return theme('activity_log_item', activity_log_load($value));
  }
}
