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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/resource.h>
# include <sys/file.h>
#else
# define PATH_MAX MAX_PATH
# include <winsock2.h>
# include <io.h>
# include <process.h>
#endif

#include "compat.h"

#include "mm.h"
#include "crc32.h"
#include "str.h"

#include "lib_private.h"
#include "usefulstuff.h"
#include "log.h"

# include "ext/random/php_random.h"
#include "ext/standard/flock_compat.h"
#include "main/php_ini.h"

#ifndef NAME_MAX
# ifdef _AIX
#  include <unistd.h>
#  define NAME_MAX pathconf("/dev/null",_PC_NAME_MAX)
# else
#  define NAME_MAX (MAXNAMELEN-1)
# endif
#endif

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

xdebug_arg *xdebug_arg_ctor(void)
{
	xdebug_arg *tmp = /*(xdebug_arg*)*/ xdmalloc(sizeof(xdebug_arg));

	tmp->args = NULL;
	tmp->c    = 0;

	return tmp;
}

void xdebug_arg_dtor(xdebug_arg *arg)
{
	int i;

	for (i = 0; i < arg->c; i++) {
		xdfree(arg->args[i]);
	}
	if (arg->args) {
		xdfree(arg->args);
	}
	xdfree(arg);
}

xdebug_str* xdebug_join(const char *delim, xdebug_arg *args, int begin, int end)
{
	int         i;
	xdebug_str *ret = xdebug_str_new();

	if (begin < 0) {
		begin = 0;
	}
	if (end > args->c - 1) {
		end = args->c - 1;
	}
	for (i = begin; i < end; i++) {
		xdebug_str_add(ret, args->args[i], 0);
		xdebug_str_add(ret, delim, 0);
	}
	xdebug_str_add(ret, args->args[end], 0);
	return ret;
}

void xdebug_explode(const char *delim, const char *str, xdebug_arg *args, int limit)
{
	const char *p1, *p2, *endp;

	endp = str + strlen(str);

	p1 = str;
	p2 = xdebug_memnstr(str, delim, strlen(delim), endp);

	if (p2 == NULL) {
		args->c++;
		args->args = (char**) xdrealloc(args->args, sizeof(char*) * args->c);
		args->args[args->c - 1] = (char*) xdmalloc(strlen(str) + 1);
		memcpy(args->args[args->c - 1], p1, strlen(str));
		args->args[args->c - 1][strlen(str)] = '\0';
	} else {
		do {
			args->c++;
			args->args = (char**) xdrealloc(args->args, sizeof(char*) * args->c);
			args->args[args->c - 1] = (char*) xdmalloc(p2 - p1 + 1);
			memcpy(args->args[args->c - 1], p1, p2 - p1);
			args->args[args->c - 1][p2 - p1] = '\0';
			p1 = p2 + strlen(delim);
		} while ((p2 = xdebug_memnstr(p1, delim, strlen(delim), endp)) != NULL && (limit == -1 || --limit > 1));

		if (p1 <= endp) {
			args->c++;
			args->args = (char**) xdrealloc(args->args, sizeof(char*) * args->c);
			args->args[args->c - 1] = (char*) xdmalloc(endp - p1 + 1);
			memcpy(args->args[args->c - 1], p1, endp - p1);
			args->args[args->c - 1][endp - p1] = '\0';
		}
	}
}

bool xdebug_is_printable(const char *str, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++) {
		if (!isprint(str[i])) {
			return false;
		}
	}

	return true;
}

const char* xdebug_memnstr(const char *haystack, const char *needle, int needle_len, const char *end)
{
	const char *p = haystack;
	char first = *needle;

	/* let end point to the last character where needle may start */
	end -= needle_len;

	while (p <= end) {
		while (*p != first)
			if (++p > end)
				return NULL;
		if (memcmp(p, needle, needle_len) == 0)
			return p;
		p++;
	}
	return NULL;
}

/* not all versions of php export this */
static int xdebug_htoi(char *s)
{
	int value;
	int c;

	c = s[0];
	if (isupper(c)) {
		c = tolower(c);
	}
	value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

	c = s[1];
	if (isupper(c)) {
		c = tolower(c);
	}
	value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

	return value;
}

/* not all versions of php export this */
int xdebug_raw_url_decode(char *str, int len)
{
	char *dest = str;
	char *data = str;

	while (len--) {
		if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) && isxdigit((int) *(data + 2))) {
			*dest = (char) xdebug_htoi(data + 1);
			data += 2;
			len -= 2;
		} else {
			*dest = *data;
		}
		data++;
		dest++;
	}
	*dest = '\0';
	return dest - str;
}

static unsigned char hexchars[] = "0123456789ABCDEF";

