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
      <span class="og_info_link_node_count">，<strong><?php echo $og->node_count ?></strong> 个主题</span>
    <?php endif?>
    
    <?php
      if (!empty($og->links['add'])) {
        foreach ($og->links['add'] as $key => $info) {
          echo '<a href="' . $info['url'] . '" class="og_links_add og_links_add_' . $key . '">' . $info['title'] . '</a>';
        }
      } else {
        echo '<a href="' . $og->url . '" title="请先加入小组" class="og_links_add_topic confirm_msg">我要发言</a>';
      }
    ?>

    <?php if (!$og->is_user) { ?>
      <?php if (!empty($og->links['join'])) {?>
        <a href="<?php echo $og->links['join']?>" class="og_links_join">加入小组</a>
      <?php } else {?>
        <a href="<?php echo $og->url?>" class="og_links_join login_msg">加入小组</a>
      <?php }?>
    <?php } ?>
    <?php if (!empty($og->links['logout'])) : ?>
      <a href="<?php echo $og->links['logout']?>" class="og_links_logout confirm">退出小组</a>
    <?php endif ?>

    <?php if (!empty($og->feed_og_url)) : ?>
      <a href="<?php echo $og->feed_og_url?>" title="订阅<?php echo $og->og_name;?>的最新话题">订阅话题</a>
    <?php endif ?>
    
    <?php if (!empty($og->feed_comment_url)) : ?>
      <a href="<?php echo $og->feed_comment_url?>" title="订阅<?php echo $og->og_name;?>的最新评论">订阅评论</a>
      <?php endif ?>

    <?php if ($og->links['admin']): ?>
      <a href="<?php echo $og->links['admin']?>" class="og_links_admin">管理小组</a>
    <?php endif?>
  </div>

</div>

<?php 
  if (count($og->tabs) > 1) {
    echo theme('item_list', $og->tabs, NULL, 'ul', array('class' => 'tabs nav nav-tabs nav-tabs-og-list'));
  }
?>

<?php if (!empty($og->term) && $og->term->description) : ?>

<div class="og_term_description">
  <?php echo filter_view($og->term->description); ?>
</div>

<?php endif; ?>

<div class="og_search_form"><?php echo $og->search_form; ?></div>

<div class="og_node">
  <?php 
    if (!empty($og->nodes)) {
      $header = array(
        array('data' => t('og', '标题'), 'class' => 'og_node_list_table_th_title'),
        array('data' => t('og', '作者'), 'class' => 'og_node_list_table_th_name'),
        array('data' => t('og', '创建时间'), 'class' => 'og_node_list_table_th_created'),
        array('data' => t('og', '回应'), 'class' => 'og_node_list_table_th_comment'),
        array('data' => t('og', '更新时间'), 'class' => 'og_node_list_table_th_updated'),
      );     
      $table = array();
      foreach ($og->nodes as $o) {
        $last_info = '';
        
        if ($o->data) {
          $o->data = unserialize($o->data);
          if (!empty($o->data['last_username'])) {
            
            if ($o->comment_count > var_get('og_comment_page_count', 20)) {
              $query = 'page='.floor($o->comment_count/var_get('og_comment_page_count', 20));
            } else {
              $query = NULL;
            }
            $last_info = t('og', '!user <br/>于 !time 前', array( 
              '!user' => l($o->data['last_username'], 'group/node/' . $o->nid,
              array('query' => $query, 'fragment' => 'comment_og_' . $o->data['last_cid'])),
              '!time' => format_interval($_SERVER['REQUEST_TIME']-$o->updated, 3)
            ));
          }
        }
        
        if (!$last_info) {
          $last_info = t('og', '!time 前', array('!time' => format_interval($_SERVER['REQUEST_TIME']-$o->updated, 3)));
        }
        
        if ($o->top) {
          $top = 'og_node_top';
        } else {
          $top = NULL;
        }
        
        $rows = array(
          array(
            'data' => l($o->title, 'group/node/'.$o->nid),
            'class' => 'og_node_list_title'
          )
        );
      
        $rows[] = array(
          'data' => l($o->og_name, 'group/'.$o->alias),
          'class' => 'og_node_list_og_name'
        );
      
        $rows[] = array(
          'data' => theme('username', $o),
          'class' => 'og_node_list_user'
        );
      
        $rows[] = array(
          'data' => format_date($o->created, 'custom', 'y/m/d H:i:s'),
          'class' => 'og_node_list_create'
        );
      
        $rows[] = array(
          'data' => l($o->comment_count, 'group/node/'.$o->nid, array('fragment' => 'og_node_comment_wrapper')),
          'class' => 'og_node_list_comment'
        );
        
        $rows[] = array(
          'data' => $last_info,
          'class' => 'og_node_list_update'
        );
        
        $table[] = array('data' => $rows, 'class' => $top);
      }
      echo theme('table', $header, $table, array('id' => 'og_node_list_table_og_front_' . $og->oid, 'class' => 'og_node_list_table'));
    } else if (!empty($og->content)) {
      echo $og->content;
    } else {
      echo system_no_content();
    }
    echo $og->pager;
  ?>
</div>
