--TEST--
report_xdebug_module: xdebug module is not reported by default
--FILE--
<?php
var_dump(extension_loaded('php_debugger'));
var_dump(extension_loaded('xdebug'));
var_dump(ini_get('xdebug.report_xdebug_module'));
var_dump(ini_get('php_debugger.report_xdebug_module'));
?>
--EXPECT--
bool(true)
bool(false)
string(1) "0"
string(1) "0"
