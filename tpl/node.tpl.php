<?php
// $Id$
?>

<h1 class="og_title"><?php echo $node->title?></h1>
<div id="og_node_content_<?php echo $node->nid?>" class="og_node_content og_node_content_parent_<?php echo $node->oid;?>">
  <div class="og_node_view_node">
    <div class="og_node_view_content">

      <div class="og_node_view_header">
        <p>
        <span class="og_node_view_header_name">
          <?php 
            if (!empty($node->uid)) {
              echo theme('username', $node);
            } else {
              echo dd_get_ip($node->data['insert_host'], 1, 0);
            }
          ?>
        </span>
        <span class="og_node_view_header_time"><?php echo t('og', '发布于 !time', array('!time' => format_date($node->created)))?></span>
        </p>
        <?php if (!empty($node->field_html)) : ?><?php echo $node->field_html; ?><?php endif; ?>
      </div>

      <div class="og_node_view_body clearfix"><?php echo $node->body?></div>
    </div>
    
    <div class="og_node_view_links">

      <?php if ($node->comment_count) {?>共 <strong><?php echo $node->comment_count?></strong> 条回应
      <?php } else {?> 还没有回应<?php }?>
      
      <?php if ($node->comment_count && $node->filter_comment) : ?>
      <a href="<?php echo $node->filter_comment?>" rel="nofollow">只看<?php echo $node->name?></a>
      <?php endif?>
      
      <?php if ($node->is_update) : ?>
      <a href="<?php echo $node->update_url?>">编辑</a>
      <?php endif?>
      
      <?php if ($node->is_recycle) : ?>
      <a href="<?php echo $node->recycle_url?>" class="confirm">放入回收站</a>
      <?php endif?>
      
      <?php if ($node->is_close) : ?>
      <a href="<?php echo $node->close_url?>" class="confirm">关闭回复</a>
      <?php endif?>
      
      <?php if ($node->is_open) : ?>
      <a href="<?php echo $node->open_url?>" class="confirm">打开回复</a>
      <?php endif?>
      
      <?php if ($node->is_delete) : ?>
      <a href="<?php echo $node->delete_url?>" class="confirm">删除</a>
      <?php endif?>
      
      <a href="<?php echo $node->feed_comment_url?>" title="订阅 <?php echo $node->title;?> 的最新评论">订阅本话题</a>
      <a href="<?php echo $node->feed_og_url?>" title="订阅 <?php echo $node->og->og_name;?> 的最新话题">订阅本小组</a>

    </div>
    
  </div>
  
  <div id="og_node_comment_wrapper" class="og_node_view_comments">
    <?php if ($node->comment_view):?>
      <?php echo $node->comment_view?>
      <?php echo $node->comment_pager?>
    <?php endif?>
    
    <?php if ($node->is_comment) { ?>
      <?php echo $node->comment_form?>
    <?php } else {?>
      <div class="og_node_view_comments_message"><?php echo $node->comment_form?></div>
    <?php }?>
  </div>
</div>
