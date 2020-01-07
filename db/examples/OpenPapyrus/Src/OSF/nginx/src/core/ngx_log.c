/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */
#include <ngx_config.h>
#include <ngx_core.h>
#pragma hdrstop

static const char * ngx_error_log(ngx_conf_t * cf, const ngx_command_t * cmd, void * conf); // F_SetHandler
static char * ngx_log_set_levels(ngx_conf_t * cf, ngx_log_t * log);
static void ngx_log_insert(ngx_log_t * log, ngx_log_t * new_log);

#if (NGX_DEBUG)

static void ngx_log_memory_writer(ngx_log_t * log, ngx_uint_t level, u_char * buf, size_t len);
static void ngx_log_memory_cleanup(void * data);

struct ngx_log_memory_buf_t {
	u_char * start;
	u_char * end;
	u_char * pos;
	ngx_atomic_t written;
};

#endif

static ngx_command_t ngx_errlog_commands[] = {
	{ ngx_string("error_log"), NGX_MAIN_CONF|NGX_CONF_1MORE, ngx_error_log, 0, 0, NULL },
	ngx_null_command
};

static ngx_core_module_t ngx_errlog_module_ctx = {
	ngx_string("errlog"), NULL, NULL
};

ngx_module_t ngx_errlog_module = {
	NGX_MODULE_V1,
	&ngx_errlog_module_ctx,            /* module context */
	ngx_errlog_commands,               /* module directives */
	NGX_CORE_MODULE,                   /* module type */
	NULL,                              /* init master */
	NULL,                              /* init module */
	NULL,                              /* init process */
	NULL,                              /* init thread */
	NULL,                              /* exit thread */
	NULL,                              /* exit process */
	NULL,                              /* exit master */
	NGX_MODULE_V1_PADDING
};

static ngx_log_t ngx_log;
static ngx_open_file_t ngx_log_file;
ngx_uint_t ngx_use_stderr = 1;

static ngx_str_t err_levels[] = {
	ngx_null_string,
	ngx_string("emerg"),
	ngx_string("alert"),
	ngx_string("crit"),
	ngx_string("error"),
	ngx_string("warn"),
	ngx_string("notice"),
	ngx_string("info"),
	ngx_string("debug")
};

static const char * debug_levels[] = { "debug_core", "debug_alloc", "debug_mutex", "debug_event", "debug_http", "debug_mail", "debug_stream" };

#if (NGX_HAVE_VARIADIC_MACROS)
	void ngx_log_error_core(ngx_uint_t level, ngx_log_t * log, ngx_err_t err, const char * fmt, ...)
#else
	void ngx_log_error_core(ngx_uint_t level, ngx_log_t * log, ngx_err_t err, const char * fmt, va_list args)
#endif
{
#if (NGX_HAVE_VARIADIC_MACROS)
	va_list args;
#endif
	u_char * msg;
	ssize_t n;
	ngx_uint_t wrote_stderr, debug_connection;
	u_char errstr[NGX_MAX_ERROR_STR];
	u_char * last = errstr + NGX_MAX_ERROR_STR;
	u_char * p = ngx_cpymem(errstr, ngx_cached_err_log_time.data, ngx_cached_err_log_time.len);
	p = ngx_slprintf(p, last, " [%V] ", &err_levels[level]);
	/* pid#tid */
	p = ngx_slprintf(p, last, "%P#" NGX_TID_T_FMT ": ", ngx_log_pid, ngx_log_tid);
	if(log->connection) {
		p = ngx_slprintf(p, last, "*%uA ", log->connection);
	}
	msg = p;
#if (NGX_HAVE_VARIADIC_MACROS)
	va_start(args, fmt);
	p = ngx_vslprintf(p, last, fmt, args);
	va_end(args);
#else
	p = ngx_vslprintf(p, last, fmt, args);
#endif
	if(err) {
		p = ngx_log_errno(p, last, err);
	}
	if(level != NGX_LOG_DEBUG && log->handler) {
		p = log->handler(log, p, last - p);
	}
	if(p > last - NGX_LINEFEED_SIZE) {
		p = last - NGX_LINEFEED_SIZE;
	}
	ngx_linefeed(p);
	wrote_stderr = 0;
	debug_connection = (log->Level & NGX_LOG_DEBUG_CONNECTION) != 0;
	while(log) {
		if(log->Level < level && !debug_connection) {
			break;
		}
		if(log->writer) {
			log->writer(log, level, errstr, p - errstr);
			goto next;
		}
		if(ngx_time() == log->disk_full_time) {
			/*
			 * on FreeBSD writing to a full filesystem with enabled softupdates
			 * may block process for much longer time than writing to non-full
			 * filesystem, so we skip writing to a log for one second
			 */
			goto next;
		}
		n = ngx_write_fd(log->file->fd, errstr, p - errstr);
		if(n == -1 && ngx_errno == NGX_ENOSPC) {
			log->disk_full_time = ngx_time();
		}
		if(log->file->fd == ngx_stderr) {
			wrote_stderr = 1;
		}
next:
		log = log->next;
	}
	if(ngx_use_stderr && level <= NGX_LOG_WARN && !wrote_stderr) {
		msg -= (7 + err_levels[level].len + 3);
		(void)ngx_sprintf(msg, "nginx: [%V] ", &err_levels[level]);
		(void)ngx_write_console(ngx_stderr, msg, p - msg);
	}
}

