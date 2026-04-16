--TEST--
XDEBUG_MODE overrides xdebug.mode
--INI--
display_errors=0
error_log=
xdebug.mode=off
--ENV--
XDEBUG_MODE=debug
--FILE--
<?php
xdebug_info();
?>
--EXPECTF--
%Athrough 'XDEBUG_MODE' / 'PHP_DEBUGGER_MODE' env variable%AStep Debugger => ✔ enabled%A
