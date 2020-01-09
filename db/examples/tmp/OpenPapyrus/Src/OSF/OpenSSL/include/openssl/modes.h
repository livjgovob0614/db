/*
 * Copyright 2008-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#include <stddef.h>

#ifdef  __cplusplus
extern "C" {
#endif
/*@funcdef*/typedef void (*block128_f)(const uchar in[16], uchar out[16], const void * key);
/*@funcdef*/typedef void (*cbc128_f)(const uchar * in, uchar * out, size_t len, const void * key, uchar ivec[16], int enc);
/*@funcdef*/typedef void (*ctr128_f)(const uchar * in, uchar * out, size_t blocks, const void * key, const uchar ivec[16]);
/*@funcdef*/typedef void (*ccm128_f)(const uchar * in, uchar * out, size_t blocks, const void * key, const uchar ivec[16], uchar cmac[16]);
void   CRYPTO_cbc128_encrypt(const uchar *in, uchar *out, size_t len, const void * key, uchar ivec[16], block128_f block);
void   CRYPTO_cbc128_decrypt(const uchar *in, uchar *out, size_t len, const void * key, uchar ivec[16], block128_f block);
void   CRYPTO_ctr128_encrypt(const uchar *in, uchar *out, size_t len, const void * key, uchar ivec[16], uchar ecount_buf[16], uint *num, block128_f block);
void   CRYPTO_ctr128_encrypt_ctr32(const uchar *in, uchar *out, size_t len, const void * key, uchar ivec[16], uchar ecount_buf[16], uint *num, ctr128_f ctr);
void   CRYPTO_ofb128_encrypt(const uchar *in, uchar *out, size_t len, const void * key, uchar ivec[16], int * num, block128_f block);
void   CRYPTO_cfb128_encrypt(const uchar *in, uchar *out, size_t len, const void * key, uchar ivec[16], int * num, int enc, block128_f block);
void   CRYPTO_cfb128_8_encrypt(const uchar *in, uchar *out, size_t length, const void * key, uchar ivec[16], int * num, int enc, block128_f block);
void   CRYPTO_cfb128_1_encrypt(const uchar *in, uchar *out, size_t bits, const void * key, uchar ivec[16], int * num, int enc, block128_f block);
size_t CRYPTO_cts128_encrypt_block(const uchar *in, uchar *out, size_t len, const void * key, uchar ivec[16], block128_f block);
size_t CRYPTO_cts128_encrypt(const uchar *in, uchar *out, size_t len, const void * key, uchar ivec[16], cbc128_f cbc);
size_t CRYPTO_cts128_decrypt_block(const uchar *in, uchar *out, size_t len, const void * key, uchar ivec[16], block128_f block);
size_t CRYPTO_cts128_decrypt(const uchar *in, uchar *out, size_t len, const void * key, uchar ivec[16], cbc128_f cbc);
size_t CRYPTO_nistcts128_encrypt_block(const uchar *in, uchar *out, size_t len, const void * key, uchar ivec[16], block128_f block);
size_t CRYPTO_nistcts128_encrypt(const uchar *in, uchar *out, size_t len, const void * key, uchar ivec[16], cbc128_f cbc);
size_t CRYPTO_nistcts128_decrypt_block(const uchar *in, uchar *out, size_t len, const void * key, uchar ivec[16], block128_f block);
size_t CRYPTO_nistcts128_decrypt(const uchar *in, uchar *out, size_t len, const void * key, uchar ivec[16], cbc128_f cbc);

typedef struct gcm128_context GCM128_CONTEXT;

GCM128_CONTEXT * CRYPTO_gcm128_new(void * key, block128_f block);
void CRYPTO_gcm128_init(GCM128_CONTEXT * ctx, void * key, block128_f block);
void CRYPTO_gcm128_setiv(GCM128_CONTEXT * ctx, const uchar * iv, size_t len);
int CRYPTO_gcm128_aad(GCM128_CONTEXT * ctx, const uchar * aad, size_t len);
int CRYPTO_gcm128_encrypt(GCM128_CONTEXT * ctx, const uchar * in, uchar * out, size_t len);
int CRYPTO_gcm128_decrypt(GCM128_CONTEXT * ctx, const uchar * in, uchar * out, size_t len);
int CRYPTO_gcm128_encrypt_ctr32(GCM128_CONTEXT * ctx, const uchar * in, uchar * out, size_t len, ctr128_f stream);
int CRYPTO_gcm128_decrypt_ctr32(GCM128_CONTEXT * ctx, const uchar * in, uchar * out, size_t len, ctr128_f stream);
int CRYPTO_gcm128_finish(GCM128_CONTEXT * ctx, const uchar * tag, size_t len);
void CRYPTO_gcm128_tag(GCM128_CONTEXT * ctx, uchar * tag, size_t len);
void CRYPTO_gcm128_release(GCM128_CONTEXT * ctx);

