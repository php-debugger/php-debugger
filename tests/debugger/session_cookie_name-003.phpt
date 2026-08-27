--TEST--
Session cookie is named PHP_DEBUGGER_SESSION for a PHP_DEBUGGER_CONFIG trigger
--ENV--
PHP_DEBUGGER_CONFIG=idekey=testing
--INI--
xdebug.mode=debug
default_charset=utf-8
xdebug.filename_format=
xdebug.client_port=9172
xdebug.log={TMPFILE:session_cookie_name-003.txt}
xdebug.log_level=10
--FILE--
<?php
require_once __DIR__ . '/../utils.inc';

echo file_get_contents(getTmpFile('session_cookie_name-003.txt'));
unlink(getTmpFile('session_cookie_name-003.txt'));
?>
--EXPECTF--
%A Adding header 'Set-Cookie: PHP_DEBUGGER_SESSION=testing; path=/; SameSite=Lax'.
%A
