--TEST--
INI alias: php_debugger.idekey sets the IDE key in init packet
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';

dbgpRunFile(
	dirname(__FILE__) . '/empty-echo.inc',
	['detach'],
	['php_debugger.mode' => 'debug', 'php_debugger.start_with_request' => 'yes', 'php_debugger.idekey' => 'MYIDEKEY']
);
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://empty-echo.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey="MYIDEKEY"><engine version=""><![CDATA[PHP Debugger]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> detach -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="1" status="stopping" reason="ok"></response>
