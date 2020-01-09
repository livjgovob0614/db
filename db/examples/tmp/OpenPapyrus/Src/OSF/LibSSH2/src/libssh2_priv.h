/* Copyright (c) 2004-2008, 2010, Sara Golemon <sarag@libssh2.org>
 * Copyright (c) 2009-2014 by Daniel Stenberg
 * Copyright (c) 2010 Simon Josefsson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided
 * that the following conditions are met:
 *
 * Redistributions of source code must retain the above
 * copyright notice, this list of conditions and the
 * following disclaimer.
 *
 * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * Neither the name of the copyright holder nor the names
 * of any other contributors may be used to endorse or
 * promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

#ifndef LIBSSH2_PRIV_H
#define LIBSSH2_PRIV_H 1

#define LIBSSH2_LIBRARY
#include <slib.h> // @sobolev
#include "libssh2_config.h"
#ifdef HAVE_WINDOWS_H
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
	#undef WIN32_LEAN_AND_MEAN
#endif
#ifdef HAVE_WS2TCPIP_H
	#include <ws2tcpip.h>
#endif

/* The following CPP block should really only be in session.c and
   packet.c.  However, AIX have #define's for 'events' and 'revents'
   and we are using those names in libssh2.h, so we need to include
   the AIX headers first, to make sure all code is compiled with
   consistent names of these fields.  While arguable the best would to
   change libssh2.h to use other names, that would break backwards
   compatibility.  For more information, see:
   https://www.mail-archive.com/libssh2-devel%40lists.sourceforge.net/msg00003.html
   https://www.mail-archive.com/libssh2-devel%40lists.sourceforge.net/msg00224.html
 */
#ifdef HAVE_POLL
	#include <sys/poll.h>
#else
	#if defined(HAVE_SELECT) && !defined(WIN32)
		#ifdef HAVE_SYS_SELECT_H
			#include <sys/select.h>
		#else
			#include <sys/time.h>
			#include <sys/types.h>
		#endif
	#endif
#endif
#ifdef HAVE_SYS_UIO_H
	#include <sys/uio.h> // Needed for struct iovec on some platforms
#endif
#ifdef HAVE_SYS_SOCKET_H
	#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
	#include <sys/ioctl.h>
#endif
#ifdef HAVE_INTTYPES_H
	#include <inttypes.h>
#endif
#include "libssh2.h"
//
//#include "libssh2_publickey.h"
//
typedef struct _LIBSSH2_PUBLICKEY LIBSSH2_PUBLICKEY;

typedef struct _libssh2_publickey_attribute {
    const char * name;
    ulong name_len;
    const char *value;
    ulong value_len;
    char mandatory;
} libssh2_publickey_attribute;

typedef struct _libssh2_publickey_list {
    uchar * packet; /* For freeing */
    const uchar *name;
    ulong name_len;
    const uchar *blob;
    ulong blob_len;
    ulong num_attrs;
    libssh2_publickey_attribute * attrs; /* free me */
} libssh2_publickey_list;
// 
// Generally use the first macro here, but if both name and value are string
// literals, you can use _fast() to take advantage of preprocessing 
// 
#define libssh2_publickey_attribute(name, value, mandatory)      { (name), strlen(name), (value), strlen(value), (mandatory) },
#define libssh2_publickey_attribute_fast(name, value, mandatory) { (name), sizeof(name) - 1, (value), sizeof(value) - 1, (mandatory) },

#ifdef __cplusplus
extern "C" {
#endif

/* Publickey Subsystem */
LIBSSH2_API LIBSSH2_PUBLICKEY *libssh2_publickey_init(LIBSSH2_SESSION *session);
LIBSSH2_API int libssh2_publickey_add_ex(LIBSSH2_PUBLICKEY *pkey, const uchar *name, ulong name_len, const uchar *blob, ulong blob_len, char overwrite, ulong num_attrs, const libssh2_publickey_attribute attrs[]);
#define libssh2_publickey_add(pkey, name, blob, blob_len, overwrite, num_attrs, attrs) \
  libssh2_publickey_add_ex((pkey), (name), strlen(name), (blob), (blob_len), (overwrite), (num_attrs), (attrs))

LIBSSH2_API int libssh2_publickey_remove_ex(LIBSSH2_PUBLICKEY *pkey, const uchar *name, ulong name_len, const uchar *blob, ulong blob_len);
#define libssh2_publickey_remove(pkey, name, blob, blob_len) libssh2_publickey_remove_ex((pkey), (name), strlen(name), (blob), (blob_len))
LIBSSH2_API int libssh2_publickey_list_fetch(LIBSSH2_PUBLICKEY *pkey, ulong *num_keys, libssh2_publickey_list **pkey_list);
LIBSSH2_API void libssh2_publickey_list_free(LIBSSH2_PUBLICKEY *pkey, libssh2_publickey_list *pkey_list);
LIBSSH2_API int libssh2_publickey_shutdown(LIBSSH2_PUBLICKEY *pkey);

#ifdef __cplusplus
} /* extern "C" */
#endif
//
#include "libssh2_sftp.h"
//
//#include "misc.h" /* for the linked list stuff */
//
struct list_head {
    struct list_node *last;
    struct list_node *first;
};

struct list_node {
    struct list_node *next;
    struct list_node *prev;
    struct list_head *head;
};

int FASTCALL _libssh2_error_flags(LIBSSH2_SESSION* session, int errcode, const char* errmsg, int errflags);
int FASTCALL _libssh2_error(LIBSSH2_SESSION* session, int errcode, const char* errmsg);
void FASTCALL _libssh2_list_init(struct list_head *head);
/* add a node last in the list */
void FASTCALL _libssh2_list_add(struct list_head *head, struct list_node *entry);
/* return the "first" node in the list this head points to */
void * FASTCALL _libssh2_list_first(struct list_head *head);
/* return the next node in the list */
void * FASTCALL _libssh2_list_next(struct list_node *node);
/* return the prev node in the list */
void * FASTCALL _libssh2_list_prev(struct list_node *node);
/* remove this node from the list */
void _libssh2_list_remove(struct list_node *entry);
size_t _libssh2_base64_encode(struct _LIBSSH2_SESSION *session, const char *inp, size_t insize, char **outptr);
uint FASTCALL _libssh2_ntohu32(const uchar *buf);
uint64 FASTCALL _libssh2_ntohu64(const uchar *buf);
void FASTCALL _libssh2_htonu32(uchar *buf, uint32 val);
void FASTCALL _libssh2_store_u32(uchar **buf, uint32 value);
void FASTCALL _libssh2_store_str(uchar **buf, const char *str, size_t len);
void *_libssh2_calloc(LIBSSH2_SESSION* session, size_t size);

#if defined(LIBSSH2_WIN32) && !defined(__MINGW32__) && !defined(__CYGWIN__)
	/* provide a private one */
	#undef HAVE_GETTIMEOFDAY
	int __cdecl _libssh2_gettimeofday(struct timeval *tp, void *tzp);
	#define HAVE_LIBSSH2_GETTIMEOFDAY
	#define LIBSSH2_GETTIMEOFDAY_WIN32 /* enable the win32 implementation */
#else
	#ifdef HAVE_GETTIMEOFDAY
		#define _libssh2_gettimeofday(x,y) gettimeofday(x,y)
		#define HAVE_LIBSSH2_GETTIMEOFDAY
	#endif
#endif
//
//#include "comp.h"
/*#ifndef FALSE
	#define FALSE 0
#endif
#ifndef TRUE
	#define TRUE 1
#endif*/
#ifdef _MSC_VER
	#define inline __inline // "inline" keyword is valid only with C++ engine!
#endif
//
// Provide iovec / writev on WIN32 platform.
//
#ifdef WIN32
	struct iovec {
		size_t iov_len;
		void * iov_base;
	};

	static inline int writev(int sock, struct iovec * iov, int nvecs)
	{
		DWORD ret;
		return (WSASend(sock, (LPWSABUF)iov, nvecs, &ret, 0, NULL, NULL) == 0) ? ret : -1;
	}
#endif /* WIN32 */

#ifdef __OS400__
	/* Force parameter type. */
	#define send(s, b, l, f)    send((s), (uchar *)(b), (l), (f))
