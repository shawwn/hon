
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
** Target restrictions:
**
** Tool restrictions:
**
******************************************************************************/

// Any precompiled headers (precompiled must come first).
//#include "Stable.h"

// Conditional-compilation control for the current project.

// Definitions used by all of our projects and components.
//#ifndef INCLUDED_COMMONENVIRONMENT_H
//	#include "Projects/Common/Inc/CommonEnvironment.h"
//#endif
//#ifndef INCLUDED_COMMONASSERTS_H
//	#include "Projects/Common/Asserts/Inc/CommonAsserts.h"
//#endif

// Definitions shared throughout this project/component.

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <io.h>
#include <string.h>
 
#ifndef INCLUDED_STEAM_H
    #include "Steam.h"
#endif
#include "SteamCLib.h"

#include <sys/stat.h>
#include <assert.h>

#ifdef _MSC_VER
#define _Thread __declspec(thread)
#else
#define _Thread
#endif

_Thread static TSteamError g_tError;


// TODO: remove the following function, it's NOT Steam generic
static void FixSlashes( char *pszName )
{
	assert(pszName);
	if (pszName)//lint !e774 always
	{
		while ( *pszName ) 
		{
			if ( *pszName == '/' )
				*pszName = '\\';
			pszName++;
		}
	}
}

/*****************************************************************/
/*****************************************************************/
// CLIB wrappers for system functions such as fopen, fread, etc.
/*****************************************************************/
/*****************************************************************/

/**************/
//STEAM_fopen
/**************/
STEAM_FILE *STEAM_fopen(const char *filename, const char *options)
{
	char wadName[_MAX_PATH];
	SteamHandle_t hndl;

	assert(filename);
	assert(options);

	if (!filename || !options)//lint !e774 always
		return NULL;

	strcpy(wadName,filename);			
	FixSlashes(wadName);

	hndl = SteamOpenFile(wadName, options, &g_tError);

	return (STEAM_FILE*)(hndl);
}

/**************/
//STEAM_tmpfile
/**************/
STEAM_FILE *STEAM_tmpfile(void)
{
	SteamHandle_t hndl;

	hndl = SteamOpenTmpFile(&g_tError);

	return (STEAM_FILE*)(hndl);
}

/**************/
//STEAM_fread
/**************/

unsigned int STEAM_fread(void *buffer, unsigned int rdsize, unsigned int count, STEAM_FILE *file)
{
	unsigned int totalSizeRead = 0;
	unsigned int bytesRead = 0;
	unsigned int amtLeftToRead = rdsize * count;
	unsigned int readBlockSize = 524288;

	SteamHandle_t hndl = (SteamHandle_t)(file);
	char* p;

	assert(buffer);
	if (!buffer)//lint !e774 always
		return 0;

	p = (char *)(buffer);
	// TKS: there's no real purpose in splitting up big reads right now
	// since I took out the progress callbacks, but we'll leave the code
	// for now since it's known to work.
	while ( amtLeftToRead )
	{
		unsigned int numBytesToRead;

		// Read the next block
		numBytesToRead = ( amtLeftToRead > readBlockSize ? readBlockSize : amtLeftToRead );
	 	bytesRead = SteamReadFile(p, 1, numBytesToRead, hndl, &g_tError);

		// If we didn't read what we thought we should, we're done...
		if ( bytesRead == 0 || (bytesRead != numBytesToRead && g_tError.eSteamError != eSteamErrorEOF) )
			break;

		// If we had an error (not EOF) we're done...
		if ( g_tError.eSteamError != eSteamErrorNone && g_tError.eSteamError != eSteamErrorEOF )
			break;

		// Update for next loop...	
		amtLeftToRead -= bytesRead; // what's left to read...
		p += bytesRead;				// where to put the next block of data...
		totalSizeRead += bytesRead; // how much we've read so far.
	}

	return totalSizeRead / rdsize;
}




/**************/
//STEAM_fgetc
/**************/
int STEAM_fgetc(STEAM_FILE *file)
{
	SteamHandle_t hndl = (SteamHandle_t)(file);

	return SteamGetc(hndl, &g_tError);
}

