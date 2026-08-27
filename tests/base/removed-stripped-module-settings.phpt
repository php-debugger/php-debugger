--TEST--
Stripped module INI settings are not registered
--FILE--
<?php
$settings = [
	'xdebug.output_dir',
	'xdebug.use_compression',
	'xdebug.file_link_format',
	'xdebug.filename_format',
	'php_debugger.output_dir',
	'php_debugger.use_compression',
	'php_debugger.file_link_format',
	'php_debugger.filename_format',
];

ob_start();
phpinfo(INFO_MODULES);
$phpinfo = ob_get_clean();

foreach ($settings as $setting) {
	printf(
		"%s: ini=%s, phpinfo=%s\n",
		$setting,
		ini_get($setting) === false ? 'absent' : 'present',
		str_contains($phpinfo, $setting) ? 'present' : 'absent',
	);
}
?>
--EXPECT--
xdebug.output_dir: ini=absent, phpinfo=absent
xdebug.use_compression: ini=absent, phpinfo=absent
xdebug.file_link_format: ini=absent, phpinfo=absent
xdebug.filename_format: ini=absent, phpinfo=absent
php_debugger.output_dir: ini=absent, phpinfo=absent
php_debugger.use_compression: ini=absent, phpinfo=absent
php_debugger.file_link_format: ini=absent, phpinfo=absent
php_debugger.filename_format: ini=absent, phpinfo=absent
