/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#include "internal/cryptlib.h"
#pragma hdrstop
/*
 * the following pointers may be changed as long as 'allow_customize' is set
 */
static int allow_customize = 1; // @global

static void *(*malloc_impl)(size_t, const char *, int) = CRYPTO_malloc;
static void *(*realloc_impl)(void *, size_t, const char *, int) = CRYPTO_realloc;
static void (* free_impl)(void *, const char *, int) = CRYPTO_free;

#ifndef OPENSSL_NO_CRYPTO_MDEBUG
	static int call_malloc_debug = 1;
#else
	static int call_malloc_debug = 0;
#endif

int CRYPTO_set_mem_functions(void *(*m)(size_t, const char *, int),
    void *(*r)(void *, size_t, const char *, int), void (* f)(void *, const char *, int))
{
	if(!allow_customize)
		return 0;
	if(m)
		malloc_impl = m;
	if(r)
		realloc_impl = r;
	if(f)
		free_impl = f;
	return 1;
}

int CRYPTO_set_mem_debug(int flag)
{
	if(!allow_customize)
		return 0;
	else {
		call_malloc_debug = flag;
		return 1;
	}
}

void CRYPTO_get_mem_functions(void *(**m)(size_t, const char *, int), void *(**r)(void *, size_t, const char *, int), void(**f) (void *, const char *, int))
{
	ASSIGN_PTR(m, malloc_impl);
	ASSIGN_PTR(r, realloc_impl);
	ASSIGN_PTR(f, free_impl);
}

void * CRYPTO_malloc(size_t num, const char * file, int line)
{
	void * ret = NULL;
	if(malloc_impl && malloc_impl != CRYPTO_malloc)
		ret = malloc_impl(num, file, line);
	else if(num > 0) {
		allow_customize = 0;
#ifndef OPENSSL_NO_CRYPTO_MDEBUG
		if(call_malloc_debug) {
			CRYPTO_mem_debug_malloc(NULL, num, 0, file, line);
			ret = malloc(num);
			CRYPTO_mem_debug_malloc(ret, num, 1, file, line);
		}
		else {
			ret = malloc(num);
		}
#else
		osslargused(file); 
		osslargused(line);
		ret = malloc(num);
#endif
	}
	return ret;
}

void * FASTCALL CRYPTO_malloc_lite(size_t num)
{
	void * ret = NULL;
	if(malloc_impl && malloc_impl != CRYPTO_malloc)
		ret = malloc_impl(num, 0, 0);
	else if(num > 0) {
		allow_customize = 0;
#ifndef OPENSSL_NO_CRYPTO_MDEBUG
		if(call_malloc_debug) {
			CRYPTO_mem_debug_malloc(NULL, num, 0, 0, 0);
			ret = malloc(num);
			CRYPTO_mem_debug_malloc(ret, num, 1, 0, 0);
		}
		else {
			ret = malloc(num);
		}
#else
		//osslargused(file); 
		//osslargused(line);
		ret = malloc(num);
#endif
	}
	return ret;
}

void * CRYPTO_zalloc(size_t num, const char * file, int line)
{
	void * ret = CRYPTO_malloc(num, file, line);
	memzero(ret, num);
	return ret;
}

void * FASTCALL CRYPTO_zalloc_lite(size_t num)
{
	void * ret = CRYPTO_malloc_lite(num);
	memzero(ret, num);
	return ret;
}

void * CRYPTO_realloc(void * str, size_t num, const char * file, int line)
{
	if(realloc_impl != NULL && realloc_impl != &CRYPTO_realloc)
		return realloc_impl(str, num, file, line);
	if(!str)
		return CRYPTO_malloc(num, file, line);
	if(num == 0) {
		CRYPTO_free(str, file, line);
		return NULL;
	}
	allow_customize = 0;
#ifndef OPENSSL_NO_CRYPTO_MDEBUG
	if(call_malloc_debug) {
		void * ret;
		CRYPTO_mem_debug_realloc(str, NULL, num, 0, file, line);
		ret = realloc(str, num);
		CRYPTO_mem_debug_realloc(str, ret, num, 1, file, line);
		return ret;
	}
#else
	osslargused(file); osslargused(line);
#endif
	return realloc(str, num);
}

void * CRYPTO_clear_realloc(void * str, size_t old_len, size_t num, const char * file, int line)
{
	void * ret = NULL;
	if(!str)
		return CRYPTO_malloc(num, file, line);
	if(num == 0) {
		CRYPTO_clear_free(str, old_len, file, line);
		return NULL;
	}
	/* Can't shrink the buffer since memcpy below copies |old_len| bytes. */
	if(num < old_len) {
		OPENSSL_cleanse((char *)str + num, old_len - num);
		return str;
	}
	ret = CRYPTO_malloc(num, file, line);
	if(ret) {
		memcpy(ret, str, old_len);
		CRYPTO_clear_free(str, old_len, file, line);
	}
	return ret;
}

void CRYPTO_free(void * str, const char * file, int line)
{
	if(free_impl && free_impl != &CRYPTO_free) {
		free_impl(str, file, line);
	}
	else {
#ifndef OPENSSL_NO_CRYPTO_MDEBUG
		if(call_malloc_debug) {
			CRYPTO_mem_debug_free(str, 0, file, line);
			free(str);
			CRYPTO_mem_debug_free(str, 1, file, line);
		}
		else {
			free(str);
		}
#else
		free(str);
#endif
	}
}

void FASTCALL CRYPTO_free_lite(void * str)
{
	if(free_impl && free_impl != &CRYPTO_free) {
		free_impl(str, 0, 0);
	}
	else {
#ifndef OPENSSL_NO_CRYPTO_MDEBUG
		if(call_malloc_debug) {
			CRYPTO_mem_debug_free(str, 0, 0, 0);
			free(str);
			CRYPTO_mem_debug_free(str, 1, 0, 0);
		}
		else {
			free(str);
		}
#else
		free(str);
#endif
	}
}

void FASTCALL CRYPTO_clear_free(void * str, size_t num, const char * file, int line)
{
	if(str) {
		if(num)
			OPENSSL_cleanse(str, num);
		CRYPTO_free(str, file, line);
	}
}
