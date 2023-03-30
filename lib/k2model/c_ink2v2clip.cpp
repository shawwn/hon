// c_ink2v2clip.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2model_common.h"

#include "c_ink2v2clip.h"

#include "c_outclip.h"

#include <s_blendedlink.h>
#include <c_filehandle.h>
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
const char	CInK2v2Clip::s_ayHeader[4] = {'C', 'L', 'I', 'P'};
//=============================================================================


/*====================
  CInK2v2Clip::CInK2v2Clip
  ====================*/
CInK2v2Clip::CInK2v2Clip()
{
}


/*====================
  CInK2v2Clip::ParseHeader
  ====================*/
bool	CInK2v2Clip::ParseHeader(const SFileBlock &cBlock)
{
	SClipHeaderK2v2 &cIn(*(SClipHeaderK2v2 *)cBlock.pData);

	if (cBlock.uiLength < sizeof(SClipHeaderK2v2))
	{
		cerr << _T("Bad clip header") << endl;
		return false;
	}

	int iVersion(LittleInt(cIn.iVersion));
	if (iVersion != 2)
	{
		if (iVersion != 2)
			cerr << _T("Invalid clip version: ") << iVersion;

		cerr << _T(", expected version is: ") << 2 << endl;
		return false;
	}

	m_pOutClip->AllocMotions(LittleInt(cIn.iNumMotions));
	m_pOutClip->SetNumFrames(LittleInt(cIn.iNumFrames));

	return true;
}


/*====================
  CInK2v2Clip::ReadAngleKeys
  ====================*/
void	CInK2v2Clip::ReadAngleKeys(float *pData, int iNumKeys, SFloatKeys *keys)
{
	keys->keys = new float[iNumKeys];

	for (int n = 0; n < iNumKeys; ++n)
		keys->keys[n] = LittleFloat(pData[n]);

	keys->num_keys = iNumKeys;
}


/*====================
  CInK2v2Clip::ReadScaleKeys
  ====================*/
void	CInK2v2Clip::ReadScaleKeys(float *pData, int iNumKeys, SFloatKeys *keys)
{
	keys->keys = new float[iNumKeys];

	for (int n = 0; n < iNumKeys; ++n)
		keys->keys[n] = LittleFloat(pData[n]);

	keys->num_keys = iNumKeys;
}


/*====================
  CInK2v2Clip::ReadPositionKeys
  ====================*/
void	CInK2v2Clip::ReadPositionKeys(float *pData, int iNumKeys, SFloatKeys *keys)
{
	keys->keys = new float[iNumKeys];

	for (int n = 0; n < iNumKeys; ++n)
		keys->keys[n] = LittleFloat(pData[n]);

	keys->num_keys = iNumKeys;
}


/*====================
  CInK2v2Clip::ReadByteKeys
  ====================*/
void	CInK2v2Clip::ReadByteKeys(byte *pData, int iNumKeys, SByteKeys *keys)
{
	keys->keys = new byte[iNumKeys];

	memcpy(keys->keys, pData, iNumKeys);

	keys->num_keys = iNumKeys;
}


/*====================
  CInK2v2Clip::ParseBoneMotionBlock
  ====================*/
bool	CInK2v2Clip::ParseBoneMotionBlock(const SFileBlock &cBlock)
{
	SBoneMotionBlockK2v2 &cIn(*(SBoneMotionBlockK2v2 *)cBlock.pData);

	try
	{
		int iBoneIndex(LittleInt(cIn.boneIndex));
		if (iBoneIndex < 0 || iBoneIndex >= int(m_pOutClip->GetNumMotions()))
			throw _TS("Invalid bone motion index");

		int iNumKeys(LittleInt(cIn.num_keys));
		SBoneMotion *pMotion(m_pOutClip->GetBoneMotion(iBoneIndex));

		StrToTString(pMotion->sBoneName, cIn.szBoneName);

		void *pKeyData((byte *)&cIn + sizeof(SBoneMotionBlockK2v2) - 256 + cIn.cNameLen + 1);

		switch (LittleInt(cIn.key_type))
		{
		case MKEY_X:
			ReadPositionKeys((float *)pKeyData, iNumKeys, &pMotion->keys_x);
			break;

		case MKEY_Y:
			ReadPositionKeys((float *)pKeyData, iNumKeys, &pMotion->keys_y);
			break;

		case MKEY_Z:
			ReadPositionKeys((float *)pKeyData, iNumKeys, &pMotion->keys_z);
			break;

		case MKEY_PITCH:
			ReadAngleKeys((float *)pKeyData, iNumKeys, &pMotion->keys_pitch);
			break;

		case MKEY_ROLL:
			ReadAngleKeys((float *)pKeyData, iNumKeys, &pMotion->keys_roll);
			break;

		case MKEY_YAW:
			ReadAngleKeys((float *)pKeyData, iNumKeys, &pMotion->keys_yaw);
			break;

		case MKEY_SCALEX:
			ReadScaleKeys((float*)pKeyData, iNumKeys, &pMotion->keys_scalex);
			break;

		case MKEY_SCALEY:
			ReadScaleKeys((float*)pKeyData, iNumKeys, &pMotion->keys_scaley);
			break;

		case MKEY_SCALEZ:
			ReadScaleKeys((float*)pKeyData, iNumKeys, &pMotion->keys_scalez);
			break;

		case MKEY_VISIBILITY:
			ReadByteKeys((byte*)pKeyData, iNumKeys, &pMotion->keys_visibility);
			break;

		default:
			throw _TS("Unknown motion key type: ") + XtoA(cIn.key_type);
		}
	}
	catch (const tstring &sReason)
	{
		cerr << sReason << endl;
		return false;
	}

	return true;
}


/*====================
  CInK2v2Clip::ReadBlocks
  ====================*/
bool	CInK2v2Clip::ReadBlocks(FileBlockList &vBlockList)
{
	if (vBlockList.empty() || strcmp(vBlockList.front().szName, "head") != 0)
	{
		cerr << _T("CInK2v2Clip::ReadBlocks: No header") << endl;
		return false;
	}

	if (!ParseHeader(vBlockList[0]))
		return false;

	for (uint ui(1); ui < vBlockList.size(); ++ui)
	{
		SFileBlock &cBlock(vBlockList[ui]);

		if (strcmp(cBlock.szName, "bmtn") == 0)
		{
			if (!ParseBoneMotionBlock(cBlock))
				return false;
		}
	}

	return true;
}


/*====================
  CInK2v2Clip::ReadFile
  ====================*/
void	CInK2v2Clip::ReadFile(const tstring &sFilename)
{
	CFileHandle hFile(sFilename, FILE_READ | FILE_BINARY);

	vector<SFileBlock> vBlockList;
	if (!hFile.BuildBlockList(s_ayHeader, sizeof(s_ayHeader), vBlockList))
		return;

	if (!ReadBlocks(vBlockList))
		return;
}