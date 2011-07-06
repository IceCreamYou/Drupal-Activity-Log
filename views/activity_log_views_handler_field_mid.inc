<?php

/**
 * @file
 *   Displays a themed activity log message.
 */

class activity_log_views_handler_field_mid extends views_handler_field {
  function render($values) {
    return theme('activity_log_item', activity_log_load($values->{$this->field_alias}));
  }
}
