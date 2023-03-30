// (C)2009 S2 Games
// c_console.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_console.h"
#include "c_cvar.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR(uint, con_maxInputHistory, 32);
CVAR(wstring, con_title, L"HoN Chat Server");
//=============================================================================

/*====================
  CConsole::~CConsole
  ====================*/
CConsole::~CConsole()
{
	delete m_pStd;
	delete m_pChatter;
	delete m_pPerf;
	delete m_pAdmin;
	delete m_pNet;
	delete m_pMM;

	delete m_pRegistry;

	delete m_pSTLHeap;
	delete m_pHeap;
}


/*====================
  CConsole::CConsole
  ====================*/
CConsole::CConsole() :
NULL_CORE_API,
m_pHeap(NULL),
m_pSTLHeap(NULL),

m_pRegistry(NULL),

m_pStd(NULL),
m_pChatter(NULL),
m_pPerf(NULL),
m_pAdmin(NULL),
m_pNet(NULL),
m_pMM(NULL),

m_bOverwrite(false),
m_bOutputLineInProgress(false),
m_zInputPos(0),
m_zInputPosStored(0),
m_iInputHistoryPos(0)
{
}


/*====================
  CConsole::CompleteSymbol
  ====================*/
void	CConsole::CompleteSymbol()
{
	if (m_sInputBuffer.empty())
		return;

	// Find the end of whatever token the cursor is in
	size_t zEnd(m_zInputPos);
	for (size_t z(zEnd); z < m_sInputBuffer.size(); ++z)
	{
		if (m_sInputBuffer[z] == L' ')
			break;

		++zEnd;
	}

	// Find the start of the token
	size_t zStart(m_zInputPos);
	for (size_t z(zStart); z > 0; --z)
	{
		if (m_sInputBuffer[z - 1] == L' ')
			break;

		--zStart;
	}

	if (zStart >= zEnd)
		return;

	vector<IConsoleElement*> vMatches;

	wstring sLower(LowerString(m_sInputBuffer.substr(zStart, zEnd - zStart)));
	for (ElementMap_it it(m_pRegistry->begin()); it != m_pRegistry->end(); ++it)
	{
		if (sLower == it->first.substr(0, sLower.size()))
			vMatches.push_back(it->second);
	}

	if (vMatches.empty())
		return;

	// If a single match is found, just set the input line to match
	if (vMatches.size() == 1)
	{
		const wstring &sName(vMatches.back()->GetName());
		m_sInputBuffer.replace(zStart, zEnd - zStart, sName);
		m_zInputPos = zStart + sName.size() + 1;
		m_sInputBuffer.insert(m_zInputPos - 1, 1, L' ');
		return;
	}

	// Print potential matches
	for (uint ui(0); ui < vMatches.size(); ++ui)
	{
		switch (vMatches[ui]->GetType())
		{
		case CONSOLE_COMMAND:	Admin() << vMatches[ui]->GetName() << newl; break;
		case CONSOLE_VARIABLE:	Admin() << vMatches[ui]->GetName() << L" = " << static_cast<ICVar*>(vMatches[ui])->GetString() << newl; break;
		}
	}

	// Match as many chars as possible
	size_t zIndex(sLower.size());
	wstring sAppend;
	vector<IConsoleElement*>::iterator itBegin(vMatches.begin());
	vector<IConsoleElement*>::iterator itEnd(vMatches.end());
	
	bool bDone(false);
	for (;;)
	{
		for (vector<IConsoleElement*>::iterator itMatch(itBegin); itMatch != itEnd; ++itMatch)
		{
			if ((*itMatch)->GetName().size() <= zIndex || (*itMatch)->GetName().at(zIndex) != (*itBegin)->GetName().at(zIndex))
			{
				bDone = true;
				break;
			}
		}

		if (bDone)
			break;

		sAppend += (*itBegin)->GetName().at(zIndex);
		++zIndex;
	}

	m_sInputBuffer += sAppend;
	m_zInputPos = m_sInputBuffer.size();
}


