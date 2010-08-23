// $Id: fusion-acquia-slate-script.js,v 1.2 2010/07/02 23:14:21 eternalistic Exp $

Drupal.behaviors.acquia_slateFirstWord = function (context) {
  $('#site-name a').each(function(){
      var bt = $(this);
      bt.html( bt.text().replace(/(^\w+)/,'<span class="first-word">$1</span>') );
  });
  $('.title-dual h2.block-title').each(function(){
      var bt = $(this);
      bt.html( bt.text().replace(/(^\w+)/,'<span class="first-word">$1</span>') );
  });
  $('.title-white-bold-first h2.block-title').each(function(){
      var bt = $(this);
      bt.html( bt.text().replace(/(^\w+)/,'<span class="first-word">$1</span>') );
  });
};

Drupal.behaviors.acquia_slateRoundedCorner = function (context) {
  // Header site info - Without preface top
  $("#header-site-info.without-preface-top #header-site-info-inner").corner("tl tr bl br 10px");

  // Header site info - With preface top
  $("#header-site-info.with-preface-top #header-site-info-inner").corner("tl tr 10px");

  // Preface top
  $(".banner-background #preface-top .preface-top-inner").corner("bl br 10px");

  // Links 
  $("div.links").corner("7px"); 

  //Terms
  $("div.terms").corner("7px"); 

  // Rounded corners with gray background
  $(".rounded-corners-gray-background .inner").corner("10px");
};

Drupal.behaviors.acquia_slateIE6fixes = function (context) {
  // IE6 & less-specific functions
  // Add hover class to primary menu li elements on hover
  if ($.browser.msie && ($.browser.version < 7)) {
    $('span.button span input').hover(function() {
      $(this).addClass('hover');
      }, function() {
        $(this).removeClass('hover');
    }); 
    $('span.button-wrapper').hover(function() {
      $(this).addClass('hover');
      }, function() {
        $(this).removeClass('hover');
    });
    $('#primary-menu ul.menu li').hover(function() {
      $(this).addClass('hover');
      }, function() {
        $(this).removeClass('hover');
    });
    $('#secondary-menu ul.links li').hover(function() {
      $(this).addClass('hover');
      }, function() {
        $(this).removeClass('hover');
    });
  };
};