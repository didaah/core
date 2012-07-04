<?php
// $Id$

/**
 * @file
 *  按模块生成语言翻译文件
 *  终端命令示例：
 *    为 www.didaah.org 站点下 system 模块生成英文语言包
 *    翻译语言文件将保存为 {module_name}/languages/{lang}.php，已有翻译语言文件将被覆盖，但原有翻译字符串将被保留
 *    php scripts/common.inc --host www.didaah.org --bootstrap full --script_name lang_export.inc --module_name system --lang en
 *
 * @TODO js 中的多语言待确定
 */

require_once DIDA_ROOT . '/modules/system/system.admin.inc';

if (!empty($_args['lang'])) {
  if (!empty($_args['module_name'])) {
    _lang_export($_args['lang'], $module);
  } else if ($fetch = db_query('SELECT * FROM {system}')) {
    foreach ($fetch as $o) {
      _lang_export($_args['lang'], $o->filename);
    }
  }
} else {
  message('请指定等导出的语言');
}

function _lang_export($lang_code, $module) {
  global $_args;

  if (!$lang_name = system_get_languages_data($lang_code)) {
    message('语言不存在');
  } else if (!$path = db_query('SELECT path FROM {system} WHERE filename = ?', array($module), array('return' => 'column'))) {
    message('模块或主题不存在');
  } else {
    
    //$path = DIDA_ROOT . '/' . $path;
    
    message('正在导出模块 - ' . $module);

    message('正在导出语言 - ' . $lang_name . '(' . $lang_code . ')');
     
    $path = DIDA_ROOT . '/' . $path;
    
    $files = _lang_export_files($path);

    // system 模块，加上系统核心文件
    if ($_args['module_name'] == 'system') {
      $files = _lang_export_files(DIDA_ROOT . '/includes', $files);
    }

    if (!empty($files)) {
      
      include_once $path . '/' . $module . '.info';

      // 截入已有翻译文件
      
      $lang_file = $path . '/languages';

      if (!is_dir($lang_file) && !mkdir($lang_file, 0777)) {
        message($lang_file . '创建失败，无法自动保存翻译文件');
      }

      $lang_file .=  '/' . $lang_code . '.php'; 

      if (is_file($lang_file)) {
        include_once $lang_file;
      } else {
        // 所有界面待翻译字符串
        $lang = array();
      }

      // 模块界面默认语言，若无，则为中文简体
      $default_lang = !empty($info['language']) ? $info['language'] : 'zh-hans';

      foreach ($files as $file) {
        message('正在分析文件 - ' . $file);
        if ($string = file_get_contents($file)) {

          $mat = $mat2 = array();

          $string = str_replace(array('\\"', "\\'"), array('[&__a]', '[&__b]'), $string);

          preg_match_all('/[^\'|"]t\(.{1,4}' . $module . '[\'|"].{1,3}\'(.*?)\'/ms', $string, $mat); 
          preg_match_all('/[^\'|"]t\(.{1,4}' . $module . '[\'|"].{1,3}"(.*?)"/ms', $string, $mat2);
          
          if (!empty($mat2[1])) {
            $mat[1] = array_merge($mat[1], $mat2[1]);
          }

          foreach ($mat[1] as $value) {
            // @TODO 需要实现从数据库中导出已完成的翻译
            // 若已有，则跳过
            if (empty($lang[$value])) {
              $value = str_replace(array('[&__a]', '[&__b]'), array('"', "'"), $value);
              $lang[$value] = $value;
            }
          }
        }
      }

      if (!empty($lang)) {
        $output = '<?php';
        $output .= "\n";
        $output .= '// $Id$';

        $message = "\n\n/**\n";
        $message .= ' * 默认语言：' . system_get_languages_data($default_lang) . "($default_lang)\n";
        $message .= ' * 翻译语言：' . $lang_name . '(' . $lang_code . ")\n";
        $message .= ' * 共有 ' . count($lang) . " 条翻译\n";
        $message .= " */\n\n";

        $output .= $message;
        $output .= '$lang = ' . var_export($lang, true) . ";\n";
        
        
        message($message);

        if (file_put_contents($lang_file, $output)) {
          message('已保存至 ' . $lang_file);
        } else {
          message('翻译文件未成功保存至 ' . $lang_file);
        }
      } else {
        message('没有需要翻译的字符串');
      }
      
    } else {
      message('没有需要翻译的字符串');
    }
  }
}

/**
 * 获取指定目录下所有待翻译的文件 
 * @param string $dir 
 *  目录
 * @param array $files 
 * @access protected
 * @return array
 */
function _lang_export_files($dir, &$files = array()) {

  if (is_dir($dir)) {
    if ($objs = glob($dir.'/*')) {
      foreach ($objs as $obj) {
        if (is_dir($obj)) {
          _lang_export_files($obj, $files);
        } else if (in_array(end(explode('.', $obj)), array('module', 'install', 'info', 'inc', 'php'))) {
          $files[] = $obj;
        }
      }
    }
  }

  return $files;
}
