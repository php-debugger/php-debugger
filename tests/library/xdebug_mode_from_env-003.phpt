--TEST--
PHP_DEBUGGER_MODE: Warning for invalid modes names PHP_DEBUGGER_MODE
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
Xdebug: [Config] Invalid mode 'debug,nonexisting' set for 'PHP_DEBUGGER_MODE' environment variable, fall back to 'xdebug.mode' configuration setting (See: http%sdocs/errors#CFG-C-ENVMODE)
Xdebug: [Config] Invalid mode 'wrongmode' set for 'xdebug.mode' configuration setting (See: http%sdocs/errors#CFG-C-MODE)
Xdebug: [Config] Invalid mode 'debug,nonexisting' set for 'PHP_DEBUGGER_MODE' environment variable, fall back to 'xdebug.mode' configuration setting (See: http%sdocs/errors#CFG-C-ENVMODE)
debug
