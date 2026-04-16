--TEST--
xdebug_info("mode")
--INI--
display_errors=0
error_log=
xdebug.mode=debug
--FILE--
<?php
var_dump(xdebug_info('mode'));
?>
--EXPECTF--
array(1) {
  [0]=>
  string(5) "debug"
}
