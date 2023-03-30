/**********************************************************************
 *<
	FILE: fp_basics.cpp

	DESCRIPTION:	A simple example of Function publishing

	OVERVIEW:		Function Publishing (FP) exposes plugin functionality to other
					plugins, and to maxscript. FP allows maxscript to more closely
					follow standard C and C++ syntax conventions.

						For example FP allows you to call a member function that is a part
						of an object like so:

						foo.BarMethod()

						whereas in standard maxscript syntax, to call a function on an object
						would require the scripter to parse two different words, i.e.:

						BarMethod foo

						Of the two examples the first more closely follows standard C/C++ syntax
						conventions.

					This plugin demonstrates how to code a simple function published interface
					that contains:

						One function that takes two numbers and returns their product.

						One function that displays a standard Win32 message box.

						One property that can be read and written to.

					This plugin is implemented as a Global Utility Plugin (.gup) to demonstrate
					that maxscript exposure for a function published interface can be done outside
					of standard maxscript plugins (.dlx). The Maxscript usage is listed below with
					the member function implementations of class FP_Basic.

	CREATED BY:		Chris Johnson

	HISTORY:		Started June 7 2005

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/


#include "fp_basics.h"

#define fp_basics_CLASS_ID	Class_ID(0x3f869fdf, 0x63e46b8e)


//================================================
//Global utility plugin skeleton
//================================================
class fp_basic_gup : public GUP {
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
		fp_basic_gup();
		~fp_basic_gup();		
};
//------------------------------------
//member implementations
//------------------------------------
fp_basic_gup::fp_basic_gup()
{

}
fp_basic_gup::~fp_basic_gup()
{

}

DWORD fp_basic_gup::Start( ) {
	return GUPRESULT_KEEP; // Activate and Stay Resident
}

void fp_basic_gup::Stop( ) {
	// TODO: Do plugin un-initialization here
}

DWORD fp_basic_gup::Control( DWORD parameter ) {
	return 0;
}

IOResult fp_basic_gup::Save(ISave *isave) {
	return IO_OK;
}

IOResult fp_basic_gup::Load(ILoad *iload) {
	return IO_OK;
}

//================================================
//Our class descriptor
//================================================
class fp_basicsClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new fp_basic_gup(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return GUP_CLASS_ID; }
	Class_ID		ClassID() { return fp_basics_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("fpbasics"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	

};

static fp_basicsClassDesc fp_basicsDesc;
ClassDesc2* Getfp_basicsDesc() { return &fp_basicsDesc; }


//================================================
//Function Publishing implementations
//================================================

//This is a contructor for class FP_Basic that takes a variable number of arugments.
FP_Basic fp_basic_desc(
	FP_BASIC_INTERFACE, //Interface_ID
	_T("fpbasics"),		//Internal Fixed Name
	IDS_CLASS_NAME,		//localized string resource ID
	&fp_basicsDesc,		//owning class descriptor
	FP_CORE,			//Flags

		//Functions -------------------------
		//Function that takes two numbers and multiplies them together
		//Note the scripter visible name of the function is passed in as a string
		IFP_Basic::em_products, _T("products"), 0, TYPE_FLOAT, 0, 2,
			_T("float_X"), 0, TYPE_FLOAT,
			_T("float_Y"), 0, TYPE_FLOAT,

		//Function that displays a message box
		IFP_Basic::em_message, _T("Message"), 0, TYPE_VOID, 0, 0,

		//Properties ------------------------
		//Property description that can has read / write functionality
		properties,
			IFP_Basic::em_getNum, IFP_Basic::em_setNum, _T("Number"), 0 , TYPE_FLOAT,

		end
);

//--------------------------------------------------------
//Maxscript usage:
//--------------------------------------------------------
//fpbasics.products 2.5 4
//--> 10.0
float FP_Basic::products(float x, float y)
{
	return x * y;
}

//--------------------------------------------------------
//Maxscript usage:
//--------------------------------------------------------
//fpbasics.message()
//--> A standard Windows message box will then appear.
void FP_Basic::message()
{
	MessageBox(NULL, _T("This was called via a void member function."), _T("Function Publishing Demonstration"), MB_OK);
}

//--------------------------------------------------------
//Maxscript usage:
//--------------------------------------------------------
//fpbasics.number
//--> 0.0
float FP_Basic::GetNum()
{
	return m_Num;
}

//--------------------------------------------------------
//Maxscript usage:
//--------------------------------------------------------
//fpbasics.number
//--> 0.0
//fpbasics.number = 4.5
//--> 4.5
//fpbasics.number
//--> 4.5
void FP_Basic::SetNum(float x)
{
	m_Num = x;
}