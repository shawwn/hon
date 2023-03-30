// k2validator.cpp
//
// K0 Template
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2validator_common.h"

#include <k0_string.h>
#include <k0_math.h>
#include <k0_system.h>
#include <c_cmdline.h>
#include <c_filehandle.h>
#include <c_xmldoc.h>
#include <c_xmlnode.h>
#include <c_mmapunzip.h>
#include <c_regexp.h>
#include <fileinterface.h>

#include <iostream>

//#include "localfile.h"
//=============================================================================

//=============================================================================
// Constants
//=============================================================================
const int NUM_FOLIAGE_LAYERS(2);
const int NUM_TERRAIN_LAYERS(2);
const int MAX_ENTITY_TARGET_PROPERTIES(8);
const int K2_MAX_NAME(64);
const int K2_MAX_PATH(260);
const int K2_MAX_BOOL(5);
const int K2_MAX_UINT(16);
const int K2_MAX_FLOAT(16);
const int K2_MAX_VEC3(K2_MAX_FLOAT + 1 + K2_MAX_FLOAT + 1 + K2_MAX_FLOAT);
const int K2_MAX_COLOR(K2_MAX_FLOAT + 1 + K2_MAX_FLOAT + 1 + K2_MAX_FLOAT + 1 + K2_MAX_FLOAT);
const int MAX_WORLD_SIZE(9);
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
#ifdef _UNICODE
#define tcout wcout
#define tcerr wcerr
#else
#define tcout cout
#define tcerr cerr
#endif

using std::tcout;
using std::tcerr;
using std::endl;
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EMapValidationResult
{
	K2_MAP_VALID = 0,
	K2_MAP_INVALID_UNKNOWN,
	K2_MAP_INVALID_UNZIP_FILE,
	K2_MAP_INVALID_TOO_MANY_FILES,
	K2_MAP_INVALID_NOT_ENOUGH_FILES,
	K2_MAP_INVALID_INCORRECT_FILENAMES,

	// XML config file
	K2_MAP_INVALID_WORLDCONFIG_DATA,

	// XML list files
	K2_MAP_INVALID_ENTITYLIST_DATA,
	K2_MAP_INVALID_LIGHTLIST_DATA,
	K2_MAP_INVALID_MATERIALLIST_DATA,
	K2_MAP_INVALID_SOUNDLIST_DATA,
	K2_MAP_INVALID_TEXTURELIST_DATA,
	K2_MAP_INVALID_TRIGGERLIST_DATA,
	K2_MAP_INVALID_WORLDOCCLUDERLIST_DATA,

	// Binary files
	K2_MAP_INVALID_HEIGHTMAP_DATA,
	K2_MAP_INVALID_VERTEXCOLORMAP_DATA,
	K2_MAP_INVALID_VERTEXFOLIAGEMAP_DATA,
	K2_MAP_INVALID_VERTEXBLOCKERMAP_DATA,
	K2_MAP_INVALID_VERTEXCLIFFMAP_DATA,
	K2_MAP_INVALID_VARIATIONCLIFFMAP_DATA,
	K2_MAP_INVALID_CLIFFSETLIST_DATA,
	K2_MAP_INVALID_TILERAMPMAP_DATA,
	K2_MAP_INVALID_TILEMATERIALMAP_DATA,
	K2_MAP_INVALID_TILEFOLIAGEMAP_DATA,
	K2_MAP_INVALID_TILECLIFFMAP_DATA,
	K2_MAP_INVALID_TILEVISBLOCKERMAP_DATA,
};

enum ESpaceType
{
	TILE_SPACE,
	GRID_SPACE,
	TEXEL_SPACE,
	NAV_TILE_SPACE,
	VIS_TILE_SPACE,

	NUM_SPACE_TYPES
};
//=============================================================================

//=============================================================================
// SMapValidationResultInfo
//=============================================================================
struct SMapValidationResultInfo
{
	tstring					sResultMessage;
	tstring					sProblemFile;
	EMapValidationResult	eCode;

	SMapValidationResultInfo() : eCode(K2_MAP_INVALID_UNKNOWN)
	{}
};
//=============================================================================


/*====================
  ValidateXMLAttr_String
  ====================*/
static bool	ValidateXMLAttr_String(PropertyMap& mapProperties, const tstring& sProperty, tstring& sVal)
{
	PropertyMap::iterator itFind(mapProperties.find(sProperty));

	// Attribute does not exist?
	if (itFind == mapProperties.end())
		return false;

	// String too long?
	const tstring& sAttrVal(itFind->second);
	if (sAttrVal.size() > K2_MAX_NAME)
		return false;

	sVal = sAttrVal;
	mapProperties.erase(itFind);
	return true;
}


/*====================
  ValidateXMLAttr_Name
  ====================*/
static bool	ValidateXMLAttr_Name(PropertyMap& mapProperties, const tstring& sProperty, tstring& sVal)
{
	PropertyMap::iterator itFind(mapProperties.find(sProperty));

	// Attribute does not exist?
	if (itFind == mapProperties.end())
		return false;

	// Name too long?
	const tstring& sAttrVal(itFind->second);
	if (sAttrVal.size() > K2_MAX_NAME)
		return false;

	// TODO: validate the name
	sVal = sAttrVal;
	mapProperties.erase(itFind);
	return true;
}


/*====================
  ValidateXMLAttr_Filepath
  ====================*/
static bool	ValidateXMLAttr_Filepath(PropertyMap& mapProperties, const tstring& sProperty, tstring& sVal)
{
	PropertyMap::iterator itFind(mapProperties.find(sProperty));

	// Attribute does not exist?
	if (itFind == mapProperties.end())
		return false;

	// Filename too long?
	const tstring& sAttrVal(itFind->second);
	if (sAttrVal.size() > K2_MAX_PATH)
		return false;

	// TODO: validate the filepath
	sVal = sAttrVal;
	mapProperties.erase(itFind);
	return true;
}


/*====================
  ValidateXMLAttr_Bool
  ====================*/
static bool	ValidateXMLAttr_Bool(PropertyMap& mapProperties, const tstring& sProperty, bool& bVal)
{
	PropertyMap::iterator itFind(mapProperties.find(sProperty));

	// Attribute does not exist?
	if (itFind == mapProperties.end())
		return false;

	// Value too long?
	const tstring& sAttrVal(itFind->second);
	if (sAttrVal.size() > K2_MAX_BOOL)
		return false;

	// Validate the bool
	if (sAttrVal == _T("true") || sAttrVal == _T("1"))
	{
		mapProperties.erase(itFind);
		bVal = true;
		return true;
	}
	if (sAttrVal == _T("false") || sAttrVal == _T("0"))
	{
		mapProperties.erase(itFind);
		bVal = false;
		return true;
	}

	return false;
}


/*====================
  ValidateXMLAttr_Uint
  ====================*/
static bool	ValidateXMLAttr_Uint(PropertyMap& mapProperties, const tstring& sProperty, uint& uiVal)
{
	PropertyMap::iterator itFind(mapProperties.find(sProperty));

	// Attribute does not exist?
	if (itFind == mapProperties.end())
		return false;

	// Value too long?
	const tstring& sAttrVal(itFind->second);
	if (sAttrVal.size() > K2_MAX_UINT)
		return false;

	// TODO: Validate the uint
	uiVal = AtoI(sAttrVal);
	mapProperties.erase(itFind);
	return true;
}


/*====================
  ValidateXMLAttr_Float
  ====================*/
static bool	ValidateXMLAttr_Float(PropertyMap& mapProperties, const tstring& sProperty, float& fVal)
{
	PropertyMap::iterator itFind(mapProperties.find(sProperty));

	// Attribute does not exist?
	if (itFind == mapProperties.end())
		return false;

	// Value too long?
	const tstring& sAttrVal(itFind->second);
	if (sAttrVal.size() > K2_MAX_FLOAT)
		return false;

	// TODO: Validate the float
	fVal = AtoF(sAttrVal);
	mapProperties.erase(itFind);
	return true;
}


/*====================
  ValidateXMLAttr_Vec3
  ====================*/
static bool	ValidateXMLAttr_Vec3(PropertyMap& mapProperties, const tstring& sProperty, CVec3f& v3Val)
{
	PropertyMap::iterator itFind(mapProperties.find(sProperty));

	// Attribute does not exist?
	if (itFind == mapProperties.end())
		return false;

	// Value too long?
	const tstring& sAttrVal(itFind->second);
	if (sAttrVal.size() > K2_MAX_VEC3)
		return false;


	// TODO: validate the vec3.
	float x, y, z;
	size_t uiPos(0);

	x = AtoF(sAttrVal);
	uiPos = sAttrVal.find(_T(' '));
	if (uiPos == tstring::npos)
		return false;

	y = AtoF(sAttrVal.substr(uiPos));
	uiPos = sAttrVal.find(_T(' '), uiPos + 1);
	if (uiPos == tstring::npos)
		return false;

	z = AtoF(sAttrVal.substr(uiPos));

	v3Val.x = x;
	v3Val.y = y;
	v3Val.z = z;

	mapProperties.erase(itFind);
	return true;
}


/*====================
  ValidateXMLAttr_Color
  ====================*/
static bool	ValidateXMLAttr_Color(PropertyMap& mapProperties, const tstring& sProperty, CVec4f& colVal)
{
	PropertyMap::iterator itFind(mapProperties.find(sProperty));

	// Attribute does not exist?
	if (itFind == mapProperties.end())
		return false;

	// Value too long?
	const tstring& sAttrVal(itFind->second);
	if (sAttrVal.size() > K2_MAX_COLOR)
		return false;

	// TODO: validate the color.
	float r, g, b, a = 1.0f;
	size_t uiPos(0);

	r = AtoF(sAttrVal);
	uiPos = sAttrVal.find(_T(' '));
	if (uiPos == tstring::npos)
		return false;

	g = AtoF(sAttrVal.substr(uiPos));
	uiPos = sAttrVal.find(_T(' '), uiPos + 1);
	if (uiPos == tstring::npos)
		return false;

	b = AtoF(sAttrVal.substr(uiPos));
	uiPos = sAttrVal.find(_T(' '), uiPos + 1);
	if (uiPos != tstring::npos)
	{
		a = AtoF(sAttrVal.substr(uiPos));
	}

	colVal.x = r;
	colVal.y = g;
	colVal.z = b;
	colVal.w = a;

	mapProperties.erase(itFind);
	return true;
}


/*====================
  ValidateXML_Node
  ====================*/
static bool	ValidateXML_Node(CXMLNodeWrite& cNode, PropertyMap& cPropertyMap, tstring& sFailReason)
{
	if (cNode.GetName().size() > K2_MAX_PATH)
	{
		sFailReason = _TS("Node name is too long");
		return false;
	}

	if (!cNode.GetContents().empty())
	{
		sFailReason = _TS("Node contents is not empty");
		return false;
	}

	PropertyList cProperties(cNode.GetPropertyList());

	sFailReason.clear();
	cPropertyMap.clear();
	while (!cProperties.empty())
	{
		const pair<tstring, tstring>& cEntry(*cProperties.begin());

		// Attribute name too long?
		if (cEntry.first.size() > K2_MAX_PATH)
		{
			sFailReason = _TS("Attribute '") + cEntry.first.substr(0, K2_MAX_PATH) + _TS("...' is too long");
			return false;
		}

		// Attribute value too long?
		if (cEntry.second.size() > K2_MAX_PATH)
		{
			sFailReason = _TS("Attribute '") + cEntry.first + _TS("' is set to a value which is too long");
			return false;
		}

		// Duplicate property?
		if (cPropertyMap.find(cEntry.first) != cPropertyMap.end())
		{
			sFailReason = _TS("Duplicate property '") + cEntry.first + _TS("'");
			return false;
		}

		cPropertyMap[cEntry.first] = cEntry.second;
		cProperties.pop_front();
	}

	return true;
}


/*====================
  K2_ValidateMap
  ====================*/
