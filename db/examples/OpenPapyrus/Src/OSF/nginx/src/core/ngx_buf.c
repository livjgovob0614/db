/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */
#include <ngx_config.h>
#include <ngx_core.h>
#pragma hdrstop

ngx_buf_t * FASTCALL ngx_create_temp_buf(ngx_pool_t * pool, size_t size)
{
	ngx_buf_t * b = (ngx_buf_t*)ngx_calloc_buf(pool);
	if(b) {
		b->start = (u_char *)ngx_palloc(pool, size);
		if(b->start == NULL) {
			return NULL;
		}
		else {
			/*
			 * set by ngx_calloc_buf():
			 *
			 *   b->file_pos = 0;
			 *   b->file_last = 0;
			 *   b->file = NULL;
			 *   b->shadow = NULL;
			 *   b->tag = 0;
			 *   and flags
			 */
			b->pos = b->start;
			b->last = b->start;
			b->end = b->last + size;
			b->temporary = 1;
		}
	}
	return b;
}

ngx_chain_t * FASTCALL ngx_alloc_chain_link(ngx_pool_t * pool)
{
	ngx_chain_t * cl = pool->chain;
	if(cl)
		pool->chain = cl->next;
	else
		cl = (ngx_chain_t *)ngx_palloc(pool, sizeof(ngx_chain_t));
	return cl;
}

ngx_chain_t * ngx_create_chain_of_bufs(ngx_pool_t * pool, ngx_bufs_t * bufs)
{
	ngx_int_t i;
	ngx_buf_t  * b;
	ngx_chain_t  * chain, * cl, ** ll;
	u_char * p = (u_char *)ngx_palloc(pool, bufs->num * bufs->size);
	if(!p) {
		return NULL;
	}
	ll = &chain;
	for(i = 0; i < bufs->num; i++) {
		b = (ngx_buf_t*)ngx_calloc_buf(pool);
		if(!b) {
			return NULL;
		}
		/*
		 * set by ngx_calloc_buf():
		 *
		 *   b->file_pos = 0;
		 *   b->file_last = 0;
		 *   b->file = NULL;
		 *   b->shadow = NULL;
		 *   b->tag = 0;
		 *   and flags
		 *
		 */
		b->pos = p;
		b->last = p;
		b->temporary = 1;
		b->start = p;
		p += bufs->size;
		b->end = p;
		cl = ngx_alloc_chain_link(pool);
		if(cl == NULL) {
			return NULL;
		}
		cl->buf = b;
		*ll = cl;
		ll = &cl->next;
	}
	*ll = NULL;
	return chain;
}

ngx_int_t ngx_chain_add_copy(ngx_pool_t * pool, ngx_chain_t ** chain, ngx_chain_t * in)
{
	ngx_chain_t  * cl;
	ngx_chain_t ** ll = chain;
	for(cl = *chain; cl; cl = cl->next) {
		ll = &cl->next;
	}
	while(in) {
		cl = ngx_alloc_chain_link(pool);
		if(cl == NULL) {
			return NGX_ERROR;
		}
		cl->buf = in->buf;
		*ll = cl;
		ll = &cl->next;
		in = in->next;
	}
	*ll = NULL;
	return NGX_OK;
}

ngx_chain_t * FASTCALL ngx_chain_get_free_buf(ngx_pool_t * p, ngx_chain_t ** ppFree)
{
	ngx_chain_t * cl;
	if(*ppFree) {
		cl = *ppFree;
		*ppFree = cl->next;
		cl->next = NULL;
		return cl;
	}
	cl = ngx_alloc_chain_link(p);
	if(cl == NULL) {
		return NULL;
	}
	cl->buf = (ngx_buf_t*)ngx_calloc_buf(p);
	if(cl->buf == NULL) {
		return NULL;
	}
	cl->next = NULL;
	return cl;
}

void ngx_chain_update_chains(ngx_pool_t * p, ngx_chain_t ** ppFree, ngx_chain_t ** ppBusy, ngx_chain_t ** out, ngx_buf_tag_t tag)
{
	ngx_chain_t * cl;
	if(*out) {
		if(*ppBusy == NULL) {
			*ppBusy = *out;
		}
		else {
			for(cl = *ppBusy; cl->next; cl = cl->next) { /* void */
			}
			cl->next = *out;
		}
		*out = NULL;
	}
	while(*ppBusy) {
		cl = *ppBusy;
		if(ngx_buf_size(cl->buf) != 0) {
			break;
		}
		else if(cl->buf->tag != tag) {
			*ppBusy = cl->next;
			ngx_free_chain(p, cl);
		}
		else {
			cl->buf->pos = cl->buf->start;
			cl->buf->last = cl->buf->start;
			*ppBusy = cl->next;
			cl->next = *ppFree;
			*ppFree = cl;
		}
	}
}

nginx_off_t ngx_chain_coalesce_file(ngx_chain_t ** in, nginx_off_t limit)
{
	nginx_off_t size, aligned, fprev;
	nginx_off_t total = 0;
	ngx_chain_t  * cl = *in;
	ngx_fd_t fd = cl->buf->file->fd;
	do {
		size = cl->buf->file_last - cl->buf->file_pos;
		if(size > limit - total) {
			size = limit - total;
			aligned = (cl->buf->file_pos + size + ngx_pagesize - 1) & ~((nginx_off_t)ngx_pagesize - 1);
			if(aligned <= cl->buf->file_last) {
				size = aligned - cl->buf->file_pos;
			}
			total += size;
			break;
		}
		total += size;
		fprev = cl->buf->file_pos + size;
		cl = cl->next;
	} while(cl && cl->buf->in_file && total < limit && fd == cl->buf->file->fd && fprev == cl->buf->file_pos);
	*in = cl;
	return total;
}

ngx_chain_t * ngx_chain_update_sent(ngx_chain_t * in, nginx_off_t sent)
{
	for(/* void */; in; in = in->next) {
		if(!ngx_buf_special(in->buf)) {
			if(sent == 0) {
				break;
			}
			else {
				nginx_off_t size = ngx_buf_size(in->buf);
				if(sent >= size) {
					sent -= size;
					if(ngx_buf_in_memory(in->buf))
						in->buf->pos = in->buf->last;
					if(in->buf->in_file)
						in->buf->file_pos = in->buf->file_last;
				}
				else {
					if(ngx_buf_in_memory(in->buf))
						in->buf->pos += (size_t)sent;
					if(in->buf->in_file)
						in->buf->file_pos += sent;
					break;
				}
			}
		}
	}
	return in;
}
