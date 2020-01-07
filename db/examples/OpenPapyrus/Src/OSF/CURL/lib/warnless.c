/***************************************************************************
*                                  _   _ ____  _
*  Project                     ___| | | |  _ \| |
*                             / __| | | | |_) | |
*                            | (__| |_| |  _ <| |___
*                             \___|\___/|_| \_\_____|
*
* Copyright (C) 1998 - 2016, Daniel Stenberg, <daniel@haxx.se>, et al.
*
* This software is licensed as described in the file COPYING, which
* you should have received as part of this distribution. The terms
* are also available at https://curl.haxx.se/docs/copyright.html.
*
* You may opt to use, copy, modify, merge, publish, distribute and/or sell
* copies of the Software, and permit persons to whom the Software is
* furnished to do so, under the terms of the COPYING file.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
*
***************************************************************************/

#include "curl_setup.h"
#pragma hdrstop
#if defined(__INTEL_COMPILER) && defined(__unix__)

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#endif /* __INTEL_COMPILER && __unix__ */

#define BUILDING_WARNLESS_C 1

//#include "warnless.h"

#define CURL_MASK_SCHAR  0x7F
#define CURL_MASK_UCHAR  0xFF

#if (SIZEOF_SHORT == 2)
	#define CURL_MASK_SSHORT  0x7FFF
	#define CURL_MASK_USHORT  0xFFFF
#elif (SIZEOF_SHORT == 4)
	#define CURL_MASK_SSHORT  0x7FFFFFFF
	#define CURL_MASK_USHORT  0xFFFFFFFF
#elif (SIZEOF_SHORT == 8)
	#define CURL_MASK_SSHORT  0x7FFFFFFFFFFFFFFF
	#define CURL_MASK_USHORT  0xFFFFFFFFFFFFFFFF
#else
	#error "SIZEOF_SHORT not defined"
#endif

#if (SIZEOF_INT == 2)
#define CURL_MASK_SINT  0x7FFF
#define CURL_MASK_UINT  0xFFFF
#elif (SIZEOF_INT == 4)
#define CURL_MASK_SINT  0x7FFFFFFF
#define CURL_MASK_UINT  0xFFFFFFFF
#elif (SIZEOF_INT == 8)
#define CURL_MASK_SINT  0x7FFFFFFFFFFFFFFF
#define CURL_MASK_UINT  0xFFFFFFFFFFFFFFFF
#elif (SIZEOF_INT == 16)
#define CURL_MASK_SINT  0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
#define CURL_MASK_UINT  0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
#else
#  error "SIZEOF_INT not defined"
#endif

#if (CURL_SIZEOF_LONG == 2)
#define CURL_MASK_SLONG  0x7FFFL
#define CURL_MASK_ULONG  0xFFFFUL
#elif (CURL_SIZEOF_LONG == 4)
#define CURL_MASK_SLONG  0x7FFFFFFFL
#define CURL_MASK_ULONG  0xFFFFFFFFUL
#elif (CURL_SIZEOF_LONG == 8)
#define CURL_MASK_SLONG  0x7FFFFFFFFFFFFFFFL
#define CURL_MASK_ULONG  0xFFFFFFFFFFFFFFFFUL
#elif (CURL_SIZEOF_LONG == 16)
#define CURL_MASK_SLONG  0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFL
#define CURL_MASK_ULONG  0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFUL
#else
#  error "CURL_SIZEOF_LONG not defined"
#endif

#if (CURL_SIZEOF_CURL_OFF_T == 2)
#define CURL_MASK_SCOFFT  CURL_OFF_T_C(0x7FFF)
#define CURL_MASK_UCOFFT  CURL_OFF_TU_C(0xFFFF)
#elif (CURL_SIZEOF_CURL_OFF_T == 4)
#define CURL_MASK_SCOFFT  CURL_OFF_T_C(0x7FFFFFFF)
#define CURL_MASK_UCOFFT  CURL_OFF_TU_C(0xFFFFFFFF)
#elif (CURL_SIZEOF_CURL_OFF_T == 8)
#define CURL_MASK_SCOFFT  CURL_OFF_T_C(0x7FFFFFFFFFFFFFFF)
#define CURL_MASK_UCOFFT  CURL_OFF_TU_C(0xFFFFFFFFFFFFFFFF)
#elif (CURL_SIZEOF_CURL_OFF_T == 16)
#define CURL_MASK_SCOFFT  CURL_OFF_T_C(0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
#define CURL_MASK_UCOFFT  CURL_OFF_TU_C(0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
#else
#  error "CURL_SIZEOF_CURL_OFF_T not defined"
#endif

