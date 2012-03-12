<?php
// $Id$

/**
 * 创建小组
 */
function og_form($og = NULL) {
  global $user;
  
  if (empty($og->oid)) {
    if (!og_is_add($user)) {
      dd_set_message(t('og', '你创建的小组数量已经超过限制'));
      dd_goto($user->path . '/group');
    }
   
    dd_set_tabs(array(
      l(t('og', '我的主页'), 'user/' . $user->uid),
      l(t('og', '我的小组'), 'user/' . $user->uid . '/group'),
      l(t('og', '创建小组'), 'group/add')
    ));

    $form['fields']['status'] = array(
      '#type' => 'hidden',
      '#default_value' => var_get_key('og', 'status', 0),
      '#required' => 1,
      '#constant' => 1,
    );
  }
   
  $form['fields']['oid'] = array(
    '#type' => 'hidden',
    '#default_value' => $og->oid,
    '#required' => 1,
    '#constant' => 1,
  );
  
  $form['fields']['og_name'] = array(
    '#type' => 'textfield',
    '#title' => t('og', '小组名称'),
    '#required' => 1,
    '#default_value' => $og->og_name,
    '#weight' => -100,
    '#description' => t('og', '不超过 %string 个字符', array('%string' => var_get_key('og', 'name_max', 128)))
  );
  
  $form['fields']['data']['des'] = array(
    '#type' => 'textarea',
    '#title' => t('og', '小组介绍'),
    '#required' => 1,
    '#default_value' => $og->data['des'],
    '#weight' => -90,
    '#description' => t('og', '不超过 %string 个字符', array('%string' => var_get_key('og', 'des_max', 1000)))
  );

  $form['fields']['data']['alias_name'] = array(
    '#type' => 'textarea',
    '#title' => t('og', '其它名称'),
    '#weight' => -80,
    '#default_value' => $og->data['alias_name'],
    '#attributes' => array('rows' => 4),
    '#description' => t('og', '小组的其它名称，每行一个，不超过 %string 个', array('%string' => var_get_key('og', 'alias_name_max', 5)))
  ); 

  $form['fields']['type'] = array(
    '#type' => 'radio',
    '#title' => t('og', '身份验证'),
    '#default_value' => !isset($og->type) ? 1 : $og->type,
    '#options' => array(
      0 => t('og', '秘密小组'),
      1 => t('og', '任何人都可以直接加入'),
      2 => t('og', '只有通过审核才能加入'),
      3 => t('og', '只有获得邀请才能加入'),
    ),
    '#required' => 1,
    '#description' => t('og', '秘密小组及组内文章，只有成员可见，不会出现在任何公共列表中')
  );

  if (empty($og->access['node'])) {
    $og->access = array(
      'node' => array('anonymous' => 1, 'user' => 2, 'og_user' => 3),
      'comment' => array('anonymous' => 1, 'user' => 2, 'og_user' => 3),
    );
  }

  /**
   * 游客、组内成员、其它登录用户权限分配
   */
  $form['fields']['_node_access'] = array(
    '#fieldset_prefix' => 'asc',
    '#fieldset_legend' => t('og', '话题权限'),
    '#fieldset_suffix' => 1,
    '#fieldset' => true,
  );

  $form['fields']['_comment_access'] = array(
    '#fieldset_prefix' => 'asc',
    '#fieldset_legend' => t('og', '评论权限'),
    '#fieldset_suffix' => 1,
    '#fieldset' => true,
  );

  foreach (og_get_access_user_type() as $key => $title) {
    $form['fields']['_node_access'][$key] = array(
      '#type' => 'radio',
      '#title' => $title,
      '#default_value' => $og->access['node'][$key],
      '#options' => array(
        1 => t('og', '无权限'),
        2 => t('og', '可浏览'),
        3 => t('og', '可浏览和发布'),
      ),
      '#required' => 1,
    );
    
    $form['fields']['_comment_access'][$key] = array(
      '#type' => 'radio',
      '#title' => $title,
      '#default_value' => $og->access['comment'][$key],
      '#options' => array(
        1 => t('og', '无权限'),
        2 => t('og', '可浏览'),
        3 => t('og', '可浏览和发布'),
      ),
      '#required' => 1,
    );
  }

  $form['fields']['uid'] = array(
    '#type' => 'hidden',
    '#default_value' => isset($og->uid) ? $og->uid : $user->uid,
    '#required' => 1,
    '#constant' => 1,
  );
  
  $form['settings'] = array(
    '#validate' => array('og_form_validate'),
    '#ajax_validate' => 1,
  );
  
  $form['fields']['submit'] = array(
    '#type' => 'submit',
    '#weight' => 100,
    '#submit' => 'og_form_submit'
  );
  
  module_alter_all('og_form', $form);
  
  dd_add_css(dd_get_path('module', 'og').'/og.css');
  return $form;
}

function og_form_validate($form, &$v) {
  return og_validate($v['_dida_values']);
}

function og_form_submit(&$form, &$v) {
  if ($og = og_save($v['_dida_values'], false)) {
    
    if (!isset($form['settings']['#redirect'])) {
      if ($og->status == 1) {
        $form['settings']['#redirect'] = url($og->path);
      } else {
        $form['settings']['#redirect'] = url('user/'.$og->uid.'/group');
      }
    }
    
    dd_set_message(t('og', '创建成功'));
  } else {
    dd_set_message(t('og', '创建失败，请重试'));
  }
}