/**************/
//STEAM_fgetc
/**************/
char *STEAM_fgets(char *string, int numCharToRead, STEAM_FILE *stream)
{
	// FIX ME
	// when SteamFgets() is implemented
	char c;
	int numCharRead = 0;
	
	assert(string);

	if (!string)//lint !e774 always
		return NULL;

	*string = c = '\0';

	// Read at most n chars from the file or until a newline
	while ( (numCharRead < numCharToRead-1) && (c != '\n') )
	{
		// Read in the next char...
		if ( STEAM_fread(&c, 1, 1, stream) != 1 )
		{
			if ( g_tError.eSteamError != eSteamErrorEOF || numCharRead == 0 )
				return NULL;	// If we hit an error, return NULL.
			
			numCharRead = numCharToRead;	// Hit EOF, no more to read, all done...
		}

		else
		{
			*string++ = c;		// add the char to the string and point to the next pos
			*string = '\0';		// append NULL
			numCharRead++;		// count the char read
		}
	}

	return string; // Has a NULL termination...
}

/**************/
//STEAM_fputc
/**************/
int STEAM_fputc(int c, STEAM_FILE *stream)
{
	SteamHandle_t hndl = (SteamHandle_t)(stream);

	return SteamPutc(c, hndl, &g_tError);
}

/**************/
//STEAM_fputs
/**************/
int STEAM_fputs(const char *string, STEAM_FILE *stream)
{	
	// FIX ME
	// when (if?) SteamFputs() is implemented

	SteamHandle_t hndl = (SteamHandle_t)(stream);
	unsigned int len;
	unsigned int n;

	assert(string);

	if (!string)//lint !e774 always
		return EOF;

	len = strlen(string);

	n = SteamWriteFile(string, sizeof(char), len, hndl, &g_tError);

	if ( n != len || g_tError.eSteamError != eSteamErrorNone )
		return EOF;

	return 1;
}

/**************/
//STEAM_fwrite
/**************/
unsigned int STEAM_fwrite(const void *buffer, unsigned int wrsize, unsigned int count,STEAM_FILE *file)
{
	SteamHandle_t hndl = (SteamHandle_t)(file);

	return SteamWriteFile(buffer, wrsize, count, hndl, &g_tError);
}

/**************/
//STEAM_fprintf
/**************/
int STEAM_fprintf( STEAM_FILE *fp, const char *format, ... )
{
	int retval;
	va_list argptr;

	// Make sure world is sane...
	if ( !fp || !format )
		return -1;

	// Init variable argument list.
	va_start(argptr, format);//lint !e1924 c-style cast

	// Pass it all on...
	retval = STEAM_vfprintf(fp, format, argptr);

	// Clean up...
	va_end(argptr);//lint !e1924 c-style

	return retval;
}

				
/**************/
//STEAM_vfprintf
//
//  Yes...this sucks...but would rather malloc than try to guess what the
//  largest formatted string will be and reserve that on the stack each 
//  time...so we're first vfprintf'ing to the NULL device to determine
//  the length of the formatted string, and then mallocing a buffer to 
//  hold it for vsprintf.
//
/**************/
int STEAM_vfprintf( STEAM_FILE *fp, const char *format, va_list argptr )
{
	int blen, plen;
	char *buf;
	FILE *nullDeviceFP;

	assert(format);

	// Make sure world is sane...
	if ( !fp || !format ) //lint !e774
		return -1;

	// Open the null device...used by vfprintf to determine the length of
	// the formatted string.
	nullDeviceFP = fopen("nul:", "w");
	if ( !nullDeviceFP )
		return -1;

	// Figure out how long the formatted string will be...dump formatted
	// string to the bit bucket.
	blen = vfprintf(nullDeviceFP, format, argptr);//lint !e668 null va_list OK?
	fclose(nullDeviceFP);
	if ( !blen )
	{
		return -1;
	}

	// Get buffer in which to build the formatted string.
	buf = (char *)(malloc((unsigned int)(blen)+1));
	if ( !buf )
	{
		return -1;
	}

	// Build the formatted string.
	plen = vsprintf(buf, format, argptr);//lint !e668 null va_list OK?
	va_end(argptr);//lint !e1924
	if ( plen != blen )
	{
		free(buf);
		return -1;
	}

	// Write out the formatted string.
	if ( plen != (int)(STEAM_fwrite(buf, 1, (unsigned int)(plen), fp)) )
	{
		free(buf);
		return -1;
	}

	free(buf);
	return plen;
}

				
/**************/
//STEAM_fclose
/**************/
int STEAM_fclose(STEAM_FILE *file)
{
	SteamHandle_t hndl = (SteamHandle_t)(file);

	return SteamCloseFile(hndl, &g_tError);
}

