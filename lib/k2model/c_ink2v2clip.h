// c_ink2v2clip.h
//
//=============================================================================
#ifndef __C_INK2V2CLIP_H__
#define __C_INK2V2CLIP_H__

//=============================================================================
// Headers
//=============================================================================
#include <c_filehandle.h>

#include "c_outclip.h"

#include "c_convexpolyhedron.h"
//=============================================================================

//=============================================================================
// Declarations / Definitions
//=============================================================================
class CXMLDoc;

#pragma pack(1)
struct SClipHeaderK2v2
{
	int		iVersion;
	int		iNumMotions;
	int		iNumFrames;
};

struct SBoneMotionBlockK2v2
{
	int		boneIndex;
	int		key_type;		//see MKEY_* defines above
	int		num_keys;

	// At the end, so we can save space
	byte	cNameLen;
	char	szBoneName[256];
};
#pragma pack()
//=============================================================================

//=============================================================================
// CInK2v2Clip
//=============================================================================
class CInK2v2Clip
{
private:
	COutClip				*m_pOutClip;

	static const char		s_ayHeader[4];

	void	ReadAngleKeys(float *pData, int iNumKeys, SFloatKeys *pKeys);
	void	ReadScaleKeys(float *pData, int iNumKeys, SFloatKeys *pKeys);
	void	ReadPositionKeys(float *pData, int iNumKeys, SFloatKeys *pKeys);
	void	ReadByteKeys(byte *pData, int iNumKeys, SByteKeys *pKeys);

	bool	ParseHeader(const SFileBlock &cBlock);
	bool	ParseBoneMotionBlock(const SFileBlock &cBlock);

	bool	ReadBlocks(FileBlockList &vBlockList);

public:
	~CInK2v2Clip()	{}
	CInK2v2Clip();

	void	SetOutClip(COutClip *pOutClip)		{ m_pOutClip = pOutClip; }

	void	ReadFile(const tstring &sFilename);
};
//=============================================================================

#endif //__C_INK2V2CLIP_H__