bool	K2_ValidateMap(const tstring& sMap, SMapValidationResultInfo* pOutInfo = NULL)
{
	tcout << _T("Validating map '") << sMap << _T("' ...") << newl;


	// Helper macro for console printing
	//#define ConsoleFunc	tcout << _T("K2_ValidateMap(") << sMap << _T(") - ")
	#define ConsoleFunc		tcout

	// Helper macro for error case
	#define K2_validation_result(myFile, myCode, myMessage)				\
		if (pOutInfo != NULL)											\
		{																\
			pOutInfo->sResultMessage.assign(myMessage);					\
			pOutInfo->sProblemFile.assign(myFile);						\
			pOutInfo->eCode = myCode;									\
		}																\
		ConsoleFunc << myMessage << endl;

	// Required map files
	static tsvector vRequiredFiles;
	if (vRequiredFiles.empty())
	{
		vRequiredFiles.resize(20);
		vRequiredFiles[ 0].assign(_T("cliffsetlist"));
		vRequiredFiles[ 1].assign(_T("entitylist"));
		vRequiredFiles[ 2].assign(_T("heightmap"));
		vRequiredFiles[ 3].assign(_T("lightlist"));
		vRequiredFiles[ 4].assign(_T("materiallist"));
		vRequiredFiles[ 5].assign(_T("soundlist"));
		vRequiredFiles[ 6].assign(_T("texturelist"));
		vRequiredFiles[ 7].assign(_T("tilecliffmap"));
		vRequiredFiles[ 8].assign(_T("tilefoliagemap"));
		vRequiredFiles[ 9].assign(_T("tilematerialmap"));
		vRequiredFiles[10].assign(_T("tilerampmap"));
		vRequiredFiles[11].assign(_T("tilevisblockermap"));
		vRequiredFiles[12].assign(_T("triggerlist"));
		vRequiredFiles[13].assign(_T("variationcliffmap"));
		vRequiredFiles[14].assign(_T("vertexblockermap"));
		vRequiredFiles[15].assign(_T("vertexcliffmap"));
		vRequiredFiles[16].assign(_T("vertexcolormap"));
		vRequiredFiles[17].assign(_T("vertexfoliagemap"));
		vRequiredFiles[18].assign(_T("worldconfig"));
		vRequiredFiles[19].assign(_T("worldoccluderlist"));
	}

	// Unzip the map
	CMMapUnzip cMapArchive;
	if (!cMapArchive.Open(sMap))
	{
		K2_validation_result(sMap, K2_MAP_INVALID_UNZIP_FILE,
			_T("Failed to open unzip file.  The file either doesn't exist or is corrupt."));
		return false;
	}

	// Verify that a map contains the required files, and ONLY those files
	tsvector vFiles(cMapArchive.GetFileList());

	// Early out: different number of files?
	if (vFiles.size() != vRequiredFiles.size())
	{
		EMapValidationResult eResult(K2_MAP_INVALID_NOT_ENOUGH_FILES);
		if (vFiles.size() > vRequiredFiles.size())
			eResult = K2_MAP_INVALID_TOO_MANY_FILES;

		K2_validation_result(sMap, eResult,
			_TS("Mapfile contains ") + XtoA(INT_SIZE(vFiles.size()))
			+ _TS(" files, but it should contain ") + XtoA(INT_SIZE(vRequiredFiles.size())));
		return false;
	}

	// Sort both sets of filenames
	std::sort(vRequiredFiles.begin(), vRequiredFiles.end());
	std::sort(vFiles.begin(), vFiles.end());

	// Compare both sets
	assert(vRequiredFiles.size() == vFiles.size());
	for (size_t uiIdx(0); uiIdx < vFiles.size(); ++uiIdx)
	{
		const tstring& sRequiedFile(vRequiredFiles[uiIdx]);
		const tstring& sTheirFile(vFiles[uiIdx]);

		if (sRequiedFile.compare(sTheirFile) != 0)
		{
			K2_validation_result(sMap + _T("/") + sTheirFile, K2_MAP_INVALID_INCORRECT_FILENAMES,
				_TS("One of the files within the map has an incorrect filename\n")
				+ _TS("Incorrect filename '") + sTheirFile + _TS("', expected '") + sRequiedFile + _TS("'"));
			return false;
		}
	}

	tstring sFailReason;

	// Validate the worldconfig
	tstring	world_name;
	tstring	world_image;
	uint	world_size(0);
	float	world_scale(0.0f);
	float	world_texturescale(0.0f);
	uint	world_texeldensity(0);
	uint	world_navigationdensity(0);
	float	world_groundlevel(0.0f);
	uint	world_minplayersperteam(0);
	uint	world_maxplayers(0);

	float	world_minimappaddingtop(0.0f);
	float	world_minimappaddingright(0.0f);
	float	world_minimappaddingbottom(0.0f);
	float	world_minimappaddingleft(0.0f);

	float	world_gameboundsleft(0.0f);
	float	world_gameboundstop(0.0f);
	float	world_gameboundsright(0.0f);
	float	world_gameboundsbottom(0.0f);

	float	world_cameraboundsleft(0.0f);
	float	world_cameraboundstop(0.0f);
	float	world_cameraboundsright(0.0f);
	float	world_cameraboundsbottom(0.0f);

	uint	world_cliffsize(0);
	bool	world_dev(false);
	tstring	world_modifiers;

	uint	world_iWidth[NUM_SPACE_TYPES];
	uint	world_iHeight[NUM_SPACE_TYPES];

	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("worldconfig"), pBuffer));
		if (pBuffer != NULL)
		{
			CXMLDoc cXml;
			if (cXml.ReadBuffer(pBuffer, uiSize))
			{
				CXMLNodeWrite cRoot;
				cXml.TranslateNodes(cRoot);

				// Read the properties list
				PropertyMap cRootProperties;
				if (!ValidateXML_Node(cRoot, cRootProperties, sFailReason))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node invalid: ") + sFailReason);
					return false;
				}

				// Validate world properties
				if (!ValidateXMLAttr_Name(cRootProperties, _T("name"), world_name))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'name' property"));
					return false;
				}
				if (!ValidateXMLAttr_Filepath(cRootProperties, _T("image"), world_image))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'image' property"));
					return false;
				}
				if (!ValidateXMLAttr_Uint(cRootProperties, _T("size"), world_size))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'size' property"));
					return false;
				}
				if (!ValidateXMLAttr_Float(cRootProperties, _T("scale"), world_scale))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'scale' property"));
					return false;
				}
				if (!ValidateXMLAttr_Float(cRootProperties, _T("texturescale"), world_texturescale))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'texturescale' property"));
					return false;
				}
				if (!ValidateXMLAttr_Uint(cRootProperties, _T("texeldensity"), world_texeldensity))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'texeldensity' property"));
					return false;
				}
				if (!ValidateXMLAttr_Float(cRootProperties, _T("groundlevel"), world_groundlevel))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'groundlevel' property"));
					return false;
				}
				if (!ValidateXMLAttr_Uint(cRootProperties, _T("minplayersperteam"), world_minplayersperteam))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'minplayersperteam' property"));
					return false;
				}
				if (!ValidateXMLAttr_Uint(cRootProperties, _T("maxplayers"), world_maxplayers))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'maxplayers' property"));
					return false;
				}
				if (!ValidateXMLAttr_Float(cRootProperties, _T("minimappaddingtop"), world_minimappaddingtop))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'minimappaddingtop' property"));
					return false;
				}
				if (!ValidateXMLAttr_Float(cRootProperties, _T("minimappaddingright"), world_minimappaddingright))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'minimappaddingright' property"));
					return false;
				}
				if (!ValidateXMLAttr_Float(cRootProperties, _T("minimappaddingbottom"), world_minimappaddingbottom))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'minimappaddingbottom' property"));
					return false;
				}
				if (!ValidateXMLAttr_Float(cRootProperties, _T("minimappaddingleft"), world_minimappaddingleft))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'minimappaddingleft' property"));
					return false;
				}
				if (!ValidateXMLAttr_Float(cRootProperties, _T("gameboundstop"), world_gameboundstop))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'gameboundstop' property"));
					return false;
				}
				if (!ValidateXMLAttr_Float(cRootProperties, _T("gameboundsright"), world_gameboundsright))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'gameboundsright' property"));
					return false;
				}
				if (!ValidateXMLAttr_Float(cRootProperties, _T("gameboundsbottom"), world_gameboundsbottom))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'gameboundsbottom' property"));
					return false;
				}
				if (!ValidateXMLAttr_Float(cRootProperties, _T("gameboundsleft"), world_gameboundsleft))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'gameboundsleft' property"));
					return false;
				}
				if (!ValidateXMLAttr_Float(cRootProperties, _T("cameraboundstop"), world_cameraboundstop))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'cameraboundstop' property"));
					return false;
				}
				if (!ValidateXMLAttr_Float(cRootProperties, _T("cameraboundsright"), world_cameraboundsright))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'cameraboundsright' property"));
					return false;
				}
				if (!ValidateXMLAttr_Float(cRootProperties, _T("cameraboundsbottom"), world_cameraboundsbottom))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'cameraboundsbottom' property"));
					return false;
				}
				if (!ValidateXMLAttr_Float(cRootProperties, _T("cameraboundsleft"), world_cameraboundsleft))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'cameraboundsleft' property"));
					return false;
				}
				if (!ValidateXMLAttr_Uint(cRootProperties, _T("cliffsize"), world_cliffsize))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'cliffsize' property"));
					return false;
				}
				if (!ValidateXMLAttr_Bool(cRootProperties, _T("dev"), world_dev))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'dev' property"));
					return false;
				}
				if (!ValidateXMLAttr_String(cRootProperties, _T("modifiers"), world_modifiers))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid 'modifiers' property"));
					return false;
				}

				// Validate music properties
				for (uint uiIdx(1); ; ++uiIdx)
				{
					const tstring& sMusicProp(_TS("music") + XtoA(uiIdx));
					if (cRootProperties.find(sMusicProp) == cRootProperties.end())
						break;

					tstring sMap_musicN;
					if (!ValidateXMLAttr_Filepath(cRootProperties, sMusicProp, sMap_musicN))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node does not specify a valid '") + sMusicProp + _TS("' property"));
						return false;
					}
				}

				// If there are any unexpected properties, abort
				if (!cRootProperties.empty())
				{
					SAFE_DELETE_ARRAY(pBuffer);
					const tstring& sPropName(cRootProperties.begin()->first);
					//const tstring& sPropVal(cRootProperties.begin()->second);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node contains unexpected property '") + sPropName + _TS("'"));
					return false;
				}

				// Validate world size
				if (world_size < 1 || world_size > MAX_WORLD_SIZE)
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> 'size' property must be between 1 and ") + XtoA(MAX_WORLD_SIZE));
					return false;
				}
				else
				{
					world_iWidth[TILE_SPACE] = M_Power(2, world_size);
					world_iHeight[TILE_SPACE] = M_Power(2, world_size);

					int iWidthTile, iHeightTile;

					iWidthTile = world_iWidth[TILE_SPACE];
					iHeightTile = world_iHeight[TILE_SPACE];

					world_iWidth[TILE_SPACE] = iWidthTile;
					world_iHeight[TILE_SPACE] = iHeightTile;
					world_iWidth[GRID_SPACE] = iWidthTile + 1;
					world_iHeight[GRID_SPACE] = iHeightTile + 1;
					world_iWidth[TEXEL_SPACE] = iWidthTile * world_texeldensity;
					world_iHeight[TEXEL_SPACE] = iHeightTile * world_texeldensity;
					world_iWidth[NAV_TILE_SPACE] = iWidthTile << world_navigationdensity;
					world_iHeight[NAV_TILE_SPACE] = iHeightTile << world_navigationdensity;
					world_iWidth[VIS_TILE_SPACE] = iWidthTile >> 1;
					world_iHeight[VIS_TILE_SPACE] = iHeightTile >> 1;

#define world_CliffGridWidth		(world_iWidth[TILE_SPACE] / world_cliffsize + 1)
#define world_CliffGridHeight		(world_iHeight[TILE_SPACE] / world_cliffsize + 1)
				}

				// Validate each <var /> node
				//	b = bool
				//	u = unsigned integer
				//	f = float
				//	v = vec3
				//	c = color (vec3 rgb or vec4 argb)
				//	p = filepath
				//	s = arbitrary string
				typedef map<tstring, tstring>	VarNameTypeMap;
				VarNameTypeMap mapAllowedVars;
				mapAllowedVars[_T("cg_worldAmbientSound")]				= _T("p"); // _T("/shared/sounds/ambiance/environment/default_wind.wav");
				mapAllowedVars[_T("cg_worldAmbientSoundVolume")]		= _T("f"); // _T("0.2462");
				mapAllowedVars[_T("gfx_clouds")]						= _T("b"); // _T("false");
				mapAllowedVars[_T("gfx_cloudScale")]					= _T("f"); // _T("329.8361");
				mapAllowedVars[_T("gfx_cloudSpeedX")]					= _T("f"); // _T("-227.8688");
				mapAllowedVars[_T("gfx_cloudSpeedY")]					= _T("f"); // _T("-310.6558");
				mapAllowedVars[_T("gfx_cloudTexture")]					= _T("p"); // _T("/world/sky/cloud_shadows/cloud_shadows1.tga");
				mapAllowedVars[_T("gfx_fogColor")]						= _T("c"); // _T("0.7500 0.7500 0.7500");
				mapAllowedVars[_T("gfx_fogDensity")]					= _T("f"); // _T("0.0012");
				mapAllowedVars[_T("gfx_fogFar")]						= _T("f"); // _T("10967.2119");
				mapAllowedVars[_T("gfx_fogNear")]						= _T("f"); // _T("11385.2451");
				mapAllowedVars[_T("gfx_fogScale")]						= _T("f"); // _T("0.5820");
				mapAllowedVars[_T("gfx_fogType")]						= _T("u"); // _T("0");
				mapAllowedVars[_T("scene_altSkyColor")]					= _T("c"); // _T("1.0000 1.0000 1.0000 1.0000");
				mapAllowedVars[_T("scene_altSkyOffset")]				= _T("f"); // _T("10.0000");
				mapAllowedVars[_T("scene_altSkySkin")]					= _T("s"); // _T("blue2");
				mapAllowedVars[_T("scene_altSkyTopOffset")]				= _T("f"); // _T("0.0000");
				mapAllowedVars[_T("scene_bgColor")]						= _T("v"); // _T("0.0000 0.0000 0.0000");
				mapAllowedVars[_T("scene_brightMax")]					= _T("f"); // _T("2.0000");
				mapAllowedVars[_T("scene_brightMin")]					= _T("f"); // _T("1.0000");
				mapAllowedVars[_T("scene_brightScale")]					= _T("f"); // _T("1.0000");
				mapAllowedVars[_T("scene_drawAltSky")]					= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_drawlightning")]				= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_drawMoon")]					= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_drawSky")]						= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_drawSkybox")]					= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_drawSun")]						= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_entityAmbientColor")]			= _T("v"); // _T("0.6000 0.7000 0.7000");
				mapAllowedVars[_T("scene_entitySunColor")]				= _T("v"); // _T("0.9000 0.9000 0.9000");
				mapAllowedVars[_T("scene_lightning1")]					= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_lightning10")]					= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_lightning11")]					= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_lightning12")]					= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_lightning2")]					= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_lightning3")]					= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_lightning4")]					= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_lightning5")]					= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_lightning6")]					= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_lightning7")]					= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_lightning8")]					= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_lightning9")]					= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_lightningAlpha")]				= _T("f"); // _T("1.0000");
				mapAllowedVars[_T("scene_lightningBlue")]				= _T("f"); // _T("1.0000");
				mapAllowedVars[_T("scene_lightningGreen")]				= _T("f"); // _T("1.0000");
				mapAllowedVars[_T("scene_lightningRed")]				= _T("f"); // _T("1.0000");
				mapAllowedVars[_T("scene_lightningTop1")]				= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_lightningTop2")]				= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_lightningTop3")]				= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_lightningTop4")]				= _T("b"); // _T("false");
				mapAllowedVars[_T("scene_moonColor")]					= _T("c"); // _T("1.0000 1.0000 1.0000 1.0000");
				mapAllowedVars[_T("scene_moonSize")]					= _T("f"); // _T("50.0000");
				mapAllowedVars[_T("scene_skyboxMaterial")]				= _T("p"); // _T("/world/sky/desert/desert.material");
				mapAllowedVars[_T("scene_skyColor")]					= _T("v"); // _T("1.0000 1.0000 1.0000 1.0000");
				mapAllowedVars[_T("scene_skyOffset")]					= _T("f"); // _T("10.0000");
				mapAllowedVars[_T("scene_skySkin")]						= _T("s"); // _T("blue2");
				mapAllowedVars[_T("scene_skyTopOffset")]				= _T("f"); // _T("0.0000");
				mapAllowedVars[_T("scene_sunAltitude")]					= _T("f"); // _T("50.9016");
				mapAllowedVars[_T("scene_sunAzimuth")]					= _T("f"); // _T("32.4590");
				mapAllowedVars[_T("scene_sunColor")]					= _T("c"); // _T("1.0000 1.0000 1.0000 1.0000");
				mapAllowedVars[_T("scene_sunSize")]						= _T("f"); // _T("2000.0000");
				mapAllowedVars[_T("scene_terrainAmbientColor")]			= _T("v"); // _T("0.4024 0.4268 0.4390");
				mapAllowedVars[_T("scene_terrainSunColor")]				= _T("v"); // _T("0.7195 0.7317 0.5121");
				mapAllowedVars[_T("scene_windSpeed")]					= _T("f"); // _T("1.8000");
				mapAllowedVars[_T("scene_windSpeedAlt")]				= _T("f"); // _T("1.8000");
				mapAllowedVars[_T("scene_windSpeedAltTop")]				= _T("f"); // _T("0.9000");
				mapAllowedVars[_T("scene_windSpeedTop")]				= _T("f"); // _T("0.9000");
				mapAllowedVars[_T("scene_worldFarClip")]				= _T("f"); // _T("900000000.0000");
				mapAllowedVars[_T("tod_brightMax1")]					= _T("f"); // _T("2.0000");
				mapAllowedVars[_T("tod_brightMax2")]					= _T("f"); // _T("2.0000");
				mapAllowedVars[_T("tod_brightMax3")]					= _T("f"); // _T("2.0000");
				mapAllowedVars[_T("tod_brightMax4")]					= _T("f"); // _T("2.0000");
				mapAllowedVars[_T("tod_brightMin1")]					= _T("f"); // _T("1.0000");
				mapAllowedVars[_T("tod_brightMin2")]					= _T("f"); // _T("1.0000");
				mapAllowedVars[_T("tod_brightMin3")]					= _T("f"); // _T("1.0000");
				mapAllowedVars[_T("tod_brightMin4")]					= _T("f"); // _T("1.0000");
				mapAllowedVars[_T("tod_brightScale1")]					= _T("f"); // _T("1.0000");
				mapAllowedVars[_T("tod_brightScale2")]					= _T("f"); // _T("1.0000");
				mapAllowedVars[_T("tod_brightScale3")]					= _T("f"); // _T("1.0000");
				mapAllowedVars[_T("tod_brightScale4")]					= _T("f"); // _T("1.0000");
				mapAllowedVars[_T("tod_entityAmbientColor1")]			= _T("c"); // _T("0.6000 0.7000 0.7000");
				mapAllowedVars[_T("tod_entityAmbientColor2")]			= _T("c"); // _T("0.5500 0.4800 0.7200");
				mapAllowedVars[_T("tod_entityAmbientColor3")]			= _T("c"); // _T("0.5000 0.6000 0.6000");
				mapAllowedVars[_T("tod_entityAmbientColor4")]			= _T("c"); // _T("0.3000 0.4000 0.4000");
				mapAllowedVars[_T("tod_entitySunColor1")]				= _T("c"); // _T("0.9000 0.9000 0.9000");
				mapAllowedVars[_T("tod_entitySunColor2")]				= _T("c"); // _T("1.0000 0.7200 0.0000");
				mapAllowedVars[_T("tod_entitySunColor3")]				= _T("c"); // _T("0.8000 0.8000 0.9500");
				mapAllowedVars[_T("tod_entitySunColor4")]				= _T("c"); // _T("1.0000 0.9000 0.5000");
				mapAllowedVars[_T("tod_terrainAmbientColor1")]			= _T("c"); // _T("0.4024 0.4268 0.4390");
				mapAllowedVars[_T("tod_terrainAmbientColor2")]			= _T("c"); // _T("0.4000 0.4000 0.6000");
				mapAllowedVars[_T("tod_terrainAmbientColor3")]			= _T("c"); // _T("0.3500 0.4000 0.4500");
				mapAllowedVars[_T("tod_terrainAmbientColor4")]			= _T("c"); // _T("0.3000 0.4000 0.4000");
				mapAllowedVars[_T("tod_terrainSunColor1")]				= _T("c"); // _T("0.7195 0.7317 0.5121");
				mapAllowedVars[_T("tod_terrainSunColor2")]				= _T("c"); // _T("0.9000 0.6000 0.4500");
				mapAllowedVars[_T("tod_terrainSunColor3")]				= _T("c"); // _T("0.6000 0.7000 0.8000");
				mapAllowedVars[_T("tod_terrainSunColor4")]				= _T("c"); // _T("1.0000 0.9000 0.5000");
				mapAllowedVars[_T("vid_foliageAnimateSpeed")]			= _T("f"); // _T("1.0000");
				mapAllowedVars[_T("vid_foliageAnimateStrength")]		= _T("f"); // _T("10.0000");
				mapAllowedVars[_T("vid_foliageMaxSlope")]				= _T("f"); // _T("0.5000");

				XMLNodeWriteList &cEntities(cRoot.GetChildren());
				for (XMLNodeWriteList::iterator it(cEntities.begin()), itEnd(cEntities.end()); it != itEnd; ++it)
				{
					CXMLNodeWrite &cNode(*it);

					if (cNode.GetName() != _T("var"))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <world /> node should only contain 'var' nodes, but contained unexpected node '") + cNode.GetName() + _TS("'"));
						return false;
					}

					// Read the properties list
					PropertyMap cVarProperties;
					if (!ValidateXML_Node(cNode, cVarProperties, sFailReason))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <var /> node invalid: ") + sFailReason);
						return false;
					}

					tstring sVarName;
					if (!ValidateXMLAttr_String(cVarProperties, _T("name"), sVarName))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <var /> node: name is invalid"));
						return false;
					}

					VarNameTypeMap::iterator itVarFind(mapAllowedVars.find(sVarName));
					if (itVarFind == mapAllowedVars.end())
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <var /> '") + sVarName + _TS("': not allowed to modify that cvar (security measure, contact us if you want to modify this cvar)"));
						return false;
					}

					const tstring& sVarType(itVarFind->second);
					if (sVarType == _T("b")) // Bool
					{
						bool bVarVal(false);
						if (!ValidateXMLAttr_Bool(cVarProperties, _T("value"), bVarVal))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <var /> '") + sVarName + _TS("': invalid boolean value"));
							return false;
						}
					}
					else if (sVarType == _T("u")) // Unsigned int
					{
						uint uiVarVal(0);
						if (!ValidateXMLAttr_Uint(cVarProperties, _T("value"), uiVarVal))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <var /> '") + sVarName + _TS("': invalid unsigned integer value"));
							return false;
						}
					}
					else if (sVarType == _T("f")) // Float
					{
						float fVarVal(0.0f);
						if (!ValidateXMLAttr_Float(cVarProperties, _T("value"), fVarVal))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <var /> '") + sVarName + _TS("': invalid float value"));
							return false;
						}
					}
					else if (sVarType == _T("v")) // Vec3
					{
						CVec3f v3VarVal(0.0f, 0.0f, 0.0f);
						if (!ValidateXMLAttr_Vec3(cVarProperties, _T("value"), v3VarVal))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <var /> '") + sVarName + _TS("': invalid vec3 value"));
							return false;
						}
					}
					else if (sVarType == _T("c")) // Color
					{
						CVec4f colVarVal(0.0f, 0.0f, 0.0f, 1.0f);
						if (!ValidateXMLAttr_Color(cVarProperties, _T("value"), colVarVal))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <var /> '") + sVarName + _TS("': invalid color value"));
							return false;
						}
					}
					else if (sVarType == _T("p")) // Path
					{
						tstring sVarVal;
						if (!ValidateXMLAttr_Filepath(cVarProperties, _T("value"), sVarVal))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <var /> '") + sVarName + _TS("': invalid filepath value"));
							return false;
						}
					}
					else if (sVarType == _T("s")) // String
					{
						tstring sVarVal;
						if (!ValidateXMLAttr_String(cVarProperties, _T("value"), sVarVal))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <var /> '") + sVarName + _TS("': invalid string"));
							return false;
						}
					}
					else
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <var /> '") + sVarName + _TS("': programmer error, unknown type '") + sVarType + _TS("'"));
						return false;
					}

					// If there are any unexpected properties, abort
					if (!cVarProperties.empty())
					{
						SAFE_DELETE_ARRAY(pBuffer);
						const tstring& sPropName(cVarProperties.begin()->first);
						//const tstring& sPropVal(cVarProperties.begin()->second);
						K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA, _TS("worldconfig <var /> node '") + sVarName + _TS("' contains unexpected property '") + sPropName + _TS("'"));
						return false;
					}
				}

				// worldconfig is valid
				SAFE_DELETE_ARRAY(pBuffer);
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA,
					_TS("worldconfig is not a valid XML file"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/worldconfig"), K2_MAP_INVALID_WORLDCONFIG_DATA,
				_TS("Could not open worldconfig data (programmer error?)"));
			return false;
		}
	}

	// Validate the entitylist
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("entitylist"), pBuffer));
		if (pBuffer != NULL)
		{
			CXMLDoc cXml;
			if (cXml.ReadBuffer(pBuffer, uiSize))
			{
				CXMLNodeWrite cRoot;
				cXml.TranslateNodes(cRoot);

				// Read the properties list
				PropertyMap cRootProperties;
				if (!ValidateXML_Node(cRoot, cRootProperties, sFailReason))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("<entitylist /> node invalid: ") + sFailReason);
					return false;
				}

				// If there are any unexpected properties, abort
				if (!cRootProperties.empty())
				{
					SAFE_DELETE_ARRAY(pBuffer);
					const tstring& sPropName(cRootProperties.begin()->first);
					//const tstring& sPropVal(cRootProperties.begin()->second);
					K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("<entitylist /> node contains unexpected property '") + sPropName + _TS("'"));
					return false;
				}

				uiset setIndices;

				XMLNodeWriteList &cEntities(cRoot.GetChildren());
				for (XMLNodeWriteList::iterator it(cEntities.begin()), itEnd(cEntities.end()); it != itEnd; ++it)
				{
					CXMLNodeWrite &cNode(*it);

					if (cNode.GetName() != _T("entity"))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("<entitylist /> node should only contain 'entity' nodes, but contained unexpected node '") + cNode.GetName() + _TS("'"));
						return false;
					}

					// Read the properties list
					PropertyMap cProperties;
					if (!ValidateXML_Node(cNode, cProperties, sFailReason))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <var /> node invalid: ") + sFailReason);
						return false;
					}

					// Index
					uint uiIdx(0);
					if (!ValidateXMLAttr_Uint(cProperties, _T("index"), uiIdx))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node: index is invalid"));
						return false;
					}

					if (setIndices.find(uiIdx) != setIndices.end())
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node: duplicate index '") + XtoA(uiIdx) + _TS("'"));
						return false;
					}
					setIndices.insert(uiIdx);

					// Angles
					CVec3f v3Angles(0.0f, 0.0f, 0.0f);
					if (!ValidateXMLAttr_Vec3(cProperties, _T("angles"), v3Angles))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): angles are invalid"));
						return false;
					}

					// Model
					tstring sModel;
					if (!ValidateXMLAttr_Filepath(cProperties, _T("model"), sModel))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): model is invalid"));
						return false;
					}

					// Name
					tstring sName;
					if (!ValidateXMLAttr_Name(cProperties, _T("name"), sName))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): name is invalid"));
						return false;
					}

					// Position
					CVec3f v3Position(0.0f, 0.0f, 0.0f);
					if (!ValidateXMLAttr_Vec3(cProperties, _T("position"), v3Position))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): position is invalid"));
						return false;
					}

					// Scale
					float fScale(1.0f);
					if (!ValidateXMLAttr_Float(cProperties, _T("scale"), fScale))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): scale is invalid"));
						return false;
					}

					// Seed
					uint uiSeed(0);
					if (!ValidateXMLAttr_Uint(cProperties, _T("seed"), uiSeed))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): seed is invalid"));
						return false;
					}

					if (cProperties.find(_T("config")) != cProperties.end())
					{
						// Config
						tstring sConfig;
						if (!ValidateXMLAttr_String(cProperties, _T("config"), sConfig))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): config is invalid"));
							return false;
						}
						// TODO: Validate entity config
					}

					// TODO: should skin be optional?
					if (cProperties.find(_T("skin")) != cProperties.end())
					{
						// Skin
						tstring sSkin;
						if (!ValidateXMLAttr_String(cProperties, _T("skin"), sSkin))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): skin is invalid"));
							return false;
						}
						// TODO: Validate entity skin
					}

					// Validate target properties
					for (uint uiIdx(0); uiIdx < MAX_ENTITY_TARGET_PROPERTIES; ++uiIdx)
					{
						const tstring& sTargetProp(_TS("target") + XtoA(uiIdx));
						if (cProperties.find(sTargetProp) == cProperties.end())
							continue;

						tstring sMap_targetN;
						if (!ValidateXMLAttr_Name(cProperties, sTargetProp, sMap_targetN))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): invalid '") + sTargetProp + _TS("' property"));
							return false;
						}
					}

					// Team
					uint uiTeam(0);
					if (!ValidateXMLAttr_Uint(cProperties, _T("team"), uiTeam))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): team is invalid"));
						return false;
					}

					// Type
					tstring sType;
					if (!ValidateXMLAttr_String(cProperties, _T("type"), sType))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): type is invalid"));
						return false;
					}
					// TODO: Validate entity type

					if (cProperties.find(_T("effecttype")) != cProperties.end())
					{
						// effecttype
						tstring sEffectType;
						if (!ValidateXMLAttr_String(cProperties, _T("effecttype"), sEffectType))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): effecttype is invalid"));
							return false;
						}
						// TODO: Validate effecttype
					}

					if (cProperties.find(_T("level")) != cProperties.end())
					{
						// level
						uint uiLevel(1);
						if (!ValidateXMLAttr_Uint(cProperties, _T("level"), uiLevel))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): level is invalid"));
							return false;
						}
					}

					if (cProperties.find(_T("icon")) != cProperties.end())
					{
						// icon
						tstring sIcon;
						if (!ValidateXMLAttr_Filepath(cProperties, _T("icon"), sIcon))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): icon is invalid"));
							return false;
						}
					}

					if (cProperties.find(_T("iconcolor")) != cProperties.end())
					{
						// iconcolor
						CVec4f v4IconColor(0.0f, 0.0f, 0.0f, 1.0f);
						if (!ValidateXMLAttr_Color(cProperties, _T("iconcolor"), v4IconColor))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): iconcolor is invalid"));
							return false;
						}
					}

					if (cProperties.find(_T("iconsize")) != cProperties.end())
					{
						// iconsize
						float fIconSize(0.0500f);
						if (!ValidateXMLAttr_Float(cProperties, _T("iconsize"), fIconSize))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): iconsize is invalid"));
							return false;
						}
					}

					if (cProperties.find(_T("firstlevel")) != cProperties.end())
					{
						// firstlevel
						uint uiFirstLevel(1);
						if (!ValidateXMLAttr_Uint(cProperties, _T("firstlevel"), uiFirstLevel))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): firstlevel is invalid"));
							return false;
						}
					}

					if (cProperties.find(_T("maxlevel")) != cProperties.end())
					{
						// maxlevel
						uint uiMaxLevel(255);
						if (!ValidateXMLAttr_Uint(cProperties, _T("maxlevel"), uiMaxLevel))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): maxlevel is invalid"));
							return false;
						}
					}

					if (cProperties.find(_T("firstspawntime")) != cProperties.end())
					{
						// firstspawntime
						uint uiFirstSpawnTime(0);
						if (!ValidateXMLAttr_Uint(cProperties, _T("firstspawntime"), uiFirstSpawnTime))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): firstspawntime is invalid"));
							return false;
						}
					}

					if (cProperties.find(_T("respawninterval")) != cProperties.end())
					{
						// respawninterval
						uint uiRespawnInterval(600000);
						if (!ValidateXMLAttr_Uint(cProperties, _T("respawninterval"), uiRespawnInterval))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): respawninterval is invalid"));
							return false;
						}
					}

