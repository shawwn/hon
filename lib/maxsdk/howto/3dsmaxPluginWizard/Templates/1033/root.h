/**********************************************************************
 *<
	FILE: [!output PROJECT_NAME].h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef __[!output CLASS_NAME]__H
#define __[!output CLASS_NAME]__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
[!if SIMPLE_TYPE]
[!if PROCEDURAL_OBJECT_TYPE]
#include "Simpobj.h"
[!endif]
[!if MODIFIER_TYPE]
#include "Simpmod.h"
#include "Simpobj.h"
[!endif]
[!if PARTICLE_TYPE]
#include "Simpobj.h"
[!endif]
[!endif]//SIMPLE TYPE

[!if SPACE_WARP_TYPE]
#include "simpmod.h"
#include "simpobj.h"
[!endif]
[!if COLPICK_TYPE]
#include "hsv.h"
[!endif]
[!if FILE_IMPORT_TYPE]
#include <direct.h>
#include <commdlg.h>
[!endif]
[!if GUP_TYPE]
#include <guplib.h>
[!endif]
[!if FRONT_END_CONTROLLER_TYPE]
#include "frontend.h"
[!endif]
[!if IMAGE_FILTER_COMPOSITOR_TYPE]
#include "tvnode.h"
#include "bmmlib.h"
#include "fltlib.h"
[!endif]
[!if IMAGE_VIEWER_TYPE]
#include "ViewFile.h"
[!endif]
[!if MODIFIER_TYPE]
#include "meshadj.h"
#include "XTCObject.h"
[!endif]
[!if SAMPLER_TYPE]
#include "samplers.h"
[!endif]
[!if SHADER_TYPE]
#include "texutil.h"
#include "shaders.h"
#include "macrorec.h"
#include "gport.h"
[!endif]
[!if SHADOW_TYPE]
#include "shadgen.h"
[!endif]
[!if TEX_TYPE]
#include "stdmat.h"
#include "imtl.h"
#include "macrorec.h"
[!endif]
[!if TRACK_VIEW_UTILITY_TYPE]
#include "tvutil.h"
[!endif]
[!if UTILITY_TYPE]
#include "utilapi.h"
[!endif]
[!if IK_TYPE]
#include "IKSolver.h"
[!endif]
[!if SKIN_GIZMO_TYPE]
#include "ISkin.h"
#include "ISkinCodes.h"
#include "icurvctl.h"
[!endif]
[!if MANIP_TYPE]
#include "Manipulator.h"
[!endif]
[!if ATMOSPHERIC_TYPE]
#include "gizmo.h"
#include "gizmoimp.h"
[!endif]
[!if EXTENSION]
#include "XTCObject.h"
[!endif]


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#endif