typedef struct ccm128_context CCM128_CONTEXT;

void CRYPTO_ccm128_init(CCM128_CONTEXT * ctx, uint M, uint L, void * key, block128_f block);
int CRYPTO_ccm128_setiv(CCM128_CONTEXT * ctx, const uchar * nonce, size_t nlen, size_t mlen);
void CRYPTO_ccm128_aad(CCM128_CONTEXT * ctx, const uchar * aad, size_t alen);
int CRYPTO_ccm128_encrypt(CCM128_CONTEXT * ctx, const uchar * inp, uchar * out, size_t len);
int CRYPTO_ccm128_decrypt(CCM128_CONTEXT * ctx, const uchar * inp, uchar * out, size_t len);
int CRYPTO_ccm128_encrypt_ccm64(CCM128_CONTEXT * ctx, const uchar * inp, uchar * out, size_t len, ccm128_f stream);
int CRYPTO_ccm128_decrypt_ccm64(CCM128_CONTEXT * ctx, const uchar * inp, uchar * out, size_t len, ccm128_f stream);
size_t CRYPTO_ccm128_tag(CCM128_CONTEXT * ctx, uchar * tag, size_t len);

typedef struct xts128_context XTS128_CONTEXT;

int CRYPTO_xts128_encrypt(const XTS128_CONTEXT * ctx, const uchar iv[16], const uchar * inp, uchar * out, size_t len, int enc);
size_t CRYPTO_128_wrap(void * key, const uchar * iv, uchar * out, const uchar * in, size_t inlen, block128_f block);
size_t CRYPTO_128_unwrap(void * key, const uchar * iv, uchar * out, const uchar * in, size_t inlen, block128_f block);
size_t CRYPTO_128_wrap_pad(void * key, const uchar * icv, uchar * out, const uchar * in, size_t inlen, block128_f block);
size_t CRYPTO_128_unwrap_pad(void * key, const uchar * icv, uchar * out, const uchar * in, size_t inlen, block128_f block);

#ifndef OPENSSL_NO_OCB
	typedef struct ocb128_context OCB128_CONTEXT;

	/*@funcdef*/typedef void (*ocb128_f)(const uchar * in, uchar * out, size_t blocks, const void * key, size_t start_block_num, uchar offset_i[16],
		const uchar L_[][16], uchar checksum[16]);

	OCB128_CONTEXT * CRYPTO_ocb128_new(void * keyenc, void * keydec, block128_f encrypt, block128_f decrypt, ocb128_f stream);
	int CRYPTO_ocb128_init(OCB128_CONTEXT * ctx, void * keyenc, void * keydec, block128_f encrypt, block128_f decrypt, ocb128_f stream);
	int CRYPTO_ocb128_copy_ctx(OCB128_CONTEXT * dest, OCB128_CONTEXT * src, void * keyenc, void * keydec);
	int CRYPTO_ocb128_setiv(OCB128_CONTEXT * ctx, const uchar * iv, size_t len, size_t taglen);
	int CRYPTO_ocb128_aad(OCB128_CONTEXT * ctx, const uchar * aad, size_t len);
	int CRYPTO_ocb128_encrypt(OCB128_CONTEXT * ctx, const uchar * in, uchar * out, size_t len);
	int CRYPTO_ocb128_decrypt(OCB128_CONTEXT * ctx, const uchar * in, uchar * out, size_t len);
	int CRYPTO_ocb128_finish(OCB128_CONTEXT * ctx, const uchar * tag, size_t len);
	int CRYPTO_ocb128_tag(OCB128_CONTEXT * ctx, uchar * tag, size_t len);
	void CRYPTO_ocb128_cleanup(OCB128_CONTEXT * ctx);
#endif /* OPENSSL_NO_OCB */

#ifdef  __cplusplus
}
#endif
