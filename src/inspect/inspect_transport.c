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

#include "php_xdebug.h"

#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#ifndef PHP_WIN32
# if HAVE_POLL_H
#  include <poll.h>
# elif HAVE_SYS_POLL_H
#  include <sys/poll.h>
# endif
# include <unistd.h>
# include <sys/socket.h>
# include <sys/un.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <arpa/inet.h>
#else
# include <winsock2.h>
# include <ws2tcpip.h>
# define poll WSAPoll
#endif

#include "ext/json/php_json.h"

#include "inspect_transport.h"
#include "inspect_commands.h"
#include "debugger/com.h"
#include "lib/log.h"
#include "lib/mm.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

/* ------------------------------------------------------------------ */
/* Log channel for inspection subsystem                               */
/* ------------------------------------------------------------------ */
#define XLOG_CHAN_INSPECT 9

/* ------------------------------------------------------------------ */
/* Helper: send data to a socket fd, ignoring return value warnings   */
/* ------------------------------------------------------------------ */
static void inspect_send_raw(int fd, const char *data, size_t len)
{
	ssize_t ret;
	ret = SSENDL(fd, data, len);
	(void)ret;
}

/* ------------------------------------------------------------------ */
/* Per-client state                                                   */
/* ------------------------------------------------------------------ */

typedef struct {
	int   fd;
	char  read_buf[XDEBUG_INSPECT_READ_BUFSIZE];
	int   read_pos;   /* bytes currently in read_buf */
} inspect_client;

/* ------------------------------------------------------------------ */
/* Module-level state (true globals — persist across requests)        */
/* ------------------------------------------------------------------ */

static int             inspect_listen_fd  = -1;
static inspect_client  inspect_clients[XDEBUG_INSPECT_MAX_CLIENTS];
static int             inspect_num_clients = 0;
static int             inspect_initialized = 0;

/* ------------------------------------------------------------------ */
/* Background thread state                                            */
/* ------------------------------------------------------------------ */
static pthread_t       inspect_thread;
static int             inspect_thread_active = 0;
static int             inspect_shutdown_pipe[2] = {-1, -1};

/* Saved settings for thread-safe access (set once at init) */
static int             saved_inspect_port = 9007;
static char            saved_inspect_mode[32] = "off";
static char            saved_inspect_socket[256] = "";

/* ------------------------------------------------------------------ */
/* INI helpers                                                        */
/* ------------------------------------------------------------------ */

int xdebug_inspect_parse_mode(const char *value)
{
	if (!value || strcmp(value, "off") == 0 || strcmp(value, "0") == 0) {
		return XDEBUG_INSPECT_OFF;
	}
	if (strcmp(value, "on") == 0 || strcmp(value, "1") == 0) {
		return XDEBUG_INSPECT_ON;
	}
	if (strcmp(value, "auto") == 0) {
		return XDEBUG_INSPECT_AUTO;
	}
	if (strcmp(value, "trigger") == 0) {
		return XDEBUG_INSPECT_TRIGGER;
	}
	return XDEBUG_INSPECT_OFF;
}

const char *xdebug_inspect_mode_name(int mode)
{
	switch (mode) {
		case XDEBUG_INSPECT_ON:      return "on";
		case XDEBUG_INSPECT_AUTO:    return "auto";
		case XDEBUG_INSPECT_TRIGGER: return "trigger";
		default:                     return "off";
	}
}

/* ------------------------------------------------------------------ */
/* Socket helpers                                                     */
/* ------------------------------------------------------------------ */

static int set_nonblocking(int fd)
{
#ifndef PHP_WIN32
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0) return -1;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
	u_long mode = 1;
	return ioctlsocket(fd, FIONBIO, &mode);
#endif
}

static int set_cloexec(int fd)
{
#ifndef PHP_WIN32
	return fcntl(fd, F_SETFD, FD_CLOEXEC);
#else
	(void)fd;
	return 0;
#endif
}

static void close_client(int idx)
{
	if (idx < 0 || idx >= inspect_num_clients) return;
	if (inspect_clients[idx].fd >= 0) {
		close(inspect_clients[idx].fd);
	}
	/* Shift remaining clients down */
	for (int i = idx; i < inspect_num_clients - 1; i++) {
		inspect_clients[i] = inspect_clients[i + 1];
	}
	inspect_num_clients--;
}

