///////////////////////////////////////////////////////////////////////  
//  CIdvMeshImporter Class
//
//  (c) 2003 IDV, Inc.
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

#pragma once

#include <vector>
#include <string>
#include "LibVector_Source/IdvVector.h"


///////////////////////////////////////////////////////////////////////  
//  class CIdvMeshImporter declaration

class CIdvMeshImporter
{
public:

    ///////////////////////////////////////////////////////////////////////
    //  struct SMaterial

    struct SMaterial
    {
        CVec                        cAmbient;
        CVec                        cDiffuse;
        CVec                        cSpecular;
        float                       fShininess;
        float                       fAlpha;
        bool                        bHasTexture;
        unsigned int                nTextureMapChannel;
        unsigned int                dlTexture;
        std::string                 strTextureFile;
    };

    ///////////////////////////////////////////////////////////////////////
    //  struct SStrip

    struct SStrip
    {
        unsigned int                nMaterialIndex;
        std::vector<unsigned short> vVertexIndexes;
        std::vector<unsigned short> vNormalIndexes;
        std::vector<unsigned short> vTextureIndexes;
    };

    ///////////////////////////////////////////////////////////////////////
    //  struct SUVWMap

    struct SUVWMap
    {
        std::vector<CVec3>          vCoords;
    };

    ///////////////////////////////////////////////////////////////////////
    //  struct SFace

    struct SFace
    {
        int                         nMaterialIndex;
        unsigned int                nA, nB, nC;
    };

    ///////////////////////////////////////////////////////////////////////
    //  struct SMesh

    struct SMesh
    {
        std::vector<SMaterial>      vMaterials;
        std::vector<CVec3>          vVertices;
        std::vector<CVec3>          vNormals;
        std::vector<SUVWMap>        vUVWMaps;
        std::vector<SStrip>         vStrips;
        std::vector<SFace>          vFaces;
        CVec3                       cMin;
        CVec3                       cMax;
    };

    virtual bool                    LoadFile(const char* strFilename, const char* strFilePath);
    virtual unsigned int            TurnMeshToDisplayList(int nMeshIndex);
            std::vector<SMesh>      m_vMeshData;

protected:
            void                    SetMaterial(SMaterial &sMat);
            std::string             m_strFilePath;

private:
            bool                    LoadMaterial(SMaterial &sNew);
            bool                    LoadStrip(SStrip &sNew);
            bool                    LoadUVWMap(SUVWMap &sNew);
            bool                    LoadFace(SFace &sNew);
            bool                    LoadMesh(SMesh &sNew);
            bool                    LoadVec(CVec3 &cNew);
            bool                    LoadVec4(CVec &cNew);
        
            FILE*                   m_pFile;
};

