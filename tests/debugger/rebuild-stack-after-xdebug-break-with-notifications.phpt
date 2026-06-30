--TEST--
Stack is correctly rebuilt after xdebug_break() with on_demand_debugging_enabled=0 and notifications enabled
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$filename = dirname(__FILE__) . '/rebuild-stack-after-xdebug-break-with-notifications.inc';

$commands = array(
	'feature_set -n notify_ok -v 1',
	'run',
	'stack_get',
	'step_into',
	'stack_get',
	'step_out',
	'stack_get',
	'detach',
);

$settings = [
	'xdebug.mode' => 'debug',
	'xdebug.start_with_request' => 'yes',
	'xdebug.on_demand_debugging_enabled' => 0
];

dbgpRunFile( $filename, $commands, $settings );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://rebuild-stack-after-xdebug-break-with-notifications.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[PHP Debugger]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n notify_ok -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="notify_ok" success="1"></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://rebuild-stack-after-xdebug-break-with-notifications.inc" lineno="8"></xdebug:message></response>

-> stack_get -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="3"><stack where="outer_function" level="0" type="file" filename="file://rebuild-stack-after-xdebug-break-with-notifications.inc" lineno="8"></stack><stack where="{main}" level="1" type="file" filename="file://rebuild-stack-after-xdebug-break-with-notifications.inc" lineno="12"></stack></response>

-> step_into -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file://rebuild-stack-after-xdebug-break-with-notifications.inc" lineno="3"></xdebug:message></response>

-> stack_get -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="5"><stack where="inner_function" level="0" type="file" filename="file://rebuild-stack-after-xdebug-break-with-notifications.inc" lineno="3"></stack><stack where="outer_function" level="1" type="file" filename="file://rebuild-stack-after-xdebug-break-with-notifications.inc" lineno="8"></stack><stack where="{main}" level="2" type="file" filename="file://rebuild-stack-after-xdebug-break-with-notifications.inc" lineno="12"></stack></response>

-> step_out -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_out" transaction_id="6" status="break" reason="ok"><xdebug:message filename="file://rebuild-stack-after-xdebug-break-with-notifications.inc" lineno="9"></xdebug:message></response>

-> stack_get -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="7"><stack where="outer_function" level="0" type="file" filename="file://rebuild-stack-after-xdebug-break-with-notifications.inc" lineno="9"></stack><stack where="{main}" level="1" type="file" filename="file://rebuild-stack-after-xdebug-break-with-notifications.inc" lineno="12"></stack></response>

-> detach -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="8" status="stopping" reason="ok"></response>