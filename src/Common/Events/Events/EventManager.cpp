// EventManager.cpp: implementation of the CEventManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EventManager.h"
#include "Notifier.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CMap<DWORD, DWORD &, CEventManager *, CEventManager *&> CEventManager::m_ThreadManagersMap;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEventManager, CObject);

CEventManager::CEventManager()
 :m_semaphoreInlayedEvent(0, EventMaxCount),
  m_EventsPool(50)
{
  m_pPrevManager = NULL;
  m_pFirstEvent = NULL;
  m_pLastEvent = NULL;
}

CEventManager::~CEventManager()
{
  ASSERT(m_pPrevManager == NULL);
  ASSERT(m_pFirstEvent == NULL);
  ASSERT(m_pLastEvent == NULL);
}

/////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CEventManager::AssertValid() const
{
  CPersistentObject::AssertValid();
}

void CEventManager::Dump(CDumpContext &dc) const
{
  CPersistentObject::Dump(dc);
}

#endif


DWORD CEventManager::m_dwQuitEvent = 0xFFFFFFFF;

BOOL CEventManager::Create(DWORD dwThreadID)
{
  if(dwThreadID == 0){
    dwThreadID = GetCurrentThreadId();
  }

  // remember the thread ID
  m_dwPrimaryThreadID = dwThreadID;

  CEventManager *pEventManager;
  // try to find the map for given thread
  if(!m_ThreadManagersMap.Lookup(dwThreadID, pEventManager)){
    // if does not exists -> create one
    pEventManager = this;
    m_ThreadManagersMap.SetAt(dwThreadID, pEventManager);
    // no previous manager
    m_pPrevManager = NULL;
  }
  else{
    // if there is some -> replace hte old one with us
    // but remember the old one
    m_pPrevManager = pEventManager;

    // insert us
    pEventManager = this;
    m_ThreadManagersMap.SetAt(dwThreadID, pEventManager);
  }
  
  return TRUE;
}

void CEventManager::Delete()
{
  CEventManager *pEventManager;
  DWORD dwThreadID;
  POSITION pos;

  // if there was some previous manager -> return it to its pos
  if(m_pPrevManager != NULL){
    m_ThreadManagersMap.SetAt(m_dwPrimaryThreadID, m_pPrevManager);
    m_pPrevManager = NULL;
  }

  // remove us from the threads we were attached to
  pos = m_ThreadManagersMap.GetStartPosition();
  while(pos != NULL){
    m_ThreadManagersMap.GetNextAssoc(pos, dwThreadID, pEventManager);
    if(pEventManager == this){
      m_ThreadManagersMap.RemoveKey(dwThreadID);
    }
  }

  // remove all events here
  SEvent *pEvent = m_pFirstEvent, *pDel;
  while(pEvent != NULL){
    pDel = pEvent;
    pEvent = pEvent->m_pNext;
    m_EventsPool.Free(pDel);
  }

  m_pFirstEvent = NULL;
  m_pLastEvent = NULL;
}

int CEventManager::DoEvent(DWORD *pQuitParam)
{
  SEvent *pEvent;

  // we have to use a semaphore for acces to events list
  m_semaphoreListLock.Lock();
  // if empty do nothing
  if(m_pFirstEvent == NULL){
    m_semaphoreListLock.Unlock();
    return 0;
  }
  /// get the first event in the queue
  pEvent = m_pFirstEvent;
  m_pFirstEvent = pEvent->m_pNext;
  if(m_pFirstEvent != NULL) m_pFirstEvent->m_pPrev = NULL;
  if(m_pLastEvent == pEvent) m_pLastEvent = pEvent->m_pPrev;

  // if we get a quit event -> special state
  // get the param and return -1
  if(pEvent->m_dwEvent == m_dwQuitEvent){
    *pQuitParam = pEvent->m_dwParam;
    m_EventsPool.Free(pEvent);
    m_semaphoreListLock.Unlock();
    return -1;
  }

  // unlock the queue -> enable operations on it
  m_semaphoreListLock.Unlock();

  try 
  {
    // send the event to its destination
    pEvent->m_pDestination->Perform(pEvent->m_dwNotID, pEvent->m_dwEvent, pEvent->m_dwParam);
  }
  catch ( ... )
  {
    m_EventsPool.Free ( pEvent );
    throw;
  }

  m_EventsPool.Free(pEvent);

  return 1;
}

