/*****************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * $Id: simplepost.c,v 1.1 2010/03/24 02:52:31 Ikkyo Exp $
 */

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

int main(void)
{
  CURL *curl;
  CURLcode res;

  static const char *postthis="moo mooo moo moo";

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://posthere.com");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postthis);

    /* if we don't provide POSTFIELDSIZE, libcurl will strlen() by
       itself */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(postthis));

    res = curl_easy_perform(curl);

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  return 0;
}
