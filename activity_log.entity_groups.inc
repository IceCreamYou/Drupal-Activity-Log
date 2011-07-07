<?php

/**
 * @file
 *   Entity group definitions and callbacks for the Activity Log module.
 */

/**
 *
 */
function activity_log_entity_groups($stream_owner = TRUE) {
  $og = module_exists('og');
  $ur = module_exists('user_relationships_api');
  $co = module_exists('comments');
  $fc = module_exists('fbss_comments');
  $ft = module_exists('facebook_status_tags');
  $tx = module_exists('taxonomy');
  $groups = array(
    'custom' => array(
      'items callback' => '',
      'title' => t(''),
      'weight' => 10,
      'expose fields' => array('id', 'type'),
    ),
    'acting user' => array(
      'items callback' => '',
      'title' => t(''),
      'weight' => -100,
    ),
    'target entity' => array(
      'items callback' => '',
      'title' => t(''),
      'weight' => -100,
    ),
  );
  if ($og) {
    $groups['active groups'] = array(
      'items callback' => '',
      'title' => t(''),
      'weight' => -100,
    );
    $groups['active groups members'] = array(
      'items callback' => '',
      'title' => t(''),
      'weight' => -100,
    );
  }
  if ($ur) {
    $groups['acting user relationships'] = array(
      'items callback' => '',
      'title' => t(''),
      'weight' => -100,
    );
    $groups['custom user relationships'] = array(
      'items callback' => '',
      'title' => t(''),
      'weight' => -100,
      'expose fields' => array('id'),
    );
    module_load_include('inc', 'user_relationships_api', 'user_relationships_api.api');
    $ur_types = user_relationships_types_load();
    foreach ($ur_types as $type) {
      $groups['acting user relationships '. $type->rtid] = array(
        'items callback' => '',
        'title' => t(''),
        'weight' => -100,
      );
      $groups['custom user relationships '. $type->rtid] = array(
        'items callback' => '',
        'title' => t(''),
        'weight' => -100,
        'expose fields' => array('id'),
      );
    }
  }
  if ($og && $ur) {
    $groups['active groups relationships active user'] = array(
      'items callback' => '',
      'title' => t('Members of active groups who have a relationship with the acting user'),
      'weight' => -100,
    );
    $groups['active groups relationships custom user'] = array(
      'items callback' => '',
      'title' => t('Members of active groups who have a relationship with a custom user'),
      'weight' => -100,
      'expose fields' => array('id'),
    );
    foreach ($ur_types as $type) {
      $groups['active groups relationships '. $type->rtid .' active user'] = array(
        'items callback' => '',
        'title' => t('Members of active groups who have a %type relationship with the acting user', array('%type' => $type->name)),
        'weight' => -100,
      );
      $groups['active groups relationships '. $type->rtid .' custom user'] = array(
        'items callback' => '',
        'title' => t('Members of active groups who have a %type relationship with a custom user', array('%type' => $type->name)),
        'weight' => -100,
        'expose fields' => array('id'),
      );
    }
  }
  if ($co) {
    $groups['node commenters'] = array(
      'items callback' => '',
      'title' => t('Other people who have commented on this node (including the node author)'),
      'weight' => -100,
    );
  }
  if ($fc) {
    $groups['status commenters'] = array(
      'items callback' => '',
      'title' => t('Other people who have commented on this status (including the sender and recipient)'),
      'weight' => -100,
    );
  }
  if ($ft) {
    $groups['mentioned users'] = array(
      'items callback' => '',
      'title' => t('The users @mentioned in a status'),
      'weight' => -100,
    );
    $groups['hashtag terms'] = array(
      'items callback' => '',
      'title' => t('The taxonomy terms corresponding to the #hashtags in a status'),
      'weight' => -100,
    );
  }
  if ($tx) {
    $groups['node terms'] = array(
      'items callback' => '',
      'title' => t('The taxonomy terms used in a node'),
      'weight' => -100,
    );
  }
  if (!$stream_owner) {
    unset($groups['active groups'], $groups['hashtag terms'], $groups['node terms']);
    $groups['target entity']['title'] = t(''); // target user
    $groups['everyone'] = array(
      'items callback' => '',
      'title' => t('Everyone'),
      'weight' => -100,
    );
  }
  return $groups;
}
