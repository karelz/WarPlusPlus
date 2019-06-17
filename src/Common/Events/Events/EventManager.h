// EventManager.h: interface for the CEventManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVENTMANAGER_H__187DD150_7179_11D2_AB3A_AF8EE90C7B60__INCLUDED_)
#define AFX_EVENTMANAGER_H__187DD150_7179_11D2_AB3A_AF8EE90C7B60__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afxtempl.h>
#include <afxmt.h>
#include "Common\PersistentStorage\PersistentObject.h"
#include "Common\MemoryPool\MultiThreadTypedMemoryPool.h"

class CNotifier;
class CObserver;

class CEventManager : public CPersistentObject  
{
  DECLARE_DYNAMIC(CEventManager);

#ifdef _DEBUG
public:
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

public:
  enum Priorities{
    Priority_Low = 0x040000000,
    Priority_Normal = 0x080000000,
    Priority_High = 0x0C0000000
  };

private:
  enum{ EventMaxCount = 0x0FFFF };

  struct tagSEvent{
    CObserver *m_pDestination; // dest of the event
    CNotifier *m_pNotifier; // source of the event
    DWORD m_dwNotID;
    DWORD m_dwEvent;
    DWORD m_dwParam;
    DWORD m_dwPriority;
    struct tagSEvent *m_pNext;
    struct tagSEvent *m_pPrev;
  };
  typedef tagSEvent SEvent;

  // list of the events
  SEvent *m_pFirstEvent, *m_pLastEvent;

public:
	int DoEventMsg(DWORD *pQuitParam);
	// ataches this event manager to given thread
  // all events for this thread will be managed by this event manager
  // if the dwThreadID is 0 -> current thread is taken
  // doesn't remeber the previous event manager for the thread
  CEventManager * AttachToThread(DWORD dwThreadID = 0);
  // Detaches the event manager from the given thread
  // If it was not attached to it (do nothing)
  void DetachFromThread(DWORD dwThreadID = 0);
	// return the event manager for given thread
  static CEventManager * FindEventManager(DWORD dwThreadID);
  // returns the previous event manager
	CEventManager * GetPrevManager();
  // removes all events for given notifier and observer
  // if the notifier is NULL -> all notifiers will be taken for given observer
  // if the observer is NULL -> all observers will be taken for given notifier
  // if both of them are NULL -> all events are removed
  void RemoveEvents(CNotifier *pNotifier, CObserver *pObserver);
  
	// inlays the quit event in the queue
  void InlayQuitEvent(DWORD dwParam);
	// proceeds event in endless loop
  // until the quit event arrives
  DWORD DoEventLoop();
	// deletes the event manager
  void Delete();
	// creates the event manager
  // parametr is the tread ID for which is this manager created
  // if 0 hte current thread is taken
  BOOL Create(DWORD dwThreadID = 0);
	// inlays new event
  // if the bOnlyOne is TRUE -> if there is event with same notifier and event ID
  // in the queue no event will be inlayed
  void InlayEvent(CNotifier *pNotifier, CObserver *pDestination, DWORD dwNotID, DWORD dwEvent, DWORD dwParam, DWORD dwPriority = Priority_Normal, BOOL bOnlyOne = FALSE);
  // waits and process one event from the queue
  int DoEvent(DWORD *pQuitParam);
	CEventManager();
	virtual ~CEventManager();

  // ID for the special quit event
  static DWORD m_dwQuitEvent;

  void PersistentSave(CPersistentStorage &storage);
  void PersistentLoad(CPersistentStorage &storage);
  void PersistentTranslatePointers(CPersistentStorage &storage);
  void PersistentInit();

  enum{ Quit_OnClose = 0x0FF000000 };
private:
  // thread ID of the thred for which was this manager created
  // if its connected to other threads -> there are more of them
  // so this identifies the primary one
	DWORD m_dwPrimaryThreadID;
	// pointer to the previous event manager
  CEventManager * m_pPrevManager;
  // semaphore which works as counter for inlayed events
	CSemaphore m_semaphoreInlayedEvent;
  // semaphore which locks the list of events
	CSemaphore m_semaphoreListLock;

  // memory pool for event structures
  CMultiThreadTypedMemoryPool<SEvent> m_EventsPool;

  // map of the thread IDs and event managers
  static CMap<DWORD, DWORD&, CEventManager *, CEventManager *&> m_ThreadManagersMap;
};

#endif // !defined(AFX_EVENTMANAGER_H__187DD150_7179_11D2_AB3A_AF8EE90C7B60__INCLUDED_)
