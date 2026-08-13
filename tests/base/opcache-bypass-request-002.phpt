--TEST--
Non-instrumented request keeps opcache enabled (near-zero overhead path)
--EXTENSIONS--
opcache
--INI--
opcache.enable=1
opcache.enable_cli=1
xdebug.mode=debug
xdebug.start_with_request=no
xdebug.on_demand_debugging_enabled=0
--FILE--
<?php
$status = opcache_get_status(false);
var_dump($status['opcache_enabled']);
var_dump(ini_get('opcache.enable'));
?>
--EXPECT--
bool(true)
string(1) "1"
