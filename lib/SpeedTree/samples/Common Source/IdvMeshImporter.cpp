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


#pragma warning (disable : 4786)
#include "IdvMeshImporter.h"
#include "Console.h"
#include "nv_dds.h"
#include <gl/gl.h>
#include <float.h>
using namespace std;

// from main.cpp
extern CConsole g_cConsole;

///////////////////////////////////////////////////////////////////////  
//  CIdvMeshImporter::LoadFile

bool CIdvMeshImporter::LoadFile(const char* strFilename, const char* strFilePath)
{
    bool bSuccess = true;
    g_cConsole.Print("loading mesh file [%s]...\n", strFilename);
    m_strFilePath = strFilePath;
    m_pFile = fopen(strFilename, "r");
    if (m_pFile)
    {
        unsigned int i = 0;
        while (!feof(m_pFile))
        {
            SMesh sNewMesh;
            if (LoadMesh(sNewMesh))
            {
                m_vMeshData.push_back(sNewMesh);
                ++i;
            }
            else
            {
                g_cConsole.PrintError("\bWarning:\b error loading mesh #%i\n", i);
                bSuccess = false;
                break;
            }
        }
        fclose(m_pFile);
    }
    else
        g_cConsole.PrintError("\bWarning:\b cannot find mesh file [%s]\n", strFilename);

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////  
//  CIdvMeshImporter::TurnMeshToDisplayList

unsigned int CIdvMeshImporter::TurnMeshToDisplayList(int nMeshIndex)
{
    SMesh sMesh = m_vMeshData.at(nMeshIndex);
    bool bMaterialsOk = true;

    unsigned int dlNew = glGenLists(1);
    glNewList(dlNew, GL_COMPILE);

    for (unsigned int i = 0; i < sMesh.vStrips.size(); ++i)
    {
        SStrip sStrip = sMesh.vStrips.at(i);
        
        if (sMesh.vMaterials.size() > sStrip.nMaterialIndex)
            SetMaterial(sMesh.vMaterials.at(sStrip.nMaterialIndex));
        else
            bMaterialsOk = false;
            
        glBegin(GL_TRIANGLE_STRIP);
            for (unsigned int j = 0; j < sStrip.vVertexIndexes.size(); ++j)
            {
                glNormal3fv(sMesh.vNormals[sStrip.vNormalIndexes[j]].m_afData);
                if (sMesh.vMaterials.size( ) > sStrip.nMaterialIndex && 
                    sMesh.vUVWMaps.size( ) > sMesh.vMaterials[sStrip.nMaterialIndex].nTextureMapChannel)
                    glTexCoord2fv(sMesh.vUVWMaps[sMesh.vMaterials[sStrip.nMaterialIndex].nTextureMapChannel].vCoords[sStrip.vTextureIndexes[j]].m_afData);
                glVertex3fv(sMesh.vVertices[sStrip.vVertexIndexes[j]].m_afData);
            }
        glEnd();
    }

    glEndList();

    if (!bMaterialsOk)
        g_cConsole.PrintError("\bWarning:\b Mesh %i has an out of range material index.", nMeshIndex);

    return dlNew;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvMeshImporter::SetMaterial

void CIdvMeshImporter::SetMaterial(SMaterial &sMat)
{
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, sMat.cAmbient.m_afData);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, sMat.cDiffuse.m_afData);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, sMat.cSpecular.m_afData);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, sMat.fShininess);

    static unsigned int dlOldTexture = 0;
    if (dlOldTexture == sMat.dlTexture)
        return;
    dlOldTexture = sMat.dlTexture;

    if (sMat.bHasTexture)
        glBindTexture(GL_TEXTURE_2D, sMat.dlTexture);
    else
        glBindTexture(GL_TEXTURE_2D, 0);
}

///////////////////////////////////////////////////////////////////////  
//  CIdvMeshImporter::LoadMesh

