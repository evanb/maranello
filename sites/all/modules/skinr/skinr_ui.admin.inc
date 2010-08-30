<?php
// $Id: skinr_ui.admin.inc,v 1.1.2.16 2010/08/11 15:12:44 moonray Exp $

/**
 * @file
 * Admin page callbacks for the skinr module.
 */

/**
 * Menu callback; displays the skinr listing page.
 */
function skinr_ui_list($form_state) {
  if (isset($form_state['values']['operation']) && $form_state['values']['operation'] == 'delete') {
    return skinr_ui_settings_multiple_delete_confirm($form_state, array_filter($form_state['values']['skinr_settings']));
  }
  $form = skinr_ui_filter_form();

  $form['#theme'] = 'skinr_ui_filter_form';
  $form['admin']  = skinr_ui_admin_skins();

  return $form;
}

/**
 * Form builder: Builds the skinr administration overview.
 */
function skinr_ui_admin_skins() {
  $session = !empty($_SESSION['skinr_ui_overview_filter']) && is_array($_SESSION['skinr_ui_overview_filter']) ? $_SESSION['skinr_ui_overview_filter'] : array();
  $filters = array();
  foreach ($session as $filter) {
    list($key, $value) = $filter;
    $filters[$key] = $value;
  }

  $sorts = array();

  $form['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Update options'),
    '#prefix' => '<div class="container-inline">',
    '#suffix' => '</div>',
  );
  $options = array();
  foreach (module_invoke_all('skinr_operations') as $operation => $array) {
    $options[$operation] = $array['label'];
  }
  $form['options']['operation'] = array(
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => 'approve',
  );
  $form['options']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
    '#submit' => array('skinr_ui_admin_skins_submit'),
  );

  $themes = list_themes();
  ksort($themes);

  $skinr_settings = array();
  $options = array();
  $destination = drupal_get_destination();
  $type = !empty($filters['type']) ? $filters['type'] : NULL;

  foreach ($themes as $theme) {
    if (!$theme->status || (!empty($filters['theme']) && $filters['theme'] != $theme->name)) {
      continue;
    }
    if (empty($type)) {
      $skinrs = skinr_get($theme->name);
    }
    else {
      $skinrs = array($type => skinr_get($theme->name, $type));
    }

    foreach ($skinrs as $module => $elements) {
      foreach ($elements as $sid => $skin) {
        $key = $theme->name .'__'. $module .'__'. $sid;
        $skinr_settings[$key] = '';

        if ($module == 'page') {
          $url_edit   = 'admin/build/skinr/rule/edit/'. $sid;
        }
        else {
          $url_edit = 'admin/build/skinr/edit/nojs/'. $module .'/'. $sid;
        }
        $url_delete = 'admin/build/skinr/delete/'. $theme->name .'/'. $module .'/'. $sid;
        $operations = array(
          'edit' => array(
            'title' => t('edit'),
            'href' => $url_edit,
            'query' => $destination,
          ),
          'delete' => array(
            'title' => t('delete'),
            'href' => $url_delete,
            'query' => $destination,
          ),
        );
        $options[$key] = array(
          'theme' => $theme->info['name'],
          'type' => $module,
          'sid' => $sid,
          'operations' => theme('links', $operations, array('class' => 'links inline')),
        );
      }
    }
  }

  $form['rows'] = array('#type' => 'value', '#value' => $options);
  $form['skinr_settings'] = array('#type' => 'checkboxes', '#options' => $skinr_settings);
  $form['#theme'] = 'skinr_ui_admin_skins';

  return $form;
}

/**
 * Validate skinr_ui_admin_skins form submissions.
 *
 * Check if any skinr settings have been selected to perform the chosen
 * 'Update option' on.
 */
function skinr_ui_admin_skins_validate($form, &$form_state) {
  $skinr_settings = array_filter($form_state['values']['skinr_settings']);
  if (count($skinr_settings) == 0) {
    form_set_error('', t('No items selected.'));
  }
}

/**
 * Process skinr_ui_admin_skins form submissions.
 * 
 * Execute the chosen 'Update option' on the selected nodes.
 */
