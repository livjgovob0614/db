/***************************************************************************
*                                  _   _ ____  _
*  Project                     ___| | | |  _ \| |
*                             / __| | | | |_) | |
*                            | (__| |_| |  _ <| |___
*                             \___|\___/|_| \_\_____|
*
* Copyright (C) 1998 - 2017, Daniel Stenberg, <daniel@haxx.se>, et al.
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
// The last #include files should be in this order 
#include "curl_printf.h"
#include "memdebug.h"

CURLcode Curl_wildcard_init(struct WildcardData * wc)
{
	Curl_llist_init(&wc->filelist, Curl_fileinfo_dtor);
	wc->state = CURLWC_INIT;
	return CURLE_OK;
}

void Curl_wildcard_dtor(struct WildcardData * wc)
{
	if(wc) {
		if(wc->tmp_dtor) {
			wc->tmp_dtor(wc->tmp);
			wc->tmp_dtor = ZERO_NULL;
			wc->tmp = NULL;
		}
		DEBUGASSERT(wc->tmp == NULL);
		Curl_llist_destroy(&wc->filelist, 0);
		SAlloc::F(wc->path);
		wc->path = NULL;
		SAlloc::F(wc->pattern);
		wc->pattern = NULL;
		wc->customptr = NULL;
		wc->state = CURLWC_INIT;
	}
}