#pragma optimize("", off)
bool CIdvMeshImporter::LoadMesh(SMesh &sNew)
{
    int nLoopVariable = 0;
    sNew.cMin.Set(FLT_MAX, FLT_MAX, FLT_MAX);
    sNew.cMax.Set(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    // materials
    fscanf(m_pFile, "%i ", &nLoopVariable);
    SMaterial sNewMaterial;
    for (unsigned int i = 0; i < nLoopVariable; ++i)
    {
        if (LoadMaterial(sNewMaterial))
            sNew.vMaterials.push_back(sNewMaterial);
        else
        {
            g_cConsole.PrintError("\bWarning:\b error loading material #%i\n", i);
            return false;
        }
    }

    // vertices
    fscanf(m_pFile, "%i ", &nLoopVariable);
    CVec3 cNewVert;
    for (i = 0; i < nLoopVariable; ++i)
    {   
        if (LoadVec(cNewVert))
        {
            sNew.vVertices.push_back(cNewVert);
            sNew.cMin[0] = min(sNew.cMin[0], cNewVert[0]);
            sNew.cMin[1] = min(sNew.cMin[1], cNewVert[1]);
            sNew.cMin[2] = min(sNew.cMin[2], cNewVert[2]);
            sNew.cMax[0] = max(sNew.cMax[0], cNewVert[0]);
            sNew.cMax[1] = max(sNew.cMax[1], cNewVert[1]);
            sNew.cMax[2] = max(sNew.cMax[2], cNewVert[2]);
        }
        else
        {
            g_cConsole.PrintError("\bWarning:\b error loading vertex #%i\n", i);
            return false;
        }
    }
    
    // normals
    fscanf(m_pFile, "%i ", &nLoopVariable);
    CVec3 cNewNormal;
    for (i = 0; i < nLoopVariable; ++i)
    {
        if (LoadVec(cNewNormal))
            sNew.vNormals.push_back(cNewNormal);
        else
        {
            g_cConsole.PrintError("\bWarning:\b error loading normal #%i\n", i);
            return false;
        }
    }

    // UVW Maps
    fscanf(m_pFile, "%i ", &nLoopVariable);
    for (i = 0; i < nLoopVariable; ++i)
    {
        SUVWMap sNewUVWMap;
        if (LoadUVWMap(sNewUVWMap))
            sNew.vUVWMaps.push_back(sNewUVWMap);
        else
        {
            g_cConsole.PrintError("\bWarning:\b error loading UVW Map #%i\n", i);
            return false;
        }
    }

    // Strips
    fscanf(m_pFile, "%i ", &nLoopVariable);
    for (i = 0; i < nLoopVariable; ++i)
    {
        SStrip sNewStrip;
        if (LoadStrip(sNewStrip))
            sNew.vStrips.push_back(sNewStrip);
        else
        {
            g_cConsole.PrintError("\bWarning:\b error loading strip #%i\n", i);
            return false;
        }
    }

    // Faces
    fscanf(m_pFile, "%i ", &nLoopVariable);
    SFace sNewFace;
    for (i = 0; i < nLoopVariable; ++i)
    {
        if (LoadFace(sNewFace))
            sNew.vFaces.push_back(sNewFace);
        else
        {
            g_cConsole.PrintError("\bWarning:\b error loading face #%i\n", i);
            return false;
        }
    }
    
    return true;
}
#pragma optimize("", on)

///////////////////////////////////////////////////////////////////////  
//  CIdvMeshImporter::LoadVec

bool CIdvMeshImporter::LoadVec(CVec3 &cNew)
{
    return (fscanf(m_pFile, "%f %f %f ", &cNew[0], &cNew[1], &cNew[2]) == 3);
}

///////////////////////////////////////////////////////////////////////  
//  CIdvMeshImporter::LoadVec4

bool CIdvMeshImporter::LoadVec4(CVec &cNew)
{
    return (fscanf(m_pFile, "%f %f %f ", &cNew[0], &cNew[1], &cNew[2]) == 3);
}


///////////////////////////////////////////////////////////////////////  
//  CIdvMeshImporter::LoadMaterial

bool CIdvMeshImporter::LoadMaterial(SMaterial &sNew)
{
    if (!LoadVec4(sNew.cAmbient) || !LoadVec4(sNew.cDiffuse) || !LoadVec4(sNew.cSpecular))
        return false;

    int nBoolTemp = 0;
    if (fscanf(m_pFile, "%f %f %i ", &sNew.fShininess, &sNew.fAlpha, &nBoolTemp) != 3)
        return false;
    sNew.cAmbient[3] = sNew.fAlpha;
    sNew.cDiffuse[3] = sNew.fAlpha;
    sNew.cSpecular[3] = sNew.fAlpha;
    sNew.bHasTexture = nBoolTemp == 1;
    if (sNew.bHasTexture)
    {
        int nFileLength = 0;
        char strName[100];
        if (fscanf(m_pFile, "%i %i ", &sNew.nTextureMapChannel, &nFileLength) != 2)
            return false;
        if (fgets(strName, 100, m_pFile) == NULL)
            return false;
        sNew.strTextureFile = strName;
        sNew.strTextureFile.resize(sNew.strTextureFile.size() - 1);
        string strTexName = m_strFilePath + sNew.strTextureFile;
        sNew.dlTexture = LoadDDS(strTexName.c_str( ), true, false);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////  
//  CIdvMeshImporter::LoadUVWMap

bool CIdvMeshImporter::LoadUVWMap(SUVWMap &sNew)
{
    int nNumCoords = 0;
    if (fscanf(m_pFile, "%i ", &nNumCoords) != 1)
        return false;
    for (int i = 0; i < nNumCoords; ++i)
    {
        CVec3 vecNew;
        if (!LoadVec(vecNew))
            return false;

        sNew.vCoords.push_back(vecNew);
    }
    return true;
}   

///////////////////////////////////////////////////////////////////////  
//  CIdvMeshImporter::LoadStrip

bool CIdvMeshImporter::LoadStrip(SStrip &sNew)
{
    short nTemp = 0;
    if (fscanf(m_pFile, "%i %i ", &sNew.nMaterialIndex, &nTemp) != 2)
        return false;
    
    while (nTemp != -1)
    {
        sNew.vVertexIndexes.push_back(nTemp);

        unsigned short nNormIndex = 0;
        unsigned short nTexIndex = 0;
        if (fscanf(m_pFile, "%i %i %i ", &nNormIndex, &nTexIndex, &nTemp) != 3)
            return false;
        
        sNew.vNormalIndexes.push_back(nNormIndex);
        sNew.vTextureIndexes.push_back(nTexIndex);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////  
//  CIdvMeshImporter::LoadFace

bool CIdvMeshImporter::LoadFace(SFace &sNew)
{
    return (fscanf(m_pFile, "%i %i %i %i ", &sNew.nMaterialIndex, &sNew.nA, &sNew.nB, &sNew.nC) == 4);
}

