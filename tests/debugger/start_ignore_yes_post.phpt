--TEST--
Starting Debugger: overridden POST ignore value is 'yes'
--XFAIL--
Phase 2 RINIT gating calls xdebug_should_ignore() at RINIT time. These tests use auto_prepend_file PHP code to set $_COOKIE/$_GET/$_POST, which only executes after RINIT. At RINIT, only getenv() works, so the superglobal override is never seen. Real HTTP headers in CGI/FPM ARE available at RINIT and work correctly.
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$xdebugLogFileName = getTmpFile('start_ignore_yes_env.txt');
@unlink( $xdebugLogFileName );

dbgpRunFile(
	dirname(__FILE__) . '/break-echo.inc',
	['stack_get', 'step_into', 'detach'],
	[
		'xdebug.mode' => 'debug', 'xdebug.start_with_request' => 'yes',
		'xdebug.log' => $xdebugLogFileName, 'xdebug.log_level' => 10,
	],
	['timeout' => 1, 'env' => [ 'XDEBUG_IGNORE' => 'yes' ], 'auto_prepend' => '<?php $_POST["XDEBUG_IGNORE"] = "yes";']
);

echo file_get_contents( $xdebugLogFileName );
@unlink( $xdebugLogFileName );
?>
--EXPECTF--
Hi!




[%d] Log opened at %s%A
[%d] [Step Debug] DEBUG: Not activating because an 'XDEBUG_IGNORE' ENV variable is present, with value 'yes'.
[%d] [Step Debug] DEBUG: Not activating because an 'XDEBUG_IGNORE' POST variable is present, with value 'yes'.
[%d] Log closed at %s
