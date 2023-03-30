
/************ (C) Copyright 2002 Valve, L.L.C. All rights reserved. ***********
**
** The copyright to the contents herein is the property of Valve, L.L.C.
** The contents may be used and/or copied only with the written permission of
** Valve, L.L.C., or in accordance with the terms and conditions stipulated in
** the agreement/contract under which the contents have been supplied.
**
*******************************************************************************
**
** Contents:
**
**		This file provides a PARTIAL wrapper for the Steam Client Interface
**      provided by Steam.h.  It provides a C-library-like interface
**      with functions such as fopen(), fread(), findnext(), etc. 
**
**		It is intended to help minimize difficulty in porting existing
**      applications to Steam. For new applications, it is recommended you
**      use the Steam.h interface directly.
**
** Target restrictions:
**
** Tool restrictions:
**
******************************************************************************/

#ifndef INCLUDED_STEAMLIBAPI_H
#define INCLUDED_STEAMLIBAPI_H

#if defined(_MSC_VER) && (_MSC_VER > 1000)
#pragma once
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum					// Filter elements returned by SteamFind{First,Next}
{
	STEAMCLibFindLocalOnly,			// limit search to local filesystem
	STEAMCLibFindRemoteOnly,			// limit search to remote repository
	STEAMCLibFindAll					// do not limit search (duplicates allowed)
} STEAMCLibFindFilter;			// redundant with Win32 version but oh well

typedef struct
{
	int uUnused;
} STEAM_FILE;

extern STEAM_FILE *	STEAM_fopen(const char *filename, const char *options);
extern int			STEAM_fclose(STEAM_FILE *file);
extern unsigned int	STEAM_fread(void *buffer, unsigned int rdsize, unsigned int count,STEAM_FILE *file);
extern int			STEAM_fgetc(STEAM_FILE *file);
extern unsigned int	STEAM_fwrite(const void *buffer, unsigned int rdsize, unsigned int count,STEAM_FILE *file);
extern int			STEAM_fprintf( STEAM_FILE *fp, const char *format, ... );
extern int			STEAM_vfprintf( STEAM_FILE *fp, const char *format, va_list argptr);
extern int			STEAM_fseek(STEAM_FILE *file, long offset, int method);
extern long			STEAM_ftell(STEAM_FILE *file);
extern int			STEAM_stat(const char *path, struct _stat *buf);
extern int			STEAM_feof( STEAM_FILE *stream );
extern int			STEAM_ferror( STEAM_FILE *stream );
extern void			STEAM_clearerr( STEAM_FILE *stream );
extern void			STEAM_strerror( char *p, unsigned int maxlen );
extern void			STEAM_rewind( STEAM_FILE *stream );
extern int			STEAM_fflush( STEAM_FILE *stream );
extern unsigned int	STEAM_FileSize( STEAM_FILE *file );
extern int			STEAM_setvbuf( STEAM_FILE *stream, char *buffer, int mode, size_t size);
extern char *		STEAM_fgets(char *string, int n, STEAM_FILE *stream);
extern int			STEAM_fputc(int c, STEAM_FILE *stream);
extern int			STEAM_fputs(const char *string, STEAM_FILE *stream);
extern STEAM_FILE *	STEAM_tmpfile(void);
extern long			STEAM_findfirst(char *pszMatchName, STEAMCLibFindFilter filter, struct _finddata_t *fileinfo );
extern int			STEAM_findnext(long dir, struct _finddata_t *fileinfo );
extern int			STEAM_findclose(long dir);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef INCLUDED_STEAMLIBAPI_H */