void CEventManager::InlayEvent(CNotifier *pNotifier, CObserver * pDestination, DWORD dwNotID, DWORD dwEvent, DWORD dwParam, DWORD dwPriority, BOOL bOnlyOne)
{
  SEvent *pNewEvent;

  ASSERT((dwEvent == m_dwQuitEvent) || (pDestination != NULL));

  // lock the list of events
  m_semaphoreListLock.Lock();

  // fill the new event
  pNewEvent = m_EventsPool.Allocate();
  pNewEvent->m_pDestination = pDestination;
  pNewEvent->m_pNotifier = pNotifier;
  pNewEvent->m_dwNotID = dwNotID;
  pNewEvent->m_dwEvent = dwEvent;
  pNewEvent->m_dwParam = dwParam;
  pNewEvent->m_dwPriority = dwPriority;
  
  BOOL bInsert = TRUE;
  SEvent *pEvent, *pAfter = NULL;

  // if the list is empty add it to the end
  if(m_pFirstEvent == NULL){
    m_pFirstEvent = pNewEvent;
    m_pLastEvent = pNewEvent;
    pNewEvent->m_pNext = NULL;
    pNewEvent->m_pPrev = NULL;
  }
  else{
    // go through the list of events in revers order
    // and insert the new event
    pEvent = m_pLastEvent;
    while(pEvent != NULL){
      // if we are to test the unique of the event
      if(bOnlyOne){
        // if the priority is greater than ours -> insert after
        // and break
        if(pEvent->m_dwPriority > dwPriority){
          pAfter = pEvent;
          break;
        }

        // if the priority is equal -> test if it's not the same event
        // if so -> break (no insert)
        if(pEvent->m_dwPriority == dwPriority){
          if((pEvent->m_pNotifier == pNotifier) && (pEvent->m_dwEvent == dwEvent)
            && (pEvent->m_pDestination == pDestination)){
            bInsert = FALSE;
            m_EventsPool.Free(pNewEvent);
            break;
          }
        }

        // elseway (we could insert it here)
        // if there is no place to insert our event make it here
        if(pAfter == NULL) pAfter = pEvent;
      }

      // if not testing the unique
      else{
        if(pEvent->m_dwPriority >= dwPriority){
          pAfter = pEvent;
          break;
        }
      }

      pEvent = pEvent->m_pPrev;
    }

    // now we have in pAfter position after which to insert
    // if the pAfter is NULL -> insert at the begining
    // bInsert says if we are to insert
    if(bInsert){
      if(pAfter){
        pNewEvent->m_pNext = pAfter->m_pNext;
        pNewEvent->m_pPrev = pAfter;
        pAfter->m_pNext = pNewEvent;
        if(pNewEvent->m_pNext != NULL)
          pNewEvent->m_pNext->m_pPrev = pNewEvent;
        else
          m_pLastEvent = pNewEvent;
      }
      else{
        pNewEvent->m_pNext = m_pFirstEvent;
        m_pFirstEvent = pNewEvent;
        pNewEvent->m_pPrev = NULL;
        if(pNewEvent->m_pNext != NULL)
          pNewEvent->m_pNext->m_pPrev = pNewEvent;
        else
          m_pLastEvent = pNewEvent;
      }
    }
  }

  // release the semaphore -> event is inlayed to the list
  m_semaphoreInlayedEvent.Unlock();

  // release the semaphore for the list
  m_semaphoreListLock.Unlock();
}

DWORD CEventManager::DoEventLoop()
{
  DWORD dwQuitParam;
  int ret = 0;

  while(ret != -1){  // if we get the Quit message -> quit
    // wait for event to be layed into our list
    m_semaphoreInlayedEvent.Lock();
    // process the event
    ret = DoEvent(&dwQuitParam);
  }

  return dwQuitParam;
}

