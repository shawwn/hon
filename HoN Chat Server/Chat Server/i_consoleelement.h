// (C)2009 S2 Games
// i_consoleelement.h
//
//=============================================================================
#ifndef __I_CONSOLEELELEMT_H__
#define __I_CONSOLEELELEMT_H__

//=============================================================================
// Definitions
//=============================================================================
enum EConsoleElementType
{
	CONSOLE_COMMAND,
	CONSOLE_VARIABLE
};
//=============================================================================

//=============================================================================
// IConsoleElement
//=============================================================================
class IConsoleElement
{
private:
	// Linked list for registering elements
	static IConsoleElement*	s_pHead;
	static IConsoleElement*	s_pTail;
	IConsoleElement*		m_pNext;

	wstring		m_sName;

	IConsoleElement();

protected:
	EConsoleElementType	m_eType;

public:
	virtual ~IConsoleElement()	{}
	IConsoleElement(const wstring &sName, EConsoleElementType eType) :
	m_sName(sName),
	m_eType(eType),
	m_pNext(NULL)
	{
		if (s_pHead == NULL)
			s_pHead = this;
		if (s_pTail != NULL)
			s_pTail->m_pNext = this;
		s_pTail = this;
	}

	const wstring&			GetName() const			{ return m_sName; }
	EConsoleElementType		GetType() const			{ return m_eType; }

	static IConsoleElement*	GetFirstElement()		{ return s_pHead; }
	IConsoleElement*		GetNextElement() const	{ return m_pNext; }
};
//=============================================================================
#endif //__I_CONSOLEELELEMT_H__