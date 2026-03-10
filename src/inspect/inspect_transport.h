/*
   +----------------------------------------------------------------------+
   | Xdebug — Runtime Inspection API Transport                            |
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

#ifndef __XDEBUG_INSPECT_TRANSPORT_H__
#define __XDEBUG_INSPECT_TRANSPORT_H__

#include "lib/php-header.h"

/* Inspection mode constants */
#define XDEBUG_INSPECT_OFF      0
#define XDEBUG_INSPECT_ON       1
#define XDEBUG_INSPECT_AUTO     2
#define XDEBUG_INSPECT_TRIGGER  3

/* Transport configuration defaults */
#define XDEBUG_INSPECT_PORT_DEFAULT    9007
#define XDEBUG_INSPECT_MAX_CLIENTS     16
#define XDEBUG_INSPECT_READ_BUFSIZE    65536  /* 64KB */

/* Lifecycle functions — called from xdebug.c */
int  inspect_transport_init(void);     /* MINIT: create listening socket */
void inspect_transport_check(void);    /* RINIT: accept new connections */
void inspect_transport_process(void);  /* periodic: read/dispatch messages */
int  inspect_transport_send(int client_id, const char *json_line);
int  inspect_transport_broadcast(const char *json_line);
void inspect_transport_shutdown(void); /* MSHUTDOWN: close everything */

/* INI helpers */
int  xdebug_inspect_parse_mode(const char *value);
const char *xdebug_inspect_mode_name(int mode);

#endif /* __XDEBUG_INSPECT_TRANSPORT_H__ */
