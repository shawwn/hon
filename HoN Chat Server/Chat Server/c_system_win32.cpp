// (C)2009 S2 Games
// c_system_win32.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_system.h"

#define _WIN32_DCOM
#define _WIN32_WINNT 0x0500
#define _WIN32_WINDOWS 0x0410
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#include <mmsystem.h>
#include <Psapi.h>

#include "c_console.h"
#include "c_netmanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint INPUT_RECORD_ARRAY_SIZE(256);

typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(
	HANDLE hProcess,
	DWORD dwPid,
	HANDLE hFile,
	MINIDUMP_TYPE DumpType,
	CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
	);

static wstring g_sDumpPath;
//=============================================================================

/*====================
  System_ExceptionFilter
  ====================*/
LONG WINAPI	System_ExceptionFilter(EXCEPTION_POINTERS *pExceptionInfo)
{
	HMODULE hDebugLib(LoadLibrary(L"dbghelp.dll"));
	if (hDebugLib == NULL)
		return EXCEPTION_EXECUTE_HANDLER;

	MINIDUMPWRITEDUMP fnDump((MINIDUMPWRITEDUMP)GetProcAddress(hDebugLib, "MiniDumpWriteDump"));
	if (fnDump == NULL)
		return EXCEPTION_EXECUTE_HANDLER;

	wstring sDumpPath(g_sDumpPath + L"crash_0000.dmp");
	int iLimit(M_Power(10, 4) - 1);
	for (int i(0); _waccess(sDumpPath.c_str(), 0) != -1 && i <= iLimit; ++i)
		sDumpPath = g_sDumpPath + L"crash_" + XtoA(i, FMT_PADZERO, 4) + L".dmp";

	HANDLE hFile(CreateFile(sDumpPath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));
	if (hFile == NULL)
		return EXCEPTION_EXECUTE_HANDLER;

	MINIDUMP_EXCEPTION_INFORMATION ExInfo;
	ExInfo.ThreadId = GetCurrentThreadId();
	ExInfo.ExceptionPointers = pExceptionInfo;
	ExInfo.ClientPointers = NULL;

	fnDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MINIDUMP_TYPE(MiniDumpWithDataSegs | MiniDumpWithIndirectlyReferencedMemory), &ExInfo, NULL, NULL);

	CloseHandle(hFile);

	// Restart
	STARTUPINFO info;
	memset(&info, 0, sizeof(STARTUPINFO));
	info.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION proc;
	memset(&proc, 0, sizeof(PROCESS_INFORMATION));

	wstring sCommandLine(::GetCommandLine());
	size_t zSize(sCommandLine.size() + 1);
	wchar_t *szCommandLine(new wchar_t[zSize]);
	memset(szCommandLine, 0, sizeof(wchar_t) * zSize);
	wcscpy_s(szCommandLine, zSize, sCommandLine.c_str());

	::CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &info, &proc);
	Sleep(15000);

	delete[] szCommandLine;

	return EXCEPTION_EXECUTE_HANDLER;
}


/*====================
  System_DummySetUnhandledExceptionFilter
  ====================*/
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI System_DummySetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	return NULL;
}


/*====================
  CSystem::CSystem
  ====================*/
CSystem::CSystem() :
NULL_CORE_API,
m_llStartTicks(0),
m_hOutputHandle(NULL),
m_hInputHandle(NULL),
m_bConsoleOverwrite(false),
m_uiCursorX(0),
m_uiCursorY(0)
{
}


/*====================
  CSystem::Init
  ====================*/
