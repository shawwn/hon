///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeMaterial Class
//
//  (c) 2003 IDV, Inc.
//
//  This class is provided to illustrate one way to incorporate
//  SpeedTreeRT into an OpenGL application.  All of the SpeedTreeRT
//  calls that must be made on a per tree basis are done by this class.
//  Calls that apply to all trees (i.e. static SpeedTreeRT functions)
//  are made in the functions in main.cpp.
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

#pragma once

///////////////////////////////////////////////////////////////////////  
//  class CSpeedTreeMaterial declaration/definiton

class CSpeedTreeMaterial
{
public:
        CSpeedTreeMaterial( )
        {
            m_afAmbient[0] = m_afAmbient[1] = m_afAmbient[2] = m_afAmbient[3] = 1.0f;
            m_afDiffuse[0] = m_afDiffuse[1] = m_afDiffuse[2] = m_afDiffuse[3] = 1.0f;
            m_afSpecular[0] = m_afSpecular[1] = m_afSpecular[2] = m_afSpecular[3] = 1.0f;
            m_afEmission[0] = m_afEmission[1] = m_afEmission[2] = m_afEmission[3] = 1.0f;
            m_fShininess = 5.0f;
        }

        void Set(const float* pMaterialArray)
        {
            memcpy(m_afDiffuse, pMaterialArray, 3 * sizeof(float));
            memcpy(m_afAmbient, pMaterialArray + 3, 3 * sizeof(float));
            memcpy(m_afSpecular, pMaterialArray + 6, 3 * sizeof(float));
            memcpy(m_afEmission, pMaterialArray + 9, 3 * sizeof(float));
            m_fShininess = pMaterialArray[12];
        }

        void Activate(void) const
        {
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_afAmbient);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_afDiffuse);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_afSpecular);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, m_afEmission);
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m_fShininess);
        }

private:
        float   m_afAmbient[4];         // ambient color (r, g, b, a) [each range from 0.0 to 1.0]
        float   m_afDiffuse[4];         // diffuse color (r, g, b, a) [each range from 0.0 to 1.0]
        float   m_afSpecular[4];        // specular color (r, g, b, a) [each range from 0.0 to 1.0]
        float   m_afEmission[4];        // emission color (r, g, b, a) [each range from 0.0 to 1.0]
        float   m_fShininess;           // shininess [ranges from 0.0 to 128.0]
};
