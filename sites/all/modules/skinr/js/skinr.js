// $Id: skinr.js,v 1.1.2.22 2010/05/01 20:11:11 jgirlygirl Exp $

(function ($) {

// Make sure our objects are defined.
Drupal.CTools = Drupal.CTools || {};
Drupal.Skinr = Drupal.Skinr || {
  editUrl: 'admin/build/skinr/edit/nojs',
  infoUrl: 'admin/build/skinr/info',
  unloadBehaviors: {},

  loadedFiles: { 'css': [], 'js': [] },
  loadedFilesSnapshot: { 'css': [], 'js': [] },

  preparedUnload: {},
  finalizedUnload: {},

  submitted: false
};

/**
 * Register functions to run when a skin is unloaded. This usually happens
 * when previewing a skin.
 */
Drupal.Skinr.registerUnload = function($script_id, $skin, $options, $function) {
  if (!$.isArray($options)) {
    $options = [$options];
  }

  if (Drupal.Skinr.unloadBehaviors[$skin] == undefined) {
    Drupal.Skinr.unloadBehaviors[$skin] = {};
  }

  for (var $i in $options) {
    if (Drupal.Skinr.unloadBehaviors[$skin][$options[$i]] == undefined) {
      Drupal.Skinr.unloadBehaviors[$skin][$options[$i]] = {};
    }
    Drupal.Skinr.unloadBehaviors[$skin][$options[$i]][$script_id] = $function;
  }
}

/**
 * Register a skin option to be unloaded.
 */
Drupal.Skinr.prepareUnload = function($skin, $options) {
  if (!$.isArray($options)) {
    $options = [$options];
  }

  if (Drupal.Skinr.preparedUnload[$skin] == undefined) {
    Drupal.Skinr.preparedUnload[$skin] = [];
  }

  for (var $i in $options) {
    if ($.inArray($options[$i], Drupal.Skinr.preparedUnload[$skin]) < 0) {
      Drupal.Skinr.preparedUnload[$skin][Drupal.Skinr.preparedUnload[$skin].length] = $options[$i];
    }
  }
}

/**
 * Register a skin option to be unloaded when resetting the dialog.
 */
Drupal.Skinr.finalUnload = function($skin, $options) {
  if (!$.isArray($options)) {
    $options = [$options];
  }

  if (Drupal.Skinr.finalizedUnload[$skin] == undefined) {
    Drupal.Skinr.finalizedUnload[$skin] = [];
  }

  for (var $i in $options) {
    if ($.inArray($options[$i], Drupal.Skinr.finalizedUnload[$skin]) < 0) {
      Drupal.Skinr.finalizedUnload[$skin][Drupal.Skinr.finalizedUnload[$skin].length] = $options[$i];
    }
  }
}

/**
 * Go through each skin option that was prepared for unload and run the
 * associated regaistered function.
 */
Drupal.Skinr.unload = function() {
  for (var $skin_name in Drupal.Skinr.preparedUnload) {
    if (Drupal.Skinr.preparedUnload[$skin_name] != undefined) {
      $skin = Drupal.Skinr.preparedUnload[$skin_name];
      for (var $j in $skin) {
        $option = $skin[$j];
        if (Drupal.Skinr.unloadBehaviors[$skin_name] != undefined) {
          if (Drupal.Skinr.unloadBehaviors[$skin_name][$option] != undefined) {
            $functions = Drupal.Skinr.unloadBehaviors[$skin_name][$option];
            for (var $k in $functions) {
              if ($.isFunction(Drupal.Skinr.unloadBehaviors[$skin_name][$option][$k])) {
                Drupal.Skinr.unloadBehaviors[$skin_name][$option][$k]();
              }
            }
          }
        }
      }
    }
  }

  Drupal.Skinr.preparedUnload = {};
}

/**
 * Initialize skinr's edit cogs.
 */
Drupal.behaviors.Skinr = function(context) {
  // Take a snapshot of the current list of files.
  if (this.initialized == undefined) {
    this.initialized = true;
    Drupal.Skinr.registerFile(null, null, null);
    Drupal.Skinr.freezeFiles();
  }

  for (var i in Drupal.settings.skinr['areas']) {
    var $module = Drupal.settings.skinr['areas'][i]['module'];
    var $sids = Drupal.settings.skinr['areas'][i]['sids'];
    var $id = Drupal.settings.skinr['areas'][i]['id'];

    var $region = $('.skinr-id-' + $id + ':not(.skinr-region-processed)', context).addClass('skinr-region-processed');
    if (Drupal.settings.skinr['areas'][i]['classes'] == undefined) {
      Drupal.settings.skinr['areas'][i]['classes'] = $($region).attr('class');
    }

    if ($region.length > 0) {
      var $links = '';
      for (var $j in $sids) {
        var $classes = '';
        if ($j == 0) {
          $classes += ' first';
        }
        if ($j == $sids.length - 1) {
          $classes += ' last';
        }
        if ($sids.length > 1) {
          $links += '<li class="skinr-link-' + $j + $classes + '"><a href="' + Drupal.settings.basePath + Drupal.Skinr.editUrl + '/' + $module + '/' + $sids[$j] + '/' + $sids +'" class="skinr-link ctools-use-dialog">' + Drupal.t('Edit skin') + ' ' + (parseInt($j) + 1) + '</a></li>';
        }
        else {
          $links = '<li class="skinr-link-0 first last"><a href="' + Drupal.settings.basePath + Drupal.Skinr.editUrl + '/' + $module + '/' + $sids[$j] +'" class="skinr-link ctools-use-dialog">' + Drupal.t('Edit skin') + '</a></li>';
        }
      }

      var $wrapper_classes = '';
      if ($module == 'page') {
        $wrapper_classes += ' skinr-links-wrapper-page';
      }

      $region.prepend('<div class="skinr-links-wrapper' + $wrapper_classes + '"><ul class="skinr-links">' + $links + '</ul></div>');
      $region.get(0).skinr = { 'module': $module, 'sids': $sids, 'id': $id };

      Drupal.behaviors.Dialog($region);
    };
  }

  $('div.skinr-links-wrapper:not(.skinr-links-wrapper-processed)', context).addClass('skinr-links-wrapper-processed').each(function () {
    var $wrapper = $(this);
    var $region = $wrapper.closest('.skinr-region');
    var $links = $wrapper.find('ul.skinr-links');
    var $trigger = $('<a class="skinr-links-trigger" href="#" />').text(Drupal.t('Configure')).click(
      function () {
        $wrapper.find('ul.skinr-links').stop(true, true).slideToggle(100);
        $wrapper.toggleClass('skinr-links-active');
        return false;
      }
    );

    // Attach hover behavior to trigger and ul.skinr-links.
    $trigger.add($links).hover(
      function () { $region.addClass('skinr-region-active'); },
      function () { $region.removeClass('skinr-region-active'); }
    );
    // Hide the contextual links when user rolls out of the .skinr-links-region.
    $region.bind('mouseleave', Drupal.Skinr.hideLinks).click(Drupal.Skinr.hideLinks);
    // Prepend the trigger.
    $links.end().prepend($trigger);
  });

  // Add an open and a close handler to the dialog.
  if (Drupal.Dialog.dialog && !Drupal.Dialog.dialog.hasClass('skinr-dialog-processed')) {
    Drupal.Dialog.dialog.addClass('skinr-dialog-processed').bind('dialogbeforeclose', Drupal.Skinr.dialogBeforeClose);
  }
}

/**
 * Disables outline for the region contextual links are associated with.
 */
Drupal.Skinr.hideLinks = function () {
  $(this).closest('.skinr-region')
    .find('.skinr-links-active').removeClass('skinr-links-active')
    .find('ul.skinr-links').hide();
};

/**
 * Cleanup function run before dialog is closed.
 */
Drupal.Skinr.dialogBeforeClose = function(event, ui) {
  if (!Drupal.Skinr.submitted) {
    // Unload js.
    Drupal.Skinr.preparedUnload = Drupal.Skinr.finalizedUnload;
    Drupal.Skinr.unload();
    Drupal.Skinr.finalizedUnload = {};

    Drupal.Skinr.unfreezeFiles();

    // Reset all the applied style changes.
    for (var i in Drupal.settings.skinr['areas']) {
      var $id = Drupal.settings.skinr['areas'][i]['id'];
      var $classes = Drupal.settings.skinr['areas'][i]['classes'];
      $('.skinr-id-' + $id).attr('class', $classes);
    }
    // @todo Remove added CSS during preview.

    // Let's do another attach behaviors now that our classes are reset.
    Drupal.attachBehaviors();
  }
  Drupal.Skinr.submitted = false;
}

/**
 * Live preview functionality for skinr.
 */
Drupal.behaviors.SkinrLivePreview = function(context) {
  $('#skinr-ui-form .skinr-ui-current-theme :input:not(.skinr-live-preview-processed)', context).addClass('skinr-live-preview-processed').change(function () {
    var $tag = $(this).attr('tagName');
    $tag = $tag.toLowerCase();

    var $module = $('#skinr-ui-form #edit-module').val();
    var $sid = $('#skinr-ui-form #edit-sid').val();
    var $sids = $('#skinr-ui-form #edit-sids').val();
    if (!$sids) {
      $sids = $sid;
    }

    var $name = $(this).attr('name');
    $name = $name.replace(/skinr_settings\[.*_group\]\[[^\]]*\]\[widgets\](\[[^\]]*\])?\[([^\]]*)\]/, '$2');

    var $rem_classes = '';
    var $add_classes = $(this).val();

    if ($tag == 'select') {
      $(this).find('option').each(function() {
        $rem_classes += ' ' + $(this).attr('value');
      });
    }
    else if ($tag == 'input') {
      var $type = $(this).attr('type');
      if ($type == 'checkbox') {
        $add_classes = '';
        $(this).closest('.form-checkboxes').find('input[type=checkbox]').each(function () {
          if ($(this).is(':checked')) {
            $add_classes += ' ' + $(this).attr('value');
          }
          else {
            $rem_classes += ' ' + $(this).attr('value');
          }
        });
      }
      else if ($type == 'radio') {
        $(this).closest('.form-radios').find('input[type=radio]').each(function () {
          $rem_classes += ' ' + $(this).attr('value');
        });
      }
    }

    // Use AJAX to grab the CSS and JS filename.
    $.ajax({
      type: 'GET',
      dataType: 'json',
      url: Drupal.settings.basePath + Drupal.Skinr.infoUrl + '/' + $name + '/' + $add_classes,
      success: function($data) {

        var $command = {
          command: 'skinrAfterupdate',
          module: $module,
          sids: $sids,
          classes: {
            remove: $rem_classes,
            add: $add_classes
          },
          css: $data.css,
          js: $data.js,
          nosave: true
        };

        Drupal.CTools.AJAX.commands.skinrAfterupdate($command);
      }
    });
  });
}

