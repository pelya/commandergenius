#ifndef ICONV_H
#define ICONV_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include <stdlib.h>


typedef int iconv_t;

extern iconv_t iconv_open(const char *tocode, const char *fromcode);

extern size_t iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
		char **outbuf, size_t *outbytesleft);
extern int iconv_close(iconv_t);

#ifdef __cplusplus
 }
#endif 

#endif