/*====================
  CConsole::RegisterElements
  ====================*/
void	CConsole::RegisterElements()
{
	IConsoleElement *pElement(IConsoleElement::GetFirstElement());

	while (pElement != NULL)
	{
		wstring sNameLower(LowerString(pElement->GetName()));
		assert(m_pRegistry->find(sNameLower) == m_pRegistry->end());
		m_pRegistry->insert(ElementMap_pair(sNameLower, pElement));

		switch (pElement->GetType())
		{
		case CONSOLE_VARIABLE:
			break;

		case CONSOLE_COMMAND:
			break;
		}

		pElement = pElement->GetNextElement();
	}
}


/*====================
  CConsole::LookupElement
  ====================*/
IConsoleElement*	CConsole::LookupElement(const wstring &sName)
{
	ElementMap_it itFind(m_pRegistry->find(LowerString(sName)));
	if (itFind == m_pRegistry->end())
		return NULL;

	return itFind->second;
}


/*====================
  CConsole::StartLogFile
  ====================*/
void	CConsole::StartLogFile()
{
	wstring sDate(m_pCore->GetDateString());
	for (uint ui(0); ui < sDate.size(); ++ui)
		sDate[ui] = (sDate[ui] == L'/') ? L'-' : sDate[ui];

	m_pStd->SetLogFile(L"./" + m_pSystem->GetProfile() + L"/logs/status-" + sDate + L".log");
	m_pChatter->SetLogFile(L"./" + m_pSystem->GetProfile() + L"/logs/chat-" + sDate + L".log");
	m_pNet->SetLogFile(L"./" + m_pSystem->GetProfile() + L"/logs/net-" + sDate + L".log");
	m_pMM->SetLogFile(L"./" + m_pSystem->GetProfile() + L"/logs/mm-" + sDate + L".log");
}


/*====================
  CConsole::Initialize
  ====================*/
bool	CConsole::Initialize(CCore *pCore)
{
	INIT_CORE_API(pCore);

	m_pHeap = m_pMemManager->AllocateHeap("console");
	m_pSTLHeap = m_pMemManager->AllocateHeap("console_stl");

	m_pRegistry = HEAP_NEW(m_pMemManager, m_pHeap) ElementMap(std::less<wstring>(), ElementMap_alloc(m_pMemManager, m_pSTLHeap));

#ifdef _DEBUG

#ifdef NO_STD_CONSOLE
	m_pStd = HEAP_NEW(m_pMemManager, m_pHeap) COutpuStream(m_pSystem, STREAM_TARGET_NONE);
#else // NO_STD_CONSOLE
	m_pStd = HEAP_NEW(m_pMemManager, m_pHeap) COutputStream(m_pSystem, STREAM_TARGET_CONSOLE | STREAM_TARGET_DEBUG);
#endif // NO_STD_CONSOLE
	m_pChatter = HEAP_NEW(m_pMemManager, m_pHeap) COutputStream(m_pSystem, STREAM_TARGET_NONE);
	m_pPerf = HEAP_NEW(m_pMemManager, m_pHeap) COutputStream(m_pSystem, STREAM_TARGET_NONE | STREAM_TARGET_CONSOLE);
	m_pAdmin = HEAP_NEW(m_pMemManager, m_pHeap) COutputStream(m_pSystem, STREAM_TARGET_CONSOLE | STREAM_TARGET_DEBUG);
	m_pNet = HEAP_NEW(m_pMemManager, m_pHeap) COutputStream(m_pSystem, STREAM_TARGET_CONSOLE | STREAM_TARGET_NONE);
	m_pMM = HEAP_NEW(m_pMemManager, m_pHeap) COutputStream(m_pSystem, STREAM_TARGET_CONSOLE | STREAM_TARGET_NONE);

#else // _DEBUG

	m_pStd = HEAP_NEW(m_pMemManager, m_pHeap) COutputStream(m_pSystem, STREAM_TARGET_NONE);
	m_pChatter = HEAP_NEW(m_pMemManager, m_pHeap) COutputStream(m_pSystem, STREAM_TARGET_NONE);
	m_pPerf = HEAP_NEW(m_pMemManager, m_pHeap) COutputStream(m_pSystem, STREAM_TARGET_NONE);
	m_pAdmin = HEAP_NEW(m_pMemManager, m_pHeap) COutputStream(m_pSystem, STREAM_TARGET_CONSOLE);
	m_pNet = HEAP_NEW(m_pMemManager, m_pHeap) COutputStream(m_pSystem, STREAM_TARGET_NONE);
	m_pMM = HEAP_NEW(m_pMemManager, m_pHeap) COutputStream(m_pSystem, STREAM_TARGET_NONE);

#endif  _DEBUG

	if (m_pStd == NULL || m_pChatter == NULL || m_pPerf == NULL || m_pAdmin == NULL)
		return false;

	StartLogFile();
	RegisterElements();

	ExecuteFile(L"startup.cfg");

	m_pSystem->SetTitle(con_title);

	return true;
}