function skinr_ui_admin_skins_submit($form, &$form_state) {
  $operations = module_invoke_all('skinr_operations');
  $operation = $operations[$form_state['values']['operation']];
  // Filter out unchecked nodes
  $skinr_settings = array_filter($form_state['values']['skinr_settings']);
  if ($function = $operation['callback']) {
    // Add in callback arguments if present.
    if (isset($operation['callback arguments'])) {
      $args = array_merge(array($nodes), $operation['callback arguments']);
    }
    else {
      $args = array($skinr_settings);
    }
    call_user_func_array($function, $args);

    cache_clear_all();
  }
  else {
    // We need to rebuild the form to go to a second step.  For example, to
    // show the confirmation form for the deletion of nodes.
    $form_state['rebuild'] = TRUE;
  }
}

/**
 * Implementation of hook_skinr_operations().
 */
function skinr_ui_skinr_operations() {
  $operations = array(
    'delete' => array(
      'label' => t('Delete'),
      'callback' => NULL,
    ),
  );
  return $operations;
}

/**
 * List skinr administration filters that can be applied.
 */
function skinr_ui_filters() {
  // Theme filter.
  $themes = list_themes();
  ksort($themes);

  $options = array();
  foreach ($themes as $theme) {
    if (!$theme->status) {
      continue;
    }
    $options[$theme->name] = $theme->info['name'];
  }

  $filters['theme'] = array(
    'title' => t('theme'),
    'options' => $options,
  );

  // Type filter.
  $config = skinr_fetch_config();

  $options = array();
  foreach ($config as $type => $data) {
    $options[$type] = $type;
  }

  $filters['type'] = array(
    'title' => t('type'),
    'options' => $options,
  );

  return $filters;
}

/**
 * Return form for skinr administration filters.
 */
function skinr_ui_filter_form() {
  $session = &$_SESSION['skinr_ui_overview_filter'];
  $session = is_array($session) ? $session : array();
  $filters = skinr_ui_filters();

  $i = 0;
  $form['filters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Show only items where'),
    '#theme' => 'skinr_ui_filters',
  );
  $form['#submit'][] = 'skinr_ui_filter_form_submit';
  foreach ($session as $filter) {
    list($type, $value) = $filter;
    $value = $filters[$type]['options'][$value];

    if ($i++) {
      $form['filters']['current'][] = array('#value' => t('<em>and</em> where <strong>%a</strong> is <strong>%b</strong>', array('%a' => $filters[$type]['title'], '%b' => $value)));
    }
    else {
      $form['filters']['current'][] = array('#value' => t('<strong>%a</strong> is <strong>%b</strong>', array('%a' => $filters[$type]['title'], '%b' => $value)));
    }
    if (in_array($type, array('theme', 'type'))) {
      // Remove the option if it is already being filtered on.
      unset($filters[$type]);
    }
  }

  foreach ($filters as $key => $filter) {
    $names[$key] = $filter['title'];
    $form['filters']['status'][$key] = array('#type' => 'select', '#options' => $filter['options']);
  }

  $form['filters']['filter'] = array('#type' => 'radios', '#options' => $names, '#default_value' => 'status');
  if (count($filters)) {
    $form['filters']['buttons']['submit'] = array('#type' => 'submit', '#value' => (count($session) ? t('Refine') : t('Filter')));
  }
  if (count($session)) {
    $form['filters']['buttons']['undo'] = array('#type' => 'submit', '#value' => t('Undo'));
    $form['filters']['buttons']['reset'] = array('#type' => 'submit', '#value' => t('Reset'));
  }

  drupal_add_js('misc/form.js', 'core');

  return $form;
}

/**
 * Theme skinr administration filter form.
 *
 * @ingroup themeable
 */
function theme_skinr_ui_filter_form($form) {
  $output = '';
  $output .= '<div id="skinr-ui-admin-filter">';
  $output .= drupal_render($form['filters']);
  $output .= '</div>';
  $output .= drupal_render($form);
  return $output;
}

/**
 * Theme skinr administration filter selector.
 *
 * @ingroup themeable
 */
