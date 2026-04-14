/*
 * Bridge header for PHP's static build system.
 * Provides phpext_php_debugger_ptr for main/internal_functions.c.
 */

#ifndef PHP_PHP_DEBUGGER_H
#define PHP_PHP_DEBUGGER_H

#include "php_xdebug.h"

extern zend_module_entry xdebug_module_entry;
#define phpext_php_debugger_ptr &xdebug_module_entry

#endif /* PHP_PHP_DEBUGGER_H */
