// (C)2009 S2 Games
// c_command.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_command.h"
//=============================================================================

/*====================
  CCommand::CCommand
  ====================*/
CCommand::CCommand(const wstring &sName, fnCommand pFunction) :
IConsoleElement(sName, CONSOLE_COMMAND),
m_pFunction(pFunction)
{
}