#if 0
					if (sType == _T("Prop_Water"))
					{
					}
					else if (sType == _T("Building_LegionTower"))
					{
					}
					else if (sType == _T("Building_HellbourneTower"))
					{
					}
					else if (sType == _T("Entity_CritterSpawner"))
					{
					}
					else if (sType == _T("Entity_NeutralCampSpawner"))
					{
					}
					else if (sType == _T("Entity_NeutralCampController"))
					{
					}
					else if (sType == _T("Gadget_Fairy_Ability4_NoPort"))
					{
					}
					else if (sType == _T("Entity_LaneNode"))
					{
					}
					else if (sType == _T("Trigger_SpawnPoint"))
					{
					}
					else if (sType == _T("Entity_PowerupSpawner"))
					{
					}
					else if (sType == _T("Entity_CreepSpawner"))
					{
					}
					else if (sType == _T("Entity_BossSpawner"))
					{
					}
					else if (sType == _T("Entity_BossController"))
					{
						// <entity
						//	angles="0.0000 0.0000 -98.2000"
						//	firstlevel="1"
						//	firstspawntime="0"
						//	icon="/shared/icons/minimap_circle.tga"
						//	iconcolor="0.6 0 0"
						//	iconsize="0.0500"
						//	index="3257"
						//	maxlevel="11"
						//	model="/tools/m.mdf"
						//	name="Kongor_Controller"
						//	position="10695.4082 7677.9038 -128.0000"
						//	respawninterval="600000"
						//	scale="1.0000"
						//	seed="0"
						//	skin="default"
						//	target0="Kongor_Spawner"
						//	target1=""
						//	target2=""
						//	team="0"
						//	type="Entity_BossController"></entity>
					}
					else if(sType == _TS("Prop_Cliff"))
					{
					}
					else if(sType == _TS("Prop_Cliff2"))
					{
					}
					else if(sType == _TS("Prop_Tree"))
					{
					}
					else if(sType == _TS("Prop_Scenery"))
					{
					}
					else if(sType == _TS("Prop_Static"))
					{
					}
					else if(sType == _TS("Building_Outpost"))
					{
					}
					else if(sType == _TS("Building_HellbourneBase"))
					{
					}
					else if(sType == _TS("Building_LegionBase"))
					{
					}
					else if(sType == _TS("Building_HellbourneWell"))
					{
					}
					else if(sType == _TS("Building_LegionWell"))
					{
					}
					else if(sType == _TS("Building_HellbourneWell_Attack"))
					{
					}
					else if(sType == _TS("Building_LegionWell_Attack"))
					{
					}
					else if(sType == _TS("Building_HellbourneRangedBarracks"))
					{
					}
					else if(sType == _TS("Building_LegionRangedBarracks"))
					{
					}
					else if(sType == _TS("Building_HellbourneMeleeBarracks"))
					{
					}
					else if(sType == _TS("Building_LegionMeleeBarracks"))
					{
					}
					else if(sType == _TS("Building_HellbourneStore"))
					{
					}
					else if(sType == _TS("Building_HellbourneWellStore"))
					{
					}
					else if(sType == _TS("Building_LegionStore"))
					{
					}
					else if(sType == _TS("Building_LegionWellStore"))
					{
					}
					else if(sType == _TS("Building_HellbourneRepeater1"))
					{
					}
					else if(sType == _TS("Building_HellbourneRepeater2"))
					{
					}
					else if(sType == _TS("Building_HellbourneRepeater3"))
					{
					}
					else if(sType == _TS("Building_HellbourneRepeater4"))
					{
					}
					else if(sType == _TS("Building_LegionRepeater1"))
					{
					}
					else if(sType == _TS("Building_LegionRepeater2"))
					{
					}
					else if(sType == _TS("Building_LegionRepeater3"))
					{
					}
					else if(sType == _TS("Building_LegionRepeater4"))
					{
					}
					else
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): type '") + sType + _TS("' is invalid"));
						return false;
					}
