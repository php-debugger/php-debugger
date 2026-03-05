--TEST--
xdebug_notify() without a debugging session active
--SKIPIF--
<?php
require __DIR__ . "/../utils.inc";
if (is_stripped_debugger()) die("skip Uses xdebug_notify (removed)");
?>
--INI--
xdebug.mode=debug
--FILE--
<?php
var_dump( xdebug_notify( "no debug session" ) );
?>
--EXPECTF--
bool(false)
