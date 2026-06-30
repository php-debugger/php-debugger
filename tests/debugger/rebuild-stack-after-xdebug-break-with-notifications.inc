<?php
function inner_function() {
	return 42;
}

function outer_function() {
	xdebug_break();
	$result = inner_function();
	return $result;
}

outer_function();