--TEST--
Compatibility stubs: emit E_DEPRECATED and return safe defaults
--INI--
error_reporting=E_ALL
--FILE--
<?php
/*
 * Every stripped Xdebug function is expected to:
 *   1. Emit an E_DEPRECATED notice
 *   2. Return a documented safe default (false / [] / 0 / 0.0 / null-void)
 *
 * We silence the deprecations with @ and re-check via set_error_handler
 * to keep the --EXPECT-- block stable and assert both behaviours.
 */

$deprecations = [];
set_error_handler(function ($errno, $errstr) use (&$deprecations) {
	if ($errno === E_DEPRECATED) {
		$deprecations[] = $errstr;
		return true;
	}
	return false;
});

function check(string $name, $actual, $expected): void {
	$ok = $actual === $expected ? 'ok' : 'FAIL';
	echo "$name: $ok\n";
}

/* profiler */
check('xdebug_get_profiler_filename', xdebug_get_profiler_filename(), false);

/* coverage */
check('xdebug_code_coverage_started', xdebug_code_coverage_started(), false);
check('xdebug_get_code_coverage',     xdebug_get_code_coverage(),     []);
check('xdebug_start_code_coverage',   xdebug_start_code_coverage(),   null);
check('xdebug_stop_code_coverage',    xdebug_stop_code_coverage(),    null);

/* tracing */
check('xdebug_get_tracefile_name',      xdebug_get_tracefile_name(),      false);
check('xdebug_start_trace',             xdebug_start_trace(),             false);
check('xdebug_stop_trace',              xdebug_stop_trace(),              false);
check('xdebug_get_function_count',      xdebug_get_function_count(),      0);
check('xdebug_start_function_monitor',  xdebug_start_function_monitor([]), null);
check('xdebug_stop_function_monitor',   xdebug_stop_function_monitor(),   null);
check('xdebug_get_monitored_functions', xdebug_get_monitored_functions(), []);

/* gcstats */
check('xdebug_get_gc_run_count',              xdebug_get_gc_run_count(),              0);
check('xdebug_get_gc_total_collected_roots',  xdebug_get_gc_total_collected_roots(),  0);
check('xdebug_get_gcstats_filename',          xdebug_get_gcstats_filename(),          false);
check('xdebug_start_gcstats',                 xdebug_start_gcstats(),                 false);
check('xdebug_stop_gcstats',                  xdebug_stop_gcstats(),                  false);

/* develop */
check('xdebug_call_class',            xdebug_call_class(),             false);
check('xdebug_call_file',             xdebug_call_file(),              false);
check('xdebug_call_function',         xdebug_call_function(),          false);
check('xdebug_call_line',             xdebug_call_line(),              0);
check('xdebug_debug_zval',            xdebug_debug_zval('x'),          null);
check('xdebug_debug_zval_stdout',     xdebug_debug_zval_stdout('x'),   null);
check('xdebug_dump_superglobals',     xdebug_dump_superglobals(),      null);
check('xdebug_get_collected_errors',  xdebug_get_collected_errors(),   []);
check('xdebug_get_function_stack',    xdebug_get_function_stack(),     []);
check('xdebug_get_stack_depth',       xdebug_get_stack_depth(),        0);
check('xdebug_memory_usage',          xdebug_memory_usage(),           0);
check('xdebug_peak_memory_usage',     xdebug_peak_memory_usage(),      0);
check('xdebug_print_function_stack',  xdebug_print_function_stack(),   null);
check('xdebug_start_error_collection', xdebug_start_error_collection(), null);
check('xdebug_stop_error_collection',  xdebug_stop_error_collection(),  null);
check('xdebug_time_index',            xdebug_time_index(),             0.0);
check('xdebug_var_dump',              xdebug_var_dump('x'),            null);

restore_error_handler();

echo "\n--- deprecations ---\n";
echo "count: ", count($deprecations), "\n";
$unique_funcs = [];
foreach ($deprecations as $msg) {
	if (preg_match('/^(xdebug_\w+)\(\)/', $msg, $m)) {
		$unique_funcs[$m[1]] = true;
	}
}
echo "unique funcs warned: ", count($unique_funcs), "\n";
?>
--EXPECT--
xdebug_get_profiler_filename: ok
xdebug_code_coverage_started: ok
xdebug_get_code_coverage: ok
xdebug_start_code_coverage: ok
xdebug_stop_code_coverage: ok
xdebug_get_tracefile_name: ok
xdebug_start_trace: ok
xdebug_stop_trace: ok
xdebug_get_function_count: ok
xdebug_start_function_monitor: ok
xdebug_stop_function_monitor: ok
xdebug_get_monitored_functions: ok
xdebug_get_gc_run_count: ok
xdebug_get_gc_total_collected_roots: ok
xdebug_get_gcstats_filename: ok
xdebug_start_gcstats: ok
xdebug_stop_gcstats: ok
xdebug_call_class: ok
xdebug_call_file: ok
xdebug_call_function: ok
xdebug_call_line: ok
xdebug_debug_zval: ok
xdebug_debug_zval_stdout: ok
xdebug_dump_superglobals: ok
xdebug_get_collected_errors: ok
xdebug_get_function_stack: ok
xdebug_get_stack_depth: ok
xdebug_memory_usage: ok
xdebug_peak_memory_usage: ok
xdebug_print_function_stack: ok
xdebug_start_error_collection: ok
xdebug_stop_error_collection: ok
xdebug_time_index: ok
xdebug_var_dump: ok

--- deprecations ---
count: 34
unique funcs warned: 34
