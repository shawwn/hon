// cvsutils.cpp
//
// CVS Utils
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "cvsutils_common.h"

#include <k0_string.h>
#include <k0_math.h>
#include <c_cmdline.h>
#include <c_filehandle.h>

#include <iostream>

//#include "localfile.h"

#ifdef _UNICODE
#define tcout wcout
#define tcerr wcerr
#else
#define tcout cout
#define tcerr cerr
#endif

using std::tcout;
using std::tcerr;
using std::endl;

const tstring NEXT_FILE("=============================================================================");
const tstring RCS_FILE("RCS file:");
const tstring BEGIN_REVISION("----------------------------");

struct SCommit
{
	tstring		sDate;
	tstring		sAuthor;
	tstring		sCommitID;
	tstring		sLogMessage;
};	
//=============================================================================

/*====================
  ParseDesc
  ====================*/
void	ParseDesc(const tstring &sDesc, smaps &mapDesc)
{
	size_t z(0);
	while (z != tstring::npos)
	{
		size_t zEndName(sDesc.find_first_of(_T(":"), z));
		if (zEndName == tstring::npos)
			break;

		tstring sName(sDesc.substr(z, zEndName - z));

		z = sDesc.find_first_not_of(_T(" "), zEndName + 1);

		size_t zEndValue(sDesc.find_first_of(_T(";"), z));
		if (zEndValue == tstring::npos)
			break;

		tstring sValue(sDesc.substr(z, zEndValue - z));

		mapDesc[sName] = sValue;

		z = sDesc.find_first_not_of(_T(" "), zEndValue + 1);

		if (z >= sDesc.length() || z == tstring::npos)
			break;
	}
}


/*====================
  K0Main
  ====================*/
int		K0Main(const tstring &sCmdLine)
{
	CCmdLine cCmdLine(sCmdLine);

	tcout << _T("CVS Utils ") << _T("v")VERSION << _T(" ") << endl;

	tstring sInFilename(cCmdLine.GetParameter(_T("in")));

	CFileHandle hFile(sInFilename, FILE_READ | FILE_TEXT);

	map<tstring, SCommit> mapCommits;
	map<tstring, SCommit> mapCommitsDate;

	do
	{
		tstring sLine(hFile.ReadLine());

		while (sLine != NEXT_FILE)
		{
			if (sLine.substr(0, RCS_FILE.length()) == RCS_FILE)
				sLine = hFile.ReadLine();
			else if (sLine == BEGIN_REVISION)
			{
				tstring sRevision(hFile.ReadLine());
				tstring sDesc(hFile.ReadLine());

				smaps mapDesc;
				ParseDesc(sDesc, mapDesc);

				sLine = hFile.ReadLine();

				tstring sLogMessage;
				while (sLine != NEXT_FILE && sLine != BEGIN_REVISION)
				{
					sLogMessage += sLine;
					sLogMessage += newl;

					sLine = hFile.ReadLine();
				}

				tstring &sState(mapDesc[_T("state")]);

				if (sState == _T("dead"))
					continue;

				tstring &sCommitID(mapDesc[_T("commitid")]);

				map<tstring, SCommit>::iterator itCommit(mapCommits.find(sCommitID));
				if (itCommit == mapCommits.end())
				{
					mapCommits[sCommitID] = SCommit();
					itCommit = mapCommits.find(sCommitID);

					SCommit &commit(itCommit->second);
					
					commit.sCommitID = sCommitID;
					commit.sDate = mapDesc[_T("date")];
					commit.sAuthor = mapDesc[_T("author")];
					commit.sLogMessage = sLogMessage;
				}
				else
				{
					SCommit &commit(itCommit->second);

					if (commit.sCommitID != sCommitID)
					{
						tcerr << _T("commitid mismatch on ") << sCommitID << newl;
						continue;
					}

#if 0
					tstring &sDate(mapDesc[_T("date")]);
					if (commit.sDate != sDate)
					{
						tcerr << _T("date mismatch on ") << sCommitID << newl;
						continue;
					}
#endif

					tstring &sAuthor(mapDesc[_T("author")]);
					if (commit.sAuthor != sAuthor)
					{
						tcerr << _T("author mismatch on ") << sCommitID << newl;
						continue;
					}

					if (commit.sLogMessage != sLogMessage)
					{
						commit.sLogMessage += sLogMessage;
						commit.sLogMessage += newl;
						continue;
					}
				}
			}
			else
				sLine = hFile.ReadLine();

			if (hFile.IsEOF())
				break;
		}
	}
	while (!hFile.IsEOF());

	// Sort by date
	for (map<tstring, SCommit>::iterator it(mapCommits.begin()); it != mapCommits.end(); ++it)
		mapCommitsDate[it->second.sDate] = it->second;

	// Output
	for (map<tstring, SCommit>::iterator it(mapCommitsDate.begin()); it != mapCommitsDate.end(); ++it)
	{
		tcout << _T("commitid: ") << it->second.sCommitID << newl;
		tcout << _T("date: ") << it->second.sDate << newl;
		tcout << _T("author: ") << it->second.sAuthor << newl << newl;
		tcout << _T("Log Message: ") << newl << it->second.sLogMessage << newl;

		tcout << _T("=============================================================================") << newl << newl;
	}

	return 0;
}


/*====================
  _tmain
  ====================*/
int		_tmain(int argc, _TCHAR* argv[])
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	tstring sCmdLine;

	for (int i(0); i < argc; ++i)
	{
		if (i > 0)
			sCmdLine += _T(' ');

		sCmdLine += argv[i];
	}

	return K0Main(sCmdLine);
}
