--TEST--
INI alias: php_debugger.* takes precedence over xdebug.* regardless of order
--INI--
php_debugger.client_host=from-php-debugger
xdebug.client_host=from-xdebug
xdebug.idekey=xdebug-key
php_debugger.idekey=debugger-key
php_debugger.client_port=9002
xdebug.client_port=9001
--FILE--
<?php
// php_debugger.* should win in all cases, regardless of INI order
var_dump(ini_get('xdebug.client_host'));
var_dump(ini_get('xdebug.idekey'));
var_dump(ini_get('xdebug.client_port'));
?>
--EXPECT--
string(17) "from-php-debugger"
string(12) "debugger-key"
string(4) "9002"
