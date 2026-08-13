--TEST--
On-demand debugging without a client warns that opcache is bypassed
--EXTENSIONS--
opcache
--INI--
opcache.enable=1
opcache.enable_cli=1
xdebug.mode=debug
xdebug.start_with_request=no
xdebug.on_demand_debugging_enabled=1
xdebug.log={TMPFILE:opcache-bypass-request-004.txt}
xdebug.log_level=7
--FILE--
<?php
require __DIR__ . '/../utils.inc';

$logFileName = getTmpFile( 'opcache-bypass-request-004.txt' );
echo file_get_contents( $logFileName );
@unlink( $logFileName );
?>
--EXPECTF--
[%d] Log opened at %s
[%d] [Config] WARN: OPcache is bypassed for every request because xdebug.on_demand_debugging_enabled=1 needs each request compiled with debugging instrumentation. Set it to 0 to keep OPcache active when no debugging client is connected.
