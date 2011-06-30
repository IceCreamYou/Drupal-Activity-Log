<?php
class activity_log_views_handler_field extends views_handler_field {
  function render($values) {
    $value = $values->{$this->field_alias};
    $record = module_invoke('activity_log', 'load', $value);
    return activity_log_evaluate_record($record);
  }
}
