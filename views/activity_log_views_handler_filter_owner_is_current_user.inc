<?php

/**
 * @file
 *   Filters Activity Log Views by stream type.
 */

/**
 * Allow filtering streams to use the current user as the owner.
 */
class activity_log_views_handler_filter_owner_is_current_user extends views_handler_filter_boolean_operator {
  function construct() {
    parent::construct();
    $this->value_value = t("Show only the current user's stream");
  }
  function query() {
    $this->ensure_my_table();
    $op = empty($this->value) ? '<>' : '=';
    $this->query->add_where($this->options['group'], "($this->table_alias.stream_owner_id $op ***CURRENT_USER***)");
  }
}