#endif

					// If there are any unexpected properties, abort
					if (!cProperties.empty())
					{
						SAFE_DELETE_ARRAY(pBuffer);
						const tstring& sPropName(cProperties.begin()->first);
						//const tstring& sPropVal(cProperties.begin()->second);
						K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA, _TS("entitylist <entity /> node (index ") + XtoA(uiIdx) + _TS("): unexpected property '") + sPropName + _TS("'"));
						return false;
					}
				}

				// entitylist is valid
				SAFE_DELETE_ARRAY(pBuffer);
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA,
					_TS("entitylist is not a valid XML file"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/entitylist"), K2_MAP_INVALID_ENTITYLIST_DATA,
				_TS("Could not open entitylist data (programmer error?)"));
			return false;
		}
	}

	// Validate the materiallist
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("materiallist"), pBuffer));
		if (pBuffer != NULL)
		{
			CXMLDoc cXml;
			if (cXml.ReadBuffer(pBuffer, uiSize))
			{
				CXMLNodeWrite cRoot;
				cXml.TranslateNodes(cRoot);

				// Read the properties list
				PropertyMap cRootProperties;
				if (!ValidateXML_Node(cRoot, cRootProperties, sFailReason))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/materiallist"), K2_MAP_INVALID_MATERIALLIST_DATA, _TS("<materiallist /> node invalid: ") + sFailReason);
					return false;
				}

				// If there are any unexpected properties, abort
				if (!cRootProperties.empty())
				{
					SAFE_DELETE_ARRAY(pBuffer);
					const tstring& sPropName(cRootProperties.begin()->first);
					//const tstring& sPropVal(cRootProperties.begin()->second);
					K2_validation_result(sMap + _T("/materiallist"), K2_MAP_INVALID_MATERIALLIST_DATA, _TS("<materiallist /> node contains unexpected property '") + sPropName + _TS("'"));
					return false;
				}

				uiset setIndices;

				XMLNodeWriteList &cEntities(cRoot.GetChildren());
				for (XMLNodeWriteList::iterator it(cEntities.begin()), itEnd(cEntities.end()); it != itEnd; ++it)
				{
					CXMLNodeWrite &cNode(*it);

					if (cNode.GetName() != _T("material"))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/materiallist"), K2_MAP_INVALID_MATERIALLIST_DATA, _TS("<materiallist /> node should only contain 'material' nodes, but contained unexpected node '") + cNode.GetName() + _TS("'"));
						return false;
					}

					// Read the properties list
					PropertyMap cProperties;
					if (!ValidateXML_Node(cNode, cProperties, sFailReason))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/materiallist"), K2_MAP_INVALID_MATERIALLIST_DATA, _TS("materiallist <material /> node invalid: ") + sFailReason);
						return false;
					}

					// id
					uint uiIdx(0);
					if (!ValidateXMLAttr_Uint(cProperties, _T("id"), uiIdx))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/materiallist"), K2_MAP_INVALID_MATERIALLIST_DATA, _TS("materiallist <material /> node: id is invalid"));
						return false;
					}

					if (setIndices.find(uiIdx) != setIndices.end())
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/materiallist"), K2_MAP_INVALID_MATERIALLIST_DATA, _TS("materiallist <material /> node: duplicate id '") + XtoA(uiIdx) + _TS("'"));
						return false;
					}
					setIndices.insert(uiIdx);

					// name
					tstring sName;
					if (!ValidateXMLAttr_Filepath(cProperties, _T("name"), sName))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/materiallist"), K2_MAP_INVALID_MATERIALLIST_DATA, _TS("materiallist <material /> node (index ") + XtoA(uiIdx) + _TS("): name is invalid"));
						return false;
					}

					// If there are any unexpected properties, abort
					if (!cProperties.empty())
					{
						SAFE_DELETE_ARRAY(pBuffer);
						const tstring& sPropName(cProperties.begin()->first);
						//const tstring& sPropVal(cProperties.begin()->second);
						K2_validation_result(sMap + _T("/materiallist"), K2_MAP_INVALID_MATERIALLIST_DATA, _TS("materiallist <material /> node (index ") + XtoA(uiIdx) + _TS("): unexpected property '") + sPropName + _TS("'"));
						return false;
					}
				}

				// materiallist is valid
				SAFE_DELETE_ARRAY(pBuffer);
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/materiallist"), K2_MAP_INVALID_MATERIALLIST_DATA,
					_TS("materiallist is not a valid XML file"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/materiallist"), K2_MAP_INVALID_MATERIALLIST_DATA,
				_TS("Could not open materiallist data (programmer error?)"));
			return false;
		}
	}

	// Validate the texturelist
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("texturelist"), pBuffer));
		if (pBuffer != NULL)
		{
			CXMLDoc cXml;
			if (cXml.ReadBuffer(pBuffer, uiSize))
			{
				CXMLNodeWrite cRoot;
				cXml.TranslateNodes(cRoot);

				// Read the properties list
				PropertyMap cRootProperties;
				if (!ValidateXML_Node(cRoot, cRootProperties, sFailReason))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/texturelist"), K2_MAP_INVALID_TEXTURELIST_DATA, _TS("<texturelist /> node invalid: ") + sFailReason);
					return false;
				}

				// If there are any unexpected properties, abort
				if (!cRootProperties.empty())
				{
					SAFE_DELETE_ARRAY(pBuffer);
					const tstring& sPropName(cRootProperties.begin()->first);
					//const tstring& sPropVal(cRootProperties.begin()->second);
					K2_validation_result(sMap + _T("/texturelist"), K2_MAP_INVALID_TEXTURELIST_DATA, _TS("<texturelist /> node contains unexpected property '") + sPropName + _TS("'"));
					return false;
				}

				uiset setIndices;

				XMLNodeWriteList &cEntities(cRoot.GetChildren());
				for (XMLNodeWriteList::iterator it(cEntities.begin()), itEnd(cEntities.end()); it != itEnd; ++it)
				{
					CXMLNodeWrite &cNode(*it);

					if (cNode.GetName() != _T("texture"))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/texturelist"), K2_MAP_INVALID_TEXTURELIST_DATA, _TS("<texturelist /> node should only contain 'texture' nodes, but contained unexpected node '") + cNode.GetName() + _TS("'"));
						return false;
					}

					// Read the properties list
					PropertyMap cProperties;
					if (!ValidateXML_Node(cNode, cProperties, sFailReason))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/texturelist"), K2_MAP_INVALID_TEXTURELIST_DATA, _TS("texturelist <texture /> node invalid: ") + sFailReason);
						return false;
					}

					// id
					uint uiIdx(0);
					if (!ValidateXMLAttr_Uint(cProperties, _T("id"), uiIdx))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/texturelist"), K2_MAP_INVALID_TEXTURELIST_DATA, _TS("texturelist <texture /> node: id is invalid"));
						return false;
					}

					if (setIndices.find(uiIdx) != setIndices.end())
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/texturelist"), K2_MAP_INVALID_TEXTURELIST_DATA, _TS("texturelist <texture /> node: duplicate id '") + XtoA(uiIdx) + _TS("'"));
						return false;
					}
					setIndices.insert(uiIdx);

					// name
					tstring sName;
					if (!ValidateXMLAttr_Filepath(cProperties, _T("name"), sName))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/texturelist"), K2_MAP_INVALID_TEXTURELIST_DATA, _TS("texturelist <texture /> node (index ") + XtoA(uiIdx) + _TS("): name is invalid"));
						return false;
					}

					// If there are any unexpected properties, abort
					if (!cProperties.empty())
					{
						SAFE_DELETE_ARRAY(pBuffer);
						const tstring& sPropName(cProperties.begin()->first);
						//const tstring& sPropVal(cProperties.begin()->second);
						K2_validation_result(sMap + _T("/texturelist"), K2_MAP_INVALID_TEXTURELIST_DATA, _TS("texturelist <texture /> node (index ") + XtoA(uiIdx) + _TS("): unexpected property '") + sPropName + _TS("'"));
						return false;
					}
				}

				// texturelist is valid
				SAFE_DELETE_ARRAY(pBuffer);
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/texturelist"), K2_MAP_INVALID_TEXTURELIST_DATA,
					_TS("texturelist is not a valid XML file"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/texturelist"), K2_MAP_INVALID_TEXTURELIST_DATA,
				_TS("Could not open texturelist data (programmer error?)"));
			return false;
		}
	}

	// Validate the soundlist
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("soundlist"), pBuffer));
		if (pBuffer != NULL)
		{
			CXMLDoc cXml;
			if (cXml.ReadBuffer(pBuffer, uiSize))
			{
				CXMLNodeWrite cRoot;
				cXml.TranslateNodes(cRoot);

				// Read the properties list
				PropertyMap cRootProperties;
				if (!ValidateXML_Node(cRoot, cRootProperties, sFailReason))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/soundlist"), K2_MAP_INVALID_SOUNDLIST_DATA, _TS("<soundlist /> node invalid: ") + sFailReason);
					return false;
				}

				// If there are any unexpected properties, abort
				if (!cRootProperties.empty())
				{
					SAFE_DELETE_ARRAY(pBuffer);
					const tstring& sPropName(cRootProperties.begin()->first);
					//const tstring& sPropVal(cRootProperties.begin()->second);
					K2_validation_result(sMap + _T("/soundlist"), K2_MAP_INVALID_SOUNDLIST_DATA, _TS("<soundlist /> node contains unexpected property '") + sPropName + _TS("'"));
					return false;
				}

				uiset setIndices;

				XMLNodeWriteList &cEntities(cRoot.GetChildren());

				for (XMLNodeWriteList::iterator it(cEntities.begin()), itEnd(cEntities.end()); it != itEnd; ++it)
				{
					CXMLNodeWrite &cNode(*it);

					if (cNode.GetName() != _T("sound"))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/soundlist"), K2_MAP_INVALID_SOUNDLIST_DATA, _TS("<soundlist /> node should only contain 'sound' nodes, but contained unexpected node '") + cNode.GetName() + _TS("'"));
						return false;
					}

					// Read the properties list
					PropertyMap cProperties;
					if (!ValidateXML_Node(cNode, cProperties, sFailReason))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/soundlist"), K2_MAP_INVALID_SOUNDLIST_DATA, _TS("soundlist <sound /> node invalid: ") + sFailReason);
						return false;
					}

					// index
					uint uiIdx(0);
					if (!ValidateXMLAttr_Uint(cProperties, _T("index"), uiIdx))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/soundlist"), K2_MAP_INVALID_SOUNDLIST_DATA, _TS("soundlist <sound /> node: 'index' is invalid"));
						return false;
					}

					if (setIndices.find(uiIdx) != setIndices.end())
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/soundlist"), K2_MAP_INVALID_SOUNDLIST_DATA, _TS("soundlist <sound /> node: duplicate index '") + XtoA(uiIdx) + _TS("'"));
						return false;
					}
					setIndices.insert(uiIdx);

					// position
					CVec3f v3Pos(0.0f, 0.0f, 0.0f);
					if (!ValidateXMLAttr_Vec3(cProperties, _T("position"), v3Pos))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/soundlist"), K2_MAP_INVALID_SOUNDLIST_DATA, _TS("soundlist <sound /> node (index ") + XtoA(uiIdx) + _TS("): 'position' is invalid"));
						return false;
					}

					// sound
					tstring sSound;
					if (!ValidateXMLAttr_Filepath(cProperties, _T("sound"), sSound))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/soundlist"), K2_MAP_INVALID_SOUNDLIST_DATA, _TS("soundlist <sound /> node (index ") + XtoA(uiIdx) + _TS("): 'sound' is invalid"));
						return false;
					}

					// volume
					float fVolume(1.0f);
					if (!ValidateXMLAttr_Float(cProperties, _T("volume"), fVolume))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/soundlist"), K2_MAP_INVALID_SOUNDLIST_DATA, _TS("soundlist <sound /> node (index ") + XtoA(uiIdx) + _TS("): 'volume' is invalid"));
						return false;
					}

					// falloff
					float fFalloff(1.0f);
					if (!ValidateXMLAttr_Float(cProperties, _T("falloff"), fFalloff))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/soundlist"), K2_MAP_INVALID_SOUNDLIST_DATA, _TS("soundlist <sound /> node (index ") + XtoA(uiIdx) + _TS("): 'falloff' is invalid"));
						return false;
					}

					// minloopdelay
					uint uiMinLoopDelay(0);
					if (!ValidateXMLAttr_Uint(cProperties, _T("minloopdelay"), uiMinLoopDelay))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/soundlist"), K2_MAP_INVALID_SOUNDLIST_DATA, _TS("soundlist <sound /> node (index ") + XtoA(uiIdx) + _TS("): 'minloopdelay' is invalid"));
						return false;
					}

					// maxloopdelay
					uint uiMaxLoopDelay(0);
					if (!ValidateXMLAttr_Uint(cProperties, _T("maxloopdelay"), uiMaxLoopDelay))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/soundlist"), K2_MAP_INVALID_SOUNDLIST_DATA, _TS("soundlist <sound /> node (index ") + XtoA(uiIdx) + _TS("): 'maxloopdelay' is invalid"));
						return false;
					}

					// If there are any unexpected properties, abort
					if (!cProperties.empty())
					{
						SAFE_DELETE_ARRAY(pBuffer);
						const tstring& sPropName(cProperties.begin()->first);
						//const tstring& sPropVal(cProperties.begin()->second);
						K2_validation_result(sMap + _T("/soundlist"), K2_MAP_INVALID_SOUNDLIST_DATA, _TS("soundlist <sound /> node (index ") + XtoA(uiIdx) + _TS("): unexpected property '") + sPropName + _TS("'"));
						return false;
					}
				}

				// soundlist is valid
				SAFE_DELETE_ARRAY(pBuffer);
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/soundlist"), K2_MAP_INVALID_SOUNDLIST_DATA,
					_TS("soundlist is not a valid XML file"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/soundlist"), K2_MAP_INVALID_SOUNDLIST_DATA,
				_TS("Could not open soundlist data (programmer error?)"));
			return false;
		}
	}

	// Validate the lightlist
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("lightlist"), pBuffer));
		if (pBuffer != NULL)
		{
			CXMLDoc cXml;
			if (cXml.ReadBuffer(pBuffer, uiSize))
			{
				CXMLNodeWrite cRoot;
				cXml.TranslateNodes(cRoot);

				// Read the properties list
				PropertyMap cRootProperties;
				if (!ValidateXML_Node(cRoot, cRootProperties, sFailReason))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/lightlist"), K2_MAP_INVALID_LIGHTLIST_DATA, _TS("<lightlist /> node invalid: ") + sFailReason);
					return false;
				}

				// If there are any unexpected properties, abort
				if (!cRootProperties.empty())
				{
					SAFE_DELETE_ARRAY(pBuffer);
					const tstring& sPropName(cRootProperties.begin()->first);
					//const tstring& sPropVal(cRootProperties.begin()->second);
					K2_validation_result(sMap + _T("/lightlist"), K2_MAP_INVALID_LIGHTLIST_DATA, _TS("<lightlist /> node contains unexpected property '") + sPropName + _TS("'"));
					return false;
				}

				uiset setIndices;

				XMLNodeWriteList &cEntities(cRoot.GetChildren());

				for (XMLNodeWriteList::iterator it(cEntities.begin()), itEnd(cEntities.end()); it != itEnd; ++it)
				{
					CXMLNodeWrite &cNode(*it);

					if (cNode.GetName() != _T("light"))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/lightlist"), K2_MAP_INVALID_LIGHTLIST_DATA, _TS("<lightlist /> node should only contain 'light' nodes, but contained unexpected node '") + cNode.GetName() + _TS("'"));
						return false;
					}

					// Read the properties list
					PropertyMap cProperties;
					if (!ValidateXML_Node(cNode, cProperties, sFailReason))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/lightlist"), K2_MAP_INVALID_LIGHTLIST_DATA, _TS("lightlist <light /> node invalid: ") + sFailReason);
						return false;
					}

					// index
					uint uiIdx(0);
					if (!ValidateXMLAttr_Uint(cProperties, _T("index"), uiIdx))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/lightlist"), K2_MAP_INVALID_LIGHTLIST_DATA, _TS("lightlist <light /> node: 'index' is invalid"));
						return false;
					}

					if (setIndices.find(uiIdx) != setIndices.end())
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/lightlist"), K2_MAP_INVALID_LIGHTLIST_DATA, _TS("lightlist <light /> node: duplicate index '") + XtoA(uiIdx) + _TS("'"));
						return false;
					}
					setIndices.insert(uiIdx);

					// position
					CVec3f v3Pos(0.0f, 0.0f, 0.0f);
					if (!ValidateXMLAttr_Vec3(cProperties, _T("position"), v3Pos))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/lightlist"), K2_MAP_INVALID_LIGHTLIST_DATA, _TS("lightlist <light /> node (index ") + XtoA(uiIdx) + _TS("): 'position' is invalid"));
						return false;
					}

					// color
					CVec3f v3Col(1.0f, 1.0f, 1.0f);
					if (!ValidateXMLAttr_Vec3(cProperties, _T("color"), v3Col))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/lightlist"), K2_MAP_INVALID_LIGHTLIST_DATA, _TS("lightlist <light /> node (index ") + XtoA(uiIdx) + _TS("): 'color' is invalid"));
						return false;
					}

					// falloffstart
					float fFalloffStart(0.0f);
					if (!ValidateXMLAttr_Float(cProperties, _T("falloffstart"), fFalloffStart))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/lightlist"), K2_MAP_INVALID_LIGHTLIST_DATA, _TS("lightlist <light /> node (index ") + XtoA(uiIdx) + _TS("): 'falloffstart' is invalid"));
						return false;
					}

					// falloffend
					float fFalloffEnd(0.0f);
					if (!ValidateXMLAttr_Float(cProperties, _T("falloffend"), fFalloffEnd))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/lightlist"), K2_MAP_INVALID_LIGHTLIST_DATA, _TS("lightlist <light /> node (index ") + XtoA(uiIdx) + _TS("): 'falloffend' is invalid"));
						return false;
					}

					// If there are any unexpected properties, abort
					if (!cProperties.empty())
					{
						SAFE_DELETE_ARRAY(pBuffer);
						const tstring& sPropName(cProperties.begin()->first);
						//const tstring& sPropVal(cProperties.begin()->second);
						K2_validation_result(sMap + _T("/lightlist"), K2_MAP_INVALID_LIGHTLIST_DATA, _TS("lightlist <light /> node (index ") + XtoA(uiIdx) + _TS("): unexpected property '") + sPropName + _TS("'"));
						return false;
					}
				}

				// lightlist is valid
				SAFE_DELETE_ARRAY(pBuffer);
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/lightlist"), K2_MAP_INVALID_LIGHTLIST_DATA,
					_TS("lightlist is not a valid XML file"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/lightlist"), K2_MAP_INVALID_LIGHTLIST_DATA,
				_TS("Could not open lightlist data (programmer error?)"));
			return false;
		}
	}

	// Validate the triggerlist
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("triggerlist"), pBuffer));
		if (pBuffer != NULL)
		{
			CXMLDoc cXml;
			if (cXml.ReadBuffer(pBuffer, uiSize))
			{
				CXMLNodeWrite cRoot;
				cXml.TranslateNodes(cRoot);

				// Read the properties list
				PropertyMap cRootProperties;
				if (!ValidateXML_Node(cRoot, cRootProperties, sFailReason))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/triggerlist"), K2_MAP_INVALID_TRIGGERLIST_DATA, _TS("<triggerlist /> node invalid: ") + sFailReason);
					return false;
				}

				// If there are any unexpected properties, abort
				if (!cRootProperties.empty())
				{
					SAFE_DELETE_ARRAY(pBuffer);
					const tstring& sPropName(cRootProperties.begin()->first);
					//const tstring& sPropVal(cRootProperties.begin()->second);
					K2_validation_result(sMap + _T("/triggerlist"), K2_MAP_INVALID_TRIGGERLIST_DATA, _TS("<triggerlist /> node contains unexpected property '") + sPropName + _TS("'"));
					return false;
				}

				set<tstring> setNames;

				XMLNodeWriteList &cEntities(cRoot.GetChildren());

				for (XMLNodeWriteList::iterator it(cEntities.begin()), itEnd(cEntities.end()); it != itEnd; ++it)
				{
					CXMLNodeWrite &cNode(*it);

					if (cNode.GetName() != _T("trigger"))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/triggerlist"), K2_MAP_INVALID_TRIGGERLIST_DATA, _TS("<triggerlist /> node should only contain 'trigger' nodes, but contained unexpected node '") + cNode.GetName() + _TS("'"));
						return false;
					}

					// Read the properties list
					PropertyMap cProperties;
					if (!ValidateXML_Node(cNode, cProperties, sFailReason))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/triggerlist"), K2_MAP_INVALID_TRIGGERLIST_DATA, _TS("triggerlist <trigger /> node invalid: ") + sFailReason);
						return false;
					}

					// name
					tstring sName;
					if (!ValidateXMLAttr_String(cProperties, _T("name"), sName))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/triggerlist"), K2_MAP_INVALID_TRIGGERLIST_DATA, _TS("triggerlist <trigger /> node: 'name' is invalid"));
						return false;
					}

					if (setNames.find(sName) != setNames.end())
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/triggerlist"), K2_MAP_INVALID_TRIGGERLIST_DATA, _TS("triggerlist <trigger /> node: duplicate name '") + XtoA(sName) + _TS("'"));
						return false;
					}
					setNames.insert(sName);

					// content
					tstring sContent;
					if (!ValidateXMLAttr_String(cProperties, _T("content"), sContent))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/triggerlist"), K2_MAP_INVALID_TRIGGERLIST_DATA, _TS("triggerlist <trigger /> node: 'content' is invalid"));
						return false;
					}

					// If there are any unexpected properties, abort
					if (!cProperties.empty())
					{
						SAFE_DELETE_ARRAY(pBuffer);
						const tstring& sPropName(cProperties.begin()->first);
						//const tstring& sPropVal(cProperties.begin()->second);
						K2_validation_result(sMap + _T("/triggerlist"), K2_MAP_INVALID_TRIGGERLIST_DATA, _TS("triggerlist <trigger /> node (name '") + XtoA(sName) + _TS("'): unexpected property '") + sPropName + _TS("'"));
						return false;
					}
				}

				// triggerlist is valid
				SAFE_DELETE_ARRAY(pBuffer);
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/triggerlist"), K2_MAP_INVALID_TRIGGERLIST_DATA,
					_TS("triggerlist is not a valid XML file"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/triggerlist"), K2_MAP_INVALID_TRIGGERLIST_DATA,
				_TS("Could not open triggerlist data (programmer error?)"));
			return false;
		}
	}

	// Validate the worldoccluderlist
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("worldoccluderlist"), pBuffer));
		if (pBuffer != NULL)
		{
			CXMLDoc cXml;
			if (cXml.ReadBuffer(pBuffer, uiSize))
			{
				CXMLNodeWrite cRoot;
				cXml.TranslateNodes(cRoot);

				// Read the properties list
				PropertyMap cRootProperties;
				if (!ValidateXML_Node(cRoot, cRootProperties, sFailReason))
				{
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/worldoccluderlist"), K2_MAP_INVALID_WORLDOCCLUDERLIST_DATA, _TS("<occluderlist /> node invalid: ") + sFailReason);
					return false;
				}

				// If there are any unexpected properties, abort
				if (!cRootProperties.empty())
				{
					SAFE_DELETE_ARRAY(pBuffer);
					const tstring& sPropName(cRootProperties.begin()->first);
					//const tstring& sPropVal(cRootProperties.begin()->second);
					K2_validation_result(sMap + _T("/worldoccluderlist"), K2_MAP_INVALID_WORLDOCCLUDERLIST_DATA, _TS("<occluderlist /> node contains unexpected property '") + sPropName + _TS("'"));
					return false;
				}

				uiset setIndices;

				XMLNodeWriteList &cEntities(cRoot.GetChildren());

				for (XMLNodeWriteList::iterator it(cEntities.begin()), itEnd(cEntities.end()); it != itEnd; ++it)
				{
					CXMLNodeWrite &cNode(*it);

					if (cNode.GetName() != _T("occluder"))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/worldoccluderlist"), K2_MAP_INVALID_WORLDOCCLUDERLIST_DATA, _TS("<occluderlist /> node should only contain 'occluder' nodes, but contained unexpected node '") + cNode.GetName() + _TS("'"));
						return false;
					}

					// Read the properties list
					PropertyMap cProperties;
					if (!ValidateXML_Node(cNode, cProperties, sFailReason))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/worldoccluderlist"), K2_MAP_INVALID_WORLDOCCLUDERLIST_DATA, _TS("occluderlist <occluder /> node invalid: ") + sFailReason);
						return false;
					}

					// index
					uint uiIdx(0);
					if (!ValidateXMLAttr_Uint(cProperties, _T("index"), uiIdx))
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/worldoccluderlist"), K2_MAP_INVALID_WORLDOCCLUDERLIST_DATA, _TS("occluderlist <occluder /> node: 'index' is invalid"));
						return false;
					}

					if (setIndices.find(uiIdx) != setIndices.end())
					{
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/worldoccluderlist"), K2_MAP_INVALID_WORLDOCCLUDERLIST_DATA, _TS("occluderlist <occluder /> node: duplicate index '") + XtoA(uiIdx) + _TS("'"));
						return false;
					}
					setIndices.insert(uiIdx);

					// If there are any unexpected properties, abort
					if (!cProperties.empty())
					{
						SAFE_DELETE_ARRAY(pBuffer);
						const tstring& sPropName(cProperties.begin()->first);
						//const tstring& sPropVal(cProperties.begin()->second);
						K2_validation_result(sMap + _T("/worldoccluderlist"), K2_MAP_INVALID_WORLDOCCLUDERLIST_DATA, _TS("occluderlist <occluder /> node (index ") + XtoA(uiIdx) + _TS("): unexpected property '") + sPropName + _TS("'"));
						return false;
					}

					XMLNodeWriteList &cChildEntities(cNode.GetChildren());
					for (XMLNodeWriteList::iterator it2(cChildEntities.begin()), it2End(cChildEntities.end()); it2 != it2End; ++it2)
					{
						CXMLNodeWrite &cChildNode(*it2);

						if (cChildNode.GetName() != _T("point"))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/worldoccluderlist"), K2_MAP_INVALID_WORLDOCCLUDERLIST_DATA, _TS("<occluder /> node should only contain 'point' nodes, but contained unexpected node '") + cChildNode.GetName() + _TS("'"));
							return false;
						}

						// Read the properties list
						PropertyMap cChildProperties;
						if (!ValidateXML_Node(cChildNode, cChildProperties, sFailReason))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/worldoccluderlist"), K2_MAP_INVALID_WORLDOCCLUDERLIST_DATA, _TS("occluderlist <occluder /> node (index ") + XtoA(uiIdx) + _TS("), 'point' node invalid: ") + sFailReason);
							return false;
						}

						// position
						CVec3f v3Pos(0.0f, 0.0f, 0.0f);
						if (!ValidateXMLAttr_Vec3(cChildProperties, _T("position"), v3Pos))
						{
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/worldoccluderlist"), K2_MAP_INVALID_WORLDOCCLUDERLIST_DATA, _TS("occluderlist <occluder /> node (index ") + XtoA(uiIdx) + _TS("), 'point' node: 'position' is invalid"));
							return false;
						}

						// If there are any unexpected properties, abort
						if (!cChildProperties.empty())
						{
							SAFE_DELETE_ARRAY(pBuffer);
							const tstring& sPropName(cChildProperties.begin()->first);
							//const tstring& sPropVal(cChildProperties.begin()->second);
							K2_validation_result(sMap + _T("/worldoccluderlist"), K2_MAP_INVALID_WORLDOCCLUDERLIST_DATA, _TS("occluderlist <occluder /> node (index ") + XtoA(uiIdx) + _TS("), 'point' node: unexpected property '") + sPropName + _TS("'"));
							return false;
						}
					}
				}

				// worldoccluderlist is valid
				SAFE_DELETE_ARRAY(pBuffer);
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/worldoccluderlist"), K2_MAP_INVALID_WORLDOCCLUDERLIST_DATA,
					_TS("worldoccluderlist is not a valid XML file"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/worldoccluderlist"), K2_MAP_INVALID_WORLDOCCLUDERLIST_DATA,
				_TS("Could not open worldoccluderlist data (programmer error?)"));
			return false;
		}
	}

	// Validate the heightmap
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("heightmap"), pBuffer));
		if (pBuffer != NULL)
		{
			FILE_INTERFACE* fph(fi_fopen("heightmap", "rb", pBuffer, uiSize));
			if (fph != NULL)
			{
				bool bNewFormat(false);
				int iWidth(0);
				int iHeight(0);
				if (fi_fread(&iWidth, 4, 1, fph) == 1
					&& fi_fread(&iHeight, 4, 1, fph) == 1)
				{
					if (iWidth < 0)
					{
						iWidth = -iWidth;
						bNewFormat = true;
					}

					if (iWidth > 0 && iHeight > 0)
					{
						size_t uiBytesPerVert(4);
						if (bNewFormat)
							uiBytesPerVert = 3;

						if (uiSize == 8 + uiBytesPerVert * iWidth * iHeight)
						{
							// data is valid
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
						}
						else
						{
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/heightmap"), K2_MAP_INVALID_HEIGHTMAP_DATA,
								_TS("Heightmap data is corrupt: size of contents does not match size specified in header"));
							return false;
						}
					}
					else
					{
						fi_fclose(fph);
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/heightmap"), K2_MAP_INVALID_HEIGHTMAP_DATA,
								_TS("Heightmap data is corrupt: invalid size specified in header"));
						return false;
					}
				}
				else
				{
					fi_fclose(fph);
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/heightmap"), K2_MAP_INVALID_HEIGHTMAP_DATA,
						_TS("Heightmap data is corrupt: could not read heightmap header"));
					return false;
				}
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/heightmap"), K2_MAP_INVALID_HEIGHTMAP_DATA,
					_TS("Heightmap data is corrupt: could not read heightmap data (programmer error?)"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/heightmap"), K2_MAP_INVALID_HEIGHTMAP_DATA,
				_TS("Could not open heightmap data (programmer error?)"));
			return false;
		}
	}

	// Validate the vertexcolormap
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("vertexcolormap"), pBuffer));
		if (pBuffer != NULL)
		{
			FILE_INTERFACE* fph(fi_fopen("vertexcolormap", "rb", pBuffer, uiSize));
			if (fph != NULL)
			{
				int iWidth(0);
				int iHeight(0);
				if (fi_fread(&iWidth, 4, 1, fph) == 1
					&& fi_fread(&iHeight, 4, 1, fph) == 1)
				{
					if (iWidth > 0 && iHeight > 0)
					{
						size_t uiBytesPerVert(4);

						if (uiSize == 8 + uiBytesPerVert * iWidth * iHeight)
						{
							// data is valid
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
						}
						else
						{
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/vertexcolormap"), K2_MAP_INVALID_VERTEXCOLORMAP_DATA,
								_TS("vertexcolormap data is corrupt: size of contents does not match size specified in header"));
							return false;
						}
					}
					else
					{
						fi_fclose(fph);
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/vertexcolormap"), K2_MAP_INVALID_VERTEXCOLORMAP_DATA,
								_TS("vertexcolormap data is corrupt: invalid size specified in header"));
						return false;
					}
				}
				else
				{
					fi_fclose(fph);
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/vertexcolormap"), K2_MAP_INVALID_VERTEXCOLORMAP_DATA,
						_TS("vertexcolormap data is corrupt: could not read vertexcolormap header"));
					return false;
				}
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/vertexcolormap"), K2_MAP_INVALID_VERTEXCOLORMAP_DATA,
					_TS("vertexcolormap data is corrupt: could not read vertexcolormap data (programmer error?)"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/vertexcolormap"), K2_MAP_INVALID_VERTEXCOLORMAP_DATA,
				_TS("Could not open vertexcolormap data (programmer error?)"));
			return false;
		}
	}

	// Validate the vertexfoliagemap
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("vertexfoliagemap"), pBuffer));
		if (pBuffer != NULL)
		{
			FILE_INTERFACE* fph(fi_fopen("vertexfoliagemap", "rb", pBuffer, uiSize));
			if (fph != NULL)
			{
				bool bNewFormat(false);
				int iLayers(0);
				int iWidth(0);
				int iHeight(0);

				if (fi_fread(&iLayers, 4, 1, fph) == 1
					&& fi_fread(&iWidth, 4, 1, fph) == 1
					&& fi_fread(&iHeight, 4, 1, fph) == 1)
				{
					if (iWidth < 0)
					{
						iWidth = -iWidth;
						bNewFormat = true;
					}

					if (iLayers == NUM_FOLIAGE_LAYERS)
					{
						if (iWidth > 0 && iHeight > 0)
						{
							size_t uiBytesPerVert(0);
							if (bNewFormat)
							{
								/*
								m_pFoliageVertices[iLayer][iTile].fDensity = (float)(hVertexFoliageMap.ReadInt16())/32.0f;
								m_pFoliageVertices[iLayer][iTile].v3Size.x = (float)(hVertexFoliageMap.ReadByte())/4.0f;
								m_pFoliageVertices[iLayer][iTile].v3Size.y = (float)(hVertexFoliageMap.ReadByte())/4.0f;
								m_pFoliageVertices[iLayer][iTile].v3Size.z = (float)(hVertexFoliageMap.ReadByte())/4.0f;
								m_pFoliageVertices[iLayer][iTile].v3Variance.x = (float)(hVertexFoliageMap.ReadByte())/4.0f;
								m_pFoliageVertices[iLayer][iTile].v3Variance.y = (float)(hVertexFoliageMap.ReadByte())/4.0f;
								m_pFoliageVertices[iLayer][iTile].v3Variance.z = (float)(hVertexFoliageMap.ReadByte())/4.0f;
								m_pFoliageVertices[iLayer][iTile].v3Color.x = (float)(hVertexFoliageMap.ReadByte())/255.0f;
								m_pFoliageVertices[iLayer][iTile].v3Color.y = (float)(hVertexFoliageMap.ReadByte())/255.0f;
								m_pFoliageVertices[iLayer][iTile].v3Color.z = (float)(hVertexFoliageMap.ReadByte())/255.0f;
								*/
								uiBytesPerVert = 2
									+ 1 + 1 + 1 
									+ 1 + 1 + 1
									+ 1 + 1 + 1;
							}
							else
							{
								/*
								m_pFoliageVertices[iLayer][iTile].fDensity = hVertexFoliageMap.ReadFloat();
								m_pFoliageVertices[iLayer][iTile].v3Size.x = hVertexFoliageMap.ReadFloat();
								m_pFoliageVertices[iLayer][iTile].v3Size.y = hVertexFoliageMap.ReadFloat();
								m_pFoliageVertices[iLayer][iTile].v3Size.z = hVertexFoliageMap.ReadFloat();
								m_pFoliageVertices[iLayer][iTile].v3Variance.x = hVertexFoliageMap.ReadFloat();
								m_pFoliageVertices[iLayer][iTile].v3Variance.y = hVertexFoliageMap.ReadFloat();
								m_pFoliageVertices[iLayer][iTile].v3Variance.z = hVertexFoliageMap.ReadFloat();
								m_pFoliageVertices[iLayer][iTile].v3Color.x = hVertexFoliageMap.ReadFloat();
								m_pFoliageVertices[iLayer][iTile].v3Color.y = hVertexFoliageMap.ReadFloat();
								m_pFoliageVertices[iLayer][iTile].v3Color.z = hVertexFoliageMap.ReadFloat();
								*/
								uiBytesPerVert = 4
									+ 4 + 4 + 4 
									+ 4 + 4 + 4
									+ 4 + 4 + 4;
							}

							if (uiSize == 12 + uiBytesPerVert * iWidth * iHeight * iLayers)
							{
								// data is valid
								fi_fclose(fph);
								SAFE_DELETE_ARRAY(pBuffer);
							}
							else
							{
								fi_fclose(fph);
								SAFE_DELETE_ARRAY(pBuffer);
								K2_validation_result(sMap + _T("/vertexfoliagemap"), K2_MAP_INVALID_VERTEXFOLIAGEMAP_DATA,
									_TS("vertexfoliagemap data is corrupt: size of contents does not match size specified in header"));
								return false;
							}
						}
						else
						{
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/vertexfoliagemap"), K2_MAP_INVALID_VERTEXFOLIAGEMAP_DATA,
									_TS("vertexfoliagemap data is corrupt: invalid size specified in header"));
							return false;
						}
					}
					else
					{
						fi_fclose(fph);
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/vertexfoliagemap"), K2_MAP_INVALID_VERTEXFOLIAGEMAP_DATA,
							_TS("vertexfoliagemap data is corrupt: invalid number of foliage layers"));
						return false;
					}
				}
				else
				{
					fi_fclose(fph);
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/vertexfoliagemap"), K2_MAP_INVALID_VERTEXFOLIAGEMAP_DATA,
						_TS("vertexfoliagemap data is corrupt: could not read vertexfoliagemap header"));
					return false;
				}
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/vertexfoliagemap"), K2_MAP_INVALID_VERTEXFOLIAGEMAP_DATA,
					_TS("vertexfoliagemap data is corrupt: could not read vertexfoliagemap data (programmer error?)"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/vertexfoliagemap"), K2_MAP_INVALID_VERTEXFOLIAGEMAP_DATA,
				_TS("Could not open vertexfoliagemap data (programmer error?)"));
			return false;
		}
	}

	// Validate the vertexblockermap
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("vertexblockermap"), pBuffer));
		if (pBuffer != NULL)
		{
			FILE_INTERFACE* fph(fi_fopen("vertexblockermap", "rb", pBuffer, uiSize));
			if (fph != NULL)
			{
				int iWidth(0);
				int iHeight(0);
				if (fi_fread(&iWidth, 4, 1, fph) == 1
					&& fi_fread(&iHeight, 4, 1, fph) == 1)
				{
					if (iWidth > 0 && iHeight > 0)
					{
						size_t uiBytesPerVert(1);

						if (uiSize == 8 + uiBytesPerVert * iWidth * iHeight)
						{
							// data is valid
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
						}
						else
						{
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/vertexblockermap"), K2_MAP_INVALID_VERTEXBLOCKERMAP_DATA,
								_TS("vertexblockermap data is corrupt: size of contents does not match size specified in header"));
							return false;
						}
					}
					else
					{
						fi_fclose(fph);
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/vertexblockermap"), K2_MAP_INVALID_VERTEXBLOCKERMAP_DATA,
								_TS("vertexblockermap data is corrupt: invalid size specified in header"));
						return false;
					}
				}
				else
				{
					fi_fclose(fph);
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/vertexblockermap"), K2_MAP_INVALID_VERTEXBLOCKERMAP_DATA,
						_TS("vertexblockermap data is corrupt: could not read vertexblockermap header"));
					return false;
				}
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/vertexblockermap"), K2_MAP_INVALID_VERTEXBLOCKERMAP_DATA,
					_TS("vertexblockermap data is corrupt: could not read vertexblockermap data (programmer error?)"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/vertexblockermap"), K2_MAP_INVALID_VERTEXBLOCKERMAP_DATA,
				_TS("Could not open vertexblockermap data (programmer error?)"));
			return false;
		}
	}

	// Validate the vertexcliffmap
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("vertexcliffmap"), pBuffer));
		if (pBuffer != NULL)
		{
			FILE_INTERFACE* fph(fi_fopen("vertexcliffmap", "rb", pBuffer, uiSize));
			if (fph != NULL)
			{
				int iWidth(0);
				int iHeight(0);
				if (fi_fread(&iWidth, 4, 1, fph) == 1
					&& fi_fread(&iHeight, 4, 1, fph) == 1)
				{
					if (iWidth == world_CliffGridWidth && iHeight == world_CliffGridHeight)
					{
						size_t uiBytesPerVert(4);

						if (uiSize == 8 + uiBytesPerVert * world_CliffGridWidth * world_CliffGridHeight)
						{
							// data is valid
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
						}
						else
						{
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/vertexcliffmap"), K2_MAP_INVALID_VERTEXCLIFFMAP_DATA,
								_TS("vertexcliffmap data is corrupt: size of contents does not match size specified in header"));
							return false;
						}
					}
					else
					{
						fi_fclose(fph);
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/vertexcliffmap"), K2_MAP_INVALID_VERTEXCLIFFMAP_DATA,
								_TS("vertexcliffmap data is corrupt: invalid size specified in header"));
						return false;
					}
				}
				else
				{
					fi_fclose(fph);
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/vertexcliffmap"), K2_MAP_INVALID_VERTEXCLIFFMAP_DATA,
						_TS("vertexcliffmap data is corrupt: could not read vertexcliffmap header"));
					return false;
				}
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/vertexcliffmap"), K2_MAP_INVALID_VERTEXCLIFFMAP_DATA,
					_TS("vertexcliffmap data is corrupt: could not read vertexcliffmap data (programmer error?)"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/vertexcliffmap"), K2_MAP_INVALID_VERTEXCLIFFMAP_DATA,
				_TS("Could not open vertexcliffmap data (programmer error?)"));
			return false;
		}
	}

	// Validate the variationcliffmap
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("variationcliffmap"), pBuffer));
		if (pBuffer != NULL)
		{
			FILE_INTERFACE* fph(fi_fopen("variationcliffmap", "rb", pBuffer, uiSize));
			if (fph != NULL)
			{
				int iWidth(0);
				int iHeight(0);
				if (fi_fread(&iWidth, 4, 1, fph) == 1
					&& fi_fread(&iHeight, 4, 1, fph) == 1)
				{
					if (iWidth > 0 && iHeight > 0)
					{
						size_t uiBytesPerVert(4 + 4);

						if (uiSize == 8 + uiBytesPerVert * iWidth * iHeight)
						{
							// data is valid
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
						}
						else
						{
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/variationcliffmap"), K2_MAP_INVALID_VARIATIONCLIFFMAP_DATA,
								_TS("variationcliffmap data is corrupt: size of contents does not match size specified in header"));
							return false;
						}
					}
					else
					{
						fi_fclose(fph);
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/variationcliffmap"), K2_MAP_INVALID_VARIATIONCLIFFMAP_DATA,
								_TS("variationcliffmap data is corrupt: invalid size specified in header"));
						return false;
					}
				}
				else
				{
					fi_fclose(fph);
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/variationcliffmap"), K2_MAP_INVALID_VARIATIONCLIFFMAP_DATA,
						_TS("variationcliffmap data is corrupt: could not read variationcliffmap header"));
					return false;
				}
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/variationcliffmap"), K2_MAP_INVALID_VARIATIONCLIFFMAP_DATA,
					_TS("variationcliffmap data is corrupt: could not read variationcliffmap data (programmer error?)"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/variationcliffmap"), K2_MAP_INVALID_VARIATIONCLIFFMAP_DATA,
				_TS("Could not open variationcliffmap data (programmer error?)"));
			return false;
		}
	}

	// Validate the cliffsetlist
	{
		CCompressedFile cCompressedFile;
		uint uiSize(cMapArchive.GetCompressedFile(_T("cliffsetlist"), cCompressedFile));

		if (uiSize != 0 || cCompressedFile.GetRawSize() != 0)
		{
			K2_validation_result(sMap + _T("/cliffsetlist"), K2_MAP_INVALID_CLIFFSETLIST_DATA,
				_TS("cliffsetlist was not expected to contain any data"));
			return false;
		}
	}

	// Validate the tilerampmap
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("tilerampmap"), pBuffer));
		if (pBuffer != NULL)
		{
			FILE_INTERFACE* fph(fi_fopen("tilerampmap", "rb", pBuffer, uiSize));
			if (fph != NULL)
			{
				int iWidth(0);
				int iHeight(0);
				if (fi_fread(&iWidth, 4, 1, fph) == 1
					&& fi_fread(&iHeight, 4, 1, fph) == 1)
				{
					if (iWidth > 0 && iHeight > 0)
					{
						size_t uiBytesPerVert(1);

						if (uiSize == 8 + uiBytesPerVert * iWidth * iHeight)
						{
							// data is valid
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
						}
						else
						{
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/tilerampmap"), K2_MAP_INVALID_TILERAMPMAP_DATA,
								_TS("tilerampmap data is corrupt: size of contents does not match size specified in header"));
							return false;
						}
					}
					else
					{
						fi_fclose(fph);
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/tilerampmap"), K2_MAP_INVALID_TILERAMPMAP_DATA,
								_TS("tilerampmap data is corrupt: invalid size specified in header"));
						return false;
					}
				}
				else
				{
					fi_fclose(fph);
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/tilerampmap"), K2_MAP_INVALID_TILERAMPMAP_DATA,
						_TS("tilerampmap data is corrupt: could not read tilerampmap header"));
					return false;
				}
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/tilerampmap"), K2_MAP_INVALID_TILERAMPMAP_DATA,
					_TS("tilerampmap data is corrupt: could not read tilerampmap data (programmer error?)"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/tilerampmap"), K2_MAP_INVALID_TILERAMPMAP_DATA,
				_TS("Could not open tilerampmap data (programmer error?)"));
			return false;
		}
	}

	// Validate the tilematerialmap
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("tilematerialmap"), pBuffer));
		if (pBuffer != NULL)
		{
			FILE_INTERFACE* fph(fi_fopen("tilematerialmap", "rb", pBuffer, uiSize));
			if (fph != NULL)
			{
				bool bNewFormat(false);
				int iLayers(0);
				int iWidth(0);
				int iHeight(0);

				if (fi_fread(&iLayers, 4, 1, fph) == 1
					&& fi_fread(&iWidth, 4, 1, fph) == 1
					&& fi_fread(&iHeight, 4, 1, fph) == 1)
				{
					if (iWidth < 0)
					{
						iWidth = -iWidth;
						bNewFormat = true;
					}

					if (iLayers == NUM_TERRAIN_LAYERS)
					{
						if (iWidth > 0 && iHeight > 0)
						{
							size_t uiBytesPerVert(0);
							if (bNewFormat)
							{
								/*
								m_pTiles[iLayer][iTile].iMaterialRef = hTileMaterialMap.ReadInt16();
								m_pTiles[iLayer][iTile].iDiffuseRef = hTileMaterialMap.ReadInt16();
								m_pTiles[iLayer][iTile].iNormalmapRef = hTileMaterialMap.ReadInt16();

								// Maintain backwards compatibility
								hTileMaterialMap.ReadFloat(); // fScale
								hTileMaterialMap.ReadInt16(); // fRotation
								hTileMaterialMap.ReadFloat(); // v2Offset.x
								hTileMaterialMap.ReadFloat(); // v2Offset.y
								*/
								uiBytesPerVert = 2 + 2 + 2
									+ 4 + 2 + 4 + 4;
							}
							else
							{
								/*
								m_pTiles[iLayer][iTile].iMaterialRef = hTileMaterialMap.ReadInt32();
								m_pTiles[iLayer][iTile].iDiffuseRef = hTileMaterialMap.ReadInt32();
								m_pTiles[iLayer][iTile].iNormalmapRef = hTileMaterialMap.ReadInt32();

								// Maintain even more backwards compatibility
								hTileMaterialMap.ReadFloat(); // fScale
								hTileMaterialMap.ReadFloat(); // fRotation
								hTileMaterialMap.ReadFloat(); // v2Offset.x
								hTileMaterialMap.ReadFloat(); // v2Offset.y
								hTileMaterialMap.ReadFloat(); // v3Proj.x
								hTileMaterialMap.ReadFloat(); // v3Proj.y
								hTileMaterialMap.ReadFloat(); // v3Proj.z
								*/
								uiBytesPerVert = 4 + 4 + 4
									+ 4 + 4 + 4 + 4
									+ 4 + 4 + 4;
							}

							if (uiSize == 12 + uiBytesPerVert * iWidth * iHeight * iLayers)
							{
								// data is valid
								fi_fclose(fph);
								SAFE_DELETE_ARRAY(pBuffer);
							}
							else
							{
								fi_fclose(fph);
								SAFE_DELETE_ARRAY(pBuffer);
								K2_validation_result(sMap + _T("/tilematerialmap"), K2_MAP_INVALID_TILEMATERIALMAP_DATA,
									_TS("tilematerialmap data is corrupt: size of contents does not match size specified in header"));
								return false;
							}
						}
						else
						{
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/tilematerialmap"), K2_MAP_INVALID_TILEMATERIALMAP_DATA,
									_TS("tilematerialmap data is corrupt: invalid size specified in header"));
							return false;
						}
					}
					else
					{
						fi_fclose(fph);
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/tilematerialmap"), K2_MAP_INVALID_TILEMATERIALMAP_DATA,
							_TS("tilematerialmap data is corrupt: invalid number of terrain layers"));
						return false;
					}
				}
				else
				{
					fi_fclose(fph);
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/tilematerialmap"), K2_MAP_INVALID_TILEMATERIALMAP_DATA,
						_TS("tilematerialmap data is corrupt: could not read tilematerialmap header"));
					return false;
				}
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/tilematerialmap"), K2_MAP_INVALID_TILEMATERIALMAP_DATA,
					_TS("tilematerialmap data is corrupt: could not read tilematerialmap data (programmer error?)"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/tilematerialmap"), K2_MAP_INVALID_TILEMATERIALMAP_DATA,
				_TS("Could not open tilematerialmap data (programmer error?)"));
			return false;
		}
	}

	// Validate the tilefoliagemap
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("tilefoliagemap"), pBuffer));
		if (pBuffer != NULL)
		{
			FILE_INTERFACE* fph(fi_fopen("tilefoliagemap", "rb", pBuffer, uiSize));
			if (fph != NULL)
			{
				int iLayers(0);
				int iWidth(0);
				int iHeight(0);

				if (fi_fread(&iLayers, 4, 1, fph) == 1
					&& fi_fread(&iWidth, 4, 1, fph) == 1
					&& fi_fread(&iHeight, 4, 1, fph) == 1)
				{
					if (iLayers == NUM_TERRAIN_LAYERS)
					{
						if (iWidth > 0 && iHeight > 0)
						{
							if ((uiSize - 12) % (iWidth * iHeight) == 0)
							{
								size_t zSize((uiSize - 12) / (iWidth * iHeight));

								size_t uiBytesPerVert(0);
								if (zSize == 16) // HACK: Old format
								{
									/*
									m_pFoliageTiles[iLayer][iTile].iMaterialRef = hTileFoliageMap.ReadInt32();
									m_pFoliageTiles[iLayer][iTile].iTextureRef = hTileFoliageMap.ReadInt32();
									m_pFoliageTiles[iLayer][iTile].yNumCrossQuads = 2;
									m_pFoliageTiles[iLayer][iTile].yFlags = 0;
									*/
									uiBytesPerVert = 4 + 4;
								}
								else
								{
									/*
									m_pFoliageTiles[iLayer][iTile].iMaterialRef = hTileFoliageMap.ReadInt32();
									m_pFoliageTiles[iLayer][iTile].iTextureRef = hTileFoliageMap.ReadInt32();
									m_pFoliageTiles[iLayer][iTile].yNumCrossQuads = hTileFoliageMap.ReadByte();
									m_pFoliageTiles[iLayer][iTile].yFlags = hTileFoliageMap.ReadByte();
									*/
									uiBytesPerVert = 4 + 4 + 1 + 1;
								}

								if (uiSize == 12 + uiBytesPerVert * iWidth * iHeight * iLayers)
								{
									// data is valid
									fi_fclose(fph);
									SAFE_DELETE_ARRAY(pBuffer);
								}
								else
								{
									fi_fclose(fph);
									SAFE_DELETE_ARRAY(pBuffer);
									K2_validation_result(sMap + _T("/tilefoliagemap"), K2_MAP_INVALID_TILEFOLIAGEMAP_DATA,
										_TS("tilefoliagemap data is corrupt: size of contents does not match size specified in header"));
									return false;
								}
							}
							else
							{
								fi_fclose(fph);
								SAFE_DELETE_ARRAY(pBuffer);
								K2_validation_result(sMap + _T("/tilefoliagemap"), K2_MAP_INVALID_TILEFOLIAGEMAP_DATA,
									_TS("tilefoliagemap data is corrupt: size of contents does not match size specified"));
								return false;
							}
						}
						else
						{
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/tilefoliagemap"), K2_MAP_INVALID_TILEFOLIAGEMAP_DATA,
									_TS("tilefoliagemap data is corrupt: invalid size specified in header"));
							return false;
						}
					}
					else
					{
						fi_fclose(fph);
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/tilefoliagemap"), K2_MAP_INVALID_TILEFOLIAGEMAP_DATA,
							_TS("tilefoliagemap data is corrupt: invalid number of terrain layers"));
						return false;
					}
				}
				else
				{
					fi_fclose(fph);
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/tilefoliagemap"), K2_MAP_INVALID_TILEFOLIAGEMAP_DATA,
						_TS("tilefoliagemap data is corrupt: could not read tilefoliagemap header"));
					return false;
				}
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/tilefoliagemap"), K2_MAP_INVALID_TILEFOLIAGEMAP_DATA,
					_TS("tilefoliagemap data is corrupt: could not read tilefoliagemap data (programmer error?)"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/tilefoliagemap"), K2_MAP_INVALID_TILEFOLIAGEMAP_DATA,
				_TS("Could not open tilefoliagemap data (programmer error?)"));
			return false;
		}
	}

	// Validate the tilecliffmap
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("tilecliffmap"), pBuffer));
		if (pBuffer != NULL)
		{
			FILE_INTERFACE* fph(fi_fopen("tilecliffmap", "rb", pBuffer, uiSize));
			if (fph != NULL)
			{
				int iWidth(0);
				int iHeight(0);
				if (fi_fread(&iWidth, 4, 1, fph) == 1
					&& fi_fread(&iHeight, 4, 1, fph) == 1)
				{
					if (iWidth > 0 && iHeight > 0)
					{
						size_t uiBytesPerVert(1);

						if (uiSize == 8 + uiBytesPerVert * iWidth * iHeight)
						{
							// data is valid
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
						}
						else
						{
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/tilecliffmap"), K2_MAP_INVALID_TILECLIFFMAP_DATA,
								_TS("tilecliffmap data is corrupt: size of contents does not match size specified in header"));
							return false;
						}
					}
					else
					{
						fi_fclose(fph);
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/tilecliffmap"), K2_MAP_INVALID_TILECLIFFMAP_DATA,
								_TS("tilecliffmap data is corrupt: invalid size specified in header"));
						return false;
					}
				}
				else
				{
					fi_fclose(fph);
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/tilecliffmap"), K2_MAP_INVALID_TILECLIFFMAP_DATA,
						_TS("tilecliffmap data is corrupt: could not read tilecliffmap header"));
					return false;
				}
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/tilecliffmap"), K2_MAP_INVALID_TILECLIFFMAP_DATA,
					_TS("tilecliffmap data is corrupt: could not read tilecliffmap data (programmer error?)"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/tilecliffmap"), K2_MAP_INVALID_TILECLIFFMAP_DATA,
				_TS("Could not open tilecliffmap data (programmer error?)"));
			return false;
		}
	}

	// Validate the tilevisblockermap
	{
		char* pBuffer(NULL);
		uint uiSize(cMapArchive.OpenUnzipFile(_T("tilevisblockermap"), pBuffer));
		if (pBuffer != NULL)
		{
			FILE_INTERFACE* fph(fi_fopen("tilevisblockermap", "rb", pBuffer, uiSize));
			if (fph != NULL)
			{
				int iWidth(0);
				int iHeight(0);
				if (fi_fread(&iWidth, 4, 1, fph) == 1
					&& fi_fread(&iHeight, 4, 1, fph) == 1)
				{
					if (iWidth > 0 && iHeight > 0)
					{
						size_t uiBytesPerVert(1);

						if (uiSize == 8 + uiBytesPerVert * iWidth * iHeight)
						{
							// data is valid
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
						}
						else
						{
							fi_fclose(fph);
							SAFE_DELETE_ARRAY(pBuffer);
							K2_validation_result(sMap + _T("/tilevisblockermap"), K2_MAP_INVALID_TILEVISBLOCKERMAP_DATA,
								_TS("tilevisblockermap data is corrupt: size of contents does not match size specified in header"));
							return false;
						}
					}
					else
					{
						fi_fclose(fph);
						SAFE_DELETE_ARRAY(pBuffer);
						K2_validation_result(sMap + _T("/tilevisblockermap"), K2_MAP_INVALID_TILEVISBLOCKERMAP_DATA,
								_TS("tilevisblockermap data is corrupt: invalid size specified in header"));
						return false;
					}
				}
				else
				{
					fi_fclose(fph);
					SAFE_DELETE_ARRAY(pBuffer);
					K2_validation_result(sMap + _T("/tilevisblockermap"), K2_MAP_INVALID_TILEVISBLOCKERMAP_DATA,
						_TS("tilevisblockermap data is corrupt: could not read tilevisblockermap header"));
					return false;
				}
			}
			else
			{
				SAFE_DELETE_ARRAY(pBuffer);
				K2_validation_result(sMap + _T("/tilevisblockermap"), K2_MAP_INVALID_TILEVISBLOCKERMAP_DATA,
					_TS("tilevisblockermap data is corrupt: could not read tilevisblockermap data (programmer error?)"));
				return false;
			}
		}
		else
		{
			K2_validation_result(sMap + _T("/tilevisblockermap"), K2_MAP_INVALID_TILEVISBLOCKERMAP_DATA,
				_TS("Could not open tilevisblockermap data (programmer error?)"));
			return false;
		}
	}

	// Map is valid
	K2_validation_result(sMap, K2_MAP_VALID, _TS("Map is valid"));
	return true;

