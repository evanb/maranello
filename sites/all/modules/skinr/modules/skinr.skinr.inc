<?php
// $Id: skinr.skinr.inc,v 1.1.2.3 2010/05/01 20:11:11 jgirlygirl Exp $
/**
 * @file
 * Provide skinr handling for page level rules.
 */

/**
 * @defgroup skinr page rule handlers
 *
 * @{
 */

/**
 * Implementation of hook_skinr_config().
 */
function skinr_skinr_config() {
  $data['page']['form']['skinr_rule_edit'] = array(
    'index_handler' => 'page_skinr_form_index_handler',
    'preprocess_hook' => 'page',
    'title' => t('page rule settings'),
    'skinr_weight' => 0,
    'collapsed' => FALSE,
  );
  $data['page']['form']['skinr_ui_form'] = array(
    'index_handler' => 'skinr_ajax_index_handler',
    'preprocess_hook' => 'page',
    'title' => t('page rule settings'),
    'collapsed' => FALSE,
  );
  $data['page']['preprocess']['page'] = array(
    'index_handler' => 'page_skinr_preprocess_index_handler',
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
 *   The index where we can find our values in Skinr's data structure.
 */
function page_skinr_form_index_handler($op, &$form, $form_state) {
  switch ($op) {
    case 'form':
      if (!empty($form['rule']['rid']['#value'])) {
        return $form['rule']['rid']['#value'];
      }
      else {
        return 0;
      }

    case 'submit':
      if (!empty($form_state['values']['rid'])) {
        return $form_state['values']['rid'];
      }
      else {
        // This is a rule, so we need to fetch the rid from DB.
        return(db_last_insert_id('skinr_rules', 'rid'));
      }
  }
}

/**
 * Skinr preprocess index handler.
 *
 * @param &$vars
 *   Passes in the $vars parameter from module_preprocess().
 * @return
 *   The index where we can find our values in Skinr's data structure. If an
 *   array is returned, it will loop through each index in Skinr's data
 *   structure and merge the returned classes.
 */
function page_skinr_preprocess_index_handler(&$vars) {
  $rules = skinr_rule_load();

  // Find any page level skinr options and return an array of them.
  $indices = array();
  foreach ($rules as $rule) {
    if (skinr_rule_visible($rule->rid)) {
      $indices[] = $rule->rid;
    }
  }
  return $indices;
}

/**
 * @}
 */