/*====================
  CConsole::Frame
  ====================*/
void	CConsole::Frame()
{
	PROFILE("CConsole::Frame")

	m_pSystem->UpdateConsole(*this);
}


/*====================
  CConsole::Shutdown
  ====================*/
void	CConsole::Shutdown()
{
	CFileHandle hConfig(L"./" + m_pSystem->GetProfile() + L"/startup.cfg", FILE_WRITE | FILE_TRUNCATE | FILE_TEXT | FILE_UTF16);
	if (!hConfig.IsOpen())
		return;

	for (ElementMap_it itElement(m_pRegistry->begin()), itEnd(m_pRegistry->end()); itElement != itEnd; ++itElement)
	{
		if (itElement->second->GetType() == CONSOLE_VARIABLE)
			hConfig << itElement->second->GetName() << L" " << static_cast<ICVar*>(itElement->second)->GetString() << wnewl;
	}
}


/*====================
  CConsole::AddInputHistory
  ====================*/
void	CConsole::AddInputHistory(const wstring &sInput)
{
	// Don't add empty lines
	if (sInput.empty())
		return;

	// Don't duplicate the last line
	if (!m_deqInputHistory.empty())
	{
		if (sInput == m_deqInputHistory.front())
			return;
	}

	m_deqInputHistory.push_front(sInput);

	if (m_deqInputHistory.size() > con_maxInputHistory)
		m_deqInputHistory.pop_back();
}


/*====================
  CConsole::Execute
  ====================*/
void	CConsole::Execute(const wstring &sLine)
{
	wsvector vArgs(TokenizeString(sLine, L' '));
	wstring sCommand(vArgs[0]);

	uint uiWrite(0);
	for (uint uiIndex(1); uiIndex < vArgs.size(); ++uiIndex)
	{
		if (vArgs[uiIndex].empty())
			continue;
		
		vArgs[uiWrite] = vArgs[uiIndex];
		++uiWrite;
	}
	vArgs.resize(uiWrite);

	IConsoleElement *pElement(LookupElement(sCommand));
	if (pElement == NULL)
	{
		Admin() << L"Element not found: " << sCommand << newl;
	}
	else
	{
		if (pElement->GetType() == CONSOLE_COMMAND)
		{
			static_cast<CCommand*>(pElement)->Execute(m_pCore, vArgs);
		}
		else if (pElement->GetType() == CONSOLE_VARIABLE)
		{
			ICVar *pVar(static_cast<ICVar*>(pElement));
			if (vArgs.empty())
				Admin() << pVar->GetName() << L" = " << pVar->GetString() << newl;
			else
				pVar->Set(ConcatinateArgs(vArgs));
		}
	}
}


/*====================
  CConsole::ExecuteBuffer
  ====================*/