void	CSystem::Initialize(CCore *pCore)
{
	INIT_CORE_API(pCore);

	// Set profile directory
	if (m_sProfile.empty())
		SetProfile(L"default");

	// Timer
	::timeBeginPeriod(1);
	m_llStartTicks = GetTicks();

	srand(uint(GetTicks() & UINT_MAX));

	// Console
	m_hOutputHandle = ::GetStdHandle(STD_OUTPUT_HANDLE);
	m_hInputHandle = ::GetStdHandle(STD_INPUT_HANDLE);
	
	DWORD dwMode(0);
	::GetConsoleMode(m_hInputHandle, &dwMode);

	::SetConsoleMode(m_hInputHandle, 0);
	::SetConsoleMode(m_hOutputHandle, ENABLE_PROCESSED_OUTPUT);
	::SetConsoleCtrlHandler(NULL, TRUE);

	// Text color
	::SetConsoleTextAttribute(m_hOutputHandle, FOREGROUND_GREEN | FOREGROUND_RED);
	
	// Set screen buffer size that window will size to
	COORD coord;
	coord.X = 100;
	coord.Y = 50;
	::SetConsoleScreenBufferSize(m_hOutputHandle, coord);

	// Maximize window
	HWND hConsoleWindow(::GetConsoleWindow());

	MINMAXINFO minmax;
	memset(&minmax, 0, sizeof(MINMAXINFO));
	::SendMessage(hConsoleWindow, WM_GETMINMAXINFO, 0, LPARAM(&minmax));

	WINDOWINFO info;
	::GetWindowInfo(hConsoleWindow, &info);
	::MoveWindow(hConsoleWindow, info.rcWindow.left, info.rcWindow.top, minmax.ptMaxSize.x * 2, minmax.ptMaxSize.y, TRUE);

	// Set screen buffer to have some history
	coord.Y = 500;
	::SetConsoleScreenBufferSize(m_hOutputHandle, coord);

	// Set handler for writing crash dumps
	::SetUnhandledExceptionFilter(System_ExceptionFilter);
	PreventSetUnhandledExceptionFilter();

	::SetErrorMode(SetErrorMode(0) | SEM_NOGPFAULTERRORBOX);
}


/*====================
  CSystem::SetProfile
  ====================*/
void	CSystem::SetProfile(const wstring &sName)
{
	if (_waccess_s(sName.c_str(), 0) != 0)
	{
		if (_wmkdir(sName.c_str()) != 0)
			MessageBox(NULL, wstring(L"Could not create directory for profile: " + sName).c_str(), L"Error", MB_OK);
	}

	wstring sLogs(sName + L"/logs");
	if (_waccess_s(sLogs.c_str(), 0) != 0)
	{
		if (_wmkdir(sLogs.c_str()) != 0)
			MessageBox(NULL, wstring(L"Could not create logs directory for profile: " + sName).c_str(), L"Error", MB_OK);
	}

	wstring sCrashDumps(sName + L"/crashdumps");
	if (_waccess_s(sCrashDumps.c_str(), 0) != 0)
	{
		if (_wmkdir(sCrashDumps.c_str()) != 0)
			MessageBox(NULL, wstring(L"Could not create crashdumps directory for profile: " + sName).c_str(), L"Error", MB_OK);
	}

	m_sProfile = sName;
	g_sDumpPath = L"./" + m_sProfile + L"/crashdumps/";
}


/*====================
  CSystem::PrintDebugMessage
  ====================*/
void	CSystem::PrintDebugMessage(const wstring &sMsg)
{
	::OutputDebugString(sMsg.c_str());
}


/*====================
  CSystem::DebugBreak
  ====================*/
void	CSystem::DebugBreak()
{
	__asm int 0x03;
}


/*====================
  CSystem::PreventSetUnhandledExceptionFilter

  This hooks the system's SetUnhandledExceptionFilter so that
  K2's exception filter cannot be replaced

  This only works on x86
  ====================*/
