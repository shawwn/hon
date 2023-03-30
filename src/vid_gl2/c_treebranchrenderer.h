// (C)2006 S2 Games
// c_treebranchrenderer.h
//
//=============================================================================
#ifndef __C_TREEBRANCHRENDERER_H__
#define __C_TREEBRANCHRENDERER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_renderer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CTreeModelDef;
class CSceneEntity;
//=============================================================================

//=============================================================================
// CTreeBranchRenderer
//=============================================================================
class CTreeBranchRenderer : public IRenderer
{
private:
	const CTreeModelDef	*m_pTreeDef;
	const CSceneEntity	&m_cEntity;
	
	// Computed in Setup
	SLODData			m_LOD;

public:
	static CPool<CTreeBranchRenderer>		s_Pool;
	
	void*	operator new(size_t z, const char *szContext = NULL, const char *szType = NULL, const char *szFile = NULL, short nLine = 0); // Uses CPool of preallocated instances
	void	operator delete(void *p) { }
	void	operator delete(void *p, const char *szContext, const char *szType, const char *szFile, short nLine) { }
	
	~CTreeBranchRenderer();
	CTreeBranchRenderer(const CSceneEntity &cEntity, const CTreeModelDef *pTreeDef,
		const D3DXMATRIXA16 &mWorldViewProj,
		const D3DXMATRIXA16 &mWorld,
		const D3DXMATRIXA16 &mWorldRotate);

	void	Setup(EMaterialPhase ePhase);
	void	Render(EMaterialPhase ePhase);
};
//=============================================================================
#endif //__C_TREEBRANCHRENDERER_H__
