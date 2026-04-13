/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2025 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.01 of the Xdebug license,   |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | https://xdebug.org/license.php                                       |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | derick@xdebug.org so we can mail you a copy immediately.             |
   +----------------------------------------------------------------------+
 */

#ifndef __HAVE_XDEBUG_COMPAT_STUBS_H__
#define __HAVE_XDEBUG_COMPAT_STUBS_H__

/* Profiler stubs */
PHP_FUNCTION(xdebug_get_profiler_filename);

/* Coverage stubs */
PHP_FUNCTION(xdebug_code_coverage_started);
PHP_FUNCTION(xdebug_get_code_coverage);
PHP_FUNCTION(xdebug_start_code_coverage);
PHP_FUNCTION(xdebug_stop_code_coverage);

/* Tracing stubs */
PHP_FUNCTION(xdebug_get_tracefile_name);
PHP_FUNCTION(xdebug_start_trace);
PHP_FUNCTION(xdebug_stop_trace);
PHP_FUNCTION(xdebug_get_function_count);
PHP_FUNCTION(xdebug_start_function_monitor);
PHP_FUNCTION(xdebug_stop_function_monitor);
PHP_FUNCTION(xdebug_get_monitored_functions);

/* GC Stats stubs */
PHP_FUNCTION(xdebug_get_gc_run_count);
PHP_FUNCTION(xdebug_get_gc_total_collected_roots);
PHP_FUNCTION(xdebug_get_gcstats_filename);
PHP_FUNCTION(xdebug_start_gcstats);
PHP_FUNCTION(xdebug_stop_gcstats);

/* Develop stubs */
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

#endif /* __HAVE_XDEBUG_COMPAT_STUBS_H__ */
