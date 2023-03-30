//
// tchar_linux.h
//
//=============================================================================

#ifndef __TCHAR_LINUX_H__
#define __TCHAR_LINUX_H__

#ifdef UNICODE
	#include <wchar.h>
	#define TCHAR wchar_t
	#define _T(x) L ## x
	#define _stscanf swscanf
	#define _tcschr wcschr
	#define _tcscmp wcscmp
	#define _tcsicmp wcscasecmp
	#define _tcscpy wcscpy
	#define _tcserror wcserror
	#define _tcslen wcslen
	#define _tcsncmp wcsncmp
	#define _tcstol wcstol
	#define _tfopen wfopen
	#define _tmkdir wmkdir
	#define _tremove wremove
	#define _tstat wstat
	#define _tstof(x) wcstof((x), NULL)
	#define _tstoi(x) (int)wcstol((x), NULL, 0)
	#define _tcsncpy wcsncpy
	#define _vstprintf vswprintf
	#define _vsntprintf vswprintf
	// function wrappers for functions that do not have wchar_t versions
	FILE *wfopen(const wchar_t *path, const wchar_t *mode);
	int wmkdir(const wchar_t *pathname, mode_t mode);
	int wremove(const wchar_t *pathname);
	int wstat(const wchar *path, struct stat *buf);
	wchat_t *wcserror(int errnum);
#else //UNICODE
	#define TCHAR char
	#define _T(x) x
	#define _stscanf sscanf
	#define _taccess access 
	#define _tcschr strchr
	#define _tcscmp strcmp
	#define _tcsicmp strcasecmp
	#define _tcscpy strcpy
	#define _tcserror strerror
	#define _tcslen strlen
	#define _tcsncmp strncmp
	#define _tcstol strtol
	#define _tfopen fopen
	#define _tmkdir mkdir
	#define _tremove remove
	#define _trename rename
	#define _trmdir rmdir
	#define _tstat stat
	#define _tstof atof
	#define _tstoi atoi
	#define _tcsncpy strncpy
	#define _vstprintf vsprintf
	#define _vsntprintf vsnprintf
#endif //UNICODE

//=============================================================================
#endif //__TCHAR_LINUX_H__
