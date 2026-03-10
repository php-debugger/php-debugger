/* This is a generated file, edit php_xdebug.stub.php instead.
 * Stub hash: edd68183eb73251289cbc16760372598e42fb968 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xdebug_break, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_xdebug_connect_to_client arginfo_xdebug_break

ZEND_BEGIN_ARG_INFO_EX(arginfo_xdebug_info, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, category, IS_STRING, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_xdebug_is_debugger_active arginfo_xdebug_break

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xdebug_notify, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(xdebug_break);
ZEND_FUNCTION(xdebug_connect_to_client);
ZEND_FUNCTION(xdebug_info);
ZEND_FUNCTION(xdebug_is_debugger_active);
ZEND_FUNCTION(xdebug_notify);

static const zend_function_entry ext_functions[] = {
	ZEND_FE(xdebug_break, arginfo_xdebug_break)
	ZEND_FE(xdebug_connect_to_client, arginfo_xdebug_connect_to_client)
	ZEND_FE(xdebug_info, arginfo_xdebug_info)
	ZEND_FE(xdebug_is_debugger_active, arginfo_xdebug_is_debugger_active)
	ZEND_FE(xdebug_notify, arginfo_xdebug_notify)
	/* php_debugger_* aliases */
	ZEND_FALIAS(php_debugger_break, xdebug_break, arginfo_xdebug_break)
	ZEND_FALIAS(php_debugger_connect_to_client, xdebug_connect_to_client, arginfo_xdebug_connect_to_client)
	ZEND_FALIAS(php_debugger_info, xdebug_info, arginfo_xdebug_info)
	ZEND_FALIAS(php_debugger_is_debugger_active, xdebug_is_debugger_active, arginfo_xdebug_is_debugger_active)
	ZEND_FALIAS(php_debugger_notify, xdebug_notify, arginfo_xdebug_notify)
	ZEND_FE_END
};
