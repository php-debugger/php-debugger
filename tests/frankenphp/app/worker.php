<?php
// Minimal FrankenPHP worker. Each iteration of the loop is a separate HTTP
// request handled by the SAME long-lived PHP process — the exact path we
// need the SAPI activate/deactivate hooks for.
ignore_user_abort(true);

$counter = 0;
$handler = static function () use (&$counter): void {
    $counter++;
    require __DIR__ . '/index.php';
};

while (frankenphp_handle_request($handler)) {
    gc_collect_cycles();
}