#endif
//
//#include "crypto.h"
//
#ifdef LIBSSH2_OPENSSL
	//
	//#include "openssl.h"
	//
	#include <openssl/opensslconf.h>
	#include <openssl/sha.h>
	#include <openssl/rsa.h>
	#include <openssl/engine.h>
	#ifndef OPENSSL_NO_DSA
		#include <openssl/dsa.h>
	#endif
	#ifndef OPENSSL_NO_MD5
		#include <openssl/md5.h>
	#endif
	#include <openssl/evp.h>
	#include <openssl/hmac.h>
	#include <openssl/bn.h>
	#include <openssl/pem.h>
	#include <openssl/rand.h>

	#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
		#define HAVE_OPAQUE_STRUCTS 1
	#endif
	#ifdef OPENSSL_NO_RSA
		#define LIBSSH2_RSA 0
	#else
		#define LIBSSH2_RSA 1
	#endif
	#ifdef OPENSSL_NO_DSA
		#define LIBSSH2_DSA 0
	#else
		#define LIBSSH2_DSA 1
	#endif
	#ifdef OPENSSL_NO_MD5
		#define LIBSSH2_MD5 0
	#else
		#define LIBSSH2_MD5 1
	#endif
	#ifdef OPENSSL_NO_RIPEMD
		#define LIBSSH2_HMAC_RIPEMD 0
	#else
		#define LIBSSH2_HMAC_RIPEMD 1
	#endif
	#define LIBSSH2_HMAC_SHA256 1
	#define LIBSSH2_HMAC_SHA512 1
	#if OPENSSL_VERSION_NUMBER >= 0x00907000L && !defined(OPENSSL_NO_AES)
		#define LIBSSH2_AES_CTR 1
		#define LIBSSH2_AES 1
	#else
		#define LIBSSH2_AES_CTR 0
		#define LIBSSH2_AES 0
	#endif
	#ifdef OPENSSL_NO_BF
		#define LIBSSH2_BLOWFISH 0
	#else
		#define LIBSSH2_BLOWFISH 1
	#endif
	#ifdef OPENSSL_NO_RC4
		#define LIBSSH2_RC4 0
	#else
		#define LIBSSH2_RC4 1
	#endif
	#ifdef OPENSSL_NO_CAST
		#define LIBSSH2_CAST 0
	#else
		#define LIBSSH2_CAST 1
	#endif
	#ifdef OPENSSL_NO_DES
		#define LIBSSH2_3DES 0
	#else
		#define LIBSSH2_3DES 1
	#endif
	#define _libssh2_random(buf, len) RAND_bytes ((buf), (len))
	#define libssh2_prepare_iovec(vec, len)  /* Empty. */
	#ifdef HAVE_OPAQUE_STRUCTS
		#define libssh2_sha1_ctx EVP_MD_CTX *
	#else
		#define libssh2_sha1_ctx EVP_MD_CTX
	#endif
	/* returns 0 in case of failure */
	int _libssh2_sha1_init(libssh2_sha1_ctx *ctx);
	#define libssh2_sha1_init(x) _libssh2_sha1_init(x)
	#ifdef HAVE_OPAQUE_STRUCTS
		#define libssh2_sha1_update(ctx, data, len) EVP_DigestUpdate(ctx, data, len)
		#define libssh2_sha1_final(ctx, out) do { EVP_DigestFinal(ctx, out, NULL); EVP_MD_CTX_free(ctx); } while(0)
	#else
		#define libssh2_sha1_update(ctx, data, len) EVP_DigestUpdate(&(ctx), data, len)
		#define libssh2_sha1_final(ctx, out) EVP_DigestFinal(&(ctx), out, NULL)
	#endif
	int _libssh2_sha1(const uchar *message, ulong len, uchar *out);
	#define libssh2_sha1(x,y,z) _libssh2_sha1(x,y,z)

	#ifdef HAVE_OPAQUE_STRUCTS
		#define libssh2_sha256_ctx EVP_MD_CTX *
	#else
		#define libssh2_sha256_ctx EVP_MD_CTX
	#endif

	/* returns 0 in case of failure */
	int _libssh2_sha256_init(libssh2_sha256_ctx *ctx);
	#define libssh2_sha256_init(x) _libssh2_sha256_init(x)
	#ifdef HAVE_OPAQUE_STRUCTS
		#define libssh2_sha256_update(ctx, data, len) EVP_DigestUpdate(ctx, data, len)
		#define libssh2_sha256_final(ctx, out) do { EVP_DigestFinal(ctx, out, NULL); EVP_MD_CTX_free(ctx); } while(0)
	#else
		#define libssh2_sha256_update(ctx, data, len) EVP_DigestUpdate(&(ctx), data, len)
		#define libssh2_sha256_final(ctx, out) EVP_DigestFinal(&(ctx), out, NULL)
	#endif
	int _libssh2_sha256(const uchar *message, ulong len, uchar *out);
	#define libssh2_sha256(x,y,z) _libssh2_sha256(x,y,z)
	#ifdef HAVE_OPAQUE_STRUCTS
		#define libssh2_md5_ctx EVP_MD_CTX *
	#else
		#define libssh2_md5_ctx EVP_MD_CTX
	#endif
	/* returns 0 in case of failure */
	int _libssh2_md5_init(libssh2_md5_ctx *ctx);
	#define libssh2_md5_init(x) _libssh2_md5_init(x)
	#ifdef HAVE_OPAQUE_STRUCTS
		#define libssh2_md5_update(ctx, data, len) EVP_DigestUpdate(ctx, data, len)
		#define libssh2_md5_final(ctx, out) do { EVP_DigestFinal(ctx, out, NULL); EVP_MD_CTX_free(ctx); } while(0)
	#else
		#define libssh2_md5_update(ctx, data, len) EVP_DigestUpdate(&(ctx), data, len)
		#define libssh2_md5_final(ctx, out) EVP_DigestFinal(&(ctx), out, NULL)
	#endif
	#ifdef HAVE_OPAQUE_STRUCTS
		#define libssh2_hmac_ctx HMAC_CTX *
		#define libssh2_hmac_ctx_init(ctx) ctx = HMAC_CTX_new()
		#define libssh2_hmac_sha1_init(ctx, key, keylen) HMAC_Init_ex(*(ctx), key, keylen, EVP_sha1(), NULL)
		#define libssh2_hmac_md5_init(ctx, key, keylen) HMAC_Init_ex(*(ctx), key, keylen, EVP_md5(), NULL)
		#define libssh2_hmac_ripemd160_init(ctx, key, keylen) HMAC_Init_ex(*(ctx), key, keylen, EVP_ripemd160(), NULL)
		#define libssh2_hmac_sha256_init(ctx, key, keylen) HMAC_Init_ex(*(ctx), key, keylen, EVP_sha256(), NULL)
		#define libssh2_hmac_sha512_init(ctx, key, keylen) HMAC_Init_ex(*(ctx), key, keylen, EVP_sha512(), NULL)
		#define libssh2_hmac_update(ctx, data, datalen) HMAC_Update(ctx, data, datalen)
		#define libssh2_hmac_final(ctx, data) HMAC_Final(ctx, data, NULL)
		#define libssh2_hmac_cleanup(ctx) HMAC_CTX_free(*(ctx))
	#else
		#define libssh2_hmac_ctx HMAC_CTX
		#define libssh2_hmac_ctx_init(ctx) HMAC_CTX_init(&ctx)
		#define libssh2_hmac_sha1_init(ctx, key, keylen) HMAC_Init_ex(ctx, key, keylen, EVP_sha1(), NULL)
		#define libssh2_hmac_md5_init(ctx, key, keylen) HMAC_Init_ex(ctx, key, keylen, EVP_md5(), NULL)
		#define libssh2_hmac_ripemd160_init(ctx, key, keylen) HMAC_Init_ex(ctx, key, keylen, EVP_ripemd160(), NULL)
		#define libssh2_hmac_sha256_init(ctx, key, keylen) HMAC_Init_ex(ctx, key, keylen, EVP_sha256(), NULL)
		#define libssh2_hmac_sha512_init(ctx, key, keylen) HMAC_Init_ex(ctx, key, keylen, EVP_sha512(), NULL)
		#define libssh2_hmac_update(ctx, data, datalen) HMAC_Update(&(ctx), data, datalen)
		#define libssh2_hmac_final(ctx, data) HMAC_Final(&(ctx), data, NULL)
		#define libssh2_hmac_cleanup(ctx) HMAC_cleanup(ctx)
	#endif
	#define libssh2_crypto_init() OpenSSL_add_all_algorithms(); ENGINE_load_builtin_engines(); ENGINE_register_all_complete()
	#define libssh2_crypto_exit()
	#define libssh2_rsa_ctx RSA
	#define _libssh2_rsa_free(rsactx) RSA_free(rsactx)
	#define libssh2_dsa_ctx DSA
	#define _libssh2_dsa_free(dsactx) DSA_free(dsactx)
	#define _libssh2_cipher_type(name) const EVP_CIPHER *(*name)(void)
	#ifdef HAVE_OPAQUE_STRUCTS
		#define _libssh2_cipher_ctx EVP_CIPHER_CTX *
	#else
		#define _libssh2_cipher_ctx EVP_CIPHER_CTX
	#endif
	#define _libssh2_cipher_aes256 EVP_aes_256_cbc
	#define _libssh2_cipher_aes192 EVP_aes_192_cbc
	#define _libssh2_cipher_aes128 EVP_aes_128_cbc
	#ifdef HAVE_EVP_AES_128_CTR
		#define _libssh2_cipher_aes128ctr EVP_aes_128_ctr
		#define _libssh2_cipher_aes192ctr EVP_aes_192_ctr
		#define _libssh2_cipher_aes256ctr EVP_aes_256_ctr
	#else
		#define _libssh2_cipher_aes128ctr _libssh2_EVP_aes_128_ctr
		#define _libssh2_cipher_aes192ctr _libssh2_EVP_aes_192_ctr
		#define _libssh2_cipher_aes256ctr _libssh2_EVP_aes_256_ctr
	#endif
	#define _libssh2_cipher_blowfish EVP_bf_cbc
	#define _libssh2_cipher_arcfour EVP_rc4
	#define _libssh2_cipher_cast5 EVP_cast5_cbc
	#define _libssh2_cipher_3des EVP_des_ede3_cbc
	#ifdef HAVE_OPAQUE_STRUCTS
		#define _libssh2_cipher_dtor(ctx) EVP_CIPHER_CTX_reset(*(ctx))
	#else
		#define _libssh2_cipher_dtor(ctx) EVP_CIPHER_CTX_cleanup(ctx)
	#endif
	#define _libssh2_bn BIGNUM
	#define _libssh2_bn_ctx BN_CTX
	#define _libssh2_bn_ctx_new() BN_CTX_new()
	#define _libssh2_bn_ctx_free(bnctx) BN_CTX_free(bnctx)
	#define _libssh2_bn_init() BN_new()
	#define _libssh2_bn_init_from_bin() _libssh2_bn_init()
	#define _libssh2_bn_rand(bn, bits, top, bottom) BN_rand(bn, bits, top, bottom)
	#define _libssh2_bn_mod_exp(r, a, p, m, ctx) BN_mod_exp(r, a, p, m, ctx)
	#define _libssh2_bn_set_word(bn, val) BN_set_word(bn, val)
	#define _libssh2_bn_from_bin(bn, len, val) BN_bin2bn(val, len, bn)
	#define _libssh2_bn_to_bin(bn, val) BN_bn2bin(bn, val)
	#define _libssh2_bn_bytes(bn) BN_num_bytes(bn)
	#define _libssh2_bn_bits(bn) BN_num_bits(bn)
	#define _libssh2_bn_free(bn) BN_clear_free(bn)

	const EVP_CIPHER *_libssh2_EVP_aes_128_ctr(void);
	const EVP_CIPHER *_libssh2_EVP_aes_192_ctr(void);
	const EVP_CIPHER *_libssh2_EVP_aes_256_ctr(void);
	//
