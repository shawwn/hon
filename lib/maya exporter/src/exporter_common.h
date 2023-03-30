// (C)2008 S2 Games
// exporter_common.h
//
// Precompiled header
//=============================================================================
#ifndef __EXPORTER_COMMON_H__
#define __EXPORTER_COMMON_H__

//=============================================================================
// Compile Defines
//=============================================================================
//#define K2_PROFILE
//=============================================================================

//=============================================================================
// Unicode
//
// This comes before everything else, because it affects a lot of the other
// libraries initialization
//=============================================================================
#include "k0_unicode.h"
//=============================================================================

//=============================================================================
// Standard headers
//=============================================================================
#include <tchar.h>
#include <float.h>
#include <errno.h>
#include <cassert>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <stdarg.h>
//=============================================================================

//=============================================================================
// K0 headers
//=============================================================================
#include "k0_types.h"
#include "k0_stl.h"
#include "k0_math.h"
#include "k0_string.h"
#include "xtoa.h"
#include "s_blendedlink.h"
//=============================================================================

//=============================================================================
// Maya Headers
//=============================================================================
#define NT_PLUGIN
#include <maya/MSimple.h>
#include <maya/MGlobal.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MItDag.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnMesh.h>
#include <maya/MMatrix.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MPointArray.h>
#include <maya/MPlugArray.h>
#include <maya/MPlug.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFnBlinnShader.h>
//=============================================================================

//=============================================================================
// Shared headers
//=============================================================================
#include "s2exporter.h"
//=============================================================================

#endif //__EXPORTER_COMMON_H__
