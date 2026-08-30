--TEST--
PHP_DEBUGGER_SESSION_STOP also clears both session cookies
--INI--
xdebug.mode=debug
default_charset=utf-8
xdebug.filename_format=
xdebug.client_port=9172
xdebug.on_demand_debugging_enabled=1
xdebug.log={TMPFILE:session_cookie_name-005.txt}
xdebug.log_level=10
--GET--
PHP_DEBUGGER_SESSION_STOP=testing
--FILE--
<?php
require_once __DIR__ . '/../utils.inc';

echo file_get_contents(getTmpFile('session_cookie_name-005.txt'));
unlink(getTmpFile('session_cookie_name-005.txt'));
?>
--EXPECTF--
%A Adding header 'Set-Cookie: XDEBUG_SESSION=deleted; %s path=/; SameSite=Lax'.
%A Adding header 'Set-Cookie: PHP_DEBUGGER_SESSION=deleted; %s path=/; SameSite=Lax'.
%A
