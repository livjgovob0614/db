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
#ifdef CHARSET_EBCDIC
	#include <openssl/ebcdic.h>
#endif

uchar * MD4(const uchar * d, size_t n, uchar * md)
{
	MD4_CTX c;
	static uchar m[MD4_DIGEST_LENGTH];
	SETIFZ(md, m);
	if(!MD4_Init(&c))
		return NULL;
#ifndef CHARSET_EBCDIC
	MD4_Update(&c, d, n);
#else
	{
		char temp[1024];
		ulong chunk;
		while(n > 0) {
			chunk = (n > sizeof(temp)) ? sizeof(temp) : n;
			ebcdic2ascii(temp, d, chunk);
			MD4_Update(&c, temp, chunk);
			n -= chunk;
			d += chunk;
		}
	}
#endif
	MD4_Final(md, &c);
	OPENSSL_cleanse(&c, sizeof(c)); /* security consideration */
	return (md);
}

