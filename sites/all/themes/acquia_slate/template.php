<?php
// $Id: template.php,v 1.4 2010/07/02 23:14:21 eternalistic Exp $

// Override theme_button for expanding graphic buttons
function acquia_slate_button($element) {
  // Make sure not to overwrite classes.
  if (isset($element['#attributes']['class'])) {
    $element['#attributes']['class'] = 'form-'. $element['#button_type'] .' '. $element['#attributes']['class'];
  }
  else {
    $element['#attributes']['class'] = 'form-'. $element['#button_type'];
  }
  
  // Wrap visible inputs with span tags for button graphics
  if (stristr($element['#attributes']['style'], 'display: none;') || stristr($element['#attributes']['class'], 'fivestar-submit') || (is_array($element["#upload_validators"]))) {
    return '<input type="submit" '. (empty($element['#name']) ? '' : 'name="'. $element['#name'] .'" ')  .'id="'. $element['#id'].'" value="'. check_plain($element['#value']) .'" '. drupal_attributes($element['#attributes']) ." />\n";
  }
  else {
    return '<span class="button-wrapper"><span class="button"><span><input type="submit" '. (empty($element['#name']) ? '' : 'name="'. $element['#name'] .'" ')  .'id="'. $element['#id'].'" value="'. check_plain($element['#value']) .'" '. drupal_attributes($element['#attributes']) ." /></span></span></span>\n";
  }
}