/* ------------------------------------------------------------------ */
/* Create listening socket (TCP or Unix)                              */
/* ------------------------------------------------------------------ */

static int create_tcp_server(int port)
{
	int sockfd;
	struct sockaddr_in addr;
	int optval = 1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		return -1;
	}

	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));

	memset(&addr, 0, sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port        = htons((unsigned short)port);

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		close(sockfd);
		return -1;
	}

	if (listen(sockfd, 5) < 0) {
		close(sockfd);
		return -1;
	}

	set_nonblocking(sockfd);
	set_cloexec(sockfd);

	return sockfd;
}

#ifndef PHP_WIN32
static int create_unix_server(const char *path)
{
	int sockfd;
	struct sockaddr_un addr;

	if (!path || !*path) {
		return -1;
	}

	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd < 0) {
		return -1;
	}

	/* Remove stale socket file if it exists */
	unlink(path);

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		close(sockfd);
		return -1;
	}

	if (listen(sockfd, 5) < 0) {
		close(sockfd);
		return -1;
	}

	set_nonblocking(sockfd);
	set_cloexec(sockfd);

	return sockfd;
}
#endif

/* ------------------------------------------------------------------ */
/* Accept new connections (non-blocking)                              */
/* ------------------------------------------------------------------ */

static void accept_new_connections(void)
{
	struct sockaddr_storage client_addr;
	socklen_t addr_len;
	int client_fd;

	if (inspect_listen_fd < 0) return;

	/* Accept as many pending connections as we can */
	while (inspect_num_clients < XDEBUG_INSPECT_MAX_CLIENTS) {
		addr_len = sizeof(client_addr);
		client_fd = accept(inspect_listen_fd, (struct sockaddr *)&client_addr, &addr_len);

		if (client_fd < 0) {
			/* No more pending connections (EAGAIN/EWOULDBLOCK) or error */
			break;
		}

		set_nonblocking(client_fd);
		set_cloexec(client_fd);

		inspect_clients[inspect_num_clients].fd       = client_fd;
		inspect_clients[inspect_num_clients].read_pos  = 0;
		inspect_num_clients++;
	}
}

/* ------------------------------------------------------------------ */
/* Thread-safe JSON helpers (pure C, no PHP allocator/functions)      */
/* ------------------------------------------------------------------ */

static long thread_json_extract_id(const char *json)
{
	const char *p = strstr(json, "\"id\"");
	if (!p) return 0;
	p += 4;
	while (*p && (*p == ' ' || *p == '\t' || *p == ':')) p++;
	return strtol(p, NULL, 10);
}

static int thread_json_extract_cmd(const char *json, char *out, int maxlen)
{
	const char *p = strstr(json, "\"cmd\"");
	if (!p) return 0;
	p += 5;
	while (*p && (*p == ' ' || *p == '\t' || *p == ':')) p++;
	if (*p != '"') return 0;
	p++;
	int i = 0;
	while (*p && *p != '"' && i < maxlen - 1) {
		if (*p == '\\' && *(p + 1)) {
			p++; /* skip escape char */
		}
		out[i++] = *p++;
	}
	out[i] = '\0';
	return i > 0;
}

static void thread_send(int fd, const char *buf, int len)
{
	int sent = 0;
	while (sent < len) {
		int n = send(fd, buf + sent, len - sent, MSG_NOSIGNAL);
		if (n <= 0) break;
		sent += n;
	}
}

