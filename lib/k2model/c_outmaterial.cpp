// c_outmaterial.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2model_common.h"

#include "c_outmaterial.h"

#include "c_outmesh.h"

#include "c_xmldoc.h"
#include "c_filehandle.h"
//=============================================================================

/*====================
  COutMaterial::COutMaterial
  ====================*/
COutMaterial::COutMaterial() :
m_pMesh(NULL),
m_bIsValid(false),

m_sVSH(_T("mesh_color")),
m_sPSH(_T("mesh_color")),
m_vecDiffuseColor(0.0f, 0.0f, 0.0f),
m_fSpecularLevel(0.0f),
m_fGlossiness(0.0f),
m_fBumpLevel(1.0f),

m_bDoubleSided(false),
m_bAlphaTest(false),
m_bOcclusion(false),
m_bLighting(true),

m_bReflect(false),
m_fReflect(0.0f)
{
	Validate();
}


/*====================
  COutMaterial::AddMap
  ====================*/
bool	COutMaterial::AddMap(const tstring &sName, const tstring &sImage, bool bRepeatU, bool bRepeatV, float fScaleU, float fScaleV)
{
	SMapParams params;
	params.sName = sImage;
	params.bRepeatU = bRepeatU;
	params.bRepeatV = bRepeatV;
	params.fScaleU = fScaleU;
	params.fScaleV = fScaleV;
	m_mapTextures[sName] = params;

	return true;
}


/*====================
  COutMaterial::AddSampler
  ====================*/
void	COutMaterial::AddSampler(CXMLDoc &xmlMaterial, const tstring &sName, const tstring &sMapName,
								 bool bRepeatU, bool bRepeatV, float fScaleU, float fScaleV,
								 int iFPS, bool bMipMaps, bool bFullQuality, bool bNoCompress,
								 bool bFiltering, bool bBorder)
{
	if (sMapName.empty())
		return;

	xmlMaterial.NewNode("sampler");
		xmlMaterial.AddProperty("name", sName);
		xmlMaterial.AddProperty("texture", Filename_StripPath(Filename_SanitizePath(sMapName)));
		xmlMaterial.AddProperty("repeat_u", XtoA(bRepeatU));
		xmlMaterial.AddProperty("repeat_v", XtoA(bRepeatV));
		xmlMaterial.AddProperty("scale_u", XtoA(fScaleU));
		xmlMaterial.AddProperty("scale_v", XtoA(fScaleV));
		xmlMaterial.AddProperty("fps", iFPS);
		xmlMaterial.AddProperty("mipmaps", XtoA(bMipMaps));
		xmlMaterial.AddProperty("fullquality", XtoA(bFullQuality));
		xmlMaterial.AddProperty("nocompress", XtoA(bNoCompress));
		xmlMaterial.AddProperty("filtering", XtoA(bFiltering));
		xmlMaterial.AddProperty("border", XtoA(bBorder));
	xmlMaterial.EndNode();
}


/*====================
  COutMaterial::AddSamplerCube
  ====================*/
void	COutMaterial::AddSamplerCube(CXMLDoc &xmlMaterial, const tstring &sName, const tstring &sMapName,
								 bool bMipMaps, bool bFullQuality, bool bNoCompress, bool bFiltering)
{
	if (sMapName.empty())
		return;

	xmlMaterial.NewNode("samplerCUBE");
		xmlMaterial.AddProperty("name", sName);
		xmlMaterial.AddProperty("texture", Filename_StripPath(Filename_SanitizePath(sMapName)));
		xmlMaterial.AddProperty("mipmaps", XtoA(bMipMaps));
		xmlMaterial.AddProperty("fullquality", XtoA(bFullQuality));
		xmlMaterial.AddProperty("nocompress", XtoA(bNoCompress));
		xmlMaterial.AddProperty("filtering", XtoA(bFiltering));
	xmlMaterial.EndNode();
}



/*====================
  COutMaterial::AddColorPhase
  ====================*/
void	COutMaterial::AddColorPhase(CXMLDoc &xmlMaterial, const tstring &sVSH,
							   const tstring &sPSH, int iTransparency, bool bTranslucent, bool bDoubleSided, bool bAlphaTest)
{
	xmlMaterial.NewNode("phase");
		xmlMaterial.AddProperty("name", "color");
		xmlMaterial.AddProperty("vs", sVSH);
		xmlMaterial.AddProperty("ps", sPSH);

		if (false)
		{
			xmlMaterial.AddProperty("srcblend", "BLEND_ONE");
			xmlMaterial.AddProperty("dstblend", "BLEND_ONE");
			xmlMaterial.AddProperty("translucent", "true");
		}
		else if (bTranslucent)
		{
			xmlMaterial.AddProperty("srcblend", "BLEND_SRC_ALPHA");
			xmlMaterial.AddProperty("dstblend", "BLEND_ONE_MINUS_SRC_ALPHA");
			xmlMaterial.AddProperty("translucent", "true");
		}
		else
		{
			xmlMaterial.AddProperty("srcblend", "BLEND_ONE");
			xmlMaterial.AddProperty("dstblend", "BLEND_ZERO");
			xmlMaterial.AddProperty("translucent", "false");
		}

		xmlMaterial.AddProperty("cull", bDoubleSided ? "CULL_NONE" : "CULL_BACK");

		if (bAlphaTest)
			xmlMaterial.AddProperty("alphatest", "true");

		xmlMaterial.AddProperty("layer", "0");

		for (map<tstring, SMapParams>::iterator it = m_mapTextures.begin(); it != m_mapTextures.end(); ++it)
		{
			if (it->first == _T("cube"))
				AddSamplerCube(xmlMaterial, it->first, it->second.sName, true, false, false, true);
			else
				AddSampler(xmlMaterial, it->first, it->second.sName, it->second.bRepeatU, it->second.bRepeatV,
					it->second.fScaleU, it->second.fScaleV, 15, true, false, false, true, false);
		}

	xmlMaterial.EndNode();
}


