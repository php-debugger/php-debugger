/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2024 Derick Rethans                               |
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "lib/php-header.h"
#include "main/php_version.h"
#include "lib/compat.h"

#if HAVE_XDEBUG

#ifndef PHP_WIN32
#include <sys/time.h>
#include <unistd.h>
#else
#include "win32/time.h"
#include <process.h>
#endif

#include "TSRM.h"
#include "SAPI.h"
#include "zend_extensions.h"
#include "main/php_ini.h"
#include "ext/standard/head.h"
#include "ext/standard/html.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "php_globals.h"
#include "main/php_output.h"
#include "ext/standard/php_var.h"

#include "php_xdebug.h"
#include "php_xdebug_arginfo.h"

#include "base/base.h"
#if HAVE_XDEBUG_CONTROL_SOCKET_SUPPORT
# include "base/ctrl_socket.h"
#endif
#include "base/filter.h"
#include "debugger/com.h"
#include "lib/usefulstuff.h"
#include "lib/lib.h"
#include "lib/llist.h"
#include "lib/log.h"
#include "lib/mm.h"
#include "lib/var_export_html.h"
#include "lib/var_export_line.h"
#include "lib/var_export_text.h"

static zend_result (*xdebug_orig_post_startup_cb)(void);
static zend_result xdebug_post_startup(void);

int xdebug_include_or_eval_handler(zend_execute_data *execute_data);

/* True globals */
int zend_xdebug_initialised = 0;
int xdebug_global_mode = 0;

zend_module_entry xdebug_module_entry = {
	STANDARD_MODULE_HEADER,
	"php_debugger",
	ext_functions,
	PHP_MINIT(xdebug),
	PHP_MSHUTDOWN(xdebug),
	PHP_RINIT(xdebug),
	PHP_RSHUTDOWN(xdebug),
	PHP_MINFO(xdebug),
	XDEBUG_VERSION,
	NO_MODULE_GLOBALS,
	ZEND_MODULE_POST_ZEND_DEACTIVATE_N(xdebug),
	STANDARD_MODULE_PROPERTIES_EX
};

ZEND_DECLARE_MODULE_GLOBALS(xdebug)

#if COMPILE_DL_PHP_DEBUGGER
ZEND_GET_MODULE(xdebug)
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE();
# endif
#endif

static PHP_INI_MH(OnUpdateStartWithRequest)
{
	if (!new_value) {
		return FAILURE;
	}

	if (!xdebug_lib_set_start_with_request(ZSTR_VAL(new_value))) {
		return FAILURE;
	}

	return SUCCESS;
}

static PHP_INI_MH(OnUpdateStartUponError)
{
	if (!new_value) {
		return FAILURE;
	}

	if (!xdebug_lib_set_start_upon_error(ZSTR_VAL(new_value))) {
		return FAILURE;
	}

	return SUCCESS;
}

#if HAVE_XDEBUG_CONTROL_SOCKET_SUPPORT
static PHP_INI_MH(OnUpdateCtrlSocket)
{
	if (!new_value) {
		return FAILURE;
	}

	if (!xdebug_lib_set_control_socket_granularity(ZSTR_VAL(new_value))) {
		return FAILURE;
	}

	return SUCCESS;
}
#endif

#ifdef P_tmpdir
# define XDEBUG_TEMP_DIR P_tmpdir
#else
# ifdef PHP_WIN32
#  define XDEBUG_TEMP_DIR "C:\\Windows\\Temp"
# else
#  define XDEBUG_TEMP_DIR "/tmp"
# endif
#endif

ZEND_INI_DISP(display_removed_setting)
{
	ZEND_PUTS("(setting removed in Xdebug 3)");
}

ZEND_INI_DISP(display_changed_setting)
{
	ZEND_PUTS("(setting renamed in Xdebug 3)");
}

