<?php
// $Id: user.tpl.php 65 2010-11-10 09:20:40Z yd2004 $
// print_r($account);
?>

<h2><?php echo $account->name; ?></h2>

<?php if ($account->content) : ?>
  <?php echo $account->content; ?>
<?php endif;?>