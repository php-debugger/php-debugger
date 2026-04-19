--TEST--
Test for bug #1656: discover_client_host alters header if multiple values are present
--ENV--
I_LIKE_COOKIES=127.0.0.1, 127.0.0.2
--INI--
xdebug.mode=debug
xdebug.start_with_request=yes
xdebug.client_discovery_header=I_LIKE_COOKIES
xdebug.discover_client_host=1
xdebug.client_port=9999
xdebug.log=
xdebug.log_level=10
--FILE--
<?php
var_dump( $_SERVER['I_LIKE_COOKIES'] );
?>
--EXPECTF--
string(20) "127.0.0.1, 127.0.0.2"
