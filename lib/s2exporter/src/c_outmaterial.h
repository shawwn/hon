// (C)2005 S2 Games
// c_outmaterial.h
//
//=============================================================================
#ifndef __C_OUTMATERIAL_H__
#define __C_OUTMATERIAL_H__

//=============================================================================
// Declarations / Definitions
//=============================================================================
struct SMapParams
{
	tstring	sName;
	bool	bRepeatU, bRepeatV;
	float	fScaleU, fScaleV;
};

class INode;
class Mtl;
class CXMLDoc;
//=============================================================================

//=============================================================================
// COutMaterial
//=============================================================================
class COutMaterial
{
private:
	COutMesh	*m_pMesh;
	bool	m_bIsValid;
	tstring	m_sStatus;

	// Material data
	tstring	m_sName;

	tstring m_sVSH, m_sPSH;

	// Shader properties
	int		m_iTransparency;
	bool	m_bTranslucent;
	bool	m_bDoubleSided;
	bool	m_bAlphaTest;
	bool	m_bOcclusion;
	bool	m_bLightmap;
	bool	m_bNormalmap;
	bool	m_bLighting;
	bool	m_bReflect;
	float	m_fReflect;

	// Global parameters
	CVec3f	m_vecDiffuseColor;
	float	m_fSpecularLevel;
	float	m_fGlossiness;
	float	m_fBumpLevel;
	float	m_fOpacity;

	// Maps
	map<tstring, SMapParams>	m_mapTextures;

public:
	~COutMaterial()						{}
	COutMaterial();
	COutMaterial(COutMesh *pMesh, INode *pNode, TimeValue time);

	const tstring&	GetName() const		{ return m_sName; }

	void			Validate()			{ m_bIsValid = true; }
	void			Invalidate()		{ m_bIsValid = false; }
	bool			IsValid() const		{ return m_bIsValid; }

	void	SetVSH(const tstring &sName)	{ m_sVSH = sName; }
	void	SetPSH(const tstring &sName)	{ m_sPSH = sName; }

	const tstring&	GetVSH()	{ return m_sVSH; }
	const tstring&	GetPSH()	{ return m_sPSH; }

	bool	AddMap(StdMat2 *pMaterial, const tstring &sName, int iMaterialID, TimeValue time);
	bool	AddMap(const tstring &sName, const tstring &sImage, bool bRepeatU, bool bRepeatV, float fScaleU, float fScaleV);

	void	AddSampler(CXMLDoc &xmlMaterial, const tstring &sName, const tstring &sMapName,
						bool bRepeatU, bool bRepeatV, float fScaleU, float fScaleV, int iFPS,
						bool bMipMaps, bool bFullQuality, bool bNoCompress,	bool bFiltering,
						bool bBorder);

	void	AddSamplerCube(CXMLDoc &xmlMaterial, const tstring &sName, const tstring &sMapName,
						bool bMipMaps, bool bFullQuality, bool bNoCompress, bool bFiltering);

	void	AddColorPhase(CXMLDoc &xmlMaterial, const tstring &sVSH,
						const tstring &sPSH, int iTransparency, bool bTranslucent, bool bDoubleSided, bool bAlphaTest);

	void	AddShadowPhase(CXMLDoc &xmlMaterial, const tstring &sVSH,
						const tstring &sPSH, int iTransparency, bool bDoubleSided, bool bAlphaTest);

	void	AddDepthPhase(CXMLDoc &xmlMaterial, const tstring &sVSH,
						const tstring &sPSH, int iTransparency, bool bDoubleSided, bool bAlphaTest);

	void	AddFadePhase(CXMLDoc &xmlMaterial, const tstring &sVSH,
						const tstring &sPSH, int iTransparency, bool bDoubleSided, bool bAlphaTest);

	tstring	WriteFile(const tstring &sPath, bool bCopyMapFiles);
};
//=============================================================================

#endif //__C_OUTMATERIAL_H__
