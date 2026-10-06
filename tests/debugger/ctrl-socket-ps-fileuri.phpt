--TEST--
Ctrl Socket: 'ps' reports the current file with no debugging client attached
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
/* Without a client the observer stops collecting frames, so 'ps' has to build
 * the stack itself to have a file name to report. */
require 'dbgp/ctrlSocketClient.php';

$c = new CtrlSocketClient( 'ctrl-socket-ps-fileuri' );

$c->runCommand('ps', false);
?>
--EXPECTF--
<?xml version="1.0" encoding="UTF-8"?>
<ctrl-response xmlns:xdebug-ctrl="https://xdebug.org/ctrl/xdebug"><ps success="1"><engine version=""><![CDATA[PHP Debugger]]></engine><fileuri><![CDATA[%sctrl-socket-ps-fileuri.php]]></fileuri><pid></pid><time></time><memory></memory></ps></ctrl-response>
