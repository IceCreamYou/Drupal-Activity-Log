<?php

/**
 * Implementation of hook_views_handlers().
 */
function activity_log_views_handlers() {
  return array(
  'info' => array(
    'path' => drupal_get_path('module', 'activity_log'),
    ),
  'handlers' => array(
    'activity_log_views_handler_field' => array(
     'parent' => 'views_handler_field',
     ),        
    ),
  );
}

/**
 * Implementation of hook_views_data
 */
function activity_log_views_data(){

  $data['activity_log_messages']['table']['group'] = t('Activity Log');
  
  $data['activity_log_messages']['table']['base'] = array(
     'field' => 'mid',
     'title' => t('Activity Log'),
     'help' => t("Activities Logged by the Activity Log module"),
     'weight' => -10,
   );
  
  $data['activity_log_messages']['mid'] = array(
     'title' => t('Activity Log Message'),
     'help' => t('The evaluated template from the event'),
     'field' => array(
       'handler' => 'activity_log_views_handler_field',
     ),
     'sort' => array(
       'handler' => 'views_handler_sort',
     ),
     'filter' => array(
       'handler' => 'views_handler_filter_string',
     ),
     'argument' => array(
       'handler' => 'views_handler_argument_string',
     ),
   );
   
  $data['activity_log_messages']['created'] = array(
      'title' => t('Time'),
      'help' => t('When the logged event happened'),
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
        'handlert' => 'views_handler_argument_date',
      ),
  );
  
  return $data;

}