/**************/
//STEAM_fseek
/**************/
int STEAM_fseek(STEAM_FILE *file, long offset, int method)
{
	SteamHandle_t hndl = (SteamHandle_t)(file);
	ESteamSeekMethod seekMethod;
	switch(method)
	{
	case SEEK_SET:
		seekMethod = eSteamSeekMethodSet;
		break;
	case SEEK_CUR:
		seekMethod = eSteamSeekMethodCur;
		break;
	case SEEK_END:
		seekMethod = eSteamSeekMethodEnd;
		break;
	default:
		assert(0);
		return -1;//lint !e527 unreachable
	}

	return SteamSeekFile(hndl, offset, seekMethod, &g_tError);
}

/**************/
//STEAM_ftell
/**************/
long STEAM_ftell(STEAM_FILE *file)
{
	SteamHandle_t hndl = (SteamHandle_t)(file);
	long steam_offset;

	// Determine the real file offset (as opposed to the logical offset represented by
	// the fread buffer...)
	steam_offset = SteamTellFile(hndl, &g_tError);
	if ( g_tError.eSteamError != eSteamErrorNone )
		return -1L;

	return steam_offset;
}

/**************/
//STEAM_feof
/**************/
int STEAM_feof( STEAM_FILE *stream )
{
	long orig_pos;
	int n;
	
	// Figure out where in the file we currently are...
	orig_pos = STEAM_ftell(stream);
	if ( g_tError.eSteamError != eSteamErrorNone )
		return 0;

	// Jump to the end...
	if ( STEAM_fseek(stream, 0L, SEEK_END) != 0 )
		return 0;

	// If we were already at the end, return true
	if ( orig_pos == STEAM_ftell(stream) )
		return 1;

	// Otherwise, go back to the original spot and return false.
	n = STEAM_fseek(stream, orig_pos, SEEK_SET);
	assert(!n);
	//lint -esym(550,n)

	return 0; 
}

/**************/
//STEAM_ferror
/**************/
int STEAM_ferror( STEAM_FILE *stream )
{
	return ( g_tError.eSteamError != eSteamErrorNone );
}//lint !e715 not refernced

/**************/
//STEAM_clearerr
/**************/
void STEAM_clearerr( STEAM_FILE *stream )
{
	SteamClearError(&g_tError);
	return;
}//lint !e715 not refernced

/**************/
//STEAM_ferror
/**************/
void STEAM_strerror( char *p, unsigned int maxlen )
{
	if ( p && maxlen > 0 )
		strncpy(p, g_tError.szDesc, maxlen);
	return;
}

/**************/
//STEAM_stat
/**************/
int STEAM_stat(const char *path, struct _stat *buf)
{
	TSteamElemInfo Info;
	int returnVal;

	char tmpPath[_MAX_PATH];

	assert(path);
	assert(buf);

	if (!path || !buf)//lint !e774 always
		return -1;

	strcpy(tmpPath, path);			
	FixSlashes(tmpPath);

	returnVal= SteamStat(tmpPath, &Info, &g_tError);

	if ( returnVal == 0 )
	{
		if (Info.bIsDir )
		{
			buf->st_mode |= _S_IFDIR;
			buf->st_size = 0;
		}
		else
		{
			buf->st_mode |= _S_IFREG;
			buf->st_size = (long)(Info.uSizeOrCount);
		}

		buf->st_atime = Info.lLastAccessTime;
		buf->st_mtime = Info.lLastModificationTime;
		buf->st_ctime = Info.lCreationTime;
	}

	return returnVal;
}

/**************/
//STEAM_rewind
/**************/
void STEAM_rewind( STEAM_FILE *stream )
{
	STEAM_fseek(stream, 0L, SEEK_SET);//lint !e534
	STEAM_clearerr(stream);
	return;
}

/**************/
//STEAM_fflush
/**************/
int STEAM_fflush( STEAM_FILE *stream )
{
	int returnVal = 0;
	SteamHandle_t hndl = (SteamHandle_t)(stream);

	if (SteamFlushFile(hndl, &g_tError))
	{
		returnVal = EOF;
	}

	return returnVal;
}