#if !(NGX_HAVE_VARIADIC_MACROS)

void ngx_cdecl ngx_log_error(ngx_uint_t level, ngx_log_t * log, ngx_err_t err, const char * fmt, ...)
{
	va_list args;
	if(log->Level >= level) {
		va_start(args, fmt);
		ngx_log_error_core(level, log, err, fmt, args);
		va_end(args);
	}
}

void ngx_cdecl ngx_log_debug_core(ngx_log_t * log, ngx_err_t err, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ngx_log_error_core(NGX_LOG_DEBUG, log, err, fmt, args);
	va_end(args);
}

#endif

void ngx_cdecl ngx_log_abort(ngx_err_t err, const char * fmt, ...)
{
	u_char * p;
	va_list args;
	u_char errstr[NGX_MAX_CONF_ERRSTR];
	va_start(args, fmt);
	p = ngx_vsnprintf(errstr, sizeof(errstr) - 1, fmt, args);
	va_end(args);
	ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, err, "%*s", p - errstr, errstr);
}

void ngx_cdecl ngx_log_stderr(ngx_err_t err, const char * fmt, ...)
{
	va_list args;
	u_char errstr[NGX_MAX_ERROR_STR];
	u_char * last = errstr + NGX_MAX_ERROR_STR;
	u_char * p = ngx_cpymem(errstr, "nginx: ", 7);
	va_start(args, fmt);
	p = ngx_vslprintf(p, last, fmt, args);
	va_end(args);
	if(err) {
		p = ngx_log_errno(p, last, err);
	}
	if(p > last - NGX_LINEFEED_SIZE) {
		p = last - NGX_LINEFEED_SIZE;
	}
	ngx_linefeed(p);
	(void)ngx_write_console(ngx_stderr, errstr, p - errstr);
}

u_char * ngx_log_errno(u_char * buf, u_char * last, ngx_err_t err)
{
	if(buf > last - 50) {
		/* leave a space for an error code */
		buf = last - 50;
		*buf++ = '.';
		*buf++ = '.';
		*buf++ = '.';
	}
#if (NGX_WIN32)
	buf = ngx_slprintf(buf, last, ((uint)err < 0x80000000) ? " (%d: " : " (%Xd: ", err);
#else
	buf = ngx_slprintf(buf, last, " (%d: ", err);
#endif
	buf = ngx_strerror(err, buf, last - buf);
	if(buf < last) {
		*buf++ = ')';
	}
	return buf;
}

