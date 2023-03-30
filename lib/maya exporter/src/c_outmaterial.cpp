// (C)2008 S2 Games
// c_outmaterial.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "exporter_common.h"

#include "c_outmaterial.h"

#include "c_outmesh.h"

#include "c_xmldoc.h"
#include "c_filehandle.h"
//=============================================================================

/*====================
  COutMaterial::COutMaterial
  ====================*/
COutMaterial::COutMaterial() :
m_bIsValid(false),
m_sStatus("Empty")
{
}

COutMaterial::COutMaterial(COutMesh *pMesh, MItDag &itDag) :
m_pMesh(pMesh),
m_bIsValid(false),
m_sStatus("Initializing..."),

m_sVSH("mesh_color"),
m_sPSH("mesh_color"),
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
	MFnMesh fnMesh(itDag.currentItem());
	MObjectArray aShaders;
	MIntArray aIndices;
	fnMesh.getConnectedShaders(0, aShaders, aIndices);
	cout << "Shaders: " << aShaders.length() << endl;
	for (uint uiShaderIndex(0); uiShaderIndex < aShaders.length(); ++uiShaderIndex)
	{
		MFnDependencyNode dnShader(aShaders[uiShaderIndex]);
		MStatus status;
		MPlug plugShader(dnShader.findPlug("surfaceShader", &status));
		if (status != MS::kSuccess)
			continue;

		MPlugArray aPlugs;
		plugShader.connectedTo(aPlugs, true, false, &status);
		if (status != MS::kSuccess)
			continue;

		if (aPlugs.length() == 0)
			continue;

		MObject node(aPlugs[0].node());
		if (node.hasFn(MFn::kLambert))
		{
			MFnLambertShader fnShader(node);
			cout << "Shader: " << fnShader.name().asChar() << endl;
			
			m_sName = fnShader.name().asChar();
			m_iTransparency = TRANSP_NONE;
			m_bDoubleSided = false;
			m_vecDiffuseColor.x = fnShader.color().r;
			m_vecDiffuseColor.y = fnShader.color().g;
			m_vecDiffuseColor.z = fnShader.color().b;
			
			if (node.hasFn(MFn::kPhong))
				m_fSpecularLevel = MFnPhongShader(node).cosPower();
			else
				m_fSpecularLevel = 0.0f;

			m_fGlossiness = 0.0f; //pow(2.0f, pMaterial->GetShininess(time) * 10.0f) * 4.0f;
			m_fBumpLevel = 0.0f; //pMaterial->GetTexmapAmt(ID_BU, time);
			m_fOpacity = 1.0f; //pMaterial->GetOpacity(time);
			m_bAlphaTest = false; //pMaterial->GetSubTexmap(ID_OP) != NULL;
			m_bOcclusion = true; //pMaterial->GetSubTexmap(ID_AM) != NULL;
			m_bLightmap = false; //pMaterial->GetSubTexmap(ID_SI) != NULL;
			m_bNormalmap = false; //pMaterial->GetSubTexmap(ID_BU) != NULL;
			m_bLighting = false; //pMaterial->GetSelfIllum(time) != 1.0f && m_pMesh->normals != NULL;
			if (node.hasFn(MFn::kPhong))
			{
				m_bReflect = MFnPhongShader(node).reflectivity() > 0.0f;
				m_fReflect = MFnPhongShader(node).reflectivity();
			}
			else
			{
				m_bReflect = false;
				m_fReflect = 0.0f;
			}

			m_bTranslucent = false;

			// Maps
			AddMap("diffuse", "$white", true, true, 1.0f, 1.0f);
#if 0
			if (!AddMap(pMaterial, "diffuse", ID_DI))
				AddMap("diffuse", "$white", true, true, 1.0f, 1.0f);

			AddMap(pMaterial, "specular", ID_SS, time);
			AddMap(pMaterial, "gloss", ID_SH, time);
			AddMap(pMaterial, "lightmap", ID_SI, time);
			AddMap(pMaterial, "occlusion", ID_AM, time);
			AddMap(pMaterial, "normalmap", ID_BU, time);
			AddMap(pMaterial, "cube", ID_RL, time);
#endif
		}
	}

	Validate();
	m_sStatus = "OK";
}


/*====================
  COutMaterial::AddMap
  ====================*/
