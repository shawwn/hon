// (C)2006 S2 Games
// i_debugrenderer.cpp
//
// Debug-style batch renderer base class (points/lines/boxes)
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9g_main.h"
#include "d3d9g_util.h"
#include "d3d9g_material.h"
#include "d3d9g_scene.h"
#include "d3d9g_state.h"
#include "d3d9g_shader.h"
#include "d3d9g_terrain.h"
#include "d3d9g_texture.h"
#include "i_debugrenderer.h"
#include "c_shaderregistry.h"

#include "../k2/c_world.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_material.h"
#include "../k2/intersection.h"
#include "../k2/c_sphere.h"
#include "../k2/c_camera.h"
#include "../k2/c_sceneentity.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_scenelight.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
//=============================================================================

/*====================
  IDebugRenderer::IDebugRenderer
  ====================*/
IDebugRenderer::IDebugRenderer() :
IRenderer(RT_UNKNOWN)
{
	m_iNumActivePointLights = 0;
	m_iNumActiveBones = 0;

	m_bObjectColor = false;
	m_pCurrentEntity = NULL;

	m_mWorld = g_mIdentity;
	m_mWorldRotate = g_mIdentity;

	// Set static sorting variables
	m_bTranslucent = false;
	m_iLayer = INT_MAX;
	m_iEffectLayer = 0;
	m_iVertexType = VERTEX_LINE;
	m_uiVertexBuffer = 0;
	m_uiIndexBuffer = 0;
	m_fDepth = 0.0f;
}


/*====================
  IDebugRenderer::~IDebugRenderer
  ====================*/
IDebugRenderer::~IDebugRenderer()
{
}


/*====================
  IDebugRenderer::Setup
  ====================*/
void	IDebugRenderer::Setup(EMaterialPhase ePhase)
{
	PROFILE("IDebugRenderer::Setup");

	m_bRender = false; // Set to true if we make it to the end of the function

	if (ePhase != PHASE_COLOR)
		return;

	m_pCam = g_pCam;
	m_bLighting = gfx_lighting;
	m_bShadows = g_bCamShadows;
	m_bFog = g_bCamFog;
	m_vAmbient = SceneManager.GetEntityAmbientColor();
	m_vSunColor = SceneManager.GetEntitySunColor();

	m_mWorldViewProj = g_mViewProj;

	g_ShaderRegistry.SetNumPointLights(m_iNumActivePointLights);
	g_ShaderRegistry.SetNumBones(m_iNumActiveBones);
	g_ShaderRegistry.SetLighting(m_bLighting);
	g_ShaderRegistry.SetShadows(m_bShadows);
	g_ShaderRegistry.SetFogofWar(g_bFogofWar);
	g_ShaderRegistry.SetFog(m_bFog);
	g_ShaderRegistry.SetTexcoords(m_iTexcoords);
	g_ShaderRegistry.SetTexkill(m_bTexkill);

	// Set dynamic sorting variables
	m_iVertexShaderInstance = g_ShaderRegistry.GetVertexShaderInstance(g_SimpleMaterial3DColored.GetPhase(ePhase).GetVertexShader());
	m_iPixelShaderInstance = g_ShaderRegistry.GetPixelShaderInstance(g_SimpleMaterial3DColored.GetPhase(ePhase).GetPixelShader());

	m_bRender = true;
}
