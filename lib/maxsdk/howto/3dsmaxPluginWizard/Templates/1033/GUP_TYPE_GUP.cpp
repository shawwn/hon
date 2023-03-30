/**********************************************************************
 *<
	FILE: [!output PROJECT_NAME].cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#include "[!output PROJECT_NAME].h"

#define [!output CLASS_NAME]_CLASS_ID	Class_ID([!output CLASSID1], [!output CLASSID2])


class [!output CLASS_NAME] : public [!output SUPER_CLASS_NAME] {
	public:

		static HWND hParams;

		// GUP Methods
		DWORD	Start			( );
		void	Stop			( );
		DWORD	Control			( DWORD parameter );
		
		// Loading/Saving
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		//Constructor/Destructor
		[!output CLASS_NAME]();
		~[!output CLASS_NAME]();		
};


[!output TEMPLATESTRING_CLASSDESC]

[!if PARAM_MAPS]
[!output TEMPLATESTRING_PARAMBLOCKDESC]
[!endif] 

[!output CLASS_NAME]::[!output CLASS_NAME]()
{

}

[!output CLASS_NAME]::~[!output CLASS_NAME]()
{

}

// Activate and Stay Resident
DWORD [!output CLASS_NAME]::Start( ) {
	
	// TODO: Do plugin initialization here
	
	// TODO: Return if you want remain loaded or not
	return GUPRESULT_KEEP;
}

void [!output CLASS_NAME]::Stop( ) {
	// TODO: Do plugin un-initialization here
}

DWORD [!output CLASS_NAME]::Control( DWORD parameter ) {
	return 0;
}

IOResult [!output CLASS_NAME]::Save(ISave *isave)
{
	return IO_OK;
}

IOResult [!output CLASS_NAME]::Load(ILoad *iload)
{
	return IO_OK;
}

