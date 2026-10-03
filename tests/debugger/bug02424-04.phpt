--TEST--
Test for bug #2424: Ctrl Socket: Invalid command arguments
--INI--
xdebug.mode=debug
xdebug.start_with_request=no
xdebug.control_socket=time
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('linux; ext-flag control-socket');
?>
--FILE--
<?php
require 'dbgp/ctrlSocketClient.php';

$c = new CtrlSocketClient( 'bug02424-04' );
$c->runCommand('ps -');

?>
--EXPECTF--
<?xml version="1.0" encoding="UTF-8"?>
<ctrl-response xmlns:xdebug-ctrl="https://xdebug.org/ctrl/xdebug"><error code="3"><message><![CDATA[invalid or missing options]]></message></error></ctrl-response>
