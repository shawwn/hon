//
// tchar_linux.cpp
//
//=============================================================================

#ifdef UNICODE

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

//POSIX filesystem filenames use chars.  Convert all
// const wchar_t* to const char*

FILE *wfopen(const wchar_t *path, const wchar_t *mode)
{
	char pathout[_MAX_PATH];
	char modeout[4]; //mode should never be more than 3 chars + '\0'
	WideToSingle(pathout, path, wcslen(path));
	WideToSingle(modeout, mode, wcslen(mode));
	return fopen(pathout, modeout);
}

int wmkdir(const wchar_t *pathname, mode_t mode)
{
	char pathout[_MAX_PATH];
	WideToSingle(pathout, pathname, wcslen(pathname));
	return mkdir(pathout, mode);
}

int wremove(const wchar_t *pathname)
{
	char pathout[_MAX_PATH];
	WideToSingle(pathout, pathname, wcslen(pathname));
	return remove(pathout);
}

int wstat(const wchar *path, struct stat *buf)
{
	char pathout[_MAX_PATH];
	WideToSingle(pathout, path, wcslen(pathname));
	return stat(path, buf);
}

wchar_t *wcserror(int errnum)
{
	static wchar_t wcserrorbuf[8196];
	char *err = strerror(errnum);
	SingleToWide(wcserrorbuf, err, (strlen(err) < 8196 ? strlen(err) : 8196));
	return wcserrorbuf;
}

#endif //UNICODE
