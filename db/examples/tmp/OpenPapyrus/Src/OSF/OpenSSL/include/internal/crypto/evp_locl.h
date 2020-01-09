/*
 * Copyright 2000-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
//
// EVP_MD_CTX related stuff 
//
struct evp_md_ctx_st {
    const EVP_MD *digest;
    ENGINE *engine;             /* functional reference if 'digest' is ENGINE-provided */
    unsigned long flags;
    void *md_data;
    EVP_PKEY_CTX *pctx; /* Public key context for sign/verify */
    int (*update) (EVP_MD_CTX *ctx, const void *data, size_t count); /* Update function: usually copied from EVP_MD */
};
//
// EVP_CIPHER_CTX
//
struct evp_cipher_ctx_st {
    const EVP_CIPHER *cipher;
    ENGINE *engine;     // functional reference if 'cipher' is ENGINE-provided 
    int    encrypt;     // encrypt or decrypt 
    int    buf_len;     // number we have left 
    uchar  oiv[EVP_MAX_IV_LENGTH];    // original iv 
    uchar  iv[EVP_MAX_IV_LENGTH];     // working iv 
    uchar  buf[EVP_MAX_BLOCK_LENGTH]; // saved partial block 
    int    num;         // used by cfb/ofb/ctr mode 
    // FIXME: Should this even exist? It appears unused 
    void * app_data;    // application stuff 
    int    key_len;     // May change for variable length cipher 
    ulong  flags;       // Various flags 
    void * cipher_data; // per EVP data 
    int    final_used;
    int    block_mask;
    uchar  Final[EVP_MAX_BLOCK_LENGTH]; // possible final block 
};

int PKCS5_v2_PBKDF2_keyivgen(EVP_CIPHER_CTX *ctx, const char *pass, int passlen, ASN1_TYPE *param, const EVP_CIPHER *c, const EVP_MD *md, int en_de);

struct evp_Encode_Ctx_st {
    /* number saved in a partial encode/decode */
    int    num;
    /*
 * The length is either the output line length (in input bytes) or the
 * shortest input line length that is ok.  Once decoding begins, the
 * length is adjusted up each time a longer line is decoded
     */
    int    length;
    uchar  enc_data[80]; /* data to encode */
    int    line_num; /* number read on current line */
    int    expect_nl;
};

typedef struct evp_pbe_st EVP_PBE_CTL;
DEFINE_STACK_OF(EVP_PBE_CTL)

int is_partially_overlapping(const void *ptr1, const void *ptr2, int len);