void	CConsole::ExecuteBuffer()
{
	Admin() << L">" << m_sInputBuffer << newl;

	m_sInputBufferStored.clear();
	m_zInputPosStored = 0;
	m_iInputHistoryPos = -1;

	if (m_sInputBuffer.empty())
		return;

	Execute(m_sInputBuffer);

	m_sInputBuffer.clear();
	m_zInputPos = 0;
}


/*====================
  CConsole::ExecuteFile
  ====================*/
void	CConsole::ExecuteFile(const wstring &sPath)
{
	CFileHandle hConfig(L"./" + m_pSystem->GetProfile() + L"/" + sPath, FILE_READ | FILE_TEXT);
	if (!hConfig.IsOpen())
		return;

	while (!hConfig.IsEOF())
		Execute(hConfig.ReadLineW());
}


/*====================
  CConsole::Paste
  ====================*/
void	CConsole::Paste()
{
	if (!m_pSystem->IsClipboardString())
		return;

	wstring sText(m_pSystem->GetClipboardString());
	if (m_zInputPos != m_sInputBuffer.size())
	{
		if (m_bOverwrite)
			m_sInputBuffer.replace(m_zInputPos, sText.size(), sText);
		else
			m_sInputBuffer.insert(m_zInputPos, sText);
	}
	else
	{
		m_sInputBuffer.append(sText.begin(), sText.end());
	}

	m_zInputPos += sText.size();
}


/*====================
  CConsole::CharEvent

  TODO: Selection w/highlighting
  ====================*/
void	CConsole::CharEvent(bool bDown, uint uiCount, uint uiKeyCode, wchar_t c, uint uiFlags)
{
	if (!bDown)
		return;

	// Characters
	if (uiFlags & KEYBOARD_CHARACTER)
	{
		if (m_zInputPos != m_sInputBuffer.size())
		{
			if (m_bOverwrite)
				m_sInputBuffer.at(m_zInputPos) = c;
			else
				m_sInputBuffer.insert(m_zInputPos, 1, c);
		}
		else
		{
			m_sInputBuffer.append(1, c);
		}

		++m_zInputPos;
		return;
	}

	// Other keys
	switch (uiKeyCode)
	{
	case 'C':
		m_pSystem->CopyToClipboard(m_sInputBuffer);
		break;

	case 'V':
		Paste();
		break;

	case 'X':
		m_pSystem->CopyToClipboard(m_sInputBuffer);
		m_sInputBuffer.clear();
		m_zInputPos = 0;
		break;

	case KEY_BACKSPACE:
		if (m_zInputPos > 0)
		{
			--m_zInputPos;
			m_sInputBuffer.erase(m_zInputPos, 1);
		}
		break;
		
	case KEY_DELETE:
		if (m_zInputPos < m_sInputBuffer.size())
			m_sInputBuffer.erase(m_zInputPos, 1);
		break;

	case KEY_TAB:
		CompleteSymbol();
		break;

	case KEY_ESCAPE:
		m_sInputBuffer.clear();
		m_zInputPos = 0;
		break;

	case KEY_LEFT:
		if (uiFlags & KEYBOARD_CTRL)
		{
			bool bFoundWord(false);
			while (m_zInputPos > 0)
			{
				bool bIsSeperator(IsTokenSeparator(m_sInputBuffer[m_zInputPos - 1]));
				if (bIsSeperator)
				{
					if (bFoundWord)
						break;
				}
				else
				{
					bFoundWord = true;
				}
				--m_zInputPos;
			}
		}
		else
		{
			if (m_zInputPos > 0)
				--m_zInputPos;
		}
		break;

	case KEY_RIGHT:
		if (uiFlags & KEYBOARD_CTRL)
		{
			bool bFoundSpace(false);
			while (m_zInputPos < m_sInputBuffer.size())
			{
				bool bIsSeperator(IsTokenSeparator(m_sInputBuffer[m_zInputPos]));
				if (bIsSeperator)
				{
					bFoundSpace = true;
				}
				else
				{
					if (bFoundSpace)
						break;
				}
				++m_zInputPos;
			}
		}
		else
		{
			if (m_zInputPos < m_sInputBuffer.size())
				++m_zInputPos;
		}
		break;

	case KEY_UP:
		if (m_iInputHistoryPos < int(m_deqInputHistory.size()) - 1)
		{
			if (m_iInputHistoryPos == -1)
			{
				m_sInputBufferStored = m_sInputBuffer;
				m_zInputPosStored = m_zInputPos;
			}

			++m_iInputHistoryPos;
			m_sInputBuffer = m_deqInputHistory[m_iInputHistoryPos];
			m_zInputPos = m_sInputBuffer.size();
		}
		break;

	case KEY_DOWN:
		if (m_iInputHistoryPos > 0)
		{
			--m_iInputHistoryPos;
			m_sInputBuffer = m_deqInputHistory[m_iInputHistoryPos];
			m_zInputPos = m_sInputBuffer.size();
		}
		else if (m_iInputHistoryPos == 0)
		{
			--m_iInputHistoryPos;
			m_sInputBuffer = m_sInputBufferStored;
			m_zInputPos = m_zInputPosStored;
		}
	break;

	case KEY_HOME:
		m_zInputPos = 0;
		break;

	case KEY_END:
		m_zInputPos = m_sInputBuffer.size();
		break;

	case KEY_ENTER:
		AddInputHistory(m_sInputBuffer);
		ExecuteBuffer();
		break;

	case KEY_INSERT:
		if (uiFlags & KEYBOARD_CTRL)
			m_pSystem->CopyToClipboard(m_sInputBuffer);
		else if (uiFlags & KEYBOARD_SHIFT)
			Paste();
		else
			m_bOverwrite = !m_bOverwrite;
		break;
	}
}


