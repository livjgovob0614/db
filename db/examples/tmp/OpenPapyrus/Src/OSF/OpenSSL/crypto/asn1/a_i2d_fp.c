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

#ifndef NO_OLD_ASN1

# ifndef OPENSSL_NO_STDIO
int ASN1_i2d_fp(i2d_of_void * i2d, FILE * out, void * x)
{
	BIO * b;
	int ret;
	if((b = BIO_new(BIO_s_file())) == NULL) {
		ASN1err(ASN1_F_ASN1_I2D_FP, ERR_R_BUF_LIB);
		return 0;
	}
	BIO_set_fp(b, out, BIO_NOCLOSE);
	ret = ASN1_i2d_bio(i2d, b, (uchar *)x);
	BIO_free(b);
	return ret;
}

# endif

int ASN1_i2d_bio(i2d_of_void * i2d, BIO * out, uchar * x)
{
	char * b;
	uchar * p;
	int i, j = 0, n, ret = 1;
	n = i2d(x, 0);
	b = (char *)OPENSSL_malloc(n);
	if(!b) {
		ASN1err(ASN1_F_ASN1_I2D_BIO, ERR_R_MALLOC_FAILURE);
		return 0;
	}

	p = (uchar *)b;
	i2d(x, &p);

	for(;; ) {
		i = BIO_write(out, &(b[j]), n);
		if(i == n)
			break;
		if(i <= 0) {
			ret = 0;
			break;
		}
		j += i;
		n -= i;
	}
	OPENSSL_free(b);
	return ret;
}

#endif

#ifndef OPENSSL_NO_STDIO
int ASN1_item_i2d_fp(const ASN1_ITEM * it, FILE * out, void * x)
{
	BIO * b;
	int ret;
	if((b = BIO_new(BIO_s_file())) == NULL) {
		ASN1err(ASN1_F_ASN1_ITEM_I2D_FP, ERR_R_BUF_LIB);
		return 0;
	}
	BIO_set_fp(b, out, BIO_NOCLOSE);
	ret = ASN1_item_i2d_bio(it, b, x);
	BIO_free(b);
	return ret;
}

#endif

int ASN1_item_i2d_bio(const ASN1_ITEM * it, BIO * out, void * x)
{
	uchar * b = NULL;
	int i, j = 0, n, ret = 1;

	n = ASN1_item_i2d((ASN1_VALUE*)x, &b, it);
	if(!b) {
		ASN1err(ASN1_F_ASN1_ITEM_I2D_BIO, ERR_R_MALLOC_FAILURE);
		return 0;
	}

	for(;; ) {
		i = BIO_write(out, &(b[j]), n);
		if(i == n)
			break;
		if(i <= 0) {
			ret = 0;
			break;
		}
		j += i;
		n -= i;
	}
	OPENSSL_free(b);
	return ret;
}

