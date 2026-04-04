--TEST--
Starting Debugger: never, break, XDEBUG_SESSION_START
--ENV--
XDEBUG_SESSION_START=foobar
--FILE--
<?php
require 'dbgp/dbgpclient.php';

dbgpRunFile(
	dirname(__FILE__) . '/break-echo.inc',
	['stack_get', 'step_into', 'detach'],
	[
		'xdebug.mode' => 'debug',
	 	'xdebug.start_with_request' => 'no',
		'xdebug.on_demand_debugging_enabled' => 1
	],
	['timeout' => 1]
);
?>
--EXPECTF--
Hi!
