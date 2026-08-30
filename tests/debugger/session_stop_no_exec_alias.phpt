--TEST--
PHP_DEBUGGER_SESSION_STOP_NO_EXEC ends the session without executing any code
--INI--
xdebug.mode=debug
--GET--
PHP_DEBUGGER_SESSION_STOP_NO_EXEC=netbeans-xdebug
--FILE--
<?php
echo "this should never run\n";
?>
--EXPECT--
DEBUG SESSION ENDED
