/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2026 Derick Rethans                               |
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

#ifndef __HAVE_USEFULSTUFF_H__
#define __HAVE_USEFULSTUFF_H__

#include "php_xdebug.h"
#include "src/lib/compat.h"

bool xdebug_is_printable(const char *str, size_t len);

char *xdebug_zstr_path_to_url(zend_string *string);
char *xdebug_xdebug_str_path_to_url(xdebug_str *string);
char *xdebug_path_from_url(zend_string *fileurl);

FILE *xdebug_fopen(char *fname, const char *mode, const char *extension, char **new_fname);

#endif