/*====================
  COutMaterial::AddShadowPhase
  ====================*/
void	COutMaterial::AddShadowPhase(CXMLDoc &xmlMaterial, const tstring &sVSH,
							   const tstring &sPSH, int iTransparency, bool bDoubleSided, bool bAlphaTest)
{
	xmlMaterial.NewNode("phase");
		xmlMaterial.AddProperty("name", "shadow");
		xmlMaterial.AddProperty("vs", sVSH);
		xmlMaterial.AddProperty("ps", sPSH);
		xmlMaterial.AddProperty("srcblend", "BLEND_ONE");
		xmlMaterial.AddProperty("dstblend", "BLEND_ZERO");
		xmlMaterial.AddProperty("translucent", "false");
		xmlMaterial.AddProperty("cull", bDoubleSided ? "CULL_NONE" : "CULL_BACK");

		if (bAlphaTest)
		{
			xmlMaterial.AddProperty("alphatest", "true");

			map<tstring, SMapParams>::iterator findit(m_mapTextures.find("diffuse"));

			if (findit != m_mapTextures.end())
			{
				AddSampler(xmlMaterial, "opacity", findit->second.sName, findit->second.bRepeatU, findit->second.bRepeatV,
					findit->second.fScaleU, findit->second.fScaleV, 15, true, false, false, true, false);
			}
		}

	xmlMaterial.EndNode();
}


/*====================
  COutMaterial::AddDepthPhase
  ====================*/
void	COutMaterial::AddDepthPhase(CXMLDoc &xmlMaterial, const tstring &sVSH,
							   const tstring &sPSH, int iTransparency, bool bDoubleSided, bool bAlphaTest)
{
	xmlMaterial.NewNode("phase");
		xmlMaterial.AddProperty("name", "depth");
		xmlMaterial.AddProperty("vs", sVSH);
		xmlMaterial.AddProperty("ps", sPSH);
		xmlMaterial.AddProperty("srcblend", "BLEND_ONE");
		xmlMaterial.AddProperty("dstblend", "BLEND_ZERO");
		xmlMaterial.AddProperty("translucent", "false");
		xmlMaterial.AddProperty("cull", bDoubleSided ? "CULL_NONE" : "CULL_BACK");

		if (bAlphaTest)
		{
			xmlMaterial.AddProperty("alphatest", "true");

			map<tstring, SMapParams>::iterator findit(m_mapTextures.find("diffuse"));

			if (findit != m_mapTextures.end())
			{
				AddSampler(xmlMaterial, "opacity", findit->second.sName, findit->second.bRepeatU, findit->second.bRepeatV,
					findit->second.fScaleU, findit->second.fScaleV, 15, true, false, false, true, false);
			}
		}

	xmlMaterial.EndNode();
}


/*====================
  COutMaterial::AddFadePhase
  ====================*/
void	COutMaterial::AddFadePhase(CXMLDoc &xmlMaterial, const tstring &sVSH,
							   const tstring &sPSH, int iTransparency, bool bDoubleSided, bool bAlphaTest)
{
	xmlMaterial.NewNode("phase");
		xmlMaterial.AddProperty("name", "fade");
		xmlMaterial.AddProperty("vs", sVSH);
		xmlMaterial.AddProperty("ps", sPSH);

		if (false)
		{
			xmlMaterial.AddProperty("srcblend", "BLEND_ONE");
			xmlMaterial.AddProperty("dstblend", "BLEND_ONE");
			xmlMaterial.AddProperty("translucent", "true");
		}
		else
		{
			xmlMaterial.AddProperty("srcblend", "BLEND_SRC_ALPHA");
			xmlMaterial.AddProperty("dstblend", "BLEND_ONE_MINUS_SRC_ALPHA");
			xmlMaterial.AddProperty("translucent", "true");
		}

		xmlMaterial.AddProperty("cull", bDoubleSided ? "CULL_NONE" : "CULL_BACK");

		if (bAlphaTest)
			xmlMaterial.AddProperty("alphatest", "true");
		
		xmlMaterial.AddProperty("layer", "0");

		for (map<tstring, SMapParams>::iterator it = m_mapTextures.begin(); it != m_mapTextures.end(); ++it)
		{
			if (it->first == _T("cube"))
				AddSamplerCube(xmlMaterial, it->first, it->second.sName, true, false, false, true);
			else
				AddSampler(xmlMaterial, it->first, it->second.sName, it->second.bRepeatU, it->second.bRepeatV,
					it->second.fScaleU, it->second.fScaleV, 15, true, false, false, true, false);
		}

	xmlMaterial.EndNode();
}