char *xdebug_raw_url_encode(char const *s, int len, int *new_length, int skip_slash)
{
	register int x, y;
	unsigned char *str;

	str = (unsigned char *) xdmalloc(3 * len + 1);
	for (x = 0, y = 0; len--; x++, y++) {
		str[y] = (unsigned char) s[x];
		if ((str[y] < '0' && str[y] != '-' && str[y] != '.' && (str[y] != '/' || !skip_slash)) ||
			(str[y] < 'A' && str[y] > '9' && str[y] != ':') ||
			(str[y] > 'Z' && str[y] < 'a' && str[y] != '_' && (str[y] != '\\' || !skip_slash)) ||
			(str[y] > 'z'))
		{
			str[y++] = '%';
			str[y++] = hexchars[(unsigned char) s[x] >> 4];
			str[y] = hexchars[(unsigned char) s[x] & 15];
		}
	}
	str[y] = '\0';
	if (new_length) {
		*new_length = y;
	}
	return ((char *) str);
}

/* fake URI's per IETF RFC 1738 and 2396 format */
char *xdebug_path_from_url(zend_string *fileurl)
{
	/* deal with file: url's */
	char *dfp = NULL;
	const char *fp = NULL, *efp = ZSTR_VAL(fileurl);
#ifdef PHP_WIN32
	int l = 0;
	int i;
#endif
	char *tmp = NULL, *ret = NULL;

	dfp = xdstrdup(efp);
	fp = dfp;
	xdebug_raw_url_decode(dfp, strlen(dfp));
	tmp = strstr(fp, "file://");

	if (tmp) {
		fp = tmp + 7;
		if (fp[0] == '/' && fp[2] == ':') {
			fp++;
		}
		ret = xdstrdup(fp);
#ifdef PHP_WIN32
		l = strlen(ret);
		/* convert '/' to '\' */
		for (i = 0; i < l; i++) {
			if (ret[i] == '/') {
				ret[i] = '\\';
			}
		}
#endif
	} else {
		ret = xdstrdup(ZSTR_VAL(fileurl));
	}

	free(dfp);
	return ret;
}

/* fake URI's per IETF RFC 1738 and 2396 format */
static char *xdebug_path_to_url(const char *fileurl, size_t fileurl_len)
{
	int l, i, new_len;
	char *tmp = NULL;
	char *encoded_fileurl;

	/* encode the url */
	encoded_fileurl = xdebug_raw_url_encode(fileurl, fileurl_len, &new_len, 1);

	if (strstr(fileurl, "://") != NULL && strstr(fileurl, "://") < strstr(fileurl, "/")) {
		/* ignore, some form of stream wrapper scheme */
		tmp = xdstrdup(fileurl);
	} else if (fileurl[0] != '/' && fileurl[0] != '\\' && fileurl[1] != ':') {
		/* convert relative paths */
		cwd_state new_state;
		char cwd[MAXPATHLEN];
		char *result;

		result = VCWD_GETCWD(cwd, MAXPATHLEN);
		if (!result) {
			cwd[0] = '\0';
		}

		new_state.cwd = estrdup(cwd);
		new_state.cwd_length = strlen(cwd);

		if (!virtual_file_ex(&new_state, fileurl, NULL, 1)) {
			char *s = estrndup(new_state.cwd, new_state.cwd_length);
			tmp = xdebug_sprintf("file://%s",s);
			efree(s);
		}
		efree(new_state.cwd);

	} else if (fileurl[1] == '/' || fileurl[1] == '\\') {
		/* convert UNC paths (eg. \\server\sharepath) */
		/* See https://docs.microsoft.com/en-us/archive/blogs/ie/file-uris-in-windows */
		tmp = xdebug_sprintf("file:%s", encoded_fileurl);
	} else if (fileurl[0] == '/' || fileurl[0] == '\\') {
		/* convert *nix paths (eg. /path) */
		tmp = xdebug_sprintf("file://%s", encoded_fileurl);
	} else if (fileurl[1] == ':') {
		/* convert windows drive paths (eg. c:\path) */
		tmp = xdebug_sprintf("file:///%s", encoded_fileurl);
	} else {
		/* no clue about it, use it raw */
		tmp = xdstrdup(encoded_fileurl);
	}
	l = strlen(tmp);
	/* convert '\' to '/' */
	for (i = 0; i < l; i++) {
		if (tmp[i] == '\\') {
			tmp[i]='/';
		}
	}
	xdfree(encoded_fileurl);
	return tmp;
}

char *xdebug_zstr_path_to_url(zend_string *string)
{
	return xdebug_path_to_url(ZSTR_VAL(string), ZSTR_LEN(string));
}

char *xdebug_xdebug_str_path_to_url(xdebug_str *string)
{
	return xdebug_path_to_url(XDEBUG_STR_VAL(string), XDEBUG_STR_LEN(string));
}

