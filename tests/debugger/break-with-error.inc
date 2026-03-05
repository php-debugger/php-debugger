<?php
$e = array();
@trigger_error('foo');
$e = array();
echo "Not triggered!\n";
?>
