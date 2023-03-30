// c_insilverbackclip.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2model_common.h"

#include "c_insilverbackclip.h"

#include "c_outclip.h"

#include <s_blendedlink.h>
#include <c_filehandle.h>
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
const char	CInSilverbackClip::s_ayHeader[4] = {'C', 'L', 'I', 'P'};
//=============================================================================


/*====================
  CInSilverbackClip::CInSilverbackClip
  ====================*/
CInSilverbackClip::CInSilverbackClip()
{
}


/*====================
  CInSilverbackClip::ParseHeader
  ====================*/
bool	CInSilverbackClip::ParseHeader(const SFileBlock &cBlock)
{
	SClipHeaderSilverback &cIn(*(SClipHeaderSilverback *)cBlock.pData);

	if (cBlock.uiLength < sizeof(SClipHeaderSilverback))
	{
		cerr << _T("Bad clip header") << endl;
		return false;
	}

	int iVersion(LittleInt(cIn.iVersion));
	if (iVersion != 1)
	{
		if (iVersion != 1)
			cerr << _T("Invalid clip version: ") << iVersion;

		cerr << _T(", expected version is: ") << 1 << endl;
		return false;
	}

	m_pOutClip->AllocMotions(LittleInt(cIn.iNumMotions));
	m_pOutClip->SetNumFrames(LittleInt(cIn.iNumFrames));

	return true;
}


/*====================
  CInSilverbackClip::ReadAngleKeys
  ====================*/
void	CInSilverbackClip::ReadAngleKeys(float *pData, int iNumKeys, SFloatKeys *pKeys)
{
	pKeys->keys = new float[iNumKeys];

	for (int n = 0; n < iNumKeys; ++n)
		pKeys->keys[n] = LittleFloat(pData[n]);

	pKeys->num_keys = iNumKeys;
}


/*====================
  CInSilverbackClip::ReadScaleKeys
  ====================*/
void	CInSilverbackClip::ReadScaleKeys(float *pData, int iNumKeys, SFloatKeys *pKeys)
{
	pKeys->keys = new float[iNumKeys];

	for (int n = 0; n < iNumKeys; ++n)
		pKeys->keys[n] = LittleFloat(pData[n]);

	pKeys->num_keys = iNumKeys;
}


/*====================
  CInSilverbackClip::ReadPositionKeys
  ====================*/
void	CInSilverbackClip::ReadPositionKeys(float *pData, int iNumKeys, SFloatKeys *pKeys)
{
	pKeys->keys = new float[iNumKeys];

	for (int n = 0; n < iNumKeys; ++n)
		pKeys->keys[n] = LittleFloat(pData[n]);

	pKeys->num_keys = iNumKeys;
}


/*====================
  CInSilverbackClip::ReadByteKeys
  ====================*/
void	CInSilverbackClip::ReadByteKeys(byte *pData, int iNumKeys, SByteKeys *pKeys)
{
	pKeys->keys = new byte[iNumKeys];

	memcpy(pKeys->keys, pData, iNumKeys);

	pKeys->num_keys = iNumKeys;
}


/*====================
  CInSilverbackClip::ParseBoneMotionBlock
  ====================*/
bool	CInSilverbackClip::ParseBoneMotionBlock(const SFileBlock &cBlock)
{
	SBoneMotionBlockSilverback &cIn(*(SBoneMotionBlockSilverback *)cBlock.pData);

	try
	{
		int iBoneIndex(LittleInt(cIn.boneIndex));
		if (iBoneIndex < 0 || iBoneIndex >= int(m_pOutClip->GetNumMotions()))
			throw _TS("Invalid bone motion index");

		int iNumKeys(LittleInt(cIn.num_keys));
		SBoneMotion *pMotion(m_pOutClip->GetBoneMotion(iBoneIndex));

		StrToTString(pMotion->sBoneName, cIn.boneName);

		void *pKeyData(&cIn + 1);

		switch (LittleInt(cIn.key_type))
		{
		case SILVERBACK_MKEY_X:
			ReadPositionKeys((float *)pKeyData, iNumKeys, &pMotion->keys_x);
			break;

		case SILVERBACK_MKEY_Y:
			ReadPositionKeys((float *)pKeyData, iNumKeys, &pMotion->keys_y);
			break;

		case SILVERBACK_MKEY_Z:
			ReadPositionKeys((float *)pKeyData, iNumKeys, &pMotion->keys_z);
			break;

		case SILVERBACK_MKEY_PITCH:
			ReadAngleKeys((float *)pKeyData, iNumKeys, &pMotion->keys_pitch);
			break;

		case SILVERBACK_MKEY_ROLL:
			ReadAngleKeys((float *)pKeyData, iNumKeys, &pMotion->keys_roll);
			break;

		case SILVERBACK_MKEY_YAW:
			ReadAngleKeys((float *)pKeyData, iNumKeys, &pMotion->keys_yaw);
			break;

		case SILVERBACK_MKEY_SCALE:
			ReadScaleKeys((float *)pKeyData, iNumKeys, &pMotion->keys_scalex);
			ReadScaleKeys((float *)pKeyData, iNumKeys, &pMotion->keys_scaley);
			ReadScaleKeys((float *)pKeyData, iNumKeys, &pMotion->keys_scalez);
			break;

		case SILVERBACK_MKEY_VISIBILITY:
			ReadByteKeys((byte *)pKeyData, iNumKeys, &pMotion->keys_visibility);
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
  CInSilverbackClip::ReadBlocks
  ====================*/
bool	CInSilverbackClip::ReadBlocks(FileBlockList &vBlockList)
{
	if (vBlockList.empty() || strcmp(vBlockList.front().szName, "head") != 0)
	{
		cerr << _T("CInSilverbackClip::ReadBlocks: No header") << endl;
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
  CInSilverbackClip::ReadFile
  ====================*/
void	CInSilverbackClip::ReadFile(const tstring &sFilename)
{
	CFileHandle hFile(sFilename, FILE_READ | FILE_BINARY);

	vector<SFileBlock> vBlockList;
	if (!hFile.BuildBlockList(s_ayHeader, sizeof(s_ayHeader), vBlockList))
		return;

	if (!ReadBlocks(vBlockList))
		return;
}