#endif
#ifdef LIBSSH2_LIBGCRYPT
	#include "libgcrypt.h"
#endif
#ifdef LIBSSH2_WINCNG
	#include "wincng.h"
#endif
#ifdef LIBSSH2_OS400QC3
	#include "os400qc3.h"
#endif
#ifdef LIBSSH2_MBEDTLS
	#include "mbedtls.h"
#endif

int _libssh2_rsa_new(libssh2_rsa_ctx ** rsa, const uchar * edata, ulong elen,
    const uchar * ndata, ulong nlen, const uchar * ddata, ulong dlen,
    const uchar * pdata, ulong plen, const uchar * qdata, ulong qlen,
    const uchar * e1data, ulong e1len, const uchar * e2data, ulong e2len, const uchar * coeffdata, ulong coefflen);
int _libssh2_rsa_new_private(libssh2_rsa_ctx ** rsa, LIBSSH2_SESSION * session, const char * filename, const uchar * passphrase);
int _libssh2_rsa_sha1_verify(libssh2_rsa_ctx * rsa, const uchar * sig, ulong sig_len, const uchar * m, ulong m_len);
int _libssh2_rsa_sha1_sign(LIBSSH2_SESSION * session, libssh2_rsa_ctx * rsactx, const uchar * hash, size_t hash_len, uchar ** signature, size_t * signature_len);
int _libssh2_rsa_new_private_frommemory(libssh2_rsa_ctx ** rsa, LIBSSH2_SESSION * session, const char * filedata, size_t filedata_len, const uchar * passphrase);
#if LIBSSH2_DSA
	int _libssh2_dsa_new(libssh2_dsa_ctx ** dsa, const uchar * pdata, ulong plen, const uchar * qdata, ulong qlen,
		const uchar * gdata, ulong glen, const uchar * ydata, ulong ylen, const uchar * x, ulong x_len);
	int _libssh2_dsa_new_private(libssh2_dsa_ctx ** dsa, LIBSSH2_SESSION * session, const char * filename, const uchar * passphrase);
	int _libssh2_dsa_sha1_verify(libssh2_dsa_ctx * dsactx, const uchar * sig, const uchar * m, ulong m_len);
	int _libssh2_dsa_sha1_sign(libssh2_dsa_ctx * dsactx, const uchar * hash, ulong hash_len, uchar * sig);
	int _libssh2_dsa_new_private_frommemory(libssh2_dsa_ctx ** dsa, LIBSSH2_SESSION * session, const char * filedata, size_t filedata_len, const uchar * passphrase);
#endif
int _libssh2_cipher_init(_libssh2_cipher_ctx * h, _libssh2_cipher_type(algo), uchar *iv, uchar *secret, int encrypt);
int _libssh2_cipher_crypt(_libssh2_cipher_ctx * ctx, _libssh2_cipher_type(algo), int encrypt, uchar *block, size_t blocksize);
int _libssh2_pub_priv_keyfile(LIBSSH2_SESSION * session, uchar ** method, size_t * method_len,
    uchar ** pubkeydata, size_t * pubkeydata_len, const char * privatekey, const char * passphrase);
int _libssh2_pub_priv_keyfilememory(LIBSSH2_SESSION * session, uchar ** method, size_t * method_len,
    uchar ** pubkeydata, size_t * pubkeydata_len, const char * privatekeydata, size_t privatekeydata_len, const char * passphrase);
void _libssh2_init_aes_ctr(void);
//
#ifdef HAVE_WINSOCK2_H
//#include <winsock2.h>
	#include <ws2tcpip.h>
#endif

/* RFC4253 section 6.1 Maximum Packet Length says:
 *
 * "All implementations MUST be able to process packets with
 * uncompressed payload length of 32768 bytes or less and
 * total packet size of 35000 bytes or less (including length,
 * padding length, payload, padding, and MAC.)."
 */
#define MAX_SSH_PACKET_LEN 35000
#define MAX_SHA_DIGEST_LEN SHA256_DIGEST_LENGTH

//#define LIBSSH2_ALLOC(session, count) session->alloc((count), &(session)->abstract)
//#define LIBSSH2_CALLOC(session, count) _libssh2_calloc(session, count)
//#define LIBSSH2_REALLOC(session, ptr, count) ((ptr) ? session->realloc((ptr), (count), &(session)->abstract) : session->alloc((count), &(session)->abstract))
//#define LIBSSH2_FREE(session, ptr) session->free((ptr), &(session)->abstract)

#define LIBSSH2_ALLOC(session, count) SAlloc::M(count)
#define LIBSSH2_CALLOC(session, count) SAlloc::C(count, 1)
#define LIBSSH2_REALLOC(session, ptr, count) SAlloc::R(ptr, count)
#define LIBSSH2_FREE(session, ptr) SAlloc::F(ptr)

#define LIBSSH2_IGNORE(session, data, datalen) session->ssh_msg_ignore((session), (data), (datalen), &(session)->abstract)
#define LIBSSH2_DEBUG(session, always_display, message, message_len, language, language_len)    \
	session->ssh_msg_debug((session), (always_display), (message), (message_len), (language), (language_len), &(session)->abstract)
#define LIBSSH2_DISCONNECT(session, reason, message, message_len, language, language_len)		   \
	session->ssh_msg_disconnect((session), (reason), (message), (message_len), (language), (language_len), &(session)->abstract)

#define LIBSSH2_MACERROR(session, data, datalen) session->macerror((session), (data), (datalen), &(session)->abstract)
#define LIBSSH2_X11_OPEN(channel, shost, sport)  channel->session->x11(((channel)->session), (channel), (shost), (sport), (&(channel)->session->abstract))
#define LIBSSH2_CHANNEL_CLOSE(session, channel)  channel->close_cb((session), &(session)->abstract, (channel), &(channel)->abstract)
#define LIBSSH2_SEND_FD(session, fd, buffer, length, flags) (session->send)(fd, buffer, length, flags, &session->abstract)
#define LIBSSH2_RECV_FD(session, fd, buffer, length, flags) (session->recv)(fd, buffer, length, flags, &session->abstract)
#define LIBSSH2_SEND(session, buffer, length, flags)  LIBSSH2_SEND_FD(session, session->socket_fd, buffer, length, flags)
#define LIBSSH2_RECV(session, buffer, length, flags)  LIBSSH2_RECV_FD(session, session->socket_fd, buffer, length, flags)

typedef struct _LIBSSH2_KEX_METHOD LIBSSH2_KEX_METHOD;
typedef struct _LIBSSH2_HOSTKEY_METHOD LIBSSH2_HOSTKEY_METHOD;
typedef struct _LIBSSH2_CRYPT_METHOD LIBSSH2_CRYPT_METHOD;
typedef struct _LIBSSH2_COMP_METHOD LIBSSH2_COMP_METHOD;
typedef struct _LIBSSH2_PACKET LIBSSH2_PACKET;

const LIBSSH2_COMP_METHOD **_libssh2_comp_methods(LIBSSH2_SESSION *session);

