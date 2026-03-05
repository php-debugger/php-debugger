--TEST--
xdebug_notify() without a debugging session active
--SKIPIF--
<?php
require __DIR__ . "/../utils.inc";
if (is_stripped_debugger()) die("skip Removed feature in stripped build");
?>
--INI--
xdebug.mode=debug
--SKIPIF--
<?php
require __DIR__ . "/../utils.inc";
if (is_stripped_debugger()) die("skip Removed feature in stripped build");
?>
--FILE--
<?php
var_dump( xdebug_notify( "no debug session" ) );
?>
--EXPECTF--
bool(false)
