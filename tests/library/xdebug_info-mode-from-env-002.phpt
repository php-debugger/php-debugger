--TEST--
xdebug_info(): mode set through PHP_DEBUGGER_MODE names that variable and does not crash
--ENV--
PHP_DEBUGGER_MODE=debug
--INI--
xdebug.mode=off
--FILE--
<?php
ob_start();
xdebug_info();
$info = ob_get_clean();

/* just the directive name and any "(through ...)" annotation */
preg_match_all('/^((?:xdebug|php_debugger)\.mode\b[^=]*?)\s*=>/m', strip_tags($info), $m);
echo join("\n", $m[1]), "\n";

/* and prove the mode the annotation describes actually took effect */
echo "mode: ", join(',', xdebug_info('mode')), "\n";
?>
--EXPECT--
xdebug.mode (through PHP_DEBUGGER_MODE)
php_debugger.mode (through PHP_DEBUGGER_MODE)
mode: debug