bool	CSystem::PreventSetUnhandledExceptionFilter()
{
#ifdef _M_IX86
	HMODULE hKernel32(::LoadLibrary(L"kernel32.dll"));
	if (hKernel32 == NULL)
		return false;

	void *pOrgEntry(::GetProcAddress(hKernel32, "SetUnhandledExceptionFilter"));
	if (pOrgEntry == NULL)
		return false;

	unsigned char newJump[100];
	INT_PTR dwOrgEntryAddr((INT_PTR)pOrgEntry);
	dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far
	void *pNewFunc(&System_DummySetUnhandledExceptionFilter);
	INT_PTR dwNewEntryAddr((INT_PTR)pNewFunc);
	INT_PTR dwRelativeAddr(dwNewEntryAddr - dwOrgEntryAddr);

	newJump[0] = 0xE9;  // JMP absolute
	memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));
	SIZE_T bytesWritten;
	BOOL bRet = ::WriteProcessMemory(GetCurrentProcess(), pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten);
	return bRet != FALSE;
#else
	return false;
#endif
}


/*====================
  CSystem::GetLastError
  ====================*/
uint	CSystem::GetLastError() const
{
	return ::GetLastError();
}


/*====================
  CSystem::GetErrorString
  ====================*/
wstring	CSystem::GetErrorString(uint err) const
{
	TCHAR szError[1024];
	wcscpy_s(szError, 1024, L"Unknown error.");

	::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
		err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szError, 1024, 0);

	// Remove the CR/LF that FormatMessage() tacks on
	wstring sError(szError);
	return sError.substr(0, sError.length() - 2);
}


/*====================
  CSystem::GetLastErrorString
  ====================*/
wstring	CSystem::GetLastErrorString() const
{
	return GetErrorString(GetLastError());
}


/*====================
  CSystem::Fatal
  ====================*/
void	CSystem::Fatal(const wstring &sMsg)
{
	::MessageBox(NULL, sMsg.c_str(), L"Fatal Error", MB_OK);
	exit(EXIT_ERROR);
}


/*====================
  CSystem::GetTicks
  ====================*/
LONGLONG	CSystem::GetTicks() const
{
	LARGE_INTEGER ll;
	::QueryPerformanceCounter(&ll);
	return ll.QuadPart;
}


/*====================
  CSystem::GetFrequency
  ====================*/
LONGLONG	CSystem::GetFrequency() const
{
	LARGE_INTEGER ll;
	::QueryPerformanceFrequency(&ll);
	return ll.QuadPart;
}


/*====================
  CSystem::SetTitle
  ====================*/
void	CSystem::SetTitle(const wstring &sTitle)
{
	::SetConsoleTitle(sTitle.c_str());
}


/*====================
  System_IsCharacter
  ====================*/
bool	System_IsCharacter(KEY_EVENT_RECORD &key)
{
	if (key.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED))
		return false;

	if (key.wVirtualKeyCode >= 'A' && key.wVirtualKeyCode <= 'Z')
		return true;

	if (key.wVirtualKeyCode >= '0' && key.wVirtualKeyCode <= '9')
		return true;

	if (key.wVirtualKeyCode == VK_SPACE)
		return true;

	if (key.wVirtualKeyCode >= VK_NUMPAD0 && key.wVirtualKeyCode <= VK_DIVIDE)
		return true;
	
	if (key.wVirtualKeyCode >= VK_OEM_1 && key.wVirtualKeyCode <= VK_OEM_3)
		return true;
	
	if (key.wVirtualKeyCode >= VK_OEM_4 && key.wVirtualKeyCode <= VK_OEM_8)
		return true;

	return false;
}


/*====================
  System_TranslateKeyboardCode
  ====================*/
