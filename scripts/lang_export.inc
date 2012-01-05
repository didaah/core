<?php
// $Id$

/**
 * @file
 *  按模块生成语言翻译文件
 *  终端命令示例：
 *    为 www.cmsns.com 站点下 system 模块生成英文语言包
 *    翻译语言文件将保存为 {module_name}/languages/{lang}.php，已有翻译语言文件将被覆盖，但原有翻译字符串将被保留
 *    php common.inc --host www.cmsns.com --bootstrap data --script_name lang_export.inc --module_name system --lang en
 */

if (!empty($_args['lang'])) {
  
  require_once DIDA_ROOT . '/modules/system/system.admin.inc';
  
  if (!$lang_name = system_get_languages_data($_args['lang'])) {
    message('语言不存在');
  } else if (!$path = dd_get_path('module', $_args['module_name'])) {
    message('模块不存在');
  } else {

    message('正在导出模块 - ' . $_args['module_name']);

    message('正在导出语言 - ' . $lang_name . '(' . $_args['lang'] . ')');
     
    $path = DIDA_ROOT . '/' . $path;
    
    $files = _lang_export_files($path);

    // system 模块，加上系统核心文件
    if ($_args['module_name'] == 'system') {
      $files = _lang_export_files(DIDA_ROOT . '/includes', $files);
    }

    if (!empty($files)) {
      
      include_once $path . '/' . $_args['module_name'] . '.info';

      // 截入已有翻译文件
      
      $lang_file = $path . '/languages';

      if (!is_dir($lang_file) && !mkdir($lang_file, 0777)) {
        message($lang_file . '创建失败，无法自动保存翻译文件');
      }

      $lang_file .=  '/' . $_args['lang'] . '.php'; 

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
          $mat = array();
          preg_match_all('/[^\'|"]t\(.{1,4}' . $_args['module_name'] . '[\'|"].{1,3}[\'|"](.*?)[\)|\'|"]/ms', $string, $mat); 
          foreach ($mat[1] as $value) {
            // @TODO 需要实现从数据库中导出已完成的翻译
            // 若已有，则跳过
            if (empty($lang[$value])) {
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
        $message .= ' * 翻译语言：' . $lang_name . '(' . $_args['lang'] . ")\n";
        $message .= ' * 共有 ' . count($lang) . " 条界面翻译\n";
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
} else {
  message('请指定等导出的语言');
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
        } else if (in_array(end(explode('.', $obj)), array('module', 'install', 'info', 'inc', 'php', 'js'))) {
          $files[] = $obj;
        }
      }
    }
  }

  return $files;
}
