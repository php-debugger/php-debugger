--TEST--
XDEBUG_SESSION_STOP_NO_EXEC and its alias both clear both session cookies
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!win');
if (!getenv('TEST_PHP_CGI_EXECUTABLE') || !is_executable(getenv('TEST_PHP_CGI_EXECUTABLE'))) {
	echo 'skip: no CGI binary available';
}
?>
--FILE--
<?php
/* The NO_EXEC trigger aborts the request before any user code runs, so the
 * request under test cannot inspect its own headers. Drive a CGI child
 * instead and read the Set-Cookie headers it emits. */
$cgi    = getenv('TEST_PHP_CGI_EXECUTABLE');
$args   = getenv('TEST_PHP_ARGS');
$script = __DIR__ . '/session_stop_no_exec_cookies.inc';

foreach (['XDEBUG_SESSION_STOP_NO_EXEC', 'PHP_DEBUGGER_SESSION_STOP_NO_EXEC'] as $trigger) {
	$cmd = 'REDIRECT_STATUS=1 REQUEST_METHOD=GET'
		. ' QUERY_STRING=' . escapeshellarg("$trigger=testing")
		. ' SCRIPT_FILENAME=' . escapeshellarg($script)
		. ' ' . escapeshellarg($cgi) . ' ' . $args
		. ' -d xdebug.mode=debug -d xdebug.client_port=9172 2>&1';

	$out = (string) shell_exec($cmd);

	preg_match_all('/^Set-Cookie: ([A-Z_]+)=([a-z]+);/m', $out, $m, PREG_SET_ORDER);
	$cookies = array_map(fn($c) => "{$c[1]}={$c[2]}", $m);
	sort($cookies);

	echo $trigger, "\n";
	echo '  ', $cookies ? join("\n  ", $cookies) : '(none)', "\n";
	echo '  body: ', trim(substr($out, (int) strpos($out, "\r\n\r\n"))), "\n";
}
?>
--EXPECT--
XDEBUG_SESSION_STOP_NO_EXEC
  PHP_DEBUGGER_SESSION=deleted
  XDEBUG_SESSION=deleted
  body: DEBUG SESSION ENDED
PHP_DEBUGGER_SESSION_STOP_NO_EXEC
  PHP_DEBUGGER_SESSION=deleted
  XDEBUG_SESSION=deleted
  body: DEBUG SESSION ENDED
