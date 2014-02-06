// $Id$
jQuery(function($) {
  $('ul.site_menu > li > ul').each(function() {
    $(this).parent().addClass('dropdown');
    $(this).prev().addClass('dropdown-toggle').append('<b class="caret"></b>').attr('data-toggle', 'dropdown');
  });
  $('ul.site_menu_children > li > ul').each(function() {
    $(this).parent().addClass('dropdown-submenu');
  });
  $('ul.tabs').addClass('nav nav-tabs');
  $('ul.site_menu a.active').parent('li').addClass('active');
  $('ul.nav-tabs a.active, ul.nav-pills a.active').parent('li').addClass('active');
  $('ul.site_menu_children a.active').parents('.site_menu_list').addClass('active');
  $('.dropdown .item_dd_item_lang_change').addClass('dropdown-menu');

  $('#block_system_adminLink .item_admin_menus > h3').prepend('<i class="icon-plus"></i>');
});