/**
 * AJAX responder command to dismiss the modal.
 */
Drupal.CTools.AJAX.commands.skinrAfterupdate = function(command) {
  var $path, $el;
  Drupal.Skinr.finalizedUnload = {};

  if (command.nosave == undefined || command.nosave == false) {
    // Let the dialogBeforeClose function know we've submitted rather than
    Drupal.Skinr.submitted = true;
  }

  if (command.module && command.sids && (command.classes.remove || command.classes.add)) {
    if (command.css) {
      // First unload all unecessary stylesheets
      for (var j in command.css) {
        $path = Drupal.settings.basePath + command.css[j].path + '?' + Drupal.settings.skinr['css_js_query_string'];
        if (!command.css[j].enabled) {
          Drupal.Skinr.unloadFile('css', $path);
        }
      }
      // Then load the newly enabled ones
      for (var j in command.css) {
        $path = Drupal.settings.basePath + command.css[j].path + '?' + Drupal.settings.skinr['css_js_query_string'];
        if (command.css[j].enabled) {
          Drupal.Skinr.loadFile('css', $path, command.css[j].media);
        }
      }
    }
    if (command.js) {
      for (var j in command.js) {
        $path = Drupal.settings.basePath + command.js[j].path + '?' + Drupal.settings.skinr['css_js_query_string'];
        if (command.js[j].enabled) {
          Drupal.Skinr.loadFile('js', $path, null);
          if (command.nosave) {
            // Need to disable this js after closing dialog.
            Drupal.Skinr.finalUnload(command.js[j].skin, command.js[j].options);
          }
        }
        else {
          // Disable js.
          Drupal.Skinr.prepareUnload(command.js[j].skin, command.js[j].options);
        }
      }
    }

    // Unload.
    Drupal.Skinr.unload();

    for (var i in Drupal.settings.skinr['areas']) {
      if (Drupal.settings.skinr['areas'][i]['module'] == command.module && Drupal.settings.skinr['areas'][i]['sids'] == command.sids) {
        $('.skinr-id-' + Drupal.settings.skinr['areas'][i]['id']).removeClass(command.classes.remove).addClass(command.classes.add);
        if (command.nosave == undefined || command.nosave == false) {
          Drupal.settings.skinr['areas'][i]['classes'] = $('.skinr-id-' + Drupal.settings.skinr['areas'][i]['id']).attr('class');
        }
      }
    }
  }

  if (command.nosave == undefined || command.nosave == false) {
    // Update our snapshot of loaded files.
    Drupal.Skinr.freezeFiles();
  }
}

