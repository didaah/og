<?php
// $Id$

// public og_update($og) {{{ 
/**
 * og_update
 *  小组设置
 * 
 * @param mixed $og 
 * @access public
 * @return void
 */
function og_update($og) {
  global $user;
  
  if (!empty($og->data['user_wait_status'])) {
    dd_set_help(t('og', '有%a个加入请求等待处理，<a href="!url">立即处理</a>',
    array('%a' => $og->data['user_wait_status'], '!url' => url($og->path . '/settings/user/wait', array('query' => 'status=0')))));
  }

  $args = arg();

  if (!empty($args[3]) && !in_array($args[3], array('node', 'user')) && $user->uid != $og->uid && !user_access('admin og')) {
    dd_set_message(t('og', '你所请求的管理功能只有组长才能够使用，请联系组长'));
    dd_goto($og->path . '/settings');
  }
  
  switch ($args[3]) {
    case 'node':
      $output = og_settings_nodes($og);
    break;
    case 'user':
      $output = og_settings_users($og);
    break;
    case 'alias':
      $output = dd_get_form('og_update_alias', $og);
    break;
    case 'logo':
      $output = dd_get_form('og_update_logo', $og);
    break;
    case 'edit':
      module_include_file('og.form.inc', 'og');
      $output = dd_get_form('og_form', $og);
    break;
    case 'field':
      $output = _og_settings_field($og);
    break;
    default:
      $header = array(
        array('data' => t('og', '时间'), 'field' => 'l.sid', 'sort' => 'desc'),
        array('data' => t('og', '用户'), 'field' => 'l.uid'),
        t('og', '注释'), t('og', '主机'), t('og', '路径')
      );
      if ($fetch = pager_query('SELECT l.timestamp, l.uid, l.value, l.url, l.host, u.name FROM {logs} l INNER JOIN {users} 
      u ON l.uid = u.uid WHERE l.ext_id = ? AND l.type = ?'.table_sql($header), 20, 0, NULL, array($og->oid, 'og'))) {
        foreach ($fetch as $o) {
          $table[] = array(
            format_date($o->timestamp),
            theme('username', $o),
            $o->value,
            dd_get_ip($o->host, 1), $o->url
          );
        }
        $output = theme('table', $header, $table, array('class' => 'og_settings_logs'));
        $output .= pager_view();
      } else {
        $output = system_no_content();
      }
  }
  
  dd_set_breadcrumb(array(
    l(t('og', '我的小组'), $user->path . '/group'),
    l($og->og_name, $og->path),
    t('og', '小组管理')
  ));
  
  // @TODO 权限问题待解决
  $tabs = array(l(t('og', '小组日志'), $og->path . '/settings'));
  $tabs[] = l(t('og', '基本设置'), $og->path . '/settings/edit');
  $tabs[] = l(t('og', '成员管理'), $og->path . '/settings/user');
  $tabs[] = l(t('og', '内容管理'), $og->path . '/settings/node');
  $tabs[] = l(t('og', '内容字段'), $og->path . '/settings/field');
  $tabs[] = l(t('og', '小组图标'), $og->path . '/settings/logo');
  $tabs[] = l(t('og', '别名设置'), $og->path . '/settings/alias');
  $tabs[] = l(t('og', '返回小组首页'), $og->path);

  dd_set_tabs($tabs);
  dd_add_css(dd_get_path('module', 'og').'/og.css');

  return $output;
}
// }}}

/**
 * 小组logo
 * @param (object) $og
 */
function og_update_logo($og) {
  if ($og->logo == var_get_key('og', 'default_logo', 'misc/images/logo.png')) {
    unset($og->logo); 
  }
  
  if ($og->logo) {
    $item[] = img(image_get_thumb($og->oid, $og->logo, '85x85'));
    $item[] = img(image_get_thumb($og->oid, $og->logo, '120x120'));
    $item[] = img(image_get_thumb($og->oid, $og->logo, '160x160'));
    $item[] = img(image_get_thumb($og->oid, $og->logo, '200x200'));
    $form['fields']['image'] = array(
      '#title' => t('og', '当前图标'),
      '#value' => theme('item_list', $item)
    );
  }
  
  $form['fields']['file'] = array(
    '#type' => 'file',
    '#required' => 1,
    '#title' => t('og', '小组图标'),
    '#description' => t('og', '你可以上传大图，最大 5 M，系统将根据不同的需求，生成缩略图，最佳比例 1：1'),
  );
  
  $form['fields']['logo'] = array(
    '#type' => 'hidden',
    '#default_value' => $og->logo,
    '#constant' => 1
  );
  
  $form['fields']['oid'] = array(
    '#type' => 'hidden',
    '#default_value' => $og->oid,
    '#constant' => 1
  );
  
  $form['settings']['#ajax_validate'] = 1;
  
  $form['fields']['submit'] = array(
    '#type' => 'submit',
    '#submit' => 'og_update_logo_submit',
    '#value' => t('og', '确认上传')
  );
  
  return $form;
}

function og_update_logo_submit($form, $v) {
  $validate = array(
    'savepath' => 'group/logo',
    'no_insert' => true, 
    'filename' => $v['oid'],
    'image' => array(
      'type' => array('jpg', 'png', 'gif'),
    ),
    'filesize' => '5120',
  );
  
  if ($file = file_save_upload($_FILES['file'], $validate, 0)) {
    dd_log('og', t('og', '设置了小组图标'), $v['oid']);
    if ($file->filepath == $v['logo']) {
      file_delete_dir(file_directory_path() . '/group/logo/'.$v['oid']);
    } else {
      db_exec('UPDATE {og} SET logo = ? WHERE oid = ?', array($file->filepath, $v['oid']));
      file_delete_file($v['logo'], $v['oid']);
    }
  }
}

/**
 * 小组别名
 * @param (object) $og
 */
function og_update_alias($og) {
  dd_set_help(t('og', '<p>别名，即友好的访问路径，默认使用数字做为访问路径，如：group/1，可以修改为：group/myname，其中的 myname 即为自定义别名。</p>设定后不要轻易修改，以免影响小组访问。'));
  
  $form['settings'] = array(
    '#validate' => array('og_update_alias_validate'),
    '#ajax_validate' => 1
  );
  $form['fields']['submit'] = array(
    '#type' => 'submit',
    '#weight' => 10
  );
  
  if ($og->alias == $og->oid) {
    unset($og->alias);
    $disable = false;
  } else if(!var_get_key('og', 'alias_update', 1)) {
    $form['settings'] = NULL;
    $form['fields']['submit'] = NULL;
    $disable = true;
  }
  $form['fields']['alias'] = array(
    '#type' => 'textfield',
    '#required' => 1,
    '#title' => t('og', '别名'),
    '#disabled' => $disable,
    '#default_value' => $og->alias,
    '#attributes' => array('maxlength' => 32, 'minlength' => 4),
    '#description' => t('og', '长度 4 - 32 个字符，只能是数字、字母、下划线(_)，且必须以字母开头')
  );
  
  $form['fields']['oid'] = array(
    '#type' => 'hidden',
    '#default_value' => $og->oid,
    '#constant' => 1
  );
  
  return $form;
}

function og_update_alias_validate($form, $v) {
  if (preg_match('/^[a-z][a-z0-9_]{3,}$/i', $v['alias'])) {
    if ($arr = og_alias_disable()) {
      if (array_search($v['alias'], $arr) !== false) {
        $t['alias']['#error'][] = t('og', '别名禁止使用，请换一个');
        return $t;
      }
    }
    if ($oid = db_query('SELECT oid FROM {og} WHERE alias = ? ', array($v['alias']), array('return' => 'column'))) {
      if ($oid == $v['oid']) {
        dd_set_message(t('og', '修改成功'));
        dd_goto('group/'.$v['alias'].'/settings/alias');
      } else {
        $t['alias']['#error'][] = t('og', '别名已经存在，请换一个');
        return $t;
      }
    }
    
    db_exec('UPDATE {og} SET alias = ? WHERE oid = ?', array($v['alias'], $v['oid']));
    dd_log('og', t('og', '设置了别名：!alias', array('!alias' => $v['alias'])), $v['oid']);
    dd_set_message(t('og', '修改成功'));
    dd_goto('group/'.$v['alias'].'/settings/alias');
    
  } else {
    $t['alias']['#error'][] = t('og', '别名不正确');
  }
  return $t;
}

/**
 * 节点管理
 * @param (object) $og
 */
function og_settings_nodes($og) {
  if ($_GET['op'] == 'status') {
    $node = og_node_load($_GET['nid']);
    if ($node->oid == $og->oid && $node->node_status != $_POST['value']) {
      
      $old_status = $node->node_status;
      $node->node_status = $_POST['value'];
      
      if ($_POST['value'] == 2) {
        $text = t('og', '!title 禁止回复，ID：!id', array('!title' => $o->title, '!id' => $o->nid));
      } else if ($_POST['value'] == 0) {
        $text = t('og', '!title 放入回收站，ID：!id', array('!title' => $o->title, '!id' => $o->nid));
      } else if ($_POST['value'] == 1) {
        $text = t('og', '!title 打开文章，ID：!id', array('!title' => $o->title, '!id' => $o->nid));
      } else {
        echo -1;
        exit;
      }
      
      if (db_exec('UPDATE {og_node} SET node_status = ? WHERE nid = ?', array($node->node_status, $node->nid))) {
        if ($_POST['value'] == 0) {
          og_set_node_count($og->oid);
        }
        
        module_invoke_all('og_node_status', $node, $old_status);
        
        dd_log('og', $o->title . $text, $og->oid);
        echo 'two';
      }
      
    }
    exit;
    
  } else if ($_GET['op'] == 'top') {
    if ($_POST['value'] == 0) {
      $text = t('og', '取消置顶');
    } else if ($_POST['value'] == 1) {
      $text = t('og', '文章置顶');
    } else {
      echo -1;
      exit;
    }
    
    $node = og_node_load($_GET['nid']);
    if ($node->oid == $og->oid && $node->top != $_POST['value']) {
      $old_top = $node->top;
      $node->top = $_POST['value'];
      if (db_exec('UPDATE {og_node} SET top = ? WHERE nid = ?', array($node->top, $node->nid))) {
        dd_log('og', $text, $og->oid);
        module_invoke_all('og_node_top', $node, $old_top);
      }
    } else {
      echo -1;
    }
    
    exit;
  }
  
  $header = array(
    t('og', '标题'),
    array('data' => t('og', '作者'), 'field' => 'n.uid'),
    array('data' => t('og', '回应'), 'field' => 'n.comment_count'),
    array('data' => t('og', '最后更新'), 'field' => 'n.updated', 'sort' => 'desc'),
    array('data' => t('og', '置顶'), 'field' => 'n.top'),
    t('og', '操作')
  );
  
  dd_set_sub_tabs(array(
    l(t('og', '开放的主题'), $og->path.'/settings/node'),
    l(t('og', '禁止回复的主题'), $og->path.'/settings/node/a'),
    l(t('og', '回收站'), $og->path.'/settings/node/b'),
  ));
  
  switch (arg(4)) {
    case 'a':
      $status = 2;
    break;
    case 'b':
      $status = 0;
    break;
    default:
      $status = 1;
  }
  
  if ($fetch = pager_query('SELECT n.title, n.top, n.nid, n.comment_count, n.node_status, n.uid, n.updated, u.name FROM {og_node} 
  n INNER JOIN {users} u ON n.uid = u.uid WHERE n.oid = ? AND n.node_status = ?'.table_sql($header), 20, 0, NULL, array($og->oid, $status))) {
    foreach ($fetch as $o) {
      $table[] = array(
        array(
          'data' => ($o->node_status ? l($o->title, 'group/node/'.$o->nid) : $o->title),
          'class' => 'og_settings_nodes_title'
        ),
        array('data' => theme('username', $o), 'class' => 'og_settings_nodes_user'),
        array('data' => $o->comment_count, 'class' => 'og_settings_nodes_count'),
        array('data' => format_date($o->updated), 'class' => 'og_settings_nodes_time'),
        array(
          'data' => dd_form_select(
            array(
              '#value' => $o->top,
              '#attributes' => array(
                'class' => 'changeconfirm dd_form_ajax_field',
                'href' => url($_GET['q'], array('query' => 'op=top&nid='.$o->nid))
              ),
              '#name' => 'og_top_'.$o->nid,
              '#options' => array(t('og', '正常'), t('og', '置顶'))
            )
          ),
          'class' => 'og_settings_nodes_top'
        ),
        array(
          'data' => dd_form_select(
            array(
              '#value' => $o->node_status,
              '#attributes' => array(
                'class' => 'changeconfirm dd_form_ajax_field',
                'href' => url($_GET['q'], array('query' => 'op=status&nid='.$o->nid))
              ),
              '#name' => 'og_nodes_'.$o->nid,
              '#options' => array(1 => t('og', '正常'), 2 => t('og', '禁止回复'), 0 => t('og', '回收站'))
            )
          ),
          'class' => 'og_settings_nodes_links'
        )
      );
    }
    
    $output = theme('table', $header, $table, array('class' => 'og_settings_nodes'));
    $output .= pager_view();
    
    dd_add_js(dd_get_path('module', 'og').'/og.js');
  } else {
    $output = system_no_content();
  }
  
  return $output;
}

/**
 * 成员管理
 * @param (object) $og
 */
function og_settings_users($og) {
  global $user;

  if (!empty($_POST) && !empty($_GET['op'])) { 

    // 不允许对小组创始人(管理员)进行操作
    if (empty($_GET['uid']) || $_GET['uid'] == $og->uid) {
      echo t('og', '错误的访问');
      exit;
    }

    // 检查要操作的用户是否存在
    if (!$name = db_query('SELECT name FROM {users} WHERE uid = ?',
    array($_GET['uid']), array('return' => 'column'))) {
      echo t('og', '错误的访问');
      exit;
    }

    if ($_GET['op'] == 'status') {
      $value = $_GET['value'];

      if ($value == -1) {
        $text = t('og', '将 %name 加入黑名单', array('%name' => $name));
      } else if ($value == 0) {
        $text = t('og', '将 %name 加入待审核', array('%name' => $name));
      } else if ($value == 1) {
        $text = t('og', '将 %name 加入已审核', array('%name' => $name));
      } else {
        echo -1;
        exit;
      }
      
      if ($uid = db_query('SELECT uid FROM {og_user} WHERE oid = ? AND user_status != ? AND uid = ?',
      array($og->oid, $value, $_GET['uid']), array('return' => 'column'))) {
        if (db_exec('UPDATE {og_user} SET user_status = ? WHERE uid = ?', array($value, $uid))) {
          // 重新统计小组成员总数
          og_set_user_count($og->oid);
          // 写入日志
          dd_log('og', $text, $og->oid);
          echo 'reload';
        }
      } else {
        echo t('og', '操作失败');
      }
      
      og_set_user_wait($og->oid);

      exit;
    } else if ($_GET['op'] == 'delete') {
      if (db_exec('DELETE FROM {og_user} WHERE oid = ? AND uid = ?', array($og->oid, $_GET['uid']))) {
        // 重新统计小组成员总数
        og_set_user_count($og->oid);
        // 重新统计待审核成员总数
        og_set_user_wait($og->oid);
        // 写入日志
        dd_log('og', t('og', '删除成员 %name', array('%name' => $name)), $og->oid);
        echo 'tr';
      } else {
        echo t('og', '删除失败');
      }
    }
    exit;
  }
  
  $header = array(
    array('data' => t('og', '成员'), 'field' => 'o.uid'),
    array('data' => t('og', '加入时间'), 'field' => 'o.timestamp', 'sort' => 'desc'),
    array('data' => t('og', '状态'), 'field' => 'o.user_status'),
    t('og', '操作')
  );
  
  $query = 'SELECT o.*, u.name FROM {og_user} o INNER JOIN {users} u ON o.uid = u.uid WHERE o.oid = ?';
  $args = array($og->oid);

  if (is_numeric($_GET['status'])) {
    $query .= ' AND o.user_status = ?';
    $args[] = $_GET['status'];
  }
  
  dd_set_sub_tabs(array(
    l(t('og', '全部成员'), $og->path . '/settings/user'),
    l(t('og', '待审核'), $og->path . '/settings/user/wait', array('query' => 'status=0')),
    l(t('og', '黑名单'), $og->path . '/settings/user/blacklist', array('query' => 'status=-1')),
  ));

  if ($fetch = pager_query($query . table_sql($header), 20, 0, NULL, $args)) {
    $status = array(
      -1 => t('og', '黑名单'),
      0 => t('og', '待审核'),
      1 => t('og', '正常'),
    );

    foreach ($fetch as $o) {
      if ($og->uid != $o->uid) {
        $links = array();

        $url = url($_GET['q'], array('query' => 'uid=' . $o->uid));

        if ($o->user_status != -1) {
          $links[] = '<a href="' . $url . '&op=status&value=-1" method="POST" class="confirmajax">' . t('og', '加入黑名单') . '</a>';
        }

        if ($o->user_status == 0) {
          $links[] = '<a href="' . $url . '&op=status&value=1" method="POST" class="confirmajax">' . t('og', '批准加入') . '</a>';
        }

        $links[] = '<a href="' . $url . '&op=delete" method="POST" class="confirmajax">' . t('og', '删除成员') . '</a>';

        $data = dd_imp($links);

        if ($o->user_status == 0) {
          $data .= '<p class="center og_settings_user_message">' . t('og', '验证消息: %text', array('%text' => ($o->value ? $o->value : t('og', '无')))) . '</p>';
        }

      } else {
        $data = '<span class="red">' . t('og', '组长') . '</span>';
      }
      
      $table[] = array(
        theme('username', $o),
        format_date($o->timestamp),
        $status[$o->user_status],
        $data,
      );
    }
    
    $output = theme('table', $header, $table, array('class' => 'og_settings_users'));
    $output .= pager_view();
  }
  return $output;
}

/**
 * 小组话题自定义属性 
 * @param object $og 
 * @access protected
 * @return string
 */
function _og_settings_field($og) {
  // 防止非法访问
  if (!empty($_GET['field_field_id']) && $field = field_get_cache_field($_GET['field_field_id'])) {
    if ($field->type != 'ext_node_' . $og->oid) dd_get_access();
  }

  $url = $og->path . '/settings/field';
  
  dd_jqui(); 
  dd_add_js(dd_get_path('module', 'system') . '/admin.js');

  $tabs[] = l('字段管理', $url);
  $tabs[] = l('字段添加', $url . '/add');
  $tabs[] = l('字段导出', $url . '/export');

  dd_set_sub_tabs($tabs); 

  switch (arg(4)) {
    case 'add':
      $field = new stdClass;
      $field->module = 'og';
      $field->type = 'ext_node_' . $og->oid;
      $field->redirect = url('group/' . $og->oid . '/settings/field');
      $output = dd_get_form('field_form', $field);
    break;
    
    case 'export':
      $output = field_export(0, 'og', 'ext_node_' . $og->oid);
    break;
    
    default:

      $output = '<h2 class="og_settings_fields_title">自定义字段</h2>';
      $output .= field_lists('og', 'ext_node_' . $og->oid);
      
      if (empty($_GET['field_field_id'])) {
        if ($fields = field_get_cache_field_group('og', 'default')) {  
          $header = array(
            t('field', '名称'),
            t('field', '类型'),
            t('field', '必需'),
          );
   
          $types = field_get_call_lists();

          foreach ($fields as $field) {
            $table[] = array(
              $field->name,
              $types[$field->field_type]['title'],
              $field->required ? '是' : '否',
            );
          }

          $output .= '<h2 class="og_settings_fields_title">系统字段</h2>';
          $output .= theme('table', $header, $table, array('class' => 'center_table'));
        }
      }

  }

  return $output;
}