function theme_skinr_ui_filters($form) {
  $output = '';
  $output .= '<ul class="clear-block">';
  if (!empty($form['current'])) {
    foreach (element_children($form['current']) as $key) {
      $output .= '<li>'. drupal_render($form['current'][$key]) .'</li>';
    }
  }

  $output .= '<li><dl class="multiselect">';
  
  $element_children = element_children($form['filter']);
  if (!empty($element_children)) {
    $output .= (!empty($form['current']) ? '<dt><em>'. t('and') .'</em> '. t('where') .'</dt>' : '') .'<dd class="a">';
    foreach ($element_children as $key) {
      $output .= drupal_render($form['filter'][$key]);
    }
    $output .= '</dd>';
  
    $output .= '<dt>'. t('is') .'</dt>';
  }
  
  $output .= '<dd class="b">';

  foreach (element_children($form['status']) as $key) {
    $output .= drupal_render($form['status'][$key]);
  }
  $output .= '</dd>';

  $output .= '</dl>';
  $output .= '<div class="container-inline" id="node-admin-buttons">'. drupal_render($form['buttons']) .'</div>';
  $output .= '</li></ul>';

  return $output;
}

/**
 * Process result from node administration filter form.
 */
function skinr_ui_filter_form_submit($form, &$form_state) {
  $filters = skinr_ui_filters();
  switch ($form_state['values']['op']) {
    case t('Filter'):
    case t('Refine'):
      if (isset($form_state['values']['filter'])) {
        $filter = $form_state['values']['filter'];

        // Flatten the options array to accommodate hierarchical/nested options.
        $flat_options = form_options_flatten($filters[$filter]['options']);

        if (isset($flat_options[$form_state['values'][$filter]])) {
          $_SESSION['skinr_ui_overview_filter'][] = array($filter, $form_state['values'][$filter]);
        }
      }
      break;
    case t('Undo'):
      array_pop($_SESSION['skinr_ui_overview_filter']);
      break;
    case t('Reset'):
      $_SESSION['skinr_ui_overview_filter'] = array();
      break;
  }
}

/**
 * Menu callback; displays the skinr settings administration form.
 */
function skinr_ui_admin_form(&$form_state) {
  $form = array();

  $form['overlay'] = array(
    '#type' => 'fieldset',
    '#title' => t('Overlay'),
    '#collapsible' => TRUE,
  );
  $form['overlay']['skinr_overlay_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Window width'),
    '#default_value' => variable_get('skinr_overlay_width', 600),
    '#description' => t('The width of the overlay window. Leave this field blank to allow the window to automatically resize itself to a minimal width.'),
  );
  $form['overlay']['skinr_overlay_height'] = array(
    '#type' => 'textfield',
    '#title' => t('Window height'),
    '#default_value' => variable_get('skinr_overlay_height', NULL),
    '#description' => t('The height of the overlay window. Leave this field blank to allow the window to automatically resize itself to the height of your browser.'),
  );

  return system_settings_form($form);
}

/**
 * Validation handler for skinr settings administration form.
 */
function skinr_ui_admin_form_validate(&$form, &$form_state) {
  if (empty($form_state['values']['skinr_overlay_width'])) {
    $form_state['values']['skinr_overlay_width'] = NULL;
  }
  if (empty($form_state['values']['skinr_overlay_height'])) {
    $form_state['values']['skinr_overlay_height'] = NULL;
  }
}

/**
 * Helper function to sort skinsets by the name in their .info file.
 */
function skinr_ui_sort_by_info_name($a, $b) {
  return strcasecmp($a->info['name'], $b->info['name']);
}

/**
 * Menu callback; displays a listing of all non-theme skinsets.
 *
 * @ingroup forms
 * @see skinr_ui_skinsets_form_submit()
 */
