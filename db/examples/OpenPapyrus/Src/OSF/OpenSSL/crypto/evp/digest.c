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
//#include "evp_locl.h"

/* This call frees resources associated with the context */
int FASTCALL EVP_MD_CTX_reset(EVP_MD_CTX * ctx)
{
	if(!ctx)
		return 1;
	// 
	// Don't assume ctx->md_data was cleaned in EVP_Digest_Final, because
	// sometimes only copies of the context are ever finalised.
	// 
	if(ctx->digest && ctx->digest->cleanup && !EVP_MD_CTX_test_flags(ctx, EVP_MD_CTX_FLAG_CLEANED))
		ctx->digest->cleanup(ctx);
	if(ctx->digest && ctx->digest->ctx_size && ctx->md_data && !EVP_MD_CTX_test_flags(ctx, EVP_MD_CTX_FLAG_REUSE)) {
		OPENSSL_clear_free(ctx->md_data, ctx->digest->ctx_size);
	}
	EVP_PKEY_CTX_free(ctx->pctx);
#ifndef OPENSSL_NO_ENGINE
	ENGINE_finish(ctx->engine);
#endif
	OPENSSL_cleanse(ctx, sizeof(*ctx));
	return 1;
}

EVP_MD_CTX * EVP_MD_CTX_new(void)
{
	return (EVP_MD_CTX*)OPENSSL_zalloc(sizeof(EVP_MD_CTX));
}

void FASTCALL EVP_MD_CTX_free(EVP_MD_CTX * ctx)
{
	EVP_MD_CTX_reset(ctx);
	OPENSSL_free(ctx);
}

int FASTCALL EVP_DigestInit(EVP_MD_CTX * ctx, const EVP_MD * type)
{
	EVP_MD_CTX_reset(ctx);
	return EVP_DigestInit_ex(ctx, type, 0);
}

int FASTCALL EVP_DigestInit_ex(EVP_MD_CTX * ctx, const EVP_MD * type, ENGINE * impl)
{
	EVP_MD_CTX_clear_flags(ctx, EVP_MD_CTX_FLAG_CLEANED);
#ifndef OPENSSL_NO_ENGINE
	/*
	 * Whether it's nice or not, "Inits" can be used on "Final"'d contexts so
	 * this context may already have an ENGINE! Try to avoid releasing the
	 * previous handle, re-querying for an ENGINE, and having a
	 * reinitialisation, when it may all be unnecessary.
	 */
	if(ctx->engine && ctx->digest && (type == NULL || (type->type == ctx->digest->type)))
		goto skip_to_init;
	if(type) {
		/*
		 * Ensure an ENGINE left lying around from last time is cleared (the
		 * previous check attempted to avoid this if the same ENGINE and
		 * EVP_MD could be used).
		 */
		ENGINE_finish(ctx->engine);
		if(impl != NULL) {
			if(!ENGINE_init(impl)) {
				EVPerr(EVP_F_EVP_DIGESTINIT_EX, EVP_R_INITIALIZATION_ERROR);
				return 0;
			}
		}
		else {
			/* Ask if an ENGINE is reserved for this job */
			impl = ENGINE_get_digest_engine(type->type);
		}
		if(impl != NULL) {
			/* There's an ENGINE for this job ... (apparently) */
			const EVP_MD * d = ENGINE_get_digest(impl, type->type);
			if(d == NULL) {
				EVPerr(EVP_F_EVP_DIGESTINIT_EX, EVP_R_INITIALIZATION_ERROR);
				ENGINE_finish(impl);
				return 0;
			}
			/* We'll use the ENGINE's private digest definition */
			type = d;
			/*
			 * Store the ENGINE functional reference so we know 'type' came
			 * from an ENGINE and we need to release it when done.
			 */
			ctx->engine = impl;
		}
		else
			ctx->engine = NULL;
	}
	else {
		if(!ctx->digest) {
			EVPerr(EVP_F_EVP_DIGESTINIT_EX, EVP_R_NO_DIGEST_SET);
			return 0;
		}
		type = ctx->digest;
	}
#endif
	if(ctx->digest != type) {
		if(ctx->digest && ctx->digest->ctx_size) {
			OPENSSL_clear_free(ctx->md_data, ctx->digest->ctx_size);
			ctx->md_data = NULL;
		}
		ctx->digest = type;
		if(!(ctx->flags & EVP_MD_CTX_FLAG_NO_INIT) && type->ctx_size) {
			ctx->update = type->update;
			ctx->md_data = OPENSSL_zalloc(type->ctx_size);
			if(ctx->md_data == NULL) {
				EVPerr(EVP_F_EVP_DIGESTINIT_EX, ERR_R_MALLOC_FAILURE);
				return 0;
			}
		}
	}
#ifndef OPENSSL_NO_ENGINE
skip_to_init:
#endif
	if(ctx->pctx) {
		int r;
		r = EVP_PKEY_CTX_ctrl(ctx->pctx, -1, EVP_PKEY_OP_TYPE_SIG, EVP_PKEY_CTRL_DIGESTINIT, 0, ctx);
		if(r <= 0 && (r != -2))
			return 0;
	}
	if(ctx->flags & EVP_MD_CTX_FLAG_NO_INIT)
		return 1;
	return ctx->digest->init(ctx);
}

