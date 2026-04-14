/*
 * Compatibility stubs for stripped Xdebug functions.
 * See compat_stubs.c for details.
 */

#ifndef __HAVE_XDEBUG_COMPAT_STUBS_H__
#define __HAVE_XDEBUG_COMPAT_STUBS_H__

/* Profiler */
PHP_FUNCTION(xdebug_get_profiler_filename);

/* Coverage */
PHP_FUNCTION(xdebug_code_coverage_started);
PHP_FUNCTION(xdebug_get_code_coverage);
PHP_FUNCTION(xdebug_start_code_coverage);
PHP_FUNCTION(xdebug_stop_code_coverage);

/* Tracing */
PHP_FUNCTION(xdebug_get_tracefile_name);
PHP_FUNCTION(xdebug_start_trace);
PHP_FUNCTION(xdebug_stop_trace);
PHP_FUNCTION(xdebug_get_function_count);
PHP_FUNCTION(xdebug_start_function_monitor);
PHP_FUNCTION(xdebug_stop_function_monitor);
PHP_FUNCTION(xdebug_get_monitored_functions);

/* GC Stats */
PHP_FUNCTION(xdebug_get_gc_run_count);
PHP_FUNCTION(xdebug_get_gc_total_collected_roots);
PHP_FUNCTION(xdebug_get_gcstats_filename);
PHP_FUNCTION(xdebug_start_gcstats);
PHP_FUNCTION(xdebug_stop_gcstats);

/* Develop */
PHP_FUNCTION(xdebug_call_class);
PHP_FUNCTION(xdebug_call_file);
PHP_FUNCTION(xdebug_call_function);
PHP_FUNCTION(xdebug_call_line);
PHP_FUNCTION(xdebug_debug_zval);
PHP_FUNCTION(xdebug_debug_zval_stdout);
PHP_FUNCTION(xdebug_dump_superglobals);
PHP_FUNCTION(xdebug_get_collected_errors);
PHP_FUNCTION(xdebug_get_function_stack);
PHP_FUNCTION(xdebug_get_stack_depth);
PHP_FUNCTION(xdebug_memory_usage);
PHP_FUNCTION(xdebug_peak_memory_usage);
PHP_FUNCTION(xdebug_print_function_stack);
PHP_FUNCTION(xdebug_start_error_collection);
PHP_FUNCTION(xdebug_stop_error_collection);
PHP_FUNCTION(xdebug_time_index);
PHP_FUNCTION(xdebug_var_dump);

#endif
