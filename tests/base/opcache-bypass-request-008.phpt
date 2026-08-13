--TEST--
Breakpoints bind in an opcache-cached file when the client connects at request start
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
if (!extension_loaded('zend opcache')) { echo 'skip: opcache extension not available'; }
?>
--FILE--
<?php
require dirname(__FILE__) . '/../debugger/dbgp/dbgpclient.php';

$filename = dirname(__FILE__) . '/opcache-bypass-request-008.inc';
$cacheDir = getTmpFile( 'opcache-bypass-request-008-cache' );

@mkdir( $cacheDir );

/* Same set-up as opcache-bypass-request-007, but for the normal production
 * path: no on-demand debugging, and the client is connected at request start.
 * The first run stores the file compiled without debugging information. */
$opcacheOptions = "-d opcache.enable=1 -d opcache.enable_cli=1 -d opcache.file_cache={$cacheDir}";
$primeCommand = getenv( 'TEST_PHP_EXECUTABLE' ) . ' ' . getenv( 'TEST_PHP_ARGS' ) . ' ' . $opcacheOptions
	. ' -d xdebug.mode=debug -d xdebug.start_with_request=no -d xdebug.on_demand_debugging_enabled=0 '
	. escapeshellarg( $filename ) . ' > /dev/null 2>&1';
exec( $primeCommand );

$cached = new RecursiveIteratorIterator( new RecursiveDirectoryIterator( $cacheDir, FilesystemIterator::SKIP_DOTS ) );
echo 'cached by the non-debugged run: ', iterator_count( $cached ) > 0 ? "yes" : "no", "\n\n";

$commands = array(
	"breakpoint_set -t line -f file://{$filename} -n 4",
	'run',
	'property_get -n $i',
	'detach',
);

dbgpRunFile( $filename, $commands, array(
	'opcache.enable' => 1,
	'opcache.enable_cli' => 1,
	'opcache.file_cache' => $cacheDir,
	'xdebug.on_demand_debugging_enabled' => 0,
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
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://opcache-bypass-request-008.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[PHP Debugger]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://opcache-bypass-request-008.inc -n 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id="{{PID}}0001"></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://opcache-bypass-request-008.inc" lineno="4"></xdebug:message></response>

-> property_get -i 3 -n $i
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="3"><property name="$i" fullname="$i" type="int"><![CDATA[0]]></property></response>

-> detach -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="4" status="stopping" reason="ok"></response>
