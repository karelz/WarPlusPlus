// Notifier.cpp: implementation of the CNotifier class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Notifier.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CNotifier, CObserver);

CNotifier::CNotifier()
{
  m_pFirstObserver = NULL;
}

CNotifier::~CNotifier()
{
  ASSERT(m_pFirstObserver == NULL);
}

CMultiThreadTypedMemoryPool<CNotifier::SObserverNode> CNotifier::m_ConnectionsPool(500);

////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CNotifier::AssertValid() const
{
  CObserver::AssertValid();
}

void CNotifier::Dump(CDumpContext &dc) const
{
  CObserver::Dump(dc);
}

#endif

// deletes the notifier object
void CNotifier::Delete()
{
  SObserverNode *pNode, *pDel;

	// remove all left observers nodes
  pNode = m_pFirstObserver;
  while(pNode != NULL){
    pDel = pNode;
    pNode = pNode->m_pNext;
    DeleteNode(pDel);
	}
  m_pFirstObserver = NULL;
}

// creates the notifier object
// if you override this function remember to call the base class one
BOOL CNotifier::Create()
{
  return TRUE;
}

// connects the observer to this notifier
void CNotifier::Connect(CObserver * pObserver, DWORD dwNotifierID, DWORD dwThreadID)
{
  SObserverNode *pObserverNode;
  CEventManager *pEventManager;

  ASSERT(pObserver != NULL);

  if(dwThreadID == 0){
    dwThreadID = GetCurrentThreadId();
  }
  
  // if no event manager for given thread -> return
  if((pEventManager = CEventManager::FindEventManager(dwThreadID)) == NULL){
    TRACE("Notifier : Requested connection on thread with no associated event manager.\n");
    ASSERT(FALSE);
    return;
  }

  WriteLock();
  // try to find the same record
  pObserverNode = m_pFirstObserver;
  while(pObserverNode != NULL){
    // if we can find the same record -> do nothing
    // the connection already exists
    if((pObserverNode->m_pObserver == pObserver)
      && (pObserverNode->m_dwNotifierID == dwNotifierID)){
	  pObserverNode->m_dwReferenceCount++;
      WriteUnlock();
      return;
    }
    pObserverNode = pObserverNode->m_pNext;
  }

  pObserverNode = AllocateConnectionNode();
  pObserverNode->m_dwNotifierID = dwNotifierID;
  pObserverNode->m_pObserver = pObserver;
  pObserverNode->m_pConnectionData = NULL;
  pObserverNode->m_dwReferenceCount = 1;
  pObserverNode->m_pEventManager = pEventManager;

  pObserverNode->m_pNext = m_pFirstObserver;
  m_pFirstObserver = pObserverNode;

  WriteUnlock();
}

// disconnects the observer from the notification
void CNotifier::Disconnect(CObserver * pObserver)
{
  SObserverNode *pNode, *pPrevNode;
	
	ASSERT(pObserver != NULL);

  WriteLock();
  pNode = m_pFirstObserver;
  pPrevNode = NULL;
  while(pNode != NULL){
    if(pNode->m_pObserver == pObserver)
      break;
    pPrevNode = pNode;
    pNode = pNode->m_pNext;
  }

  // no such connection exists
  if(pNode == NULL){
    WriteUnlock();
    return;
  }

  pNode->m_dwReferenceCount--;
  if(pNode->m_dwReferenceCount > 0){
    WriteUnlock();
	return;
  }

  // remove all pending events for the observer
  pNode->m_pEventManager->RemoveEvents(this, pObserver);

  // remove the node from the list
  if(pPrevNode == NULL)
    m_pFirstObserver = pNode->m_pNext;
  else
    pPrevNode->m_pNext = pNode->m_pNext;

  // delete the node
  DeleteNode(pNode);

  WriteUnlock();
}

// Finds the node for givven observer
CNotifier::SObserverNode * CNotifier::FindObserverNode(CObserver * pObserver)
{
  SObserverNode *pNode;

  ASSERT(pObserver != NULL);

	ReadLock();
  pNode = m_pFirstObserver;
	while(pNode != NULL){
    if(pNode->m_pObserver == pObserver){
      ReadUnlock();
      return pNode;
    }
    pNode = pNode->m_pNext;
	}
  ReadUnlock();

	return NULL;
}

// delete one node -> override this virtual method to do some
// more clening (espacialy for m_pConnectionData member)
void CNotifier::DeleteNode(SObserverNode * pNode)
{
  FreeConnectionNode(pNode);
}

