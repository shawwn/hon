/*****************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * $Id: lib554.c,v 1.1 2009/02/28 06:59:33 Ikkyo Exp $
 */

#include "test.h"

#include "memdebug.h"

static char data[]="this is what we post to the silly web server\n";

struct WriteThis {
  char *readptr;
  size_t sizeleft;
};

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct WriteThis *pooh = (struct WriteThis *)userp;

  if(size*nmemb < 1)
    return 0;

  if(pooh->sizeleft) {
    *(char *)ptr = pooh->readptr[0]; /* copy one single byte */
    pooh->readptr++;                 /* advance pointer */
    pooh->sizeleft--;                /* less data left */
    return 1;                        /* we return 1 byte at a time! */
  }

  return 0;                         /* no more data left to deliver */
}

int test(char *URL)
{
  CURL *curl;
  CURLcode res=CURLE_OK;
  CURLFORMcode formrc;

  struct curl_httppost *formpost=NULL;
  struct curl_httppost *lastptr=NULL;
  struct WriteThis pooh;

  if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
    fprintf(stderr, "curl_global_init() failed\n");
    return TEST_ERR_MAJOR_BAD;
  }

  pooh.readptr = data;
  pooh.sizeleft = strlen(data);

  /* Fill in the file upload field */
  formrc = curl_formadd(&formpost,
                        &lastptr,
                        CURLFORM_COPYNAME, "sendfile",
                        CURLFORM_STREAM, &pooh,
                        CURLFORM_CONTENTSLENGTH, pooh.sizeleft,
                        CURLFORM_FILENAME, "postit2.c",
                        CURLFORM_END);

  if(formrc)
    printf("curl_formadd(1) = %d\n", (int)formrc);

  /* Fill in the filename field */
  formrc = curl_formadd(&formpost,
                        &lastptr,
                        CURLFORM_COPYNAME, "filename",
                        CURLFORM_COPYCONTENTS, "postit2.c",
                        CURLFORM_END);

  if(formrc)
    printf("curl_formadd(2) = %d\n", (int)formrc);

  /* Fill in a submit field too */
  formrc = curl_formadd(&formpost,
                        &lastptr,
                        CURLFORM_COPYNAME, "submit",
                        CURLFORM_COPYCONTENTS, "send",
                        CURLFORM_END);

  if(formrc)
    printf("curl_formadd(3) = %d\n", (int)formrc);

  if ((curl = curl_easy_init()) == NULL) {
    fprintf(stderr, "curl_easy_init() failed\n");
    curl_formfree(formpost);
    curl_global_cleanup();
    return TEST_ERR_MAJOR_BAD;
  }

  /* First set the URL that is about to receive our POST. */
  curl_easy_setopt(curl, CURLOPT_URL, URL);

  /* Now specify we want to POST data */
  curl_easy_setopt(curl, CURLOPT_POST, 1L);

  /* Set the expected POST size */
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)pooh.sizeleft);

  /* we want to use our own read function */
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

  /* pointer to pass to our read function */
  curl_easy_setopt(curl, CURLOPT_READDATA, &pooh);

  /* send a multi-part formpost */
  curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

  /* get verbose debug output please */
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

  /* include headers in the output */
  curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

  /* Perform the request, res will get the return code */
  res = curl_easy_perform(curl);

  /* always cleanup */
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  /* now cleanup the formpost chain */
  curl_formfree(formpost);

  return res;
}
