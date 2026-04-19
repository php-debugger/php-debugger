<?php
function stackMe($e)
{
	return array_reverse($e->getTrace());
}

function breakOnMe()
{
	throw new Exception("Bite Me");
}

set_exception_handler('stackMe');

$foo = breakOnMe();
?>
