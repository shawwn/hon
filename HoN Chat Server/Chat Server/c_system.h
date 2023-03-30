// (C)2009 S2 Games
// c_system.h
//
//=============================================================================
#ifndef __C_SYSTEM_H__
#define __C_SYSTEM_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_core.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CConsole;
class CClientManager;
//=============================================================================

//=============================================================================
// CSystem
//=============================================================================
class CSystem
{
private:
	DECLARE_CORE_API;

	// Timers
	LONGLONG	m_llStartTicks;

	// Console
	void*		m_hOutputHandle;
	void*		m_hInputHandle;
	bool		m_bConsoleOverwrite;
	wstring		m_sConsoleInputBuffer;

	uint		m_uiCursorX;
	uint		m_uiCursorY;

	wstring		m_sProfile;

public:
	~CSystem()	{}
	CSystem();

	void				Initialize(CCore *pCore);
	void				SetProfile(const wstring &sName);
	const wstring&		GetProfile() const					{ return m_sProfile; }

	// Debugging
	static void			PrintDebugMessage(const wstring &sMsg);
	static void			DebugBreak();

	// Errors
	bool				PreventSetUnhandledExceptionFilter();
	uint				GetLastError() const;
	wstring				GetErrorString(uint uiError) const;
	wstring				GetLastErrorString() const;
	static void			Fatal(const wstring &sMsg);

	// Timers
	inline uint			GetMilliseconds() const					{ return uint((GetTicks() - m_llStartTicks) / (GetFrequency() / 1000)); }
	inline uint			GetSeconds() const						{ return uint((GetTicks() - m_llStartTicks) / GetFrequency()); }
	LONGLONG			GetTicks() const;
	LONGLONG			GetFrequency() const;

	// Console
	void*				GetOutputHandle()						{ return m_hOutputHandle; }
	void*				GetInputHandle()						{ return m_hInputHandle; }
	void				SetTitle(const wstring &sTitle);
	void				UpdateConsole(CConsole &console);
	void				WriteToConsole(const string &sText)		{ WriteToConsole(SingleToWide(sText)); }
	void				WriteToConsole(const wstring &sText);

	// Clipboard
	void				CopyToClipboard(const wstring &sText);
	bool				IsClipboardString() const;
	wstring				GetClipboardString() const;

	// Misc
	void				Sleep(uint uiMsecs) const;

	// Memory
	static ULONGLONG	GetTotalPhysicalMemory();
	static ULONGLONG	GetFreePhysicalMemory();
	static ULONGLONG	GetTotalVirtualMemory();
	static ULONGLONG	GetFreeVirtualMemory();
	static ULONGLONG	GetTotalPageFile();
	static ULONGLONG	GetFreePageFile();
	static ULONGLONG	GetProcessMemoryUsage();
	static ULONGLONG	GetProcessVirtualMemoryUsage();

	void				ProcessCommandLine(CCore *pCore);
};
//=============================================================================

#endif //__C_SYSTEM_H__
