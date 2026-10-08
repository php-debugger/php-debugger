--TEST--
Test for bug #2439: Overloaded set_time_limit() always returns false, even when the limit was applied
--INI--
xdebug.mode=debug
xdebug.start_with_request=no
--FILE--
<?php
ini_set('max_execution_time', '42');

var_dump(set_time_limit(0));
var_dump(ini_get('max_execution_time'));
?>
--EXPECT--
bool(true)
string(1) "0"
