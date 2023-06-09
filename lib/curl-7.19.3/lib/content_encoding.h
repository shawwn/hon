#ifndef __CURL_CONTENT_ENCODING_H
#define __CURL_CONTENT_ENCODING_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2008, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * $Id: content_encoding.h,v 1.1 2009/02/28 06:59:18 Ikkyo Exp $
 ***************************************************************************/
#include "setup.h"

/*
 * Comma-separated list all supported Content-Encodings ('identity' is implied)
 */
#ifdef HAVE_LIBZ
#define ALL_CONTENT_ENCODINGS "deflate, gzip"
#else
#define ALL_CONTENT_ENCODINGS "identity"
#endif

CURLcode Curl_unencode_deflate_write(struct connectdata *conn,
                                     struct SingleRequest *req,
                                     ssize_t nread);

CURLcode
Curl_unencode_gzip_write(struct connectdata *conn,
                         struct SingleRequest *k,
                         ssize_t nread);

#endif
