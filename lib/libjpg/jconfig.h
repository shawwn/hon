/* jconfig.vc --- jconfig.h for Microsoft Visual C++ on Windows 95 or NT. */
/* see jconfig.doc for explanations */

#define USE_SECURE_CRT_FUNCTIONS
#ifdef USE_SECURE_CRT_FUNCTIONS
#define MAX_GETENV_S_LEN			1024
#define GETENV_S_PREP(buffer)		size_t __getenv_s_return_size__; (buffer) = malloc(MAX_GETENV_S_LEN);
#define GETENV_S(buffer, var)		(getenv_s(&__getenv_s_return_size__, (buffer), MAX_GETENV_S_LEN, var) == 0)
#define GETENV_S_CLEANUP(buffer)	if ((buffer) != NULL) free(buffer);
#define SSCANF_S					sscanf_s
#define SPRINTF_S					sprintf_s
#define SPRINTF_S_SIZE(size)		(size),
#else
#define GETENV_S_PREP(buffer)
#define GETENV_S(buffer, var)		((buffer = getenv(var)) != NULL)
#define GETENV_S_CLEANUP(buffer)
#define SSCANF_S	sscanf
#define SPRINTF_S	sprintf
#define SPRINTF_S_SIZE(size)
#endif

#define NO_GETENV
#define HAVE_PROTOTYPES
#define HAVE_UNSIGNED_CHAR
#define HAVE_UNSIGNED_SHORT
/* #define void char */
/* #define const */
#undef CHAR_IS_UNSIGNED
#define HAVE_STDDEF_H
#define HAVE_STDLIB_H
#undef NEED_BSD_STRINGS
#undef NEED_SYS_TYPES_H
#undef NEED_FAR_POINTERS	/* we presume a 32-bit flat memory model */
#undef NEED_SHORT_EXTERNAL_NAMES
#undef INCOMPLETE_TYPES_BROKEN

/* Define "boolean" as unsigned char, not int, per Windows custom */
#ifndef __RPCNDR_H__		/* don't conflict if rpcndr.h already read */
typedef unsigned char boolean;
#endif
#define HAVE_BOOLEAN		/* prevent jmorecfg.h from redefining it */


#ifdef JPEG_INTERNALS

#undef RIGHT_SHIFT_IS_UNSIGNED

#endif /* JPEG_INTERNALS */

#ifdef JPEG_CJPEG_DJPEG

#define BMP_SUPPORTED		/* BMP image file format */
#define GIF_SUPPORTED		/* GIF image file format */
#define PPM_SUPPORTED		/* PBMPLUS PPM/PGM image file format */
#undef RLE_SUPPORTED		/* Utah RLE image file format */
#define TARGA_SUPPORTED		/* Targa image file format */

#define TWO_FILE_COMMANDLINE	/* optional */
#define USE_SETMODE		/* Microsoft has setmode() */
#undef NEED_SIGNAL_CATCHER
#undef DONT_USE_B_MODE
#undef PROGRESS_REPORT		/* optional */

#endif /* JPEG_CJPEG_DJPEG */
