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

CEventManager *g_pEventManager = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEventManager, CObject);

CEventManager::CEventManager()
 : m_semaphoreInlayedEvent(0, EventMaxCount)
{
  m_pPrevManager = NULL;
}

CEventManager::~CEventManager()
{
  Delete();
}

/////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CEventManager::AssertValid() const
{
  CObject::AssertValid();
}

void CEventManager::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
  dc << "Unprocessed events : " << m_listEvents.GetCount() << "\n";
}

#endif


DWORD CEventManager::m_dwQuitEvent = 0xFFFFFFFF;

BOOL CEventManager::Create()
{
  m_pPrevManager = g_pEventManager;
  g_pEventManager = this;
  
  return TRUE;
}

void CEventManager::Delete()
{
  POSITION pos;

  g_pEventManager = m_pPrevManager;
  m_pPrevManager = NULL;

  pos = m_listEvents.GetHeadPosition();
  while(pos != NULL){
    delete m_listEvents.GetNext(pos);
  }
  m_listEvents.RemoveAll();
}

int CEventManager::DoEvent(DWORD *pQuitParam)
{
  SEvent *pEvent;
  CSingleLock lock(&m_semaphoreListLock);

  // we have to use a semaphore for acces to events list
  lock.Lock();
  if(m_listEvents.IsEmpty()) return 0;
  pEvent = m_listEvents.RemoveHead();
  lock.Unlock();

  // if we get a quit event -> special state
  // get the param and return -1
  if(pEvent->m_dwEvent == m_dwQuitEvent){
    *pQuitParam = pEvent->m_dwParam;
    delete pEvent;
    return -1;
  }

  pEvent->m_pNotifier->DoNotification(pEvent->m_dwEvent, pEvent->m_dwParam);
  delete pEvent;

  return 1;
}

void CEventManager::InlayEvent(CNotifier * pNotifier, DWORD dwEvent, DWORD dwParam, DWORD dwPriority, BOOL bOnlyOne)
{
  SEvent *pNewEvent;
  CSingleLock lock(&m_semaphoreListLock);

  ASSERT((dwEvent == m_dwQuitEvent) || (pNotifier != NULL));

  pNewEvent = new SEvent;
  pNewEvent->m_pNotifier = pNotifier;
  pNewEvent->m_dwEvent = dwEvent;
  pNewEvent->m_dwParam = dwParam;
  pNewEvent->m_dwPriority = dwPriority;
  
  lock.Lock();

  POSITION pos, curpos = NULL, newpos = NULL;
  BOOL bInsert = TRUE;
  SEvent *pEvent;

  if(m_listEvents.IsEmpty()){
    m_listEvents.AddTail(pNewEvent);
  }
  else{
    // go through the list of events in revers order
    // and insert the new event
    curpos = m_listEvents.GetTailPosition();
    while(curpos != NULL){
      pos = curpos;
      pEvent = m_listEvents.GetPrev(curpos);
      
      // if we are to test the unique of the event
      if(bOnlyOne){
        // if the priority is greater than ours -> insert after
        // and break
        if(pEvent->m_dwPriority > dwPriority){
          newpos = pos;
          break;
        }

        // if the priority is equal -> test if it's not the same event
        // if so -> break (no insert)
        if(pEvent->m_dwPriority == dwPriority){
          if((pEvent->m_pNotifier == pNotifier) && (pEvent->m_dwEvent == dwEvent)){
            bInsert = FALSE;
            break;
          }
        }

        // elseway (we could insert it here)
        // if there is no place to insert our event make it here
        if(newpos == NULL) newpos = pos;
      }

      // if not testing th eunique
      else{
        if(pEvent->m_dwPriority >= dwPriority){
          newpos = pos;
          break;
        }
      }
    }

    // now we have in newpos position after which to insert
    // if the newpos is NULL -> insert at the begining
    // bInsert says if we are to insert
    if(bInsert){
      if(newpos){
        m_listEvents.InsertAfter(newpos, pNewEvent);
      }
      else{
        m_listEvents.AddHead(pNewEvent);
      }
    }
  }

  lock.Unlock();
  // release the semaphore -> event is inlayed to the list
  m_semaphoreInlayedEvent.Unlock();
}

DWORD CEventManager::DoEventLoop()
{
  DWORD dwQuitParam;
  int ret;

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
  InlayEvent(NULL, m_dwQuitEvent, dwParam);
}
