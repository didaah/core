<?php
/**
 * 默认区块模板 
 * $Id: block.tpl.php 27 2010-06-23 06:39:46Z yd2004 $
 */
?>

<div class="block block_module_<?php echo $block->module ?>" id="block_<?php echo $block->module ?>_<?php echo $block->delta ?>">
  <?php if ($block->title) :?>
    <h3 class="block_title"><?php echo $block->title?></h3>
  <?php endif?>
  <div class="block_content">
    <?php echo $block->content?>
  </div>
</div>