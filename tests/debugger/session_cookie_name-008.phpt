--TEST--
PHP_DEBUGGER_SESSION_START from the environment only (variables_order without E)
--ENV--
PHP_DEBUGGER_SESSION_START=testing
--INI--
xdebug.mode=debug
default_charset=utf-8
xdebug.filename_format=
xdebug.client_port=9172
variables_order=GPCS
xdebug.log={TMPFILE:session_cookie_name-008.txt}
xdebug.log_level=10
--FILE--
<?php
require_once __DIR__ . '/../utils.inc';

echo file_get_contents(getTmpFile('session_cookie_name-008.txt'));
unlink(getTmpFile('session_cookie_name-008.txt'));
?>
--EXPECTF--
%A Found 'PHP_DEBUGGER_SESSION_START' ENV variable, with value 'testing'
%A Adding header 'Set-Cookie: PHP_DEBUGGER_SESSION=testing; path=/; SameSite=Lax'.
%A
