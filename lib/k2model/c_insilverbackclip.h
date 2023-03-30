// c_insilverbackclip.h
//
//=============================================================================
#ifndef __C_INSILVERBACKCLIP_H__
#define __C_INSILVERBACKCLIP_H__

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

const int	CLIP_BONE_NAME_LENGTH(32);

enum ESilverbackKeyType
{
	SILVERBACK_MKEY_X,
	SILVERBACK_MKEY_Y,
	SILVERBACK_MKEY_Z,
	SILVERBACK_MKEY_PITCH,
	SILVERBACK_MKEY_ROLL,
	SILVERBACK_MKEY_YAW,
	SILVERBACK_MKEY_VISIBILITY,
	SILVERBACK_MKEY_SCALE,
	NUM_SILVERBACK_MKEY_TYPES
};

#pragma pack(1)
struct SClipHeaderSilverback
{
	int		iVersion;
	int		iNumMotions;
	int		iNumFrames;
};

struct SBoneMotionBlockSilverback
{
	char	boneName[CLIP_BONE_NAME_LENGTH];
	int		boneIndex;
	int		key_type;		//see MKEY_* defines above
	int		num_keys;
};
#pragma pack()
//=============================================================================

//=============================================================================
// CInSilverbackClip
//=============================================================================
class CInSilverbackClip
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
	~CInSilverbackClip()	{}
	CInSilverbackClip();

	void	SetOutClip(COutClip *pOutClip)		{ m_pOutClip = pOutClip; }

	void	ReadFile(const tstring &sFilename);
};
//=============================================================================

#endif //__C_INSILVERBACKCLIP_H__
