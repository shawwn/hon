/*	
 *		imacroscript.h - Public interface to Macro script manager for the MAX CUI
 *
 *	Macro scripts (or macros) are scripts that live in buttons and menus in the new 
 *  customizable UI.  The macro script manager keeps a directory of all known macros 
 *  and provides an API for running and editing macros and for accessing and 
 *  updating the directory.
 *
 *  The directory is used by the CUI to provide a list of available macros in the 
 *  toolbar/menu/shelf editor.  The API also provides a way for the CUI
 *  to open a macro editor to allow on-the-fly creation of macro scripts. 
 *
 *  Macros are normally entered into the directory by the MAXScript compiler as a
 *  side-effect of compiling a macro () definition.  Anyone using MAXScript can at
 *  any time eval a macro definition and thereby add CUI macro scripts.
 *  Consequently, macros can be stored in any script file and be loaded just by
 *  executing the file.  The macro definition syntax permits any number of macros per
 *  file.  
 *
 *  Most macros will be stored in files in a special CUI macro or config
 *  directory so that a user can take all his custom UI stuff with him by copying
 *  directories.  (this directory supports recursive scanning of sub-dirs,
 *  so users can organize their macros)
 *  On-the-fly macro creation in the CUI editor or by text drag-and-
 *  drop onto the shelf or by evaling a definition in the listener will generate 
 *  a file in this directory to provide permanent storage.
 *
 *			Copyright � Autodesk, Inc, 1998.  John Wainwright.
 *
 */

#ifndef _H_IMACROSCRIPT
#define _H_IMACROSCRIPT

#ifndef ScripterExport
#	ifdef BLD_MAXSCRIPT
#		define ScripterExport __declspec( dllexport )
#	else
#		define ScripterExport __declspec( dllimport )
#	endif
#endif

#include "iFnPub.h"

typedef short MacroID;

class Value;

/* ----------------  macro directory -------------------------- */

class MacroEntry : public BaseInterfaceServer
{
public:
	// access
	virtual MacroID		GetID() = 0;
	virtual TSTR&		GetName() = 0;
	virtual TSTR&		GetCategory() = 0;
	virtual TSTR&		GetInternalCategory() = 0;
	virtual void		SetInternalCategory(TCHAR* ic) = 0;
	virtual TSTR&		GetFileName() = 0;
	virtual void		SetFileName(TCHAR* fn) = 0;
	virtual long		GetOffset() = 0;
	virtual void		SetOffset(int o) = 0;
	virtual Value*		GetCode() = 0;
	virtual void		SetCode(Value* code) = 0;
	virtual TSTR&		GetToolTip() = 0;
	virtual void		SetToolTip(TCHAR* tt) = 0;
	virtual TSTR&		GetButtonText() = 0;
	virtual void		SetButtonText(TCHAR* bt) = 0;
	virtual void		SetButtonIcon(TCHAR* icnf, int indx) = 0;
	virtual TSTR&		GetButtonIconFile() = 0;
	virtual int			GetButtonIconIndex() = 0;
	virtual void		SetFlags(short mask) = 0;
	virtual void		ClearFlags(short mask) = 0;
	virtual short		GetFlags(short mask) = 0;

	// execution
	virtual Value*		Execute() = 0;
	virtual BOOL		Compile() = 0;
	virtual void		Reset() = 0;

	// cleanup
	virtual void		DeleteThis() = 0;

	// R4 extension to support body structured into handlers
	virtual Value*		CallHandler(Value* handler_or_name, Value** arg_list, int count, BOOL hold = TRUE)=0;
	virtual FPStatus	CallHandler(TCHAR* handler_name, FPParams* params, FPValue& result, BOOL hold = TRUE)=0;
	virtual Value*		GetHandler(Value* handler_name)=0;
	virtual BOOL		HasHandler(TCHAR* handler_name)=0;

	virtual MaxIcon*	GetIcon()=0;
};

#define ME_DROPPED_SCRIPT	0x0001		// macro made from some drag-and-dropped text
#define ME_SILENT_ERRORS	0x0002		// macro won't report any runtime errors
#define ME_HAS_EXECUTE		0x0004		// has execute handler
#define ME_TEMPORARY		0x0008		// temporary dropScript

#define BAD_MACRO_ID		-1			// illegal macroID 

class MacroDir : public InterfaceServer
{
public:

	// access by ID or category & name strings
	virtual MacroEntry*	GetMacro(MacroID mid) = 0;
	virtual MacroEntry*	FindMacro(TCHAR* category, TCHAR* name) = 0;
	virtual BOOL		ValidID(MacroID mid) = 0;

	// iteration 
	virtual int			Count() = 0;
	virtual MacroEntry*	GetMacro(int index) = 0;

	// macro entry management

// LAM - 10/15/01 - when SDK can change, remove the following and uncomment the versions where internalCategory
// is passed in. Internally, only the verions using internalCategory are used.
	virtual MacroEntry*	AddMacro(TCHAR* category, TCHAR* name, TCHAR* tooltip, TCHAR* buttonText,
									TCHAR* sourceFile, int sourceOffset) = 0;
	virtual MacroEntry*	AddMacro(TCHAR* category, TCHAR* name, TCHAR* tooltip, TCHAR* buttonText,
									TCHAR* sourceText) = 0;

//	MacroEntry*			AddMacro(TCHAR* category, TCHAR* internalCategory, TCHAR* name, TCHAR* tooltip, TCHAR* buttonText,
//								 TCHAR* sourceFile, int sourceOffset);
//	MacroEntry*			AddMacro(TCHAR* category, TCHAR* internalCategory, TCHAR* name, TCHAR* tooltip, TCHAR* buttonText,
//								 TCHAR* sourceText);

    virtual BOOL		SetMacro(MacroID mid, TCHAR* tooltip, TCHAR* btnText, TCHAR* sourceFile, int sourceOffset) = 0;
	virtual TCHAR*		MakeNameValid(TCHAR* s) = 0;
	virtual TCHAR*		MakeCategoryValid(TCHAR* s) = 0;
	
	// editing
	virtual BOOL		EditMacro(MacroID mid) = 0;
	
	// execution
	virtual Value*		Execute(MacroID mid) = 0;
	// R4 extension to support body structured into handlers
	virtual Value*		CallHandler(MacroID mid, Value* handler_or_name, Value** arg_list, int count, BOOL hold = TRUE)=0;
	virtual FPStatus	CallHandler(MacroID mid, TCHAR* handler_name, FPParams* params, FPValue& result, BOOL hold = TRUE)=0;
	virtual Value*		GetHandler(MacroID mid, Value* handler_name)=0;

	// macro-script file directory scanning & loading
	virtual void		LoadMacroScripts(TCHAR* path_name = NULL, BOOL recurse = TRUE) = 0;
	virtual MacroEntry*	LoadMacroScript(TCHAR* file_name) = 0;

	// set & get default path for storing/searching macro script files
	virtual void		SetMacroScriptPath(TCHAR* path_name) = 0;
	virtual TCHAR*		GetMacroScriptPath() = 0;
};

#if defined(BLD_CORE)
	extern MacroDir& GetMacroScriptDir();
#else
	extern ScripterExport MacroDir& GetMacroScriptDir();
	extern ScripterExport void InitMacroScriptDir();
#endif

// ActionTableIds
const ActionTableId   kActionMacroScripts = 647394;
const ActionContextId kActionMacroScriptsContext = 647394;

#endif    // _H_IMACROSCRIPT
