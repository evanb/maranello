<?php
// $Id: content.skinr.inc,v 1.1.2.1 2010/06/09 13:52:25 moonray Exp $

/**
 * @file
 * Provide skinr handling for content.module
 */

/**
 * @defgroup skinr_content_module content.module handlers
 *
 * @{
 */

/**
 * Implementation of hook_skinr_config().
 */
function content_skinr_config() {
  $data['content']['form']['content_field_edit_form'] = array(
    'index_handler' => 'content_skinr_form_index_handler',
    'preprocess_hook_callback' => 'content_skinr_preprocess_hook_callback',
    'title' => t('content settings'),
  );
  $data['content']['form']['skinr_ui_form'] = array(
    'index_handler' => 'skinr_ajax_index_handler',
    'preprocess_hook_callback' => 'content_skinr_preprocess_hook_callback',
    'title' => t('content settings'),
    'collapsed' => FALSE,
  );
  $data['content']['preprocess']['content_field'] = array(
    'index_handler' => 'content_skinr_preprocess_index_handler',
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
function content_skinr_form_index_handler($op, &$form, $form_state) {
  switch ($op) {
    case 'form':
      // Fix weights for this form to have the proper order of elements.
      $form['field']['#weight'] = 2;
      $form['submit']['#weight'] = 50;
      
      return $form['#field']['field_name'] .'_'. $form['#field']['type_name'];

    case 'submit':
      return $form_state['values']['field_name'] .'_'. $form_state['values']['type_name'];
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
function content_skinr_preprocess_hook_callback(&$form, $form_state) {
  $preprocess_hooks = array();

  $preprocess_hooks[] = 'content_field';

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
function content_skinr_preprocess_index_handler(&$vars) {
  return $vars['field_name'] .'_'. $vars['node']->type;
}

/**
 * @}
 */
