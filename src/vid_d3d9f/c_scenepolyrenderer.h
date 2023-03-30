// (C)2006 S2 Games
// c_scenepolyrenderer.h
//
//=============================================================================
#ifndef __C_SCENEPOLYRENDERER_H__
#define __C_SCENEPOLYRENDERER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_renderer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SSceneFaceVert;
//=============================================================================

//=============================================================================
// CScenePolyRenderer
//=============================================================================
class CScenePolyRenderer : public IRenderer
{
private:
	ResHandle			m_hMaterial;
	SSceneFaceVert		*m_pVerts;
	uint				m_uiNumVerts;
	int					m_iFlags;

public:
	static CPool<CScenePolyRenderer>		s_Pool;
	
	void*	operator new(size_t z); // Uses CPool of preallocated instances
	
	~CScenePolyRenderer();
	CScenePolyRenderer(ResHandle hMaterial, SSceneFaceVert *pVerts, uint uiNumVerts, int iFlags);

	void	Setup(EMaterialPhase ePhase);
	void	Render(EMaterialPhase ePhase);
};
//=============================================================================
#endif //__C_SCENEPOLYRENDERER_H__
