<?php
// $Id: views.skinr.inc,v 1.5.2.1.2.6 2010/05/01 20:11:11 jgirlygirl Exp $

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
function views_skinr_config() {
  $data['views']['form']['views_ui_edit_display_form'] = array(
    'index_handler' => 'views_skinr_form_index_handler',
    'access_handler' => 'views_skinr_access_handler_display',
    'data_handler' => 'views_skinr_data_handler',
    'submit_handler' => 'views_skinr_submit_handler_display',
    'submit_handler_attach_to' => array('buttons', 'submit', '#submit'),
    'preprocess_hook_callback' => 'views_skinr_preprocess_hook_callback',
    'title' => t('views style settings'),
    'skinr_weight' => 0,
    'collapsed' => FALSE,
  );
  $data['views']['form']['views_ui_edit_view_form'] = array(
    'access_handler' => 'views_skinr_access_handler_view',
    'submit_handler' => 'views_skinr_submit_handler_view',
  );
  $data['views']['form']['skinr_ui_form'] = array(
    'index_handler' => 'skinr_ajax_index_handler',
    'preprocess_hook_callback' => 'views_skinr_preprocess_hook_callback',
    'title' => t('views style settings'),
    'collapsed' => FALSE,
  );
  $data['views']['preprocess']['views_view'] = array(
    'index_handler' => 'views_skinr_preprocess_index_handler',
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
function views_skinr_form_index_handler($op, &$form, $form_state) {
  switch ($op) {
    case 'form':
      $default = $form_state['view']->display_handler->is_defaulted('style_options');
      if ($default) {
        return 'view-'. $form_state['view']->name .'-display-default';
      }
      else {
        return 'view-'. $form_state['view']->name .'-display-'. $form_state['view']->current_display;
      }

    case 'submit':
      // Can't use display_handler->is_defaulted('style_options') on submit
      $default = ($form_state['#section'] == 'default-style_options');
      if ($default) {
        return 'view-'. $form_state['view']->name .'-display-default';
      }
      else {
        return 'view-'. $form_state['view']->name .'-display-'. $form_state['display_id'];
      }
  }
}

/**
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
function views_skinr_access_handler_display($op, &$form, $form_state) {
  if ($access = skinr_access_handler($op, $form, $form_state)) {
    // Views uses the same form_id for all forms, but it sets a $section
    // variable to distinguish between them. So only show the skinr settings
    // form on the style options form.
    if ($form_state['section'] != 'style_options') {
      $access = FALSE;
    }
  }

  return $access;
}

function views_skinr_access_handler_view($op, &$form, $form_state) {
  // We don't want the skinr settings form to appear on this form. We only want
  // to intercept it so we can save our cached data.

  // Since we're not using the form, we won't get our form submitter, so let's
  // add it manually.

  // Only add submit handler once
  if (!in_array('skinr_ui_form_submit', $form['#submit'])) {
    $form['#submit'][] = 'skinr_ui_form_submit';
  }
  // Special for views
  if (isset($form['buttons']['save']['#submit']) && !in_array('skinr_ui_form_submit', $form['buttons']['save']['#submit'])) {
    $form['buttons']['save']['#submit'][] = 'skinr_ui_form_submit';
  }
  if (isset($form['buttons']['cancel']['#submit']) && !in_array('skinr_ui_form_submit', $form['buttons']['cancel']['#submit'])) {
    // Need to make this happen before views' submit handler because it hijacks
    // things and does a drupal_goto(). That effectively prevents any other
    // submit handlers from being processed.
    array_unshift($form['buttons']['cancel']['#submit'], 'skinr_ui_form_submit');
  }

  return FALSE;
}

/**
 * Skinr data handler.
 * This is the data that populates the skinr form.
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
 *   A skinr object.
 */
function views_skinr_data_handler(&$form, $form_state, $theme, $module, $form_settings) {
  // Ensure we have the required index_handler
  if (empty($form_settings['index_handler'])) {
    trigger_error(sprintf("No index_handler was found for form_id '%s' in module '%s'.", $form_id, $module), E_USER_ERROR);
  }
  $index = skinr_handler('form_index_handler', 'form', $form_settings['index_handler'], $form, $form_state);

  // Fetch skinr data for this view from cache
  views_include('cache');

  if ($skinrs = views_object_cache_get('skinr', $form_state['view']->name)) {
    if (isset($skinrs[$theme][$index])) {
      $skinr = new stdClass();
      $skinr->theme = $theme;
      $skinr->module = $module;
      $skinr->sid = $index;
      $skinr->skins = $skinrs[$theme][$index];

      return $skinr;
    }
    else {
      return array();
    }
  }

  // No data exists in cache, so let's grab it from the regular source
  return skinr_get($theme, $module, $index);
}

/**
 * Skinr submit handler.
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
function views_skinr_submit_handler_display(&$form, $form_state, $module, $form_settings) {
  foreach ($form_state['values']['skinr_settings'][$module .'_group'] as $theme_name => $theme) {
    if ((isset($theme['widgets']) && count($theme['widgets'])) || isset($theme['_additional'])) {
      $sid   = skinr_handler('form_index_handler', 'submit', $form_settings['index_handler'], $form, $form_state);
      $value = array();

      if (is_array($theme['widgets'])) {
        foreach ($theme['widgets'] as $skin_id => $skin_value) {
          $value[$skin_id] = $skin_value;
        }
      }

      if (isset($theme['_additional'])) {
        $theme['_additional'] = trim($theme['_additional']);
        if (!empty($theme['_additional'])) {
          $value['_additional'] = $theme['_additional'];
        }
      }

      if (isset($theme['_template'])) {
        $theme['_template'] = trim($theme['_template']);
        if (!empty($theme['_template'])) {
          $value['_template'] = $theme['_template'];
        }
      }

      if (empty($sid)) {
        // We didn't receive a valid sid, so raise an error
        drupal_set_message(t("Skinr settings weren't saved due to an error."), 'error');
      }

      // Save skinr_settings for this view in cache
      views_include('cache');
      if (!$skinrs = views_object_cache_get('skinr', $form_state['view']->name, TRUE)) {
        $skinrs = array();
        // Fetch skinr data
        $skinr = skinr_get($theme_name);
        if (isset($skinr[$module])) {
          foreach ($skinr[$module] as $skinr_key => $skinr_value) {
            if (drupal_substr($skinr_key, 0, drupal_strlen('view-'. $form_state['view']->name)) == 'view-'. $form_state['view']->name) {
              $skinrs[$theme_name][$skinr_key] = $skinr_value;
            }
          }
        }
      }
      $skinrs[$theme_name][$sid] = $value;
      views_object_cache_set('skinr', $form_state['view']->name, $skinrs);
    }
  }
}

function views_skinr_submit_handler_view(&$form, $form_state, $module, $form_settings) {
  switch ($form_state['values']['op']) {
    case t('Save'):
      // Fetch skinr data for this view from cache
      views_include('cache');

      if ($skinrs = views_object_cache_get('skinr', $form_state['view']->name, TRUE)) {
        foreach ($skinrs as $theme_name => $theme) {
          foreach ($theme as $sid => $skins) {
            $skinr = new stdClass();
            $skinr->theme = $theme_name;
            $skinr->module = $module;
            $skinr->sid = $sid;
            $skinr->skins = $skins;

            skinr_set($skinr);
          }
        }
        views_object_cache_clear('skinr', $form_state['view']->name);
      }
      break;
    case t('Cancel'):
      // Remove our data from views cache
      views_include('cache');

      views_object_cache_clear('skinr', $form_state['view']->name);
      break;
  }
}

/**
 * Skinr form preprocess_hook callback.
 *
 * @param &$form
 *   Passes in the $form parameter from hook_form_alter().
 * @param $form_state
 *   Passes in the $form_state parameter from hook_form_alter().
 * @return
 *   The preprocess_hook we wish to use.
 */
function views_skinr_preprocess_hook_callback(&$form, $form_state) {
  $preprocess_hooks = array('views_view');

  if (!empty($form_state['view']) && !empty($form_state['view']->name)) {
    $view = $form_state['view'];
  }
  elseif(isset($form['skinr']['sid']['#value'])) {
    $sidinfo = _views_skinr_sidinfo($form['skinr']['sid']['#value']);
    if ($view = views_get_view($sidinfo['view'])) {
      $view->execute_display($sidinfo['display']);
    }
  }

  if (!empty($view)) {
    $display = $view->display[$view->current_display];

    // Create list of suggested templates.
    $preprocess_hooks = views_theme_functions('views_view', $view, $display);
    // Fetch additional style based suggested templates.
    $additional_hooks = views_theme_functions($view->style_plugin->definition['theme'], $view, $display);
    
    $preprocess_hooks = array_merge($additional_hooks, $preprocess_hooks);
  }

  return $preprocess_hooks;
}

function _views_skinr_sidinfo($sid) {
  // view-[view_name]-display-[display_name]
  $pos = strpos($sid, '-display-');
  if ($pos !== FALSE) {
    $sidinfo = array();
    $sidinfo['view'] = substr($sid, strlen('view-'), $pos - strlen('view-'));
    $sidinfo['display'] = substr($sid, $pos + strlen('-display-'));

    return $sidinfo;
  }

  return FALSE;
}

/**
 * Skinr preprocess index handler.
 *
 * @param &$vars
 *   Passes in the $vars parameter from skinr_preprocess().
 * @return
 *   The index where we can find our values in Skinrs data structure. If an
 *   array is returned, it will loop through each index in Skinr's data
 *   structure and merge the returned classes.
 */
function views_skinr_preprocess_index_handler(&$vars) {
  $default = $vars['view']->display_handler->is_defaulted('style_options');
  if ($default) {
    return 'view-'. $vars['view']->name .'-display-default';
  }
  else {
    return 'view-'. $vars['view']->name .'-display-'. $vars['view']->current_display;
  }
}

/**
 * @}
 */
