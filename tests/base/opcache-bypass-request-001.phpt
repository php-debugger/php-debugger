--TEST--
Connected debug session disables opcache (on_demand_debugging_enabled=0)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
/* The spawned PHP enables opcache itself (see ini_options below); all that
 * is needed here is for the extension to be present in the test binary. */
if (!extension_loaded('zend opcache')) { echo 'skip: opcache extension not available'; }
?>
--FILE--
<?php
require dirname(__FILE__) . '/../debugger/dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/opcache-bypass-request-001.inc';

$commands = array(
	'step_into',
	// ini_get("opcache.enable") . ":" . (int) opcache_get_status(false)["opcache_enabled"]
	'eval -- aW5pX2dldCgib3BjYWNoZS5lbmFibGUiKSAuICI6IiAuIChpbnQpIG9wY2FjaGVfZ2V0X3N0YXR1cyhmYWxzZSlbIm9wY2FjaGVfZW5hYmxlZCJd',
	'detach',
);

dbgpRunFile( $filename, $commands, array(
	'opcache.enable' => 1,
	'opcache.enable_cli' => 1,
	'xdebug.on_demand_debugging_enabled' => 0,
) );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://opcache-bypass-request-001.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[PHP Debugger]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://opcache-bypass-request-001.inc" lineno="2"></xdebug:message></response>

-> eval -i 2 -- aW5pX2dldCgib3BjYWNoZS5lbmFibGUiKSAuICI6IiAuIChpbnQpIG9wY2FjaGVfZ2V0X3N0YXR1cyhmYWxzZSlbIm9wY2FjaGVfZW5hYmxlZCJd
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="2"><property type="string" size="3" encoding="base64"><![CDATA[MDow]]></property></response>

-> detach -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="3" status="stopping" reason="ok"></response>
