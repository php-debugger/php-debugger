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

int xdebug_frankenphp_in_use(void)
{
	return is_frankenphp;
}

/* Per-request reset, called at the start of each FrankenPHP worker request. */
static int xdebug_frankenphp_sapi_activate(void)
{
	int result = original_sapi_activate ? original_sapi_activate() : SUCCESS;

	if (!XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		return result;
	}

	/* Keep opcache's optimizer off. MINIT forces EXT_STMT for the whole
	 * process, but opcache stays enabled here, and the optimizer would move
	 * or drop the statements breakpoints resolve against. This has to happen
	 * per request rather than once at MINIT: a worker generation is a single
	 * PHP request, so the engine restores every INI value it altered when
	 * that generation ends, and MINIT does not run again when FrankenPHP
	 * starts the next one (max_requests, a watcher restart, a crash). */
	xdebug_disable_opcache_optimizer();

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
	 * the existing path in xdebug_debugger_statement_call() picks it up.
	 *
	 * Also re-arm the observer for the request: the worker's process-start
	 * RINIT may have left observer_active=false (no IDE at startup), and
	 * FrankenPHP does not run RINIT again per request — only sapi_activate.
	 * Without this, xdebug_execute_begin fast-paths out and never pushes a
	 * stack frame for the current function. xdebug_debugger_statement_call
	 * would then bail out on its empty-stack check, so breakpoints in the
	 * function that triggered the connection (and any function entered
	 * before observer activation) would be missed. See issue #63. */
	if (xdebug_lib_start_with_request() || has_debug_trigger()) {
		XG_DBG(context).do_connect_to_client = 1;
		XG_BASE(observer_active) = 1;
	} else {
		XG_BASE(observer_active) = 0;
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
	/* Note: at MINIT time on the FrankenPHP SAPI, sapi_module.name is set
	 * to "frankenphp" but sapi_module.activate may still be NULL — the SAPI
	 * fills the activate hook in later. We install our wrapper here, which
	 * the SAPI's per-request dispatch then invokes (FrankenPHP's worker loop
	 * calls activate/deactivate around each request even though it does not
	 * re-run RINIT/RSHUTDOWN). */
	if (!sapi_module.name || strcmp(sapi_module.name, "frankenphp") != 0) {
		return;
	}

	is_frankenphp = 1;

	original_sapi_activate    = sapi_module.activate;
	sapi_module.activate      = xdebug_frankenphp_sapi_activate;

	original_sapi_deactivate  = sapi_module.deactivate;
	sapi_module.deactivate    = xdebug_frankenphp_sapi_deactivate;

	/* In worker mode the per-request decision to debug is made by
	 * sapi_activate, but PHP_RINIT runs only once at worker startup —
	 * before any request has set a trigger. The normal RINIT path skips
	 * setting ZEND_COMPILE_EXTENDED_STMT when no IDE is connected. With
	 * the worker flow, that means user files compiled by later trigger
	 * requests still have no EXT_STMT opcodes, so line breakpoints can
	 * never resolve. Force it on for the whole process — the small
	 * per-statement overhead is acceptable for a SAPI that exists to
	 * serve interactive workloads. Unlike an INI setting this survives
	 * worker restarts, as the engine never resets compiler_options.
	 * The matching optimizer disable lives in sapi_activate. See issue
	 * #63. */
	CG(compiler_options) |= ZEND_COMPILE_EXTENDED_STMT;
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
