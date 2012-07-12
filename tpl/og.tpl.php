<?php
// $Id$
?>

<div class="og_info">
  <div class="og_info_logo">
    <img src="<?php echo $og->logo?>" alt="<?php echo $og->og_name ?> LOGO" title="<?php echo $og->og_name ?>"/>
  </div>
  
  <div class="og_info_content">
    <div class="og_info_header">
      <span class="og_info_header_create">创建于<?php echo format_date($og->created); ?></span>
      <span class="og_info_header_user">组长：<?php echo theme('username', $og); ?></span>
    </div>
    <div class="og_info_des"><?php echo $og->data['des']?></div>
  </div>
  
  <div class="og_info_link">
    <span class="og_info_link_node_type">小组状态：<?php echo $og->type_message ?></span>
    
    <span class="og_info_link_user_count">共<strong>
    <a href="<?php echo $og->url?>/user"><?php echo $og->user_count ?></a>
    </strong>名成员</span>
    
    <?php if ($og->node_count): ?>
      <span class="og_info_link_node_count">，<strong><?php echo $og->node_count ?></strong>个话题</span>
    <?php endif?>
    
    <?php if ($og->links['addtopic']) { ?>
      <a href="<?php echo $og->links['addtopic']?>" class="og_links_add_topic">我要发言</a>
    <?php } else {?>
      <a href="<?php echo $og->url?>" title="请先加入小组" class="og_links_add_topic confirm_msg">我要发言</a>
    
      <?php if ($og->links['join']) {?>
        <a href="<?php echo $og->links['join']?>" class="og_links_join">加入小组</a>
      <?php } else {?>
        <a href="<?php echo $og->url?>" class="og_links_join login_msg">加入小组</a>
      <?php }?>
      
    <?php }?>
    
    <?php if ($og->links['logout']): ?>
      <a href="<?php echo $og->links['logout']?>" class="og_links_logout confirm">退出小组</a>
    <?php endif?>
    
    <?php if ($og->links['admin']): ?>
      <a href="<?php echo $og->links['admin']?>" class="og_links_admin">管理小组</a>
    <?php endif?>
  </div>

</div>

<?php 
  if (!empty($og->tabs)) {
    echo theme('item_list', $og->tabs, NULL, 'ul', array('class' => 'tabs'));
  }
?>

<?php if (!empty($og->term) && $og->term->description) : ?>

<div class="og_term_description">
  <?php echo filter_view($og->term->description); ?>
</div>

<?php endif; ?>

<div class="og_node">
  <?php 
    if (!empty($og->nodes)) {
      echo $og->nodes;
    } else if (!empty($og->content)) {
      echo $og->content;
    } else {
      echo system_no_content();
    }
    echo $og->pager;
  ?>
</div>
