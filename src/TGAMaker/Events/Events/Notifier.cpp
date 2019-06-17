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

}

CNotifier::~CNotifier()
{
}

////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CNotifier::AssertValid() const
{
  CObserver::AssertValid();
  m_listObservers.AssertValid();
}

void CNotifier::Dump(CDumpContext &dc) const
{
  CObserver::Dump(dc);
  dc << m_listObservers;
}

#endif

// for all observers connected to us call their method Perform
// with the event we are working on
// this is equal to sending a message to them
void CNotifier::DoNotification(DWORD dwEvent, DWORD dwParam)
{
  POSITION pos;
  SObserverNode *pNode;
  DWORD dwE, dwP;

// lock the actions around the nodes
  Lock();
  if(!PrepareNotification(dwEvent, dwParam)){
    RemoveDeleted();
    Unlock();
    return;
  }
  pos = m_listObservers.GetHeadPosition();
  while(pos != NULL){
    dwE = dwEvent; dwP = dwParam; // copy them -> they can be changed
    pNode = m_listObservers.GetNext(pos);
    if(pNode->m_bDeleted) continue;
    if(!PreNotify(pNode, dwE, dwP)) continue;

    // unlock before sending the message (prevent from deadlock)
    // connect can be called from the Perform
    Unlock();
    if(pNode->m_pObserver->Perform(pNode->m_dwNotifierID, dwE, dwP)){
      Lock();
      break;
    }
    Lock();
    // imidietly after lock it again
  }
  // remove all deleted items (this time it's safe)
  RemoveDeleted();
  Unlock();
}

// deletes the notifier object
void CNotifier::Delete()
{
	POSITION pos;
  SObserverNode *pNode;

	// remove all left observers nodes
  pos = m_listObservers.GetHeadPosition();
	while(pos != NULL){
    pNode = m_listObservers.GetNext(pos);
    DeleteNode(pNode);
	}
	m_listObservers.RemoveAll();
}

// creates the notifier object
// if you override this function remember to call the base class one
BOOL CNotifier::Create()
{
  return TRUE;
}

// connects the observer to this notifier
void CNotifier::Connect(CObserver * pObserver, DWORD dwNotifierID)
{
	SObserverNode *pObserverNode;

  ASSERT(pObserver != NULL);

  // have to lock it (two observers can try to connect at same time)
  Lock();
  // try to find the same record
  POSITION pos = m_listObservers.GetHeadPosition();
  while(pos != NULL){
    pObserverNode = m_listObservers.GetNext(pos);
    // if we can find the same record -> do nothing
    // the connection already exists
    if((pObserverNode->m_pObserver == pObserver)
      && (pObserverNode->m_dwNotifierID == dwNotifierID)){
      Unlock();
      return;
    }
  }

  pObserverNode = new SObserverNode;
	pObserverNode->m_bDeleted = FALSE;
	pObserverNode->m_dwNotifierID = dwNotifierID;
	pObserverNode->m_pObserver = pObserver;
	pObserverNode->m_pConnectionData = NULL;

	m_listObservers.AddHead(pObserverNode);
  Unlock();
}

// disconnects the observer from the notification
void CNotifier::Disconnect(CObserver * pObserver)
{
  SObserverNode *pNode;
	
	ASSERT(pObserver != NULL);

	// lock it
  Lock();
  pNode = FindObserverNode(pObserver);
	// we have to mark the node for deleting
  // can't delete it (it can be performed in DoNotification)
  if(pNode != NULL)
    pNode->m_bDeleted = TRUE;
  Unlock();
}

// Finds the node for givven observer
// we can't lock something here because it's called
// from within locked sections (such as Disconnect)
// it's used during the connection and disconnection (they are locked)
CNotifier::SObserverNode * CNotifier::FindObserverNode(CObserver * pObserver)
{
  SObserverNode *pNode;
	POSITION pos;

  ASSERT(pObserver != NULL);

	pos = m_listObservers.GetHeadPosition();
	while(pos != NULL){
    pNode = m_listObservers.GetNext(pos);
		if(pNode->m_pObserver == pObserver) return pNode;
	}

	return NULL;
}

// Removes all nodes marked for deleting
// no locking (locked by DoNotification)
void CNotifier::RemoveDeleted()
{
  POSITION pos, pos2;
  SObserverNode *pNode;

  pos = m_listObservers.GetHeadPosition();
  while(pos != NULL){
    pos2 = pos;
    pNode = m_listObservers.GetNext(pos);
    if(!pNode->m_bDeleted) continue;
    m_listObservers.RemoveAt(pos2);
    DeleteNode(pNode);
  }
}

// delete one node -> override this virtual method to do some
// more clening (espacialy for m_pConnectionData member)
void CNotifier::DeleteNode(SObserverNode * pNode)
{
  delete pNode;
}

// called before sending the event to the observer
// override this function to some more testing if to send the event
// or (and) to modify the event
BOOL CNotifier::PreNotify(SObserverNode * pNode, DWORD & dwEvent, DWORD & dwParam)
{
  return TRUE;
}

// inlays the event for this notifier to current event manager
void CNotifier::InlayEvent(DWORD dwEvent, DWORD dwParam, DWORD dwPriority, BOOL bOnlyOne)
{
  if(g_pEventManager != NULL){
    g_pEventManager->InlayEvent(this, dwEvent, dwParam, dwPriority, bOnlyOne);
  }
}

BOOL CNotifier::PrepareNotification(DWORD & dwEvent, DWORD & dwParam)
{
  return TRUE;
}