<?php
// $Id$

/**
 * 用户小组首页
 * @param (object) $ac
 *  用户对象
 */
function og_user_view($ac) {
  global $user;
  
  dd_set_tabs(array(
    l(t('og', '!string的主页', array('!string' => $ac->title)), $ac->path),
    l(t('og', '!string的小组', array('!string' => $ac->title)), $ac->path.'/group'),
  ));
  
  dd_set_sub_tabs(array(
    l(t('og', '!string的小组', array('!string' => $ac->title)), $ac->path.'/group'),
    l(t('og', '!string的话题', array('!string' => $ac->title)), $ac->path.'/group/node'),
    l(t('og', '!string评论过的话题', array('!string' => $ac->title)), $ac->path.'/group/comment'),
    l(t('og', '所有小组'), 'group/all'),
    l(t('og', '所有话题'), 'group/node'),
  ));
  
  dd_set_title(array(t('og', '!string的小组', array('!string' => $ac->title))));
  
  if (user_access('og add')) {
    dd_set_tabs(array(l(t('og', '创建小组'), 'group/add')));
  }
  
  $header = array(
    t('og', 'LOGO'),
    array('data' => t('og', '小组名称'), 'field' => 'o.og_name'),
    array('data' => t('og', '组长'), 'field' => 'o.uid'),
    array('data' => t('og', '创建时间'), 'field' => 'o.oid', 'sort' => 'desc'),
    array('data' => t('og', '成员'), 'field' => 'o.user_count'),
    array('data' => t('og', '话题'), 'field' => 'o.node_count'),
    array('data' => t('og', '状态'), 'field' => 'o.status'),
    t('og', '操作')
  );
  
  // 自己可以查看私密、待审核小组
  if ($user->uid == $ac->uid) {
    $fetch = pager_query('SELECT o.uid, o.oid, o.alias, o.og_name, o.status, o.type, o.logo, o.node_count,
    o.created, o.user_count, i.name, u.user_status, u.rid FROM {og} o INNER JOIN {og_user} u ON o.oid = u.oid 
    INNER JOIN {users} i ON o.uid = i.uid WHERE u.uid = ? AND u.user_status != -1'
    .table_sql($header), 20, 0, NULL, array($user->uid));
    
  } else { // 其它人，只可查看公开、正常小组
    $fetch = pager_query('SELECT o.uid, o.oid, o.alias, o.status, o.og_name, o.logo, o.node_count,
    o.created, o.user_count, o.uid, i.name FROM {og} o INNER JOIN {og_user} u ON o.oid = u.oid INNER JOIN 
    {users} i ON o.uid = i.uid WHERE o.status = 1 AND o.type > 0 AND u.uid = ? AND u.user_status = 1'
    .table_sql($header), 20, 0, NULL, array($ac->uid));
  }
  
  if ($fetch) {
    foreach ($fetch as $o) {
      if ($o->logo) {
        $o->logo = image_get_thumb($o->oid, $o->logo, '50x50');
      } else {
        $o->logo = var_get_key('og', 'default_logo', 'misc/images/logo.png');
      }
      
      $links = array();
      
      $links[] = l(t('og', '访问'), 'group/'.$o->alias);
      
      if ($ac->uid == $user->uid) {
        if ($o->uid == $user->uid) {
          if ($o->status != 0) {
            $links[] = l(t('og', '管理'), 'group/'.$o->alias.'/settings');
          } else {
            $links[] = '<a href="#" class="confirm_msg" title="'.t('og', '正在审核，请稍候').'">'.t('og', '管理').'</a>';
          }
        }
        
        $links[] = l(t('og', '发表话题'), 'group/'.$o->alias.'/add/topic');
        
      }
      
      if ($o->status == 0) {
        $status = t('og', '待审核');
      } else if ($o->status == -1) {
        $status = t('og', '已屏蔽');
      } else {
        $status = t('og', '正常');
      }
      
      $data .= l($o->og_name, 'group/'.$o->alias);
      $table[] = array(
        array(
          'data' => l(img($o->logo, $o->og_name, $o->og_name), 'group/'.$o->alias, array('html' => true)),
          'class' => 'og_user_table_logo'
        ),
        array('data' => l($o->og_name, 'group/'.$o->alias), 'class' => 'og_user_table_name'),
        array('data' => theme('username', $o), 'class' => 'og_user_table_user'),
        array('data' => format_date($o->created), 'class' => 'og_user_table_created'),
        array('data' => l($o->user_count, 'group/'.$o->alias.'/user'), 'class' => 'og_user_table_user_count'),
        array('data' => l($o->node_count, 'group/'.$o->alias), 'class' => 'og_user_table_node_count'),
        array('data' => $status, 'class' => 'og_user_table_status'),
        array('data' => dd_imp($links), 'class' => 'og_user_table_links')
      );
    }
    
    $output = theme('table', $header, $table, array('class' => 'og_user_table og_user_table_group'));
    $output .= pager_view();
    
  } else if ($user->uid == $ac->uid && user_access('og add')) {
    $output = system_no_content(t('og', '你还没有创建或加入任何群组，<a href="!url">立即创建</a>', array('!url' => url('group/add'))));
  } else {
    $output = system_no_content(t('og', '%string 还没有创建或加入任何群组', array('%string' => $ac->name)));
  }
  
  dd_set_html_head('<link rel="alternate" type="application/rss+xml" title="'.t('og', '订阅最新话题').'" href="'.f('custom.php?module=og&op=feed&type=node&bootstrap=full').'" />');
  dd_set_html_head('<link rel="alternate" type="application/rss+xml" title="'.t('og', '订阅最新评论').'" href="'.f('custom.php?module=og&op=feed&type=comment&bootstrap=full').'" />');
  
  dd_add_css(dd_get_path('module', 'og').'/og.css');
  return $output; 
}

