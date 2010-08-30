<?php
// $Id: skinr.inc,v 1.1.2.1 2010/05/25 10:26:56 moonray Exp $

/**
 * @file
 * Definition of the 'skinr' panel style.
 */

// Plugin definition

/**
 * Implementation of hook_panels_style_info().
 */
function skinr_skinr_panels_styles() {
  return array(
    'skinr' => array(
      'title'              => t('Skinr'),
      'description'        => t('Allows you to apply Skinr skins.'),
      'render panel'       => 'panels_skinr_style_render_panel',
      'render pane'        => 'panels_skinr_style_render_pane',
      'settings form'      => 'panels_skinr_style_settings_form',
      'pane settings form' => 'panels_skinr_style_settings_form',
    ),
  );
}

/**
 * Render panel callback.
 *
 * @todo: Need to wait for a panels update to panels_render_panel()
 *        The panel_id is not sent through properly, yet. Once it works as
 *        promised, we can remove the workaround.
 *
 * @ingroup themeable
 */
function theme_panels_skinr_style_render_panel($display, $panel_id, $panes, $settings) {
  // Workaround for missing/invalid $panel_id.
  if (empty($panel_id)) {
    // We grab the first pane that we know exists in this panel and extract the panel_id from it.
    $panes_keys = array_keys($panes);
    $panel_id = $display->content[$panes_keys[0]]->panel;
  }

  $output = '';
  $print_separator = FALSE;
  foreach ($panes as $pane_id => $content) {
    // Add the separator if we've already displayed a pane.
    if ($print_separator) {
      $output .= '<div class="panel-separator"></div>';
    }
    $output .= $text = panels_render_pane($content, $display->content[$pane_id], $display);

    // If we displayed a pane, this will become true; if not, it will become
    // false.
    $print_separator = (bool) $text;
  }

  // Add skinr styles
  $hook = 'panels';
  if (isset($display->panel_settings[$panel_id]['style']) && isset($display->panel_settings[$panel_id]['style']) == 'skinr') {
    // Specific style set for this panel.
    $sid = 'display-'. $display->did .'-panel-'. $panel_id;
  }
  else {
    // No specific style set, so use the display's style.
    $sid = 'display-'. $display->did;
  }
  $style = array();
  if($skinr = skinr_get(NULL, $hook, $sid)) {
    $style = implode(' ', skinr_flatten_skins_array($skinr->skins));
  }

  if (!empty($style)) {
    $output = '<div class="'. $style .'">'. $output .'</div>';
  }

  return $output;
}

/**
 * Render pane callback.
 *
 * @ingroup themeable
 */
function theme_panels_skinr_style_render_pane($content, $pane, $display) {
  return theme('panels_pane', $content, $pane, $display);
}

/**
 * Settings form callback.
 */
function panels_skinr_style_settings_form($settings, $panels_display = NULL, $region = NULL, $type = NULL) {
  // Return an empty form, and let skinr handle the rest.
  $form = array();
  return $form;
}