bool	COutMaterial::AddMap(MObject &material, const tstring &sName, int iMaterialID)
{
#if 0
	Texmap *pBaseTex = pMaterial->GetSubTexmap(iMaterialID);
	if (pBaseTex == NULL || pMaterial->GetTexmapAmt(iMaterialID, time) == 0.0f)
		return false;

	//
	// Normal Bump
	//

	Class_ID cID(pBaseTex->ClassID());

	if (iMaterialID == ID_BU && cID == Class_ID(0x243e22c6, 0x63f6a014))
	{
		Texmap *pSubBaseTex = pBaseTex->GetSubTexmap(0);

		if (!pSubBaseTex || pSubBaseTex->ClassID() != Class_ID(BMTEX_CLASS_ID, 0))
			return false;

		BitmapTex *pTex = static_cast<BitmapTex*>(pSubBaseTex);
		SMapParams params;
		params.sName = pTex->GetMapName();
		params.bRepeatU = (pTex->GetTextureTiling() & U_WRAP) != 0;
		params.bRepeatV = (pTex->GetTextureTiling() & V_WRAP) != 0;
		params.fScaleU = pTex->GetUVGen()->GetUScl(time);
		params.fScaleV = pTex->GetUVGen()->GetVScl(time);
		m_mapTextures[sName] = params;

		return false;
	}


	//
	// Bitmap
	//

	if (pBaseTex->ClassID() != Class_ID(BMTEX_CLASS_ID, 0))
		return false;

	BitmapTex *pTex = static_cast<BitmapTex*>(pBaseTex);
	SMapParams params;
	params.sName = pTex->GetMapName();
	params.bRepeatU = (pTex->GetTextureTiling() & U_WRAP) != 0;
	params.bRepeatV = (pTex->GetTextureTiling() & V_WRAP) != 0;
	params.fScaleU = pTex->GetUVGen()->GetUScl(time);
	params.fScaleV = pTex->GetUVGen()->GetVScl(time);
	m_mapTextures[sName] = params;
#endif

	return true;
}

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

	xmlMaterial.NewNode("samplercube");
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

		if (iTransparency == TRANSP_ADDITIVE)
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

		if (iTransparency == TRANSP_ADDITIVE)
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
		sMeshTypeColor += _T("c");

	xmlMaterial.NewNode("material");

		xmlMaterial.NewNode("parameters");
			xmlMaterial.AddProperty("vDiffuseColor", XtoA(m_vecDiffuseColor));
			xmlMaterial.AddProperty("fSpecularLevel", XtoA(m_fSpecularLevel));
			xmlMaterial.AddProperty("fGlossiness", XtoA(m_fGlossiness, 0, 0, 0));
			xmlMaterial.AddProperty("fOpacity", XtoA(m_fOpacity));
			if (m_fReflect != 0.0f)
				xmlMaterial.AddProperty("fReflect", XtoA(m_fReflect));
		xmlMaterial.EndNode();

		if (m_iTransparency != TRANSP_ADDITIVE)
			AddShadowPhase(xmlMaterial, sMeshType + (m_bAlphaTest ?  "_shadow_opacity" : "_shadow"), sMeshType + (m_bAlphaTest ? "_shadow_opacity" : "_shadow"), 0, m_bDoubleSided, m_bAlphaTest);

		AddDepthPhase(xmlMaterial, sMeshType + (m_bAlphaTest ? "_depth_opacity" : "_depth"), sMeshType + (m_bAlphaTest ? "_depth_opacity" : "_depth"), 0, m_bDoubleSided, m_bAlphaTest);
		AddColorPhase(xmlMaterial, m_bLighting ? sMeshTypeColor + _T("_color") + (m_bReflect ? _T("_enviro") : _T("")) + (!m_bNormalmap ? _T("_flat") : _T("")) : sMeshType + _T("_unlit"), m_bLighting ? sMeshType + _T("_color") + (m_bReflect ? _T("_enviro_reflect") : _T("")) + (!m_bNormalmap ? _T("_flat") : _T("")) + (m_bOcclusion ? _T("_occlusion") : _T("")) + (m_bLightmap ? _T("_lightmap") : _T("")) : sMeshType + _T("_unlit"), m_iTransparency, m_bTranslucent, m_bDoubleSided, m_bAlphaTest);
		AddFadePhase(xmlMaterial, m_bLighting ? sMeshTypeColor + _T("_color") + (m_bReflect ? _T("_enviro") : _T("")) + (!m_bNormalmap ? _T("_flat") : _T("")) : sMeshType + _T("_unlit"), m_bLighting ? sMeshType + _T("_color") + (m_bReflect ? _T("_enviro_reflect") : _T("")) + (!m_bNormalmap ? _T("_flat") : _T("")) + (m_bOcclusion ? _T("_occlusion") : _T("")) + (m_bLightmap ? _T("_lightmap") : _T("")) : sMeshType + _T("_unlit"), m_iTransparency, m_bDoubleSided, m_bAlphaTest);

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