typedef enum {
	libssh2_NB_state_idle = 0,
	libssh2_NB_state_allocated,
	libssh2_NB_state_created,
	libssh2_NB_state_sent,
	libssh2_NB_state_sent1,
	libssh2_NB_state_sent2,
	libssh2_NB_state_sent3,
	libssh2_NB_state_sent4,
	libssh2_NB_state_sent5,
	libssh2_NB_state_sent6,
	libssh2_NB_state_sent7,
	libssh2_NB_state_jump1,
	libssh2_NB_state_jump2,
	libssh2_NB_state_jump3,
	libssh2_NB_state_jump4,
	libssh2_NB_state_jump5,
	libssh2_NB_state_end
} libssh2_nonblocking_states;

typedef struct packet_require_state_t {
	libssh2_nonblocking_states state;
	time_t start;
} packet_require_state_t;

typedef struct packet_requirev_state_t {
	time_t start;
} packet_requirev_state_t;

typedef struct kmdhgGPshakex_state_t {
	libssh2_nonblocking_states state;
	uchar * e_packet;
	uchar * s_packet;
	uchar * tmp;
	uchar h_sig_comp[MAX_SHA_DIGEST_LEN];
	uchar c;
	size_t e_packet_len;
	size_t s_packet_len;
	size_t tmp_len;
	_libssh2_bn_ctx * ctx;
	_libssh2_bn * x;
	_libssh2_bn * e;
	_libssh2_bn * f;
	_libssh2_bn * k;
	uchar * s;
	uchar * f_value;
	uchar * k_value;
	uchar * h_sig;
	size_t f_value_len;
	size_t k_value_len;
	size_t h_sig_len;
	void * exchange_hash;
	packet_require_state_t req_state;
	libssh2_nonblocking_states burn_state;
} kmdhgGPshakex_state_t;

typedef struct key_exchange_state_low_t {
	libssh2_nonblocking_states state;
	packet_require_state_t req_state;
	kmdhgGPshakex_state_t exchange_state;
	_libssh2_bn * p;        /* SSH2 defined value (p_value) */
	_libssh2_bn * g;        /* SSH2 defined value (2) */
	uchar request[13];
	uchar * data;
	size_t request_len;
	size_t data_len;
} key_exchange_state_low_t;

typedef struct key_exchange_state_t {
	libssh2_nonblocking_states state;
	packet_require_state_t req_state;
	key_exchange_state_low_t key_state_low;
	uchar * data;
	size_t data_len;
	uchar * oldlocal;
	size_t oldlocal_len;
} key_exchange_state_t;

#define FwdNotReq "Forward not requested"

typedef struct packet_queue_listener_state_t {
	libssh2_nonblocking_states state;
	uchar packet[17 + (sizeof(FwdNotReq) - 1)];
	uchar * host;
	uchar * shost;
	uint32 sender_channel;
	uint32 initial_window_size;
	uint32 packet_size;
	uint32 port;
	uint32 sport;
	uint32 host_len;
	uint32 shost_len;
	LIBSSH2_CHANNEL * channel;
} packet_queue_listener_state_t;

#define X11FwdUnAvil "X11 Forward Unavailable"

typedef struct packet_x11_open_state_t {
	libssh2_nonblocking_states state;
	uchar packet[17 + (sizeof(X11FwdUnAvil) - 1)];
	uchar * shost;
	uint32 sender_channel;
	uint32 initial_window_size;
	uint32 packet_size;
	uint32 sport;
	uint32 shost_len;
	LIBSSH2_CHANNEL * channel;
} packet_x11_open_state_t;

struct _LIBSSH2_PACKET {
	struct list_node node; /* linked list header */
	uchar * data; /* the raw unencrypted payload */
	size_t data_len;
	size_t data_head; // Where to start reading data from, used for channel data that's been partially consumed 
};

typedef struct _libssh2_channel_data {
	uint32 id; /* Identifier */
	/* Limits and restrictions */
	uint32 window_size_initial;
	uint32 window_size;
	uint32 packet_size;
	/* Set to 1 when CHANNEL_CLOSE / CHANNEL_EOF sent/received */
	char   close;
	char   eof;
	char   extended_data_ignore_mode;
} libssh2_channel_data;

struct _LIBSSH2_CHANNEL {
	struct list_node node;
	uchar * channel_type;
	unsigned channel_type_len;
	int exit_status; /* channel's program exit status */
	char * exit_signal; /* channel's program exit signal (without the SIG prefix) */
	libssh2_channel_data local, remote;
	uint32 adjust_queue; /* Amount of bytes to be refunded to receive window (but not yet sent) */
	uint32 read_avail; /* Data immediately available for reading */
	LIBSSH2_SESSION * session;
	void * abstract;
	LIBSSH2_CHANNEL_CLOSE_FUNC((*close_cb));
	/* State variables used in libssh2_channel_setenv_ex() */
	libssh2_nonblocking_states setenv_state;
	uchar * setenv_packet;
	size_t setenv_packet_len;
	uchar setenv_local_channel[4];
	packet_requirev_state_t setenv_packet_requirev_state;
	libssh2_nonblocking_states reqPTY_state; // State variables used in libssh2_channel_request_pty_ex() libssh2_channel_request_pty_size_ex() 
	uchar reqPTY_packet[41 + 256];
	size_t reqPTY_packet_len;
	uchar reqPTY_local_channel[4];
	packet_requirev_state_t reqPTY_packet_requirev_state;
	libssh2_nonblocking_states reqX11_state; /* State variables used in libssh2_channel_x11_req_ex() */
	uchar * reqX11_packet;
	size_t reqX11_packet_len;
	uchar reqX11_local_channel[4];
	packet_requirev_state_t reqX11_packet_requirev_state;
	libssh2_nonblocking_states process_state; /* State variables used in libssh2_channel_process_startup() */
	uchar * process_packet;
	size_t process_packet_len;
	uchar process_local_channel[4];
	packet_requirev_state_t process_packet_requirev_state;
	libssh2_nonblocking_states flush_state; /* State variables used in libssh2_channel_flush_ex() */
	size_t flush_refund_bytes;
	size_t flush_flush_bytes;
	libssh2_nonblocking_states adjust_state; /* State variables used in libssh2_channel_receive_window_adjust() */
	uchar adjust_adjust[9]; /* packet_type(1) + channel(4) + adjustment(4) */
	libssh2_nonblocking_states read_state; /* State variables used in libssh2_channel_read_ex() */
	uint32 read_local_id;
	libssh2_nonblocking_states write_state; /* State variables used in libssh2_channel_write_ex() */
	uchar write_packet[13];
	size_t write_packet_len;
	size_t write_bufwrite;
	libssh2_nonblocking_states close_state; /* State variables used in libssh2_channel_close() */
	uchar close_packet[5];
	libssh2_nonblocking_states wait_eof_state; /* State variables used in libssh2_channel_wait_closedeof() */
	libssh2_nonblocking_states wait_closed_state; /* State variables used in libssh2_channel_wait_closed() */
	libssh2_nonblocking_states free_state; /* State variables used in libssh2_channel_free() */
	libssh2_nonblocking_states extData2_state; /* State variables used in libssh2_channel_handle_extended_data2() */
};

struct _LIBSSH2_LISTENER {
	struct list_node node; /* linked list header */
	LIBSSH2_SESSION * session;
	char * host;
	int    port;
	/* a list of CHANNELs for this listener */
	struct list_head queue;
	int    queue_size;
	int    queue_maxsize;
	/* State variables used in libssh2_channel_forward_cancel() */
	libssh2_nonblocking_states chanFwdCncl_state;
	uchar * chanFwdCncl_data;
	size_t chanFwdCncl_data_len;
};

typedef struct _libssh2_endpoint_data {
	uchar * banner;
	uchar * kexinit;
	size_t kexinit_len;
	const  LIBSSH2_CRYPT_METHOD * crypt;
	void * crypt_abstract;
	const  struct _LIBSSH2_MAC_METHOD * mac;
	uint32 seqno;
	void * mac_abstract;
	const  LIBSSH2_COMP_METHOD * comp;
	void * comp_abstract;
	/* Method Preferences -- NULL yields "load order" */
	char * crypt_prefs;
	char * mac_prefs;
	char * comp_prefs;
	char * lang_prefs;
} libssh2_endpoint_data;

#define PACKETBUFSIZE (1024*16)

struct transportpacket {
	/* ------------- for incoming data --------------- */
	uchar  buf[PACKETBUFSIZE];
	uchar  init[5]; /* first 5 bytes of the incoming data stream, still encrypted */
	size_t writeidx;    /* at what array index we do the next write into the buffer */
	size_t readidx;     /* at what array index we do the next read from the buffer */
	uint32 packet_length; /* the most recent packet_length as read from the network data */
	uint8  padding_length; /* the most recent padding_length as read from the network data */
	size_t data_num;    /* How much of the total package that has been read so far. */
	size_t total_num;   /* How much a total package is supposed to be, in number of bytes. A full package is packet_length + padding_length + 4 + mac_length. */
	uchar * payload; /* this is a pointer to a LIBSSH2_ALLOC() area to which we write decrypted data */
	uchar * wptr; /* write pointer into the payload to where we are currently writing decrypted data */
	/* ------------- for outgoing data --------------- */
	uchar  outbuf[MAX_SSH_PACKET_LEN]; /* area for the outgoing data */
	int    ototal_num;     /* size of outbuf in number of bytes */
	const  uchar * odata; /* original pointer to the data */
	size_t olen;        /* original size of the data we stored in outbuf */
	size_t osent;       /* number of bytes already sent */
};

