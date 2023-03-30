// HardwareInfo.h: interface for the CHardwareInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HARDWAREINFO_H__9107CD9D_99ED_4B85_80CF_717A196B9CF5__INCLUDED_)
#define AFX_HARDWAREINFO_H__9107CD9D_99ED_4B85_80CF_717A196B9CF5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "cpu.h"

class __declspec(dllexport) CHardwareInfo  
{
private:
	CCPU m_cpu;

public:
	void GetCPUSpeed( int &cpuSpeed );
	void GetCPUDescription( char* cpuDesc );
	void GetVideoAdapterDescription( char* vidAdptDesc );
	void GetVideoAdapterTotalMemory( int &vidAdptMem );
	void GetTotalSystemMemory( int &sysMem );
	void GetVideoAdapterPixelShaderModel( int &majorVersion, int &minorVersion );
	void GetVideoAdapterVertexShaderModel( int &majorVersion, int &minorVersion );

	CHardwareInfo();
	virtual ~CHardwareInfo(); 
};	

#endif // !defined(AFX_HARDWAREINFO_H__9107CD9D_99ED_4B85_80CF_717A196B9CF5__INCLUDED_)