/**
 * Helper functions to keep track of loaded css and js files, and to
 * dynamically load and unload them.
 */

Drupal.Skinr.freezeFiles = function () {
  Drupal.Skinr.loadedFilesSnapshot = jQuery.extend(true, {}, Drupal.Skinr.loadedFiles);
}

Drupal.Skinr.unfreezeFiles = function () {
  var $diff = Drupal.Skinr.diffFiles(Drupal.Skinr.loadedFiles['css'], Drupal.Skinr.loadedFilesSnapshot['css']);
  for ($i in $diff) {
    Drupal.Skinr.unloadFile('css', $diff[$i].file, $diff[$i].media);
  }

  var $diff = Drupal.Skinr.diffFiles(Drupal.Skinr.loadedFilesSnapshot['css'], Drupal.Skinr.loadedFiles['css']);
  for ($i in $diff) {
    Drupal.Skinr.loadFile('css', $diff[$i].file, $diff[$i].media);
  }
}

Drupal.Skinr.diffFiles = function($a, $b) {
  var $diff = $.grep($a, function($el, $key) {
    for (var $i in $b) {
      var $obj = $b[$i];
      if ($el.file == $obj.file && $el.media == $obj.media) {
        return false;
      }
    }
    return true;
  });
  return $diff;
}

