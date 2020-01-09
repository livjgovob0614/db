/*------------------------------------------------------------------------
 *  Copyright 2007-2010 (c) Jeff Brown <spadix@users.sourceforge.net>
 *
 *  This file is part of the ZBar Bar Code Reader.
 *
 *  The ZBar Bar Code Reader is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU Lesser Public License as
 *  published by the Free Software Foundation; either version 2.1 of
 *  the License, or (at your option) any later version.
 *
 *  The ZBar Bar Code Reader is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with the ZBar Bar Code Reader; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301  USA
 *
 *  http://sourceforge.net/projects/zbar
 *------------------------------------------------------------------------*/

#include <zbar.h>
#pragma hdrstop
#include "error.h"

int _zbar_verbosity = 0;

static const char * const sev_str[] = {
	"FATAL ERROR", "ERROR", "OK", "WARNING", "NOTE"
};
#define SEV_MAX (strlen(sev_str[0]))

static const char * const mod_str[] = {
	"processor", "video", "window", "image scanner", "<unknown>"
};
#define MOD_MAX (strlen(mod_str[ZBAR_MOD_IMAGE_SCANNER]))

static const char * const err_str[] = {
	"no error",             /* OK */
	"out of memory",        /* NOMEM */
	"internal library error", /* INTERNAL */
	"unsupported request",  /* UNSUPPORTED */
	"invalid request",      /* INVALID */
	"system error",         /* SYSTEM */
	"locking error",        /* LOCKING */
	"all resources busy",   /* BUSY */
	"X11 display error",    /* XDISPLAY */
	"X11 protocol error",   /* XPROTO */
	"output window is closed", /* CLOSED */
	"windows system error", /* WINAPI */
	"unknown error"         /* NUM */
};
#define ERR_MAX (strlen(err_str[ZBAR_ERR_CLOSED]))

int zbar_version(uint * major, uint * minor)
{
	ASSIGN_PTR(major, ZBAR_VERSION_MAJOR);
	ASSIGN_PTR(minor, ZBAR_VERSION_MINOR);
	return 0;
}

void zbar_set_verbosity(int level)
{
	_zbar_verbosity = level;
}

void zbar_increase_verbosity()
{
	if(!_zbar_verbosity)
		_zbar_verbosity++;
	else
		_zbar_verbosity <<= 1;
}

int _zbar_error_spew(const void * container, int verbosity)
{
	const errinfo_t * err = static_cast<const errinfo_t *>(container);
	assert(err->magic == ERRINFO_MAGIC);
	fprintf(stderr, "%s", _zbar_error_string(err, verbosity));
	return (-err->sev);
}

zbar_error_t _zbar_get_error_code(const void * container)
{
	const errinfo_t * err = static_cast<const errinfo_t *>(container);
	assert(err->magic == ERRINFO_MAGIC);
	return err->type;
}

/* ERROR: zbar video in v4l1_set_format():
 *   system error: blah[: blah]
 */

const char * _zbar_error_string(const void * container, int verbosity)
{
	static const char basefmt[] = "%s: zbar %s in %s():\n    %s: ";
	errinfo_t * err = (errinfo_t *)container;
	const char * sev, * mod, * func, * type;
	int len;
	assert(err->magic == ERRINFO_MAGIC);
	if(err->sev >= SEV_FATAL && err->sev <= SEV_NOTE)
		sev = sev_str[err->sev + 2];
	else
		sev = sev_str[1];
	if(err->module >= ZBAR_MOD_PROCESSOR && err->module < ZBAR_MOD_UNKNOWN)
		mod = mod_str[err->module];
	else
		mod = mod_str[ZBAR_MOD_UNKNOWN];
	func = (err->func) ? err->func : "<unknown>";
	if(err->type >= 0 && err->type < ZBAR_ERR_NUM)
		type = err_str[err->type];
	else
		type = err_str[ZBAR_ERR_NUM];
	len = SEV_MAX + MOD_MAX + ERR_MAX + strlen(func) + sizeof(basefmt);
	err->buf = static_cast<char *>(SAlloc::R(err->buf, len));
	len = sprintf(err->buf, basefmt, sev, mod, func, type);
	if(len <= 0)
		return ("<unknown>");

	if(err->detail) {
		int newlen = len + strlen(err->detail) + 1;
		if(strstr(err->detail, "%s")) {
			if(!err->arg_str)
				err->arg_str = _strdup("<?>");
			err->buf = static_cast<char *>(SAlloc::R(err->buf, newlen + strlen(err->arg_str)));
			len += sprintf(err->buf + len, err->detail, err->arg_str);
		}
		else if(strstr(err->detail, "%d") || strstr(err->detail, "%x")) {
			err->buf = static_cast<char *>(SAlloc::R(err->buf, newlen + 32));
			len += sprintf(err->buf + len, err->detail, err->arg_int);
		}
		else {
			err->buf = static_cast<char *>(SAlloc::R(err->buf, newlen));
			len += sprintf(err->buf + len, "%s", err->detail);
		}
		if(len <= 0)
			return ("<unknown>");
	}
#ifdef HAVE_ERRNO_H
	if(err->type == ZBAR_ERR_SYSTEM) {
		static const char sysfmt[] = ": %s (%d)\n";
		const char * syserr = strerror(err->errnum);
		err->buf = SAlloc::R(err->buf, len + strlen(sysfmt) + strlen(syserr));
		len += sprintf(err->buf + len, sysfmt, syserr, err->errnum);
	}
#endif
#ifdef _WIN32
	else if(err->type == ZBAR_ERR_WINAPI) {
		char * syserr = NULL;
		if(::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err->errnum, 0, reinterpret_cast<LPTSTR>(&syserr), 1, NULL) && syserr) {
			char sysfmt[] = ": %s (%d)\n";
			err->buf = static_cast<char *>(SAlloc::R(err->buf, len + strlen(sysfmt) + strlen(syserr)));
			len += sprintf(err->buf + len, sysfmt, syserr, err->errnum);
			LocalFree(syserr);
		}
	}
