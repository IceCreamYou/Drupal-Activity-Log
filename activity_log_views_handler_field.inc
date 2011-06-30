<?php
class activity_log_views_handler_field extends views_handler_field {
  function render($values) {
    $value = $values->{$this->field_alias};
    $record = activity_log_load($value);
    return theme('activity_log', $record);
  }
}