#undef K2_validation_result
#undef ConsoleFunc
}


/*====================
  K0Main
  ====================*/
int		K0Main(const tstring &sCmdLine)
{
	CCmdLine cCmdLine(sCmdLine);

	tcout << _T("K2Validator ") << _T("v")VERSION << _T(" ") << endl;
	tcout << endl;
	tcout << endl;

	tstring sMap(cCmdLine.GetParameter(_T("map")));

	// Validate the map
	SMapValidationResultInfo sResult;
	bool bResult(K2_ValidateMap(sMap, &sResult));

	tcout << endl;

	if (!bResult)
	{
		// Invalid map
		return -1 - (int)sResult.eCode;
	}

	// Valid map
	return 0;
}


/*====================
  _tmain
  ====================*/
#ifdef _WIN32
int		_tmain(int argc, _TCHAR* argv[])
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
#else
int main(int argc, char *argv[])
{
#endif

	K0System.Init(argc, argv);

	tstring sCmdLine;

#if 0
	//sCmdLine = _T("-map ../test/caldavar.s2z");
	sCmdLine = _T("-map ../test/darkwoodvale.s2z");
#else
	if (argc == 1)
	{
		tcout << _T("K2Validator -map <filepath>") << endl;
		return 0;
	}

	for (int i(1); i < argc; ++i)
	{
		if (i > 1)
			sCmdLine += _T(' ');

		sCmdLine += argv[i];
	}
#endif

	return K0Main(sCmdLine);
}