/* Thread-safe command dispatch — builds response with snprintf */
static void thread_dispatch(int fd, const char *line)
{
	char cmd[64] = {0};
	long id = thread_json_extract_id(line);
	char buf[4096];
	int len;

	if (!thread_json_extract_cmd(line, cmd, sizeof(cmd))) {
		len = snprintf(buf, sizeof(buf),
			"{\"id\":%ld,\"ok\":false,\"error\":\"missing 'cmd' field\"}\n", id);
	} else if (strcmp(cmd, "status") == 0) {
		len = snprintf(buf, sizeof(buf),
			"{\"id\":%ld,\"ok\":true,\"extension\":\"%s\","
			"\"version\":\"%s\","
			"\"php_version\":\"%s\","
			"\"sapi\":\"%s\","
			"\"inspect_mode\":\"%s\","
			"\"inspect_port\":%d,"
			"\"inspect_socket\":\"%s\"}\n",
			id, XDEBUG_NAME, XDEBUG_VERSION, PHP_VERSION,
			sapi_module.name, saved_inspect_mode,
			saved_inspect_port, saved_inspect_socket);
	} else {
		len = snprintf(buf, sizeof(buf),
			"{\"id\":%ld,\"ok\":false,\"error\":\"command '%s' not yet implemented\"}\n",
			id, cmd);
	}

	if (len > 0 && len < (int)sizeof(buf)) {
		thread_send(fd, buf, len);
	}
}

/* Process buffered lines for a client (thread context) */
static void thread_process_lines(int idx)
{
	inspect_client *c = &inspect_clients[idx];
	char *start = c->read_buf;
	int remaining = c->read_pos;

	while (remaining > 0) {
		char *nl = memchr(start, '\n', remaining);
		if (!nl) break;

		*nl = '\0';
		int line_len = (int)(nl - start);

		if (line_len > 0) {
			thread_dispatch(c->fd, start);
		}

		start = nl + 1;
		remaining = c->read_pos - (int)(start - c->read_buf);
	}

	if (start > c->read_buf && remaining > 0) {
		memmove(c->read_buf, start, remaining);
	}
	c->read_pos = remaining;
}

/* ------------------------------------------------------------------ */
/* Background thread main function                                    */
/* ------------------------------------------------------------------ */

static void *inspect_thread_func(void *arg)
{
	(void)arg;
	struct pollfd pfds[XDEBUG_INSPECT_MAX_CLIENTS + 2];

	while (1) {
		int n = 0;

		/* Shutdown pipe (always slot 0) */
		pfds[n].fd      = inspect_shutdown_pipe[0];
		pfds[n].events  = POLLIN;
		pfds[n].revents = 0;
		n++;

		/* Listen socket (slot 1) */
		if (inspect_listen_fd >= 0) {
			pfds[n].fd      = inspect_listen_fd;
			pfds[n].events  = POLLIN;
			pfds[n].revents = 0;
			n++;
		}

		/* Client sockets */
		int client_base = n;
		for (int i = 0; i < inspect_num_clients; i++) {
			pfds[n].fd      = inspect_clients[i].fd;
			pfds[n].events  = POLLIN;
			pfds[n].revents = 0;
			n++;
		}

		int ready = poll(pfds, (nfds_t)n, 200);
		if (ready < 0) {
			if (errno == EINTR) continue;
			break;
		}
		if (ready == 0) continue;

		/* Check shutdown pipe */
		if (pfds[0].revents & POLLIN) {
			break;
		}

		/* Check listen socket for new connections */
		if (inspect_listen_fd >= 0 && n > 1 && (pfds[1].revents & POLLIN)) {
			accept_new_connections();
		}

		/* Process client data in reverse order (safe for close_client shifting) */
		int num_at_poll = inspect_num_clients;
		for (int i = num_at_poll - 1; i >= 0; i--) {
			int pi = client_base + i;
			if (pi >= n) continue;
			if (!(pfds[pi].revents & (POLLIN | POLLHUP | POLLERR))) continue;

			inspect_client *c = &inspect_clients[i];
			int space = XDEBUG_INSPECT_READ_BUFSIZE - c->read_pos - 1;

			if (space <= 0) {
				close_client(i);
				continue;
			}

			int bytes = recv(c->fd, c->read_buf + c->read_pos, space, 0);
			if (bytes <= 0) {
				close_client(i);
				continue;
			}

			c->read_pos += bytes;
			c->read_buf[c->read_pos] = '\0';
			thread_process_lines(i);
		}
	}

	return NULL;
}

/* ------------------------------------------------------------------ */
/* Process incoming JSON lines from a single client (main thread)     */
/* ------------------------------------------------------------------ */

