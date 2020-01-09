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

int RSA_sign_ASN1_OCTET_STRING(int type, const uchar * m, uint m_len, uchar * sigret, uint * siglen, RSA * rsa)
{
	ASN1_OCTET_STRING sig;
	int i, j, ret = 1;
	uchar * p, * s;
	sig.type = V_ASN1_OCTET_STRING;
	sig.length = m_len;
	sig.data = (uchar *)m;
	i = i2d_ASN1_OCTET_STRING(&sig, 0);
	j = RSA_size(rsa);
	if(i > (j - RSA_PKCS1_PADDING_SIZE)) {
		RSAerr(RSA_F_RSA_SIGN_ASN1_OCTET_STRING, RSA_R_DIGEST_TOO_BIG_FOR_RSA_KEY);
		return 0;
	}
	s = (uchar *)OPENSSL_malloc((uint)j + 1);
	if(s == NULL) {
		RSAerr(RSA_F_RSA_SIGN_ASN1_OCTET_STRING, ERR_R_MALLOC_FAILURE);
		return 0;
	}
	p = s;
	i2d_ASN1_OCTET_STRING(&sig, &p);
	i = RSA_private_encrypt(i, s, sigret, rsa, RSA_PKCS1_PADDING);
	if(i <= 0)
		ret = 0;
	else
		*siglen = i;

	OPENSSL_clear_free(s, (uint)j + 1);
	return ret;
}

int RSA_verify_ASN1_OCTET_STRING(int dtype,
    const uchar * m,
    uint m_len, uchar * sigbuf,
    uint siglen, RSA * rsa)
{
	int i, ret = 0;
	uchar * s;
	const uchar * p;
	ASN1_OCTET_STRING * sig = NULL;

	if(siglen != (uint)RSA_size(rsa)) {
		RSAerr(RSA_F_RSA_VERIFY_ASN1_OCTET_STRING,
		    RSA_R_WRONG_SIGNATURE_LENGTH);
		return 0;
	}

	s = (uchar *)OPENSSL_malloc((uint)siglen);
	if(s == NULL) {
		RSAerr(RSA_F_RSA_VERIFY_ASN1_OCTET_STRING, ERR_R_MALLOC_FAILURE);
		goto err;
	}
	i = RSA_public_decrypt((int)siglen, sigbuf, s, rsa, RSA_PKCS1_PADDING);

	if(i <= 0)
		goto err;

	p = s;
	sig = d2i_ASN1_OCTET_STRING(NULL, &p, (long)i);
	if(sig == NULL)
		goto err;

	if(((uint)sig->length != m_len) ||
	    (memcmp(m, sig->data, m_len) != 0)) {
		RSAerr(RSA_F_RSA_VERIFY_ASN1_OCTET_STRING, RSA_R_BAD_SIGNATURE);
	}
	else
		ret = 1;
err:
	ASN1_OCTET_STRING_free(sig);
	OPENSSL_clear_free(s, (uint)siglen);
	return ret;
}

