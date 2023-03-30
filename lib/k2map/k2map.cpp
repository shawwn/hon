// k2map.cpp
//
// K0 Template
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2map_common.h"

#include <k0_string.h>
#include <k0_math.h>
#include <c_cmdline.h>
#include <c_filehandle.h>
#include <c_xmldoc.h>
#include <c_xmlnode.h>

#include <iostream>

//#include "localfile.h"

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

/*====================
  RotateEntityList
  ====================*/
void	RotateEntityList(const tstring &sFilename)
{
	CFileHandle hFile(sFilename, FILE_READ | FILE_BINARY);

	CXMLDoc cXml;

	uint uiLength(0);
	const char *pBuffer(hFile.GetBuffer(uiLength));

	if (!cXml.ReadBuffer(pBuffer, uiLength))
		return;

	CXMLNodeWrite cRoot;

	cXml.TranslateNodes(cRoot);

	XMLNodeWriteList &cEntities(cRoot.GetChildren());

	for (XMLNodeWriteList::iterator it(cEntities.begin()), itEnd(cEntities.end()); it != itEnd; ++it)
	{
		CXMLNodeWrite &cNode(*it);

		if (cNode.HasProperty(_T("angles")))
		{
			CVec3f v3Angles(AtoV3(cNode.GetProperty(_T("angles"))));

			v3Angles[YAW] += 90.0f;

			if (v3Angles[YAW] >= 360.0f)
				v3Angles[YAW] -= 360.0f;

			cNode.SetProperty(_T("angles"), XtoA(v3Angles));
		}

		if (cNode.HasProperty(_T("position")))
		{
			CVec3f v3Position(AtoV3(cNode.GetProperty(_T("position"))));

			v3Position = M_RotatePointAroundLine(v3Position, CVec3f(8192.0f, 8192.0f, 0.0f), CVec3f(8192.0f, 8192.0f, 1.0f), 90.0f);

			cNode.SetProperty(_T("position"), XtoA(v3Position));
		}
	}

	cXml.WriteNodes(sFilename + _T("2"), cRoot, false);
}


/*====================
  RotateHeightMap
  ====================*/
void	RotateHeightMap(const tstring &sFilename)
{
	CFileHandle hInHeightMap(sFilename, FILE_READ | FILE_BINARY);

	int iWidth(hInHeightMap.ReadInt32());
	int iHeight(hInHeightMap.ReadInt32());

	bool bNewFileFormat(false);
	if (iWidth < 0)
	{
		iWidth = -iWidth;
		bNewFileFormat = true;
	}

	int iGridArea(iWidth * iHeight);
	float *pHeightMap(new float[iWidth * iHeight]);

	for (int iTile(0); iTile < iGridArea; ++iTile)
		pHeightMap[iTile] = float(hInHeightMap.ReadByte()) / 256.0f;
	for (int iTile(0); iTile < iGridArea; ++iTile)
		pHeightMap[iTile] += float(hInHeightMap.ReadByte());
	for (int iTile(0); iTile < iGridArea; ++iTile)
	{
		pHeightMap[iTile] += float(hInHeightMap.ReadByte()) * 256.0f;
		pHeightMap[iTile] -= 32768.0f;
	}

	float *pRotatedHeightMap(new float[iWidth * iHeight]);
	for (int iTileY(0); iTileY < iHeight; ++iTileY)
	{
		for (int iTileX(0); iTileX < iWidth; ++iTileX)
		{
			int iRotateTileX(iTileY);
			int iRotateTileY(iWidth - iTileX - 1);

			pRotatedHeightMap[iTileY * iWidth + iTileX] = pHeightMap[iRotateTileY * iWidth + iRotateTileX];
		}
	}

	CFileHandle hOutHeightMap(sFilename + _T("2"), FILE_WRITE | FILE_BINARY);

	hOutHeightMap.WriteInt32(-iWidth);
	hOutHeightMap.WriteInt32(iHeight);

	for (int iTile(0); iTile < iGridArea; ++iTile)
		hOutHeightMap.WriteByte(((byte)((int)(((pRotatedHeightMap[iTile] + 32768.0f) * 256.0f) + 0.5f) % 256)));
	for (int iTile(0); iTile < iGridArea; ++iTile)
		hOutHeightMap.WriteByte(((byte)((int)(pRotatedHeightMap[iTile] + 32768.0f) % 256)));
	for (int iTile(0); iTile < iGridArea; ++iTile)
		hOutHeightMap.WriteByte(((byte)((int)((pRotatedHeightMap[iTile] + 32768.0f) / 256.0f) % 256)));

	delete[] pHeightMap;
	delete[] pRotatedHeightMap;
}


