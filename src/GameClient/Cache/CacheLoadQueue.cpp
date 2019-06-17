// CacheLoadQueue.cpp: implementation of the CCacheLoadQueue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <iostream.h>
#include "CacheLoadQueue.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCacheLoadQueue::CCacheLoadQueue()
{
	m_pQueueHead=NULL;
	m_pQueueTail=NULL;
	m_dwQueueLength=0;
	m_bItIsOver=TRUE;

	m_dwLoaded=0;
	m_dwDequeued=0;

  m_pNumObjects=new CSemaphore(0, 10240000);

  m_pQueueThread = NULL;
}

CCacheLoadQueue::~CCacheLoadQueue()
{
	// Zamkneme frontu
	VERIFY(m_csQueueAccess.Lock());

	// Vsechny handly z ni vypareme
	CCachedObjectHandle *pHandle=m_pQueueHead;

	TRACE("Delka fronty je %d\n", m_dwQueueLength);
	while(pHandle) {
		CCachedObjectHandle *pNextHandle=pHandle->NextLoad();
		pHandle->NextLoad(NULL);
		pHandle=pNextHandle;
		m_dwQueueLength--;
		TRACE("Delka fronty --\n");
	}

	m_pQueueHead=NULL;
	m_pQueueTail=NULL;
	// Ted by delka fronty mela byt nulova
	TRACE("Delka fronty je %d\n", m_dwQueueLength);

	ASSERT(m_dwQueueLength==0);

	// Rekneme nasemu threadu, jako by ve fronte byl novy zaznam
  VERIFY(m_pNumObjects->Unlock());

	// Odemkneme frontu
	VERIFY(m_csQueueAccess.Unlock());

  if(m_pQueueThread != NULL){
    if(WaitForSingleObject(m_pQueueThread->m_hThread, INFINITE)){
      TerminateThread(m_pQueueThread->m_hThread, -1);
    }
    delete m_pQueueThread;
    m_pQueueThread = NULL;
  }

  ASSERT(m_pNumObjects!=NULL);
  delete m_pNumObjects;
  m_pNumObjects = NULL;
}

void CCacheLoadQueue::QueueLoad(CCachedObjectHandle *pObjectHandle) 
{
	// Nesmi jiz byt ve fronte
	if(pObjectHandle->NextLoad()!=NULL) return;
	if(m_pQueueTail==pObjectHandle) return;

	VERIFY(m_csQueueAccess.Lock());
	m_dwQueueLength++;

	// Jestlize je fronta prazdna
	if(!m_pQueueTail) {	
	
		m_pQueueHead=m_pQueueTail=pObjectHandle;
		VERIFY(m_csQueueAccess.Unlock());
	} else {
		m_pQueueTail->NextLoad(pObjectHandle);
		m_pQueueTail=pObjectHandle;
		VERIFY(m_csQueueAccess.Unlock());
	}
  VERIFY(m_pNumObjects->Unlock());
}

void CCacheLoadQueue::RunQueue()
{
	m_bItIsOver=FALSE;
	m_pQueueThread = AfxBeginThread(gCacheLoadQueueMessagePump, (LPVOID)this, THREAD_PRIORITY_NORMAL,
    0, CREATE_SUSPENDED, NULL);
  m_pQueueThread->m_bAutoDelete = FALSE;
  m_pQueueThread->ResumeThread();
}

void CCacheLoadQueue::Dequeue(CCachedObjectHandle *pObjectHandle) {		
	// Zamkneme si frontu
	m_csQueueAccess.Lock();

	if(pObjectHandle->NextLoad()==NULL && pObjectHandle!=m_pQueueHead)
	{
		// Objekt ve fronte neni
		m_csQueueAccess.Unlock();
		return;
	}

	// Projdeme celou frontu
	CCachedObjectHandle *pHandle, *pPrevHandle;
	pHandle=m_pQueueHead;	
	pPrevHandle=NULL;

	while(pHandle && (pHandle!=pObjectHandle)) {
		pPrevHandle=pHandle;
		pHandle=pHandle->NextLoad();
	}

	if(pHandle && pHandle==pObjectHandle) {
		// Nasli jsme zaznam pro smazani
		if(pPrevHandle==NULL) {
			// Mazeme shodou okolnosti hlavicku
			m_pQueueHead=pHandle->NextLoad();
			if(pHandle==m_pQueueTail) {
				// A zaroven ocasek
				ASSERT(pHandle->NextLoad()==NULL);
				m_pQueueTail=NULL;
			}
		} else {
			// Premostime mazany handle
			pPrevHandle->NextLoad(pHandle->NextLoad());

			// A pokud to byl ocas, zrusime ho
			if(pHandle==m_pQueueTail) {
				ASSERT(pHandle->NextLoad()==NULL);
				m_pQueueTail=pPrevHandle;
			}			
		}
		// Vyrusime odkaz do fronty
		pHandle->NextLoad(NULL);
		// A snizime delku fronty
		m_dwQueueLength--;
		m_dwDequeued++;
	}

	// A nyni ji muzeme opet odemknout
	m_csQueueAccess.Unlock();
}

// Metoda na zpracovavani veci v message queue
static UINT gCacheLoadQueueMessagePump(LPVOID pParam) {
	// Ukazatel na nasi cache
	CCacheLoadQueue *pQueue=(CCacheLoadQueue *)pParam;
	
	for(;;) { /* Na veky vekuv */

    // Pockam az tam neco je
    VERIFY(pQueue->m_pNumObjects->Lock());

    // Zamkneme frontu
    VERIFY(pQueue->m_csQueueAccess.Lock());

		// Zapamatujeme si prvni polozku fronty
		CCachedObjectHandle *pHandle=pQueue->m_pQueueHead;
		
		// Nahrajeme objekt
		if(pHandle) {
			// Posuneme se na dalsi objekt
			pQueue->m_pQueueHead=pHandle->NextLoad();
			// Vynulujeme naloadenemu objektu ukazatel na dalsi load
			pHandle->NextLoad(NULL);
			
			if(!pQueue->m_pQueueHead) {
				// Fronta je prazdna, vynulujeme i ocas
				pQueue->m_pQueueTail=NULL;
			}
			pQueue->m_dwQueueLength--;

      pHandle->LockWrite();

      // Odemkneme frontu
			pQueue->m_csQueueAccess.Unlock();

			// Je tam, tak ho nacteme
			pHandle->LoadAfterBeingWriteLocked();
			pQueue->m_dwLoaded++;

		}
    else{
      // Zamkneme frontu
      VERIFY(pQueue->m_csQueueAccess.Unlock());

      break;
		}

	}

	pQueue->m_bItIsOver=TRUE;

	return 0;
}