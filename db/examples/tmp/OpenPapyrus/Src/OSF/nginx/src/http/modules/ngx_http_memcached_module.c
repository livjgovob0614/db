/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */
#include <ngx_config.h>
#include <ngx_core.h>
#pragma hdrstop

struct ngx_http_memcached_loc_conf_t {
	ngx_http_upstream_conf_t upstream;
	ngx_int_t index;
	ngx_uint_t gzip_flag;
};

struct ngx_http_memcached_ctx_t {
	size_t rest;
	ngx_http_request_t * request;
	ngx_str_t key;
};

static ngx_int_t ngx_http_memcached_create_request(ngx_http_request_t * r);
static ngx_int_t ngx_http_memcached_reinit_request(ngx_http_request_t * r);
static ngx_int_t ngx_http_memcached_process_header(ngx_http_request_t * r);
static ngx_int_t ngx_http_memcached_filter_init(void * data);
static ngx_int_t ngx_http_memcached_filter(void * data, ssize_t bytes);
static void ngx_http_memcached_abort_request(ngx_http_request_t * r);
static void ngx_http_memcached_finalize_request(ngx_http_request_t * r, ngx_int_t rc);
static void * ngx_http_memcached_create_loc_conf(ngx_conf_t * cf);
static char * ngx_http_memcached_merge_loc_conf(ngx_conf_t * cf, void * parent, void * child);
static const char * ngx_http_memcached_pass(ngx_conf_t * cf, const ngx_command_t * cmd, void * conf); // F_SetHandler

static ngx_conf_bitmask_t ngx_http_memcached_next_upstream_masks[] = {
	{ ngx_string("error"), NGX_HTTP_UPSTREAM_FT_ERROR },
	{ ngx_string("timeout"), NGX_HTTP_UPSTREAM_FT_TIMEOUT },
	{ ngx_string("invalid_response"), NGX_HTTP_UPSTREAM_FT_INVALID_HEADER },
	{ ngx_string("not_found"), NGX_HTTP_UPSTREAM_FT_HTTP_404 },
	{ ngx_string("off"), NGX_HTTP_UPSTREAM_FT_OFF },
	{ ngx_null_string, 0 }
};

static ngx_command_t ngx_http_memcached_commands[] = {
	{ ngx_string("memcached_pass"), NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_TAKE1,
	  ngx_http_memcached_pass, NGX_HTTP_LOC_CONF_OFFSET, 0, NULL },
	{ ngx_string("memcached_bind"), NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE12,
	  ngx_http_upstream_bind_set_slot, NGX_HTTP_LOC_CONF_OFFSET, offsetof(ngx_http_memcached_loc_conf_t, upstream.local), NULL },
	{ ngx_string("memcached_connect_timeout"), NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
	  ngx_conf_set_msec_slot, NGX_HTTP_LOC_CONF_OFFSET, offsetof(ngx_http_memcached_loc_conf_t, upstream.connect_timeout), NULL },
	{ ngx_string("memcached_send_timeout"), NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
	  ngx_conf_set_msec_slot, NGX_HTTP_LOC_CONF_OFFSET, offsetof(ngx_http_memcached_loc_conf_t, upstream.send_timeout), NULL },
	{ ngx_string("memcached_buffer_size"), NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
	  ngx_conf_set_size_slot, NGX_HTTP_LOC_CONF_OFFSET, offsetof(ngx_http_memcached_loc_conf_t, upstream.buffer_size), NULL },
	{ ngx_string("memcached_read_timeout"), NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
	  ngx_conf_set_msec_slot, NGX_HTTP_LOC_CONF_OFFSET, offsetof(ngx_http_memcached_loc_conf_t, upstream.read_timeout), NULL },
	{ ngx_string("memcached_next_upstream"), NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_1MORE,
	  ngx_conf_set_bitmask_slot, NGX_HTTP_LOC_CONF_OFFSET, offsetof(ngx_http_memcached_loc_conf_t, upstream.next_upstream), &ngx_http_memcached_next_upstream_masks },
	{ ngx_string("memcached_next_upstream_tries"), NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
	  ngx_conf_set_num_slot, NGX_HTTP_LOC_CONF_OFFSET, offsetof(ngx_http_memcached_loc_conf_t, upstream.next_upstream_tries), NULL },
	{ ngx_string("memcached_next_upstream_timeout"), NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
	  ngx_conf_set_msec_slot, NGX_HTTP_LOC_CONF_OFFSET, offsetof(ngx_http_memcached_loc_conf_t, upstream.next_upstream_timeout), NULL },
	{ ngx_string("memcached_gzip_flag"), NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
	  ngx_conf_set_num_slot, NGX_HTTP_LOC_CONF_OFFSET, offsetof(ngx_http_memcached_loc_conf_t, gzip_flag), NULL },
	ngx_null_command
};

