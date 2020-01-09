/*
 * Copyright 2006-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#include "internal/cryptlib.h"
#pragma hdrstop
#include "dsa_locl.h"

/* DSA pkey context structure */

typedef struct {
	/* Parameter gen parameters */
	int nbits;              /* size of p in bits (default: 1024) */
	int qbits;              /* size of q in bits (default: 160) */
	const EVP_MD * pmd;     /* MD for parameter generation */
	/* Keygen callback info */
	int gentmp[2];
	/* message digest */
	const EVP_MD * md;      /* MD for the signature */
} DSA_PKEY_CTX;

static int pkey_dsa_init(EVP_PKEY_CTX * ctx)
{
	DSA_PKEY_CTX * dctx = (DSA_PKEY_CTX*)OPENSSL_malloc(sizeof(*dctx));
	if(dctx == NULL)
		return 0;
	dctx->nbits = 1024;
	dctx->qbits = 160;
	dctx->pmd = NULL;
	dctx->md = NULL;
	ctx->data = dctx;
	ctx->keygen_info = dctx->gentmp;
	ctx->keygen_info_count = 2;
	return 1;
}

static int pkey_dsa_copy(EVP_PKEY_CTX * dst, EVP_PKEY_CTX * src)
{
	DSA_PKEY_CTX * dctx, * sctx;
	if(!pkey_dsa_init(dst))
		return 0;
	sctx = (DSA_PKEY_CTX*)src->data;
	dctx = (DSA_PKEY_CTX*)dst->data;
	dctx->nbits = sctx->nbits;
	dctx->qbits = sctx->qbits;
	dctx->pmd = sctx->pmd;
	dctx->md = sctx->md;
	return 1;
}

static void pkey_dsa_cleanup(EVP_PKEY_CTX * ctx)
{
	DSA_PKEY_CTX * dctx = static_cast<DSA_PKEY_CTX *>(ctx->data);
	OPENSSL_free(dctx);
}

static int pkey_dsa_sign(EVP_PKEY_CTX * ctx, uchar * sig, size_t * siglen, const uchar * tbs, size_t tbslen)
{
	int ret;
	uint sltmp;
	DSA_PKEY_CTX * dctx = static_cast<DSA_PKEY_CTX *>(ctx->data);
	DSA * dsa = ctx->pkey->pkey.dsa;
	if(dctx->md) {
		if(tbslen != (size_t)EVP_MD_size(dctx->md))
			return 0;
	}
	else {
		if(tbslen != SHA_DIGEST_LENGTH)
			return 0;
	}

	ret = DSA_sign(0, tbs, tbslen, sig, &sltmp, dsa);

	if(ret <= 0)
		return ret;
	*siglen = sltmp;
	return 1;
}

static int pkey_dsa_verify(EVP_PKEY_CTX * ctx, const uchar * sig, size_t siglen, const uchar * tbs, size_t tbslen)
{
	int ret;
	DSA_PKEY_CTX * dctx = static_cast<DSA_PKEY_CTX *>(ctx->data);
	DSA * dsa = ctx->pkey->pkey.dsa;
	if(dctx->md) {
		if(tbslen != (size_t)EVP_MD_size(dctx->md))
			return 0;
	}
	else {
		if(tbslen != SHA_DIGEST_LENGTH)
			return 0;
	}
	ret = DSA_verify(0, tbs, tbslen, sig, siglen, dsa);
	return ret;
}

