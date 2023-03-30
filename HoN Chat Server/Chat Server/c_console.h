// (C)2009 S2 Games
// c_console.h
//
//=============================================================================
#ifndef __C_CONSOLE_H__
#define __C_CONSOLE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_core.h"

#include "c_outputstream.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CCommand;
class CHeap;
//=============================================================================

//=============================================================================
// CConsole
//=============================================================================
class CConsole
{
private:
	DECLARE_STL_MAP_TYPES(Element, wstring, IConsoleElement*);

	DECLARE_CORE_API;

	CHeap*			m_pHeap;
	CHeap*			m_pSTLHeap;

	ElementMap*		m_pRegistry;

	COutputStream*	m_pStd;
	COutputStream*	m_pChatter;
	COutputStream*	m_pAdmin;
	COutputStream*	m_pPerf;
	COutputStream*	m_pNet;
	COutputStream*	m_pMM;

	bool			m_bOutputLineInProgress;
	bool			m_bOverwrite;

	wstring			m_sInputBuffer;
	size_t			m_zInputPos;

	wstring			m_sInputBufferStored;
	size_t			m_zInputPosStored;

	wsdeque			m_deqInputHistory;
	int				m_iInputHistoryPos;

	void				CompleteSymbol();
	void				RegisterElements();
	IConsoleElement*	LookupElement(const wstring &sName);

	void	Paste();

public:
	~CConsole();
	CConsole();

	COutputStream&	Std()								{ return *m_pStd; }
	COutputStream&	Chatter()							{ return *m_pChatter; }
	COutputStream&	Perf()								{ return *m_pPerf; }
	COutputStream&	Admin()								{ return *m_pAdmin; }
	COutputStream&	Net()								{ return *m_pNet; }
	COutputStream&	MatchMaker()						{ return *m_pMM; }

	void			StartLogFile();

	bool			Initialize(CCore *pCore);
	void			Frame();
	void			Shutdown();

	const wstring&	GetInputBuffer() const				{ return m_sInputBuffer; }
	size_t			GetInputPos() const					{ return m_zInputPos; }
	bool			GetOverwrite() const				{ return m_bOverwrite; }

	void			AddInputHistory(const wstring &sInput);
	void			Execute(const wstring &sLine);
	void			ExecuteBuffer();
	void			ExecuteFile(const wstring &sPath);

	void			CharEvent(bool bDown, uint uiCount, uint uiKeyCode, wchar_t c, uint uiFlags);

	void			AddOutputHistory(const string &s);
	void			AddOutputHistory(const wstring &s);

	void			PrintCommandList();
	void			PrintVariableList();

	template<class T>
	CConsole&		operator<<(T _x)					{ AddOutputHistory(XtoA(_x)); return *this; }
};
//=============================================================================

#endif //__C_CONSOLE_H__
