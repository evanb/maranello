<?php
// $Id: skinr_ui.rules.inc,v 1.1.2.4 2010/05/14 12:04:45 moonray Exp $

/**
 * @file
 * Admin page callbacks for the skinr module.
 */

/**
 * Menu callback; displays the skinr rules listing.
 */
function skinr_rules() {
  $output = '';

  $headers = array(t('Title'), t('Skinr ID'), array('data' => t('Operations'), 'colspan' => 2));

  $rules = skinr_rule_load();
  $rows = array();
  foreach ($rules as $rule) {
    $row = array(
      check_plain($rule->title),
      check_plain($rule->rid),
      l(t('edit'), 'admin/build/skinr/rules/edit/'. $rule->rid),
      l(t('delete'), 'admin/build/skinr/rules/delete/'. $rule->rid),
    );
    $rows[] = $row;
  }

  $link = l(t('Create a new rule'), 'admin/build/skinr/rules/add');
  $row = array();
  if (empty($rows)) {
    $row[] = array(
      'data' => t('No rules have been set up yet. !url.', array('!url' => $link)),
      'colspan' => 3,
    );
  }
  else {
    $row[] = array(
      'data' => t('!url.', array('!url' => $link)),
      'colspan' => 3,
    );
  }
  $rows[] = $row;

  $output .= theme('table', $headers, $rows);
  $output .= drupal_render($form);

  return $output;
}

/**
 * Menu callback; displays the edit form for a skinr rule.
 */
function skinr_rule_edit(&$form_state, $rid = NULL) {
  $form = array(
    '#attributes' => array('class' => 'skinr-form'),
  );

  $form['rule'] = array(
    '#type' => 'fieldset',
    '#title' => t('Skinr rule visibility'),
    '#collapsible' => TRUE,
    '#weight' => -1,
  );

  if (isset($form_state['values'])) {
    $rule = array(
      'title' => $form_state['values']['title'],
      'roles' => $form_state['values']['roles'],
      'visibility' => $form_state['values']['visibility'],
      'pages' => $form_state['values']['pages'],
    );
  }
  elseif(!is_null($rid) && $rule = skinr_rule_load($rid)) {
    $rule = (array) $rule;
    $form['rule']['rid'] = array(
      '#type' => 'hidden',
      '#value' => $rid,
    );
  }
  else {
    $rule = array(
      'title' => '',
      'roles' => array(),
      'visibility' => 0,
      'pages' => '',
    );
  }

  $form['rule']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $rule['title'],
    '#description' => t('Descriptive title for this rule; used by administrators.'),
    '#required' => TRUE,
  );

  // Inject the page level form elements here.
  $form['rule']['roles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Show for specific roles'),
    '#default_value' => $rule['roles'],
    '#options' => user_roles(),
    '#description' => t('Show only for the selected role(s). If you select no roles, it will be visible to all users.'),
  );

  // Set up options and description.
  $options = array(0 => t('Show on every page except the listed pages.'), 1 => t('Show on only the listed pages.'));
  $description = t("Enter one page per line as Drupal paths. The '*' character is a wildcard. Example paths are %blog for the blog page and %blog-wildcard for every personal blog. %front is the front page.", array('%blog' => 'blog', '%blog-wildcard' => 'blog/*', '%front' => '<front>'));

  // Add the PHP specific stuff, if user has access.
  if (user_access('use PHP for visibility')) {
    $options[2] = t('Show if the following PHP code returns <code>TRUE</code> (PHP-mode, experts only).');
    $description .= ' '. t('If the PHP-mode is chosen, enter PHP code between %php. Note that executing incorrect PHP-code can break your Drupal site.', array('%php' => '<?php ?>'));
  }
  $form['rule']['visibility'] = array(
    '#type' => 'radios',
    '#title' => t('Show on specific pages'),
    '#options' => $options,
    '#default_value' => $rule['visibility'],
    '#required' => TRUE,
  );
  $form['rule']['pages'] = array(
    '#type' => 'textarea',
    '#title' => t('Pages'),
    '#default_value' => $rule['pages'],
    '#description' => $description,
  );

  $form['buttons']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  if (isset($rule['rid'])) {
    $form['buttons']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('skinr_rule_delete_submit'),
    );
  }

  return $form;
}

function skinr_rule_edit_submit($form, &$form_state) {
  $rule = new stdClass();
  $rule->rid = !empty($form_state['values']['rid']) ? $form_state['values']['rid'] : NULL;
  $rule->title = $form_state['values']['title'];
  $rule->roles = $form_state['values']['roles'];
  $rule->visibility = $form_state['values']['visibility'];
  $rule->pages = $form_state['values']['pages'];

  skinr_rule_save($rule);

  $form_state['redirect'] = 'admin/build/skinr/rules';
}

function skinr_rule_delete_submit($form, &$form_state) {
  $destination = '';
  if (isset($_REQUEST['destination'])) {
    $destination = drupal_get_destination();
    unset($_REQUEST['destination']);
  }
  $form_state['redirect'] = array('admin/build/skinr/rules/delete/'. $form_state['values']['rid'], $destination);
}

/**
 * Menu callback; displays the delete confirmation for a skinr page rule.
 */
function skinr_rule_delete_confirm(&$form_state, $rid) {
  $form['rid'] = array(
    '#type' => 'value',
    '#value' => $rid,
  );

  $rule = skinr_rule_load($rid);
  return confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $rule->title)),
    isset($_GET['destination']) ? $_GET['destination'] : 'admin/build/skinr/rules',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

function skinr_rule_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    skinr_rule_delete($form_state['values']['rid']);
  }

  $form_state['redirect'] = 'admin/build/skinr/rules';
}
