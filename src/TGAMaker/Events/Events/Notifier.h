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

class CNotifier : public CObserver  
{
protected:
  struct tagSObserverNode{
		CObserver *m_pObserver;
		DWORD m_dwNotifierID;
		BOOL m_bDeleted;
		LPVOID m_pConnectionData;
	};
	typedef tagSObserverNode SObserverNode;

  DECLARE_DYNAMIC(CNotifier);

#ifdef _DEBUG
public:
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

public:
  // inlays new event (source this object)
  // if bOnlyOne is TRUE -> if there is event with same notifier and event ID
  // in the queue -> no event will be sent
  void InlayEvent(DWORD dwEvent, DWORD dwParam, DWORD dwPriority = CEventManager::Priority_Normal, BOOL bOnlyOne = FALSE);
	// disconnects tobserver from the object
  virtual void Disconnect(CObserver *pObserver);
	// connects observer to this object
  virtual void Connect(CObserver *pObserver, DWORD dwNotifierID);
	// creates the object
  BOOL Create();
	// deletes the object
  virtual void Delete();
  // sends the event to observers
  virtual void DoNotification(DWORD dwEvent, DWORD dwParam);
	CNotifier();
	virtual ~CNotifier();

protected:
	// called before the start of performing event to observers
  virtual BOOL PrepareNotification(DWORD &dwEvent, DWORD &dwParam);
	// called before performing the event at given observer
  virtual BOOL PreNotify(SObserverNode *pNode, DWORD &dwEvent, DWORD &dwParam);
  // called to delete the node
  virtual void DeleteNode(SObserverNode *pNode);
	// removes all deleted observers from the connection list
  void RemoveDeleted();
	// finds a connection node for given observer
  SObserverNode * FindObserverNode(CObserver *pObserver);
  // list of connected observers
  CTypedPtrList<CPtrList, SObserverNode *> m_listObservers;
private:
  // locks critical actions (implemented in CMultithreadNotifier)
  virtual void Lock() {}
  // unlocks critical actions (implemented in CMultithreadNotifier)
  virtual void Unlock() {}

  friend class CMultithreadNotifier;
};

#endif // !defined(AFX_NOTIFIER_H__D0A2D058_1644_11D2_8AFD_000000000000__INCLUDED_)
