--TEST--
report_xdebug_module: xdebug.report_xdebug_module=1 re-enables the module alias
--INI--
xdebug.report_xdebug_module=1
--FILE--
<?php
var_dump(extension_loaded('php_debugger'));
var_dump(extension_loaded('xdebug'));
?>
--EXPECT--
bool(true)
bool(true)