struct _LIBSSH2_PUBLICKEY {
	LIBSSH2_CHANNEL * channel;
	uint32 version;
	libssh2_nonblocking_states receive_state; /* State variables used in libssh2_publickey_packet_receive() */
	uchar * receive_packet;
	size_t receive_packet_len;
	libssh2_nonblocking_states add_state; /* State variables used in libssh2_publickey_add_ex() */
	uchar * add_packet;
	uchar * add_s;
	libssh2_nonblocking_states remove_state; /* State variables used in libssh2_publickey_remove_ex() */
	uchar * remove_packet;
	uchar * remove_s;
	libssh2_nonblocking_states listFetch_state; /* State variables used in libssh2_publickey_list_fetch() */
	uchar * listFetch_s;
	uchar listFetch_buffer[12];
	uchar * listFetch_data;
	size_t listFetch_data_len;
};

#define LIBSSH2_SCP_RESPONSE_BUFLEN     256

struct flags {
	int sigpipe; /* LIBSSH2_FLAG_SIGPIPE */
	int compress; /* LIBSSH2_FLAG_COMPRESS */
};

struct _LIBSSH2_SESSION {
	/* Memory management callbacks */
	void * abstract;
	//LIBSSH2_ALLOC_FUNC((*alloc));
	//LIBSSH2_REALLOC_FUNC((*realloc));
	//LIBSSH2_FREE_FUNC((*free));

	/* Other callbacks */
	LIBSSH2_IGNORE_FUNC((*ssh_msg_ignore));
	LIBSSH2_DEBUG_FUNC((*ssh_msg_debug));
	LIBSSH2_DISCONNECT_FUNC((*ssh_msg_disconnect));
	LIBSSH2_MACERROR_FUNC((*macerror));
	LIBSSH2_X11_OPEN_FUNC((*x11));
	LIBSSH2_SEND_FUNC((*send));
	LIBSSH2_RECV_FUNC((*recv));

	/* Method preferences -- NULL yields "load order" */
	char * kex_prefs;
	char * hostkey_prefs;
	int state;
	/* Flag options */
	struct flags flag;

	/* Agreed Key Exchange Method */
	const LIBSSH2_KEX_METHOD * kex;
	uint burn_optimistic_kexinit : 1;
	uchar * session_id;
	uint32 session_id_len;
	/* this is set to TRUE if a blocking API behavior is requested */
	int api_block_mode;
	/* Timeout used when blocking API behavior is active */
	long api_timeout;
	/* Server's public key */
	const LIBSSH2_HOSTKEY_METHOD * hostkey;
	void * server_hostkey_abstract;
	/* Either set with libssh2_session_hostkey() (for server mode)
	 * Or read from server in (eg) KEXDH_INIT (for client mode)
	 */
	uchar * server_hostkey;
	uint32 server_hostkey_len;
#if LIBSSH2_MD5
	uchar server_hostkey_md5[MD5_DIGEST_LENGTH];
	int server_hostkey_md5_valid;
#endif                          /* ! LIBSSH2_MD5 */
	uchar server_hostkey_sha1[SHA_DIGEST_LENGTH];
	int server_hostkey_sha1_valid;
	libssh2_endpoint_data remote; /* (remote as source of data -- packet_read ) */
	libssh2_endpoint_data local; /* (local as source of data -- packet_write ) */
	struct list_head packets; /* Inbound Data linked list -- Sometimes the packet that comes in isn't the packet we're ready for */
	struct list_head channels; /* Active connection channels */
	uint32 next_channel;
	struct list_head listeners; /* list of LIBSSH2_LISTENER structs */
	/* Actual I/O socket */
	libssh2_socket_t socket_fd;
	int socket_state;
	int socket_block_directions;
	int socket_prev_blockstate; /* stores the state of the socket blockiness when libssh2_session_startup() is called */
	/* Error tracking */
	const char * err_msg;
	int err_code;
	int err_flags;
	struct transportpacket packet; /* struct members for packet-level reading */
#ifdef LIBSSH2DEBUG
	int showmask;           /* what debug/trace messages to display */
	libssh2_trace_handler_func tracehandler; /* callback to display trace messages */
	void* tracehandler_context; /* context for the trace handler */
#endif
	libssh2_nonblocking_states banner_TxRx_state; /* State variables used in libssh2_banner_send() */
	char banner_TxRx_banner[256];
	ssize_t banner_TxRx_total_send;
	libssh2_nonblocking_states kexinit_state; /* State variables used in libssh2_kexinit() */
	uchar * kexinit_data;
	size_t kexinit_data_len;
	libssh2_nonblocking_states startup_state; /* State variables used in libssh2_session_startup() */
	uchar * startup_data;
	size_t startup_data_len;
	uchar startup_service[sizeof("ssh-userauth") + 5 - 1];
	size_t startup_service_length;
	packet_require_state_t startup_req_state;
	key_exchange_state_t startup_key_state;
	libssh2_nonblocking_states free_state; /* State variables used in libssh2_session_free() */
	libssh2_nonblocking_states disconnect_state; /* State variables used in libssh2_session_disconnect_ex() */
	uchar disconnect_data[256 + 13];
	size_t disconnect_data_len;
	libssh2_nonblocking_states readPack_state; /* State variables used in libssh2_packet_read() */
	int readPack_encrypted;
	libssh2_nonblocking_states userauth_list_state; /* State variables used in libssh2_userauth_list() */
	uchar * userauth_list_data;
	size_t userauth_list_data_len;
	packet_requirev_state_t userauth_list_packet_requirev_state;
	libssh2_nonblocking_states userauth_pswd_state; /* State variables used in libssh2_userauth_password_ex() */
	uchar * userauth_pswd_data;
	uchar userauth_pswd_data0;
	size_t userauth_pswd_data_len;
	char * userauth_pswd_newpw;
	int userauth_pswd_newpw_len;
	packet_requirev_state_t userauth_pswd_packet_requirev_state;
	libssh2_nonblocking_states userauth_host_state; /* State variables used in libssh2_userauth_hostbased_fromfile_ex() */
	uchar * userauth_host_data;
	size_t userauth_host_data_len;
	uchar * userauth_host_packet;
	size_t userauth_host_packet_len;
	uchar * userauth_host_method;
	size_t userauth_host_method_len;
	uchar * userauth_host_s;
	packet_requirev_state_t userauth_host_packet_requirev_state;
	libssh2_nonblocking_states userauth_pblc_state; /* State variables used in libssh2_userauth_publickey_fromfile_ex() */
	uchar * userauth_pblc_data;
	size_t userauth_pblc_data_len;
	uchar * userauth_pblc_packet;
	size_t userauth_pblc_packet_len;
	uchar * userauth_pblc_method;
	size_t userauth_pblc_method_len;
	uchar * userauth_pblc_s;
	uchar * userauth_pblc_b;
	packet_requirev_state_t userauth_pblc_packet_requirev_state;
	libssh2_nonblocking_states userauth_kybd_state; /* State variables used in libssh2_userauth_keyboard_interactive_ex() */
	uchar * userauth_kybd_data;
	size_t userauth_kybd_data_len;
	uchar * userauth_kybd_packet;
	size_t userauth_kybd_packet_len;
	uint userauth_kybd_auth_name_len;
	char * userauth_kybd_auth_name;
	unsigned userauth_kybd_auth_instruction_len;
	char * userauth_kybd_auth_instruction;
	uint userauth_kybd_num_prompts;
	int userauth_kybd_auth_failure;
	LIBSSH2_USERAUTH_KBDINT_PROMPT * userauth_kybd_prompts;
	LIBSSH2_USERAUTH_KBDINT_RESPONSE * userauth_kybd_responses;
	packet_requirev_state_t userauth_kybd_packet_requirev_state;
	libssh2_nonblocking_states open_state; /* State variables used in libssh2_channel_open_ex() */
	packet_requirev_state_t open_packet_requirev_state;
	LIBSSH2_CHANNEL * open_channel;
	uchar * open_packet;
	size_t open_packet_len;
	uchar * open_data;
	size_t open_data_len;
	uint32 open_local_channel;
	libssh2_nonblocking_states direct_state; /* State variables used in libssh2_channel_direct_tcpip_ex() */
	uchar * direct_message;
	size_t direct_host_len;
	size_t direct_shost_len;
	size_t direct_message_len;
	libssh2_nonblocking_states fwdLstn_state; /* State variables used in libssh2_channel_forward_listen_ex() */
	uchar * fwdLstn_packet;
	uint32 fwdLstn_host_len;
	uint32 fwdLstn_packet_len;
	packet_requirev_state_t fwdLstn_packet_requirev_state;
	libssh2_nonblocking_states pkeyInit_state; /* State variables used in libssh2_publickey_init() */
	LIBSSH2_PUBLICKEY * pkeyInit_pkey;
	LIBSSH2_CHANNEL * pkeyInit_channel;
	uchar * pkeyInit_data;
	size_t pkeyInit_data_len;
	uchar pkeyInit_buffer[19]; /* 19 = packet_len(4) + version_len(4) + "version"(7) + version_num(4) */
	size_t pkeyInit_buffer_sent; /* how much of buffer that has been sent */
	libssh2_nonblocking_states packAdd_state; /* State variables used in libssh2_packet_add() */
	LIBSSH2_CHANNEL * packAdd_channelp; /* keeper of the channel during EAGAIN states */
	packet_queue_listener_state_t packAdd_Qlstn_state;
	packet_x11_open_state_t packAdd_x11open_state;
	libssh2_nonblocking_states fullpacket_state; /* State variables used in fullpacket() */
	int fullpacket_macstate;
	size_t fullpacket_payload_len;
	int fullpacket_packet_type;
	libssh2_nonblocking_states sftpInit_state; /* State variables used in libssh2_sftp_init() */
	LIBSSH2_SFTP * sftpInit_sftp;
	LIBSSH2_CHANNEL * sftpInit_channel;
	uchar sftpInit_buffer[9]; // sftp_header(5){excludes request_id} + version_id(4) 
	int sftpInit_sent; // number of bytes from the buffer that have been sent 
	libssh2_nonblocking_states scpRecv_state; /* State variables used in libssh2_scp_recv() / libssh_scp_recv2() */
	uchar * scpRecv_command;
	size_t scpRecv_command_len;
	uchar scpRecv_response[LIBSSH2_SCP_RESPONSE_BUFLEN];
	size_t scpRecv_response_len;
	long scpRecv_mode;
#if defined(HAVE_LONGLONG) && defined(HAVE_STRTOLL)
	/* we have the type and we can parse such numbers */
	long long scpRecv_size;
#define scpsize_strtol strtoll
#elif defined(HAVE_STRTOI64)
	__int64 scpRecv_size;
#define scpsize_strtol _strtoi64
#else
	long scpRecv_size;
#define scpsize_strtol strtol
#endif
	long scpRecv_mtime;
	long scpRecv_atime;
	LIBSSH2_CHANNEL * scpRecv_channel;
	libssh2_nonblocking_states scpSend_state; /* State variables used in libssh2_scp_send_ex() */
	uchar * scpSend_command;
	size_t scpSend_command_len;
	uchar scpSend_response[LIBSSH2_SCP_RESPONSE_BUFLEN];
	size_t scpSend_response_len;
	LIBSSH2_CHANNEL * scpSend_channel;
	/* Keepalive variables used by keepalive.c. */
	int keepalive_interval;
	int keepalive_want_reply;
	time_t keepalive_last_sent;
};

