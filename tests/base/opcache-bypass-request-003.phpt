--TEST--
Instrumented request disables opcache (on_demand_debugging_enabled=1)
--EXTENSIONS--
opcache
--INI--
opcache.enable=1
opcache.enable_cli=1
xdebug.mode=debug
xdebug.start_with_request=no
xdebug.on_demand_debugging_enabled=1
--FILE--
<?php
$status = opcache_get_status(false);
var_dump($status['opcache_enabled']);
var_dump(ini_get('opcache.enable'));
?>
--EXPECT--
bool(false)
string(1) "0"