// inlays the event for this notifier to current event manager
void CNotifier::InlayEvent(DWORD dwEvent, DWORD dwParam, CObserver *pObserver, DWORD dwPriority, BOOL bOnlyOne, BOOL bLocked)
{
  SObserverNode *pNode;

  if(pObserver){
    // find the connection node for the observer
    if(!bLocked)
      ReadLock();
    pNode = m_pFirstObserver;
    while(pNode != NULL){
      if(pNode->m_pObserver == pObserver)
        break;
      pNode = pNode->m_pNext;
    }
    if(pNode == NULL){
      // no such observer connection
      if(!bLocked)
        ReadUnlock();
      return;
    }

    // inlay the event
    pNode->m_pEventManager->InlayEvent(this, pObserver, pNode->m_dwNotifierID,
      dwEvent, dwParam, dwPriority, bOnlyOne);

    if(!bLocked)
      ReadUnlock();
    // well done
    return;
  }

  if(!bLocked)
    ReadLock();

  // go through all connections
  pNode = m_pFirstObserver;
  while(pNode != NULL){

    // inlay the event
    pNode->m_pEventManager->InlayEvent(this, pNode->m_pObserver,
      pNode->m_dwNotifierID, dwEvent, dwParam, dwPriority, bOnlyOne);

    pNode = pNode->m_pNext;
  }

  if(!bLocked)
    ReadUnlock();
}

BOOL CNotifier::SendEvent(DWORD dwEvent, DWORD dwParam, CObserver *pObserver)
{
  SObserverNode *pNode;

  if(pObserver){
    pNode = FindObserverNode(pObserver);
    if(pNode)
      return pObserver->Perform(pNode->m_dwNotifierID, dwEvent, dwParam);
    return FALSE;
  }

  // here is small hack -> undocumented
  // abort notifier uses this portion of code
  // it sends event to NULL observer -> all of them
  // here we won't take care of some semaphores or so
  // it's special action
  pNode = m_pFirstObserver;
  SObserverNode * pDoIt;
  while(pNode != NULL){
    pDoIt = pNode;
    pNode = pNode->m_pNext;
    pDoIt->m_pObserver->Perform(pDoIt->m_dwNotifierID, dwEvent, dwParam);
  }
  return FALSE;
}

CNotifier::SObserverNode * CNotifier::OpenConnections()
{
  ReadLock();
  return m_pFirstObserver;
}

CNotifier::SObserverNode * CNotifier::GetNextConnection(CNotifier::SObserverNode *pNode)
{
  return pNode->m_pNext;
}

void CNotifier::CloseConnections()
{
  ReadUnlock();
}

void CNotifier::PersistentSave(CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
  // store count
  SObserverNode *pNode;
  DWORD dwCount;
  for(pNode = m_pFirstObserver, dwCount = 0; pNode != NULL; pNode = pNode->m_pNext, dwCount++);

  storage << dwCount; 

  pNode = m_pFirstObserver;
  // and all nodes (as they are)
  while(pNode != NULL){
    // we are now not able to store some connection data (possible pointers inside)
    ASSERT(pNode->m_pConnectionData == NULL);
    storage.Write(pNode, sizeof(SObserverNode));
    pNode = pNode->m_pNext;
  }
}

void CNotifier::PersistentLoad(CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    // here just read the data

  DWORD dwCount, i;
  // count
  storage >> dwCount;

  SObserverNode *pObserverNode;

  // and all structures
  for(i = 0; i < dwCount; i++){
    pObserverNode = AllocateConnectionNode();
    storage.Read(pObserverNode, sizeof(SObserverNode));
    // add it to the tail
    SObserverNode *p = m_pFirstObserver;
    pObserverNode->m_pNext = NULL;
    if(p == NULL){
      m_pFirstObserver = pObserverNode;
    }
    else{
      while(p->m_pNext != NULL) p = p->m_pNext;
      p->m_pNext = pObserverNode;
    }
  }
}

void CNotifier::PersistentTranslatePointers(CPersistentStorage &storage)
{
  // now translate pointers
  SObserverNode *pNode;

  pNode = m_pFirstObserver;
  while(pNode != NULL){
    // translate manager and observer
    pNode->m_pEventManager = (CEventManager *)storage.TranslatePointer(pNode->m_pEventManager);
    pNode->m_pObserver = (CObserver *)storage.TranslatePointer(pNode->m_pObserver);
    pNode = pNode->m_pNext;
  }
}

void CNotifier::PersistentInit()
{
  // do nothing
}