/* session.state bits */
#define LIBSSH2_STATE_EXCHANGING_KEYS   0x00000001
#define LIBSSH2_STATE_NEWKEYS           0x00000002
#define LIBSSH2_STATE_AUTHENTICATED     0x00000004
#define LIBSSH2_STATE_KEX_ACTIVE        0x00000008

/* session.flag helpers */
#ifdef MSG_NOSIGNAL
	#define LIBSSH2_SOCKET_SEND_FLAGS(session) (((session)->flag.sigpipe) ? 0 : MSG_NOSIGNAL)
	#define LIBSSH2_SOCKET_RECV_FLAGS(session) (((session)->flag.sigpipe) ? 0 : MSG_NOSIGNAL)
#else
	/* If MSG_NOSIGNAL isn't defined we're SOL on blocking SIGPIPE */
	#define LIBSSH2_SOCKET_SEND_FLAGS(session)      0
	#define LIBSSH2_SOCKET_RECV_FLAGS(session)      0
#endif

/* --------- */

/* libssh2 extensible ssh api, ultimately I'd like to allow loading additional
   methods via .so/.dll */

struct _LIBSSH2_KEX_METHOD {
	const char * name;
	/* Key exchange, populates session->* and returns 0 on success, non-0 on error */
	int (* exchange_keys)(LIBSSH2_SESSION * session, key_exchange_state_low_t * key_state);
	long flags;
};

struct _LIBSSH2_HOSTKEY_METHOD {
	const char * name;
	ulong hash_len;
	int (* init)(LIBSSH2_SESSION * session, const uchar * hostkey_data, size_t hostkey_data_len, void ** abstract);
	int (* initPEM)(LIBSSH2_SESSION * session, const char * privkeyfile, const uchar * passphrase, void ** abstract);
	int (* initPEMFromMemory)(LIBSSH2_SESSION * session, const char * privkeyfiledata, size_t privkeyfiledata_len, const uchar * passphrase, void ** abstract);
	int (* sig_verify)(LIBSSH2_SESSION * session, const uchar * sig, size_t sig_len, const uchar * m, size_t m_len, void ** abstract);
	int (* signv)(LIBSSH2_SESSION * session, uchar ** signature, size_t * signature_len, int veccount, const struct iovec datavec[], void ** abstract);
	int (* encrypt)(LIBSSH2_SESSION * session, uchar ** dst, size_t * dst_len, const uchar * src, size_t src_len, void ** abstract);
	int (* dtor)(LIBSSH2_SESSION * session, void ** abstract);
};

struct _LIBSSH2_CRYPT_METHOD {
	const char * name;
	int blocksize;
	/* iv and key sizes (-1 for variable length) */
	int iv_len;
	int secret_len;
	long flags;
	int (* init)(LIBSSH2_SESSION * session, const LIBSSH2_CRYPT_METHOD * method, uchar * iv, int * free_iv, uchar * secret, int * free_secret, int encrypt, void ** abstract);
	int (* crypt)(LIBSSH2_SESSION * session, uchar * block, size_t blocksize, void ** abstract);
	int (* dtor)(LIBSSH2_SESSION * session, void ** abstract);

	_libssh2_cipher_type(algo);
};

struct _LIBSSH2_COMP_METHOD {
	const char * name;
	int compress; /* 1 if it does compress, 0 if it doesn't */
	int use_in_auth; /* 1 if compression should be used in userauth */
	int (* init)(LIBSSH2_SESSION * session, int compress, void ** abstract);
	int (* comp)(LIBSSH2_SESSION * session,
	    uchar * dest,
	    size_t * dest_len,
	    const uchar * src,
	    size_t src_len,
	    void ** abstract);
	int (* decomp)(LIBSSH2_SESSION * session,
	    uchar ** dest,
	    size_t * dest_len,
	    size_t payload_limit,
	    const uchar * src,
	    size_t src_len,
	    void ** abstract);
	int (* dtor)(LIBSSH2_SESSION * session, int compress, void ** abstract);
};

#ifdef LIBSSH2DEBUG
void _libssh2_debug(LIBSSH2_SESSION * session, int context, const char * format,
    ...);
#else
#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || defined(__GNUC__)
/* C99 supported and also by older GCC */
#define _libssh2_debug(x, y, z, ...) do {} while(0)
#else
/* no gcc and not C99, do static and hopefully inline */
static inline void _libssh2_debug(LIBSSH2_SESSION * session, int context, const char * format, ...)
{
	(void)session;
	(void)context;
	(void)format;
}

#endif
#endif

#define LIBSSH2_SOCKET_UNKNOWN                   1
#define LIBSSH2_SOCKET_CONNECTED                 0
#define LIBSSH2_SOCKET_DISCONNECTED             -1

#define LIBSSH2_MAC_UNCONFIRMED                  1 // Initial packet state, prior to MAC check 
#define LIBSSH2_MAC_CONFIRMED                    0 // When MAC type is "none" (proto initiation phase) all packets are deemed "confirmed" 
#define LIBSSH2_MAC_INVALID                     -1 // Something very bad is going on 

/* Flags for _libssh2_error_flags */
/* Error message is allocated on the heap */
#define LIBSSH2_ERR_FLAG_DUP                     1

/* SSH Packet Types -- Defined by internet draft */
/* Transport Layer */
#define SSH_MSG_DISCONNECT                          1
#define SSH_MSG_IGNORE                              2
#define SSH_MSG_UNIMPLEMENTED                       3
#define SSH_MSG_DEBUG                               4
#define SSH_MSG_SERVICE_REQUEST                     5
#define SSH_MSG_SERVICE_ACCEPT                      6

#define SSH_MSG_KEXINIT                             20
#define SSH_MSG_NEWKEYS                             21

/* diffie-hellman-group1-sha1 */
#define SSH_MSG_KEXDH_INIT                          30
#define SSH_MSG_KEXDH_REPLY                         31

/* diffie-hellman-group-exchange-sha1 and diffie-hellman-group-exchange-sha256 */
#define SSH_MSG_KEX_DH_GEX_REQUEST_OLD              30
#define SSH_MSG_KEX_DH_GEX_REQUEST                  34
#define SSH_MSG_KEX_DH_GEX_GROUP                    31
#define SSH_MSG_KEX_DH_GEX_INIT                     32
#define SSH_MSG_KEX_DH_GEX_REPLY                    33

