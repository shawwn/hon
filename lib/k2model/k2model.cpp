// k2model.cpp
//
// K2 Model converter
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2model_common.h"

#include <k0_string.h>
#include <k0_math.h>
#include <c_cmdline.h>

#include <iostream>

#include "c_outmodel.h"
#include "c_ink2v3model.h"
#include "c_insilverbackmodel.h"
#include "c_outclip.h"
#include "c_ink2v2clip.h"
#include "c_insilverbackclip.h"

#ifdef _UNICODE
#define tcout wcout
#define tcerr wcerr
#else
#define tcout cout
#define tcerr cerr
#endif

using std::tcout;
using std::tcerr;
using std::endl;
//=============================================================================

/*====================
  K0Main
  ====================*/
int		K0Main(const tstring &sCmdLine)
{
	CCmdLine cCmdLine(sCmdLine);

	tcout << _T("K2 Model Converter ") << _T("v")VERSION << _T(" ") << endl;

	tstring sInFilename(cCmdLine.GetParameter(_T("in")));
	tstring sExt(Filename_GetExtension(sInFilename));

	if (CompareNoCase(sExt, _T("model")) == 0)
	{
#if 0
		CInSilverbackModel cInModel;
#else
		CInK2v3Model cInModel;
#endif
		COutModel cOutModel;
		
		cInModel.SetOutModel(&cOutModel);
		cInModel.ReadFile(sInFilename);

		if (cCmdLine.HasParameter(_T("out")))
			cOutModel.WriteFile(cCmdLine.GetParameter(_T("out")));
	}
	else if (CompareNoCase(sExt, _T("clip")) == 0)
	{
		CInSilverbackClip cInClip;
		COutClip cOutClip;
		
		cInClip.SetOutClip(&cOutClip);
		cInClip.ReadFile(sInFilename);

		if (cCmdLine.HasParameter(_T("out")))
			cOutClip.WriteFile(cCmdLine.GetParameter(_T("out")));
	}

	return 0;
}


/*====================
  _tmain
  ====================*/
int		_tmain(int argc, _TCHAR* argv[])
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	tstring sCmdLine;

	for (int i(0); i < argc; ++i)
	{
		if (i > 0)
			sCmdLine += _T(' ');

		sCmdLine += argv[i];
	}

	return K0Main(sCmdLine);
}
