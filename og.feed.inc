<?php
// $Id$

/**
 * 全站或小组最新话题 rss
 */
function og_get_feed_node($oid = NULL) {
  global $conf, $base_url;
  
  $title = '';
  
  if ($oid) {
    $fetch = db_query('SELECT n.title, n.nid, n.comment_count, n.body, n.node_status, n.uid, 
    n.updated, n.format, n.created, o.og_name, o.alias, r.name FROM {og} o 
    INNER JOIN {og_node} n ON o.oid = n.oid 
    INNER JOIN {users} r ON n.uid = r.uid WHERE o.oid = ? 
    AND n.node_status > 0 AND o.status = 1 
    AND o.type > 0 ORDER BY n.nid DESC',
    array($oid), array('limit' => 10));
  } else {
    $fetch = db_query('SELECT n.title, n.nid, n.comment_count, n.body, n.uid, n.updated,
    n.created, o.og_name, o.alias, r.name FROM {og} o INNER JOIN {og_node} n ON o.oid = n.oid 
    INNER JOIN {users} r ON n.uid = r.uid WHERE n.node_status > 0 AND o.status = 1 AND o.type > 0 
    ORDER BY n.nid DESC', NULL, array('limit' => 10));
    
    $link = $base_url;
    $title = t('og', '最新话题');
  }
  
  if ($fetch) {
    foreach ($fetch as $o) {
      if (!$title) {
        $link = url('group/'.$o->alias, array('absolute' => 1));
        $title = t('og', '!string 小组的最新话题', array('!string' => $og->og_name));
      }
      
      $o->path = url('group/node/'.$o->nid, array('absolute' => 1));
      
      $info = '<p align="right">';
      
      $info .= dd_imp(array(
        t('og', '!name 发布于 !og', array('!name' => $o->name, '!og' => $o->og_name)),
        l(t('og', '!count 条评论，加入讨论&gt&gt', array('!count' => $o->comment_count)), $o->path.'#og_node_comment_wrapper')
      ));
      
      $info .= '</p>';
      
      $items .= dd_rss_item(
        array(
          'title' => check_plain($o->title),
          'description' => filter_view(dd_get_summary($o->body, 1000, $o->format), $o->format).'...' . $info,
          'link' => $o->path,
          'comments' => $o->path.'#og_node_comment_wrapper'
        ), 'item',
        array(
          array('key' => 'pubDate', 'value' => gmdate('r', $o->created)),
          array('key' => 'dc:creator', 'value' => $o->name),
          array('key' => 'guid', 'value' => $o->nid . ' at ' . $base_url, 'attributes' => array('isPermaLink' => 'false'))
        )
      );
    }
  } else {
    $items = t('og', '没有数据');
  }
  
  return dd_feed_wrapper($items, $title, $link);
}

/**
 * 全站或小组或单篇话题的最新评论 rss
 */
function og_get_feed_comment($oid = NULL, $nid = NULL) {
  global $conf, $base_url;
  
  $title  = '';
  
  if ($oid) {
    $og = og_load($oid);
    if (!og_access_view($og)) {
      return dd_feed_wrapper(t('og', '没有权限访问'), t('og', '没有权限访问'));
    }
    
    $fetch = db_query('SELECT c.ext_id, c.created, c.title AS subject, c.name, c.cid, c.body, c.format, n.title 
    FROM {comment} c INNER JOIN {og_node} n ON c.ext_id = n.nid WHERE n.oid = ? 
    AND c.ext_type = ? AND c.status > 0 AND n.node_status > 0
    ORDER BY c.created DESC', array($oid, 'og'), array('limit' => 10));
    
  } else if ($nid) {
    $node = og_node_load($nid);
    if (!og_access_node_view($node)) {
      return dd_feed_wrapper(t('og', '没有权限访问'), t('og', '没有权限访问'));
    }
    
    $fetch = db_query('SELECT c.ext_id, c.created, c.title AS subject, c.name, c.cid, c.body, c.format FROM {comment} c WHERE c.ext_id = ? 
    AND c.ext_type = ? AND c.status > 0 ORDER BY c.created DESC', array($nid, 'og'), array('limit' => 50));
    
  } else {
    
    $fetch = db_query('SELECT c.ext_id, c.created, c.title AS subject, c.name, c.cid, c.body, c.format, n.title 
    FROM {comment} c INNER JOIN {og_node} n ON c.ext_id = n.nid INNER JOIN {og} o ON n.oid = o.oid
    WHERE c.ext_type = ? AND c.status > 0 AND n.node_status > 0 AND o.status = 1 
    AND o.type > 0 ORDER BY c.created DESC', array('og'), array('limit' => 10));
    
    $link = url('group/all', array('absolute' => 1));
    $title = t('og', '小组最新评论');
  }
  
  if ($fetch) {
    foreach ($fetch as $o) {
      $path = url('group/node/'.$o->ext_id, array('absolute' => 1));
      
      if (!$title) {
        if ($oid) {
          $title = t('og', '!string 小组的最新评论', array('!string' => $og->og_name));
          $link = url($og->alias, array('absolute' => 1));
        } else if ($nid) {
          $link = $path;
          $title = t('og', '!string 的最新评论', array('!string' => $node->title));
        }
      }
      
      $info = '<p align="right">';
      
      $info .= dd_imp(array(
        t('og', '!name 评论于 !time', array('!name' => $o->name, '!time' => format_date($o->created))),
        l(t('og', '加入讨论&gt&gt'), $path.'#comment_form')
      ));
      
      $info .= '</p>';
      
      $items .= dd_rss_item(
        array(
          'title' => '#'.$o->subject,
          'description' => filter_view(dd_get_summary($o->body, 500, $o->format), $o->format).'...' . $info,
          'link' => $path.'#comment_og_'.$o->cid,
          'comments' => $path.'#comment_form'
        ), 'item',
        array(
            array('key' => 'pubDate', 'value' => gmdate('r', $o->created)),
            array('key' => 'dc:creator', 'value' => $o->name),
            array('key' => 'guid', 'value' => $o->cid . ' at ' . $base_url, 'attributes' => array('isPermaLink' => 'false'))
         )
      );
    }
  } else {
    $items = t('og', '没有数据');
  }
  
  return dd_feed_wrapper($items, $title, $link);
}