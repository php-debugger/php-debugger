--TEST--
xdebug_info() reports that opcache is bypassed for an instrumented request
--EXTENSIONS--
opcache
--INI--
opcache.enable=1
opcache.enable_cli=1
xdebug.mode=debug
xdebug.start_with_request=no
xdebug.on_demand_debugging_enabled=1
--FILE--
<?php
ob_start();
xdebug_info();
$info = ob_get_clean();

foreach ( explode( "\n", $info ) as $line )
{
	if ( str_starts_with( $line, 'OPcache' ) )
	{
		echo $line, "\n";
	}
}
?>
--EXPECT--
OPcache is bypassed for this request, so that every file is compiled with debugging information