#if HAVE_XDEBUG_CONTROL_SOCKET_SUPPORT
ZEND_INI_DISP(display_control_socket)
{
	switch (XINI_BASE(control_socket_granularity))
	{
		case XDEBUG_CONTROL_SOCKET_OFF:
			ZEND_PUTS("off");
			break;
		case XDEBUG_CONTROL_SOCKET_DEFAULT:
			php_printf("time: %ldms", XINI_BASE(control_socket_threshold_ms));
			break;
		case XDEBUG_CONTROL_SOCKET_TIME:
			php_printf("time: %ldms", XINI_BASE(control_socket_threshold_ms));
			break;
	}
}
#endif

static const char *xdebug_start_with_request_types[5] = { "", "default", "yes", "no", "trigger" };

ZEND_INI_DISP(display_start_with_request)
{
	char *value;

	if (type == ZEND_INI_DISPLAY_ORIG && ini_entry->modified) {
		value = ZSTR_VAL(ini_entry->orig_value);
	} else if (ini_entry->value) {
		value = ZSTR_VAL(ini_entry->value);
	} else {
		value = NULL;
	}
	if (value) {
		ZEND_PUTS(xdebug_start_with_request_types[xdebug_lib_get_start_with_request()]);
	} else {
		ZEND_PUTS("?");
	}
}

static const char *xdebug_start_upon_error_types[4] = { "", "default", "yes", "no" };

ZEND_INI_DISP(display_start_upon_error)
{
	char *value;

	if (type == ZEND_INI_DISPLAY_ORIG && ini_entry->modified) {
		value = ZSTR_VAL(ini_entry->orig_value);
	} else if (ini_entry->value) {
		value = ZSTR_VAL(ini_entry->value);
	} else {
		value = NULL;
	}
	if (value) {
		ZEND_PUTS(xdebug_start_upon_error_types[xdebug_lib_get_start_upon_error()]);
	} else {
		ZEND_PUTS("?");
	}
}

#if HAVE_XDEBUG_ZLIB
# define USE_COMPRESSION_DEFAULT "1"
#else
# define USE_COMPRESSION_DEFAULT "0"
#endif


/*
 * php_debugger.* INI alias wrappers.
 * Only apply when the user explicitly set the directive; skip defaults
 * to avoid overwriting xdebug.* values. On success, sync the canonical
 * xdebug.* entry so ini_get() reflects the effective value.
 */

static inline bool php_debugger_ini_is_explicitly_set(zend_ini_entry *entry)
{
	if (entry->modified) return true;
	/* During MINIT entry->modified is always 0; fall back to config scanner */
	return zend_get_configuration_directive(entry->name) != NULL;
}

static void php_debugger_sync_canonical(zend_ini_entry *alias_entry, zend_string *new_value)
{
	const char *alias_name = ZSTR_VAL(alias_entry->name);
	if (strncmp(alias_name, "php_debugger.", sizeof("php_debugger.") - 1) != 0) return;

	char canonical[128];
	snprintf(canonical, sizeof(canonical), "xdebug.%s", alias_name + sizeof("php_debugger.") - 1);

	zend_string *key = zend_string_init(canonical, strlen(canonical), 0);
	zend_ini_entry *canon = zend_hash_find_ptr(EG(ini_directives), key);
	zend_string_release(key);
	if (!canon) return;

	if (canon->value) zend_string_release(canon->value);
	canon->value = zend_string_copy(new_value);
}
#define PHP_DEBUGGER_INI_WRAPPER(name, delegate) \
	static PHP_INI_MH(name) { \
		if (!php_debugger_ini_is_explicitly_set(entry)) return SUCCESS; \
		int rc = delegate(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage); \
		if (rc == SUCCESS) php_debugger_sync_canonical(entry, new_value); \
		return rc; \
	}

