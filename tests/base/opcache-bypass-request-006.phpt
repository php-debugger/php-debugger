--TEST--
xdebug_info() does not mention opcache when the request is not instrumented
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
ob_start();
xdebug_info();
$info = ob_get_clean();

var_dump( str_contains( $info, 'OPcache is bypassed' ) );
?>
--EXPECT--
bool(false)