static int pkey_dsa_ctrl(EVP_PKEY_CTX * ctx, int type, int p1, void * p2)
{
	DSA_PKEY_CTX * dctx = static_cast<DSA_PKEY_CTX *>(ctx->data);
	switch(type) {
		case EVP_PKEY_CTRL_DSA_PARAMGEN_BITS:
		    if(p1 < 256)
			    return -2;
		    dctx->nbits = p1;
		    return 1;
		case EVP_PKEY_CTRL_DSA_PARAMGEN_Q_BITS:
		    if(p1 != 160 && p1 != 224 && p1 && p1 != 256)
			    return -2;
		    dctx->qbits = p1;
		    return 1;
		case EVP_PKEY_CTRL_DSA_PARAMGEN_MD:
			{
				const int evp_md_type = EVP_MD_type((const EVP_MD*)p2);
				if(!oneof3(evp_md_type, NID_sha1, NID_sha224, NID_sha256)) {
					DSAerr(DSA_F_PKEY_DSA_CTRL, DSA_R_INVALID_DIGEST_TYPE);
					return 0;
				}
				else {
					dctx->pmd = (EVP_MD*)p2;
					return 1;
				}
			}
		case EVP_PKEY_CTRL_MD:
			{
				const int evp_md_type = EVP_MD_type((const EVP_MD*)p2);
				if(!oneof7(evp_md_type, NID_sha1, NID_dsa, NID_dsaWithSHA, NID_sha224, NID_sha256, NID_sha384, NID_sha512)) {
					DSAerr(DSA_F_PKEY_DSA_CTRL, DSA_R_INVALID_DIGEST_TYPE);
					return 0;
				}
				else {
					dctx->md = (EVP_MD*)p2;
					return 1;
				}
			}
		case EVP_PKEY_CTRL_GET_MD:
		    *(const EVP_MD**)p2 = dctx->md;
		    return 1;

		case EVP_PKEY_CTRL_DIGESTINIT:
		case EVP_PKEY_CTRL_PKCS7_SIGN:
		case EVP_PKEY_CTRL_CMS_SIGN:
		    return 1;

		case EVP_PKEY_CTRL_PEER_KEY:
		    DSAerr(DSA_F_PKEY_DSA_CTRL,
		    EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
		    return -2;
		default:
		    return -2;
	}
}

static int pkey_dsa_ctrl_str(EVP_PKEY_CTX * ctx, const char * type, const char * value)
{
	if(sstreq(type, "dsa_paramgen_bits")) {
		int nbits = atoi(value);
		return EVP_PKEY_CTX_set_dsa_paramgen_bits(ctx, nbits);
	}
	else if(sstreq(type, "dsa_paramgen_q_bits")) {
		int qbits = atoi(value);
		return EVP_PKEY_CTX_ctrl(ctx, EVP_PKEY_DSA, EVP_PKEY_OP_PARAMGEN, EVP_PKEY_CTRL_DSA_PARAMGEN_Q_BITS, qbits, NULL);
	}
	else if(sstreq(type, "dsa_paramgen_md")) {
		return EVP_PKEY_CTX_ctrl(ctx, EVP_PKEY_DSA, EVP_PKEY_OP_PARAMGEN, EVP_PKEY_CTRL_DSA_PARAMGEN_MD, 0, (void *)EVP_get_digestbyname(value));
	}
	else
		return -2;
}

static int pkey_dsa_paramgen(EVP_PKEY_CTX * ctx, EVP_PKEY * pkey)
{
	DSA * dsa = NULL;
	DSA_PKEY_CTX * dctx = static_cast<DSA_PKEY_CTX *>(ctx->data);
	BN_GENCB * pcb;
	int ret;
	if(ctx->pkey_gencb) {
		pcb = BN_GENCB_new();
		if(pcb == NULL)
			return 0;
		evp_pkey_set_cb_translate(pcb, ctx);
	}
	else
		pcb = NULL;
	dsa = DSA_new();
	if(dsa == NULL) {
		BN_GENCB_free(pcb);
		return 0;
	}
	ret = dsa_builtin_paramgen(dsa, dctx->nbits, dctx->qbits, dctx->pmd,
	    NULL, 0, NULL, NULL, NULL, pcb);
	BN_GENCB_free(pcb);
	if(ret)
		EVP_PKEY_assign_DSA(pkey, dsa);
	else
		DSA_free(dsa);
	return ret;
}

static int pkey_dsa_keygen(EVP_PKEY_CTX * ctx, EVP_PKEY * pkey)
{
	DSA * dsa = NULL;
	if(ctx->pkey == NULL) {
		DSAerr(DSA_F_PKEY_DSA_KEYGEN, DSA_R_NO_PARAMETERS_SET);
		return 0;
	}
	dsa = DSA_new();
	if(dsa == NULL)
		return 0;
	EVP_PKEY_assign_DSA(pkey, dsa);
	/* Note: if error return, pkey is freed by parent routine */
	if(!EVP_PKEY_copy_parameters(pkey, ctx->pkey))
		return 0;
	return DSA_generate_key(pkey->pkey.dsa);
}

const EVP_PKEY_METHOD dsa_pkey_meth = {
	EVP_PKEY_DSA,
	EVP_PKEY_FLAG_AUTOARGLEN,
	pkey_dsa_init,
	pkey_dsa_copy,
	pkey_dsa_cleanup,

	0,
	pkey_dsa_paramgen,

	0,
	pkey_dsa_keygen,

	0,
	pkey_dsa_sign,

	0,
	pkey_dsa_verify,

	0, 0,

	0, 0, 0, 0,

	0, 0,

	0, 0,

	0, 0,

	pkey_dsa_ctrl,
	pkey_dsa_ctrl_str
};
