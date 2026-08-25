--TEST--
php_debugger_*() aliases exist for every public xdebug_*() debugger function
--INI--
display_errors=0
error_log=
xdebug.mode=debug
--FILE--
<?php
$aliases = [
	'xdebug_break'              => 'php_debugger_break',
	'xdebug_connect_to_client'  => 'php_debugger_connect_to_client',
	'xdebug_info'               => 'php_debugger_info',
	'xdebug_is_debugger_active' => 'php_debugger_is_debugger_active',
	'xdebug_notify'             => 'php_debugger_notify',
];

foreach ($aliases as $canonical => $alias) {
	$rc = new ReflectionFunction($canonical);
	$ra = new ReflectionFunction($alias);

	printf(
		"%s: exists=%s params=%s required=%s\n",
		$alias,
		function_exists($alias) ? 'yes' : 'no',
		$rc->getNumberOfParameters() === $ra->getNumberOfParameters() ? 'same' : 'DIFFERENT',
		$rc->getNumberOfRequiredParameters() === $ra->getNumberOfRequiredParameters() ? 'same' : 'DIFFERENT'
	);
}

echo "\n";
var_dump(xdebug_info('mode') === php_debugger_info('mode'));
var_dump(xdebug_is_debugger_active() === php_debugger_is_debugger_active());
?>
--EXPECT--
php_debugger_break: exists=yes params=same required=same
php_debugger_connect_to_client: exists=yes params=same required=same
php_debugger_info: exists=yes params=same required=same
php_debugger_is_debugger_active: exists=yes params=same required=same
php_debugger_notify: exists=yes params=same required=same

bool(true)
bool(true)
