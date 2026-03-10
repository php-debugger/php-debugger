/*
   +----------------------------------------------------------------------+
   | Xdebug — Runtime Inspection API Commands                             |
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

#ifndef __XDEBUG_INSPECT_COMMANDS_H__
#define __XDEBUG_INSPECT_COMMANDS_H__

#include "lib/php-header.h"

/*
 * Each command handler receives:
 *   client_fd  — the socket fd of the requesting client
 *   request_id — the "id" field from the JSON request (0 if absent)
 *   params     — the full parsed JSON request as a zval (IS_ARRAY)
 *
 * The handler must return a heap-allocated JSON string (the response line)
 * which the caller will send and then free with efree().
 * Return NULL to send no response.
 */
typedef char *(*inspect_cmd_handler_fn)(int client_fd, zend_long request_id, zval *params);

/* Command dispatch — finds and calls the right handler */
char *inspect_dispatch_command(int client_fd, const char *cmd, zend_long request_id, zval *params);

/* Stub command handlers */
char *inspect_cmd_status(int client_fd, zend_long request_id, zval *params);
char *inspect_cmd_inspect(int client_fd, zend_long request_id, zval *params);
char *inspect_cmd_snapshot(int client_fd, zend_long request_id, zval *params);
char *inspect_cmd_watch(int client_fd, zend_long request_id, zval *params);
char *inspect_cmd_trace(int client_fd, zend_long request_id, zval *params);
char *inspect_cmd_eval(int client_fd, zend_long request_id, zval *params);
char *inspect_cmd_profile_start(int client_fd, zend_long request_id, zval *params);
char *inspect_cmd_profile_stop(int client_fd, zend_long request_id, zval *params);
char *inspect_cmd_profile_status(int client_fd, zend_long request_id, zval *params);
char *inspect_cmd_profile_dump(int client_fd, zend_long request_id, zval *params);

#endif /* __XDEBUG_INSPECT_COMMANDS_H__ */
