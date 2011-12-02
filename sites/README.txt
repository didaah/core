// $Id: README.txt 74 2011-01-04 14:59:05Z yd2004 $

./sites/modules：非核心模块
./sites/themes：自定义主题
./sites/cache：缓存目录，必须可读写
./sites/fiels：默认文件目录，必须可读写
./sites/logs：程序运行生成的各种日志


如何使用同一程序，创建多个站点？
以当前访问域名自动匹配，请看 ./sites/demo.example.com 下说明文件，但不建议使用这个方式
推荐使用自定义配置，请看本目录下 default.config.php 文件