<!DOCTYPE html>
<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <title><?php echo $head_title; ?></title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <?php echo $heads; ?>
    <?php echo $styles; ?>
    <?php echo $scripts; ?>
  </head>
  <body class="<?php echo $user_login_status;?> <?php echo $args_id;?>">
    <div id="header" class="navbar navbar-inverse navbar-fixed-top">
      <div class="navbar-inner">
        <div class="container">
          <a href="<?php echo $base_path?>" title="<?php echo $site_global->name; ?>" class="logo brand">
            <img src="<?php echo $site_global->logo; ?>" alt="<?php echo $site_global->slogan;?>" title="<?php echo $site_global->name; ?>" />
          </a>
          <?php echo $menu; ?>
          <ul class="nav pull-right">
            <li class="dropdown">
            <a data-toggle="dropdown" class="dropdown-toggle" role="button" href="#" title="<?php echo t('bootheme', '选择语言'); ?>">
<?php 
global  $language_code, $conf;
echo $conf['languages'][$language_code];
?>
                <b class="caret"></b>
              </a>
              <?php echo $lang_links;?>
            </li>
          </ul>
        </div>
      </div>
    </div>

    <div id="main" class="container"> 
      <div class="row">
        <?php if ($left) : ?>
        <div id="sidebar-left" class="sidebar span5">
          <?php echo $left; ?>
        </div>
        <?php endif; ?>

        <?php
          if ($left && $right) {
            $content_class = 'span14';
          } else if ($left) {
            $content_class = 'span19 last';
          } else if ($right) {
            $content_class = 'span19';
          } else {
            $content_class = '';
          }
        ?>

        <div id="content" class="<?php echo $content_class;?>">
          <?php echo $breadcrumb; ?>
          <?php echo $tabs; ?>
          <?php echo $sub_tabs; ?>
          <?php echo $messages; ?>
          <?php echo $help; ?>
          <?php echo $content; ?>
        </div>

        <?php if ($right) : ?>
        <div id="sidebar-right" class="sidebar span5 last">
          <?php echo $right; ?>
        </div>
        <?php endif; ?>
      </div>
    </div>

    <div id="footer">
      <div class="container">
        <?php echo $site_global->footer; ?>
        <?php echo $footer; ?>
        <?php echo $debug; ?>
      </div>
    </div>

    <?php echo $closure; ?>
  </body>
</html>