void CEventManager::InlayQuitEvent(DWORD dwParam)
{
  InlayEvent(NULL, NULL, 0, m_dwQuitEvent, dwParam);
}

void CEventManager::RemoveEvents(CNotifier *pNotifier, CObserver *pObserver)
{
  SEvent *pEvent, *pDel;

  m_semaphoreListLock.Lock();

  pEvent = m_pFirstEvent;
  while(pEvent != NULL){
    // if the source and dest are equal -> remove the event
    if(!pObserver || (pObserver == pEvent->m_pDestination)){
      if(!pNotifier || (pNotifier == pEvent->m_pNotifier)){
        // remove and delete it
        pDel = pEvent;
        pEvent = pEvent->m_pNext;

        if(pDel->m_pPrev != NULL)
          pDel->m_pPrev->m_pNext = pDel->m_pNext;
        else
          m_pFirstEvent = pDel->m_pNext;

        if(pDel->m_pNext != NULL)
          pDel->m_pNext->m_pPrev = pDel->m_pPrev;
        else
          m_pLastEvent = pDel->m_pPrev;

        m_EventsPool.Free(pDel);
        continue;
      }
    }
    pEvent = pEvent->m_pNext;
  }

  m_semaphoreListLock.Unlock();
}

CEventManager * CEventManager::GetPrevManager()
{
  return m_pPrevManager;
}

CEventManager * CEventManager::FindEventManager(DWORD dwThreadID)
{
  CEventManager *pRes = NULL;

  if(!m_ThreadManagersMap.Lookup(dwThreadID, pRes)){
    return NULL;
  }
  return pRes;
}

CEventManager * CEventManager::AttachToThread(DWORD dwThreadID)
{
  CEventManager *pEventManager;

  if(dwThreadID == 0){
    dwThreadID = GetCurrentThreadId();
  }

  if(!m_ThreadManagersMap.Lookup(dwThreadID, pEventManager))
    pEventManager = NULL;
  CEventManager *pEM = this;
  m_ThreadManagersMap.SetAt(dwThreadID, pEM);
  return pEventManager;
}

void CEventManager::DetachFromThread(DWORD dwThreadID)
{
  if(dwThreadID == 0){
    dwThreadID = GetCurrentThreadId();
  }

  CEventManager *pEventManager;
  if(!m_ThreadManagersMap.Lookup(dwThreadID, pEventManager)){
    TRACE("Detach from thread which has no attached event manager.\n");
    return;
  }
  if(pEventManager != this){
    TRACE("Detach from thread which we were not attach to.\n");
    return;
  }
  m_ThreadManagersMap.RemoveKey(dwThreadID);
}

int CEventManager::DoEventMsg(DWORD *pQuitParam)
{
  HANDLE hSemaphore = (HANDLE)m_semaphoreInlayedEvent;

  switch(MsgWaitForMultipleObjects(1, &hSemaphore, FALSE, INFINITE, QS_ALLEVENTS)){
  case WAIT_OBJECT_0:
    break; // we locked the semaphore -> some event in the queue

  case WAIT_OBJECT_0 + 1:
    return 0; // some message is in the threads queue

  default:
    return 0; // some error -> ????
  }

  // so take the message
  return DoEvent(pQuitParam);
}

void CEventManager::PersistentSave(CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
  // store the previous manager pointer
  storage << (DWORD)m_pPrevManager;
}

void CEventManager::PersistentLoad(CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
  // and previous manager
  DWORD dwHlp;
  storage >> dwHlp;
  m_pPrevManager = (CEventManager *)dwHlp;
}

void CEventManager::PersistentTranslatePointers(CPersistentStorage &storage)
{
  m_pPrevManager = (CEventManager *)storage.TranslatePointer(m_pPrevManager);
}

void CEventManager::PersistentInit()
{
  // do nothing
}