/*====================
  RotateCliffMap
  ====================*/
void	RotateCliffMap(const tstring &sFilename)
{
	CFileHandle hInCliffMap(sFilename, FILE_READ | FILE_BINARY);

	int iWidth(hInCliffMap.ReadInt32());
	int iHeight(hInCliffMap.ReadInt32());

	int iGridArea(iWidth * iHeight);
	byte *pCliffMap(new byte[iWidth * iHeight]);

	for (int iTile(0); iTile < iGridArea; ++iTile)
		pCliffMap[iTile] = hInCliffMap.ReadByte();
	
	byte *pRotatedCliffMap(new byte[iWidth * iHeight]);
	for (int iTileY(0); iTileY < iHeight; ++iTileY)
	{
		for (int iTileX(0); iTileX < iWidth; ++iTileX)
		{
			int iRotateTileX(iTileY);
			int iRotateTileY(iWidth - iTileX - 1);

			pRotatedCliffMap[iTileY * iWidth + iTileX] = pCliffMap[iRotateTileY * iWidth + iRotateTileX];
		}
	}

	CFileHandle hOutCliffMap(sFilename + _T("2"), FILE_WRITE | FILE_BINARY);

	hOutCliffMap.WriteInt32(iWidth);
	hOutCliffMap.WriteInt32(iHeight);

	for (int iTile(0); iTile < iGridArea; ++iTile)
		hOutCliffMap.WriteByte(pRotatedCliffMap[iTile]);

	delete[] pCliffMap;
	delete[] pRotatedCliffMap;
}


/*====================
  RotateComponent
  ====================*/
void	RotateComponent(const tstring &sFilename)
{
	CFileHandle hIn(sFilename, FILE_READ | FILE_BINARY);

	int iWidth(hIn.ReadInt32());
	int iHeight(hIn.ReadInt32());

	bool bNewFileFormat(false);
	if (iWidth < 0)
	{
		iWidth = -iWidth;
		bNewFileFormat = true;
	}

	int iSizeRemaining(hIn.GetLength() - 8);
	int iSize(iSizeRemaining / (iWidth * iHeight));

	byte *pTileMap(new byte[iWidth * iHeight * iSize]);

	for (int i(0); i < iWidth * iHeight * iSize; ++i)
		pTileMap[i] = hIn.ReadByte();
	
	byte *pRotatedTileMap(new byte[iWidth * iHeight * iSize]);
	for (int iTileY(0); iTileY < iHeight; ++iTileY)
	{
		for (int iTileX(0); iTileX < iWidth; ++iTileX)
		{
			int iRotateTileX(iTileY);
			int iRotateTileY(iWidth - iTileX - 1);

			int i0((iTileY * iWidth + iTileX) * iSize);
			int i1((iRotateTileY * iWidth + iRotateTileX) * iSize);

			for (int iByte(0); iByte < iSize; ++iByte)
			{
				pRotatedTileMap[i0 + iByte] = pTileMap[i1 + iByte];
			}
		}
	}

	CFileHandle hOut(sFilename + _T("2"), FILE_WRITE | FILE_BINARY);

	hOut.WriteInt32(bNewFileFormat ? -iWidth : iWidth);
	hOut.WriteInt32(iHeight);

	for (int i(0); i < iWidth * iHeight * iSize; ++i)
		hOut.WriteByte(pRotatedTileMap[i]);

	delete[] pTileMap;
	delete[] pRotatedTileMap;
}