static ngx_http_module_t ngx_http_memcached_module_ctx = {
	NULL,                              /* preconfiguration */
	NULL,                              /* postconfiguration */
	NULL,                              /* create main configuration */
	NULL,                              /* init main configuration */
	NULL,                              /* create server configuration */
	NULL,                              /* merge server configuration */
	ngx_http_memcached_create_loc_conf, /* create location configuration */
	ngx_http_memcached_merge_loc_conf  /* merge location configuration */
};

ngx_module_t ngx_http_memcached_module = {
	NGX_MODULE_V1,
	&ngx_http_memcached_module_ctx,    /* module context */
	ngx_http_memcached_commands,       /* module directives */
	NGX_HTTP_MODULE,                   /* module type */
	NULL,                              /* init master */
	NULL,                              /* init module */
	NULL,                              /* init process */
	NULL,                              /* init thread */
	NULL,                              /* exit thread */
	NULL,                              /* exit process */
	NULL,                              /* exit master */
	NGX_MODULE_V1_PADDING
};

static ngx_str_t ngx_http_memcached_key = ngx_string("memcached_key");

#define NGX_HTTP_MEMCACHED_END   (sizeof(ngx_http_memcached_end) - 1)
static u_char ngx_http_memcached_end[] = CRLF "END" CRLF;

