--TEST--
PHP_DEBUGGER_MODE: Invalid mode warning names PHP_DEBUGGER_MODE, not XDEBUG_MODE
--INI--
display_errors=0
error_log=
xdebug.mode=wrongmode
--ENV--
PHP_DEBUGGER_MODE=debug,nonexisting
--FILE--
<?php
echo join( ',', xdebug_info( 'mode' ) );
?>
--EXPECTF--
Xdebug: [Config] Invalid mode 'debug,nonexisting' set for 'PHP_DEBUGGER_MODE' environment variable, fall back to the 'xdebug.mode' / 'php_debugger.mode' configuration setting (See: http%sdocs/errors#CFG-C-ENVMODE)
Xdebug: [Config] Invalid mode 'wrongmode' set for the 'xdebug.mode' / 'php_debugger.mode' configuration setting (See: http%sdocs/errors#CFG-C-MODE)
Xdebug: [Config] Invalid mode 'debug,nonexisting' set for 'PHP_DEBUGGER_MODE' environment variable, fall back to the 'xdebug.mode' / 'php_debugger.mode' configuration setting (See: http%sdocs/errors#CFG-C-ENVMODE)
debug
