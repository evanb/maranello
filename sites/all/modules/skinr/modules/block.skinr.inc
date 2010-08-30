<?php
// $Id: block.skinr.inc,v 1.3.2.1.2.6 2010/04/05 22:41:45 moonray Exp $
/**
 * @file
 * Provide skinr handling for block.module
 */

/**
 * @defgroup skinr_block_module block.module handlers
 *
 * @{
 */

/**
 * Implementation of hook_skinr_config().
 */
function block_skinr_config() {
  $data['block']['form']['block_admin_configure'] = array(
    'index_handler' => 'block_skinr_form_index_handler',
    'preprocess_hook_callback' => 'block_skinr_preprocess_hook_callback',
    'title' => t('block settings'),
  );
  $data['block']['form']['block_add_block_form'] = array(
    'index_handler' => 'block_skinr_form_index_handler',
    'preprocess_hook_callback' => 'block_skinr_preprocess_hook_callback',
    'title' => t('block settings'),
  );
  $data['block']['form']['skinr_ui_form'] = array(
    'index_handler' => 'skinr_ajax_index_handler',
    'preprocess_hook_callback' => 'block_skinr_preprocess_hook_callback',
    'title' => t('block settings'),
    'collapsed' => FALSE,
  );
  $data['block']['preprocess']['block'] = array(
    'index_handler' => 'block_skinr_preprocess_index_handler',
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
function block_skinr_form_index_handler($op, &$form, $form_state) {
  switch ($op) {
    case 'form':
      $form['submit']['#weight'] = 50;
      return $form['module']['#value'] .'-'. $form['delta']['#value'];

    case 'submit':
      $form_id = $form['form_id']['#value'];
      if ($form_id == 'block_add_block_form') {
        // This is a new block, so we need to fetch the delta from DB
        if ($delta = db_result(db_query("SELECT delta FROM {blocks} WHERE bid = '%s'", db_last_insert_id('boxes', 'bid')))) {
          return $form_state['values']['module'] .'-'. $delta;
        }
      }
      else {
        return $form_state['values']['module'] .'-'. $form_state['values']['delta'];
      }
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
function block_skinr_preprocess_hook_callback(&$form, $form_state) {
  $preprocess_hooks = array();

  if (!isset($form['module']['#value']) && isset($form['skinr']['sid']['#value'])) {
    $result = db_query_range(db_rewrite_sql("SELECT DISTINCT b.* FROM {blocks} b WHERE CONCAT(b.module, '-', b.delta) = '%s'", 'b', 'bid'), $form['skinr']['sid']['#value'], 0, 1);
    while ($block = db_fetch_object($result)) {
      $preprocess_hooks[] = 'block_'. $block->module;
    }
  }
  else {
    $preprocess_hooks[] = 'block_'. $form['module']['#value'];
  }
  $preprocess_hooks[] = 'block';

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
function block_skinr_preprocess_index_handler(&$vars) {
  return $vars['block']->module .'-'. $vars['block']->delta;
}

/**
 * @}
 */