static ngx_int_t ngx_http_memcached_handler(ngx_http_request_t * pReq)
{
	ngx_int_t rc;
	ngx_http_upstream_t * u;
	if(!(pReq->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
		return NGX_HTTP_NOT_ALLOWED;
	}
	rc = ngx_http_discard_request_body(pReq);
	if(rc != NGX_OK) {
		return rc;
	}
	if(ngx_http_set_content_type(pReq) != NGX_OK) {
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}
	if(ngx_http_upstream_create(pReq) != NGX_OK) {
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}
	u = pReq->upstream;
	ngx_str_set(&u->schema, "memcached://");
	u->output.tag = (ngx_buf_tag_t)&ngx_http_memcached_module;
	{
		ngx_http_memcached_loc_conf_t * mlcf = (ngx_http_memcached_loc_conf_t*)ngx_http_get_module_loc_conf(pReq, ngx_http_memcached_module);
		u->conf = &mlcf->upstream;
		u->create_request = ngx_http_memcached_create_request;
		u->reinit_request = ngx_http_memcached_reinit_request;
		u->process_header = ngx_http_memcached_process_header;
		u->abort_request = ngx_http_memcached_abort_request;
		u->finalize_request = ngx_http_memcached_finalize_request;
		{
			ngx_http_memcached_ctx_t * ctx = (ngx_http_memcached_ctx_t *)ngx_palloc(pReq->pool, sizeof(ngx_http_memcached_ctx_t));
			if(!ctx) {
				return NGX_HTTP_INTERNAL_SERVER_ERROR;
			}
			ctx->request = pReq;
			ngx_http_set_ctx(pReq, ctx, ngx_http_memcached_module);
			u->input_filter_init = ngx_http_memcached_filter_init;
			u->input_filter = ngx_http_memcached_filter;
			u->input_filter_ctx = ctx;
			pReq->main->count++;
			ngx_http_upstream_init(pReq);
		}
	}
	return NGX_DONE;
}

static ngx_int_t ngx_http_memcached_create_request(ngx_http_request_t * pReq)
{
	size_t len;
	uintptr_t escape;
	ngx_buf_t  * b;
	ngx_chain_t  * cl;
	ngx_http_memcached_loc_conf_t  * mlcf = (ngx_http_memcached_loc_conf_t*)ngx_http_get_module_loc_conf(pReq, ngx_http_memcached_module);
	ngx_http_variable_value_t * vv = ngx_http_get_indexed_variable(pReq, mlcf->index);
	if(vv == NULL || vv->not_found || vv->len == 0) {
		ngx_log_error(NGX_LOG_ERR, pReq->connection->log, 0, "the \"$memcached_key\" variable is not set");
		return NGX_ERROR;
	}
	escape = 2 * ngx_escape_uri(NULL, vv->data, vv->len, NGX_ESCAPE_MEMCACHED);
	len = sizeof("get ") - 1 + vv->len + escape + sizeof(CRLF) - 1;
	b = ngx_create_temp_buf(pReq->pool, len);
	if(!b) {
		return NGX_ERROR;
	}
	cl = ngx_alloc_chain_link(pReq->pool);
	if(cl == NULL) {
		return NGX_ERROR;
	}
	cl->buf = b;
	cl->next = NULL;
	pReq->upstream->request_bufs = cl;
	*b->last++ = 'g'; *b->last++ = 'e'; *b->last++ = 't'; *b->last++ = ' ';
	{
		ngx_http_memcached_ctx_t  * ctx = (ngx_http_memcached_ctx_t *)ngx_http_get_module_ctx(pReq, ngx_http_memcached_module);
		ctx->key.data = b->last;
		if(escape == 0) {
			b->last = ngx_copy(b->last, vv->data, vv->len);
		}
		else {
			b->last = (u_char *)ngx_escape_uri(b->last, vv->data, vv->len, NGX_ESCAPE_MEMCACHED);
		}
		ctx->key.len = b->last - ctx->key.data;
		ngx_log_debug1(NGX_LOG_DEBUG_HTTP, pReq->connection->log, 0, "http memcached request: \"%V\"", &ctx->key);
		*b->last++ = __CR; *b->last++ = LF;
	}
	return NGX_OK;
}

static ngx_int_t ngx_http_memcached_reinit_request(ngx_http_request_t * r)
{
	return NGX_OK;
}

static ngx_int_t ngx_http_memcached_process_header(ngx_http_request_t * r)
{
	u_char * p, * start;
	ngx_str_t line;
	ngx_uint_t flags;
	ngx_table_elt_t  * h;
	ngx_http_upstream_t * u = r->upstream;
	int    found = 0;
	for(p = u->buffer.pos; !found && p < u->buffer.last; p++) {
		if(*p == LF)
			found = 1;
	}
	if(!found)
		return NGX_AGAIN;
	else {
		line.data = u->buffer.pos;
		line.len = p - u->buffer.pos;
		if(line.len == 0 || *(p - 1) != __CR) {
			goto no_valid__;
		}
		*p = '\0';
		line.len--;
		ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "memcached: \"%V\"", &line);
		p = u->buffer.pos;
		{
			ngx_http_memcached_ctx_t * ctx = (ngx_http_memcached_ctx_t *)ngx_http_get_module_ctx(r, ngx_http_memcached_module);
			ngx_http_memcached_loc_conf_t * mlcf = (ngx_http_memcached_loc_conf_t*)ngx_http_get_module_loc_conf(r, ngx_http_memcached_module);
			if(ngx_strncmp(p, "VALUE ", sizeof("VALUE ") - 1) == 0) {
				p += sizeof("VALUE ") - 1;
				if(ngx_strncmp(p, ctx->key.data, ctx->key.len) != 0) {
					ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "memcached sent invalid key in response \"%V\" for key \"%V\"", &line, &ctx->key);
					return NGX_HTTP_UPSTREAM_INVALID_HEADER;
				}
				p += ctx->key.len;
				if(*p++ != ' ') {
					goto no_valid__;
				}
				// flags 
				start = p;
				while(*p) {
					if(*p++ == ' ') {
						if(mlcf->gzip_flag) {
							goto flags__;
						}
						else {
							goto length__;
						}
					}
				}
				goto no_valid__;
flags__:
				flags = ngx_atoi(start, p - start - 1);
				if(flags == (ngx_uint_t)NGX_ERROR) {
					ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "memcached sent invalid flags in response \"%V\" for key \"%V\"", &line, &ctx->key);
					return NGX_HTTP_UPSTREAM_INVALID_HEADER;
				}
				if(flags & mlcf->gzip_flag) {
					h = (ngx_table_elt_t*)ngx_list_push(&r->headers_out.headers);
					if(h == NULL) {
						return NGX_ERROR;
					}
					h->hash = 1;
					ngx_str_set(&h->key, "Content-Encoding");
					ngx_str_set(&h->value, "gzip");
					r->headers_out.content_encoding = h;
				}
length__:
				start = p;
				p = line.data + line.len;
				u->headers_in.content_length_n = ngx_atoof(start, p - start);
				if(u->headers_in.content_length_n == NGX_ERROR) {
					ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "memcached sent invalid length in response \"%V\" for key \"%V\"", &line, &ctx->key);
					return NGX_HTTP_UPSTREAM_INVALID_HEADER;
				}
				else {
					u->headers_in.status_n = 200;
					u->state->status = 200;
					u->buffer.pos = p + sizeof(CRLF) - 1;
					return NGX_OK;
				}
			}
			if(ngx_strcmp(p, "END\x0d") == 0) {
				ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, "key: \"%V\" was not found by memcached", &ctx->key);
				u->headers_in.content_length_n = 0;
				u->headers_in.status_n = 404;
				u->state->status = 404;
				u->buffer.pos = p + sizeof("END" CRLF) - 1;
				u->keepalive = 1;
				return NGX_OK;
			}
		}
