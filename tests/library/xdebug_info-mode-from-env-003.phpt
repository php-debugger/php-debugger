--TEST--
xdebug_info(): mode set through the INI setting carries no environment annotation
--INI--
xdebug.mode=debug
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
xdebug.mode
php_debugger.mode
mode: debug