/*====================
  CConsole::AddOutputHistory
  ====================*/
void	CConsole::AddOutputHistory(const string &s)
{
	if (s.empty())
		return;

	svector vOut(TokenizeString(NormalizeLineBreaks(s, "\x0a"), '\x0a'));
	for (svector_it it(vOut.begin()); it != vOut.end(); ++it)
	{
		if (it->empty())
		{
			Std() << newl;
			m_bOutputLineInProgress = false;
			continue;
		}

		if (!m_bOutputLineInProgress)
			Std() << "[" << m_pCore->GetTimeString() << "] ";

		Std() << *it;
		m_bOutputLineInProgress = true;
	}
}

void	CConsole::AddOutputHistory(const wstring &s)
{
	if (s.empty())
		return;

	wsvector vOut(TokenizeString(NormalizeLineBreaks(s, L"\x0a"), L'\x0a'));
	for (wsvector_it it(vOut.begin()); it != vOut.end(); ++it)
	{
		if (it->empty())
		{
			Std() << newl;
			m_bOutputLineInProgress = false;
			continue;
		}

		if (!m_bOutputLineInProgress)
			Std() << L"[" << m_pCore->GetTimeString() << L"] ";

		Std() << *it;
		m_bOutputLineInProgress = true;
	}
}


/*====================
  CConsole::PrintCommandList
  ====================*/
void	CConsole::PrintCommandList()
{
	for (ElementMap_it it(m_pRegistry->begin()), itEnd(m_pRegistry->end()); it != itEnd; ++it)
	{
		if (it->second->GetType() == CONSOLE_COMMAND)
			Admin() << it->second->GetName() << newl;
	}
}


/*====================
  CConsole::PrintVariableList
  ====================*/
void	CConsole::PrintVariableList()
{
	for (ElementMap_it it(m_pRegistry->begin()), itEnd(m_pRegistry->end()); it != itEnd; ++it)
	{
		if (it->second->GetType() == CONSOLE_VARIABLE)
			Admin() << it->second->GetName() << L" = " << static_cast<ICVar*>(it->second)->GetString() << newl;
	}
}