uint	System_TranslateKeyboardCode(KEY_EVENT_RECORD &key)
{
	if (key.wVirtualKeyCode == VK_CONTROL)
		return KEY_INVALID;

	if (System_IsCharacter(key))
		return key.uChar.UnicodeChar;

	if (key.wVirtualKeyCode >= 'A' && key.wVirtualKeyCode <= 'Z')
		return key.wVirtualKeyCode;

	if (key.wVirtualKeyCode >= '0' && key.wVirtualKeyCode <= '9')
		return key.wVirtualKeyCode;

	switch (key.wVirtualKeyCode)
	{
	case VK_BACK:	return KEY_BACKSPACE;
	case VK_TAB:	return KEY_TAB;
	case VK_RETURN:	return KEY_ENTER;
	case VK_ESCAPE:	return KEY_ESCAPE;
	case VK_PRIOR:	return KEY_PGUP;
	case VK_NEXT:	return KEY_PGDN;
	case VK_END:	return KEY_END;
	case VK_HOME:	return KEY_HOME;
	case VK_LEFT:	return KEY_LEFT;
	case VK_UP:		return KEY_UP;
	case VK_RIGHT:	return KEY_RIGHT;
	case VK_DOWN:	return KEY_DOWN;
	case VK_INSERT:	return KEY_INSERT;
	case VK_DELETE:	return KEY_DELETE;
	default:		return KEY_INVALID;
	}
}


/*====================
  CSystem::UpdateConsole
  ====================*/
void	CSystem::UpdateConsole(CConsole &console)
{
	PROFILE("CSystem::UpdateConsole")

	// Input
	DWORD dwInputRecordCount(0);
	INPUT_RECORD aInputRecords[INPUT_RECORD_ARRAY_SIZE];

	::GetNumberOfConsoleInputEvents(m_hInputHandle, &dwInputRecordCount);
	if (dwInputRecordCount > 0)
	{
		if (!::ReadConsoleInput(m_hInputHandle, aInputRecords, INPUT_RECORD_ARRAY_SIZE, &dwInputRecordCount))
			PrintDebugMessage(GetLastErrorString() + L"\n");

		for (uint uiIndex(0); uiIndex < dwInputRecordCount; ++uiIndex)
		{
			switch (aInputRecords[uiIndex].EventType)
			{
			case KEY_EVENT:
				uint uiFlags(0);
				if (aInputRecords[uiIndex].Event.KeyEvent.dwControlKeyState & LEFT_ALT_PRESSED)
					uiFlags |= KEYBOARD_ALT_LEFT;
				if (aInputRecords[uiIndex].Event.KeyEvent.dwControlKeyState & RIGHT_ALT_PRESSED)
					uiFlags |= KEYBOARD_ALT_RIGHT;
				if (aInputRecords[uiIndex].Event.KeyEvent.dwControlKeyState & LEFT_CTRL_PRESSED)
					uiFlags |= KEYBOARD_CTRL_LEFT;
				if (aInputRecords[uiIndex].Event.KeyEvent.dwControlKeyState & RIGHT_CTRL_PRESSED)
					uiFlags |= KEYBOARD_CTRL_RIGHT;
				if (aInputRecords[uiIndex].Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED)
					uiFlags |= KEYBOARD_SHIFT;
				if (System_IsCharacter(aInputRecords[uiIndex].Event.KeyEvent))
					uiFlags |= KEYBOARD_CHARACTER;

				console.CharEvent(
					aInputRecords[uiIndex].Event.KeyEvent.bKeyDown == 1,
					aInputRecords[uiIndex].Event.KeyEvent.wRepeatCount,
					System_TranslateKeyboardCode(aInputRecords[uiIndex].Event.KeyEvent),
					aInputRecords[uiIndex].Event.KeyEvent.uChar.UnicodeChar,
					uiFlags
					);
				break;
			}
		}
	}

	if (m_bConsoleOverwrite != console.GetOverwrite())
	{
		m_bConsoleOverwrite = console.GetOverwrite();

		CONSOLE_CURSOR_INFO cci;
		cci.bVisible = TRUE;
		cci.dwSize = console.GetOverwrite() ? 100 : 25;
		::SetConsoleCursorInfo(m_hOutputHandle, &cci);
	}

	// Output
	DWORD dwNumberOfCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi; 
	::GetConsoleScreenBufferInfo(m_hOutputHandle, &csbi);

	wstring sInputLine(L">" + console.GetInputBuffer());
	size_t zInputPos(console.GetInputPos() + 1);

	if (sInputLine != m_sConsoleInputBuffer)
	{
		COORD coord;
		coord.X = 0;
		coord.Y = csbi.dwCursorPosition.Y;

		::FillConsoleOutputCharacter(m_hOutputHandle, ' ', csbi.dwSize.X, coord, &dwNumberOfCharsWritten);

		::SetConsoleCursorPosition(m_hOutputHandle, coord);

		::WriteConsole(m_hOutputHandle, sInputLine.c_str(), DWORD(sInputLine.length()), &dwNumberOfCharsWritten, NULL);

		m_sConsoleInputBuffer = sInputLine;
	}

	if (csbi.dwCursorPosition.X != zInputPos)
	{
		COORD coord;
		coord.X = ushort(zInputPos);
		coord.Y = csbi.dwCursorPosition.Y;

		::SetConsoleCursorPosition(m_hOutputHandle, coord);
	}
}


