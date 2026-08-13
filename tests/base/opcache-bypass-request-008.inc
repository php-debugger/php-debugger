<?php
function worker($i)
{
	echo "$i\n";
}

for ($i = 0; $i < 5; $i++)
{
	worker($i);
}
