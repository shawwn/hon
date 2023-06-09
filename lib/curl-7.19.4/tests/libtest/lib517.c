/*****************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * $Id: lib517.c,v 1.1 2009/03/18 18:44:52 Ikkyo Exp $
 */

#include "test.h"

#include "memdebug.h"

static const char *dates[]={
  "Sun, 06 Nov 1994 08:49:37 GMT",
  "Sunday, 06-Nov-94 08:49:37 GMT",
  "Sun Nov  6 08:49:37 1994",
  "06 Nov 1994 08:49:37 GMT",
  "06-Nov-94 08:49:37 GMT",
  "Nov  6 08:49:37 1994",
  "06 Nov 1994 08:49:37",
  "06-Nov-94 08:49:37",
  "1994 Nov 6 08:49:37",
  "GMT 08:49:37 06-Nov-94 Sunday",
  "94 6 Nov 08:49:37",
  "1994 Nov 6",
  "06-Nov-94",
  "Sun Nov 6 94",
  "1994.Nov.6",
  "Sun/Nov/6/94/GMT",
  "Sun, 06 Nov 1994 08:49:37 CET",
  "06 Nov 1994 08:49:37 EST",
  "Sun, 12 Sep 2004 15:05:58 -0700",
  "Sat, 11 Sep 2004 21:32:11 +0200",
  "20040912 15:05:58 -0700",
  "20040911 +0200",
  "Thu, 01-Jan-1970 00:59:59 GMT",
  "Thu, 01-Jan-1970 01:00:00 GMT",
/*  "2094 Nov 6", See ../data/test517 for details */
  NULL
};

int test(char *URL)
{
  int i;

  (void)URL; /* not used */

  for(i=0; dates[i]; i++) {
    printf("%d: %s => %ld\n", i, dates[i], (long)curl_getdate(dates[i], NULL));
  }

  return 0;
}
