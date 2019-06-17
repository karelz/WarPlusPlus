// CacheLoadQueue.h: interface for the CCacheLoadQueue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CACHELOADQUEUE_H__D1D6D204_F4CA_11D3_BFFC_0000B4A08F9A__INCLUDED_)
#define AFX_CACHELOADQUEUE_H__D1D6D204_F4CA_11D3_BFFC_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CachedObjectHandle.h"
#include <afxmt.h>

class CCacheLoadQueue
{
	friend UINT gCacheLoadQueueMessagePump(LPVOID pParam);

	////////////////////////////////////////////////
	// Konstrukce a destrukce
public:
	// Konstruktor
	CCacheLoadQueue();

	// Destruktor
	virtual ~CCacheLoadQueue();

	////////////////////////////////////////////////
	// Prace s frontou
public:
	// Rozjeti cyklu zpracovavani fronty
	void RunQueue();

	// Vlozeni zadosti na load do fronty
	void QueueLoad(CCachedObjectHandle *pObjectHandle);

	// Odstraneni zadosti na load z fronty
	void Dequeue(CCachedObjectHandle *pObjectHandle);

	////////////////////////////////////////////////
	// Pomocne funkce
public:
	// Pocet uspesnych nahrani objektu do cache
	DWORD NumLoads() { return m_dwLoaded; }

	// Pocet vyruseni objektu ze struktur cache
	DWORD NumDequeues() { return m_dwDequeued; }

	////////////////////////////////////////////////
	// Data
private:
	// Hlava fronty
	CCachedObjectHandle *m_pQueueHead;

	// Ocas fronty
	CCachedObjectHandle *m_pQueueTail;

	// Pocet prvku ve fronte
	DWORD m_dwQueueLength;

	// Kriticka sekce kodu - pristupovani ke fronte
	CCriticalSection m_csQueueAccess;

	// Semafor, pocitajici prvky ve fronte
	CSemaphore *m_pNumObjects;

  // The queue thread
  CWinThread *m_pQueueThread;

	// Uz skoncil thread svou cinnost?
	volatile BOOL m_bItIsOver;

	////////////////////////////////////////////////
	// Statistika
private:
	// Kolik objektu fronta uspesne nahrala
	DWORD m_dwLoaded;

	// Kolikrat fronta musela objekt vyparat
	DWORD m_dwDequeued;
};

#endif // !defined(AFX_CACHELOADQUEUE_H__D1D6D204_F4CA_11D3_BFFC_0000B4A08F9A__INCLUDED_)
