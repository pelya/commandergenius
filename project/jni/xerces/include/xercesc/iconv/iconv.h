// iconv wrapper
#ifndef XERCES_DUMMY_ICONV_H
#define XERCES_DUMMY_ICONV_H

#undef HAVE_ICONV
#ifdef __cplusplus
extern "C" {
#endif

#define XERCES_ICONV_ERROR		(size_t)-1
#define XERCES_ICONV_E2BIG		(size_t)-2
#define XERCES_ICONV_EILSEQ		(size_t)-3
#define XERCES_ICONV_EINVAL		(size_t)-4
/*@}*/


#define XERCES_iconv_utf8_locale(S)	XERCES_iconv_string("", "UTF-8", S, strlen(S)+1)
#define XERCES_iconv_utf8_ucs2(S)		(Uint16 *)XERCES_iconv_string("UCS-2", "UTF-8", S, strlen(S)+1)
#define XERCES_iconv_utf8_ucs4(S)		(Uint32 *)XERCES_iconv_string("UCS-4", "UTF-8", S, strlen(S)+1)



typedef struct _XERCES_iconv_t *XERCES_iconv_t;

extern XERCES_iconv_t XERCES_iconv_open(const char *tocode, const char *fromcode);
extern int XERCES_iconv_close(XERCES_iconv_t cd);
extern size_t XERCES_iconv(XERCES_iconv_t cd, const char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft);
extern char * XERCES_iconv_string(const char *tocode, const char *fromcode, const char *inbuf, size_t inbytesleft);

#define iconv_t XERCES_iconv_t
#define iconv XERCES_iconv
#define iconv_open XERCES_iconv_open
#define iconv_close XERCES_iconv_close

#ifdef __cplusplus
}
#endif

#define HAVE_ICONV 1

#endif