static void process_client_lines(int idx)
{
	inspect_client *c = &inspect_clients[idx];
	char *line_start;
	char *newline;
	int   remaining;

	line_start = c->read_buf;
	remaining  = c->read_pos;

	while (remaining > 0) {
		newline = memchr(line_start, '\n', remaining);
		if (!newline) break;

		/* Null-terminate the line */
		*newline = '\0';
		int line_len = (int)(newline - line_start);

		/* Skip empty lines */
		if (line_len > 0) {
			/* Parse JSON */
			zval    parsed;
			zend_string *json_str = zend_string_init(line_start, line_len, 0);

			ZVAL_UNDEF(&parsed);
			if (php_json_decode_ex(&parsed, ZSTR_VAL(json_str), ZSTR_LEN(json_str),
					PHP_JSON_OBJECT_AS_ARRAY, 512) != SUCCESS
				|| Z_TYPE(parsed) != IS_ARRAY)
			{
				/* JSON parse error — send error response */
				const char *err = "{\"ok\":false,\"error\":\"invalid JSON\"}\n";
				inspect_send_raw(c->fd, err, strlen(err));
				zval_ptr_dtor(&parsed);
				zend_string_release(json_str);
			} else {
				/* Extract "cmd" and "id" fields */
				zval *zcmd = zend_hash_str_find(Z_ARRVAL(parsed), "cmd", sizeof("cmd") - 1);
				zval *zid  = zend_hash_str_find(Z_ARRVAL(parsed), "id",  sizeof("id") - 1);

				const char *cmd  = (zcmd && Z_TYPE_P(zcmd) == IS_STRING) ? Z_STRVAL_P(zcmd) : NULL;
				zend_long   req_id = 0;
				if (zid) {
					if (Z_TYPE_P(zid) == IS_LONG) {
						req_id = Z_LVAL_P(zid);
					} else if (Z_TYPE_P(zid) == IS_STRING) {
						req_id = ZEND_STRTOL(Z_STRVAL_P(zid), NULL, 10);
					}
				}

				/* Dispatch to command handler */
				char *response = inspect_dispatch_command(c->fd, cmd, req_id, &parsed);
				if (response) {
					inspect_send_raw(c->fd, response, strlen(response));
					efree(response);
				}

				zval_ptr_dtor(&parsed);
				zend_string_release(json_str);
			}
		}

		/* Advance past this line */
		line_start = newline + 1;
		remaining  = c->read_pos - (int)(line_start - c->read_buf);
	}

	/* Move any partial (un-newlined) data to the start of the buffer */
	if (line_start > c->read_buf && remaining > 0) {
		memmove(c->read_buf, line_start, remaining);
	}
	c->read_pos = remaining;
}

/* ------------------------------------------------------------------ */
/* Public API                                                         */
/* ------------------------------------------------------------------ */

int inspect_transport_init(void)
{
	int mode = XG(settings.library.inspect_mode);

	if (mode == XDEBUG_INSPECT_OFF) {
		return SUCCESS;
	}

	/* Already initialized (e.g. persistent module in FPM) */
	if (inspect_initialized) {
		return SUCCESS;
	}

	/* Initialize client slots */
	memset(inspect_clients, 0, sizeof(inspect_clients));
	for (int i = 0; i < XDEBUG_INSPECT_MAX_CLIENTS; i++) {
		inspect_clients[i].fd = -1;
	}
	inspect_num_clients = 0;

	/* Determine transport: Unix socket has priority */
	const char *socket_path = XG(settings.library.inspect_socket);
	int         port        = (int)XG(settings.library.inspect_port);

#ifndef PHP_WIN32
	if (socket_path && *socket_path) {
		inspect_listen_fd = create_unix_server(socket_path);
	} else
#endif
	{
		inspect_listen_fd = create_tcp_server(port);
	}

	if (inspect_listen_fd < 0) {
		return FAILURE;
	}

	/* Save settings for thread-safe access */
	saved_inspect_port = port;
	strncpy(saved_inspect_mode, xdebug_inspect_mode_name(mode), sizeof(saved_inspect_mode) - 1);
	saved_inspect_mode[sizeof(saved_inspect_mode) - 1] = '\0';
	if (socket_path) {
		strncpy(saved_inspect_socket, socket_path, sizeof(saved_inspect_socket) - 1);
		saved_inspect_socket[sizeof(saved_inspect_socket) - 1] = '\0';
	}

	/* Create shutdown pipe for signaling the thread */
	if (pipe(inspect_shutdown_pipe) == 0) {
		set_nonblocking(inspect_shutdown_pipe[0]);
		set_cloexec(inspect_shutdown_pipe[0]);
		set_cloexec(inspect_shutdown_pipe[1]);

		/* Start background thread */
		if (pthread_create(&inspect_thread, NULL, inspect_thread_func, NULL) == 0) {
			inspect_thread_active = 1;
		}
	}

	inspect_initialized = 1;
	return SUCCESS;
}

