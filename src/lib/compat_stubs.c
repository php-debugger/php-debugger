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

/*
 * Compatibility stubs for Xdebug functions that were removed when
 * non-debugger modules (profiler, coverage, tracing, gcstats, develop)
 * were stripped from php-debugger.
 *
 * Each stub emits E_DEPRECATED and returns a safe default value matching
 * what Xdebug returns when the corresponding feature is inactive.
 */

#include "lib/php-header.h"
#include "ext/standard/php_var.h"

#include "lib/compat_stubs.h"

/* ===== Profiler stubs ================================================= */

PHP_FUNCTION(xdebug_get_profiler_filename)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_get_profiler_filename() is not available in php-debugger, "
		"profiling support has been removed");
	RETURN_FALSE;
}

/* ===== Coverage stubs ================================================= */

PHP_FUNCTION(xdebug_code_coverage_started)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_code_coverage_started() is not available in php-debugger, "
		"coverage support has been removed");
	RETURN_FALSE;
}

PHP_FUNCTION(xdebug_get_code_coverage)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_get_code_coverage() is not available in php-debugger, "
		"coverage support has been removed");
	RETURN_EMPTY_ARRAY();
}

PHP_FUNCTION(xdebug_start_code_coverage)
{
	zend_long options = 0;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(options)
	ZEND_PARSE_PARAMETERS_END();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_start_code_coverage() is not available in php-debugger, "
		"coverage support has been removed");
}

PHP_FUNCTION(xdebug_stop_code_coverage)
{
	bool clean_up = 1;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(clean_up)
	ZEND_PARSE_PARAMETERS_END();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_stop_code_coverage() is not available in php-debugger, "
		"coverage support has been removed");
}

/* ===== Tracing stubs ================================================== */

PHP_FUNCTION(xdebug_get_tracefile_name)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_get_tracefile_name() is not available in php-debugger, "
		"tracing support has been removed");
	RETURN_FALSE;
}

PHP_FUNCTION(xdebug_start_trace)
{
	char *trace_file = NULL;
	size_t trace_file_len = 0;
	zend_long options = 0;

	ZEND_PARSE_PARAMETERS_START(0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING_OR_NULL(trace_file, trace_file_len)
		Z_PARAM_LONG(options)
	ZEND_PARSE_PARAMETERS_END();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_start_trace() is not available in php-debugger, "
		"tracing support has been removed");
	RETURN_FALSE;
}

PHP_FUNCTION(xdebug_stop_trace)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_stop_trace() is not available in php-debugger, "
		"tracing support has been removed");
	RETURN_FALSE;
}

PHP_FUNCTION(xdebug_get_function_count)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_get_function_count() is not available in php-debugger, "
		"tracing support has been removed");
	RETURN_LONG(0);
}

PHP_FUNCTION(xdebug_start_function_monitor)
{
	zval *function_names;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ARRAY(function_names)
	ZEND_PARSE_PARAMETERS_END();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_start_function_monitor() is not available in php-debugger, "
		"tracing support has been removed");
}

PHP_FUNCTION(xdebug_stop_function_monitor)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_stop_function_monitor() is not available in php-debugger, "
		"tracing support has been removed");
}

PHP_FUNCTION(xdebug_get_monitored_functions)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_get_monitored_functions() is not available in php-debugger, "
		"tracing support has been removed");
	RETURN_EMPTY_ARRAY();
}

/* ===== GC Stats stubs ================================================= */

PHP_FUNCTION(xdebug_get_gc_run_count)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_get_gc_run_count() is not available in php-debugger, "
		"gcstats support has been removed");
	RETURN_LONG(0);
}

PHP_FUNCTION(xdebug_get_gc_total_collected_roots)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_get_gc_total_collected_roots() is not available in php-debugger, "
		"gcstats support has been removed");
	RETURN_LONG(0);
}

PHP_FUNCTION(xdebug_get_gcstats_filename)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_get_gcstats_filename() is not available in php-debugger, "
		"gcstats support has been removed");
	RETURN_FALSE;
}

PHP_FUNCTION(xdebug_start_gcstats)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_start_gcstats() is not available in php-debugger, "
		"gcstats support has been removed");
	RETURN_FALSE;
}

PHP_FUNCTION(xdebug_stop_gcstats)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_stop_gcstats() is not available in php-debugger, "
		"gcstats support has been removed");
	RETURN_FALSE;
}

/* ===== Develop stubs ================================================== */

