///////////////////////////////////////////////////////////////////////  
//         Name: StructsSupport.h
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//      Copyright (c) 2001-2004 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      1233 Washington St. Suite 610
//      Columbia, SC 29201
//      Voice: (803) 799-1699
//      Fax:   (803) 931-0320
//      Web:   http://www.idvinc.com
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may not 
//  be copied or disclosed except in accordance with the terms of that 
//  agreement.

#pragma once

//
//  forward references
//

class CIdvBranch;


///////////////////////////////////////////////////////////////////////  
//  struct SIdvBranchVertex declaration
//
//  Used during tree branch computation, SIdvBranchVertex are used as the
//  branch vertexes place holders before skinning.

struct SIdvBranchVertex
{
        CVec3               m_cDirection;           // direction the branch is growing in
        CVec3               m_cPos;                 // (x,y,z) position of this branch vertex
        float               m_fRadius;              // the computed radius at this point along the branch
        CRotTransform       m_cTrans;               // rotation matrix that governs the direction vector
        float               m_fRunningLength;       // running length of the branch
        float               m_fWindWeight;          // wind weight of all vertices in the corresponding cross-section
};



///////////////////////////////////////////////////////////////////////  
//  struct SIdvLeafTexture declaration
//
//  Used to store attributes of each individual leaf texture

struct SIdvLeafTexture
{
        SIdvLeafTexture( ) :
            m_bBlossom(false),
            m_cColor(0.8f, 0.8f, 0.8f),
            m_fColorVariance(0.2f),
            m_cOrigin(0.5f, 1.0f),
            m_cSize(0.12f, 0.12f),
            m_cSizeUsed(10.0f, 10.0f)
        {
        }

        bool                m_bBlossom;             // flag that specifies if this texture can be used as a blossom texture
        CVec3               m_cColor;               // base color for the leaf texture  
        float               m_fColorVariance;       // color variance allowed for the leaf texture
        CIdvFilename        m_strFilename;          // original texture map filename specified in SpeedTreeCAD
        CVec3               m_cOrigin;              // two-dimensional (s, t) origin of the leaf texture map
        CVec3               m_cSize;                // dimension of the texture, [0] = width, [1] = height
        CVec3               m_cSizeUsed;            // adjusted size, based on user-specified leaf size
};


///////////////////////////////////////////////////////////////////////  
//  struct SIdvBranch declaration
//
//  This structure encapsulates a child branch

struct SIdvBranch
{
        SIdvBranch( ) :
            m_nPreVertexIndex(0),
            m_fPercent(0.0f),
            m_pBranch(NULL)
        {
        }

        int         m_nPreVertexIndex;              // the branch vertex of the parent immediately before when this child was spawned
        float       m_fPercent;                     // percent between the two parent's vertexes where this child was created
        CIdvBranch* m_pBranch;                      // branch this SIdvBranch instance encapsulates
};


///////////////////////////////////////////////////////////////////////  
//  struct SIdvBranchFlare declaration
//
//  This structure encapsulates a child branch

struct SIdvBranchFlare
{
        SIdvBranchFlare( ) :
            m_fAngle(0.0f),
            m_fRadialInfluence(0.0f),
            m_fRadialExponent(1.0f),
            m_fLengthInfluence(0.0f),
            m_fLengthExponent(1.0f),
            m_fDistance(0.0f)
        {
        }

        float       m_fAngle;                       // angle of the center of the flare
        float       m_fRadialInfluence;             // angle within which branch vertices are affected by this flare
        float       m_fRadialExponent;              // controls sharpness of flare points 
        float       m_fLengthInfluence;             // distance along the branch that vertices are affected by this flare
        float       m_fLengthExponent;              // controls lengthwise profile of flares
        float       m_fDistance;                    // distance of flare at point of maximum influence

        float       Distance(float fInputAngle, float fInputProgress) const
        {
            float fDistance = 0.0f;
            // compute angle difference (accounting for 360/0 crossing)
            float fTestAngle = m_fAngle;
            float fAngleDifference = fabsf(fInputAngle - fTestAngle);
            if (fAngleDifference > c_fPi)
            {
                if (fInputAngle < fTestAngle)
                    fInputAngle += c_fTwoPi;
                else
                    fTestAngle += c_fTwoPi;
            }
            fAngleDifference = fabsf(fInputAngle - fTestAngle);

            if (fAngleDifference < m_fRadialInfluence)
            {
                float fProgressDifference = m_fLengthInfluence - fInputProgress;
                if (fProgressDifference > 0.0f)
                {
                    float fRadialContribution = 1.0f - (fAngleDifference / m_fRadialInfluence);
                    fRadialContribution = powf(fRadialContribution, m_fRadialExponent);

                    float fVerticalContribution = fProgressDifference / m_fLengthInfluence;
                    fVerticalContribution = powf(fVerticalContribution, m_fLengthExponent);
                    fDistance = m_fDistance * fRadialContribution * fVerticalContribution;
                }

            }

            return fDistance;
        }
};

