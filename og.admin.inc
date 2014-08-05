<?php
// $Id$

function _og_admin_tabs() {
  dd_set_tabs(array(
    l(t('og', '小组列表'), 'admin/og'),
    l(t('og', '小组审核'), 'admin/og/status'),
    l(t('og', '话题管理'), 'admin/og/node'),
    l(t('og', '话题字段'), 'admin/og/node/fields'),
    l(t('og', '小组设置'), 'admin/og/settings'),
    l(t('og', '创建小组'), 'group/add'),
  ));
}

/**
 * 小组审核
 */
function og_admin_status() {
  if ($_GET['op'] == 'body') {
    if ($o = db_query('SELECT og_name, data, type FROM {og} WHERE oid = ? AND status = 0',
    array($_GET['oid']), array('return' => 'one'))) {
      
      if ($o->data) $data = unserialize($o->data);
      
      $item[] = $o->og_name;
      if ($data['des']) {
        $item[] = nl2br($data['des']);
      }
      
      echo theme('print', theme('item_list', $item));
      exit;
    }
  } else if ($_GET['op'] == 'ok' && db_exec('UPDATE {og} SET status = 1 WHERE oid = ? AND status = 0',
  array($_GET['oid']))) {
    echo 1;
    exit;
  }
  
  _og_admin_tabs();
  
  if ($fetch = pager_query('SELECT o.og_name, o.oid, o.uid, o.created, u.name FROM {og} o INNER JOIN
   {users} u ON o.uid = u.uid WHERE o.status = 0 ORDER BY o.oid ASC', 20)) {
    foreach ($fetch as $o) {
      $table[] = array(
        array('data' => l($o->og_name, $_GET['q'],
          array('query' => 'iframe=true&op=body&oid='.$o->oid, 'attributes' => array('class' => 'dialog'))),
          'class' => 'og_admin_td_title'
        ),
        theme('username', $o), format_date($o->created),
        l(t('og', '通过'), $_GET['q'], array('query' => 'op=ok&oid='.$o->oid, 'attributes' => array('level' => 'two', 'class' => 'confirmajax')))
      );
    }
    
    $output = theme('table', array(t('og', '名称'), t('og', '用户'), t('og', '时间'), t('og', '时间')), $table, array('class' => 'og_admin_table'));
    $output .= pager_view();
    
    dd_jqui();
  } else {
    $output = system_no_content(t('og', '没有需要审核的小组'));
  }
  
  dd_add_js(dd_get_path('module', 'og').'/og.js');
  dd_add_css(dd_get_path('module', 'og').'/og.css');
  return $output; 
}

/**
 * 话题通用字段 
 * @access public
 * @return string
 */
function og_admin_node_fields() {
  _og_admin_tabs();
  
  dd_set_sub_tabs(array(
    l('字段管理', 'admin/og/node/fields'),
    l('字段添加', 'admin/og/node/fields/add'),
    l('字段导出', 'admin/og/node/fields/export'),
  ));

  switch (arg(4)) {
    case 'add':
      $field->module = 'og';
      $field->type = 'default';
      $field->redirect = url('admin/og/node/fields');
      $output = dd_get_form('field_form', $field);
    break;
    
    case 'export':
      $output = field_export(0, 'og', 'default');
    break;
    
    default:
      $output = field_lists('og', 'default');
  }

  return $output;
}

/**
 * 节点管理
 */
function og_admin_node() {
  if ($_GET['op'] == 'status') {
    
    if ($_POST['value'] == 2) {
      $text = t('og', '禁止回复');
    } else if ($_POST['value'] == 0) {
      $text = t('og', '删除文章');
    } else if ($_POST['value'] == 1) {
      $text = t('og', '恢复文章');
    } else {
      echo -1;
      exit;
    }
    
    if ($oid = db_query('SELECT oid FROM {og_node} WHERE nid = ? AND node_status != ?',
    array($_GET['nid'], $_POST['value']), array('return' => 'column'))) {
      if (db_exec('UPDATE {og_node} SET node_status = ? WHERE nid = ?', array($_POST['value'], $_GET['nid']))) {
        og_set_node_count($oid);
        dd_log('og', $text, $oid);
        echo 'two';
      }
    }
    exit;
  }
  
  _og_admin_tabs();
  
  $header = array(
    t('og', '标题'),
    array('data' => t('og', '小组'), 'field' => 'n.oid'),
    array('data' => t('og', '作者'), 'field' => 'n.uid'),
    array('data' => t('og', '回应'), 'field' => 'n.comment_count'),
    array('data' => t('og', '最后更新'), 'field' => 'n.updated', 'sort' => 'desc'),
    t('og', '操作')
  );
  
  dd_set_sub_tabs(array(
    l(t('og', '开放的主题'), 'admin/og/node'),
    l(t('og', '禁止回复的主题'), 'admin/og/node/a'),
    l(t('og', '回收站'), 'admin/og/node/b'),
  ));
  
  switch (arg(3)) {
    case 'a':
      $status = 2;
    break;
    case 'b':
      $status = 0;
    break;
    default:
      $status = 1;
  }
  
  if ($fetch = pager_query('SELECT n.title, n.nid, n.comment_count, n.node_status, n.uid, n.updated, u.name, 
  o.og_name, o.alias FROM {og_node} n INNER JOIN {users} u ON n.uid = u.uid INNER JOIN {og} o ON n.oid = o.oid 
  WHERE n.node_status = ?'.table_sql($header), 20, 0, NULL, array($status))) {
    foreach ($fetch as $o) {
      $table[] = array(
        array(
          'data' => l($o->title, 'group/node/'.$o->nid),
          'class' => 'og_admin_td_title'
        ),
        l($o->og_name, 'group/'.$o->alias),
        theme('username', $o), $o->comment_count, format_date($o->updated),
        dd_form_select(
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
      );
    }
    
    $output = theme('table', $header, $table, array('class' => 'og_admin_table'));
    $output .= pager_view();
  } else {
    $output = system_no_content();
  }
  
  dd_add_js(dd_get_path('module', 'og').'/og.js');
  dd_add_css(dd_get_path('module', 'og').'/og.css');
  return $output;
}

/**
 * 小组管理
 */
function og_admin_view() {
  if ($_GET['op'] == 'status') {
    
    if ($_POST['value'] == -1) {
      $text = t('og', '屏蔽小组');
    } else if ($_POST['value'] == 0) {
      $text = t('og', '审核不通过');
    } else if ($_POST['value'] == 1) {
      $text = t('og', '审核通过');
    } else {
      echo -1;
      exit;
    }
    
    if ($oid = db_query('SELECT oid FROM {og} WHERE oid = ? AND status != ?',
    array($_GET['oid'], $_POST['value']), array('return' => 'column'))) {
      if (db_exec('UPDATE {og} SET status = ? WHERE oid = ?', array($_POST['value'], $oid))) {
        dd_log('og', $text, $oid);
      }
    } else {
      echo -1;
    }
    exit;
  }
  
  _og_admin_tabs();
  
  $header = array(
    t('og', '名称'),
    array('data' => t('og', '组长'), 'field' => 'o.uid'),
    array('data' => t('og', '成员'), 'field' => 'o.user_count'),
    array('data' => t('og', '话题'), 'field' => 'o.node_count'),
    array('data' => t('og', '创建时间'), 'field' => 'o.oid', 'sort' => 'desc'),
    array('data' => t('og', '状态'), 'field' => 'o.status'),
  );
  
  if ($fetch = pager_query('SELECT o.*, u.name FROM {og} o INNER JOIN
   {users} u ON o.uid = u.uid'.table_sql($header), 20)) {
    foreach ($fetch as $o) {
      $table[] = array(
        array('data' => l($o->og_name, 'group/'.$o->alias), 'class' => 'og_admin_td_title'),
        theme('username', $o), $o->user_count, $o->node_count, format_date($o->created),
        dd_form_select(
          array(
            '#value' => $o->status,
            '#attributes' => array(
              'class' => 'changeconfirm dd_form_ajax_field',
              'href' => url($_GET['q'], array('query' => 'op=status&oid='.$o->oid))
            ),
            '#name' => 'og_admin_status_'.$o->oid,
            '#options' => array(0 => t('og', '待审核'), 1 => t('og', '正常'), -1 => t('og', '屏蔽'))
          )
        )
      );
    }
    
    $output = theme('table', $header, $table, array('class' => 'og_admin_table'));
    $output .= pager_view();
    
    dd_jqui();
  } else {
    $output = system_no_content(t('og', '没有小组'));
  }
  
  dd_add_css(dd_get_path('module', 'og').'/og.css');
  return $output; 
}

/**
 * 小组设置
 */
function og_admin_settings() {
  _og_admin_tabs();
  
  $form['fields']['og']['status'] = array(
    '#type' => 'radio',
    '#title' => t('og', '小组审核'),
    '#options' => array(t('og', '需要审核'), t('og', '不需要审核')),
    '#required' => 1,
    '#default_value' => var_get_key('og', 'status', 0),
    '#description' => t('og', '新创建的小组，是否需要审核')
  );
  
  $form['fields']['og']['delete'] = array(
    '#type' => 'radio',
    '#title' => t('og', '小组解散'),
    '#options' => array(t('og', '不允许解散'), t('og', '允许解散')),
    '#required' => 1,
    '#default_value' => var_get_key('og', 'delete', 0),
    '#description' => t('og', '是否允许组长解散小组，解散即删除，不可恢复')
  );
  
  $form['fields']['og']['ping'] = array(
    '#type' => 'radio',
    '#title' => t('og', 'ping 服务'),
    '#options' => array(t('og', '否'), t('og', '是')),
    '#required' => 1,
    '#default_value' => var_get_key('og', 'ping', 1),
    '#description' => t('og', '如有新增内容，通知更新服务器')
  );
   
  $form['fields']['og']['node_status'] = array(
    '#type' => 'radio',
    '#title' => t('og', '主题审核'),
    '#options' => array(t('og', '否'), t('og', '是')),
    '#required' => 1,
    '#default_value' => var_get_key('og', 'node_status', 0),
    '#description' => t('og', '游客发布主题时需要审核')
  );
  
  $form['fields']['og']['default_logo'] = array(
    '#type' => 'textfield',
    '#title' => t('og', '默认图标'),
    '#required' => 1,
    '#default_value' => var_get_key('og', 'default_logo', 'misc/images/logo.png'),
    '#description' => t('og', '若小组没有上传图标，默认显示此图，可以是站外 URL')
  );
  
  $form['fields']['og']['name_max'] = array(
    '#type' => 'textfield',
    '#title' => t('og', '名称长度'),
    '#required' => 1,
    '#default_value' => var_get_key('og', 'name_max', 128),
    '#description' => t('og', '小组名称最大长度，只能是数字，不超过 128')
  );
   
  $form['fields']['og']['name_update'] = array(
    '#type' => 'radio',
    '#title' => t('og', '名称修改'),
    '#options' => array(t('og', '不允许修改'), t('og', '允许修改')),
    '#required' => 1,
    '#default_value' => var_get_key('og', 'name_update', 0),
    '#description' => t('og', '是否允许组长多次修改小组的名称')
  );
  
  $form['fields']['og']['des_max'] = array(
    '#type' => 'textfield',
    '#title' => t('og', '介绍长度'),
    '#required' => 1,
    '#default_value' => var_get_key('og', 'des_max', 1000),
    '#description' => t('og', '小组介绍最大长度，只能是数字，不宜过长')
  );
  
  $form['fields']['og']['alias_update'] = array(
    '#type' => 'radio',
    '#title' => t('og', '别名修改'),
    '#options' => array(t('og', '不允许修改'), t('og', '允许修改')),
    '#required' => 1,
    '#default_value' => var_get_key('og', 'alias_update', 1),
    '#description' => t('og', '是否允许组长多次修改小组的别名')
  );
  
  $form['fields']['og']['alias_disable'] = array(
    '#type' => 'textfield',
    '#title' => t('og', '禁用别名'),
    '#default_value' => var_get_key('og', 'alias_disable'),
    '#description' => t('og', '禁止使用的别名，多个以半角空格分隔，如：admin config')
  );
   
  $form['fields']['og']['field_admin'] = array(
    '#type' => 'radio',
    '#title' => t('og', '字段管理'),
    '#options' => array(t('og', '不允许'), t('og', '允许')),
    '#required' => 1,
    '#default_value' => var_get_key('og', 'field_admin', 0),
    '#description' => t('og', '是否允许组长管理字段')
  );
    
  $form['fields']['og']['admin_user'] = array(
    '#type' => 'radio',
    '#title' => t('og', '增加助手'),
    '#options' => array(t('og', '不允许'), t('og', '允许')),
    '#required' => 1,
    '#default_value' => var_get_key('og', 'admin_user', 0),
    '#description' => t('og', '是否允许组长增加或取消副组长')
  );
  
  if ($roles = user_roles_opt()) {
    $limit = var_get_key('og', 'role_limit');
    
    $form['fields']['og']['role_limit'] = array(
      '#fieldset_prefix' => 'asc',
      '#fieldset_legend' => t('field', '小组数量'),
      '#fieldset_suffix' => 1,
      '#description' => t('og', '允许创建的小组数量，留空或 0 则为不限制。用户首先必须拥有创建小组的权限')
    );
    
    foreach ($roles as $rid => $name) {
      $form['fields']['og']['role_limit'][$rid] = array(
        '#type' => 'textfield',
        '#title' => $name,
        '#default_value' => $limit[$rid],
      );
    }
    
  }
  
  $form['settings']['#validate'][] = 'og_admin_settings_validate';
  
  $form['fields']['submit'] = array(
    '#type' => 'submit',
    '#submit' => 'og_admin_settings_submit'
  );
  
  return $form;
}

function og_admin_settings_validate($form, $v) {
  if (!is_numeric($v['og']['name_max'])) {
    $t['og']['name_max']['#error'][] = t('og', '只能是数字');
  } else if (!is_numeric($v['og']['des_max'])) {
    $t['og']['des_max']['#error'][] = t('og', '只能是数字');
  } else if ($v['og']['role_limit']) {
    foreach ($v['og']['role_limit'] as $rid => $value) {
      if ($value && !is_numeric($value)) {
        $t['og']['role_limit'][$rid]['#error'][] = t('og', '只能是数字');
      }
    }
  }
  return $t;
}

function og_admin_settings_submit($form, $v) {
  var_set('og', $v['og']);
}
