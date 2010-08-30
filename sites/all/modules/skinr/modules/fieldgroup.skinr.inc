<?php
// $Id: fieldgroup.skinr.inc,v 1.1.2.1 2010/06/09 13:52:25 moonray Exp $

/**
 * @file
 * Provide skinr handling for fieldgroup.module
 */

/**
 * @defgroup skinr_fieldgroup_module fieldgroup.module handlers
 *
 * @{
 */

/**
 * Implementation of hook_skinr_config().
 */
function fieldgroup_skinr_config() {
  $data['fieldgroup']['form']['fieldgroup_group_edit_form'] = array(
    'index_handler' => 'fieldgroup_skinr_form_index_handler',
    'preprocess_hook_callback' => 'fieldgroup_skinr_preprocess_hook_callback',
    'title' => t('fieldgroup settings'),
  );
  $data['fieldgroup']['form']['skinr_ui_form'] = array(
    'index_handler' => 'skinr_ajax_index_handler',
    'preprocess_hook_callback' => 'fieldgroup_skinr_preprocess_hook_callback',
    'title' => t('fieldgroup settings'),
    'collapsed' => FALSE,
  );
  $data['fieldgroup']['preprocess']['fieldgroup_simple'] = array(
    'index_handler' => 'fieldgroup_skinr_preprocess_index_handler',
  );
  $data['fieldgroup']['preprocess']['fieldgroup_fieldset'] = array(
    'index_handler' => 'fieldgroup_skinr_preprocess_index_handler',
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
function fieldgroup_skinr_form_index_handler($op, &$form, $form_state) {
  switch ($op) {
    case 'form':
      // Fix weights for this form to have the proper order of elements.
      $form['submit']['#weight'] = 50;
      
      return $form['group_name']['#default_value'] .'_'. $form['#content_type']['type'];

    case 'submit':
      return $form_state['values']['group_name'] .'_'. $form['#content_type']['type'];
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
function fieldgroup_skinr_preprocess_hook_callback(&$form, $form_state) {
  $preprocess_hooks = array();

  $preprocess_hooks[] = 'fieldgroup_simple';
  $preprocess_hooks[] = 'fieldgroup_fieldset';

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
function fieldgroup_skinr_preprocess_index_handler(&$vars) {
  return $vars['element']['#group_name'] .'_'. $vars['element']['#node']->type;
}

/**
 * Implementation of hook_fieldgroup_view().
 */
function skinr_fieldgroup_view($node, &$element, &$group, $context) {
  // We need to fake a preprocess to get our skinr data, since these
  // renderings of the fieldgroup don't have a preprocess hook.
  $exclude = array('no_style', 'simple', 'hidden');
  if (!in_array($group['settings']['display'][$context]['format'], $exclude)) {
    $vars = array(
      'element' => $element,
    );
    $vars['element']['#group_name'] = $group['group_name'];
    $vars['element']['#node'] = $node;

    skinr_preprocess($vars, 'fieldgroup_fieldset');
    
    if ($vars['skinr']) {
      $element['#attributes']['class'] .= ' '. $vars['skinr'];
    }
  }
}

/**
 * @}
 */
