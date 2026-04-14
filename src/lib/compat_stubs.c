/*
 * Compatibility stubs for Xdebug functions that were removed when
 * non-debugger modules (profiler, coverage, tracing, gcstats, develop)
 * were stripped from php-debugger.
 *
 * Each stub emits E_DEPRECATED and returns a safe default value.
 */

#include "lib/php-header.h"

#include "lib/compat_stubs.h"

/* --- Stub generators -------------------------------------------------- */

#define COMPAT_STUB_RETURN_FALSE(func, module) \
PHP_FUNCTION(func) { \
	ZEND_PARSE_PARAMETERS_NONE(); \
	php_error_docref(NULL, E_DEPRECATED, \
		#func "() is not available in php-debugger, " \
		module " support has been removed"); \
	RETURN_FALSE; \
}

#define COMPAT_STUB_RETURN_LONG(func, module, val) \
PHP_FUNCTION(func) { \
	ZEND_PARSE_PARAMETERS_NONE(); \
	php_error_docref(NULL, E_DEPRECATED, \
		#func "() is not available in php-debugger, " \
		module " support has been removed"); \
	RETURN_LONG(val); \
}

#define COMPAT_STUB_RETURN_EMPTY_ARRAY(func, module) \
PHP_FUNCTION(func) { \
	ZEND_PARSE_PARAMETERS_NONE(); \
	php_error_docref(NULL, E_DEPRECATED, \
		#func "() is not available in php-debugger, " \
		module " support has been removed"); \
	RETURN_EMPTY_ARRAY(); \
}

#define COMPAT_STUB_VOID(func, module) \
PHP_FUNCTION(func) { \
	ZEND_PARSE_PARAMETERS_NONE(); \
	php_error_docref(NULL, E_DEPRECATED, \
		#func "() is not available in php-debugger, " \
		module " support has been removed"); \
}

#define COMPAT_STUB_RETURN_DOUBLE(func, module, val) \
PHP_FUNCTION(func) { \
	ZEND_PARSE_PARAMETERS_NONE(); \
	php_error_docref(NULL, E_DEPRECATED, \
		#func "() is not available in php-debugger, " \
		module " support has been removed"); \
	RETURN_DOUBLE(val); \
}

/* Helper for stubs that accept an optional long and return void */
#define COMPAT_STUB_VOID_OPT_LONG(func, module, param, defval) \
PHP_FUNCTION(func) { \
	zend_long param = defval; \
	ZEND_PARSE_PARAMETERS_START(0, 1) \
		Z_PARAM_OPTIONAL \
		Z_PARAM_LONG(param) \
	ZEND_PARSE_PARAMETERS_END(); \
	php_error_docref(NULL, E_DEPRECATED, \
		#func "() is not available in php-debugger, " \
		module " support has been removed"); \
}

/* Helper for stubs that accept an optional long depth and return false */
#define COMPAT_STUB_DEPTH_FALSE(func, module) \
PHP_FUNCTION(func) { \
	zend_long depth = 2; \
	ZEND_PARSE_PARAMETERS_START(0, 1) \
		Z_PARAM_OPTIONAL \
		Z_PARAM_LONG(depth) \
	ZEND_PARSE_PARAMETERS_END(); \
	php_error_docref(NULL, E_DEPRECATED, \
		#func "() is not available in php-debugger, " \
		module " support has been removed"); \
	RETURN_FALSE; \
}

/* --- Profiler --------------------------------------------------------- */

COMPAT_STUB_RETURN_FALSE(xdebug_get_profiler_filename, "profiling")

/* --- Coverage --------------------------------------------------------- */

COMPAT_STUB_RETURN_FALSE(xdebug_code_coverage_started, "coverage")
COMPAT_STUB_RETURN_EMPTY_ARRAY(xdebug_get_code_coverage, "coverage")
COMPAT_STUB_VOID_OPT_LONG(xdebug_start_code_coverage, "coverage", options, 0)

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

/* --- Tracing ---------------------------------------------------------- */

COMPAT_STUB_RETURN_FALSE(xdebug_get_tracefile_name, "tracing")

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

COMPAT_STUB_RETURN_FALSE(xdebug_stop_trace, "tracing")
COMPAT_STUB_RETURN_LONG(xdebug_get_function_count, "tracing", 0)

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

COMPAT_STUB_VOID(xdebug_stop_function_monitor, "tracing")
COMPAT_STUB_RETURN_EMPTY_ARRAY(xdebug_get_monitored_functions, "tracing")

/* --- GC Stats --------------------------------------------------------- */

COMPAT_STUB_RETURN_LONG(xdebug_get_gc_run_count, "gcstats", 0)
COMPAT_STUB_RETURN_LONG(xdebug_get_gc_total_collected_roots, "gcstats", 0)
COMPAT_STUB_RETURN_FALSE(xdebug_get_gcstats_filename, "gcstats")
COMPAT_STUB_RETURN_FALSE(xdebug_start_gcstats, "gcstats")
COMPAT_STUB_RETURN_FALSE(xdebug_stop_gcstats, "gcstats")

/* --- Develop ---------------------------------------------------------- */

COMPAT_STUB_DEPTH_FALSE(xdebug_call_class, "develop")
COMPAT_STUB_DEPTH_FALSE(xdebug_call_file, "develop")
COMPAT_STUB_DEPTH_FALSE(xdebug_call_function, "develop")

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
		Z_PARAM_VARIADIC('s', args, argc)
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
		Z_PARAM_VARIADIC('s', args, argc)
	ZEND_PARSE_PARAMETERS_END();
	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_debug_zval_stdout() is not available in php-debugger, "
		"develop support has been removed");
}

COMPAT_STUB_VOID(xdebug_dump_superglobals, "develop")

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

COMPAT_STUB_RETURN_LONG(xdebug_get_stack_depth, "develop", 0)
COMPAT_STUB_RETURN_LONG(xdebug_memory_usage, "develop", 0)
COMPAT_STUB_RETURN_LONG(xdebug_peak_memory_usage, "develop", 0)

PHP_FUNCTION(xdebug_print_function_stack)
{
	char *message = (char *)"user triggered";
	size_t message_len = sizeof("user triggered") - 1;
	zend_long options = 0;
	ZEND_PARSE_PARAMETERS_START(0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(message, message_len)
		Z_PARAM_LONG(options)
	ZEND_PARSE_PARAMETERS_END();
	(void)message;
	(void)message_len;
	(void)options;
	php_error_docref(NULL, E_DEPRECATED,
		"xdebug_print_function_stack() is not available in php-debugger, "
		"develop support has been removed");
}

COMPAT_STUB_VOID(xdebug_start_error_collection, "develop")
COMPAT_STUB_VOID(xdebug_stop_error_collection, "develop")
COMPAT_STUB_RETURN_DOUBLE(xdebug_time_index, "develop", 0.0)

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
}
