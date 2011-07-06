<?php

/**
 * @file
 *   Hook documentation for the Activity Log module.
 */

/**
 * Implementation of hook_activity_log_collapse_methods().
 *
 * Specifies available options for combining an array of strings into a single
 * string. These options are exposed on the "Log activity" Rules action form
 * and are used to collapse an array of evaluated items into the [collection]
 * token in the group template.
 *
 * @return
 *   An associative array of options where the key is the name of the
 *   function for collapsing an array and the value is the human-friendly name
 *   of the option. The available functions should take a $collection array and
 *   an integral $count of the number of items in that collection as parameters
 *   and return a string summarizing the items in the $collection. The
 *   $collection will always have at least 2 items.
 */
function hook_activity_log_collapse_methods() {
  return array(
    'activity_log_collapse_inline' => t('Inline (A, B, and 3 others)'),
    'activity_log_collapse_list_horizontal' => t('Horizontal list (A B C D)'),
    'activity_log_collapse_list_vertical' => t('Vertical list (each item on its own line)'),
  );
}

/**
 * Implementation of hook_activity_log_event().
 *
 * Invoked when an activity is logged; allows taking action when this happens.
 *
 * @param $event
 *   The activity event object (properties correspond with the columns in the
 *   {activity_log_events} table).
 * @param $group
 *   The activity message object (properties correspond with the columns in the
 *   {activity_log_messages} table).
 * @param $settings
 *   An array containing the evaluated settings for the executed Activity Log
 *   Rules action.
 */
function hook_activity_log_event($event, $group, $settings) {
  if (module_exists('radioactivity')) {
    module_load_include('inc', 'radioactivity');
    $aids = explode(',', $group->aids);
    if (count($aids) > 1) {
      radioactivity_add_energy($group->mid, 'act_log', 'group:'. $record->tid);
    }
    else {
      radioactivity_add_energy($group->mid, 'act_log', 'event:'. $record->tid);
    }
  }
}
