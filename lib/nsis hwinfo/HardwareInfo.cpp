// HardwareInfo.cpp: implementation of the CHardwareInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "HardwareInfo.h"
#include <ddraw.h>
#include <d3d9.h>
#include "math.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHardwareInfo::CHardwareInfo()
{

}

CHardwareInfo::~CHardwareInfo()
{

}

void CHardwareInfo::GetCPUSpeed( int &cpuSpeed )
{
	cpuSpeed = m_cpu.GetMHz();


	//LARGE_INTEGER ulFreq, ulTicks, ulValue, ulStartCounter, ulEAX_EDX, ulResult;

	// it is number of ticks per seconds
	//QueryPerformanceFrequency(&ulFreq);
	// current value of the performance counter
	
	//QueryPerformanceCounter(&ulTicks);
	// calculate one second interval
	//ulValue.QuadPart = ulTicks.QuadPart + ulFreq.QuadPart;
	// read time stamp counter
	// this asm instruction load the highorder 32 bit of the register into EDX
	// and the lower order 32 bits into EAX
	//_asm 
	//{
	//	rdtsc
	//		mov ulEAX_EDX.LowPart, EAX
	//		mov ulEAX_EDX.HighPart, EDX
	//}
	
	// start no of ticks
	//ulStartCounter.QuadPart = ulEAX_EDX.QuadPart;
	// loop for 1 second
	//do 
	//{
	//	QueryPerformanceCounter(&ulTicks);
//	} //while (ulTicks.QuadPart <= ulValue.QuadPart);
//	
	// get the actual no of ticks
//	_asm 
//	{
//		rdtsc
//			mov ulEAX_EDX.LowPart, EAX 
//			mov ulEAX_EDX.HighPart, EDX  
//	}
	
	// calculate result
//	ulResult.QuadPart = ulEAX_EDX.QuadPart - ulStartCounter.QuadPart;
	//cpuSpeed = abs( (int)ulResult.QuadPart / 1000000 );
}

void CHardwareInfo::GetCPUDescription( char* cpuDesc )
{
	char szProcName[MAX_PATH];
	DWORD dwProcNameBuffer = MAX_PATH - 1;
	DWORD dwType = REG_SZ;
	HKEY hkResult = NULL;
	
	if( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", NULL, KEY_READ, &hkResult ) )
		if( ERROR_SUCCESS == RegQueryValueEx( hkResult, "ProcessorNameString", NULL, &dwType, (LPBYTE)szProcName, &dwProcNameBuffer ) )
		{
			strcpy(cpuDesc, szProcName);
		}	
		else if( ERROR_SUCCESS == RegQueryValueEx( hkResult, "Identifier", NULL, &dwType, (LPBYTE)szProcName, &dwProcNameBuffer ) )
		{
			strcpy(cpuDesc, szProcName);
		}
		else
			RegCloseKey( hkResult );
	else
		RegCloseKey( HKEY_LOCAL_MACHINE );
	
	// Skip any whitespace at the beginning of the string
	if (*cpuDesc == ' ')
		*cpuDesc++;
}

void CHardwareInfo::GetVideoAdapterDescription( char* vidAdptDesc )
{
	LPDIRECTDRAW lpDD; 
	LPDIRECTDRAW7 lpDD7;   
	DDDEVICEIDENTIFIER2 ddDeviceIdentifier2;

	ZeroMemory( &ddDeviceIdentifier2, sizeof( DDDEVICEIDENTIFIER2 ) ); 


	DirectDrawCreateEx( NULL, (LPVOID*)&lpDD, IID_IDirectDraw7, NULL );
	if( lpDD )
	{
		lpDD->QueryInterface( IID_IDirectDraw7, (LPVOID*)&lpDD7 );
		lpDD7->GetDeviceIdentifier( &ddDeviceIdentifier2, NULL );
		strcpy(vidAdptDesc, ddDeviceIdentifier2.szDescription);
	}
}

void CHardwareInfo::GetVideoAdapterTotalMemory( int &vidAdptMem )
{
	LPDIRECTDRAW lpDD; 
	DDCAPS ddCaps;

	ZeroMemory( &ddCaps, sizeof( DDCAPS ) );
	ddCaps.dwSize = sizeof(ddCaps);

	DirectDrawCreateEx( NULL, (LPVOID*)&lpDD, IID_IDirectDraw7, NULL );
	if( lpDD )
	{
		lpDD->GetCaps( &ddCaps, NULL );
		int horzRes = GetSystemMetrics( SM_CXSCREEN );
		int vertRes = GetSystemMetrics( SM_CYSCREEN );
		HDC hDC = GetDC( GetDesktopWindow() );
		int bpp = GetDeviceCaps( hDC, BITSPIXEL );
		vidAdptMem = (int)( ( ddCaps.dwVidMemTotal + ( ( horzRes * vertRes * bpp ) / 8 ) ) / 1048576.f ); 

		if( vidAdptMem + 32 >= 768 )
			vidAdptMem = 768;
		else if( vidAdptMem + 32 >= 640 )
			vidAdptMem = 640;
		else if( vidAdptMem + 32 >= 512 )
			vidAdptMem = 512;
		else if( vidAdptMem + 32 >= 384 )
			vidAdptMem = 384;
		else if( vidAdptMem + 32 >= 256 )
			vidAdptMem = 256;
		else if( vidAdptMem + 32 >= 128 )
			vidAdptMem = 128;
		else if( vidAdptMem + 16 >= 64 )
			vidAdptMem = 64;
		else if( vidAdptMem + 16 >= 32 )
			vidAdptMem = 32;
		else if( vidAdptMem + 8 >= 16 )
			vidAdptMem = 16;
		else
			vidAdptMem = 0;

	}
}

void CHardwareInfo::GetTotalSystemMemory( int &sysMem )
{
	MEMORYSTATUS memorystatus;
	GlobalMemoryStatus( &memorystatus );
	sysMem = (int)( ceil( memorystatus.dwTotalPhys / 1048576.f ) ); 
}

void CHardwareInfo::GetVideoAdapterPixelShaderModel( int &majorVersion, int &minorVersion )
{
	IDirect3D9 *pd3d(Direct3DCreate9(D3D_SDK_VERSION));
	if (!pd3d)
		return;

	D3DCAPS9 d3dCaps;
	ZeroMemory( &d3dCaps, sizeof( D3DCAPS9 ) );
	pd3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps);
	majorVersion = (d3dCaps.PixelShaderVersion >> 8) & 0xff;
	minorVersion = d3dCaps.PixelShaderVersion & 0xff;
}

void CHardwareInfo::GetVideoAdapterVertexShaderModel( int &majorVersion, int &minorVersion )
{
	IDirect3D9 *pd3d(Direct3DCreate9(D3D_SDK_VERSION));
	if (!pd3d)
		return;

	D3DCAPS9 d3dCaps;
	ZeroMemory( &d3dCaps, sizeof( D3DCAPS9 ) );
	pd3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps);
	majorVersion = (d3dCaps.VertexShaderVersion >> 8) & 0xff;
	minorVersion = d3dCaps.VertexShaderVersion & 0xff;
}