PHP_FUNCTION(xdebug_call_class)
{
	zend_long depth = 2;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(depth)
	ZEND_PARSE_PARAMETERS_END();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_call_class() is not available in php-debugger, "
		"develop support has been removed");
	RETURN_FALSE;
}

PHP_FUNCTION(xdebug_call_file)
{
	zend_long depth = 2;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(depth)
	ZEND_PARSE_PARAMETERS_END();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_call_file() is not available in php-debugger, "
		"develop support has been removed");
	RETURN_FALSE;
}

PHP_FUNCTION(xdebug_call_function)
{
	zend_long depth = 2;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(depth)
	ZEND_PARSE_PARAMETERS_END();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_call_function() is not available in php-debugger, "
		"develop support has been removed");
	RETURN_FALSE;
}

PHP_FUNCTION(xdebug_call_line)
{
	zend_long depth = 2;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(depth)
	ZEND_PARSE_PARAMETERS_END();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_call_line() is not available in php-debugger, "
		"develop support has been removed");
	RETURN_LONG(0);
}

PHP_FUNCTION(xdebug_debug_zval)
{
	zval *args;
	int argc;

	ZEND_PARSE_PARAMETERS_START(1, -1)
		Z_PARAM_VARIADIC('+', args, argc)
	ZEND_PARSE_PARAMETERS_END();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_debug_zval() is not available in php-debugger, "
		"develop support has been removed");
}

PHP_FUNCTION(xdebug_debug_zval_stdout)
{
	zval *args;
	int argc;

	ZEND_PARSE_PARAMETERS_START(1, -1)
		Z_PARAM_VARIADIC('+', args, argc)
	ZEND_PARSE_PARAMETERS_END();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_debug_zval_stdout() is not available in php-debugger, "
		"develop support has been removed");
}

PHP_FUNCTION(xdebug_dump_superglobals)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_dump_superglobals() is not available in php-debugger, "
		"develop support has been removed");
}

PHP_FUNCTION(xdebug_get_collected_errors)
{
	bool empty_list = 0;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(empty_list)
	ZEND_PARSE_PARAMETERS_END();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_get_collected_errors() is not available in php-debugger, "
		"develop support has been removed");
	RETURN_EMPTY_ARRAY();
}

PHP_FUNCTION(xdebug_get_function_stack)
{
	zval *options = NULL;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ARRAY(options)
	ZEND_PARSE_PARAMETERS_END();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_get_function_stack() is not available in php-debugger, "
		"develop support has been removed");
	RETURN_EMPTY_ARRAY();
}

PHP_FUNCTION(xdebug_get_stack_depth)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_get_stack_depth() is not available in php-debugger, "
		"develop support has been removed");
	RETURN_LONG(0);
}

PHP_FUNCTION(xdebug_memory_usage)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_memory_usage() is not available in php-debugger, "
		"develop support has been removed");
	RETURN_LONG(0);
}

PHP_FUNCTION(xdebug_peak_memory_usage)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_peak_memory_usage() is not available in php-debugger, "
		"develop support has been removed");
	RETURN_LONG(0);
}

PHP_FUNCTION(xdebug_print_function_stack)
{
	char *message = NULL;
	size_t message_len = 0;
	zend_long options = 0;

	ZEND_PARSE_PARAMETERS_START(0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(message, message_len)
		Z_PARAM_LONG(options)
	ZEND_PARSE_PARAMETERS_END();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_print_function_stack() is not available in php-debugger, "
		"develop support has been removed");
}

PHP_FUNCTION(xdebug_start_error_collection)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_start_error_collection() is not available in php-debugger, "
		"develop support has been removed");
}

PHP_FUNCTION(xdebug_stop_error_collection)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_stop_error_collection() is not available in php-debugger, "
		"develop support has been removed");
}

PHP_FUNCTION(xdebug_time_index)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_time_index() is not available in php-debugger, "
		"develop support has been removed");
	RETURN_DOUBLE(0.0);
}

PHP_FUNCTION(xdebug_var_dump)
{
	zval *args;
	int argc;

	ZEND_PARSE_PARAMETERS_START(1, -1)
		Z_PARAM_VARIADIC('+', args, argc)
	ZEND_PARSE_PARAMETERS_END();

	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_var_dump() is not available in php-debugger, "
		"develop support has been removed");

	/* Fall through to PHP's native var_dump */
	for (int i = 0; i < argc; i++) {
		php_var_dump(&args[i], 1);
	}
}
