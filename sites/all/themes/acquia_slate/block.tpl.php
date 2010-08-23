<?php
// $Id: block.tpl.php,v 1.3 2010/07/02 23:14:21 eternalistic Exp $
?>

<div id="block-<?php print $block->module .'-'. $block->delta; ?>" class="block block-<?php print $block->module ?> <?php print $block_zebra; ?> <?php print $position; ?> <?php print $skinr; ?>">
  <div class="inner clearfix">
    <div class="corner-top"><div class="corner-top-right corner"></div><div class="corner-top-left corner"></div></div>
    <div class="inner-inner">
      <?php if (isset($edit_links)): ?>
      <?php print $edit_links; ?>
      <?php endif; ?>
      <?php if ($block->subject): ?>
      <h2 class="title block-title"><?php print $block->subject ?></h2>
      <?php endif;?>
      <div class="content clearfix">
        <?php print $block->content ?>
      </div><!-- /content -->
    </div><!-- /inner-inner -->
    <div class="corner-bottom"><div class="corner-bottom-right corner"></div><div class="corner-bottom-left corner"></div></div>
  </div><!-- /block-inner -->
</div><!-- /block -->
