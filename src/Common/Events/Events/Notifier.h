// Notifier.h: interface for the CNotifier class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NOTIFIER_H__D0A2D058_1644_11D2_8AFD_000000000000__INCLUDED_)
#define AFX_NOTIFIER_H__D0A2D058_1644_11D2_8AFD_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "EventManager.h"
#include "Observer.h"
#include "Common\PersistentStorage\PersistentObject.h"

class CNotifier : public CObserver  
{
protected:
  struct tagSObserverNode{
		CObserver *m_pObserver;
        CEventManager *m_pEventManager;
		DWORD m_dwNotifierID;
		LPVOID m_pConnectionData;
		DWORD m_dwReferenceCount;
    struct tagSObserverNode *m_pNext;
	};
	typedef tagSObserverNode SObserverNode;

  DECLARE_DYNAMIC(CNotifier);

#ifdef _DEBUG
public:
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

public:
	// closes the loop through the connections
  // this functions unlocks the object
  void CloseConnections();
	// returns connection node for given position
  // and moves the position to the next one
  // pos is NULL f at the end of list
  SObserverNode * GetNextConnection(SObserverNode * pNode);
  // returns pos to the first connection (NULL if empty)
  // locks the object for read access, so you shouldn't change anything -> might broke down
  // you shouldn't call any other functions on this object
  // in the connections loop (it would cause deadlock)
	SObserverNode * OpenConnections();
	// send immidietly the event to the observer
  // the pObserver can't be NULL
  // rather use the InlayEvent -> it uses queue for the event
  // also this invokes the reaction to the event in current thread
  // returns TRUE -> if the called object processed the message
  // or FALSE if not (method Perform returned TRUE or FALSE)
  BOOL SendEvent(DWORD dwEvent, DWORD dwParam, CObserver *pObserver);
  // inlays new event (source this object) for given observer
  // if the observer is 0 -> all connected observers will get this event
  // if bOnlyOne is TRUE -> if there is event with same notifier and event ID
  // in the queue -> no event will be sent
  void InlayEvent(DWORD dwEvent, DWORD dwParam, CObserver *pObserver = 0, DWORD dwPriority = CEventManager::Priority_Normal, BOOL bOnlyOne = FALSE, BOOL bLocked = FALSE);
	// disconnects tobserver from the object
  virtual void Disconnect(CObserver *pObserver);
	// connects observer to this object in given thread
  // if the thread is 0 -> current thread is taken
  virtual void Connect(CObserver *pObserver, DWORD dwNotifierID, DWORD dwThreadID = 0);
	// creates the object
  BOOL Create();
	// deletes the object
  virtual void Delete();
  // sends the event to observers
	CNotifier();
	virtual ~CNotifier();

  void PersistentSave(CPersistentStorage &storage);
  void PersistentLoad(CPersistentStorage &storage);
  void PersistentTranslatePointers(CPersistentStorage &storage);
  void PersistentInit();

protected:
	// called before the start of performing event to observers
	// called before performing the event at given observer
  // called to delete the node
  virtual void DeleteNode(SObserverNode *pNode);
	// removes all deleted observers from the connection list
	// finds a connection node for given observer
  SObserverNode * FindObserverNode(CObserver *pObserver);
  // list of connected observers
  SObserverNode * m_pFirstObserver;

protected:
  // locks critical actions (implemented in CMultithreadNotifier)
  // read locks - any number of read acceses at the time, no write access
  // write lock - one write access only (no reads or other writes)
  virtual void ReadLock() {}
  virtual void WriteLock() {}
  // unlocks critical actions (implemented in CMultithreadNotifier)
  virtual void ReadUnlock() {}
  virtual void WriteUnlock() {}

private:
  // memory pool stuff (pool and mutexes)
  static CMultiThreadTypedMemoryPool<SObserverNode> m_ConnectionsPool;
  // some helper functions
  SObserverNode *AllocateConnectionNode(){
    SObserverNode *pNode;
    pNode = m_ConnectionsPool.Allocate();
    return pNode;
  }
  void FreeConnectionNode(SObserverNode *pNode){
    m_ConnectionsPool.Free(pNode);
  }

  friend class CMultithreadNotifier;
};

#endif // !defined(AFX_NOTIFIER_H__D0A2D058_1644_11D2_8AFD_000000000000__INCLUDED_)
