<?php
// $Id$

/**
 * 小组节点表单
 * @param (object) $node
 *  节点对象，其中 $node->og 为小组对象
 */
function og_node_form($node) {
  global $user;

  $types = og_og_node_info();
  
  if (!$types[$node->node_type]) {
    echo dd_get_access();
    exit;
  }
  
  if (!$node->nid) {
    $title = t('og', '发表!title', array('!title' => $types[$node->node_type]['title']));
  } else {
    $title = t('og', '编辑!title', array('!title' => $types[$node->node_type]['title']));
  }
  
  $form['fields']['node_type'] = array(
    '#type' => 'hidden',
    '#default_value' => $node->node_type,
    '#constant' => 1
  );
  $form['fields']['nid'] = array(
    '#type' => 'hidden',
    '#default_value' => $node->nid,
    '#constant' => 1
  );
  $form['fields']['oid'] = array(
    '#type' => 'hidden',
    '#default_value' => $node->og->oid,
    '#constant' => 1
  );
  $form['fields']['uid'] = array(
    '#type' => 'hidden',
    '#default_value' => $GLOBALS['user']->uid,
    '#constant' => 1
  );
  $form['settings'] = array(
    '#values_object' => true,
    '#validate' => array('og_node_form_validate'),
    '#ajax_validate' => 1,
  );
    
  if ($types[$node->node_type]['default_form']) {
    $form['fields']['title'] = array(
      '#title' => t('og', '标题'),
      '#type' => 'textfield',
      '#default_value' => $node->title,
      '#required' => 1,
      '#weight' => -100,
      '#attributes' => array('maxlength' => 128, 'size' => 40)  
    );
    $form['fields']['body'] = array(
      '#title' => t('og', '内容'),
      '#type' => 'textarea',
      '#weight' => -2,
      '#default_value' => $node->body,
      '#required' => 1,
      '#attributes' => array('cols' => 60, 'rows' => 10)  
    );
   
    global $form_tokens;
  
    $file_url = url('files/editor', array('query' => 'op=markitup&name=upload&type=image&image_size=800x600'));

    if (!empty($node->nid)) {
      $file_url .= '&ext_type=og_node&ext_id2=' . $node->oid . '&ext_id=' . $node->nid;
    } else {
      // 内容提交保存时，以 $form_token 做为关联条件
      $file_url .= '&status=-1&ext_type=' . $form_tokens['og_node_form'];
    }
      
    if (var_get_key('og', 'node_editor', 1) == 1) {
      dd_get_editor_text('#og_node_form_type_body', 'html', array(
        'upload' => true,
        'filebrowserImageUploadUrl' => $file_url,
        'upload_image' => true,
      ));
    } else if (var_get_key('og', 'node_editor', 1) == 2) {
      dd_get_editor('og_node_form_type_body', array(
        'toolbar' => 'medium',
        'upload_image' => 1,
        'filebrowserImageUploadUrl' => str_replace('markitup', 'ckeditor', $file_url),
        'height' => 200, 
      ));
    }
    
    $form['fields']['format'] = filter_form_field($node->format);
    
    $form['fields']['redirect'] = og_get_redirect_field();

    $form['fields']['submit'] = array(
      '#type' => 'submit',
      '#weight' => 100,
      '#submit' => 'og_node_form_submit'
    );
  }

  /**
   * 已启用新浪微博模块，并且设置里允许使用，则附加表单
   */
  if (module_exists('weibo')) {
    weibo_tweet_get_form_alter($form, 3);
  }

  $form['fields']['get_images'] = array(
    '#type' => 'radio',
    '#title' => '下载',
    '#default_value' => 0,
    '#options' => array('否', '是'),
    '#description' => '将内容中站外图片下载到本地'
  );
  
  field_get_node_form('og', 'ext_node_' . $node->oid, $node, $form);
  field_get_node_form('og', 'default', $node, $form);

  module_alter_all('og_node_form', $form, $node);
  
  dd_set_title(array($title));
  dd_set_breadcrumb(array(l(t('og', '我的小组'), $user->path.'/group'), l($node->og->og_name, 'group/'.$node->og->alias), $title));
  return $form;
}

function og_node_form_validate(&$form, &$v) {
  return og_node_validate($v['_dida_values']);
}

function og_node_form_submit(&$form, &$v) {
  if ($node = og_node_save($v['_dida_values'], false)) {
    if (!isset($form['settings']['#rediret'])) {
      $form['settings']['#redirect'] = og_get_redirect_path($v['_dida_values']->redirect, $node);
    }

    if (empty($v['_dida_values']->_db_write_record_is_update)) {
      // 新增时，将从编辑器上传的图片，关联到话题
      db_exec('UPDATE {files} SET ext_type = ?, ext_id = ?, ext_id2 = ?, status = 1 
      WHERE ext_type = ?', array('og_node', $node->nid, $node->oid, $form['#form_token']));
    }
 
    dd_set_message(t('og', '操作成功'));
  } else {
    dd_set_message(t('og', '操作失败，请重试'));
  }
}

/**
 * 修改节点状态
 * @param (object) $node
 */
function og_node_status($node) {
  $old_status = $node->node_status;
  if ($_GET['op'] == 'close') {
    $node->node_status = 2;
    $text = t('og', '!title 禁止回复，ID：!id', array('!title' => $node->title, '!id' => $node->nid));
    $url = url('group/node/'.$node->nid);
  } else if ($_GET['op'] == 'recycle') {
    $node->node_status = 0;
    $text = t('og', '!title 放入回收站，ID：!id', array('!title' => $node->title, '!id' => $node->nid));
    $url = url('group/'.$node->og->alias.'/settings/node');
  } else if ($_GET['op'] == 'open') {
    $node->node_status = 1;
    $text = t('og', '!title 打开评论，ID：!id', array('!title' => $node->title, '!id' => $node->nid));
    $url = url('group/node/'.$node->nid);
  } else {
    dd_set_message(t('og', '错误的请求'));
    dd_goto('group/node/'.$node->nid);
  }
  
  if ($old_status == $node->node_status) {
    dd_set_message(t('og', '操作成功'));
    dd_goto('group/node/'.$node->nid);
  }
  
  if (db_exec('UPDATE {og_node} SET node_status = ? WHERE nid = ?', array($node->node_status, $node->nid))) {
    if ($value == 0) {
      og_set_node_count($node->oid);
    }
    
    module_invoke_all('og_node_status', $node, $old_status);
    
    dd_log('og', $text, $node->oid, 'group/node/'.$node->nid);
    dd_set_message(t('og', '操作成功'));
  }
  
  dd_goto($url);
}