void inspect_transport_check(void)
{
	/* When thread is active, it handles accept */
	if (inspect_thread_active) return;
	if (!inspect_initialized || inspect_listen_fd < 0) return;

	accept_new_connections();
}

void inspect_transport_process(void)
{
	/* When thread is active, it handles processing */
	if (inspect_thread_active) return;

	struct pollfd pfds[XDEBUG_INSPECT_MAX_CLIENTS];
	nfds_t n;
	int ready;

	if (!inspect_initialized || inspect_num_clients == 0) return;

	n = (nfds_t)inspect_num_clients;

	for (nfds_t i = 0; i < n; i++) {
		pfds[i].fd      = inspect_clients[i].fd;
		pfds[i].events  = POLLIN;
		pfds[i].revents = 0;
	}

	ready = poll(pfds, n, 0);  /* non-blocking: timeout=0 */
	if (ready <= 0) return;

	/* Process clients in reverse order so close_client() index shifting is safe */
	for (int i = (int)n - 1; i >= 0; i--) {
		if (!(pfds[i].revents & (POLLIN | POLLHUP | POLLERR))) continue;

		inspect_client *c = &inspect_clients[i];
		int space = XDEBUG_INSPECT_READ_BUFSIZE - c->read_pos - 1;

		if (space <= 0) {
			close_client(i);
			continue;
		}

		int bytes = SREAD(c->fd, c->read_buf + c->read_pos, space);

		if (bytes <= 0) {
			close_client(i);
			continue;
		}

		c->read_pos += bytes;
		c->read_buf[c->read_pos] = '\0';

		process_client_lines(i);
	}
}

int inspect_transport_send(int client_id, const char *json_line)
{
	if (client_id < 0 || client_id >= inspect_num_clients) return FAILURE;
	if (!json_line) return FAILURE;

	inspect_send_raw(inspect_clients[client_id].fd, json_line, strlen(json_line));
	return SUCCESS;
}

int inspect_transport_broadcast(const char *json_line)
{
	if (!json_line || inspect_num_clients == 0) return FAILURE;

	for (int i = 0; i < inspect_num_clients; i++) {
		inspect_send_raw(inspect_clients[i].fd, json_line, strlen(json_line));
	}
	return SUCCESS;
}

void inspect_transport_shutdown(void)
{
	if (!inspect_initialized) return;

	/* Signal the background thread to stop and wait for it */
	if (inspect_thread_active) {
		char c = 'x';
		ssize_t w = write(inspect_shutdown_pipe[1], &c, 1);
		(void)w;
		pthread_join(inspect_thread, NULL);
		inspect_thread_active = 0;
	}

	/* Close shutdown pipe */
	if (inspect_shutdown_pipe[0] >= 0) {
		close(inspect_shutdown_pipe[0]);
		inspect_shutdown_pipe[0] = -1;
	}
	if (inspect_shutdown_pipe[1] >= 0) {
		close(inspect_shutdown_pipe[1]);
		inspect_shutdown_pipe[1] = -1;
	}

	/* Close all client connections */
	for (int i = 0; i < inspect_num_clients; i++) {
		if (inspect_clients[i].fd >= 0) {
			close(inspect_clients[i].fd);
			inspect_clients[i].fd = -1;
		}
	}
	inspect_num_clients = 0;

	/* Close listening socket */
	if (inspect_listen_fd >= 0) {
		close(inspect_listen_fd);

		/* Remove Unix socket file if applicable */
#ifndef PHP_WIN32
		if (saved_inspect_socket[0]) {
			unlink(saved_inspect_socket);
		}
#endif

		inspect_listen_fd = -1;
	}

	inspect_initialized = 0;
}
