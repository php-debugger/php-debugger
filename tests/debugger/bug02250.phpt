--TEST--
Test for bug #2250: Multiple 'DEBUG SESSION ENDED' messages
--SKIPIF--
<?php
require __DIR__ . "/../utils.inc";
if (is_stripped_debugger()) die("skip Removed feature in stripped build");
?>
--INI--
xdebug.mode=debug
--GET--
XDEBUG_SESSION_STOP_NO_EXEC=netbeans-xdebug
--SKIPIF--
<?php
require __DIR__ . "/../utils.inc";
if (is_stripped_debugger()) die("skip Removed feature in stripped build");
?>
--FILE--
<?php
xdebug_info();
?>
--EXPECT--
DEBUG SESSION ENDED
