--TEST--
xdebug_info("mode") overriden by XDEBUG_MODE
--INI--
display_errors=0
error_log=
xdebug.mode=off
--ENV--
XDEBUG_MODE=debug
--FILE--
<?php
var_dump(xdebug_info('mode'));
?>
--EXPECTF--
array(1) {
  [0]=>
  string(5) "debug"
}
