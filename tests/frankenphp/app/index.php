<?php
// Set a breakpoint in PhpStorm/VS Code on the line below — every request
// served by the worker should pause here when XDEBUG_SESSION is sent.
$pid = getmypid();
$now = (new DateTimeImmutable())->format(DATE_ATOM);
$worker_iteration = $GLOBALS['counter'] ?? 'n/a';

header('Content-Type: text/plain');
echo "pid={$pid} time={$now} iter={$worker_iteration}\n";
echo "cookie: " . ($_COOKIE['XDEBUG_SESSION'] ?? '(none)') . "\n";
