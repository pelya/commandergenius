#ifndef CONFIG_H
#define CONFIG_H

#define PACKAGE_NAME "supertux2"

/* #undef ENABLE_SQDBG */

/* #undef ENABLE_BINRELOC */
#define INSTALL_SUBDIR_BIN "games"
#define INSTALL_SUBDIR_SHARE "share/games/supertux2"

#ifdef __LP64__
#define SIZEOF_VOIDP 8
#else
#define SIZEOF_VOIDP 4
#endif
#if SIZEOF_VOID_P == 8
#define _SQ64
#endif

/* #undef HAVE_ICONV_CONST */
#ifdef HAVE_ICONV_CONST
#define ICONV_CONST const
#else
#define ICONV_CONST
#endif

/* #undef WORDS_BIGENDIAN */

#define HAVE_OPENGL

#define HAVE_LIBCURL

#define BUILD_DATA_DIR "data"

#define BUILD_CONFIG_DATA_DIR "data"

/* #undef ENABLE_DISCORD */

/* #undef UBUNTU_TOUCH */
#define ENABLE_TOUCHSCREEN_SUPPORT

/* #undef REMOVE_QUIT_BUTTON */

#endif /*CONFIG_H*/
