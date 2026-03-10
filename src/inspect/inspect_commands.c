/*
   +----------------------------------------------------------------------+
   | Xdebug — Runtime Inspection API Commands (Stubs)                     |
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

#include "php_xdebug.h"

#include "inspect_commands.h"
#include "inspect_transport.h"

#include "ext/json/php_json.h"
#include "zend_smart_str.h"
#include "lib/mm.h"

#include <string.h>

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

/* ------------------------------------------------------------------ */
/* Helper: build a JSON response string (caller must efree)           */
/* ------------------------------------------------------------------ */

static char *make_json_response(zend_long id, zval *data)
{
	smart_str buf = {0};
	zval      response;

	array_init(&response);

	if (id > 0) {
		add_assoc_long(&response, "id", id);
	}

	/* Merge data keys into response */
	if (data && Z_TYPE_P(data) == IS_ARRAY) {
		zend_string *key;
		zval        *val;

		ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(data), key, val) {
			if (key) {
				Z_TRY_ADDREF_P(val);
				zend_hash_update(Z_ARRVAL(response), key, val);
			}
		} ZEND_HASH_FOREACH_END();
	}

	php_json_encode(&buf, &response, PHP_JSON_UNESCAPED_SLASHES);
	zval_ptr_dtor(&response);

	/* Append newline for JSON lines framing */
	smart_str_appendc(&buf, '\n');
	smart_str_0(&buf);

	char *result = estrdup(ZSTR_VAL(buf.s));
	smart_str_free(&buf);
	return result;
}

static char *make_ok_response(zend_long id, const char *message)
{
	zval data;
	array_init(&data);
	add_assoc_bool(&data, "ok", 1);
	if (message) {
		add_assoc_string(&data, "message", message);
	}
	char *result = make_json_response(id, &data);
	zval_ptr_dtor(&data);
	return result;
}

static char *make_error_response(zend_long id, const char *error)
{
	zval data;
	array_init(&data);
	add_assoc_bool(&data, "ok", 0);
	add_assoc_string(&data, "error", error);
	char *result = make_json_response(id, &data);
	zval_ptr_dtor(&data);
	return result;
}

static char *make_not_implemented_response(zend_long id, const char *cmd)
{
	char msg[256];
	snprintf(msg, sizeof(msg), "command '%s' not yet implemented", cmd);
	return make_error_response(id, msg);
}

/* ------------------------------------------------------------------ */
/* Command dispatch table                                             */
/* ------------------------------------------------------------------ */

typedef struct {
	const char            *name;
	inspect_cmd_handler_fn handler;
} inspect_cmd_entry;

static inspect_cmd_entry inspect_command_table[] = {
	{ "status",          inspect_cmd_status },
	{ "inspect",         inspect_cmd_inspect },
	{ "snapshot",        inspect_cmd_snapshot },
	{ "watch",           inspect_cmd_watch },
	{ "trace",           inspect_cmd_trace },
	{ "eval",            inspect_cmd_eval },
	{ "profile.start",   inspect_cmd_profile_start },
	{ "profile.stop",    inspect_cmd_profile_stop },
	{ "profile.status",  inspect_cmd_profile_status },
	{ "profile.dump",    inspect_cmd_profile_dump },
	{ NULL, NULL }
};

char *inspect_dispatch_command(int client_fd, const char *cmd, zend_long request_id, zval *params)
{
	inspect_cmd_entry *entry;

	if (!cmd || !*cmd) {
		return make_error_response(request_id, "missing 'cmd' field");
	}

	for (entry = inspect_command_table; entry->name != NULL; entry++) {
		if (strcmp(entry->name, cmd) == 0) {
			return entry->handler(client_fd, request_id, params);
		}
	}

	char msg[256];
	snprintf(msg, sizeof(msg), "unknown command: '%s'", cmd);
	return make_error_response(request_id, msg);
}

/* ------------------------------------------------------------------ */
/* Command implementations                                            */
/* ------------------------------------------------------------------ */

char *inspect_cmd_status(int client_fd, zend_long request_id, zval *params)
{
	zval data;
	(void)client_fd;
	(void)params;

	array_init(&data);
	add_assoc_bool(&data, "ok", 1);
	add_assoc_string(&data, "extension", XDEBUG_NAME);
	add_assoc_string(&data, "version", XDEBUG_VERSION);
	add_assoc_string(&data, "php_version", PHP_VERSION);
	add_assoc_string(&data, "sapi", (char *)sapi_module.name);
	add_assoc_string(&data, "inspect_mode", xdebug_inspect_mode_name(XG(settings.library.inspect_mode)));
	add_assoc_long(&data, "inspect_port", XG(settings.library.inspect_port));
	add_assoc_string(&data, "inspect_socket", XG(settings.library.inspect_socket));

	char *result = make_json_response(request_id, &data);
	zval_ptr_dtor(&data);
	return result;
}

char *inspect_cmd_inspect(int client_fd, zend_long request_id, zval *params)
{
	(void)client_fd;
	(void)params;
	return make_not_implemented_response(request_id, "inspect");
}

char *inspect_cmd_snapshot(int client_fd, zend_long request_id, zval *params)
{
	(void)client_fd;
	(void)params;
	return make_not_implemented_response(request_id, "snapshot");
}

char *inspect_cmd_watch(int client_fd, zend_long request_id, zval *params)
{
	(void)client_fd;
	(void)params;
	return make_not_implemented_response(request_id, "watch");
}

char *inspect_cmd_trace(int client_fd, zend_long request_id, zval *params)
{
	(void)client_fd;
	(void)params;
	return make_not_implemented_response(request_id, "trace");
}

char *inspect_cmd_eval(int client_fd, zend_long request_id, zval *params)
{
	(void)client_fd;
	(void)params;
	return make_not_implemented_response(request_id, "eval");
}

char *inspect_cmd_profile_start(int client_fd, zend_long request_id, zval *params)
{
	(void)client_fd;
	(void)params;
	return make_not_implemented_response(request_id, "profile.start");
}

char *inspect_cmd_profile_stop(int client_fd, zend_long request_id, zval *params)
{
	(void)client_fd;
	(void)params;
	return make_not_implemented_response(request_id, "profile.stop");
}

char *inspect_cmd_profile_status(int client_fd, zend_long request_id, zval *params)
{
	(void)client_fd;
	(void)params;
	return make_not_implemented_response(request_id, "profile.status");
}

char *inspect_cmd_profile_dump(int client_fd, zend_long request_id, zval *params)
{
	(void)client_fd;
	(void)params;
	return make_not_implemented_response(request_id, "profile.dump");
}
