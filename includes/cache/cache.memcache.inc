<?php
//$Id$

if (cache_memcache_connect()) {
  /**
   * 缓存清除
   * @param (string) $type
   * 	操作类型：
   * 		cid - 按 cid 清除
   * 		all - 清空所有缓存表
   *    empty - 清空指定缓存表
   *    * - 按关键字模糊清除
   *    table - 清除指定表过期内容
   * @param (string) $cid
   * 	指定 cid
   * @param (string) $table
   *  指定缓存表
   */
  function cache_del($type = 'cid', $cid = NULL, $table = 'cache') {
    global $mem, $mem_prefix;
    switch ($type) {
      case 'cid':
        $mem->delete($mem_prefix . $table . '_' . $cid, 0);
      break;
      case 'all': // 全部清除
      case 'empty': // 清除表中所有数据
      case '*': // 模糊匹配
        /**
         *  @TODO 全部清除，非最佳处理方式
         *  当多个站点共用一个 memcache 实例时，会将其它站点缓存也清除掉
         */
        $mem->flush();
      break;
      case 'table':
        // 清除表中已到过期缓存，memcache 会自动清除
    }
  }

  /**
   * 获取缓存
   * @param (string) $cid
   * 	缓存 id
   * @param (string) $table
   * 	缓存表名
   * @return (object) $cache
   */
  function cache_get($cid, $table = 'cache') {
    global $mem, $mem_prefix;

    $cid = $mem_prefix . $table . '_' . $cid;
    
    if ($data = $mem->get(array($cid))) {
      $cache = new stdClass();
      if (isset($data[$cid])) {
        $cache->data = $data[$cid];
      }
      return $cache;
    }
  }

  /**
   * 写入缓存，若已存在，则更新
   * @param (string) $cid
   * 	缓存 id
   * @param (*) $data
   * 	待写入数据
   * @param (string) $table
   * 	缓存表名
   * @param (int) $expire
   * 	过期时间，从当前时间算起，以秒为单位
   */
  function cache_set($cid, $data, $table = 'cache', $expire = NULL) {
    global $mem, $mem_prefix, $mem_compress;
    
    if (empty($expire) || $expire > 2592000) {
      $expire = 2592000;
    }
    
    $cid = $mem_prefix . $table . '_' . $cid;

    return $mem->set($cid, $data, $mem_compress, $expire);
  }

} else {
  // 无法连接 memcache，使用默认缓存
  require_once DIDA_ROOT . '/includes/cache.inc';
}

/**
 * memcache 连接
 */
function cache_memcache_connect() {
  global $mem, $mem_prefix, $mem_compress;
  
  if (!isset($mem)) {
    $mem = new Memcache;
    $mem_prefix = var_get('cache_memcache_prefix', '');
    $mem_compress = var_get('cache_memcache_compress', 0.2);
    $host = var_get('cache_memcache_host', 'localhost');
    $port = var_get('cache_memcache_port', 11211);
    if ($mem->pconnect($host, $port)) {
      if ($mem_compress) {
        $mem->setCompressThreshold(20000, $mem_compress);
      }
      return true;
    }
  }

  return false;
}

/**
 * 缓存自定义配置
 * @param (array) $form
 */
function cache_settings_custom_form(&$form) {
  $form['fields']['cache_memcache_host'] = array(
    '#title' => t('system', 'memcache 主机'),
    '#default_value' => var_get('cache_memcache_host', 'localhost'),
    '#type' => 'textfield',
    '#required' => 1
  );

  $form['fields']['cache_memcache_port'] = array(
    '#title' => t('system', 'memcache 端口'),
    '#default_value' => var_get('cache_memcache_port', 11211),
    '#type' => 'textfield',
    '#required' => 1
  );

  $form['fields']['cache_memcache_prefix'] = array(
    '#title' => t('system', 'memcache 前缀'),
    '#default_value' => var_get('cache_memcache_prefix', str_replace('.', '', $_SERVER['HTTP_HOST'])),
    '#description' => '当有多个站点连接同一个 memcache 服务时适用',
    '#type' => 'textfield',
    '#required' => 1
  );

  $form['fields']['cache_memcache_compress'] = array(
    '#title' => t('system', 'memcache 压缩'),
    '#default_value' => var_get('cache_memcache_compress', 0.2),
    '#type' => 'textfield',
    '#description' => t('system', '数值为压缩等级，0 为不压缩'),
    '#required' => 1
  ); 
}