#endif
	else {
		err->buf = static_cast<char *>(SAlloc::R(err->buf, len + 2));
		len += sprintf(err->buf + len, "\n");
	}
	return (err->buf);
}
//
//
//
void cdecl zprintf(int level, const char * pFormat, ...)
{
	if(_zbar_verbosity >= level) {
		va_list argptr;
		va_start(argptr, pFormat);
		fprintf(stderr, pFormat, argptr);
	}
}

int err_copy(void * dst_c, void * src_c)
{
	errinfo_t * dst = static_cast<errinfo_t *>(dst_c);
	errinfo_t * src = static_cast<errinfo_t *>(src_c);
	assert(dst->magic == ERRINFO_MAGIC);
	assert(src->magic == ERRINFO_MAGIC);
	dst->errnum = src->errnum;
	dst->sev = src->sev;
	dst->type = src->type;
	dst->func = src->func;
	dst->detail = src->detail;
	dst->arg_str = src->arg_str;
	src->arg_str = NULL; /* unused at src, avoid double free */
	dst->arg_int = src->arg_int;
	return -1;
}

int err_capture(const void * container, errsev_t sev, zbar_error_t type, const char * func, const char * detail)
{
	errinfo_t * err = static_cast<errinfo_t *>(const_cast<void *>(container)); // @badcast
	assert(err->magic == ERRINFO_MAGIC);
#ifdef HAVE_ERRNO_H
	if(type == ZBAR_ERR_SYSTEM)
		err->errnum = errno;
#endif
#ifdef _WIN32
	if(type == ZBAR_ERR_WINAPI)
		err->errnum = GetLastError();
#endif
	err->sev = sev;
	err->type = type;
	err->func = func;
	err->detail = detail;
	if(_zbar_verbosity >= 1)
		_zbar_error_spew(err, 0);
	return -1;
}

int err_capture_str(const void * container, errsev_t sev, zbar_error_t type, const char * func, const char * detail, const char * arg)
{
	errinfo_t * err = (errinfo_t *)container;
	assert(err->magic == ERRINFO_MAGIC);
	SAlloc::F(err->arg_str);
	err->arg_str = _strdup(arg);
	return err_capture(container, sev, type, func, detail);
}

int err_capture_int(const void * container, errsev_t sev, zbar_error_t type, const char * func, const char * detail, int arg)
{
	errinfo_t * err = (errinfo_t *)container;
	assert(err->magic == ERRINFO_MAGIC);
	err->arg_int = arg;
	return err_capture(container, sev, type, func, detail);
}

int err_capture_num(const void * container, errsev_t sev, zbar_error_t type, const char * func, const char * detail, int num)
{
	errinfo_t * err = (errinfo_t *)container;
	assert(err->magic == ERRINFO_MAGIC);
	err->errnum = num;
	return err_capture(container, sev, type, func, detail);
}

void err_init(errinfo_t * err, errmodule_t module)
{
	err->magic = ERRINFO_MAGIC;
	err->module = module;
}

void err_cleanup(errinfo_t * err)
{
	assert(err->magic == ERRINFO_MAGIC);
	ZFREE(err->buf);
	ZFREE(err->arg_str);
}