function skinr_ui_skinsets_form() {
  $skinsets = skinr_rebuild_skinset_data();

  uasort($skinsets, 'skinr_ui_sort_by_info_name');

  // Store module list for use in the theme function.
  $form['skinsets'] = array('#type' => 'value', '#value' => $skinsets);

  $options = array();
  $status = array();
  $incompatible_core = array();
  $incompatible_php = array();

  foreach ($skinsets as $skinset) {
    $screenshot = NULL;
    $skinset_keys = array($skinset->name);
    // Look for a screenshot in the current skinset.
    if (isset($skinsets[$skinset->name]) && file_exists($skinsets[$skinset->name]->info['screenshot'])) {
      $screenshot = $skinsets[$skinset->name]->info['screenshot'];
    }
    $screenshot = $screenshot ? theme('image', $screenshot, t('Screenshot for %theme theme', array('%theme' => $skinset->info['name'])), '', array('class' => 'screenshot'), FALSE) : t('no screenshot');

    $form[$skinset->name]['screenshot'] = array('#value' => $screenshot);
    $form[$skinset->name]['info'] = array(
      '#type' => 'value',
      '#value' => $skinset->info,
    );
    $options[$skinset->name] = '';

    if (!empty($skinset->status)) {
      $status[] = $skinset->name;
    }
    else {
      // Ensure this theme is compatible with this version of core.
      if (!isset($skinset->info['core']) || $skinset->info['core'] != DRUPAL_CORE_COMPATIBILITY) {
        $incompatible_core[] = $skinset->name;
      }
      if (version_compare(phpversion(), $skinset->info['php']) < 0) {
        $incompatible_php[$skinset->name] = $skinset->info['php'];
      }
    }

    $form[$skinset->name]['operations'] = array(
      '#value' => l('configure', 'admin/build/skinr/skins/settings/'. $skinset->name),
    );
  }

  $form['status'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => $status,
    '#incompatible_skinsets_core' => drupal_map_assoc($incompatible_core),
    '#incompatible_skinsets_php' => $incompatible_php,
  );
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  $form['buttons']['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset to defaults'),
  );
  return $form;
}

/**
 * Menu callback; displays a listing of all skins in a skinsets, allowing you
 * to enable or disable them individually for each theme.
 *
 * @ingroup forms
 * @see skinr_ui_skinsets_settings_form_submit()
 */
function skinr_ui_skinsets_settings_form($form_state, $skinset_name) {
  $form = array(
    '#tree' => TRUE,
  );

  $skinsets = skinr_skinsets('skinset');
  if (!empty($skinsets[$skinset_name])) {
    $skinset = $skinsets[$skinset_name];
  }

  $themes = list_themes();
  ksort($themes);

  foreach ($skinset->skins as $skin_name => $skin) {
    $form[$skin_name]['info'] = array(
      '#type' => 'value',
      '#value' => $skin,
    );

    $status = array();
    $options = array();
    foreach ($themes as $theme) {
      if (!$theme->status) {
        continue;
      }

      $options[$theme->name] = '';

      if (!empty($skin['status'][$theme->name])) {
        $status[] = $theme->name;
      }
    }

    $form[$skin_name]['status'] = array(
      '#type' => 'checkboxes',
      '#options' => $options,
      '#default_value' => $status,
    );
  }

  $form['skinset'] = array(
    '#type' => 'value',
    '#value' => $skinset_name,
  );
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  $form['buttons']['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset to defaults'),
  );

  return $form;
}

/**
 * Process skinr_ui_skinsets_form form submissions.
 */
function skinr_ui_skinsets_form_submit($form, &$form_state) {
  // Store list of previously enabled themes and disable all themes
  $old_skinset_list = $new_skinset_list = array();
  foreach (skinr_skinsets('skinset') as $skinset) {
    if ($skinset->status) {
      $old_skinset_list[] = $skinset->name;
    }
  }
  db_query("UPDATE {skinr_skinsets} SET status = 0");

  if ($form_state['values']['op'] == t('Save configuration')) {
    if (is_array($form_state['values']['status'])) {
      foreach ($form_state['values']['status'] as $key => $choice) {
        if ($choice) {
          $new_theme_list[] = $key;
          db_query("UPDATE {skinr_skinsets} SET status = 1 WHERE name = '%s'", $key);
        }
      }
    }
  }
  else {
    // Revert to defaults: disable all skinsets.
  }

  // Refresh skinsets from DB.
  skinr_skinsets('skinset', TRUE);

  // @todo Disable any skins from skinsets that are now disabled.

  drupal_set_message(t('The configuration options have been saved.'));
  $form_state['redirect'] = 'admin/build/skinr/skins';

  return;
}

/**
 * Process skinr_ui_skinsets_settings_form form submissions.
 */
