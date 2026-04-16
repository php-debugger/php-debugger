--TEST--
Test for bug #1782: Make sure we use SameSite=Lax cookies (>= PHP 7.3)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.3');
?>
--ENV--
XDEBUG_CONFIG=idekey=testing
--INI--
xdebug.mode=debug
default_charset=utf-8
xdebug.filename_format=
xdebug.client_port=9172
xdebug.log={TMPFILE:bug01782.txt}
xdebug.log_level=10
--FILE--
<?php
require_once __DIR__ . '/../utils.inc';

echo file_get_contents(getTmpFile('bug01782.txt'));
unlink(getTmpFile('bug01782.txt'));
?>
--EXPECTF--
%A Adding header 'Set-Cookie: XDEBUG_SESSION=testing; path=/; SameSite=Lax'.
%A