PHP_DEBUGGER_INI_WRAPPER(OnUpdatePhpDebuggerString,           OnUpdateString)
PHP_DEBUGGER_INI_WRAPPER(OnUpdatePhpDebuggerLong,             OnUpdateLong)
PHP_DEBUGGER_INI_WRAPPER(OnUpdatePhpDebuggerBool,             OnUpdateBool)
PHP_DEBUGGER_INI_WRAPPER(OnUpdatePhpDebuggerStartWithRequest, OnUpdateStartWithRequest)
PHP_DEBUGGER_INI_WRAPPER(OnUpdatePhpDebuggerStartUponError,   OnUpdateStartUponError)

#if HAVE_XDEBUG_CONTROL_SOCKET_SUPPORT
PHP_DEBUGGER_INI_WRAPPER(OnUpdatePhpDebuggerCtrlSocket,       OnUpdateCtrlSocket)
#endif

PHP_INI_BEGIN()
	/* Library settings */
	STD_PHP_INI_ENTRY("xdebug.mode",               "debug",               PHP_INI_SYSTEM,                OnUpdateString, settings.library.requested_mode,   zend_xdebug_globals, xdebug_globals)
	PHP_INI_ENTRY_EX( "xdebug.start_with_request", "default",               PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateStartWithRequest, display_start_with_request)
	PHP_INI_ENTRY_EX( "xdebug.start_upon_error",   "default",               PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateStartUponError,   display_start_upon_error)
	STD_PHP_INI_ENTRY("xdebug.output_dir",         XDEBUG_TEMP_DIR,         PHP_INI_ALL,                   OnUpdateString, settings.library.output_dir,       zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.use_compression",    USE_COMPRESSION_DEFAULT, PHP_INI_ALL,                   OnUpdateBool,   settings.library.use_compression,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trigger_value",      "",                      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, settings.library.trigger_value,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.file_link_format",   "",                      PHP_INI_ALL,                   OnUpdateString, settings.library.file_link_format, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.filename_format",    "",                      PHP_INI_ALL,                   OnUpdateString, settings.library.filename_format,  zend_xdebug_globals, xdebug_globals)
#if HAVE_XDEBUG_CONTROL_SOCKET_SUPPORT
	PHP_INI_ENTRY_EX("xdebug.control_socket",      "default",               PHP_INI_ALL,                   OnUpdateCtrlSocket, display_control_socket)