function skinr_ui_skinsets_settings_form_submit($form, &$form_state) {
  if ($form_state['values']['op'] == t('Save configuration')) {
    foreach ($form_state['values'] as $skin_name => $skin) {
      if (is_array($form_state['values'][$skin_name]['status'])) {
        $skinr_skin = new StdClass();
        $skinr_skin->name = $form_state['values']['skinset'];
        $skinr_skin->type = 'skinset';
        $skinr_skin->skin = $skin_name;
        $skinr_skin->status = _skinr_array_strip_empty($form_state['values'][$skin_name]['status']);

        if (db_result(db_query("SELECT 1 FROM {skinr_skins} WHERE name = '%s' AND skin = '%s'", $skinr_skin->name, $skinr_skin->skin))) {
          // Record exists, so let's update.
          drupal_write_record('skinr_skins', $skinr_skin, array('name', 'skin'));
        }
        else {
          // Insert a new record.
          drupal_write_record('skinr_skins', $skinr_skin);
        }
      }
    }
  }
  else {
    // Revert to defaults: disable all skinsets.
  }

  drupal_set_message(t('The configuration options have been saved.'));
  $form_state['redirect'] = 'admin/build/skinr/skins';

  return;
}

/**
 * Skinr settings export form.
 */
function skinr_ui_export_form(&$form_state, $theme = NULL) {
  $form = array();
  $themes = list_themes();

  if ($theme) {
    // Export an individual theme.
    $theme = str_replace('-', '_', $theme);
    $skinrs = skinr_get($theme);

    // Convert classes to arrays.
    foreach ($skinrs as $module => $data) {
      foreach ($data as $sid => $skinr) {
        $skinrs[$module][$sid] = (array) $skinrs[$module][$sid];
      }
    }

    $code = '$skinrs = '. var_export($skinrs, TRUE) .';';
    $lines = substr_count($code, "\n") + 1;

    $form['theme'] = array(
      '#type' => 'textfield',
      '#title' => t('Theme'),
      '#value' => $themes[$theme]->info['name'],
      '#disabled' => TRUE,
    );

    $form['skinr_settings'] = array(
      '#type' => 'textarea',
      '#title' => t('Skinr settings'),
      '#default_value' => $code,
      '#rows' => min($lines, 150),
    );
  }
  else {
    // Give the option for which theme to export.

    $options = array();
    ksort($themes);
    $current_theme = skinr_current_theme(TRUE);

    // Put default theme at the top.
    $options[$current_theme] = $themes[$current_theme]->info['name'].' ['. t('default') .']';

    foreach ($themes as $theme) {
      if ($theme->name == $current_theme) {
        // Do nothing.
      }
      elseif ($theme->status) {
        $options[$theme->name] = $theme->info['name'] .' ['. t('enabled') .']';
      }
      else {
        $options[$theme->name] = $theme->info['name'];
      }
    }

    $form['theme'] = array(
      '#type' => 'select',
      '#title' => t('Theme'),
      '#description' => t('Theme to export the skinr settings for.'),
      '#options' => $options,
      '#required' => TRUE,
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Export'),
    );
  }

  return $form;
}

/**
 * Validation handler for skinr settings export form.
 */
function skinr_ui_export_form_validate(&$form, &$form_state) {
  if (!empty($form_state['values']['theme']) && preg_match('/[^a-zA-Z0-9_]/', $form_state['values']['theme'])) {
    form_error($form['theme'], t('The theme name must be alphanumeric and can contain underscores only.'));
  }
}

/**
 * Submit handler for skinr settings export form.
 */
function skinr_ui_export_form_submit(&$form, &$form_state) {
  drupal_goto('admin/build/skinr/export/'. str_replace('_', '-', $form_state['values']['theme']));
}

/**
 * Skinr settings import form.
 */
function skinr_ui_import_form(&$form_state) {
  $form = array();

  $options = array();
  $themes = list_themes();
  ksort($themes);
  $current_theme = skinr_current_theme(TRUE);

  // Put default theme at the top.
  $options[$current_theme] = $themes[$current_theme]->info['name'].' ['. t('default') .']';

  foreach ($themes as $theme) {
    if ($theme->name == $current_theme) {
      // Do nothing.
    }
    elseif ($theme->status) {
      $options[$theme->name] = $theme->info['name'] .' ['. t('enabled') .']';
    }
    else {
      $options[$theme->name] = $theme->info['name'];
    }
  }

  $form['theme'] = array(
    '#type' => 'select',
    '#title' => t('Theme'),
    '#options' => $options,
    '#required' => TRUE,
  );
  $form['skinr_settings'] = array(
    '#type' => 'textarea',
    '#title' => t('Skinr settings'),
    '#description' => t('Paste skinr settings here.'),
    '#rows' => 16,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );

  return $form;
}

