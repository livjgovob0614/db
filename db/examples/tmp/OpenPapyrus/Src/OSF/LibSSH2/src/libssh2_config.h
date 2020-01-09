#ifndef LIBSSH2_CONFIG_H
#define LIBSSH2_CONFIG_H
//
#define LIBSSH2_WIN32
#define LIBSSH2_OPENSSL
//
// @sobolev #ifndef WIN32
	// @sobolev #define WIN32
// @sobolev #endif
#ifndef _CRT_SECURE_NO_DEPRECATE
	#define _CRT_SECURE_NO_DEPRECATE 1
#endif /* _CRT_SECURE_NO_DEPRECATE */
// @sobolev #include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#ifdef __MINGW32__
	#define HAVE_UNISTD_H
	#define HAVE_INTTYPES_H
	#define HAVE_SYS_TIME_H
	#define HAVE_GETTIMEOFDAY
#endif /* __MINGW32__ */
#define HAVE_LIBCRYPT32
#define HAVE_WINSOCK2_H
#define HAVE_IOCTLSOCKET
#define HAVE_SELECT
#ifdef _MSC_VER
	#if _MSC_VER < 1900
		/* @sobolev 
		#define snprintf _snprintf
		#if _MSC_VER < 1500
			#define vsnprintf _vsnprintf
		#endif
		#define strdup _strdup
		#define strncasecmp _strnicmp
		#define strcasecmp _stricmp
		*/
	#endif
#else
	#ifndef __MINGW32__
		#define strncasecmp strnicmp
		#define strcasecmp stricmp
	#endif /* __MINGW32__ */
#endif /* _MSC_VER */
#define LIBSSH2_DH_GEX_NEW 1 // Enable newer diffie-hellman-group-exchange-sha1 syntax 
#define LIBSSH2_HAVE_ZLIB    // @sobolev
// #define LIBSSH2_WINCNG 

#endif /* LIBSSH2_CONFIG_H */