no_valid__:
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "memcached sent invalid response: \"%V\"", &line);
		return NGX_HTTP_UPSTREAM_INVALID_HEADER;
	}
}

static ngx_int_t ngx_http_memcached_filter_init(void * data)
{
	ngx_http_memcached_ctx_t  * ctx = (ngx_http_memcached_ctx_t *)data;
	ngx_http_upstream_t  * u = ctx->request->upstream;
	if(u->headers_in.status_n != 404) {
		u->length = u->headers_in.content_length_n + NGX_HTTP_MEMCACHED_END;
		ctx->rest = NGX_HTTP_MEMCACHED_END;
	}
	else {
		u->length = 0;
	}
	return NGX_OK;
}

static ngx_int_t ngx_http_memcached_filter(void * data, ssize_t bytes)
{
	ngx_http_memcached_ctx_t  * ctx = (ngx_http_memcached_ctx_t *)data;
	u_char * last;
	ngx_chain_t   * cl, ** ll;
	ngx_http_upstream_t  * u = ctx->request->upstream;
	ngx_buf_t * b = &u->buffer;
	if(u->length == (ssize_t)ctx->rest) {
		if(ngx_strncmp(b->last, ngx_http_memcached_end + NGX_HTTP_MEMCACHED_END - ctx->rest, bytes) != 0) {
			ngx_log_error(NGX_LOG_ERR, ctx->request->connection->log, 0, "memcached sent invalid trailer");
			u->length = 0;
			ctx->rest = 0;
			return NGX_OK;
		}
		u->length -= bytes;
		ctx->rest -= bytes;
		if(u->length == 0) {
			u->keepalive = 1;
		}
		return NGX_OK;
	}
	for(cl = u->out_bufs, ll = &u->out_bufs; cl; cl = cl->next) {
		ll = &cl->next;
	}
	cl = ngx_chain_get_free_buf(ctx->request->pool, &u->free_bufs);
	if(cl == NULL) {
		return NGX_ERROR;
	}
	cl->buf->flush = 1;
	cl->buf->memory = 1;
	*ll = cl;
	last = b->last;
	cl->buf->pos = last;
	b->last += bytes;
	cl->buf->last = b->last;
	cl->buf->tag = u->output.tag;
	ngx_log_debug4(NGX_LOG_DEBUG_HTTP, ctx->request->connection->log, 0, "memcached filter bytes:%z size:%z length:%O rest:%z", bytes, b->last - b->pos, u->length, ctx->rest);
	if(bytes <= (ssize_t)(u->length - NGX_HTTP_MEMCACHED_END)) {
		u->length -= bytes;
		return NGX_OK;
	}
	last += (size_t)(u->length - NGX_HTTP_MEMCACHED_END);
	if(ngx_strncmp(last, ngx_http_memcached_end, b->last - last) != 0) {
		ngx_log_error(NGX_LOG_ERR, ctx->request->connection->log, 0, "memcached sent invalid trailer");
		b->last = last;
		cl->buf->last = last;
		u->length = 0;
		ctx->rest = 0;
		return NGX_OK;
	}
	ctx->rest -= b->last - last;
	b->last = last;
	cl->buf->last = last;
	u->length = ctx->rest;
	if(u->length == 0) {
		u->keepalive = 1;
	}
	return NGX_OK;
}

static void ngx_http_memcached_abort_request(ngx_http_request_t * r)
{
	ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "abort http memcached request");
}

static void ngx_http_memcached_finalize_request(ngx_http_request_t * r, ngx_int_t rc)
{
	ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "finalize http memcached request");
}