/**
 * Validation handler for Skinr settings import form.
 */
function skinr_ui_import_form_validate(&$form, &$form_state) {
  if (empty($form_state['values']['skinr_settings'])) {
    // Error.
    form_error($form['skinr_settings'], t('These are not valid Skinr settings.'));
    return;
  }

  $skinrs = '';
  ob_start();
  eval($form_state['values']['skinr_settings']);
  ob_end_clean();

  foreach ($skinrs as $module => $ignored) {
    if (!is_array($skinrs[$module])) {
      // Error.
      form_error($form['skinr_settings'], t('These are not valid Skinr settings.'));
    }
    foreach ($skinrs[$module] as $sid => $ignored) {
      if (!is_array($skinrs[$module][$sid])) {
        // Error.
        form_error($form['skinr_settings'], t('These are not valid Skinr settings.'));
      }

      // Cast skinr array to object so we can actually use it.
      $skinrs[$module][$sid] = (object) $skinrs[$module][$sid];
      if (!skinr_validate($skinrs[$module][$sid])) {
        // Error.
        form_error($form['skinr_settings'], t('These are not valid Skinr settings.'));
      }
    }
  }

  if (!empty($form_state['values']['theme']) && preg_match('/[^a-zA-Z0-9_]/', $form_state['values']['theme'])) {
    form_error($form['theme'], t('The theme name must be alphanumeric and can contain underscores only.'));
  }

  $form_state['skinrs'] = &$skinrs;
}

/**
 * Submit handler for skinr settings import form.
 */
function skinr_ui_import_form_submit(&$form, &$form_state) {
  foreach ($form_state['skinrs'] as $module => $skinrs) {
    foreach ($skinrs as $sid => $skinr) {
      if (!skinr_set($skinr)) {
        drupal_set_message(t('Skinr settings for the theme %theme could not be saved!', array('%theme' => $form_state['values']['theme'])), 'error');
      }
    }
  }

  drupal_set_message(t('Skinr settings for the theme %theme have been saved.', array('%theme' => $form_state['values']['theme'])));
  drupal_goto('admin/build/skinr');
}

/**
 * Menu callback; displays the delete confirmation for a skinr page rule.
 */
function skinr_ui_delete_confirm(&$form_state, $theme, $module, $sid) {
  $form['theme'] = array(
    '#type' => 'value',
    '#value' => isset($form_state['theme']) ? $form_state['theme'] : $theme,
  );
  $form['module'] = array(
    '#type' => 'value',
    '#value' => isset($form_state['module']) ? $form_state['module'] : $module,
  );
  $form['sid'] = array(
    '#type' => 'value',
    '#value' => isset($form_state['sid']) ? $form_state['sid'] : $sid,
  );

  $themes = list_themes();

  return confirm_form($form,
    t('Are you sure you want to delete these Skinr settings?'),
    isset($_GET['destination']) ? $_GET['destination'] : 'admin/build/skinr',
    t('This action cannot be undone.<br /> Theme: !theme<br />Module: !module<br />Skinr ID: !sid', array('!theme' => $themes[$theme]->info['name'], '!module' => $module, '!sid' => $sid)),
    t('Delete'),
    t('Cancel')
  );
}

function skinr_ui_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $skinr = new stdClass();
    $skinr->theme = $form_state['values']['theme'];
    $skinr->module = $form_state['values']['module'];
    $skinr->sid = $form_state['values']['sid'];
    $skinr->skins = array();
    $skinr->settings = array();

    skinr_set($skinr);
  }

  $form_state['redirect'] = 'admin/build/skinr';
}

/**
 * Theme function for Skinr UI's list form.
 *
 * @param $form
 *   An associative array containing the structure of the form.
 * @ingroup themeable
 */
