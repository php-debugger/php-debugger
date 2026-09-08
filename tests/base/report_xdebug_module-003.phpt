--TEST--
report_xdebug_module: php_debugger.report_xdebug_module=1 re-enables the module alias
--INI--
php_debugger.report_xdebug_module=1
--FILE--
<?php
var_dump(extension_loaded('php_debugger'));
var_dump(extension_loaded('xdebug'));
var_dump(ini_get('xdebug.report_xdebug_module'));
?>
--EXPECT--
bool(true)
bool(true)
string(1) "1"