int FASTCALL EVP_DigestUpdate(EVP_MD_CTX * ctx, const void * data, size_t count)
{
	return ctx->update(ctx, data, count);
}

/* The caller can assume that this removes any secret data from the context */
int FASTCALL EVP_DigestFinal(EVP_MD_CTX * ctx, uchar * md, uint * size)
{
	int ret = EVP_DigestFinal_ex(ctx, md, size);
	EVP_MD_CTX_reset(ctx);
	return ret;
}

/* The caller can assume that this removes any secret data from the context */
int FASTCALL EVP_DigestFinal_ex(EVP_MD_CTX * ctx, uchar * md, uint * size)
{
	int ret;
	OPENSSL_assert(ctx->digest->md_size <= EVP_MAX_MD_SIZE);
	ret = ctx->digest->final(ctx, md);
	ASSIGN_PTR(size, ctx->digest->md_size);
	if(ctx->digest->cleanup) {
		ctx->digest->cleanup(ctx);
		EVP_MD_CTX_set_flags(ctx, EVP_MD_CTX_FLAG_CLEANED);
	}
	OPENSSL_cleanse(ctx->md_data, ctx->digest->ctx_size);
	return ret;
}

int FASTCALL EVP_MD_CTX_copy(EVP_MD_CTX * out, const EVP_MD_CTX * in)
{
	EVP_MD_CTX_reset(out);
	return EVP_MD_CTX_copy_ex(out, in);
}

int EVP_MD_CTX_copy_ex(EVP_MD_CTX * out, const EVP_MD_CTX * in)
{
	uchar * tmp_buf;
	if(!in || (in->digest == NULL)) {
		EVPerr(EVP_F_EVP_MD_CTX_COPY_EX, EVP_R_INPUT_NOT_INITIALIZED);
		return 0;
	}
#ifndef OPENSSL_NO_ENGINE
	/* Make sure it's safe to copy a digest context using an ENGINE */
	if(in->engine && !ENGINE_init(in->engine)) {
		EVPerr(EVP_F_EVP_MD_CTX_COPY_EX, ERR_R_ENGINE_LIB);
		return 0;
	}
#endif
	if(out->digest == in->digest) {
		tmp_buf = reinterpret_cast<uchar *>(out->md_data);
		EVP_MD_CTX_set_flags(out, EVP_MD_CTX_FLAG_REUSE);
	}
	else
		tmp_buf = NULL;
	EVP_MD_CTX_reset(out);
	memcpy(out, in, sizeof(*out));
	/* Null these variables, since they are getting fixed up
	 * properly below.  Anything else may cause a memleak and/or
	 * double free if any of the memory allocations below fail
	 */
	out->md_data = NULL;
	out->pctx = NULL;
	if(in->md_data && out->digest->ctx_size) {
		if(tmp_buf)
			out->md_data = tmp_buf;
		else {
			out->md_data = OPENSSL_malloc(out->digest->ctx_size);
			if(out->md_data == NULL) {
				EVPerr(EVP_F_EVP_MD_CTX_COPY_EX, ERR_R_MALLOC_FAILURE);
				return 0;
			}
		}
		memcpy(out->md_data, in->md_data, out->digest->ctx_size);
	}
	out->update = in->update;
	if(in->pctx) {
		out->pctx = EVP_PKEY_CTX_dup(in->pctx);
		if(!out->pctx) {
			EVP_MD_CTX_reset(out);
			return 0;
		}
	}
	return (out->digest->copy) ? out->digest->copy(out, in) : 1;
}

int FASTCALL EVP_Digest(const void * data, size_t count, uchar * md, uint * size, const EVP_MD * type, ENGINE * impl)
{
	EVP_MD_CTX * ctx = EVP_MD_CTX_new();
	int ret;
	if(!ctx)
		return 0;
	EVP_MD_CTX_set_flags(ctx, EVP_MD_CTX_FLAG_ONESHOT);
	ret = EVP_DigestInit_ex(ctx, type, impl) && EVP_DigestUpdate(ctx, data, count) && EVP_DigestFinal_ex(ctx, md, size);
	EVP_MD_CTX_free(ctx);
	return ret;
}

int EVP_MD_CTX_ctrl(EVP_MD_CTX * ctx, int cmd, int p1, void * p2)
{
	if(ctx->digest && ctx->digest->md_ctrl) {
		int ret = ctx->digest->md_ctrl(ctx, cmd, p1, p2);
		if(ret <= 0)
			return 0;
		return 1;
	}
	return 0;
}