ngx_log_t * ngx_log_init(const u_char * prefix)
{
	u_char * p = 0; // ��������� �� �������������� ������ (��� ��������� ��������)
	ngx_log.file = &ngx_log_file;
	ngx_log.Level = NGX_LOG_NOTICE;
	u_char * p_name = (u_char *)NGX_ERROR_LOG_PATH;
	// 
	// we use ngx_strlen() here since BCC warns about condition is always false and unreachable code
	// 
	const size_t nlen = ngx_strlen(p_name);
	if(nlen == 0)
		ngx_log_file.fd = ngx_stderr;
	else {
		size_t plen;
#if (NGX_WIN32)
		if(p_name[1] != ':') {
#else
		if(p_org_name[0] != '/') {
#endif
			if(prefix) {
				plen = ngx_strlen(prefix);
			}
			else {
#ifdef NGX_PREFIX
				prefix = (u_char *)NGX_PREFIX;
				plen = ngx_strlen(prefix);
#else
				plen = 0;
#endif
			}
			if(plen) {
				p_name = (u_char *)malloc(plen + nlen + 2);
				if(p_name == NULL) {
					return NULL;
				}
				p = ngx_cpymem(p_name, prefix, plen);
				if(!ngx_path_separator(*(p - 1))) {
					*p++ = '/';
				}
				ngx_cpystrn(p, (u_char *)NGX_ERROR_LOG_PATH, nlen + 1);
				p = p_name;
			}
		}
		ngx_log_file.fd = ngx_open_file(p_name, NGX_FILE_APPEND, NGX_FILE_CREATE_OR_OPEN, NGX_FILE_DEFAULT_ACCESS);
		if(ngx_log_file.fd == NGX_INVALID_FILE) {
			ngx_log_stderr(ngx_errno, "[alert] could not open error log file: " ngx_open_file_n " \"%s\" failed", p_name);
#if (NGX_WIN32)
			ngx_event_log(ngx_errno, "could not open error log file: " ngx_open_file_n " \"%s\" failed", p_name);
#endif
			ngx_log_file.fd = ngx_stderr;
		}
		if(p) {
			ngx_free(p);
		}
	}
	return &ngx_log;
}

ngx_int_t ngx_log_open_default(ngx_cycle_t * cycle)
{
	static ngx_str_t error_log = ngx_string(NGX_ERROR_LOG_PATH);
	if(!ngx_log_get_file_log(&cycle->new_log)) {
		ngx_log_t * log;
		if(cycle->new_log.Level) {
			// there are some error logs, but no files 
			log = (ngx_log_t *)ngx_pcalloc(cycle->pool, sizeof(ngx_log_t));
			if(log == NULL) {
				return NGX_ERROR;
			}
		}
		else {
			log = &cycle->new_log; // no error logs at all 
		}
		log->Level = NGX_LOG_ERR;
		log->file = ngx_conf_open_file(cycle, &error_log);
		if(log->file == NULL) {
			return NGX_ERROR;
		}
		if(log != &cycle->new_log) {
			ngx_log_insert(&cycle->new_log, log);
		}
	}
	return NGX_OK;
}

ngx_int_t ngx_log_redirect_stderr(ngx_cycle_t * cycle)
{
	if(!cycle->log_use_stderr) {
		// file log always exists when we are called 
		ngx_fd_t fd = ngx_log_get_file_log(cycle->log)->file->fd;
		if(fd != ngx_stderr) {
			if(ngx_set_stderr(fd) == NGX_FILE_ERROR) {
				ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno, ngx_set_stderr_n " failed");
				return NGX_ERROR;
			}
		}
	}
	return NGX_OK;
}

ngx_log_t * ngx_log_get_file_log(ngx_log_t * head)
{
	for(ngx_log_t * log = head; log; log = log->next) {
		if(log->file)
			return log;
	}
	return NULL;
}

static char * ngx_log_set_levels(ngx_conf_t * cf, ngx_log_t * log)
{
	ngx_uint_t i, n, d, found;
	if(cf->args->nelts == 2) {
		log->Level = NGX_LOG_ERR;
	}
	else {
		ngx_str_t * value = static_cast<ngx_str_t *>(cf->args->elts);
		for(i = 2; i < cf->args->nelts; i++) {
			found = 0;
			for(n = 1; n <= NGX_LOG_DEBUG; n++) {
				if(ngx_strcmp(value[i].data, err_levels[n].data) == 0) {
					if(log->Level) {
						ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "duplicate log level \"%V\"", &value[i]);
						return NGX_CONF_ERROR;
					}
					log->Level = n;
					found = 1;
					break;
				}
			}
			for(n = 0, d = NGX_LOG_DEBUG_FIRST; d <= NGX_LOG_DEBUG_LAST; d <<= 1) {
				if(ngx_strcmp(value[i].data, debug_levels[n++]) == 0) {
					if(log->Level & ~NGX_LOG_DEBUG_ALL) {
						ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "invalid log level \"%V\"", &value[i]);
						return NGX_CONF_ERROR;
					}
					log->Level |= d;
					found = 1;
					break;
				}
			}
			if(!found) {
				ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "invalid log level \"%V\"", &value[i]);
				return NGX_CONF_ERROR;
			}
		}
		if(log->Level == NGX_LOG_DEBUG)
			log->Level = NGX_LOG_DEBUG_ALL;
	}
	return NGX_CONF_OK;
}

static const char * ngx_error_log(ngx_conf_t * cf, const ngx_command_t * cmd, void * conf) // F_SetHandler
{
	ngx_log_t * dummy = &cf->cycle->new_log;
	return ngx_log_set_log(cf, &dummy);
}

