--TEST--
Starting Debugger: trigger with shared secret, XDEBUG_SESSION_START
--ENV--
XDEBUG_SESSION_START=foobar
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$xdebugLogFileName = getTmpFile('start_with_request_trigger_session_start_shared_secret-001.txt');
@unlink( $xdebugLogFileName );

dbgpRunFile(
	dirname(__FILE__) . '/empty-echo.inc',
	['step_into', 'step_into', 'property_get -n $e', 'detach'],
	[
		'xdebug.mode' => 'debug',
		'xdebug.start_with_request' => 'trigger', 'xdebug.trigger_value' => 'not-foobar',
		'variables_order' => 'PGCS',
		'xdebug.log' => $xdebugLogFileName, 'xdebug.log_level' => 10,
		'xdebug.control_socket' => 'off', 'xdebug.path_mapping' => 'off',
	],
	['timeout' => 1]
);

echo file_get_contents( $xdebugLogFileName );
@unlink( $xdebugLogFileName );
?>
--EXPECTF--
Hi!




[%d] Log opened at %s
[%d] [Step Debug] DEBUG: Found 'XDEBUG_SESSION_START' ENV variable, with value 'foobar'
[%d] [Step Debug] DEBUG: Adding header 'Set-Cookie: XDEBUG_SESSION=foobar; path=/; SameSite=Lax'.
[%d] [Step Debug] INFO: Not activating through legacy method because xdebug.trigger_value is set
[%d] [Config] DEBUG: Checking if trigger 'XDEBUG_TRIGGER' is enabled
[%d] [Config] INFO: Trigger value for 'XDEBUG_TRIGGER' not found, falling back to 'XDEBUG_SESSION'
[%d] [Config] INFO: Trigger value for 'PHP_DEBUGGER_SESSION' not found, so not activating
[%d] [Step Debug] DEBUG: Adding header 'Content-type: text/html; charset=UTF-8'.
[%d] Log closed at %s
