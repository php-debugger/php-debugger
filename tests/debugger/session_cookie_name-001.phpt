--TEST--
Session cookie is named XDEBUG_SESSION for an XDEBUG_SESSION_START trigger ($_ENV)
--ENV--
XDEBUG_SESSION_START=testing
--INI--
xdebug.mode=debug
default_charset=utf-8
xdebug.filename_format=
xdebug.client_port=9172
xdebug.log={TMPFILE:session_cookie_name-001.txt}
xdebug.log_level=10
--FILE--
<?php
require_once __DIR__ . '/../utils.inc';

echo file_get_contents(getTmpFile('session_cookie_name-001.txt'));
unlink(getTmpFile('session_cookie_name-001.txt'));
?>
--EXPECTF--
%A Found 'XDEBUG_SESSION_START' HTTP variable, with value 'testing'
%A Adding header 'Set-Cookie: XDEBUG_SESSION=testing; path=/; SameSite=Lax'.
%A
