// (C)2005 S2 Games
// meshtools.h
//
// useful mesh functions for max plugins
//=============================================================================
#ifndef MESHTOOLS_H
#define MESHTOOLS_H

//=============================================================================
// Headers
//=============================================================================
#include "Max.h"
#include "meshdlib.h"
//=============================================================================

#define FRAME(x) (GetTicksPerFrame() * x)

TriObject*	GetTriObject(Interface* ip, TimeValue time, INode* node, int &deleteIt);
bool		TestMeshConvexity(Mesh& mesh);

#endif //MESHTOOLS_H
