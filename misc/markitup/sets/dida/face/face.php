<?php
// $Id: face.php 50 2010-09-07 19:29:21Z yd2004 $

header('Content-Type:text/plain');

$images = array();

if ($objs = glob('*')) {
  foreach ($objs as $obj) {
    if (is_dir($obj)) {
      
      if (is_file($obj.'/info.php')) {
        include $obj.'/info.php';
        $name = $info['name'];
      } else {
        $name = $obj;
      }
      
      $images['contents'][$obj]['name'] = $name;
      $images['contents'][$obj]['items'] = glob($obj.'/*gif');
    }
  }
}

if (!$images['contents']) {
  $images['error'] = 'error';
}

echo json_encode($images);