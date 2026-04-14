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

#ifndef __XDEBUG_DEBUGGER_FRANKENPHP_H__
#define __XDEBUG_DEBUGGER_FRANKENPHP_H__

/*
 * FrankenPHP worker mode support.
 *
 * In FrankenPHP worker mode, MINIT/RINIT/RSHUTDOWN/MSHUTDOWN run once per
 * worker (not per request). Per-request setup happens through
 * sapi_module.activate / deactivate. We hook those to drive a per-request
 * debugger lifecycle so the debug session is reset between requests.
 *
 * If the SAPI is not "frankenphp", these functions are no-ops.
 */
void xdebug_frankenphp_minit(void);
void xdebug_frankenphp_mshutdown(void);

#endif /* __XDEBUG_DEBUGGER_FRANKENPHP_H__ */
