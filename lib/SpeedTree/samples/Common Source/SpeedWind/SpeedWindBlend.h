///////////////////////////////////////////////////////////////////////  
//	SpeedWindBlend.h
//
//	(c) 2004 IDV, Inc.
//
//	This class blends SpeedWind objects into a single set of wind outputs.
//
//
//	*** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//	This software is supplied under the terms of a license agreement or
//	nondisclosure agreement with Interactive Data Visualization and may
//	not be copied or disclosed except in accordance with the terms of
//	that agreement.
//
//      Copyright (c) 2001-2004 IDV, Inc.
//      All Rights Reserved.
//
//		IDV, Inc.
//		1233 Washington St. Suite 610
//		Columbia, SC 29201
//		Voice: (803) 799-1699
//		Fax:   (803) 931-0320
//		Web:   http://www.idvinc.com


/////////////////////////////////////////////////////////////////////////////
// Preprocessor

#pragma once
#include <vector>
#include "SpeedWind.h"
#include <math.h>
#include <string>

#define SET_FLOAT_ARRAY3(pArray, a, b, c) pArray[0] = a; pArray[1] = b; pArray[2] = c;
#define SET_FLOAT_ARRAY2(pArray, a, b) pArray[0] = a; pArray[1] = b;

/////////////////////////////////////////////////////////////////////////////
// Forward references


/////////////////////////////////////////////////////////////////////////////
// CSpeedWindBlend

class CSpeedWindBlend
{
public:
		// construction/destruction
										CSpeedWindBlend(void);

		// SpeedWind management
		void							AddSpeedWind(CSpeedWind* pSpeedWind)							{ m_vSpeedWinds.push_back(pSpeedWind); }
		void							ReplaceSpeedWind(CSpeedWind* pSpeedWind, unsigned int uiIndex)	{ m_vSpeedWinds[uiIndex] = pSpeedWind; }
		unsigned int					GetNumSpeedWinds(void) const									{ return static_cast<unsigned int>(m_vSpeedWinds.size( )); }
		CSpeedWind*						GetSpeedWind(unsigned int uiIndex) const						{ return m_vSpeedWinds[uiIndex]; }
		void							ClearSpeedWinds(void)											{ m_vSpeedWinds.clear( ); }
		void							RemoveSpeedWind(unsigned int uiIndex)							{ m_vSpeedWinds.erase(m_vSpeedWinds.begin( ) + uiIndex); }
		void							CreateMatricesAndRockAngles(void);


		// updating
		void							Advance(void);
		
		// matrix access
		unsigned int					GetNumWindMatrices(void) const									{ return m_uiNumWindMatrices; }
		const float*					GetWindMatrix(unsigned int uiIndex) const						{ return reinterpret_cast<const float*>(m_pWindMatrices[uiIndex].m_afData); }

		// leaf angle access
		unsigned int					GetNumLeafAngles(void) const									{ return m_uiNumLeafAngles; }
		const float*					GetLeafAngles(CSpeedWind::ELeafAngles eAngle) const				{ return m_pLeafAngles[eAngle]; }
		void							BuildLeafAngleMatrices(const float* pCameraDir);
		const float*					GetLeafAngleMatrix(unsigned int uiIndex) const					{ return reinterpret_cast<const float*>(m_pLeafAngleMatrices[uiIndex].m_afData); }

private:
		// SpeedWind inputs
		std::vector<CSpeedWind*>		m_vSpeedWinds;

		// matrices
		unsigned int					m_uiNumWindMatrices;
		CSpeedWindMatrix*				m_pWindMatrices;

		// leaf angles
		unsigned int					m_uiNumLeafAngles;
		float*							m_pLeafAngles[CSpeedWind::NUM_LEAF_ANGLES];
		CSpeedWindMatrix*				m_pLeafAngleMatrices;

};