/**
 * 小组列表，包括所有公开的小组
 */
function og_page_all() {
  global $user;
  
  if ($user->uid) {
  
    dd_set_tabs(array(
      l(t('og', '我的主页'), $user->path),
      l(t('og', '我的小组'), $user->path.'/group', array('attributes' => array('class' => 'active'))),
    ));
    
    dd_set_sub_tabs(array(
      l(t('og', '我的小组'), $user->path.'/group'),
      l(t('og', '我的话题'), $user->path.'/group/node'),
      l(t('og', '我评论过的话题'), $ac->path.'/group/comment'),
      l(t('og', '所有小组'), 'group/all'),
      l(t('og', '所有话题'), 'group/node'),
    ));
    
  } else {
    dd_set_tabs(array(
      l(t('og', '小组'), 'group/all'),
      l(t('og', '话题'), 'group/node'),
    ));
  }
  
  if (user_access('og add')) {
    dd_set_tabs(array(l(t('og', '创建小组'), 'group/add')));
  }
  
  $header = array(
    t('og', 'LOGO'),
    array('data' => t('og', '小组名称'), 'field' => 'o.og_name'),
    array('data' => t('og', '组长'), 'field' => 'o.uid'),
    array('data' => t('og', '创建时间'), 'field' => 'o.oid', 'sort' => 'desc'),
    array('data' => t('og', '成员'), 'field' => 'o.user_count'),
    array('data' => t('og', '话题'), 'field' => 'o.node_count'),
    t('og', '操作')
  );
  
  if ($fetch = pager_query('SELECT o.uid, o.oid, o.alias, o.status, o.og_name, o.logo, o.node_count,
  o.created, o.user_count, o.uid, i.name FROM {og} o INNER JOIN {users} i ON o.uid = i.uid 
  WHERE o.status = 1 AND o.type > 0'.table_sql($header), 20)) {
    foreach ($fetch as $o) {
      if ($o->logo) {
        $o->logo = image_get_thumb($o->oid, $o->logo, '50x50');
      } else {
        $o->logo = var_get_key('og', 'default_logo', 'misc/images/logo.png');
      }
      
      $links = array();
      
      $links[] = l(t('og', '访问'), 'group/'.$o->alias);
      
      if ($o->uid == $user->uid) {
        if ($o->status != 0) {
          $links[] = l(t('og', '管理'), 'group/'.$o->alias.'/settings');
        } else {
          $links[] = '<a href="#" class="confirm_msg" title="'.t('og', '正在审核，请稍候').'">'.t('og', '管理').'</a>';
        }
      }
      
      $data .= l($o->og_name, 'group/'.$o->alias);
      $table[] = array(
        array(
          'data' => l(img($o->logo, $o->og_name, $o->og_name), 'group/'.$o->alias, array('html' => true)),
          'class' => 'og_user_table_logo'
        ),
        array('data' => l($o->og_name, 'group/'.$o->alias), 'class' => 'og_user_table_name'),
        array('data' => theme('username', $o), 'class' => 'og_user_table_user'),
        array('data' => format_date($o->created), 'class' => 'og_user_table_created'),
        array('data' => l($o->user_count, 'group/'.$o->alias.'/user'), 'class' => 'og_user_table_user_count'),
        array('data' => l($o->node_count, 'group/'.$o->alias), 'class' => 'og_user_table_node_count'),
        array('data' => dd_imp($links), 'class' => 'og_user_table_links')
      );
    }
    
    $output = theme('table', $header, $table, array('class' => 'og_user_table og_user_table_group'));
    $output .= pager_view();
    
  } else if (user_access('og add')) {
    $output = system_no_content(t('og', '还没有群组，<a href="!url">立即创建</a>', array('!url' => url('group/add'))));
  } else {
    $output = system_no_content();
  }
  
  dd_set_html_head('<link rel="alternate" type="application/rss+xml" title="'.t('og', '订阅最新话题').'" href="'.f('custom.php?module=og&op=feed&type=node&bootstrap=full').'" />');
  dd_set_html_head('<link rel="alternate" type="application/rss+xml" title="'.t('og', '订阅最新评论').'" href="'.f('custom.php?module=og&op=feed&type=comment&bootstrap=full').'" />');
  dd_add_css(dd_get_path('module', 'og').'/og.css');
  return $output; 
}


/**
 * 用户发表的话题
 * @param (object) $ac
 */
function og_user_node($ac) {
  global $user;
  
  dd_set_tabs(array(
    l(t('og', '!string的主页', array('!string' => $ac->title)), $ac->path),
    l(
      t('og', '!string的小组', 
      array('!string' => $ac->title)), $ac->path.'/group',
      array('attributes' => array('class' => 'active'))
    ),
  ));
  
  dd_set_sub_tabs(array(
    l(t('og', '!string的小组', array('!string' => $ac->title)), $ac->path.'/group'),
    l(t('og', '!string的话题', array('!string' => $ac->title)), $ac->path.'/group/node'),
    l(t('og', '!string评论过的话题', array('!string' => $ac->title)), $ac->path.'/group/comment'),
    l(t('og', '所有小组'), 'group/all'),
    l(t('og', '所有话题'), 'group/node'),
  ));
  
  $header = array(
    array('data' => t('og', '话题'), 'field' => 'n.nid'),
    array('data' => t('og', '小组'), 'field' => 'n.oid'),
    array('data' => t('og', '发布时间'), 'field' => 'n.created'),
    array('data' => t('og', '回应'), 'field' => 'n.comment_count'),
    array('data' => t('og', '更新时间'), 'field' => 'n.updated', 'sort' => 'desc'),
  );
  
  dd_set_title(array(t('og', '!string的话题', array('!string' => $ac->title))));
  
  if (user_access('og add')) {
    dd_set_tabs(array(l(t('og', '创建小组'), 'group/add')));
  }
  
  // 自己可以查看已加入的公开、秘密小组的所有正常(status > 0)话题，以及自己发布的所有话题
  if ($user->uid == $ac->uid) {
    
    $fetch = pager_query('SELECT n.data, n.title, n.nid, n.comment_count, n.node_status, n.uid, n.updated,
    n.created, o.og_name, o.alias FROM {og} o INNER JOIN {og_user} u ON o.oid = u.oid 
    INNER JOIN {og_node} n ON o.oid = n.oid WHERE o.status = 1 AND u.uid = ? AND u.user_status != -1 AND 
    n.uid = ? AND n.node_status > 0' .table_sql($header), 20, 0, NULL, array($ac->uid, $ac->uid));
    
  } else { // 其它人只可查看当前用户在非秘密小组发表的公共(status > 0)话题
    
    $fetch = pager_query('SELECT n.data, n.title, n.nid, n.comment_count, n.node_status, n.uid, n.updated,
    n.created, o.og_name, o.alias FROM {og} o INNER JOIN {og_user} u ON o.oid = u.oid 
    INNER JOIN {og_node} n ON o.oid = n.oid WHERE u.uid = ? AND n.uid = ? AND u.user_status != -1 AND 
    n.node_status > 0 AND o.status = 1 AND o.type > 0' 
    .table_sql($header), 20, 0, NULL, array($ac->uid, $ac->uid));
    
  }
  
  if ($fetch) {
    $output = og_node_list_default($fetch, $header, 1, 0, 1, 'user_node');
    $output .= pager_view();
  } else {
    $output = system_no_content();
  }
  
  dd_set_html_head('<link rel="alternate" type="application/rss+xml" title="'.t('og', '订阅最新话题').'" href="'.f('custom.php?module=og&op=feed&type=node&bootstrap=full').'" />');
  dd_set_html_head('<link rel="alternate" type="application/rss+xml" title="'.t('og', '订阅最新评论').'" href="'.f('custom.php?module=og&op=feed&type=comment&bootstrap=full').'" />');
  dd_add_css(dd_get_path('module', 'og').'/og.css');
  return $output; 
}

/**
 * 用户评论过的话题
 * @param (object) $ac
 */
function og_user_comment($ac) {
  global $user;
  
  dd_set_tabs(array(
    l(t('og', '!string的主页', array('!string' => $ac->title)), $ac->path),
    l(
      t('og', '!string的小组', 
      array('!string' => $ac->title)), $ac->path.'/group',
      array('attributes' => array('class' => 'active'))
    ),
  ));
  
  dd_set_sub_tabs(array(
    l(t('og', '!string的小组', array('!string' => $ac->title)), $ac->path.'/group'),
    l(t('og', '!string的话题', array('!string' => $ac->title)), $ac->path.'/group/node'),
    l(t('og', '!string评论过的话题', array('!string' => $ac->title)), $ac->path.'/group/comment'),
    l(t('og', '所有小组'), 'group/all'),
    l(t('og', '所有话题'), 'group/node'),
  ));
  
  $header = array(
    array('data' => t('og', '话题'), 'field' => 'n.nid'),
    array('data' => t('og', '小组'), 'field' => 'n.oid'),
    array('data' => t('og', '作者'), 'field' => 'n.uid'),
    array('data' => t('og', '发布时间'), 'field' => 'n.created'),
    array('data' => t('og', '回应'), 'field' => 'n.comment_count'),
    array('data' => t('og', '更新时间'), 'field' => 'n.updated', 'sort' => 'desc'),
  );
  
  dd_set_title(array(t('og', '!string的话题', array('!string' => $ac->title))));
  
  if (user_access('og add')) {
    dd_set_tabs(array(l(t('og', '创建小组'), 'group/add')));
  }
  
  // 自己可以查看已加入的公开、秘密小组的所有正常(status > 0)话题，以及自己发布的所有话题
  if ($user->uid == $ac->uid) {
    
    $count = db_query('SELECT COUNT(DISTINCT(c.ext_id)) FROM {og} o 
    INNER JOIN {og_node} n ON o.oid = n.oid INNER JOIN {comment} c ON c.ext_id = n.nid 
    WHERE c.uid = ? AND c.ext_type = ? AND o.status = 1 AND n.node_status > 0', 
    array($ac->uid, 'og'), array('return' => 'column'));
    
    $fetch = pager_query('SELECT DISTINCT(c.ext_id), n.data, n.title, n.nid, 
    n.comment_count, n.node_status, n.uid, n.updated, n.created, o.og_name, 
    o.alias, u.name FROM {og} o INNER JOIN {og_node} n ON o.oid = n.oid INNER JOIN 
    {users} u ON n.uid = u.uid INNER JOIN {comment} c ON c.ext_id = n.nid 
    WHERE c.uid = ? AND c.ext_type = ? AND o.status = 1 AND n.node_status > 0' 
    . table_sql($header), 20, 0, $count, array($ac->uid, 'og'));
    
  } else { // 其它人只可查看当前用户在非秘密小组发表的公共(status > 0)话题
    
    $count = db_query('SELECT COUNT(DISTINCT(c.ext_id)) FROM {og} o 
    INNER JOIN {og_node} n ON o.oid = n.oid INNER JOIN {comment} c ON c.ext_id = n.nid 
    WHERE c.uid = ? AND c.ext_type = ? AND o.status = 1 AND n.node_status > 0 AND o.type > 0', 
    array($ac->uid, 'og'), array('return' => 'column'));
    
    $fetch = pager_query('SELECT DISTINCT(c.ext_id), n.data, n.title, n.nid, 
    n.comment_count, n.node_status, n.uid, n.updated, n.created, o.og_name, 
    o.alias, u.name FROM {og} o INNER JOIN {og_node} n ON o.oid = n.oid INNER JOIN 
    {users} u ON n.uid = u.uid INNER JOIN {comment} c ON c.ext_id = n.nid 
    WHERE c.uid = ? AND c.ext_type = ? AND o.status = 1 AND o.type > 0 
    AND n.node_status > 0' . table_sql($header), 20, 0, $count, array($ac->uid, 'og'));
    
  }
  
  if ($fetch) {
    $output = og_node_list_default($fetch, $header, 1, 1, 1, 'user_comment');
    $output .= pager_view();
  } else {
    $output = system_no_content();
  }
  
  dd_set_html_head('<link rel="alternate" type="application/rss+xml" title="'.t('og', '订阅最新话题').'" href="'.f('custom.php?module=og&op=feed&type=node&bootstrap=full').'" />');
  dd_set_html_head('<link rel="alternate" type="application/rss+xml" title="'.t('og', '订阅最新评论').'" href="'.f('custom.php?module=og&op=feed&type=comment&bootstrap=full').'" />');
  dd_add_css(dd_get_path('module', 'og').'/og.css');
  return $output; 
}

/**
 * 所有公开的话题
 */
function og_page_node() {
  global $user;
  
  if ($user->uid) {
    dd_set_tabs(array(
      l(t('og', '我的主页'), $user->path),
      l(t('og', '我的小组'), $user->path.'/group', array('attributes' => array('class' => 'active'))),
    ));
    
    dd_set_sub_tabs(array(
      l(t('og', '我的小组'), $user->path.'/group'),
      l(t('og', '我的话题'), $user->path.'/group/node'),
      l(t('og', '我评论过的话题'), $user->path.'/group/comment'),
      l(t('og', '所有小组'), 'group/all'),
      l(t('og', '所有话题'), 'group/node'),
    ));
  } else {
    dd_set_tabs(array(
      l(t('og', '小组'), 'group/all'),
      l(t('og', '话题'), 'group/node'),
    ));
  }
  
  $header = array(
    array('data' => t('og', '话题'), 'field' => 'n.nid'),
    array('data' => t('og', '小组'), 'field' => 'n.oid'),
    array('data' => t('og', '作者'), 'field' => 'n.uid'),
    array('data' => t('og', '发布时间'), 'field' => 'n.created'),
    array('data' => t('og', '回应'), 'field' => 'n.comment_count'),
    array('data' => t('og', '更新时间'), 'field' => 'n.updated', 'sort' => 'desc'),
  );
  
  if (user_access('og add')) {
    dd_set_tabs(array(l(t('og', '创建小组'), 'group/add')));
  }
  
  // 只可查看非秘密小组发表的公共(status > 0)话题
  if ($fetch = pager_query('SELECT n.data, n.title, n.nid, n.comment_count, n.node_status, n.uid, n.updated,
  n.created, o.og_name, o.alias, r.name FROM {og} o INNER JOIN {og_node} n ON o.oid = n.oid 
  INNER JOIN {users} r ON n.uid = r.uid WHERE n.node_status > 0 AND o.status = 1 AND o.type > 0' .
  table_sql($header), 20)) {
    $output = $output = og_node_list_default($fetch, $header);
    $output .= pager_view();
  } else {
    $output = system_no_content();
  }
  
  dd_set_html_head('<link rel="alternate" type="application/rss+xml" title="'.t('og', '订阅最新话题').'" href="'.f('custom.php?module=og&op=feed&type=node&bootstrap=full').'" />');
  dd_set_html_head('<link rel="alternate" type="application/rss+xml" title="'.t('og', '订阅最新评论').'" href="'.f('custom.php?module=og&op=feed&type=comment&bootstrap=full').'" />');
  
  dd_add_css(dd_get_path('module', 'og').'/og.css');
  return $output; 
}

/**
 * 节点浏览
 * @param (object) $node
 *  节点对象
 */
function og_node_view($node) {
  global $user;
  
  if ($user->uid > 0) {
    $breadcrumb = array(
      l(t('og', '我的小组'), $user->path.'/group'),
    );
  } else {
    $breadcrumb = array(
      l(t('og', '小组'), 'group/all'),
      l(t('og', '话题'), 'group/node'),
    );
  }
  
  if (!empty($node->og->field_id)) {
  }
  
  $breadcrumb[] = l($node->og->og_name, $node->og->path);
  $breadcrumb[] = $node->title;
  
  dd_set_breadcrumb($breadcrumb);

  // $node->avatar = f(user_get_avatar($node->uid, '50x50'));
  
  $node->body = filter_view($node->body, $node->format);
  
  if ($node->is_delete = og_access_node_delete($node)) {
    $node->delete_url = url('group/node/delete/'.$node->nid);
  }
  
  if ($node->is_update = og_access_node_update($node)) {
    $node->update_url = url('group/node/edit/'.$node->nid);
  }
  
  $node->is_comment = $node->is_recycle = $node->is_close = $node->is_open = NULL;
  
  if (og_access_node_status($node)) {
    if ($node->node_status != 0) {
      if ($node->node_status == 1) {
        $node->close_url = url('group/node/status/'.$node->nid, array('query' => 'op=close'));
        $node->is_close = true;
      } else if ($node->node_status == 2) {
        $node->open_url = url('group/node/status/'.$node->nid, array('query' => 'op=open'));
        $node->is_open = true;
      }
      $node->recycle_url = url('group/node/status/'.$node->nid, array('query' => 'op=recycle'));
      $node->is_recycle = true;
    }
  }
  
  if (og_is_group_user($user->uid, $node->og)) {
    if ($node->node_status == 1) {
      $comment = new stdClass;
      $comment->ext_type = 'og';
      $comment->ext_id = $node->nid;
      $comment->ext_id2 = $node->oid;
      $comment->title = t('og', '回复：!title', array('!title' => $node->title));
      $node->is_comment = true; 
      $node->comment_form = dd_get_form('comment_form', $comment);
    } else {
      $node->comment_form = t('og', '评论已禁止');
    }
  } else if ($user->uid) {
    $node->comment_form = l(t('og', '你需要加入小组才能发布评论'), $node->og->path.'/join', dd_redirect());
  } else {
    $node->comment_form = l(t('og', '你需要加入小组才能发布评论'), 'user/login', dd_redirect());
  }
  
  if ($node->comment_count) {
    
    if ($data = comment_ext_view('og', $node->nid, var_get('og_comment_page_count', 20), true)) {
      $node->comment_view = $data->view;
      $node->comment_pager = $data->pager;
    } else if (!empty($_GET['filter_comment'])) {
      $node->comment_view = system_no_content(l(t('og', '该用户没有发布评论，返回查看话题下的全部评论'), 'group/node/' . $node->nid));
      $node->comment_pager = '';
    }
    
    $node->filter_comment = url('group/node/' . $node->nid, array('query' => 'filter_comment=' . $node->uid));
    
  }
   
  dd_set_meta(array(
    'description' => mb_substr(str_replace(array("\n", "\t", "\r", ' ', '　'), '', strip_tags($node->body)), 0, 250) .'…'
  ));

  $node->feed_og_url = f('custom.php?module=og&op=feed&type=node&bootstrap=full&oid='.$node->og->oid);
  $node->feed_comment_url = f('custom.php?module=og&op=feed&type=comment&bootstrap=full&nid='.$node->nid);
  dd_set_html_head('<link rel="alternate" type="application/rss+xml" title="'.t('og', '订阅当前小组最新话题').'" href="'.$node->feed_og_url.'" />');
  dd_set_html_head('<link rel="alternate" type="application/rss+xml" title="'.t('og', '订阅当前话题最新评论').'" href="'.$node->feed_comment_url.'" />');
  
  module_alter_all('og_node_view', $node);

  if (!empty($node->fields)) {
    field_get_node_view('og', 'ext_node_' . $node->oid, $node);
    field_get_node_view('og', 'default', $node);
  }
  
  dd_set_title(array($node->title, $node->og->og_name));
  
  dd_add_css(dd_get_path('module', 'og').'/og.css');
  return theme('og_node', $node);
}

/**
 * 加入小组
 * @param (object) $og
 */
function og_join($og) {
  if ($og->type == 2) {
    $form['fields']['msg'] = array(
      '#title' => t('og', '需要审核'),
      '#value' => t('og', '小组设置了身份验证，需要通过审核才能加入')
    );
    $form['fields']['value'] = array(
      '#title' => t('og', '验证消息'),
      '#type' => 'textfield',
      '#description' => t('og', '告诉小组管理员，你是谁，为什么要加入小组')
    );
    $form['fields']['uid'] = array(
      '#type' => 'hidden',
      '#default_value' => $GLOBALS['user']->uid,
      '#constant' => 1
    );
    $form['fields']['oid'] = array(
      '#type' => 'hidden',
      '#default_value' => $og->oid,
      '#constant' => 1
    );
    $form['fields']['path'] = array(
      '#type' => 'hidden',
      '#default_value' => $og->path,
      '#constant' => 1
    );
    $form['fields']['submit']['#type'] = 'submit';
    $form['settings']['#validate'][] = 'og_join_validate';
    
    dd_set_breadcrumb(array(
      l(t('og', '我的小组'), $user->path.'/group'),
      l($og->og_name, $og->path),
      t('og', '加入申请')
    ));
  } else if ($og->type == 1) {
    if (db_exec('INSERT INTO {og_user} (uid, oid, user_status, rid, value, timestamp) VALUES (?, ?, 1, 0, ?, ?)',
    array($GLOBALS['user']->uid, $og->oid, '', $_SERVER['REQUEST_TIME']))) {
      og_set_user_count($og->oid);
      dd_log('og', t('og', '加入小组'), $og->oid, $og->path, 0, $GLOBALS['user']->uid);
      dd_set_message(t('og', '加入成功'));
      dd_goto($og->path);
    }
  }
  return $form;
}

function og_join_validate($form, $v) {
  if ($o = db_query('SELECT id, user_status, FROM {og_user} WHERE oid = ? AND uid = ?',
  array($v['oid'], $v['uid']), array('return' => 'column'))) {
    if ($o->user_status == -1) {
      dd_set_message(t('og', '错误的访问'));
      dd_goto('user/'.$v['uid']);
    } else {
      dd_set_message(t('og', '你已经加入过了'));
      dd_goto($v['path']);
    }
  } else if (db_exec('INSERT INTO {og_user} (uid, oid, user_status, rid, value, timestamp) VALUES (?, ?, 0, 0, ?, ?)',
  array($v['uid'], $v['oid'], $v['value'], $_SERVER['REQUEST_TIME']))) {
    
    dd_set_message(t('og', '申请已提交，请等待处理'));
    
    og_set_user_count($v['oid']);
    dd_log('og', t('og', '申请加入'), $v['oid'], $v['path'], 0, $v['uid']);
    og_set_user_wait($v['oid']);
    
    dd_goto('user/'.$v['uid']);
  }
}

/**
 * 退出小组
 * @param (object) $og
 */
function og_logout($og) {
  global $user;
  if ($_GET['op'] == 'bye') {
    if (db_exec('DELETE FROM {og_user} WHERE oid = ? AND uid = ?', array($og->oid, $user->uid))) {
      dd_log('og', t('og', '退出小组'), $og->oid, $og->path, 0, $user->uid);
      dd_set_message(t('og', '成功退出 %string 小组', array('%string' => $og->og_name)));
      og_set_user_count($og->oid);
      dd_goto($user->path);
    }
  }
  
  dd_set_breadcrumb(array(l(t('og', '我的小组'), $user->path.'/group'), l($og->og_name, $og->path)));
  
  dd_set_title(array(t('og', '退出小组')));
  
  dd_add_css(dd_get_path('module', 'og').'/og.css');
  
  return l(t('og', '确认退出吗？此操作不可恢复！'), $og->path.'/logout',
  array('query' => 'op=bye', 'attributes' => array('class' => 'og_logout_link')));
}

/**
 * 小组用户
 * @param (object) $og
 */
function og_view_user($og) {
  global $user;
  
  $output = og_view($og, 'user');
  
  if ($user->uid) {
    dd_set_breadcrumb(array(l(t('og', '我的小组'), $user->path.'/group'), l($og->og_name, $og->path), t('og', '成员')));
  } else {
    dd_set_breadcrumb(array(l(t('og', '小组'), 'group/all'), l($og->og_name, $og->path), t('og', '成员')));
  }
  
  if ($fetch = pager_query('SELECT o.uid, u.name FROM {og_user} o INNER JOIN {users} u ON o.uid = u.uid 
  WHERE o.oid = ? AND o.user_status = 1 ORDER BY o.timestamp DESC', 20, 0, NULL, array($og->oid))) {
    foreach ($fetch as $o) {
      $data = img(user_get_avatar($o->uid, '85x85'), $o->name);
      $data .= theme('username', $o);
      if ($og->uid == $o->uid) {
        $data .= '<span class="red">('.t('og', '组长').')</span>';
      }
      $item[] = $data;
    }
    
    dd_add_css(dd_get_path('module', 'og').'/og.css');
    
    $output .= theme('item_list', $item, NULL, 'ul', array('class' => 'og_view_users'));
    $output .= pager_view();
  }
  
  return $output;
}