/* User Authentication */
#define SSH_MSG_USERAUTH_REQUEST                    50
#define SSH_MSG_USERAUTH_FAILURE                    51
#define SSH_MSG_USERAUTH_SUCCESS                    52
#define SSH_MSG_USERAUTH_BANNER                     53

/* "public key" method */
#define SSH_MSG_USERAUTH_PK_OK                      60
/* "password" method */
#define SSH_MSG_USERAUTH_PASSWD_CHANGEREQ           60
/* "keyboard-interactive" method */
#define SSH_MSG_USERAUTH_INFO_REQUEST               60
#define SSH_MSG_USERAUTH_INFO_RESPONSE              61

/* Channels */
#define SSH_MSG_GLOBAL_REQUEST                      80
#define SSH_MSG_REQUEST_SUCCESS                     81
#define SSH_MSG_REQUEST_FAILURE                     82

#define SSH_MSG_CHANNEL_OPEN                        90
#define SSH_MSG_CHANNEL_OPEN_CONFIRMATION           91
#define SSH_MSG_CHANNEL_OPEN_FAILURE                92
#define SSH_MSG_CHANNEL_WINDOW_ADJUST               93
#define SSH_MSG_CHANNEL_DATA                        94
#define SSH_MSG_CHANNEL_EXTENDED_DATA               95
#define SSH_MSG_CHANNEL_EOF                         96
#define SSH_MSG_CHANNEL_CLOSE                       97
#define SSH_MSG_CHANNEL_REQUEST                     98
#define SSH_MSG_CHANNEL_SUCCESS                     99
#define SSH_MSG_CHANNEL_FAILURE                     100

/* Error codes returned in SSH_MSG_CHANNEL_OPEN_FAILURE message
   (see RFC4254) */
#define SSH_OPEN_ADMINISTRATIVELY_PROHIBITED 1
#define SSH_OPEN_CONNECT_FAILED              2
#define SSH_OPEN_UNKNOWN_CHANNELTYPE         3
#define SSH_OPEN_RESOURCE_SHORTAGE           4

ssize_t _libssh2_recv(libssh2_socket_t socket, void * buffer, size_t length, int flags, void ** abstract);
ssize_t _libssh2_send(libssh2_socket_t socket, const void * buffer, size_t length, int flags, void ** abstract);

#define LIBSSH2_READ_TIMEOUT 60 // generic timeout in seconds used when waiting for more data to arrive 

int _libssh2_kex_exchange(LIBSSH2_SESSION * session, int reexchange, key_exchange_state_t * state);
/* Let crypt.c/hostkey.c expose their method structs */
const LIBSSH2_CRYPT_METHOD ** libssh2_crypt_methods(void);
const LIBSSH2_HOSTKEY_METHOD ** libssh2_hostkey_methods(void);

/* pem.c */
int _libssh2_pem_parse(LIBSSH2_SESSION * session, const char * headerbegin, const char * headerend, FILE * fp, uchar ** data, uint * datalen);
int _libssh2_pem_parse_memory(LIBSSH2_SESSION * session, const char * headerbegin, const char * headerend, const char * filedata, size_t filedata_len, uchar ** data, uint * datalen);
int _libssh2_pem_decode_sequence(uchar ** data, uint * datalen);
int _libssh2_pem_decode_integer(uchar ** data, uint * datalen, uchar ** i, uint * ilen);
/* global.c */
void _libssh2_init_if_needed(void);

#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a)[0]))

/* define to output the int64 type in a *printf() */
#if defined( __BORLANDC__ ) || defined( _MSC_VER ) || defined( __MINGW32__ )
	#define LIBSSH2_INT64_T_FORMAT "I64d"
#else
	#define LIBSSH2_INT64_T_FORMAT "lld"
#endif
//
//#include "channel.h"
//
/*
 * Adjust the receive window for a channel by adjustment bytes. If the amount
 * to be adjusted is less than LIBSSH2_CHANNEL_MINADJUST and force is 0 the
 * adjustment amount will be queued for a later packet.
 *
 * Always non-blocking.
 */
int _libssh2_channel_receive_window_adjust(LIBSSH2_CHANNEL * channel, uint32 adjustment, uchar force, uint * store);
/*
 * Flush data from one (or all) stream
 * Returns number of bytes flushed, or negative on failure
 */
int _libssh2_channel_flush(LIBSSH2_CHANNEL * channel, int streamid);
/*
 * Make sure a channel is closed, then remove the channel from the session
 * and free its resource(s)
 *
 * Returns 0 on success, negative on failure
 */
int _libssh2_channel_free(LIBSSH2_CHANNEL * channel);
int _libssh2_channel_extended_data(LIBSSH2_CHANNEL * channel, int ignore_mode);
/*
 * Send data to a channel
 */
ssize_t _libssh2_channel_write(LIBSSH2_CHANNEL * channel, int stream_id, const uchar * buf, size_t buflen);
/*
 * Establish a generic session channel
 */
LIBSSH2_CHANNEL * _libssh2_channel_open(LIBSSH2_SESSION * session, const char * channel_type, uint32 channel_type_len,
    uint32 window_size, uint32 packet_size, const uchar * message, size_t message_len);
/*
 * Primitive for libssh2_channel_(shell|exec|subsystem)
 */
int _libssh2_channel_process_startup(LIBSSH2_CHANNEL * channel, const char * request, size_t request_len, const char * message, size_t message_len);
/*
 * Read data from a channel
 *
 * It is important to not return 0 until the currently read channel is
 * complete. If we read stuff from the wire but it was no payload data to fill
 * in the buffer with, we MUST make sure to return PACKET_EAGAIN.
 */
ssize_t _libssh2_channel_read(LIBSSH2_CHANNEL * channel, int stream_id, char * buf, size_t buflen);
uint32 _libssh2_channel_nextid(LIBSSH2_SESSION * session);
LIBSSH2_CHANNEL * _libssh2_channel_locate(LIBSSH2_SESSION * session, uint32 channel_id);
size_t _libssh2_channel_packet_data_len(LIBSSH2_CHANNEL * channel, int stream_id);
int _libssh2_channel_close(LIBSSH2_CHANNEL * channel);
/*
 * Stop listening on a remote port and free the listener
 * Toss out any pending (un-accept()ed) connections
 *
 * Return 0 on success, LIBSSH2_ERROR_EAGAIN if would block, -1 on error
 */
int _libssh2_channel_forward_cancel(LIBSSH2_LISTENER * listener);
//
//#include "packet.h"
//
//
int _libssh2_packet_read(LIBSSH2_SESSION * session);
int _libssh2_packet_ask(LIBSSH2_SESSION * session, uchar packet_type, uchar ** data, size_t * data_len,
    int match_ofs, const uchar * match_buf, size_t match_len);
int _libssh2_packet_askv(LIBSSH2_SESSION * session, const uchar * packet_types,
    uchar ** data, size_t * data_len, int match_ofs, const uchar * match_buf, size_t match_len);
int _libssh2_packet_require(LIBSSH2_SESSION * session, uchar packet_type, uchar ** data, size_t * data_len, int match_ofs,
    const uchar * match_buf, size_t match_len, packet_require_state_t * state);
int _libssh2_packet_requirev(LIBSSH2_SESSION * session, const uchar * packet_types,
    uchar ** data, size_t * data_len, int match_ofs, const uchar * match_buf, size_t match_len, packet_requirev_state_t * state);
int _libssh2_packet_burn(LIBSSH2_SESSION * session, libssh2_nonblocking_states * state);
int _libssh2_packet_write(LIBSSH2_SESSION * session, uchar * data, ulong data_len);
int _libssh2_packet_add(LIBSSH2_SESSION * session, uchar * data, size_t datalen, int macstate);
//
//#include "transport.h"
//
/*
 * libssh2_transport_send
 *
 * Send a packet, encrypting it and adding a MAC code if necessary
 * Returns 0 on success, non-zero on failure.
 *
 * The data is provided as _two_ data areas that are combined by this
 * function.  The 'data' part is sent immediately before 'data2'. 'data2' can
 * be set to NULL (or data2_len to 0) to only use a single part.
 *
 * Returns LIBSSH2_ERROR_EAGAIN if it would block or if the whole packet was
 * not sent yet. If it does so, the caller should call this function again as
 * soon as it is likely that more data can be sent, and this function MUST
 * then be called with the same argument set (same data pointer and same
 * data_len) until ERROR_NONE or failure is returned.
 *
 * This function DOES NOT call _libssh2_error() on any errors.
 */
int _libssh2_transport_send(LIBSSH2_SESSION *session, const uchar *data, size_t data_len, const uchar *data2, size_t data2_len);
/*
 * _libssh2_transport_read
 *
 * Collect a packet into the input brigade block only controls whether or not
 * to wait for a packet to start.
 *
 * Returns packet type added to input brigade (PACKET_NONE if nothing added),
 * or PACKET_FAIL on failure and PACKET_EAGAIN if it couldn't process a full
 * packet.
 */
