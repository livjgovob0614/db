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
//#include <openssl/pkcs12.h>
#ifdef OPENSSL_FIPS
	#include <openssl/fips.h>
#endif
//#include <openssl/ts.h>
//#include <openssl/ct.h>
//#include <openssl/kdf.h>

int err_load_crypto_strings_int(void)
{
	if(
#ifdef OPENSSL_FIPS
	    FIPS_set_error_callbacks(ERR_put_error, ERR_add_error_vdata) == 0 ||
#endif
#ifndef OPENSSL_NO_ERR
	    ERR_load_ERR_strings() == 0 || /* include error strings for SYSerr */ ERR_load_BN_strings() == 0 ||
# ifndef OPENSSL_NO_RSA
	    ERR_load_RSA_strings() == 0 ||
# endif
# ifndef OPENSSL_NO_DH
	    ERR_load_DH_strings() == 0 ||
# endif
	    ERR_load_EVP_strings() == 0 || ERR_load_BUF_strings() == 0 || ERR_load_OBJ_strings() == 0 || ERR_load_PEM_strings() == 0 ||
# ifndef OPENSSL_NO_DSA
	    ERR_load_DSA_strings() == 0 ||
# endif
	    ERR_load_X509_strings() == 0 || ERR_load_ASN1_strings() == 0 || ERR_load_CONF_strings() == 0 || ERR_load_CRYPTO_strings() == 0 ||
# ifndef OPENSSL_NO_COMP
	    ERR_load_COMP_strings() == 0 ||
# endif
# ifndef OPENSSL_NO_EC
	    ERR_load_EC_strings() == 0 ||
# endif
	    /* skip ERR_load_SSL_strings() because it is not in this library */
	    ERR_load_BIO_strings() == 0 || ERR_load_PKCS7_strings() == 0 || ERR_load_X509V3_strings() == 0 || ERR_load_PKCS12_strings() == 0 ||
	    ERR_load_RAND_strings() == 0 || ERR_load_DSO_strings() == 0 ||
# ifndef OPENSSL_NO_TS
	    ERR_load_TS_strings() == 0 ||
# endif
# ifndef OPENSSL_NO_ENGINE
	    ERR_load_ENGINE_strings() == 0 ||
# endif
# ifndef OPENSSL_NO_OCSP
	    ERR_load_OCSP_strings() == 0 ||
# endif
#ifndef OPENSSL_NO_UI
	    ERR_load_UI_strings() == 0 ||
#endif
# ifdef OPENSSL_FIPS
	    ERR_load_FIPS_strings() == 0 ||
# endif
# ifndef OPENSSL_NO_CMS
	    ERR_load_CMS_strings() == 0 ||
# endif
# ifndef OPENSSL_NO_CT
	    ERR_load_CT_strings() == 0 ||
# endif
	    ERR_load_ASYNC_strings() == 0 ||
#endif
	    ERR_load_KDF_strings() == 0)
		return 0;

	return 1;
}

