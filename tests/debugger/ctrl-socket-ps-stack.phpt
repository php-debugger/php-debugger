--TEST--
Ctrl Socket: 'ps' command with a populated stack
--INI--
xdebug.mode=debug
xdebug.start_with_request=no
xdebug.on_demand_debugging_enabled=1
xdebug.control_socket=time
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('linux; ext-flag control-socket');
?>
--FILE--
<?php
/* 'ps' reports the bottom stack frame's file name. With
 * on_demand_debugging_enabled the observer keeps collecting frames even though
 * no client is attached, so unlike the bug02424-* tests the stack is populated
 * by the time the command is dispatched — which is the case that has to survive
 * the response tree being freed. */
require 'dbgp/ctrlSocketClient.php';

$c = new CtrlSocketClient( 'ctrl-socket-ps-stack' );

$c->runCommand('ps');
?>
--EXPECTF--
<?xml version="1.0" encoding="UTF-8"?>
<ctrl-response xmlns:xdebug-ctrl="https://xdebug.org/ctrl/xdebug"><ps success="1"><engine version=""><![CDATA[PHP Debugger]]></engine><fileuri></fileuri><pid></pid><time></time><memory></memory></ps></ctrl-response>
