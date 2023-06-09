// (C)2008 S2 Games
// c_vec.h
//
//=============================================================================
#ifndef __C_VEC_H__
#define __C_VEC_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_vec2.h"
#include "c_vec3.h"
#include "c_vec4.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
// Common vectors
#define V_ZERO		CVec3f(0.0f, 0.0f, 0.0f)

#define V2_ZERO		CVec2f(0.0f, 0.0f)
#define V3_ZERO		CVec3f(0.0f, 0.0f, 0.0f)
#define V4_ZERO		CVec4f(0.0f, 0.0f, 0.0f, 0.0f)

#define V_UP		CVec3f(0.0f, 0.0f, 1.0f)
//=============================================================================

#endif //__C_VEC_H__