/*
 * This function reads the binary stream as specified in chapter 6 of RFC4253
 * "The Secure Shell (SSH) Transport Layer Protocol"
 */
int _libssh2_transport_read(LIBSSH2_SESSION * session);
//
//#include "session.h"
//
// Conveniance-macros to allow code like this;
//
// int rc = BLOCK_ADJUST(rc, session, session_startup(session, sock) );
//
// int rc = BLOCK_ADJUST_ERRNO(ptr, session, session_startup(session, sock) );
//
// The point of course being to make sure that while in non-blocking mode
// these always return no matter what the return code is, but in blocking mode
// it blocks if EAGAIN is the reason for the return from the underlying function.
//
#define BLOCK_ADJUST(rc,sess,x) \
    do { \
       time_t entry_time = time(NULL); \
       do { \
          rc = x; \
          /* the order of the check below is important to properly deal with the case when the 'sess' is freed */ \
          if((rc != LIBSSH2_ERROR_EAGAIN) || !sess->api_block_mode) \
              break; \
          rc = _libssh2_wait_socket(sess, entry_time);  \
       } while(!rc);   \
    } while(0)
// 
// For functions that returns a pointer, we need to check if the API is
// non-blocking and return immediately. If the pointer is non-NULL we return
// immediately. If the API is blocking and we get a NULL we check the errno
// and *only* if that is EAGAIN we loop and wait for socket action.
// 
#define BLOCK_ADJUST_ERRNO(ptr,sess,x) \
    do { \
       time_t entry_time = time(NULL); \
       int rc; \
       do { \
           ptr = x; \
           if(!sess->api_block_mode || ptr || (libssh2_session_last_errno(sess) != LIBSSH2_ERROR_EAGAIN) ) \
               break; \
           rc = _libssh2_wait_socket(sess, entry_time); \
        } while(!rc); \
    } while(0)

int FASTCALL _libssh2_wait_socket(LIBSSH2_SESSION *session, time_t entry_time);
//
// this is the lib-internal set blocking function 
//
int _libssh2_session_set_blocking(LIBSSH2_SESSION * session, int blocking);
//
//#include "sftp.h"
//
// 
// MAX_SFTP_OUTGOING_SIZE MUST not be larger than 32500 or so. This is the
// amount of data sent in each FXP_WRITE packet
// 
#define MAX_SFTP_OUTGOING_SIZE 30000
#define MAX_SFTP_READ_SIZE     30000 // MAX_SFTP_READ_SIZE is how much data is asked for at max in each FXP_READ packets.

struct sftp_pipeline_chunk {
	struct list_node node;
	uint64 offset; /* READ: offset at which to start reading WRITE: not used */
	size_t len; /* WRITE: size of the data to write READ: how many bytes that was asked for */
	size_t sent;
	ssize_t lefttosend; /* if 0, the entire packet has been sent off */
	uint32 request_id;
	uchar packet[1]; /* data */
};

struct sftp_zombie_requests {
	struct list_node node;
	uint32 request_id;
};

//#ifndef MIN
//#define MIN(x, y) ((x)<(y) ? (x) : (y))
//#endif

struct _LIBSSH2_SFTP_PACKET {
	struct list_node node; /* linked list header */
	uint32 request_id;
	uchar * data;
	size_t data_len;          /* payload size */
};

typedef struct _LIBSSH2_SFTP_PACKET LIBSSH2_SFTP_PACKET;

#define SFTP_HANDLE_MAXLEN 256 // according to spec! 

struct _LIBSSH2_SFTP_HANDLE {
	struct list_node node;
	LIBSSH2_SFTP * sftp;
	char handle[SFTP_HANDLE_MAXLEN];
	size_t handle_len;
	enum {
		LIBSSH2_SFTP_HANDLE_FILE,
		LIBSSH2_SFTP_HANDLE_DIR
	} handle_type;

	union _libssh2_sftp_handle_data {
		struct _libssh2_sftp_handle_file_data {
			uint64 offset;
			uint64 offset_sent;
			size_t acked; // container for acked data that hasn't been returned to caller yet, used for sftp_write
			// 'data' is used by sftp_read() and is allocated data that has
			// been received already from the server but wasn't returned to
			// the caller yet. It is of size 'data_len' and 'data_left is the
			// number of bytes not yet returned, counted from the end of the buffer.
			uchar * data;
			size_t data_len;
			size_t data_left;
			char eof; /* we have read to the end */
		} file;
		struct _libssh2_sftp_handle_dir_data {
			uint32 names_left;
			void * names_packet;
			char * next_name;
		} dir;
	} u;
	libssh2_nonblocking_states close_state; /* State variables used in libssh2_sftp_close_handle() */
	uint32 close_request_id;
	uchar * close_packet;
	struct list_head packet_list; /* list of outstanding packets sent to server */
};

struct _LIBSSH2_SFTP {
	LIBSSH2_CHANNEL * channel;
	uint32 request_id, version;
	struct list_head packets;
	struct list_head zombie_requests; /* List of FXP_READ responses to ignore because EOF already received. */
	struct list_head sftp_handles; /* a list of _LIBSSH2_SFTP_HANDLE structs */
	uint32 last_errno;
	/* Holder for partial packet, use in libssh2_sftp_packet_read() */
	uchar partial_size[4];  /* buffer for size field   */
	size_t partial_size_len;        /* size field length       */
	uchar * partial_packet; /* The data                */
	uint32 partial_len;           /* Desired number of bytes */
	size_t partial_received;        /* Bytes received so far   */
	time_t requirev_start; /* Time that libssh2_sftp_packet_requirev() started reading */
	libssh2_nonblocking_states open_state; /* State variables used in libssh2_sftp_open_ex() */
	uchar * open_packet;
	uint32 open_packet_len; /* 32 bit on the wire */
	size_t open_packet_sent;
	uint32 open_request_id;
	libssh2_nonblocking_states read_state; /* State variable used in sftp_read() */
	libssh2_nonblocking_states packet_state; /* State variable used in sftp_packet_read() */
	libssh2_nonblocking_states write_state; /* State variable used in sftp_write() */
	libssh2_nonblocking_states fsync_state; /* State variables used in sftp_fsync() */
	uchar * fsync_packet;
	uint32 fsync_request_id;
	libssh2_nonblocking_states readdir_state; /* State variables used in libssh2_sftp_readdir() */
	uchar * readdir_packet;
	uint32 readdir_request_id;
	libssh2_nonblocking_states fstat_state; /* State variables used in libssh2_sftp_fstat_ex() */
	uchar * fstat_packet;
	uint32 fstat_request_id;
	libssh2_nonblocking_states unlink_state; /* State variables used in libssh2_sftp_unlink_ex() */
	uchar * unlink_packet;
	uint32 unlink_request_id;
	libssh2_nonblocking_states rename_state; /* State variables used in libssh2_sftp_rename_ex() */
	uchar * rename_packet;
	uchar * rename_s;
	uint32 rename_request_id;
	libssh2_nonblocking_states fstatvfs_state; /* State variables used in libssh2_sftp_fstatvfs() */
	uchar * fstatvfs_packet;
	uint32 fstatvfs_request_id;
	libssh2_nonblocking_states statvfs_state; /* State variables used in libssh2_sftp_statvfs() */
	uchar * statvfs_packet;
	uint32 statvfs_request_id;
	libssh2_nonblocking_states mkdir_state; /* State variables used in libssh2_sftp_mkdir() */
	uchar * mkdir_packet;
	uint32 mkdir_request_id;
	libssh2_nonblocking_states rmdir_state; /* State variables used in libssh2_sftp_rmdir() */
	uchar * rmdir_packet;
	uint32 rmdir_request_id;
	libssh2_nonblocking_states stat_state; /* State variables used in libssh2_sftp_stat() */
	uchar * stat_packet;
	uint32 stat_request_id;
	libssh2_nonblocking_states symlink_state; /* State variables used in libssh2_sftp_symlink() */
	uchar * symlink_packet;
	uint32 symlink_request_id;
};
//
//#include "userauth.h"
//
int _libssh2_userauth_publickey(LIBSSH2_SESSION *session, const char * username, uint username_len, const uchar *pubkeydata, ulong pubkeydata_len, LIBSSH2_USERAUTH_PUBLICKEY_SIGN_FUNC((*sign_callback)), void * abstract);
//
//#include "mac.h"
//
struct _LIBSSH2_MAC_METHOD {
	const char * name;
	int mac_len; // The length of a given MAC packet 
	int key_len; // integrity key length 
	// Message Authentication Code Hashing algo 
	int (* init)(LIBSSH2_SESSION * session, uchar * key, int * free_key, void ** abstract);
	int (* hash)(LIBSSH2_SESSION * session, uchar * buf, uint32 seqno, const uchar * packet, uint32 packet_len, const uchar * addtl, uint32 addtl_len, void ** abstract);
	int (* dtor)(LIBSSH2_SESSION * session, void ** abstract);
};

typedef struct _LIBSSH2_MAC_METHOD LIBSSH2_MAC_METHOD;

const LIBSSH2_MAC_METHOD ** _libssh2_mac_methods(void);
//
#endif /* LIBSSH2_H */
