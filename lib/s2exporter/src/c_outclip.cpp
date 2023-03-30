// (C)2008 S2 Games
// c_outclip.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "exporter_common.h"

#include "Max.h"
#include "stdmat.h"
#include "shaders.h"

#include "c_outclip.h"

#include "c_outbone.h"
//=============================================================================

/*====================
  COutClip::COutClip
  ====================*/
COutClip::COutClip() :
m_bIsValid(false),
m_sStatus("Empty")
{
}

COutClip::COutClip(INode *pNode, TimeValue time) :
m_bIsValid(false),
m_sStatus("Initializing...")
{
	try
	{
		Validate();
		m_sStatus = "OK";
	}
	catch (const char *szMsg)
	{
		m_sStatus = szMsg;
		Invalidate();
	}
}


/*====================
  COutClip::AddBone
  ====================*/
uint	COutClip::AddBone(const COutBone &newBone)
{
	if (m_vMotions.size() < newBone.GetIndex() + 1)
		m_vMotions.resize(newBone.GetIndex() + 1);

	SBoneMotion &newMotion(m_vMotions[newBone.GetIndex()]);

	newMotion.sBoneName = newBone.GetName();

	newMotion.keys_pitch.keys = NULL;
	newMotion.keys_pitch.num_keys = 0;
	newMotion.keys_roll.keys = NULL;
	newMotion.keys_roll.num_keys = 0;
	newMotion.keys_yaw.keys = NULL;
	newMotion.keys_yaw.num_keys = 0;

	newMotion.keys_x.keys = NULL;
	newMotion.keys_x.num_keys = 0;
	newMotion.keys_y.keys = NULL;
	newMotion.keys_y.num_keys = 0;
	newMotion.keys_z.keys = NULL;
	newMotion.keys_z.num_keys = 0;

	newMotion.keys_scalex.keys = NULL;
	newMotion.keys_scalex.num_keys = 0;
	newMotion.keys_scaley.keys = NULL;
	newMotion.keys_scaley.num_keys = 0;
	newMotion.keys_scalez.keys = NULL;
	newMotion.keys_scalez.num_keys = 0;

	newMotion.keys_visibility.keys = NULL;
	newMotion.keys_visibility.num_keys = 0;

	return uint(m_vMotions.size());
}


/*====================
  COutClip::BeginBlock
  ====================*/
void	COutClip::BeginBlock(const char *blockName, FILE *stream)
{
	int templength = 0;

	fwrite(blockName, 4, 1, stream);
	//we'll fill in the length here on the EndBlock call
	seekback = ftell(stream);
	fwrite(&templength, 4, 1, stream);
}


/*====================
  COutClip::WriteBlockData
  ====================*/
void	COutClip::WriteBlockData(void *data, int size, FILE *stream)
{
	fwrite(data, size, 1, stream);
}


/*====================
  COutClip::EndBlock
  ====================*/
void	COutClip::EndBlock(FILE *stream)
{
	fpos_t curpos = ftell(stream);
	int length = curpos - seekback - 4;
	fseek(stream, seekback, SEEK_SET);
	fwrite(&length, 4, 1, stream);
	fseek(stream, curpos, SEEK_SET);
}


/*====================
  COutClip::WriteFloatKeys
  ====================*/
void	COutClip::WriteFloatKeys(int boneindex, SFloatKeys *keys, int keytype, const tstring &sBoneName, FILE *stream)
{
	BeginBlock("bmtn", stream);

	keyBlock_t keyblock;
	keyblock.boneIndex = boneindex;
	keyblock.key_type = keytype;
	keyblock.num_keys = keys->num_keys;
	keyblock.cNameLen = MIN(sBoneName.length(), size_t(255));
	strncpy(keyblock.szBoneName, sBoneName.c_str(), keyblock.cNameLen);
	keyblock.szBoneName[keyblock.cNameLen] = 0;

	WriteBlockData(&keyblock, sizeof(keyblock) - 256 + keyblock.cNameLen + 1, stream);

	// Write keys
	for (int n = 0; n < keys->num_keys; ++n)
		WriteBlockData(&keys->keys[n], sizeof(float), stream);

	EndBlock(stream);
}


/*====================
  COutClip::WriteByteKeys
  ====================*/
void	COutClip::WriteByteKeys(int boneindex, SByteKeys *keys, int keytype, const tstring &sBoneName, FILE *stream)
{
	BeginBlock("bmtn", stream);

	keyBlock_t keyblock;
	keyblock.boneIndex = boneindex;
	keyblock.key_type = keytype;
	keyblock.num_keys = keys->num_keys;
	keyblock.cNameLen = MIN(sBoneName.length(), size_t(255));
	strncpy(keyblock.szBoneName, sBoneName.c_str(), keyblock.cNameLen);
	keyblock.szBoneName[keyblock.cNameLen] = 0;

	WriteBlockData(&keyblock, sizeof(keyblock) - 256 + keyblock.cNameLen + 1, stream);

	// Write keys
	for (int n = 0; n < keys->num_keys; ++n)
		WriteBlockData(&keys->keys[n], 1, stream);

	EndBlock(stream);
}


/*====================
  COutClip::WriteFile
  ====================*/
tstring	COutClip::WriteFile(const tstring &sFilename)
{
	FILE *stream = fopen(sFilename.c_str(), "wb");
	int iNumframes = g_S2ModelExporter.GetNumFrames();

	if (!stream)
	{
		g_S2ModelExporter.MessageBox("Couldn't create .clip file", "File Error");
		return _T("");
	}

	fwrite("CLIP", 4, 1, stream);

	BeginBlock("head", stream);

	SClipHeader clipheader;

	clipheader.iVersion = CLIP_VERSION;
	clipheader.iNumMotions = m_vMotions.size();
	clipheader.iNumFrames = iNumframes;

	WriteBlockData(&clipheader, sizeof(clipheader), stream);

	EndBlock(stream);

	for (uint n(0); n < m_vMotions.size(); ++n)
	{
		// write keys
		WriteFloatKeys(n, &m_vMotions[n].keys_x, MKEY_X, m_vMotions[n].sBoneName, stream);
		WriteFloatKeys(n, &m_vMotions[n].keys_y, MKEY_Y, m_vMotions[n].sBoneName, stream);
		WriteFloatKeys(n, &m_vMotions[n].keys_z, MKEY_Z, m_vMotions[n].sBoneName, stream);
		WriteFloatKeys(n, &m_vMotions[n].keys_pitch, MKEY_PITCH, m_vMotions[n].sBoneName, stream);
		WriteFloatKeys(n, &m_vMotions[n].keys_roll, MKEY_ROLL, m_vMotions[n].sBoneName, stream);
		WriteFloatKeys(n, &m_vMotions[n].keys_yaw, MKEY_YAW, m_vMotions[n].sBoneName, stream);
		WriteFloatKeys(n, &m_vMotions[n].keys_scalex, MKEY_SCALEX, m_vMotions[n].sBoneName, stream);
		WriteFloatKeys(n, &m_vMotions[n].keys_scaley, MKEY_SCALEY, m_vMotions[n].sBoneName, stream);
		WriteFloatKeys(n, &m_vMotions[n].keys_scalez, MKEY_SCALEZ, m_vMotions[n].sBoneName, stream);
		WriteByteKeys(n, &m_vMotions[n].keys_visibility, MKEY_VISIBILITY, m_vMotions[n].sBoneName, stream);
	}

	fclose(stream);

	return _T("Successfully wrote clip file to ") + sFilename;
}
