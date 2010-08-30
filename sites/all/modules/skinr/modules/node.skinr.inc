<?php
// $Id: node.skinr.inc,v 1.3.2.1.2.5 2010/03/04 14:06:59 moonray Exp $
/**
 * @file
 * Provide skinr handling for node.module
 */

/**
 * @defgroup skinr_node_module node.module handlers
 *
 * @{
 */

/**
 * Implementation of hook_skinr_config().
 */
function node_skinr_config() {
  $data['node']['form']['node_type_form'] = array(
    'index_handler' => 'node_skinr_form_index_handler',
    'preprocess_hook_callback' => 'node_skinr_preprocess_hook_callback',
    'title' => t('node settings'),
  );
  $data['node']['form']['skinr_ui_form'] = array(
    'index_handler' => 'skinr_ajax_index_handler',
    'preprocess_hook_callback' => 'node_skinr_preprocess_hook_callback',
    'title' => t('node settings'),
    'collapsed' => FALSE,
  );
  $data['node']['preprocess']['node'] = array(
    'index_handler' => 'node_skinr_preprocess_index_handler',
  );

  return $data;
}

/**
 * Skinr form index handler.
 *
 * @param $op
 *   What kind of action is being performed. Possible values:
 *   - "form": the form elements for Skinr are being inserted in a form
 *   - "submit": the form has been submitted.
 * @param &$form
 *   - For "form", passes in the $form parameter from hook_form_alter().
 *   - For "submit", passes in the $form parameter from hook_form_submit().
 * @param $form_state
 *   - For "form", passes in the $form_state parameter from hook_form_alter().
 *   - For "submit", passes in the $form_state parameter from hook_form_submit().
 * @return
 *   The index where we can find our values in Skinrs data structure.
 */
function node_skinr_form_index_handler($op, &$form, $form_state) {
  switch ($op) {
    case 'form':
      return $form['#node_type']->type;

    case 'submit':
      // Clear old variable before we set a new one if the node type has changed
      if ($form_state['values']['old_type'] != $form_state['values']['type']) {
        foreach ($form_state['values']['skinr_settings']['node_group'] as $theme_name => $theme_data) {
          $skinr = new stdClass();
          $skinr->theme = $theme_name;
          $skinr->module = 'node';
          $skinr->sid = $form_state['values']['old_type'];
          $skinr->skins = array();

          skinr_set($skinr);
        }
      }
    return $form_state['values']['type'];
  }
}

/**
 * Skinr preprocess_hook_callback.
 *
 * @param &$form
 *   Passes in the $form parameter from hook_form_alter().
 * @param $form_state
 *   Passes in the $form_state parameter from hook_form_alter().
 * @return
 *   The preprocess_hook we wish to use.
 */
function node_skinr_preprocess_hook_callback(&$form, $form_state) {
  $preprocess_hooks = array();

  if (!isset($form['#node_type']->type) && !empty($form['skinr']['sid']['#value'])) {
    $preprocess_hooks[] = 'node_'. $form['skinr']['sid']['#value'];
  }
  else {
    $preprocess_hooks[] = 'node_'. $form['#node_type']->type;
  }
  $preprocess_hooks[] = 'node';

  return $preprocess_hooks;
}

/**
 * Skinr preprocess index handler.
 *
 * @param &$vars
 *   Passes in the $vars parameter from module_preprocess().
 * @return
 *   The index where we can find our values in Skinrs data structure. If an
 *   array is returned, it will loop through each index in Skinr's data
 *   structure and merge the returned classes.
 */
function node_skinr_preprocess_index_handler(&$vars) {
  return $vars['node']->type;
}

/**
 * @}
 */
