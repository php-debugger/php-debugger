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
#define _GNU_SOURCE 1
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "arg.h"
#include "str.h"
#include "xdebug_strndup.h"

#ifdef __linux__
int xdebug_read_systemd_private_tmp_directory(char **private_tmp)
{
	pid_t       current_pid;
	char       *mountinfo_fn;
	FILE       *mountinfo_fd;
	size_t      bytes_read;
	char        buffer[8192] = { 0 };
	xdebug_arg *lines;
	int         i;
	int         retval = 0;

	/* Open right file in /proc */
	current_pid = getpid();
	mountinfo_fn = xdebug_sprintf("/proc/%ld/mountinfo", current_pid);
	mountinfo_fd = fopen(mountinfo_fn, "r");
	xdfree(mountinfo_fn);
	if (!mountinfo_fd) {
		return retval;
	}

	/* Read contents and split in lines */
	bytes_read = fread(buffer, 1, sizeof(buffer), mountinfo_fd);
	if (!bytes_read) {
		fclose(mountinfo_fd);
		return retval;
	}

	lines = xdebug_arg_ctor();
	xdebug_explode("\n", buffer, lines, -1);

	/* Check whether each line has /tmp/systemd-private, and parse accordingly.
	 * There is a " " in front as there is often also a /var/tmp/systemd-private
	 * entry that we need to ignore. */
	for (i = 0; i < lines->c; i++) {
		const char *mountpoint;
		const char *slash;

		mountpoint = strstr(lines->args[i], " /tmp/systemd-private");
		if (mountpoint == NULL) {
			continue;
		}

		mountpoint++;

		slash = strchr(mountpoint + 1, '/');
		if (!slash) {
			continue;
		}

		slash = strchr(slash + 1, '/');
		if (!slash) {
			continue;
		}

		*private_tmp = xdstrndup(mountpoint, slash - mountpoint);

		retval = 1;
		break;
	}

	/* Clean up and return */
	xdebug_arg_dtor(lines);
	fclose(mountinfo_fd);
	return retval;
}

#endif
