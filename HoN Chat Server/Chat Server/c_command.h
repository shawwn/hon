// (C)2009 S2 Games
// c_command.h
//
//=============================================================================
#ifndef __C_COMMAND_H__
#define __C_COMMAND_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consoleelement.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define CMD(name, min) \
void	cmd##name##Fn(CCore *pCore, const wsvector &vArgList); \
void	cmd##name##CheckFn(CCore *pCore, const wsvector &vArgList) \
{ \
	assert(pCore != NULL); \
\
	if (vArgList.size() < min) \
	{ \
		pCore->GetConsole()->Admin() << L"Command requires at least " L#min L" arguments." << newl; \
		return; \
	} \
\
	cmd##name##Fn(pCore, vArgList); \
} \
CCommand cmd##name(L#name, cmd##name##CheckFn); \
void	cmd##name##Fn(CCore *pCore, const wsvector &vArgList)
//=============================================================================

//=============================================================================
// CCommand
//=============================================================================
class CCommand : public IConsoleElement
{
private:
	typedef void	(*fnCommand)(CCore *pCore, const wsvector &vArgList);

	fnCommand	m_pFunction;

	CCommand();

public:
	~CCommand()	{}
	CCommand(const wstring &sName, fnCommand pFunction);

	void	Execute(CCore *pCore, const wsvector &vArgList)	{ m_pFunction(pCore, vArgList); }
};
//=============================================================================

#endif //__C_COMMAND_H__