#if (SIZEOF_SIZE_T == SIZEOF_SHORT)
#define CURL_MASK_SSIZE_T  CURL_MASK_SSHORT
#define CURL_MASK_USIZE_T  CURL_MASK_USHORT
#elif (SIZEOF_SIZE_T == SIZEOF_INT)
#define CURL_MASK_SSIZE_T  CURL_MASK_SINT
#define CURL_MASK_USIZE_T  CURL_MASK_UINT
#elif (SIZEOF_SIZE_T == CURL_SIZEOF_LONG)
#define CURL_MASK_SSIZE_T  CURL_MASK_SLONG
#define CURL_MASK_USIZE_T  CURL_MASK_ULONG
#elif (SIZEOF_SIZE_T == CURL_SIZEOF_CURL_OFF_T)
#define CURL_MASK_SSIZE_T  CURL_MASK_SCOFFT
#define CURL_MASK_USIZE_T  CURL_MASK_UCOFFT
#else
#  error "SIZEOF_SIZE_T not defined"
#endif
/*
** ulong to ushort
*/
ushort FASTCALL curlx_ultous(ulong ulnum)
{
#ifdef __INTEL_COMPILER
	#pragma warning(push)
	#pragma warning(disable:810) /* conversion may lose significant bits */
#endif
	DEBUGASSERT(ulnum <= (ulong)CURL_MASK_USHORT);
	return (ushort)(ulnum & (ulong)CURL_MASK_USHORT);
#ifdef __INTEL_COMPILER
	#pragma warning(pop)
#endif
}
/*
** ulong to uchar
*/
uchar FASTCALL curlx_ultouc(ulong ulnum)
{
#ifdef __INTEL_COMPILER
	#pragma warning(push)
	#pragma warning(disable:810) /* conversion may lose significant bits */
#endif
	DEBUGASSERT(ulnum <= (ulong)CURL_MASK_UCHAR);
	return (uchar)(ulnum & (ulong)CURL_MASK_UCHAR);
#ifdef __INTEL_COMPILER
	#pragma warning(pop)
#endif
}
/*
** ulong to signed int
*/
int FASTCALL curlx_ultosi(ulong ulnum)
{
#ifdef __INTEL_COMPILER
	#pragma warning(push)
	#pragma warning(disable:810) /* conversion may lose significant bits */
#endif
	DEBUGASSERT(ulnum <= (ulong)CURL_MASK_SINT);
	return (int)(ulnum & (ulong)CURL_MASK_SINT);
#ifdef __INTEL_COMPILER
	#pragma warning(pop)
#endif
}
/*
** unsigned size_t to signed curl_off_t
*/
curl_off_t FASTCALL curlx_uztoso(size_t uznum)
{
#ifdef __INTEL_COMPILER
	#pragma warning(push)
	#pragma warning(disable:810) /* conversion may lose significant bits */
#elif defined(_MSC_VER)
	#pragma warning(push)
	#pragma warning(disable:4310) /* cast truncates constant value */
#endif
	DEBUGASSERT(uznum <= (size_t)CURL_MASK_SCOFFT);
	return (curl_off_t)(uznum & (size_t)CURL_MASK_SCOFFT);
#if defined(__INTEL_COMPILER) || defined(_MSC_VER)
	#pragma warning(pop)
#endif
}
/*
** unsigned size_t to signed int
*/
int FASTCALL curlx_uztosi(size_t uznum)
{
#ifdef __INTEL_COMPILER
	#pragma warning(push)
	#pragma warning(disable:810) /* conversion may lose significant bits */
#endif
	DEBUGASSERT(uznum <= (size_t)CURL_MASK_SINT);
	return (int)(uznum & (size_t)CURL_MASK_SINT);
#ifdef __INTEL_COMPILER
	#pragma warning(pop)
#endif
}
/*
** unsigned size_t to ulong
*/
ulong FASTCALL curlx_uztoul(size_t uznum)
{
#ifdef __INTEL_COMPILER
	#pragma warning(push)
	#pragma warning(disable:810) /* conversion may lose significant bits */
#endif
#if (CURL_SIZEOF_LONG < SIZEOF_SIZE_T)
	DEBUGASSERT(uznum <= (size_t)CURL_MASK_ULONG);
#endif
	return (ulong)(uznum & (size_t)CURL_MASK_ULONG);
#ifdef __INTEL_COMPILER
	#pragma warning(pop)
#endif
}
/*
** unsigned size_t to uint
*/
uint FASTCALL curlx_uztoui(size_t uznum)
{
#ifdef __INTEL_COMPILER
	#pragma warning(push)
	#pragma warning(disable:810) /* conversion may lose significant bits */
#endif
#if (SIZEOF_INT < SIZEOF_SIZE_T)
	DEBUGASSERT(uznum <= (size_t)CURL_MASK_UINT);
#endif
	return (uint)(uznum & (size_t)CURL_MASK_UINT);
#ifdef __INTEL_COMPILER
	#pragma warning(pop)
#endif
}
/*
** signed long to signed int
*/
int FASTCALL curlx_sltosi(long slnum)
{
#ifdef __INTEL_COMPILER
	#pragma warning(push)
	#pragma warning(disable:810) /* conversion may lose significant bits */
#endif
	DEBUGASSERT(slnum >= 0);
#if (SIZEOF_INT < CURL_SIZEOF_LONG)
	DEBUGASSERT((ulong)slnum <= (ulong)CURL_MASK_SINT);
#endif
	return (int)(slnum & (long)CURL_MASK_SINT);
#ifdef __INTEL_COMPILER
	#pragma warning(pop)
#endif
}
/*
** signed long to uint
*/
uint FASTCALL curlx_sltoui(long slnum)
{
#ifdef __INTEL_COMPILER
	#pragma warning(push)
	#pragma warning(disable:810) /* conversion may lose significant bits */
#endif
	DEBUGASSERT(slnum >= 0);
#if (SIZEOF_INT < CURL_SIZEOF_LONG)
	DEBUGASSERT((ulong)slnum <= (ulong)CURL_MASK_UINT);
#endif
	return (uint)(slnum & (long)CURL_MASK_UINT);
#ifdef __INTEL_COMPILER
	#pragma warning(pop)
#endif
}
/*
** signed long to ushort
*/
ushort FASTCALL curlx_sltous(long slnum)
{
#ifdef __INTEL_COMPILER
	#pragma warning(push)
	#pragma warning(disable:810) /* conversion may lose significant bits */
#endif
	DEBUGASSERT(slnum >= 0);
	DEBUGASSERT((ulong)slnum <= (ulong)CURL_MASK_USHORT);
	return (ushort)(slnum & (long)CURL_MASK_USHORT);
#ifdef __INTEL_COMPILER
	#pragma warning(pop)
#endif
}
/*
** unsigned size_t to signed ssize_t
*/
ssize_t FASTCALL curlx_uztosz(size_t uznum)
{
#ifdef __INTEL_COMPILER
	#pragma warning(push)
	#pragma warning(disable:810) /* conversion may lose significant bits */
#endif
	DEBUGASSERT(uznum <= (size_t)CURL_MASK_SSIZE_T);
	return (ssize_t)(uznum & (size_t)CURL_MASK_SSIZE_T);
#ifdef __INTEL_COMPILER
	#pragma warning(pop)
#endif
}
/*
** signed curl_off_t to unsigned size_t
*/
size_t FASTCALL curlx_sotouz(curl_off_t sonum)
{
#ifdef __INTEL_COMPILER
	#pragma warning(push)
	#pragma warning(disable:810) /* conversion may lose significant bits */
#endif
	DEBUGASSERT(sonum >= 0);
	return (size_t)(sonum & (curl_off_t)CURL_MASK_USIZE_T);
#ifdef __INTEL_COMPILER
	#pragma warning(pop)
#endif
}
/*
** signed ssize_t to signed int
*/
int FASTCALL curlx_sztosi(ssize_t sznum)
{
#ifdef __INTEL_COMPILER
	#pragma warning(push)
	#pragma warning(disable:810) /* conversion may lose significant bits */
#endif
	DEBUGASSERT(sznum >= 0);
#if (SIZEOF_INT < SIZEOF_SIZE_T)
	DEBUGASSERT((size_t)sznum <= (size_t)CURL_MASK_SINT);
#endif
	return (int)(sznum & (ssize_t)CURL_MASK_SINT);
#ifdef __INTEL_COMPILER
	#pragma warning(pop)
#endif
}
/*
** uint to ushort
*/
ushort FASTCALL curlx_uitous(uint uinum)
{
#ifdef __INTEL_COMPILER
	#pragma warning(push)
	#pragma warning(disable:810) /* conversion may lose significant bits */
#endif
	DEBUGASSERT(uinum <= (uint)CURL_MASK_USHORT);
	return (ushort)(uinum & (uint)CURL_MASK_USHORT);
#ifdef __INTEL_COMPILER
	#pragma warning(pop)
#endif
}
/*
** uint to uchar
*/
uchar FASTCALL curlx_uitouc(uint uinum)
{
#ifdef __INTEL_COMPILER
	#pragma warning(push)
	#pragma warning(disable:810) /* conversion may lose significant bits */
#endif
	DEBUGASSERT(uinum <= (uint)CURL_MASK_UCHAR);
	return (uchar)(uinum & (uint)CURL_MASK_UCHAR);
#ifdef __INTEL_COMPILER
	#pragma warning(pop)
#endif
}
/*
** uint to signed int
*/
int FASTCALL curlx_uitosi(uint uinum)
{
#ifdef __INTEL_COMPILER
	#pragma warning(push)
	#pragma warning(disable:810) /* conversion may lose significant bits */
#endif
	DEBUGASSERT(uinum <= (uint)CURL_MASK_SINT);
	return (int)(uinum & (uint)CURL_MASK_SINT);
#ifdef __INTEL_COMPILER
	#pragma warning(pop)
#endif
}
/*
** signed int to unsigned size_t
*/
size_t FASTCALL curlx_sitouz(int sinum)
{
#ifdef __INTEL_COMPILER
	#pragma warning(push)
	#pragma warning(disable:810) /* conversion may lose significant bits */
#endif
	DEBUGASSERT(sinum >= 0);
	return (size_t)sinum;
#ifdef __INTEL_COMPILER
	# pragma warning(pop)
#endif
}

