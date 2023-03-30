/**********************************************************************
 *<
	FILE:			fp_basics.h

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
					of standard maxscript plugins (.dlx). The Maxscript usage is listed in fp_basics.cpp with
					the member function implementations of class FP_Basic.

	CREATED BY:		Chris Johnson

	HISTORY:		Started June 7 2005

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef __fp_basics__H
#define __fp_basics__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "iFnPub.h"


#include <guplib.h>


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

//======================================================
//Function Publishing Examples
//======================================================

#define FP_BASIC_INTERFACE Interface_ID(0x7d0c759f, 0x7714b4b)

inline class IFP_Basic* GetFP_basic() 
{ 
	return (IFP_Basic*) GetCOREInterface(FP_BASIC_INTERFACE); 
}

//Class IFP_Basic is the first of two basic classes that must be used.
//It contains pure virtual functions and enums.
//Early examples of function publishing had enums stored outside of the class
//	but they work fine as members.
class IFP_Basic : public FPStaticInterface
{
	public:
		virtual float GetNum() = 0;
		virtual void  SetNum(float x) = 0;
		virtual float products(float x, float y) = 0;
		virtual void  message() = 0;
		enum { em_getNum, em_setNum, em_products, em_message };
};


//Class FP_Basic must inherit the pure virtual class declared above.
//It also includes Function maps which describe the functions and properties
//we desire for our plugin.
class FP_Basic : public IFP_Basic
{
	private:
		float m_Num;
	public:
		float GetNum();
		void  SetNum(float x);

		float products(float x, float y);
		void  message();

	DECLARE_DESCRIPTOR(FP_Basic);
	BEGIN_FUNCTION_MAP;
		//PROP_FNS is an abbreviation for PROPerty FunctioNS. It requires getter and setter
		//functions, a getter and setter enumeration, and a type (Not in that order). 
		//They must be entered in the order shown below.
		PROP_FNS(IFP_Basic::em_getNum, GetNum, IFP_Basic::em_setNum, SetNum, TYPE_FLOAT);
		//FN_2 is an abbreviation for a function that takes two arguements.
		//Here the function is registered as returning a float value, the function name
		//is passed in as well as the types of the two arguments.
		FN_2(IFP_Basic::em_products, TYPE_FLOAT, products, TYPE_FLOAT, TYPE_FLOAT);
		//VFN_0 is an abbreviation for a Void Function with 0 arguments. Hence a function
		//that closes with a (), i.e. foo.BarMethod()
		VFN_0(IFP_Basic::em_message, message);
	END_FUNCTION_MAP;
};

#endif
