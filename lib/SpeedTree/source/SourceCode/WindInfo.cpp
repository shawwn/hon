///////////////////////////////////////////////////////////////////////  
//         Name: WindInfo.h
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

#include "Debug.h"
#include "TreeEngine.h"
#include "FileAccess.h"


///////////////////////////////////////////////////////////////////////  
//  SIdvWindInfo::SIdvWindInfo definition
//

SIdvWindInfo::SIdvWindInfo( ) : 
    m_cLeafFactors(0.5f, 0.5f),
    m_cLeafOscillation(0.0f, 0.0f, 0.0f),
    m_fStrength(0.0f)
{
}


///////////////////////////////////////////////////////////////////////  
//  SIdvWindInfo::Parse definition
//

void SIdvWindInfo::Parse(CTreeFileAccess& cFile)
{
    int nToken = cFile.ParseToken( );
    do
    {
        switch (nToken)
        {
            case File_Wind_Direction:
                (void) cFile.ParseVector3( ); // wind direction is no longer used
                break;
            case File_Wind_BranchOscillation:
                (void) cFile.ParseVector3( ); // branch oscillation is no longer used
                break;
            case File_Wind_LeafOscillation:
                m_cLeafOscillation = cFile.ParseVector3( );
                break;
            case File_Wind_BranchFactors:
                (void) cFile.ParseVector3( ); // branch factor is no longer used
                break;
            case File_Wind_LeafFactors:
                m_cLeafFactors = cFile.ParseVector3( );
                break;
            case File_Wind_Strength:
                m_fStrength = cFile.ParseFloat( );
                break;
            case File_Wind_Enabled:
                (void) cFile.ParseBool( ); // wind enabled is no longer used
                break;
            default:
                throw(IdvFileError("malformed general wind information"));
        }
        nToken = cFile.ParseToken( );
    } while (nToken != File_EndWindInfo);
}


///////////////////////////////////////////////////////////////////////  
//  SIdvWindInfo::Save definition
//

void SIdvWindInfo::Save(CTreeFileAccess& cFile) const
{
    cFile.SaveToken(File_BeginWindInfo);

    cFile.SaveToken(File_Wind_Direction);
    cFile.SaveVector3(CVec3( )); // wind direction is no longer used

    cFile.SaveToken(File_Wind_BranchOscillation);
    cFile.SaveVector3(CVec3( )); // branch oscillation is no longer used

    cFile.SaveToken(File_Wind_LeafOscillation);
    cFile.SaveVector3(m_cLeafOscillation);

    cFile.SaveToken(File_Wind_BranchFactors);
    cFile.SaveVector3(CVec3( )); // branch factors is no longer used

    cFile.SaveToken(File_Wind_LeafFactors);
    cFile.SaveVector3(m_cLeafFactors);

    cFile.SaveToken(File_Wind_Strength);
    cFile.SaveFloat(m_fStrength);

    cFile.SaveToken(File_Wind_Enabled);
    cFile.SaveBool(false); // wind enabled is no longer used

    cFile.SaveToken(File_EndWindInfo);
}