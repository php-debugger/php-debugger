--TEST--
PHP_DEBUGGER_SESSION_START as a POST variable names the cookie PHP_DEBUGGER_SESSION
--INI--
xdebug.mode=debug
default_charset=utf-8
xdebug.filename_format=
xdebug.client_port=9172
xdebug.log={TMPFILE:session_cookie_name-007.txt}
xdebug.log_level=10
--POST--
PHP_DEBUGGER_SESSION_START=testing
--FILE--
<?php
require_once __DIR__ . '/../utils.inc';

echo file_get_contents(getTmpFile('session_cookie_name-007.txt'));
unlink(getTmpFile('session_cookie_name-007.txt'));
?>
--EXPECTF--
%A Found 'PHP_DEBUGGER_SESSION_START' HTTP variable, with value 'testing'
%A Adding header 'Set-Cookie: PHP_DEBUGGER_SESSION=testing; path=/; SameSite=Lax'.
%A