/*====================
  COutMaterial::WriteFile
  ====================*/
tstring	COutMaterial::WriteFile(const tstring &sPath, bool bCopyMapFiles)
{
	tstring sReturn;

	CXMLDoc	xmlMaterial;

	tstring sMeshType;

	if (m_pMesh->tverts[0] && m_pMesh->tverts[1])
		sMeshType = _T("mesh2");
	else if (m_pMesh->tverts[0])
		sMeshType = _T("mesh");
	else
		sMeshType = _T("simple");

	tstring sMeshTypeColor(sMeshType);
	if (m_pMesh->colors[0])
		sMeshTypeColor += _T("_colored");

	xmlMaterial.NewNode("material");

		xmlMaterial.NewNode("parameters");
			xmlMaterial.AddProperty("vDiffuseColor", XtoA(m_vecDiffuseColor));
			xmlMaterial.AddProperty("fSpecularLevel", XtoA(m_fSpecularLevel));
			xmlMaterial.AddProperty("fGlossiness", XtoA(m_fGlossiness, 0, 0, 0));
			xmlMaterial.AddProperty("fOpacity", XtoA(m_fOpacity));
			if (m_fReflect != 0.0f)
				xmlMaterial.AddProperty("fReflect", XtoA(m_fReflect));
		xmlMaterial.EndNode();

		if (false)
			AddShadowPhase(xmlMaterial, sMeshType + (m_bAlphaTest ?  "_shadow_opacity" : "_shadow"), sMeshType + (m_bAlphaTest ? "_shadow_opacity" : "_shadow"), 0, m_bDoubleSided, m_bAlphaTest);

		AddDepthPhase(xmlMaterial, sMeshType + (m_bAlphaTest ? "_depth_opacity" : "_depth"), sMeshType + (m_bAlphaTest ? "_depth_opacity" : "_depth"), 0, m_bDoubleSided, m_bAlphaTest);
		AddColorPhase(xmlMaterial, m_bLighting ? sMeshTypeColor + _T("_shadowed") + (m_bReflect ? _T("_reflect") : _T("")) + (!m_bNormalmap ? _T("_flat") : _T("")) : sMeshType + _T("_unlit"), m_bLighting ? sMeshType + _T("_shadowed") + (m_bReflect ? _T("_reflect") : _T("")) + (!m_bNormalmap ? _T("_flat") : _T("")) + (m_bOcclusion ? _T("_occlusion") : _T("")) + (m_bLightmap ? _T("_lightmap") : _T("")) : sMeshType + _T("_unlit"), m_iTransparency, m_bTranslucent, m_bDoubleSided, m_bAlphaTest);
		AddFadePhase(xmlMaterial, m_bLighting ? sMeshTypeColor + _T("_shadowed") + (m_bReflect ? _T("_reflect") : _T("")) + (!m_bNormalmap ? _T("_flat") : _T("")) : sMeshType + _T("_unlit"), m_bLighting ? sMeshType + _T("_shadowed") + (m_bReflect ? _T("_reflect") : _T("")) + (!m_bNormalmap ? _T("_flat") : _T("")) + (m_bOcclusion ? _T("_occlusion") : _T("")) + (m_bLightmap ? _T("_lightmap") : _T("")) : sMeshType + _T("_unlit"), m_iTransparency, m_bDoubleSided, m_bAlphaTest);

	xmlMaterial.EndNode();

	CFileHandle	hFile(sPath + _T("/") + m_sName + ".material", FILE_WRITE | FILE_TEXT);
	xmlMaterial.WriteFile(hFile);
	sReturn = "Wrote file " + m_sName + ".material\r\n";

	// Copy the actual image files to the output location
	if (!bCopyMapFiles)
		return sReturn;

	for (map<tstring, SMapParams>::iterator it = m_mapTextures.begin(); it != m_mapTextures.end(); ++it)
	{
		if (it->second.sName[0] == _T('$'))
			continue;

		tstring sMapFile = Filename_SanitizePath(it->second.sName);
		if (sMapFile.empty())
			continue;

		if (!CopyFile(sMapFile.c_str(), tstring(sPath + Filename_StripPath(sMapFile)).c_str(), FALSE))
			sReturn += "WARNING: Unable to copy texture " + Filename_StripPath(sMapFile) + " to " + sPath + "\r\n";
		else
			sReturn += "Copied texture " + Filename_StripPath(sMapFile) + " to " + sPath + "\r\n";
	}

	return sReturn;
}
