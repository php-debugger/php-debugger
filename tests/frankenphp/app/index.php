<?php
require_once __DIR__ . '/lib.php';

$pid = getmypid();
$now = (new DateTimeImmutable())->format(DATE_ATOM);
$worker_iteration = $GLOBALS['counter'] ?? 'n/a';

$result = workload((int)$worker_iteration);

header('Content-Type: text/plain');
echo "pid={$pid} time={$now} iter={$worker_iteration}\n";
echo "result={$result}\n";
echo "cookie: " . ($_COOKIE['XDEBUG_SESSION'] ?? '(none)') . "\n";
