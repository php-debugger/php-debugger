--TEST--
Test for Xdebug's remote log output through PHP's log
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; !win');
?>
--ENV--
I_LIKE_COOKIES=cookiehost
--INI--
xdebug.mode=debug
xdebug.start_with_request=yes
xdebug.log=
xdebug.discover_client_host=1
xdebug.client_host=doesnotexist2
xdebug.client_port=9003
xdebug.client_discovery_header=I_LIKE_COOKIES
--FILE--
<?php
echo strlen("foo"), "\n";
?>
--EXPECTF--
3
