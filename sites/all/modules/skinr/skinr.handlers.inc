<?php
// $Id: skinr.handlers.inc,v 1.2.4.6 2010/06/09 13:52:25 moonray Exp $
/**
 * @file
 * Defines the various handler objects to support Skinr.
 */

/*
 * Skinr access handler.
 *
 * @param $op
 *   What kind of action is being performed. Possible values:
 *   - "access skinr": access to edit skinr's selector
 *   - "access skinr classes": access to edit skinr's additional classes
 * @param &$form
 *   Passes in the $form parameter from hook_form_alter().
 * @param $form_state
 *   Passes in the $form_state parameter from hook_form_alter().
 * @return
 *   TRUE if we get access, FALSE if we don't.
 */
function skinr_access_handler($op, &$form, $form_state) {
  switch ($op) {
    case 'access skinr':
      return user_access('access skinr') || user_access('administer skinr');
    case 'access skinr classes':
      return user_access('access skinr classes') || user_access('administer skinr');
  }
}

/**
 * Skinr form index handler for ajax form.
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
function skinr_ajax_index_handler($op, &$form, $form_state) {
  switch ($op) {
    case 'form':
      if (empty($form['skinr']['sid']['#value'])) {
        trigger_error(sprintf("The form with form_id '%s' is not a valid AJAX form.", $form['form_id']['#value']), E_USER_ERROR);
        return FALSE;
      }
      return $form['skinr']['sid']['#value'];

    case 'submit':
      if (empty($form_state['values']['sid'])) {
        trigger_error(sprintf("The form with form_id '%s' is not a valid AJAX form.", $form['form_id']['#value']), E_USER_ERROR);
        return FALSE;
      }
      return $form_state['values']['sid'];
  }
}

/*
 * Skinr data handler.
 *
 * @param &$form
 *   Passes in the $form parameter from hook_form_submit().
 * @param $form_state
 *   Passes in the $form_state parameter from hook_form_submit().
 * @param $module
 *   The module that is currently being processed.
 * @param $form_settings
 *   The settings from hook_skinr_config() for the form that's currently being
 *   processed.
 * @return
 *   TRUE if we get access, FALSE if we don't.
 */
function skinr_data_handler(&$form, $form_state, $theme, $module, $form_settings) {
  // Ensure we have the required index_handler
  if (empty($form_settings['index_handler'])) {
    trigger_error(sprintf("No index_handler was found for form_id '%s' in module '%s'.", $form_id, $module), E_USER_ERROR);
  }
  $index = skinr_handler('form_index_handler', 'form', $form_settings['index_handler'], $form, $form_state);

  return skinr_get($theme, $module, $index);
}

/**
 * Skinr submit handler.
 *
 * @param &$form
 *   Passes in the $form parameter from hook_form_submit().
 * @param $form_state
 *   Passes in the $form_state parameter from hook_form_submit().
 * @param $module
 *   The module that is currently being processed.
 * @param $form_settings
 *   The settings from hook_skinr_config() for the form that's currently being
 *   processed.
 * @return
 *   TRUE if we get access, FALSE if we don't.
 */
function skinr_submit_handler(&$form, $form_state, $module, $form_settings) {
  if (isset($form_state['values']['skinr_settings'][$module .'_group'])) {
    foreach ($form_state['values']['skinr_settings'][$module .'_group'] as $theme_name => $theme) {
      if ((isset($theme['widgets']) && count($theme['widgets'])) || isset($theme['_additional'])) {
        $skinr = new stdClass();
        $skinr->theme = $theme_name;
        $skinr->module = $module;
        $skinr->sid = skinr_handler('form_index_handler', 'submit', $form_settings['index_handler'], $form, $form_state);
        $skinr->skins = array();

        if (isset($theme['widgets']) && is_array($theme['widgets'])) {
          foreach ($theme['widgets'] as $skin_id => $skin_value) {
            $skinr->skins[$skin_id] = $skin_value;
          }
        }

        if (isset($theme['_additional'])) {
          $theme['_additional'] = trim($theme['_additional']);
          if (!empty($theme['_additional'])) {
            $skinr->skins['_additional'] = $theme['_additional'];
          }
        }

        if (isset($theme['_template'])) {
          $theme['_template'] = trim($theme['_template']);
          if (!empty($theme['_template'])) {
            $skinr->skins['_template'] = $theme['_template'];
          }
        }

        if (empty($skinr->sid)) {
          // We didn't receive a valid sid, so raise an error
          drupal_set_message(t("Skinr settings weren't saved due to an error."), 'error');
        }

        skinr_set($skinr);
      }
    }
  }
}

// Declare API compatibility on behalf of core modules:

/**
 * Implementation of hook_skinr_api().
 *
 * This one is used as the base to reduce errors when updating.
 */
function skinr_skinr_api() {
  return array(
    'api' => 1,
    'path' => drupal_get_path('module', 'skinr') .'/modules',
  );
}

function block_skinr_api() { return skinr_skinr_api(); }

function content_skinr_api() { return skinr_skinr_api(); }

function comment_skinr_api() { return skinr_skinr_api(); }

function fieldgroup_skinr_api() { return skinr_skinr_api(); }

function node_skinr_api() { return skinr_skinr_api(); }

function panels_skinr_api() { return skinr_skinr_api(); }

function views_skinr_api() { return skinr_skinr_api(); }