/*====================
  RotateLayeredComponent
  ====================*/
void	RotateLayeredComponent(const tstring &sFilename)
{
	CFileHandle hIn(sFilename, FILE_READ | FILE_BINARY);

	int iLayers(hIn.ReadInt32());
	int iWidth(hIn.ReadInt32());
	int iHeight(hIn.ReadInt32());

	bool bNewFileFormat(false);
	if (iWidth < 0)
	{
		iWidth = -iWidth;
		bNewFileFormat = true;
	}

	int iSizeRemaining(hIn.GetLength() - 12);
	int iSize(iSizeRemaining / (iWidth * iHeight * iLayers));

	byte **pTileMap = new byte *[iLayers];
	byte **pRotatedTileMap = new byte *[iLayers];

	for (int iLayer(0); iLayer < iLayers; ++iLayer)
	{
		pTileMap[iLayer] = new byte[iWidth * iHeight * iSize];

		for (int i(0); i < iWidth * iHeight * iSize; ++i)
			pTileMap[iLayer][i] = hIn.ReadByte();
		
		pRotatedTileMap[iLayer] = new byte[iWidth * iHeight * iSize];
		for (int iTileY(0); iTileY < iHeight; ++iTileY)
		{
			for (int iTileX(0); iTileX < iWidth; ++iTileX)
			{
				int iRotateTileX(iTileY);
				int iRotateTileY(iWidth - iTileX - 1);

				int i0((iTileY * iWidth + iTileX) * iSize);
				int i1((iRotateTileY * iWidth + iRotateTileX) * iSize);

				for (int iByte(0); iByte < iSize; ++iByte)
				{
					pRotatedTileMap[iLayer][i0 + iByte] = pTileMap[iLayer][i1 + iByte];
				}
			}
		}
	}

	CFileHandle hOut(sFilename + _T("2"), FILE_WRITE | FILE_BINARY);

	hOut.WriteInt32(iLayers);
	hOut.WriteInt32(bNewFileFormat ? -iWidth : iWidth);
	hOut.WriteInt32(iHeight);

	for (int iLayer(0); iLayer < iLayers; ++iLayer)
	{
		for (int i(0); i < iWidth * iHeight * iSize; ++i)
			hOut.WriteByte(pRotatedTileMap[iLayer][i]);
	}

	delete[] pTileMap;
	delete[] pRotatedTileMap;
}


/*====================
  K0Main
  ====================*/
int		K0Main(const tstring &sCmdLine)
{
	CCmdLine cCmdLine(sCmdLine);

	tcout << _T("K2Map ") << _T("v")VERSION << _T(" ") << endl;

	tstring sMap(cCmdLine.GetParameter(_T("map")));

	RotateEntityList(sMap + _T("/entitylist"));
	RotateHeightMap(sMap + _T("/heightmap"));
	RotateComponent(sMap + _T("/tilecliffmap"));
	RotateComponent(sMap + _T("/tilevisblockermap"));
	RotateComponent(sMap + _T("/vertexcolormap"));
	RotateComponent(sMap + _T("/vertexblockermap"));
	RotateLayeredComponent(sMap + _T("/tilematerialmap"));
	RotateLayeredComponent(sMap + _T("/tilefoliagemap"));
	RotateLayeredComponent(sMap + _T("/vertexfoliagemap"));

	return 0;
}


/*====================
  _tmain
  ====================*/
int		_tmain(int argc, _TCHAR* argv[])
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	if (argc == 1)
	{
		tcout << _T("K2Map -map <filepath>") << endl;
		return 0;
	}

	tstring sCmdLine;
	for (int i(1); i < argc; ++i)
	{
		if (i > 1)
			sCmdLine += _T(' ');

		sCmdLine += argv[i];
	}

	return K0Main(sCmdLine);
}