char * ngx_log_set_log(ngx_conf_t * cf, ngx_log_t ** head)
{
	ngx_log_t * new_log;
	ngx_str_t * value, name;
	ngx_syslog_peer_t  * peer;
	if(*head && (*head)->Level == 0) {
		new_log = *head;
	}
	else {
		new_log = (ngx_log_t *)ngx_pcalloc(cf->pool, sizeof(ngx_log_t));
		if(new_log == NULL) {
			return NGX_CONF_ERROR;
		}
		if(*head == NULL) {
			*head = new_log;
		}
	}
	value = static_cast<ngx_str_t *>(cf->args->elts);
	if(ngx_strcmp(value[1].data, "stderr") == 0) {
		ngx_str_null(&name);
		cf->cycle->log_use_stderr = 1;
		new_log->file = ngx_conf_open_file(cf->cycle, &name);
		if(new_log->file == NULL) {
			return NGX_CONF_ERROR;
		}
	}
	else if(ngx_strncmp(value[1].data, "memory:", 7) == 0) {
#if (NGX_DEBUG)
		size_t size, needed;
		ngx_pool_cleanup_t  * cln;
		ngx_log_memory_buf_t  * buf;
		value[1].len -= 7;
		value[1].data += 7;
		needed = sizeof("MEMLOG  :" NGX_LINEFEED) + cf->conf_file->file.name.len + NGX_SIZE_T_LEN + NGX_INT_T_LEN + NGX_MAX_ERROR_STR;
		size = ngx_parse_size(&value[1]);
		if(size == (size_t)NGX_ERROR || size < needed) {
			ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "invalid buffer size \"%V\"", &value[1]);
			return NGX_CONF_ERROR;
		}
		buf = (ngx_log_memory_buf_t *)ngx_pcalloc(cf->pool, sizeof(ngx_log_memory_buf_t));
		if(!buf) {
			return NGX_CONF_ERROR;
		}
		buf->start = static_cast<u_char *>(ngx_pnalloc(cf->pool, size));
		if(buf->start == NULL) {
			return NGX_CONF_ERROR;
		}
		buf->end = buf->start + size;
		buf->pos = ngx_slprintf(buf->start, buf->end, "MEMLOG %uz %V:%ui%N", size, &cf->conf_file->file.name, cf->conf_file->line);
		memset(buf->pos, ' ', buf->end - buf->pos);
		cln = ngx_pool_cleanup_add(cf->pool, 0);
		if(cln == NULL) {
			return NGX_CONF_ERROR;
		}
		cln->data = new_log;
		cln->handler = ngx_log_memory_cleanup;
		new_log->writer = ngx_log_memory_writer;
		new_log->wdata = buf;
#else
		ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "nginx was built without debug support");
		return NGX_CONF_ERROR;
#endif
	}
	else if(ngx_strncmp(value[1].data, "syslog:", 7) == 0) {
		peer = (ngx_syslog_peer_t *)ngx_pcalloc(cf->pool, sizeof(ngx_syslog_peer_t));
		if(peer == NULL) {
			return NGX_CONF_ERROR;
		}
		if(ngx_syslog_process_conf(cf, peer) != NGX_CONF_OK) {
			return NGX_CONF_ERROR;
		}
		new_log->writer = ngx_syslog_writer;
		new_log->wdata = peer;
	}
	else {
		new_log->file = ngx_conf_open_file(cf->cycle, &value[1]);
		if(new_log->file == NULL) {
			return NGX_CONF_ERROR;
		}
	}
	if(ngx_log_set_levels(cf, new_log) != NGX_CONF_OK) {
		return NGX_CONF_ERROR;
	}
	if(*head != new_log) {
		ngx_log_insert(*head, new_log);
	}
	return NGX_CONF_OK;
}

static void ngx_log_insert(ngx_log_t * pLog, ngx_log_t * pNewLog)
{
	if(pNewLog->Level > pLog->Level) {
		// 
		// list head address is permanent, insert new log after head and swap its contents with head
		// 
		//ngx_log_t tmp = *pLog;
		//*pLog = *pNewLog;
		//*pNewLog = tmp;
		memswap(pLog, pNewLog, sizeof(*pLog));
		pLog->next = pNewLog;
	}
	else {
		while(pLog->next) {
			if(pNewLog->Level > pLog->next->Level) {
				pNewLog->next = pLog->next;
				pLog->next = pNewLog;
				return;
			}
			pLog = pLog->next;
		}
		pLog->next = pNewLog;
	}
}

#if (NGX_DEBUG)

static void ngx_log_memory_writer(ngx_log_t * log, ngx_uint_t level, u_char * buf, size_t len)
{
	ngx_log_memory_buf_t * mem = (ngx_log_memory_buf_t *)log->wdata;
	if(mem) {
		size_t written = ngx_atomic_fetch_add(&mem->written, len);
		u_char * p = mem->pos + written % (mem->end - mem->pos);
		size_t avail = mem->end - p;
		if(avail >= len) {
			memcpy(p, buf, len);
		}
		else {
			memcpy(p, buf, avail);
			memcpy(mem->pos, buf + avail, len - avail);
		}
	}
}

static void ngx_log_memory_cleanup(void * data)
{
	ngx_log_t * log = (ngx_log_t *)data;
	ngx_log_debug0(NGX_LOG_DEBUG_CORE, log, 0, "destroy memory log buffer");
	log->wdata = NULL;
}

#endif
