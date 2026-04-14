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

#include <string.h>

#include "lib/php-header.h"
#include "SAPI.h"

#include "php_xdebug.h"
#include "com.h"
#include "debugger.h"
#include "frankenphp.h"
#include "lib/lib.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static int (*original_sapi_activate)(void)   = NULL;
static int (*original_sapi_deactivate)(void) = NULL;
static int is_frankenphp = 0;

/* Scan a 'k=v' style string (cookies separated by ';', query string by '&')
 * for one of the trigger variable names at a key boundary. */
static int has_trigger_in_string(const char *str, char delim)
{
	static const char *triggers[] = {
		"XDEBUG_SESSION", "XDEBUG_TRIGGER",
		"PHP_DEBUGGER_SESSION", "PHP_DEBUGGER_TRIGGER",
		NULL
	};
	const char **t;

	if (!str) {
		return 0;
	}

	for (t = triggers; *t; t++) {
		size_t len = strlen(*t);
		const char *p = str;

		while ((p = strstr(p, *t)) != NULL) {
			char prev = (p == str) ? delim : p[-1];
			char next = p[len];

			if ((prev == delim || prev == ' ') && next == '=') {
				return 1;
			}
			p += len;
		}
	}
	return 0;
}

static int has_debug_trigger(void)
{
	return has_trigger_in_string(SG(request_info).cookie_data, ';') ||
	       has_trigger_in_string(SG(request_info).query_string, '&');
}

/* Per-request reset, called at the start of each FrankenPHP worker request. */
static int xdebug_frankenphp_sapi_activate(void)
{
	int result = original_sapi_activate ? original_sapi_activate() : SUCCESS;

	if (!XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		return result;
	}

	/* Reset per-request debugger flags. The full RINIT path already ran once
	 * for the worker; here we only undo state that should not leak across
	 * requests inside the worker loop. */
	XG_DBG(detached)            = 0;
	XG_DBG(no_exec)             = 0;
	XG_DBG(breakpoints_allowed) = 1;

	XG_DBG(context).do_break             = 0;
	XG_DBG(context).do_step              = 0;
	XG_DBG(context).do_next              = 0;
	XG_DBG(context).do_finish            = 0;
	XG_DBG(context).do_connect_to_client = 0;

	/* Trigger detection: superglobals are not yet populated this early in the
	 * SAPI lifecycle, so we read the raw request data. If a trigger is present
	 * (or start_with_request=yes), arm the connect-on-next-statement flag —
	 * the existing path in xdebug_debugger_statement_call() picks it up. */
	if (xdebug_lib_start_with_request() || has_debug_trigger()) {
		XG_DBG(context).do_connect_to_client = 1;
	}

	/* If a debug session is still alive (e.g. user kept it open across
	 * requests), drain any breakpoint_set / breakpoint_remove the IDE pushed
	 * while the worker was busy. */
	if (xdebug_is_debug_connection_active() && XG_DBG(context).handler && XG_DBG(context).handler->remote_poll_pending) {
		XG_DBG(context).handler->remote_poll_pending(&(XG_DBG(context)));
	}

	return result;
}

/* Per-request teardown, called at the end of each FrankenPHP worker request.
 * Tears down the DBGp session so the next request starts fresh. */
static int xdebug_frankenphp_sapi_deactivate(void)
{
	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG) && xdebug_is_debug_connection_active()) {
		XG_DBG(context).handler->remote_deinit(&(XG_DBG(context)));
		xdebug_mark_debug_connection_not_active();
	}

	return original_sapi_deactivate ? original_sapi_deactivate() : SUCCESS;
}

void xdebug_frankenphp_minit(void)
{
	if (!sapi_module.name || strcmp(sapi_module.name, "frankenphp") != 0) {
		return;
	}

	is_frankenphp = 1;

	original_sapi_activate    = sapi_module.activate;
	sapi_module.activate      = xdebug_frankenphp_sapi_activate;

	original_sapi_deactivate  = sapi_module.deactivate;
	sapi_module.deactivate    = xdebug_frankenphp_sapi_deactivate;
}

void xdebug_frankenphp_mshutdown(void)
{
	if (!is_frankenphp) {
		return;
	}

	sapi_module.activate     = original_sapi_activate;
	sapi_module.deactivate   = original_sapi_deactivate;
	original_sapi_activate   = NULL;
	original_sapi_deactivate = NULL;
	is_frankenphp            = 0;
}
