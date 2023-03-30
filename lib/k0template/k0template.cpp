// k0template.cpp
//
// K0 Template
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k0template_common.h"

#include <k0_iostream.h>
#include <k0_string.h>
#include <k0_math.h>
#include <c_cmdline.h>

#include <intrin.h>

//#include "localfile.h"
//=============================================================================

/*====================
  CAS
  16 bits
  ====================*/
inline
int16	CAS(volatile int16 *pDest, int16 iNew, int16 iOld)
{
	return _InterlockedCompareExchange16(pDest, iNew, iOld);
}


/*====================
  CAS
  32 bits
  ====================*/
inline
int32	CAS(volatile int32 *pDest, int32 iNew, int32 iOld)
{
	return _InterlockedCompareExchange(pDest, iNew, iOld);
}


/*====================
  CAS
  64 bits
  ====================*/
inline
int64	CAS(volatile int64 *pDest, int64 iNew, int64 iOld)
{
	return _InterlockedCompareExchange64(pDest, iNew, iOld);
}


/*====================
  XCHG
  32 bits
  ====================*/
inline
int32	XCHG(volatile int32 *pDest, int32 iNew)
{
	return _InterlockedExchange(pDest, iNew);
}


/*====================
  MT_AssignIf
  64 bits
  The success of the assignment is returned in eax
  ====================*/
inline
bool	MT_AssignIf(volatile int64 *pDest, int64 iNew, int64 iOld)
{
	__asm
	{
		lea		esi,iOld;
		lea		edi,iNew;

		mov		eax,[esi];
		mov		edx,4[esi];
		mov		ebx,[edi];
		mov		ecx,4[edi];
		mov		esi,pDest;
		lock cmpxchg8b	[esi];
		mov		eax,0;
		setz	al;
	}
}


// if defined, will not do any locking on shared data
//#define SKIP_LOCKING

typedef int32 LOCK;
const LOCK LOCK_IS_FREE(0);
const LOCK LOCK_IS_TAKEN(1);

//=============================================================================
// CSpinlock
//=============================================================================
class CSpinlock
{
private:
	volatile LOCK m_State;

public:
	CSpinlock() :
	m_State(LOCK_IS_FREE)
	{
	}

	void Lock() 
	{
#if !defined(SKIP_LOCKING)
		// If m_State == LOCK_IS_FREE, it is set to LOCK_IS_TAKEN
		// atomically, so only 1 caller gets the lock.
		// If m_State == LOCK_IS_TAKEN,
		// the result is LOCK_IS_TAKEN, and the while loop keeps spinning.

		while (CAS(&m_State, LOCK_IS_TAKEN, LOCK_IS_FREE) == LOCK_IS_TAKEN)
		{
			// spin!
		}

		// At this point, the lock is acquired.
#endif
	}

	void Unlock()
	{
#if !defined(SKIP_LOCKING)
		XCHG(&m_State, LOCK_IS_FREE);
#endif
	}
};
//=============================================================================

//=============================================================================
// CLFList
// Lock-free List
//=============================================================================
template<class T>
class CLFList
{
private:
	struct SNode
	{
		SNode	*pNext;
		T		Value;
	};

	union UHead
	{
		struct SValue
		{
			SNode	*pNext;
			int16	iSequence0;
			int16	iSequence1;
		} u;
		int64 iValue64;
	};

	volatile UHead	m_uHead;

public:
	CLFList() :
	{
		m_uHead.iValue64 = 0;
	}

	void PushFront(SNode *pNode)
	{
		UHead uOldHead, uNewHead;

		uNewHead.Value.pNext = pNode;

		while (true)
		{
			uOldHead = m_uHead;
			uNewHead.u.iSequence0 = uOldHead.u.iSequence0 + 1;
			uNewHead.u.iSequence1 = uOldHead.u.iSequence1 + 1;

			pNode->pNext = uOldHead.u.pNext;

			if (MT_AssignIf(&m_uHead.iValue64, uNewHead.iValue64, uOldHead.iValue64))
				break;
		}
	}
};
//=============================================================================
 
//=============================================================================
// CSharedTask
//=============================================================================
class CSharedTask
{
private:
	CSpinlock	m_Lock;
	queue<int>	m_Queue;
	int			m_iTicketNumber;
	int			m_iSum;

public:
	CSharedTask() :
	m_iTicketNumber(0)
	{
	}

	void Produce()
	{
		m_Lock.Lock();

		if (m_Queue.size() > 0xffff)
		{
			m_Lock.Unlock();
			return;
		}

		//tcout << ">" << m_iTicketNumber << endl;
		m_Queue.push(m_iTicketNumber++);

		m_Lock.Unlock();
	}

	void Consume()
	{
		m_Lock.Lock();

		if (!m_Queue.empty())
		{
			int x(m_Queue.front());
			m_Queue.pop();

			m_iSum += x;
		}

		m_Lock.Unlock();
	}

	void Lock()
	{
		m_Lock.Lock();
	}

	int		GetTicketNumber() const		{ return m_iTicketNumber; }
} g_SharedTask;
//=============================================================================

/*====================
  ProducerThread
  ====================*/
DWORD WINAPI
ProducerThread(LPVOID unused)
{
	UNREFERENCED_PARAMETER(unused);

	while (true)
	{
		g_SharedTask.Produce();

		//Sleep(rand() % 20);
	}

	return 0;
}


/*====================
  ConsumerThread
  ====================*/
DWORD WINAPI
ConsumerThread(LPVOID unused)
{
	UNREFERENCED_PARAMETER(unused);

	while (true)
	{
		g_SharedTask.Consume();

		//Sleep(rand() % 20);
	}

	return 0;
}


/*====================
  K0Main
  ====================*/
int		K0Main(const tstring &sCmdLine)
{
	CCmdLine cCmdLine(sCmdLine);

	tcout << _T("K0 Template ") << _T("v")VERSION << _T(" ") << endl;

	const int timeoutTime = 500;
	int unused1, unused2;
	HANDLE threads[4];

	// The program creates 4 threads:
	// two producer threads adding to the queue
	// and two consumers taking data out and printing it.
	threads[0] = CreateThread(NULL, 0, ProducerThread, &unused1, 0, (LPDWORD)&unused2);
	threads[1] = CreateThread(NULL, 0, ConsumerThread, &unused1, 0, (LPDWORD)&unused2);
	threads[2] = CreateThread(NULL, 0, ProducerThread, &unused1, 0, (LPDWORD)&unused2);
	threads[3] = CreateThread(NULL, 0, ConsumerThread, &unused1, 0, (LPDWORD)&unused2);

	WaitForMultipleObjects(4, threads, TRUE, timeoutTime);

	g_SharedTask.Lock();

	tcout << _T("<") << g_SharedTask.GetTicketNumber() << endl;

	return 0;
}


/*====================
  _tmain
  ====================*/
int		_tmain(int argc, _TCHAR* argv[])
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	tstring sCmdLine;

	for (int i(0); i < argc; ++i)
	{
		if (i > 0)
			sCmdLine += _T(' ');

		sCmdLine += argv[i];
	}

	return K0Main(sCmdLine);
}
