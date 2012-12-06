<?php
// $Id$

/**
 * 字段浏览页面
 */
function field_page($field) {
  dd_set_title(array($field->name));
  dd_set_breadcrumb(array(l($field->name, 'fields/'.$field->field_id)));
  return module_invoke($field->module, 'field_view', $field);
}

/**
 * select、radio、selects、checkbox、tags类型浏览页面
 */
 
function field_term_page($term) {
  dd_set_title(array($term->name, $term->field->name));
  dd_set_breadcrumb(array(l($term->field->name, 'fields/'.$term->field->field_id), $term->name));
  return module_invoke($term->field->module, 'field_term_view', $term);
}

/**
 * 分类 rss
 */
function field_term_feed($term) {
  return module_invoke($term->field->module, 'field_term_feed', $term);
}

/**
 * 自动补全、多级联动等数据请求址
 */
function  _field_system_tools_ajax($v, $g) {
  dd_set_header('Content-Type: text/plain; charset=utf-8');
  $json['error'] = 1;
  
  switch ($g['op']) {
    case 'select':
      if ($v['__default_value']) {
        if ($fetch = db_query('SELECT tid, name FROM {fields_term} WHERE field_id = ? AND pid = ? 
          ORDER BY weight ASC, tid ASC', array($g['fid'], $v['__default_value']))) {
          foreach ($fetch as $o) {
            $json['contents'][] = array('tid' => $o->tid, 'name' => $o->name);
          }
          $json['error'] = 0;
        }
      }
    break;

    case 'autotag': // 标签提示
      if ($field = field_load($g['fid'])) {
        if (!empty($field->data['validate']['autocomplete']) && !empty($v['value'])) {
          $json['error'] = 0;

          $value = $v['value'];
          $prefix = '';

          if (strpos($value, ',') !== false || strpos($value, '，') !== false) {
            $arr = array_unique(explode(',', str_replace('，', ',', $value)));
            if (count($arr) < 2) {
              $json['error'] = 1;
              break;
            }

            $value = trim(array_pop($arr));

            if (empty($value)) break;

            $prefix = implode(',', $arr) . ',';
          }

            if ($fetch = db_query('SELECT tid, name FROM {fields_term} WHERE field_id = ? AND name LIKE ? 
            ORDER BY weight ASC, tid ASC', array($g['fid'], $value . '%'), array('limit' => 10))) {
              foreach ($fetch as $o) {
                if (empty($arr) || !in_array($o->name, $arr)) {
                  $json['contents'][] = $prefix . $o->name;
                }
              }
            }

        }
      }

    case 'auto': // 输入 ajax 验证
      if ($field = field_load($g['fid'])) {
        if ($field->data['validate']['type'] == 'custom_ok' && $field->data['validate']['settings']['values']) {
          $json['error'] = 0;
           switch ($field->data['validate']['settings']['value_type']) {
            case 1: // 普通模式
              $json['contents'] = dd_line_to_array($field->data['validate']['settings']['values'], 1);
            break;
            case 3: // php 模式
              $field_value = $v['value'];
              $field_type = 'autocomplete';
              $json['contents'] = eval($field->data['validate']['settings']['values']);
          }
        }
      }
  }
  
  return json_encode($json);
}
