///////////////////////////////////////////////////////////////////////  
//         Name: Instances.h
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

#include "LibGlobals_Source/IdvGlobals.h"
#include "LibVector_Source/IdvVector.h"
#pragma warning (push,3)
#include <vector>
#pragma warning (pop)


class CSpeedTreeRT;

///////////////////////////////////////////////////////////////////////  
//  struct STreeInstanceData definition
//
//  If a new CSpeedTreeRT is created that is an instance of another tree,
//  this structure stores the instance's overriding values.

struct STreeInstanceData
{
        STreeInstanceData( ) :
            m_pParent(NULL),
            m_fLodLevel(1.0f)
        {
        }

        const CSpeedTreeRT* m_pParent;      // which tree this is an instance of
        CVec3               m_cPosition;    // position for this instance, different from parent
        float               m_fLodLevel;    // this instance's LOD value
};



///////////////////////////////////////////////////////////////////////  
//  struct SInstanceList definition
//
//  A single tree may have multiple instances.  This structure tracks all
//  of the instances of a single (m_vInstances), as well as all of the
//  unique trees in an entire scene (m_vUniqueTrees).

struct SInstanceList
{
        std::vector<CSpeedTreeRT*>  m_vInstances;       // list of instances of a particular tree
static  std::vector<CSpeedTreeRT*>  m_vUniqueTrees;     // list of all trees in the scene that aren't instances
};




