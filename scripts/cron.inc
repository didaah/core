<?php
// $Id$

/**
 * 计划任务统一管理脚本，请将此脚本加入到系统计划任务中，间隔时间自行控制，建议为 1-3 分钟。
 * 如：* * * * * php $path/scripts/cron.inc --host www.didaah.org
 */

$is_cron = true;

require_once 'common.inc';

// protected _cron_database() {{{ 
/**
 * 读取 cron 表中的任务，每次 10 条
 * 
 * @access protected
 * @return void
 */
function _cron_database() {
  if ($fetch = db_query('SELECT * FROM {cron} WHERE status = 0 
  ORDER BY weight ASC, cid ASC', NULL, array('limit' => 10))) {
    foreach ($fetch as $o) {
      if (!$o->data) continue;
      
      $data = unserialize($o->data);
      
      if ($data['includes']) {
        foreach ($data['includes'] as $filepath) {
          include_once $filepath;
        }
      }
      if (function_exists($data['func']) && call_user_func_array($data['func'], $data['args'])) {
        db_exec('UPDATE {cron} SET status = 1 WHERE cid = ?', array($o->cid));
        if ($data['success'] && function_exists($data['success'])) {
          call_user_func_array($data['success'], $data['args']);
        }
      }
      
    }
  }
}
// }}}

// protected _cron_function() {{{ 
/**
 * 执行 hook_cron() 中定义的计划任务
 * _cron_function
 * 
 * @access protected
 * @return void
 */
function _cron_function() {
  // 获得上一次执行时间
  $timestamp = var_get('cron_last_time', 0);

  /**
   * 获得最小执行时间，默认为 3600 秒，防止频繁执行
   * 触发 hook_cron() 时，将遵循管理员设置的时间间隔
   */
  $time_min = var_get('cron_min_time', 3600);

  if (!empty($time_min) && $timestamp > (time() - $time_min)) {
    return;
  }

  // 触发 hook_cron()
  module_invoke_all('cron', $timestamp);

  // 写入运行时间
  var_set('cron_last_time', time());

  // 写入日志
  dd_log('cron', t('system', '成功运行了计划任务'));
}
// }}}

// protected _cron_get_php_ps() {{{ 
/**
 * 读取系统中所有正在执行的 php 进程，筛选出正在执行的计划任务
 * _cron_get_php_ps
 * 
 * @access protected
 * @return void
 */
function _cron_get_php_ps() {
  exec('ps aux', $data);

  $header = array_shift($data);

  $process = array();

  $keys = array_filter(explode(' ', strtolower($header)));

  if (empty($data)) return false;

  foreach ($data as $text) {
    if (strpos($text, 'is_dida_cron') === false) continue;
    $rows = array();
    foreach ($keys as $key) {
      $count = strpos($text, ' ');
      if ($key != 'command') {
        $rows[$key] = substr($text, 0, $count);
        $text = trim(substr($text, $count));
      } else {
        $rows[$key] = $text;
      }
    }
    $process[$rows['command']] = $rows;
  }
  
  return $process;
}
// }}}

// protected _cron_jobs() {{{ 
/**
 * 加载所有模块中 cronjobs.inc 文件，触发 hook_cronjobs()
 * _cron_jobs
 * 
 * @access protected
 * @return void
 */
function _cron_jobs() {
  global $_args;

  module_include_file('cronjobs.inc');

  $cronjobs = _module_invoke_all('cronjobs');

  // 获取所有 php 进程，找出正在执行的任务
  if ($process = _cron_get_php_ps()) {
    $count = count($process);
    if ($count > 20) {
      dd_log('cron', t('system', '已有 !count 个任务正在执行中', array('!count' => $count)));
      return;
    }
  }

  // 所有任务上一次的执行时间
  if ($cache = cache_get('dida_cron_last_timestamps')) {
    $timestamps = $cache->data;
  } else {
    $timestamps = array();
  }

  if (empty($cronjobs)) return false;

  $year = date('Y');
  $month = date('n');
  $day = date('j');
  $hour = date('G');
  $minute = date('i');

  foreach ($cronjobs as $_module => $jobs) {
    foreach ($jobs as $key => $data) {

      $module = !empty($data['module']) ? $data['module'] : $_module;
      $command = 'php ' . DIDA_ROOT . '/scripts/common.inc --bootstrap full --host ' . $_args['host'];
      $command .= ' --is_dida_cron 1 --module_scripts ' . $module . ' --script_name '  . $data['script'];
      if (!empty($data['param'])) {
        $command .= ' ' . $data['param'];
      }
      
      // 上一次执行还没有结束
      if (!empty($process[$command])) {
        // 检查已耗费的时间
        if (!empty($timestamps[$command])) {
          $time = time()-$timestamps[$command];
          // 已超时，终止进程
          if (!empty($data['timeout']) && $time > $data['timeout']) {
            exec('kill -9 ' . $process[$command]);
            dd_log('cron', t('wb', '已终止超时任务 !command，pid：!pid', array('!command' => $command, '!pid' => $process[$command]['pid'])));
          } else if ($time > 3600) {
            dd_log('cron', t('wb', '任务 !command 已运行超过 !time 秒了，pid：!pid', array('!command' => $command, '!time' => $time, '!pid' => $process[$command]['pid'])));
          }
        }
      } else {
        // 检查是否需要执行
        $is_command = false;

        // @TODO 年月不允许使用 */3 这种格式
        if ($data['year'] != '*' && $data['year'] != $year) continue;
        if ($data['month'] != '*' && $data['month'] != $month) continue;

        $id = '#';
        if (empty($timestamps[$command])) {
          // 上次执行时间戳为 0，则需要执行
          $is_command = true;
        } else {

          // 03 -> 3
          foreach (array('day', 'month', 'minute') as $key) {
            if (is_numeric($data[$key])) {
              $data[$key] = intval($data[$key]);
            }
          }

          /**
           *  若 $data['day'] 为数字，且设定日期(年月日)小于当前日期，表明可能已错过执行时间
           *  则判断上次执行年月日是否小于设定日期，若小于，需要立即执行
           *  例如：模块要求 16 号执行，但今天已是 17 号，于是判断上一次执行时间，若小于 16 号，则需立即执行
           */
          if (is_numeric($data['day']) && $data['day'] < $day && date('Ynj', $timestamps[$command]) < intval($year . $month . $data['day'])) {
            $is_command = true;
            $id = '#1';
          } else if (strpos($data['day'], '/') !== false && $timestamps[$command] < (time()-preg_replace('/[^0-9]/', '', $data['day'])*86400)) {
            // */3 格式的参数，假如每 3 天执行一次，则判断上次执行时间是否为 3 天之前，若是，需立即执行
            // 可能带来数分钟的误差
            $is_command = true;
            $id = '#2';
          } else if ($data['day'] == '*' || $data['day'] == $day) { 
            /**
             *  若 $data['hour'] 为数字，且设定日期(年月日小时)小于当前日期，表明可能已错过执行时间
             *  则判断上次执行年月日小时是否小于设定日期，若小于，需要立即执行
             *  例如：模块要求 16 号 13 点执行，但今天已是 17 号，于是判断上一次执行时间，若小于 16 号 13 点，则需立即执行
             */
            if (is_numeric($data['hour']) && $data['hour'] < $hour && 
            date('YnjG', $timestamps[$command]) < intval($year . $month . $day . $data['hour'])) {
              $is_command = true;
              $id = '#3';
            } else if (strpos($data['hour'], '/') !== false && $timestamps[$command] < (time()-preg_replace('/[^0-9]/', '', $data['hour'])*3600)) {
              // */3 格式的参数，假如每 3 小时执行一次，则判断上次执行时间是否为 3 小时之前，若是，需立即执行
              // 可能带来数分钟的误差
              $is_command = true;
              $id = '#4';
            } else if ($data['hour'] == '*' || $data['hour'] == $hour) {
              // 每分钟或当前分钟需执行，判断上次执行时间分钟数是否与当前相同，相同则表示已经执行过
              // 通常情况不需要此判断，因为 cron.inc 的执行间隔不可能小于 1 分钟
              if (($data['minute'] == '*' || $data['minute'] == $minute) && date('i', $timestamps[$command]) != $minute) {
                $is_command = true;
                $id = '#5';
              } else if (strpos($data['minute'], '/') !== false && $timestamps[$command] < (time()-preg_replace('/[^0-9]/', '', $data['minute'])*60)) {
                // */3 格式的参数，假如每 3 分钟执行一次，则判断上次执行时间是否为 3 分钟之前，若是，需立即执行
                // 可能带来1分钟的误差，取决于 cron.inc 的执行间隔
                $is_command = true;
                $id = '#6';
              } else if (is_numeric($data['minute']) && $data['minute'] < $minute && $timestamps[$command] < (time()-3600)) {
                // 上次执行在 1 小时之前，需立即执行
                $is_command = true;
                $id = '#7';
              }
            }
          }
        }
      }

      if ($is_command) {
        // 立即执行
        $timestamps[$command] = time(); // 设置执行时间
        exec($command . ' > /dev/null &');
        dd_log('cron', t('system', '任务 !command 开始执行(!id)', array('!command' => $command, '!id' => $id)));
      }
    }
  }

  // 写入所有任务上次执行的时间戳
  cache_set('dida_cron_last_timestamps', $timestamps);
}
// }}}

_cron_database();

_cron_function();

_cron_jobs();

dd_log('cron', t('system', '成功运行了 cron.inc'));