Drupal.Skinr.registerFile = function($type, $file, $media) {
  if (this.initialized == undefined) {
    this.initialized = true;
    $el = $('link[rel*=style]').each(function() {
      Drupal.Skinr.registerFile('css', $(this).attr('href'), $(this).attr('media'));
    });
    $el = $('script[type*=javascript][src]').each(function() {
      Drupal.Skinr.registerFile('js', $(this).attr('src'));
    });
  }

  if (!$file) {
    return false;
  }

  if ($.inArray({ file: $file, media: $media }, Drupal.Skinr.loadedFiles[$type]) < 0) {
    Drupal.Skinr.loadedFiles[$type][Drupal.Skinr.loadedFiles[$type].length] = { file: $file, media: $media };
    return true;
  }
  return false;
}

Drupal.Skinr.unregisterFile = function($type, $file) {
  for (var $key in Drupal.Skinr.loadedFiles[$type]) {
    if (Drupal.Skinr.loadedFiles[$type][$key].file == $file) {
      Drupal.Skinr.loadedFiles[$type].splice($key, 1);
      return true;
    }
  }
  return false;
}

Drupal.Skinr.loadFile = function($type, $file, $media) {
  if (Drupal.Skinr.registerFile($type, $file, $media)) {
    if ($type == 'css') {
      $el = $('link[rel*=style][href=' + $file + ']');
      if ($el.get() == '') {
        $('<link />').attr({
          href: $file,
          media: $media,
          rel: 'stylesheet',
          type: 'text/css'
        }).appendTo('head');
      }
    }
    else if ($type == 'js') {
      try {
        $.getScript($file, function($data) {
          // This might be a bit excessive to run on every script load,
          // but the scripts load asynchronously.
          Drupal.attachBehaviors();
        });
      }
      catch (err) {
        Drupal.Skinr.unregisterFile($type, $file);
      }
    }
    return true;
  }
  return false;
}

Drupal.Skinr.unloadFile = function($type, $file) {
  if (Drupal.Skinr.unregisterFile($type, $file)) {
    if ($type == 'css') {
      $el = $('link[rel*=style][href=' + $file + ']');
      if ($el.get() != '') {
        $el.disabled = true;
        $el.remove();
      }
      return true;
    }
    // Can't unload js files!
  }
  return false;
}

})(jQuery);
