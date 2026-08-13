--TEST--
Breakpoints bind in a file that opcache cached during an earlier non-debugged request
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
if (!extension_loaded('zend opcache')) { echo 'skip: opcache extension not available'; }
?>
--FILE--
<?php
require dirname(__FILE__) . '/../debugger/dbgp/dbgpclient.php';

$filename = dirname(__FILE__) . '/opcache-bypass-request-007.inc';
$cacheDir = getTmpFile( 'opcache-bypass-request-007-cache' );

@mkdir( $cacheDir );

/* Run the script once *without* debugging, so opcache stores a copy of it
 * compiled without debugging information. opcache's file cache is used
 * instead of shared memory because it survives into the next process, the
 * way SHM survives into the next request of an FPM worker. */
$opcacheOptions = "-d opcache.enable=1 -d opcache.enable_cli=1 -d opcache.file_cache={$cacheDir}";
$primeCommand = getenv( 'TEST_PHP_EXECUTABLE' ) . ' ' . getenv( 'TEST_PHP_ARGS' ) . ' ' . $opcacheOptions
	. ' -d xdebug.mode=debug -d xdebug.start_with_request=no -d xdebug.on_demand_debugging_enabled=0 '
	. escapeshellarg( $filename ) . ' > /dev/null 2>&1';
exec( $primeCommand );

$cached = new RecursiveIteratorIterator( new RecursiveDirectoryIterator( $cacheDir, FilesystemIterator::SKIP_DOTS ) );
echo 'cached by the non-debugged run: ', iterator_count( $cached ) > 0 ? "yes" : "no", "\n\n";

/* Now debug it. The breakpoint can only bind if this request recompiled the
 * file instead of reusing the cached, non-instrumented copy. */
$commands = array(
	'breakpoint_set -t line -n 8 -- ' . base64_encode( '$i == 4' ),
	'run',
	'property_get -n $i',
	'detach',
);

dbgpRunFile( $filename, $commands, array(
	'opcache.enable' => 1,
	'opcache.enable_cli' => 1,
	'opcache.file_cache' => $cacheDir,
	'xdebug.mode' => 'debug',
	'xdebug.start_with_request' => 'trigger',
	'xdebug.on_demand_debugging_enabled' => 1,
) );

foreach ( new RecursiveIteratorIterator( new RecursiveDirectoryIterator( $cacheDir, FilesystemIterator::SKIP_DOTS ), RecursiveIteratorIterator::CHILD_FIRST ) as $entry )
{
	$entry->isDir() ? @rmdir( $entry->getPathname() ) : @unlink( $entry->getPathname() );
}
@rmdir( $cacheDir );
?>
--EXPECT--
cached by the non-debugged run: yes

<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://opcache-bypass-request-007.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[PHP Debugger]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -n 8 -- JGkgPT0gNA==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id="{{PID}}0001"></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://opcache-bypass-request-007.inc" lineno="8"></xdebug:message></response>

-> property_get -i 3 -n $i
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="3"><property name="$i" fullname="$i" type="int"><![CDATA[4]]></property></response>

-> detach -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="4" status="stopping" reason="ok"></response>