#endif
	STD_PHP_INI_ENTRY("xdebug.path_mapping",       "0",                     PHP_INI_ALL,                   OnUpdateBool,   settings.library.path_mapping,     zend_xdebug_globals, xdebug_globals)

	STD_PHP_INI_ENTRY("xdebug.log",       "",           PHP_INI_ALL, OnUpdateString, settings.library.log,       zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.log_level", XLOG_DEFAULT, PHP_INI_ALL, OnUpdateLong,   settings.library.log_level, zend_xdebug_globals, xdebug_globals)

	/* Variable display settings */
	STD_PHP_INI_ENTRY("xdebug.var_display_max_children", "128",     PHP_INI_ALL,    OnUpdateLong,   settings.library.display_max_children, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.var_display_max_data",     "512",     PHP_INI_ALL,    OnUpdateLong,   settings.library.display_max_data,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.var_display_max_depth",    "3",       PHP_INI_ALL,    OnUpdateLong,   settings.library.display_max_depth,    zend_xdebug_globals, xdebug_globals)

	/* Base settings */
	STD_PHP_INI_ENTRY("xdebug.max_nesting_level", "512",                PHP_INI_ALL,    OnUpdateLong,   settings.base.max_nesting_level, zend_xdebug_globals, xdebug_globals)

	/* Xdebug Cloud */
	STD_PHP_INI_ENTRY("xdebug.cloud_id", "", PHP_INI_SYSTEM, OnUpdateString, settings.debugger.cloud_id, zend_xdebug_globals, xdebug_globals)

	/* Step debugger settings */
	STD_PHP_INI_ENTRY("xdebug.client_host",             "localhost",                        PHP_INI_ALL, OnUpdateString, settings.debugger.client_host,             zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.client_port",             XDEBUG_CLIENT_PORT_S,               PHP_INI_ALL, OnUpdateLong,   settings.debugger.client_port,             zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.discover_client_host",  "0",                                PHP_INI_ALL, OnUpdateBool,   settings.debugger.discover_client_host,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.client_discovery_header", "HTTP_X_FORWARDED_FOR,REMOTE_ADDR", PHP_INI_ALL, OnUpdateString, settings.debugger.client_discovery_header, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.idekey",                  "",                                 PHP_INI_ALL, OnUpdateString, settings.debugger.ide_key_setting,         zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.connect_timeout_ms",      "200",                              PHP_INI_ALL, OnUpdateLong,   settings.debugger.connect_timeout_ms,      zend_xdebug_globals, xdebug_globals)

PHP_INI_END()

/* php_debugger.* INI aliases — same storage as xdebug.* */
static const zend_ini_entry_def php_debugger_ini_entries[] = {
	/* Library settings */
	STD_PHP_INI_ENTRY("php_debugger.mode",               "debug",               PHP_INI_SYSTEM,                OnUpdatePhpDebuggerString, settings.library.requested_mode,   zend_xdebug_globals, xdebug_globals)
	PHP_INI_ENTRY_EX( "php_debugger.start_with_request", "default",               PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdatePhpDebuggerStartWithRequest, display_start_with_request)
	PHP_INI_ENTRY_EX( "php_debugger.start_upon_error",   "default",               PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdatePhpDebuggerStartUponError,   display_start_upon_error)
	STD_PHP_INI_ENTRY("php_debugger.output_dir",         XDEBUG_TEMP_DIR,         PHP_INI_ALL,                   OnUpdatePhpDebuggerString, settings.library.output_dir,       zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("php_debugger.use_compression",    USE_COMPRESSION_DEFAULT, PHP_INI_ALL,                   OnUpdatePhpDebuggerBool,   settings.library.use_compression,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("php_debugger.trigger_value",      "",                      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdatePhpDebuggerString, settings.library.trigger_value,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("php_debugger.file_link_format",   "",                      PHP_INI_ALL,                   OnUpdatePhpDebuggerString, settings.library.file_link_format, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("php_debugger.filename_format",    "",                      PHP_INI_ALL,                   OnUpdatePhpDebuggerString, settings.library.filename_format,  zend_xdebug_globals, xdebug_globals)
#if HAVE_XDEBUG_CONTROL_SOCKET_SUPPORT
	PHP_INI_ENTRY_EX("php_debugger.control_socket",      "default",               PHP_INI_ALL,                   OnUpdatePhpDebuggerCtrlSocket, display_control_socket)
#endif
	STD_PHP_INI_ENTRY("php_debugger.path_mapping",       "0",                     PHP_INI_ALL,                   OnUpdatePhpDebuggerBool,   settings.library.path_mapping,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("php_debugger.log",       "",           PHP_INI_ALL, OnUpdatePhpDebuggerString, settings.library.log,       zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("php_debugger.log_level", XLOG_DEFAULT, PHP_INI_ALL, OnUpdatePhpDebuggerLong,   settings.library.log_level, zend_xdebug_globals, xdebug_globals)
	/* Variable display settings */
	STD_PHP_INI_ENTRY("php_debugger.var_display_max_children", "128",     PHP_INI_ALL,    OnUpdatePhpDebuggerLong,   settings.library.display_max_children, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("php_debugger.var_display_max_data",     "512",     PHP_INI_ALL,    OnUpdatePhpDebuggerLong,   settings.library.display_max_data,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("php_debugger.var_display_max_depth",    "3",       PHP_INI_ALL,    OnUpdatePhpDebuggerLong,   settings.library.display_max_depth,    zend_xdebug_globals, xdebug_globals)
	/* Base settings */
	STD_PHP_INI_ENTRY("php_debugger.max_nesting_level", "512",                PHP_INI_ALL,    OnUpdatePhpDebuggerLong,   settings.base.max_nesting_level, zend_xdebug_globals, xdebug_globals)
	/* Cloud */
	STD_PHP_INI_ENTRY("php_debugger.cloud_id", "", PHP_INI_SYSTEM, OnUpdatePhpDebuggerString, settings.debugger.cloud_id, zend_xdebug_globals, xdebug_globals)
	/* Step debugger settings */
	STD_PHP_INI_ENTRY("php_debugger.client_host",             "localhost",                        PHP_INI_ALL, OnUpdatePhpDebuggerString, settings.debugger.client_host,             zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("php_debugger.client_port",             XDEBUG_CLIENT_PORT_S,               PHP_INI_ALL, OnUpdatePhpDebuggerLong,   settings.debugger.client_port,             zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("php_debugger.discover_client_host",  "0",                                PHP_INI_ALL, OnUpdatePhpDebuggerBool,   settings.debugger.discover_client_host,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("php_debugger.client_discovery_header", "HTTP_X_FORWARDED_FOR,REMOTE_ADDR", PHP_INI_ALL, OnUpdatePhpDebuggerString, settings.debugger.client_discovery_header, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("php_debugger.idekey",                  "",                                 PHP_INI_ALL, OnUpdatePhpDebuggerString, settings.debugger.ide_key_setting,         zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("php_debugger.connect_timeout_ms",      "200",                              PHP_INI_ALL, OnUpdatePhpDebuggerLong,   settings.debugger.connect_timeout_ms,      zend_xdebug_globals, xdebug_globals)
	{0}
};


static void xdebug_init_base_globals(xdebug_base_globals_t *xg)
{
	xg->stack                = NULL;
	xg->in_debug_info        = 0;
	xg->output_is_tty        = OUTPUT_NOT_CHECKED;
	xg->in_execution         = 0;
	xg->in_var_serialisation = 0;
	xg->error_reporting_override   = 0;
	xg->error_reporting_overridden = 0;
	xg->statement_handler_enabled  = false;

	xg->filter_type_stack         = XDEBUG_FILTER_NONE;
	xg->filters_stack             = NULL;

	xg->php_version_compile_time = PHP_VERSION;
	xg->php_version_run_time     = zend_get_module_version("standard");

	xdebug_nanotime_init(xg);
}

static void php_xdebug_init_globals(zend_xdebug_globals *xg)
{
	memset(&xg->globals, 0, sizeof(xg->globals));

	xdebug_init_library_globals(&xg->globals.library);
	xdebug_init_base_globals(&xg->globals.base);

	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		xdebug_init_debugger_globals(&xg->globals.debugger);
	}
}

static void php_xdebug_shutdown_globals(zend_xdebug_globals *xg)
{
}

static void xdebug_env_config(void)
{
	char       *config = getenv("XDEBUG_CONFIG");
	xdebug_arg *parts;
	int			i;
	/*
		XDEBUG_CONFIG / PHP_DEBUGGER_CONFIG format:
		XDEBUG_CONFIG=var=val var=val
	*/
	if (!config) {
		config = getenv("PHP_DEBUGGER_CONFIG");
	}
	if (!config) {
		return;
	}

	parts = xdebug_arg_ctor();
	xdebug_explode(" ", config, parts, -1);

	for (i = 0; i < parts->c; ++i) {
		const char *name = NULL;
		char *envvar = parts->args[i];
		char *envval = NULL;
		char *eq = strchr(envvar, '=');
		if (!eq || !*eq) {
			continue;
		}
		*eq = 0;
		envval = eq + 1;
		if (!*envval) {
			continue;
		}

		if (strcasecmp(envvar, "discover_client_host") == 0) {
			name = "xdebug.discover_client_host";
		} else
		if (strcasecmp(envvar, "client_port") == 0) {
			name = "xdebug.client_port";
		} else
		if (strcasecmp(envvar, "client_host") == 0) {
			name = "xdebug.client_host";
		} else
		if (strcasecmp(envvar, "cloud_id") == 0) {
			name = "xdebug.cloud_id";
		} else
		if (strcasecmp(envvar, "idekey") == 0) {
			name = "xdebug.idekey";
		} else
		if (strcasecmp(envvar, "output_dir") == 0) {
			name = "xdebug.output_dir";
		} else
		if (strcasecmp(envvar, "log") == 0) {
			name = "xdebug.log";
		} else
		if (strcasecmp(envvar, "log_level") == 0) {
			name = "xdebug.log_level";
		} else
		if (strcasecmp(envvar, "cli_color") == 0) {
			name = "xdebug.cli_color";
		}

		if (name) {
			zend_string *ini_name = zend_string_init(name, strlen(name), 0);
			zend_string *ini_val = zend_string_init(envval, strlen(envval), 0);
			zend_alter_ini_entry(ini_name, ini_val, PHP_INI_SYSTEM, PHP_INI_STAGE_ACTIVATE);
			zend_string_release(ini_val);
			zend_string_release(ini_name);
		}
	}

	xdebug_arg_dtor(parts);
}

int xdebug_is_output_tty(void)
{
	if (XG_BASE(output_is_tty) == OUTPUT_NOT_CHECKED) {
#ifndef PHP_WIN32
		XG_BASE(output_is_tty) = isatty(STDOUT_FILENO);
#else
		XG_BASE(output_is_tty) = getenv("ANSICON") != NULL;
#endif
	}
	return (XG_BASE(output_is_tty));
}

PHP_MINIT_FUNCTION(xdebug)
{
	ZEND_INIT_MODULE_GLOBALS(xdebug, php_xdebug_init_globals, php_xdebug_shutdown_globals);
	REGISTER_INI_ENTRIES();

	/* Register "xdebug" as a module alias so extension_loaded('xdebug') still works.
	 * Uses a separate dummy module entry to avoid double-free in module_registry cleanup. */
	{
		static zend_module_entry xdebug_compat_module_entry = {0};
		xdebug_compat_module_entry.name = "xdebug";
		xdebug_compat_module_entry.version = XDEBUG_VERSION;
		xdebug_compat_module_entry.type = MODULE_PERSISTENT;
		xdebug_compat_module_entry.module_number = module_number;
		xdebug_compat_module_entry.zend_api = ZEND_MODULE_API_NO;
		zend_string *alias_name = zend_string_init_interned("xdebug", sizeof("xdebug") - 1, 1);
		zend_hash_add_ptr(&module_registry, alias_name, &xdebug_compat_module_entry);
		zend_string_release(alias_name);
	}

	/* Register php_debugger.* INI aliases pointing to the same storage as xdebug.* */
	zend_register_ini_entries(php_debugger_ini_entries, module_number);

	xdebug_filter_register_constants(INIT_FUNC_ARGS_PASSTHRU);

	/* Locking in mode as it currently is */
	if (!xdebug_lib_set_mode(XG(settings.library.requested_mode))) {
		xdebug_lib_set_mode("debug");
	}

	if (XDEBUG_MODE_IS_OFF()) {
		return SUCCESS;
	}

	xdebug_library_minit();
	xdebug_base_minit(INIT_FUNC_ARGS_PASSTHRU);

	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		xdebug_debugger_minit();
	}

	/* Overload the "include_or_eval" opcode if the mode is 'debug' or 'trace' */
	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		xdebug_register_with_opcode_multi_handler(ZEND_INCLUDE_OR_EVAL, xdebug_include_or_eval_handler);
	}

	/* Coverage must be last, as it has a catch all override for opcodes */

	if (zend_xdebug_initialised == 0) {
		zend_error(E_WARNING, "PHP Debugger MUST be loaded as a Zend extension");
	}

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(xdebug)
{
	if (XDEBUG_MODE_IS_OFF()) {
#ifdef ZTS
		ts_free_id(xdebug_globals_id);
#endif
		return SUCCESS;
	}

	xdebug_library_mshutdown();

#ifdef ZTS
	ts_free_id(xdebug_globals_id);
#else
	php_xdebug_shutdown_globals(&xdebug_globals);
#endif

	return SUCCESS;
}

static void xdebug_init_auto_globals(void)
{
	zend_is_auto_global_str((char*) ZEND_STRL("_ENV"));
	zend_is_auto_global_str((char*) ZEND_STRL("_GET"));
	zend_is_auto_global_str((char*) ZEND_STRL("_POST"));
	zend_is_auto_global_str((char*) ZEND_STRL("_COOKIE"));
	zend_is_auto_global_str((char*) ZEND_STRL("_REQUEST"));
	zend_is_auto_global_str((char*) ZEND_STRL("_FILES"));
	zend_is_auto_global_str((char*) ZEND_STRL("_SERVER"));
	zend_is_auto_global_str((char*) ZEND_STRL("_SESSION"));
}

PHP_RINIT_FUNCTION(xdebug)
{
#if defined(ZTS) && defined(COMPILE_DL_XDEBUG)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	if (XDEBUG_MODE_IS_OFF()) {
		return SUCCESS;
	}

	/* Get xdebug ini entries from the environment also,
	   this can override the idekey if one is set */
	xdebug_env_config();

	xdebug_library_rinit();

	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		xdebug_debugger_rinit();

		if (xdebug_debugger_bailout_if_no_exec_requested()) {
			zend_bailout();
		}
	}

	xdebug_init_auto_globals();

	/* Early debug init: attempt connection at RINIT so observer_active is set
	 * before any user code runs. This allows xdebug_observer_init to return
	 * {NULL, NULL} for functions first-called when no debugger is connected. */
	xdebug_base_rinit();

	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		/* Check early if debugging could be requested this request.
		 * For start_with_request=default (trigger mode), check if any
		 * trigger is present. If not, disable all heavy hooks for
		 * near-zero overhead. The actual connection happens on first
		 * function call if triggers are present. */
		/* Check if debugging could be requested this request.
		 * For trigger/default mode: check triggers, cookies, env vars.
		 * For yes mode: always expect a connection.
		 * For no mode: no debugging will happen.
		 * Note: xdebug_break() can initiate connections without triggers,
		 * but it handles re-enabling the observer itself. */
		/* Respect start_with_request=no and XDEBUG_IGNORE */
		int debug_requested = !xdebug_lib_never_start_with_request() && !xdebug_should_ignore() && (
			xdebug_lib_start_with_request(XDEBUG_MODE_STEP_DEBUG) ||
			xdebug_lib_start_with_trigger(XDEBUG_MODE_STEP_DEBUG, NULL) ||
			xdebug_lib_start_upon_error() ||
			getenv("XDEBUG_SESSION_START") != NULL ||
			getenv("PHP_DEBUGGER_SESSION_START") != NULL
		);

		if (debug_requested) {
			/* Debug session requested: check if a client is actually listening
			 * before enabling expensive EXT_STMT opcodes. This avoids ~2x
			 * overhead when triggers are present but no IDE is connected. */
			if (xdebug_early_connect_to_client()) {
				CG(compiler_options) = CG(compiler_options) | ZEND_COMPILE_EXTENDED_STMT;
			} else {
				/* Trigger present but no client listening — stay dormant */
				XG_BASE(observer_active) = 0;
				XG_BASE(statement_handler_enabled) = false;
			}
		} else {
			/* No debug trigger: disable all heavy hooks for near-zero overhead.
			 * Note: xdebug_break() jit mode won't have full stepping support
			 * without EXT_STMT opcodes. Use start_with_request=yes or a trigger
			 * for full debugging support. */
			XG_BASE(observer_active) = 0;
			XG_BASE(statement_handler_enabled) = false;
		}
	}

	return SUCCESS;
}

ZEND_MODULE_POST_ZEND_DEACTIVATE_D(xdebug)
{
	if (XDEBUG_MODE_IS_OFF()) {
		return SUCCESS;
	}

	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		xdebug_debugger_post_deactivate();
	}

	xdebug_base_post_deactivate();
	xdebug_library_post_deactivate();

	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(xdebug)
{
	if (XDEBUG_MODE_IS_OFF()) {
		return SUCCESS;
	}

	xdebug_base_rshutdown();

	return SUCCESS;
}

PHP_MINFO_FUNCTION(xdebug)
{
	xdebug_print_info();

	if (zend_xdebug_initialised == 0) {
		php_info_print_table_start();
		php_info_print_table_header(1, "XDEBUG NOT LOADED AS ZEND EXTENSION");
		php_info_print_table_end();
	}

	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		xdebug_debugger_minfo();
	}

	DISPLAY_INI_ENTRIES();
}

ZEND_DLEXPORT void xdebug_statement_call(zend_execute_data *frame)
{
	zend_op_array *op_array;
	int            lineno;

	if (XDEBUG_MODE_IS_OFF()) {
		return;
	}

	if (!EG(current_execute_data)) {
		return;
	}

#if HAVE_XDEBUG_CONTROL_SOCKET_SUPPORT
	xdebug_control_socket_dispatch();
#endif

	if (!XG_BASE(statement_handler_enabled)) {
		return;
	}

	op_array = &frame->func->op_array;
	lineno = EG(current_execute_data)->opline->lineno;

	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		xdebug_debugger_statement_call(op_array->filename, lineno);
	}
}

ZEND_DLEXPORT int xdebug_zend_startup(zend_extension *extension)
{
	xdebug_library_zend_startup();
	xdebug_debugger_zend_startup();

	zend_xdebug_initialised = 1;

	xdebug_orig_post_startup_cb = zend_post_startup_cb;
	zend_post_startup_cb = xdebug_post_startup;

	return zend_startup_module(&xdebug_module_entry);
}

static zend_result xdebug_post_startup(void)
{
	if (xdebug_orig_post_startup_cb) {
		int (*cb)(void) = xdebug_orig_post_startup_cb;

		xdebug_orig_post_startup_cb = NULL;
		if (cb() != SUCCESS) {
			return FAILURE;
		}
	}

	xdebug_base_post_startup();

	return SUCCESS;
}

ZEND_DLEXPORT void xdebug_zend_shutdown(zend_extension *extension)
{
	xdebug_debugger_zend_shutdown();

	xdebug_library_zend_shutdown();
}

ZEND_DLEXPORT void xdebug_init_oparray(zend_op_array *op_array)
{
	if (XDEBUG_MODE_IS_OFF()) {
		return;
	}

}

#ifndef ZEND_EXT_API
#define ZEND_EXT_API    ZEND_DLEXPORT
#endif

ZEND_EXT_API zend_extension_version_info extension_version_info = { ZEND_EXTENSION_API_NO, (char*) ZEND_EXTENSION_BUILD_ID };

ZEND_DLEXPORT zend_extension zend_extension_entry = {
	(char*) XDEBUG_NAME,
	(char*) XDEBUG_VERSION,
	(char*) XDEBUG_AUTHOR,
	(char*) XDEBUG_URL_FAQ,
	(char*) XDEBUG_COPYRIGHT_SHORT,
	xdebug_zend_startup,
	xdebug_zend_shutdown,
	NULL,           /* activate_func_t */
	NULL,           /* deactivate_func_t */
	NULL,           /* message_handler_func_t */
	NULL,           /* op_array_handler_func_t */
	xdebug_statement_call, /* statement_handler_func_t */
	NULL,           /* fcall_begin_handler_func_t */
	NULL,           /* fcall_end_handler_func_t */
	xdebug_init_oparray,   /* op_array_ctor_func_t */
	NULL,           /* op_array_dtor_func_t */
	STANDARD_ZEND_EXTENSION_PROPERTIES
};

#endif