/**************/
//STEAM_setvbuf
/**************/
int STEAM_setvbuf( STEAM_FILE *stream, char *buffer, int mode, size_t size)
{
	int returnVal = 0;
	SteamHandle_t hndl = (SteamHandle_t)(stream);
	ESteamBufferMethod eMethod;

	switch (mode)
	{
	case _IOFBF:
	case _IOLBF:
		eMethod = eSteamBufferMethodFBF;
		break;
	case _IONBF:
		eMethod = eSteamBufferMethodNBF;
		break;
	default:
		assert(0);
		return -1;//lint !e527 unreachable
	}

	if (SteamSetvBuf(hndl, buffer, eMethod, size, &g_tError))
	{
		returnVal = -1;
	}

	return returnVal;
}


/**************/
//STEAM_findfirst
/**************/
long STEAM_findfirst(char *pszMatchName, STEAMCLibFindFilter filter, struct _finddata_t *fileinfo )
{
	TSteamElemInfo steamFindInfo;
	long hResult = -1;
	SteamHandle_t steamResult;
	char tmpMatchName[_MAX_PATH];
	ESteamFindFilter eFilter;

	assert(pszMatchName);
	assert(fileinfo);

	if (!pszMatchName || !fileinfo)//lint !e774 always
		return -1;

	strcpy(tmpMatchName, pszMatchName);			
	FixSlashes(tmpMatchName);

	switch (filter)
	{
	case STEAMCLibFindLocalOnly:
		eFilter = eSteamFindLocalOnly;
		break;
	case STEAMCLibFindRemoteOnly:
		eFilter = eSteamFindRemoteOnly;
		break;
	case STEAMCLibFindAll:
		eFilter = eSteamFindAll;
		break;
	default:
		assert(0);
		return -1;//lint !e527 unreachable
	}

	steamResult = SteamFindFirst(tmpMatchName, eFilter, &steamFindInfo, &g_tError);

	if ( steamResult == STEAM_INVALID_HANDLE )
	{
		hResult = -1;
	}

	else
	{
		hResult = (long)(steamResult);
		strcpy(fileinfo->name, steamFindInfo.cszName);

		fileinfo->size = steamFindInfo.uSizeOrCount;
		fileinfo->time_access = steamFindInfo.lLastAccessTime;
		fileinfo->time_create = steamFindInfo.lCreationTime;
		fileinfo->time_write = steamFindInfo.lLastModificationTime;
		
		// Determine if the found object is a directory...
		if ( steamFindInfo.bIsDir )
			fileinfo->attrib = _A_SUBDIR;
		else
			fileinfo->attrib = _A_NORMAL;

		// ??? Is there anything we can do with bIsLocal?
	}
	
	return hResult;
}

/**************/
//STEAM_findnext
/**************/
int STEAM_findnext(long dir, struct _finddata_t *fileinfo )
{
	TSteamElemInfo steamFindInfo;
	SteamHandle_t hDir = (SteamHandle_t)(dir);
	int result;

	assert(fileinfo);

	if (!fileinfo)//lint !e774 always
		return -1;

	result = SteamFindNext(hDir, &steamFindInfo, &g_tError);

	if ( result == 0 )
	{
		dir = (long)(hDir);
		strcpy(fileinfo->name, steamFindInfo.cszName);

		fileinfo->size = steamFindInfo.uSizeOrCount;
		fileinfo->time_access = steamFindInfo.lLastAccessTime;
		fileinfo->time_create = steamFindInfo.lCreationTime;
		fileinfo->time_write = steamFindInfo.lLastModificationTime;
		
		// Determine if the found object is a directory...
		if ( steamFindInfo.bIsDir )
			fileinfo->attrib = _A_SUBDIR;
		else
			fileinfo->attrib = _A_NORMAL;

		// ??? Is there anything we can do with bIsLocal?
	}
	
	return result;
}

/**************/
//STEAM_findclose
/**************/
int STEAM_findclose(long dir)
{
	SteamHandle_t hDir = (SteamHandle_t)(dir);

	return SteamFindClose(hDir, &g_tError);
}

/**************/
//STEAM_FileSize
/**************/
unsigned int STEAM_FileSize( STEAM_FILE *file )
{
	SteamHandle_t hFile = (SteamHandle_t)(file);

	return (unsigned int)(SteamSizeFile(hFile, &g_tError));
}