#ifndef PHP_WIN32
static FILE *xdebug_open_file(char *fname, const char *mode, const char *extension, char **new_fname)
{
	FILE *fh;
	char *tmp_fname;

	if (extension) {
		tmp_fname = xdebug_sprintf("%s.%s", fname, extension);
	} else {
		tmp_fname = xdstrdup(fname);
	}
	fh = fopen(tmp_fname, mode);
	if (fh && new_fname) {
		*new_fname = tmp_fname;
	} else {
		xdfree(tmp_fname);
	}
	return fh;
}

static FILE *xdebug_open_file_with_random_ext(char *fname, const char *mode, const char *extension, char **new_fname)
{
	FILE *fh;
	char *tmp_fname;

	if (extension) {
		tmp_fname = xdebug_sprintf("%s.%06x.%s", fname, (long) (1000000 * php_combined_lcg()), extension);
	} else {
		tmp_fname = xdebug_sprintf("%s.%06x", fname, (long) (1000000 * php_combined_lcg()), extension);
	}
	fh = fopen(tmp_fname, mode);
	if (fh && new_fname) {
		*new_fname = tmp_fname;
	} else {
		xdfree(tmp_fname);
	}
	return fh;
}

FILE *xdebug_fopen(char *fname, const char *mode, const char *extension, char **new_fname)
{
	int   r;
	FILE *fh;
	struct stat buf;
	char *tmp_fname = NULL;
	int   filename_len = 0;

	/* We're not doing any tricks for append mode... as that has atomic writes
	 * anyway. And we ignore read mode as well. */
	if (mode[0] == 'a' || mode[0] == 'r') {
		return xdebug_open_file(fname, mode, extension, new_fname);
	}

	/* Make sure we don't open a file with a path that's too long */
	filename_len += (fname ? strlen(fname) : 0); /* filename */
	filename_len += (extension ? strlen(extension) : 0) + 1; /* extension (+ ".") */
	filename_len += 8; /* possible random extension (+ ".") */
	if (filename_len > NAME_MAX) {
		fname[NAME_MAX - (extension ? strlen(extension) : 0 )] = '\0';
	}

	/* In write mode however we do have to do some stuff. */
	/* 1. Check if the file exists */
	if (extension) {
		tmp_fname = xdebug_sprintf("%s.%s", fname, extension);
	} else {
		tmp_fname = xdstrdup(fname);
	}
	r = stat(tmp_fname, &buf);
	/* We're not freeing "tmp_fname" as that is used in the freopen as well. */

	if (r == -1) {
		/* 2. Cool, the file doesn't exist so we can open it without probs now. */
		fh = xdebug_open_file(fname, "w", extension, new_fname);
		goto lock;
	}

	/* 3. It exists, check if we can open it. */
	fh = xdebug_open_file(fname, "r+", extension, new_fname);
	if (!fh) {
		/* 4. If fh == null we couldn't even open the file, so open a new one with a new name */
		fh = xdebug_open_file_with_random_ext(fname, "w", extension, new_fname);
		goto lock;
	}

	/* 5. It exists and we can open it, check if we can exclusively lock it. */
	r = flock(fileno(fh), LOCK_EX | LOCK_NB);
	if (r == -1) {
		if (errno == EWOULDBLOCK) {
			fclose(fh);
			/* 6. The file is in use, so we open one with a new name. */
			fh = xdebug_open_file_with_random_ext(fname, "w", extension, new_fname);
			goto lock;
		}
	}

	/* 7. We established a lock, now we truncate the already-opened file
	 *    descriptor. Using the fd (rather than reopening by name via
	 *    freopen) avoids a TOCTOU race where the path could be swapped
	 *    between the earlier stat() and this operation. */
	if (ftruncate(fileno(fh), 0) == -1) {
		fclose(fh);
		fh = NULL;
	} else {
		rewind(fh);
	}

lock: /* Yes yes, an evil goto label here!!! */
	if (fh) {
		/* 8. We have to lock again after the reopen as that basically closes
		 * the file and opens it again. There is a small race condition here...
		 */
		flock(fileno(fh), LOCK_EX | LOCK_NB);
	}
	xdfree(tmp_fname);
	return fh;
}
#else
FILE *xdebug_fopen(char *fname, const char *mode, const char *extension, char **new_fname)
{
	char *tmp_fname;
	FILE *ret;

	if (extension) {
		tmp_fname = xdebug_sprintf("%s.%s", fname, extension);
	} else {
		tmp_fname = xdstrdup(fname);
	}
	ret = fopen(tmp_fname, mode);
	if (new_fname) {
		*new_fname = tmp_fname;
	} else {
		xdfree(tmp_fname);
	}
	return ret;
}
#endif