function theme_skinr_ui_admin_skins($form) {
  $has_skins = isset($form['rows']['#value']) && is_array($form['rows']['#value']);
  $select_header = $has_skins ? theme('table_select_header_cell') : '';
  $headers = array(
    $select_header,
    array('data' => t('Theme'), 'field' => 'theme'),
    array('data' => t('Type'), 'field' => 'type'),
    array('data' => t('Skinr ID'), 'field' => 'sid'),
    t('Operations'),
  );

  $output .= drupal_render($form['options']);
  if ($has_skins) {
    $ts = tablesort_init($headers);
    $sort = SORT_DESC;
    if ($ts['sort'] == 'asc') {
      $sort = SORT_ASC;
    }

    switch ($ts['sql']) {
      case 'type':
        $sortby = 2;
        break;

      case 'sid':
        $sortby = 3;
        break;

      case 'theme':
      default:
        $sortby = 1;
        break;
    }

    $rows = array();
    $sorts = array();
    foreach ($form['rows']['#value'] as $key => $value) {
      $row = array();
      $row[] = drupal_render($form['skinr_settings'][$key]);
      $row[] = $value['theme'];
      $row[] = $value['type'];
      $row[] = $value['sid'];
      $row[] = $value['operations'];
      $rows[$key] = $row;
      $sorts[] = strip_tags($rows[$key][$sortby]);
    }

    // Sort table.
    array_multisort($sorts, $sort, $rows);
  }
  else {
    $rows[] = array(array('data' => t('No skinr settings available.'), 'colspan' => '5'));
  }

  // Output table.
  $output .= theme('table', $headers, $rows);

  $output .= drupal_render($form);

  return $output;
}

/**
 * Theme function for Skinr UI's skinsets form.
 *
 * @param $form
 *   An associative array containing the structure of the form.
 * @ingroup themeable
 */
function theme_skinr_ui_skinsets_form($form) {
  // Individual table headers.
  $header = array(t('Screenshot'), t('Name'), t('Version'), t('Enabled'), t('Operations'));

  // Pull package information from skinsets list and start grouping skinsets.
  $skinsets = $form['skinsets']['#value'];
  $packages = array();
  foreach ($skinsets as $skinset) {
    if (!isset($skinset->info['package']) || !$skinset->info['package']) {
      $skinset->info['package'] = t('Other');
    }
    $packages[$skinset->info['package']][$skinset->name] = $skinset->info;
  }
  ksort($packages);

  // Display packages.
  $output = '';
  foreach ($packages as $package => $skinsets) {
    $rows = array();
    foreach ($skinsets as $key => $skinset) {
      // Only look for skinsets.
      if (!isset($form[$key]['info'])) {
        continue;
      }

      // Fetch info.
      $info = $form[$key]['info']['#value'];
      // Localize skinset description.
      $description = t($info['description']);

      // Make sure it is compatible and render the checkbox if so.
      if (isset($form['status']['#incompatible_skinsets_core'][$key])) {
        unset($form['status'][$key]);
        $status = theme('image', 'misc/watchdog-error.png', t('incompatible'), t('Incompatible with this version of Drupal core'));
        $description .= '<div class="incompatible">'. t('This version is incompatible with the !core_version version of Drupal core.', array('!core_version' => VERSION)) .'</div>';
      }
      elseif (isset($form['status']['#incompatible_skinsets_php'][$key])) {
        unset($form['status'][$key]);
        $status = theme('image', 'misc/watchdog-error.png', t('incompatible'), t('Incompatible with this version of PHP'));
        $php_required = $form['status']['#incompatible_themes_php'][$key];
        if (substr_count($php_required, '.') < 2) {
          $php_required .= '.*';
        }
        $description .= '<div class="incompatible">'. t('This skinset requires PHP version @php_required and is incompatible with PHP version !php_version.', array('@php_required' => $php_required, '!php_version' => phpversion())) .'</div>';
      }
      else {
        $status = drupal_render($form['status'][$key]);
      }

      // Style theme info
      $content = '<div class="skinset-info"><h2>'. $info['name'] .'</h2><div class="description">'. $description .'</div></div>';

      // Build rows
      $row = array();
      $row[] = drupal_render($form[$key]['screenshot']);
      $row[] = $content;
      $row[] = isset($info['version']) ? $info['version'] : '';
      $row[] = array('data' => $status, 'align' => 'center');
      $row[] = array('data' => drupal_render($form[$key]['operations']), 'align' => 'center');
      $rows[] = $row;
    }

    $fieldset = array(
      '#title' => t($package),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#value' => theme('table', $header, $rows, array('class' => 'package')),
    );
    $output .= theme('fieldset', $fieldset);
  }

  $output .= drupal_render($form);
  return $output;
}

