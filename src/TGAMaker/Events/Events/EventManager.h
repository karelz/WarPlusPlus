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

class CNotifier;

class CEventManager : public CObject  
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
    CNotifier *m_pNotifier;
    DWORD m_dwEvent;
    DWORD m_dwParam;
    DWORD m_dwPriority;
  };
  typedef tagSEvent SEvent;

  // list of the events
  CTypedPtrList<CPtrList, SEvent *> m_listEvents;

public:
  
	// inlays the quit event in the queue
  void InlayQuitEvent(DWORD dwParam);
	// proceeds event in endless loop
  // until the quit event arrives
  DWORD DoEventLoop();
	// deletes the event manager
  void Delete();
	// creates the event manager
  BOOL Create();
	// inlays new event
  // if the bOnlyOne is TRUE -> if there is event with same notifier and event ID
  // in the queue no event will be inlayed
  void InlayEvent(CNotifier *pNotifier, DWORD dwEvent, DWORD dwParam, DWORD dwPriority = Priority_Normal, BOOL bOnlyOne = FALSE);
  // waits and process one event from the queue
  int DoEvent(DWORD *pQuitParam);
	CEventManager();
	virtual ~CEventManager();

  // ID for the special quit event
  static DWORD m_dwQuitEvent;

  enum{ Quit_OnClose = 0x0FF000000 };
private:
	// pointer to the previous event manager
  CEventManager * m_pPrevManager;
  // semaphore which works as counter for inlayed events
	CSemaphore m_semaphoreInlayedEvent;
  // semaphore which locks the list of events
	CSemaphore m_semaphoreListLock;
};

extern CEventManager *g_pEventManager;

#endif // !defined(AFX_EVENTMANAGER_H__187DD150_7179_11D2_AB3A_AF8EE90C7B60__INCLUDED_)