#ifdef USE_WINSOCK
/*
** curl_socket_t to signed int
*/
int FASTCALL curlx_sktosi(curl_socket_t s)
{
	return (int)((ssize_t)s);
}
/*
** signed int to curl_socket_t
*/
curl_socket_t FASTCALL curlx_sitosk(int i)
{
	return (curl_socket_t)((ssize_t)i);
}
#endif /* USE_WINSOCK */

#if defined(WIN32) || defined(_WIN32)

ssize_t curlx_read(int fd, void * buf, size_t count)
{
	return (ssize_t)_read(fd, buf, curlx_uztoui(count));
}

ssize_t curlx_write(int fd, const void * buf, size_t count)
{
	return (ssize_t)_write(fd, buf, curlx_uztoui(count));
}

#endif /* WIN32 || _WIN32 */

#if defined(__INTEL_COMPILER) && defined(__unix__)

int FASTCALL curlx_FD_ISSET(int fd, fd_set * fdset)
{
#pragma warning(push)
#pragma warning(disable:1469) /* clobber ignored */
	return FD_ISSET(fd, fdset);
#pragma warning(pop)
}

void FASTCALL curlx_FD_SET(int fd, fd_set * fdset)
{
  #pragma warning(push)
  #pragma warning(disable:1469) /* clobber ignored */
	FD_SET(fd, fdset);
  #pragma warning(pop)
}

void FASTCALL curlx_FD_ZERO(fd_set * fdset)
{
  #pragma warning(push)
  #pragma warning(disable:593) /* variable was set but never used */
	FD_ZERO(fdset);
  #pragma warning(pop)
}

ushort FASTCALL curlx_htons(ushort usnum)
{
#if (__INTEL_COMPILER == 910) && defined(__i386__)
	return (ushort)(((usnum << 8) & 0xFF00) | ((usnum >> 8) & 0x00FF));
#else
  #pragma warning(push)
  #pragma warning(disable:810) /* conversion may lose significant bits */
	return htons(usnum);
  #pragma warning(pop)
#endif
}

ushort FASTCALL curlx_ntohs(ushort usnum)
{
#if (__INTEL_COMPILER == 910) && defined(__i386__)
	return (ushort)(((usnum << 8) & 0xFF00) | ((usnum >> 8) & 0x00FF));
#else
  #pragma warning(push)
  #pragma warning(disable:810) /* conversion may lose significant bits */
	return ntohs(usnum);
  #pragma warning(pop)
#endif
}

#endif /* __INTEL_COMPILER && __unix__ */
