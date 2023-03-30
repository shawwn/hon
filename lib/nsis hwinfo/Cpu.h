/*-----------------------------------------------------------------------------
*
* File        : CPU.H
* Author      : Pavlos Touboulidis
* Description : CPU class to get MHz
*             : 
* Notes       : 
*             : 
*
*----------------------------------------------------------------------------*/
#ifndef	__CPU_H__
#define	__CPU_H__



#include <windows.h>



#pragma warning ( push )
#pragma warning ( disable : 4035 )

#define	CCPU_I32TO64(high, low)	(((__int64)high<<32)+low)
typedef	void	(*CCPU_FUNC)(unsigned int param);



class	CCPU
{
public:
	static	unsigned	int	ReadTimeStampCounterLow()
	{
		_asm
		{
			_emit	0Fh		;RDTSC
			_emit	31h
		}
		//						returns eax
	}
	static	unsigned	int	ReadTimeStampCounterHigh()
	{
		_asm
		{
			_emit	0Fh		;RDTSC
			_emit	31h
			mov	eax, edx ;eax=edx
		}
		//						returns eax
	}
	static	unsigned	__int64	ReadTimeStampCounter()
	{
		_asm
		{
			_emit	0Fh		;RDTSC
			_emit	31h
		}
		//						returns edx:eax
	}
	static	void	ReadTimeStampCounter(unsigned int *uHigh, unsigned int *uLow)
	{
		_asm
		{
			_emit	0Fh		;RDTSC
			_emit	31h

			mov	ebx, uHigh
			mov	dword ptr [ebx], edx
			mov	ebx, uLow
			mov	dword ptr [ebx], eax
		}
	}
	static	__int64	GetCyclesDifference(CCPU_FUNC func, unsigned int param)
	{
		unsigned	int	r_edx1, r_eax1;
		unsigned	int	r_edx2, r_eax2;

		//
		// Calculation
		//
		__asm
		{
			push	param			;push parameter param
			mov	ebx, func	;store func in ebx

			_emit	0Fh			;RDTSC
			_emit	31h

			mov	esi, eax		;esi=eax
			mov	edi, edx		;edi=edx

			call	ebx			;call func

			_emit	0Fh			;RDTSC
			_emit	31h

			pop	ebx

			mov	r_edx2, edx	;r_edx2=edx
			mov	r_eax2, eax	;r_eax2=eax

			mov	r_edx1, edi	;r_edx2=edi
			mov	r_eax1, esi	;r_eax2=esi
		}

		return(CCPU_I32TO64(r_edx2, r_eax2) - CCPU_I32TO64(r_edx1, r_eax1));
	}
private:
	static	void	_Delay(unsigned int ms)
	{
		LARGE_INTEGER	freq, c1, c2;
		__int64			x;

		if (!QueryPerformanceFrequency(&freq))
			return;
		x = freq.QuadPart/1000*ms;

		QueryPerformanceCounter(&c1);
		do
		{
			QueryPerformanceCounter(&c2);
		}while(c2.QuadPart-c1.QuadPart < x);
	}
	static	void	_DelayOverhead(unsigned int ms)
	{
		LARGE_INTEGER	freq, c1, c2;
		__int64			x;

		if (!QueryPerformanceFrequency(&freq))
			return;
		x = freq.QuadPart/1000*ms;

		QueryPerformanceCounter(&c1);
		do
		{
			QueryPerformanceCounter(&c2);
		}while(c2.QuadPart-c1.QuadPart == x);
	}
public:
	//
	// Returns the MHz of the CPU
	//
	// Parameter             Description
	// ------------------    ---------------------------------------------------
	//    uTimes             The number of times to run the test. The function
	//                       runs the test this number of times and returns the
	//                       average. Defaults to 1.
	//    uMsecPerTime       Milliseconds each test will run. Defaults to 50.
	//    nThreadPriority    If different than THREAD_PRIORITY_ERROR_RETURN,
	//                       it will set the current thread's priority to
	//                       this value, and will restore it when the tests
	//                       finish. Defaults to THREAD_PRIORITY_TIME_CRITICAL.
	//    dwPriorityClass    If different than 0, it will set the current
	//                       process's priority class to this value, and will
	//                       restore it when the tests finish.
	//                       Defaults to REALTIME_PRIORITY_CLASS.
	//
	// Notes
	// -------------------------------------------------------------------------
	// 1. The default parameter values should be ok.
	//    However, the result may be wrong if (for example) the cache
	//    is flushing to the hard disk at the time of the test.
	// 2. Requires a Pentium+ class processor (RDTSC)
	// 3. Requires support of high resolution timer. Most (if not all) Windows
	//    machines are ok.
	//
	static	int	GetMHz(unsigned int uTimes=1, unsigned int uMsecPerTime=50, 
							int nThreadPriority=THREAD_PRIORITY_TIME_CRITICAL,
							DWORD dwPriorityClass=REALTIME_PRIORITY_CLASS)
	{
		__int64	total=0, overhead=0;

		if (!uTimes || !uMsecPerTime)
			return(0);

		DWORD		dwOldPC;												// old priority class
		int		nOldTP;												// old thread priority

		// Set Process Priority Class
		if (dwPriorityClass != 0)									// if it's to be set
		{
			HANDLE	hProcess = GetCurrentProcess();			// get process handle
			
			dwOldPC = GetPriorityClass(hProcess);				// get current priority
			if (dwOldPC != 0)											// if valid
				SetPriorityClass(hProcess, dwPriorityClass);	// set it
			else															// else
				dwPriorityClass = 0;									// invalidate
		}
		// Set Thread Priority
		if (nThreadPriority != THREAD_PRIORITY_ERROR_RETURN)	// if it's to be set
		{
			HANDLE	hThread = GetCurrentThread();				// get thread handle
			
			nOldTP = GetThreadPriority(hThread);				// get current priority
			if (nOldTP != THREAD_PRIORITY_ERROR_RETURN)		// if valid
				SetThreadPriority(hThread, nThreadPriority);	// set it
			else															// else
				nThreadPriority = THREAD_PRIORITY_ERROR_RETURN;	// invalidate
		}

		for(unsigned int i=0; i<uTimes; i++)
		{
			total += GetCyclesDifference(CCPU::_Delay, uMsecPerTime);
			overhead += GetCyclesDifference(CCPU::_DelayOverhead, uMsecPerTime);
		}

		// Restore Thread Priority
		if (nThreadPriority != THREAD_PRIORITY_ERROR_RETURN)	// if valid
			SetThreadPriority(GetCurrentThread(), nOldTP);		// set the old one
		// Restore Process Priority Class
		if (dwPriorityClass != 0)										// if valid
			SetPriorityClass(GetCurrentProcess(), dwOldPC);		// set the old one


		total -= overhead;
		total /= uTimes;
		total /= uMsecPerTime;
		total /= 1000;

		return((int)total);
	}
};


#pragma warning ( pop )
#undef	CCPU_I32TO64



#endif	//__CPU_H__
