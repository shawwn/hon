///////////////////////////////////////////////////////////////////////  
//  SpeedTreeRT DirectX Example
//
//  (c) 2003 IDV, Inc.
//
//  This example demonstrates how to render trees using SpeedTreeRT
//  and DirectX.  Techniques illustrated include ".spt" file parsing,
//  static lighting, dynamic lighting, LOD implementation, cloning,
//  instancing, and dynamic wind effects.
//
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may
//  not be copied or disclosed except in accordance with the terms of
//  that agreement.
//
//      Copyright (c) 2001-2003 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      1233 Washington St. Suite 610
//      Columbia, SC 29201
//      Voice: (803) 799-1699
//      Fax:   (803) 931-0320
//      Web:   http://www.idvinc.com


///////////////////////////////////////////////////////////////////////  
//  Includes

#pragma once
#include "d3dutil.h"
#include "SpeedTreeConfig.h"
#include <map>
#include <string>


///////////////////////////////////////////////////////////////////////  
//  Branch Vertex Format

static DWORD D3DFVF_SPEEDTREE_BRANCH_VERTEX =
        D3DFVF_XYZ |
        D3DFVF_NORMAL |
        D3DFVF_TEXCOORDSIZE4(0) |
        D3DFVF_TEXCOORDSIZE2(1) | 
        D3DFVF_TEXCOORDSIZE2(2) |
        D3DFVF_TEXCOORDSIZE3(3) |
        D3DFVF_TEX4;


///////////////////////////////////////////////////////////////////////
// FVF Branch Vertex Structure

struct SFVFBranchVertex
{
        D3DXVECTOR3     m_vPosition;            // Always Used                          
        D3DXVECTOR3     m_vNormal;              // Dynamic Lighting Only
        FLOAT           m_afTexCoords[2];       // Always Used
        FLOAT           m_afShadowCoords[2];    // Texture coordinates for the shadows
        FLOAT           m_fWindIndex;           // GPU Only
        FLOAT           m_fWindWeight;          
        D3DXVECTOR3     m_vBinormal;
        D3DXVECTOR3     m_vTangent;
};


///////////////////////////////////////////////////////////////////////  
//  Frond Vertex Format

static DWORD D3DFVF_SPEEDTREE_FROND_VERTEX =
        D3DFVF_XYZ |
        D3DFVF_NORMAL |
        D3DFVF_TEXCOORDSIZE4(0) |
        D3DFVF_TEXCOORDSIZE2(1) |
        D3DFVF_TEX2;


///////////////////////////////////////////////////////////////////////
// FVF Frond Vertex Structure

struct SFVFFrondVertex
{
        D3DXVECTOR3     m_vPosition;            // position                         
        D3DXVECTOR3     m_vNormal;              // dynamic lighting normal          
        FLOAT           m_afTexCoords[2];       // base texcoords
        FLOAT           m_afShadowCoords[2];    // texture coordinates for the shadows
        FLOAT           m_fWindIndex;           // wind params
        FLOAT           m_fWindWeight;          
};


///////////////////////////////////////////////////////////////////////  
//  Leaf Vertex Format

static DWORD D3DFVF_SPEEDTREE_LEAF_VERTEX =
        D3DFVF_XYZ |                            // always have the position
        D3DFVF_NORMAL |
        D3DFVF_TEXCOORDSIZE2(0) | 
        D3DFVF_TEXCOORDSIZE2(1) | 
        D3DFVF_TEXCOORDSIZE3(2) |
        D3DFVF_TEX3;


///////////////////////////////////////////////////////////////////////
// FVF Leaf Vertex Structure

struct SFVFLeafVertex
{
        D3DXVECTOR3     m_vPosition;            // Always Used                          
        D3DXVECTOR3     m_vNormal;              // Dynamic Lighting Only            
        FLOAT           m_fTexCoords[2];        // Always Used
        FLOAT           m_fWindIndex;           // Only used when GPU is involved
        FLOAT           m_fWindWeight;                  
        FLOAT           m_fLeafPlacementIndex;
        FLOAT           m_fLeafScalarValue;
        FLOAT           m_fLeafRockIndex;
};