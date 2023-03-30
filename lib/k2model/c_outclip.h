// c_outclip.h
//
//=============================================================================
#ifndef __C_OUTCLIP_H__
#define __C_OUTCLIP_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Declarations / Definitions
//=============================================================================
class CXMLDoc;
class COutBone;

struct SFloatKeys
{
	float			*keys;				// Will allocate num_keys keys
	int				num_keys;
};

struct SQuatKeys
{
	CVec4f			*keys;
	int				num_keys;
};


struct SByteKeys
{
	byte			*keys;
	int				num_keys;
};

struct SBoneMotion
{
	tstring			sBoneName;

	// Rotation keys (converted to a quaternion for interpolation)
	SFloatKeys		keys_pitch;
	SFloatKeys		keys_roll;
	SFloatKeys		keys_yaw;

	// Position keys
	SFloatKeys		keys_x;
	SFloatKeys		keys_y;
	SFloatKeys		keys_z;

	// Scale keys
	SFloatKeys		keys_scalex;
	SFloatKeys		keys_scaley;
	SFloatKeys		keys_scalez;

	// Visibility keys
	SByteKeys		keys_visibility;

	// Precomputed Rotation keys
	SQuatKeys		keys_quat;
};

enum keyTypes_enum
{
	MKEY_X,
	MKEY_Y,
	MKEY_Z,
	MKEY_PITCH,
	MKEY_ROLL,
	MKEY_YAW,
	MKEY_VISIBILITY,
	MKEY_SCALEX,
	MKEY_SCALEY,
	MKEY_SCALEZ,
	NUM_MKEY_TYPES
};

#pragma pack(1)		//make sure there's no member padding.  these structs must match up with the file read in

struct SClipHeader
{
	int iVersion;
	int iNumMotions;
	int iNumFrames;
};

typedef struct keyBlock_s
{
	int			boneIndex;
	int			key_type;		//see MKEY_* defines above
	int			num_keys;

	// At the end, so we can save space
	byte		cNameLen;
	char		szBoneName[256];
}
keyBlock_t;

#pragma pack()

const int CLIP_VERSION(2);
//=============================================================================

//=============================================================================
// COutClip
//=============================================================================
class COutClip
{
private:
	bool	m_bIsValid;

	vector<SBoneMotion>		m_vMotions;
	int		m_iNumFrames;

	int		seekback;		// position in the file to fill in the block length

	void	BeginBlock(const char *blockname, FILE *stream);
	void	WriteBlockData(void *data, int size, FILE *stream);
	void	EndBlock(FILE *stream);
	void	WriteFloatKeys(int boneindex, SFloatKeys *keys, int keytype, const tstring &sBoneName, FILE *stream);
	void	WriteByteKeys(int boneindex, SByteKeys *keys, int keytype, const tstring &sBoneName, FILE *stream);

public:
	~COutClip()	{}
	COutClip();

	void			Validate()				{ m_bIsValid = true; }
	void			Invalidate()			{ m_bIsValid = false; }
	bool			IsValid() const			{ return m_bIsValid; }

	uint			AddBone(const COutBone &newBone);
	void			AllocMotions(uint uiNumMotions);
	void			SetNumFrames(int iNumFrames)	{ m_iNumFrames = iNumFrames; }

	uint			GetNumMotions()			{ return uint(m_vMotions.size()); }
	SBoneMotion*	GetBoneMotion(uint ui)	{ return &m_vMotions[ui]; }

	tstring			WriteFile(const tstring &sFilename);
};
//=============================================================================
#endif //__C_OUTCLIP_H__