/*====================
  CSystem::WriteToConsole
  ====================*/
void	CSystem::WriteToConsole(const wstring &sText)
{
	DWORD dwNumberOfCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi; 
	::GetConsoleScreenBufferInfo(m_hOutputHandle, &csbi);

	// Clear input line
	COORD coordInput;
	coordInput.X = 0;
	coordInput.Y = csbi.dwCursorPosition.Y;

	if (!m_sConsoleInputBuffer.empty())
	{
		::FillConsoleOutputCharacter(m_hOutputHandle, L' ', csbi.dwSize.X, coordInput, &dwNumberOfCharsWritten);
		m_sConsoleInputBuffer.clear();
	}

	// Write new text
	COORD coordWrite;
	coordWrite.X = m_uiCursorX;
	coordWrite.Y = m_uiCursorY;
	::SetConsoleCursorPosition(m_hOutputHandle, coordWrite);
	
	::WriteConsole(m_hOutputHandle, sText.c_str(), DWORD(sText.length()), &dwNumberOfCharsWritten, NULL);

	CONSOLE_SCREEN_BUFFER_INFO info;
	::GetConsoleScreenBufferInfo(m_hOutputHandle, &info);
	m_uiCursorX = info.dwCursorPosition.X;
	m_uiCursorY = info.dwCursorPosition.Y;
}


/*====================
  CSystem::CopyToClipboard
  ====================*/
void	CSystem::CopyToClipboard(const wstring &sText)
{
	HGLOBAL hGlobal(::GlobalAlloc(GHND | GMEM_SHARE, (sText.length() + 1) * sizeof(wchar_t)));

	TCHAR *pGlobal(static_cast<wchar_t*>(::GlobalLock(hGlobal)));

	for (wstring::const_iterator it(sText.begin()), itEnd(sText.end()); it != itEnd; ++it)
		*pGlobal++ = *it;

	::GlobalUnlock(hGlobal);

	::OpenClipboard(::GetConsoleWindow());

	::EmptyClipboard();

	//::SetClipboardData(CF_TEXT, hGlobal);
	::SetClipboardData(CF_UNICODETEXT, hGlobal);

	::CloseClipboard();
}


/*====================
  CSystem::IsClipboardString
  ====================*/
bool	CSystem::IsClipboardString() const
{
	//return ::IsClipboardFormatAvailable(CF_TEXT) != FALSE;
	return ::IsClipboardFormatAvailable(CF_UNICODETEXT) != FALSE;
}


/*====================
  CSystem::GetClipboardString
  ====================*/
wstring	CSystem::GetClipboardString() const
{
	::OpenClipboard(::GetConsoleWindow());

	//HGLOBAL hGlobal(::GetClipboardData(CF_TEXT));
	HGLOBAL hGlobal(::GetClipboardData(CF_UNICODETEXT));
	if (!hGlobal)
		return WSNULL;

	wstring sReturn;
	wchar_t *pGlobal(static_cast<wchar_t*>(::GlobalLock(hGlobal)));
	while (pGlobal && *pGlobal)
		sReturn += *pGlobal++;

	::GlobalUnlock(hGlobal);
	::CloseClipboard();

	return sReturn;
}