static void * ngx_http_memcached_create_loc_conf(ngx_conf_t * cf)
{
	ngx_http_memcached_loc_conf_t  * conf = (ngx_http_memcached_loc_conf_t*)ngx_pcalloc(cf->pool, sizeof(ngx_http_memcached_loc_conf_t));
	if(conf) {
		/*
		 * set by ngx_pcalloc():
		 *
		 *   conf->upstream.bufs.num = 0;
		 *   conf->upstream.next_upstream = 0;
		 *   conf->upstream.temp_path = NULL;
		 *   conf->upstream.uri = { 0, NULL };
		 *   conf->upstream.location = NULL;
		 */
		conf->upstream.local = (ngx_http_upstream_local_t*)NGX_CONF_UNSET_PTR;
		conf->upstream.next_upstream_tries = NGX_CONF_UNSET_UINT;
		conf->upstream.connect_timeout = NGX_CONF_UNSET_MSEC;
		conf->upstream.send_timeout = NGX_CONF_UNSET_MSEC;
		conf->upstream.read_timeout = NGX_CONF_UNSET_MSEC;
		conf->upstream.next_upstream_timeout = NGX_CONF_UNSET_MSEC;
		conf->upstream.buffer_size = NGX_CONF_UNSET_SIZE;
		/* the hardcoded values */
		conf->upstream.cyclic_temp_file = 0;
		conf->upstream.buffering = 0;
		conf->upstream.ignore_client_abort = 0;
		conf->upstream.send_lowat = 0;
		conf->upstream.bufs.num = 0;
		conf->upstream.busy_buffers_size = 0;
		conf->upstream.max_temp_file_size = 0;
		conf->upstream.temp_file_write_size = 0;
		conf->upstream.intercept_errors = 1;
		conf->upstream.intercept_404 = 1;
		conf->upstream.pass_request_headers = 0;
		conf->upstream.pass_request_body = 0;
		conf->upstream.force_ranges = 1;
		conf->index = NGX_CONF_UNSET;
		conf->gzip_flag = NGX_CONF_UNSET_UINT;
	}
	return conf;
}

static char * ngx_http_memcached_merge_loc_conf(ngx_conf_t * cf, void * parent, void * child)
{
	ngx_http_memcached_loc_conf_t * prev = (ngx_http_memcached_loc_conf_t *)parent;
	ngx_http_memcached_loc_conf_t * conf = (ngx_http_memcached_loc_conf_t *)child;
	ngx_conf_merge_ptr_value(conf->upstream.local, prev->upstream.local, NULL);
	ngx_conf_merge_uint_value(conf->upstream.next_upstream_tries, prev->upstream.next_upstream_tries, 0);
	ngx_conf_merge_msec_value(conf->upstream.connect_timeout, prev->upstream.connect_timeout, 60000);
	ngx_conf_merge_msec_value(conf->upstream.send_timeout, prev->upstream.send_timeout, 60000);
	ngx_conf_merge_msec_value(conf->upstream.read_timeout, prev->upstream.read_timeout, 60000);
	ngx_conf_merge_msec_value(conf->upstream.next_upstream_timeout, prev->upstream.next_upstream_timeout, 0);
	ngx_conf_merge_size_value(conf->upstream.buffer_size, prev->upstream.buffer_size, (size_t)ngx_pagesize);
	ngx_conf_merge_bitmask_value(conf->upstream.next_upstream,
	    prev->upstream.next_upstream, (NGX_CONF_BITMASK_SET|NGX_HTTP_UPSTREAM_FT_ERROR|NGX_HTTP_UPSTREAM_FT_TIMEOUT));
	if(conf->upstream.next_upstream & NGX_HTTP_UPSTREAM_FT_OFF) {
		conf->upstream.next_upstream = NGX_CONF_BITMASK_SET|NGX_HTTP_UPSTREAM_FT_OFF;
	}
	if(conf->upstream.upstream == NULL) {
		conf->upstream.upstream = prev->upstream.upstream;
	}
	if(conf->index == NGX_CONF_UNSET) {
		conf->index = prev->index;
	}
	ngx_conf_merge_uint_value(conf->gzip_flag, prev->gzip_flag, 0);
	return NGX_CONF_OK;
}

static const char * ngx_http_memcached_pass(ngx_conf_t * cf, const ngx_command_t * cmd, void * conf) // F_SetHandler
{
	ngx_http_memcached_loc_conf_t * mlcf = (ngx_http_memcached_loc_conf_t *)conf;
	ngx_url_t u;
	ngx_http_core_loc_conf_t  * clcf;
	if(mlcf->upstream.upstream) {
		return "is duplicate";
	}
	else {
		ngx_str_t * value = static_cast<ngx_str_t *>(cf->args->elts);
		memzero(&u, sizeof(ngx_url_t));
		u.url = value[1];
		u.no_resolve = 1;
		mlcf->upstream.upstream = ngx_http_upstream_add(cf, &u, 0);
		if(mlcf->upstream.upstream == NULL) {
			return NGX_CONF_ERROR;
		}
		clcf = (ngx_http_core_loc_conf_t*)ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
		clcf->F_HttpHandler = ngx_http_memcached_handler;
		if(clcf->name.data[clcf->name.len - 1] == '/') {
			clcf->auto_redirect = 1;
		}
		mlcf->index = ngx_http_get_variable_index(cf, &ngx_http_memcached_key);
		if(mlcf->index == NGX_ERROR) {
			return NGX_CONF_ERROR;
		}
		return NGX_CONF_OK;
	}
}

