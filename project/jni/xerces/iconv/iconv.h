// SDL_iconv >> iconv wrapper dummy
#ifndef DUMMY_ICONV_H
#define DUMMY_ICONV_H

#undef HAVE_ICONV
#ifdef __cplusplus
extern "C" {
#endif

#include <SDL.h>
#define iconv_t SDL_iconv_t
#define iconv SDL_iconv
#define iconv_open SDL_iconv_open
#define iconv_close SDL_iconv_close

#ifdef __cplusplus
}
#endif

#define HAVE_ICONV 1

#endif
