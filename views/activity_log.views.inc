<?php

/**
 * Implementation of hook_views_handlers().
 */
function activity_log_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'activity_log') . '/views',
    ),
    'handlers' => array(
      'activity_log_views_handler_field_mid' => array(
        'parent' => 'views_handler_field',
      ),
      'activity_log_views_handler_filter_owner' => array(
        'parent' => 'views_handler_filter_numeric',
      ),
      'activity_log_views_handler_argument_owner' => array(
        'parent' => 'views_handler_argument_numeric',
      )
    ),
  );
}

/**
 * Implementation of hook_views_data().
 */
function activity_log_views_data() {

  $data['activity_log_messages']['table']['group'] = t('Activity Log');
  
  $data['activity_log_messages']['table']['base'] = array(
    'field' => 'mid',
    'title' => t('Activity Log'),
    'help' => t('Activities Logged by the Activity Log module'),
    'weight' => -10,
  );
  
  $data['activity_log_messages']['mid'] = array(
    'title' => t('Activity Log Message'),
    'help' => t('The evaluated template from the event'),
    'field' => array(
      'handler' => 'activity_log_views_handler_field_mid',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
  );
  
  $data['activity_log_messages']['stream_owner_id'] = array(
    'title' => t('Stream Owner'),
    'help' => t('The entity in whose activity stream the messages are intended to appear'),
    'filter' => array(
      'handler' => 'activity_log_views_handler_filter_owner',
    ),
    'argument' => array(
      'handler' => 'activity_log_views_handler_argument_owner',
    ),
  );
  
  $data['activity_log_messages']['created'] = array(
    'title' => t('Created time'),
    'help' => t('The time at which the message (or first message if grouping is enabled) was recorded'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_date',
    ),
  );
  
  $data['activity_log_messages']['last_updated'] = array(
    'title' => t('Updated time'),
    'help' => t('The time at which the most recent message was added to a grouped message'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_date',
    ),
  );
  
  return $data;
}