/*====================
  CSystem::Sleep
  ====================*/
void	CSystem::Sleep(uint uiMsecs) const
{
	::Sleep(uiMsecs);
}


/*====================
  CSystem::GetTotalPhysicalMemory
  ====================*/
ULONGLONG	CSystem::GetTotalPhysicalMemory()
{
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&status);
	return status.ullTotalPhys;
}


/*====================
  CSystem::GetFreePhysicalMemory
  ====================*/
ULONGLONG	CSystem::GetFreePhysicalMemory()
{
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&status);
	return status.ullAvailPhys;
}


/*====================
  CSystem::GetTotalVirtualMemory
  ====================*/
ULONGLONG	CSystem::GetTotalVirtualMemory()
{
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&status);
	return status.ullTotalVirtual;
}


/*====================
  CSystem::GetFreeVirtualMemory
  ====================*/
ULONGLONG	CSystem::GetFreeVirtualMemory()
{
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&status);
	return status.ullAvailVirtual;
}


/*====================
  CSystem::GetTotalPageFile
  ====================*/
ULONGLONG	CSystem::GetTotalPageFile()
{
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&status);
	return status.ullTotalPageFile;
}


/*====================
  CSystem::GetFreePageFile
  ====================*/
ULONGLONG	CSystem::GetFreePageFile()
{
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&status);
	return status.ullAvailPageFile;
}


/*====================
  CSystem::GetProcessMemoryUsage
  ====================*/
ULONGLONG	CSystem::GetProcessMemoryUsage()
{
	PROCESS_MEMORY_COUNTERS procMemory;
	GetProcessMemoryInfo(GetCurrentProcess(), &procMemory, sizeof(PROCESS_MEMORY_COUNTERS));
	return procMemory.WorkingSetSize;
}


/*====================
  CSystem::GetProcessVirtualMemoryUsage
  ====================*/
ULONGLONG	CSystem::GetProcessVirtualMemoryUsage()
{
	PROCESS_MEMORY_COUNTERS procMemory;
	GetProcessMemoryInfo(GetCurrentProcess(), &procMemory, sizeof(PROCESS_MEMORY_COUNTERS));
	return procMemory.PagefileUsage;
}


/*====================
  ProcessCommandLine
  ====================*/
void	CSystem::ProcessCommandLine(CCore *pCore)
{
	wstring sCommandLine(::GetCommandLine());
	wsvector vArgs;

	size_t zStart(sCommandLine.find_first_not_of(L" \n\r\t"));
	size_t zEnd(zStart);

	while (zStart < sCommandLine.length())
	{
		if (sCommandLine[zStart] == L'"')
			zEnd = sCommandLine.find(L"\"", ++zStart);
		else
			zEnd = sCommandLine.find_first_of(L" \n\r\t", zStart);

		vArgs.push_back(sCommandLine.substr(zStart, zEnd - zStart));
		if (zEnd == wstring::npos)
			zStart = zEnd;
		else
			zStart = sCommandLine.find_first_not_of(L" \n\r\t", zEnd + 1);
	}

	for (wsvector_it itArg(vArgs.begin()), itEnd(vArgs.end()); itArg != itEnd; )
	{
		if (*itArg == L"-listen")
		{
			++itArg;

			ushort unPort(0);
			if (itArg != itEnd && (*itArg)[0] != L'-')
			{
				unPort = AtoN(*itArg);
				++itArg;
			}

			pCore->GetNetManager()->StartListening(unPort);
			continue;
		}
		else if (*itArg == L"-profile")
		{
			++itArg;
			SetProfile(*itArg);
		}

		++itArg;
	}
}
