--TEST--
php_debugger_*() diagnostics name the alias that was actually called
--INI--
display_errors=0
error_log=
xdebug.mode=debug
xdebug.start_with_request=no
xdebug.on_demand_debugging_enabled=0
--FILE--
<?php
set_error_handler(function ($no, $str) { echo $str, "\n"; return true; });

xdebug_break();
php_debugger_break();
xdebug_connect_to_client();
php_debugger_connect_to_client();
?>
--EXPECT--
xdebug_break() ignored: no active debug session and on-demand debugging is disabled. Set xdebug.on_demand_debugging_enabled=1 to enable mid-request debugging
php_debugger_break() ignored: no active debug session and on-demand debugging is disabled. Set xdebug.on_demand_debugging_enabled=1 to enable mid-request debugging
xdebug_connect_to_client() ignored: On-demand debugging is disabled. Set xdebug.on_demand_debugging_enabled=1 to enable mid-request debugging
php_debugger_connect_to_client() ignored: On-demand debugging is disabled. Set xdebug.on_demand_debugging_enabled=1 to enable mid-request debugging