/**
 * Theme function for Skinr UI's skinsets form.
 *
 * @param $form
 *   An associative array containing the structure of the form.
 * @ingroup themeable
 */
function theme_skinr_ui_skinsets_settings_form($form) {
  // Individual table headers.
  $header = array(t('Enabled'), t('Name'), t('Operations'));

  // Display skins.
  $output = '';

  // Get current theme, but make sure it's not the admin theme when we're editing with AJAX.
  $current_theme = skinr_current_theme(TRUE);

  $themes = list_themes();
  ksort($themes);

  foreach ($themes as $theme) {
    if (!$theme->status) {
      continue;
    }

    $rows = array();
    foreach ($form as $key => $skin) {
      // Only look for skins.
      if (!is_array($form[$key]) || !isset($form[$key]['info'])) {
        continue;
      }

      // Fetch info.
      $info = $form[$key]['info']['#value'];
      // Localize skinset title and description.
      $title = t($info['title']);
      $description = t($info['description']);

      // The hooks this skin is working with.
      $features = array();
      foreach ($info['features'] as $feature) {
        $features[] = $feature == '*' ? t('all hooks') : $feature;
      }
      $features = t('Used by: !features', array('!features' => implode(', ', $features)));

      $status = drupal_render($form[$key]['status'][$theme->name]);

      // Style theme info
      $content = '<div class="skin-info"><h2>'. $title .'</h2><div class="description">'. $description .'</div><div class="features">'. $features .'</div></div>';

      // Build rows
      $row = array();
      $row[] = array('data' => $status, 'align' => 'center');
      $row[] = $content;
      $row[] = array('data' => drupal_render($form[$key]['operations']), 'align' => 'center');
      $rows[] = $row;
    }

    $fieldset = array(
      '#title' => t($theme->info['name']),
      '#collapsible' => TRUE,
      '#collapsed' => $theme->name == $current_theme ? FALSE : TRUE,
      '#value' => theme('table', $header, $rows, array('class' => 'theme')),
    );
    $output .= theme('fieldset', $fieldset);
  }

  $output .= drupal_render($form);
  return $output;
}

function skinr_ui_settings_multiple_delete_confirm(&$form_state, $skinr_settings) {
  $themes = list_themes();

  $form['skinr_settings'] = array(
    '#prefix' => '<ul>',
    '#suffix' => '</ul>',
    '#tree' => TRUE,
  );
  // array_filter returns only elements with TRUE values
  foreach ($skinr_settings as $skinr_setting => $value) {
    $parts = explode('__', $skinr_setting, 3);
    $form['skinr_settings'][$skinr_setting] = array(
      '#type' => 'hidden',
      '#value' => $skinr_setting,
      '#prefix' => '<li>',
      '#suffix' => t('Skinr ID %sid of type %type for theme %theme', array('%sid' => $parts[2], '%type' => $parts[1], '%theme' => $themes[$parts[0]]->info['name'])) ."</li>\n",
    );
  }
  $form['operation'] = array('#type' => 'hidden', '#value' => 'delete');
  $form['#submit'][] = 'skinr_ui_settings_multiple_delete_confirm_submit';
  return confirm_form($form,
                      t('Are you sure you want to delete these items?'),
                      'admin/build/skinr', t('This action cannot be undone.'),
                      t('Delete all'), t('Cancel'));
}

function skinr_ui_settings_multiple_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    foreach ($form_state['values']['skinr_settings'] as $skinr_setting => $value) {
      $parts = explode('__', $skinr_setting, 3);

      $skinr = new StdClass();
      $skinr->theme = $parts[0];
      $skinr->module = $parts[1];
      $skinr->sid = $parts[2];
      $skinr->skins = array();

      skinr_set($skinr);
    }
    drupal_set_message(t('The items have been deleted.'));
  }
  $form_state['redirect'] = 'admin/build/skinr';
  return;
}
