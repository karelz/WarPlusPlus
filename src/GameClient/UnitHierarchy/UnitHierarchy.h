// UnitHierarchy.h: interface for the CUnitHierarchy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNITHIERARCHY_H__2D1E3F05_7E75_11D4_B0F4_004F49068BD6__INCLUDED_)
#define AFX_UNITHIERARCHY_H__2D1E3F05_7E75_11D4_B0F4_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UnitHierarchyUnit.h"

class CUnitHierarchy : public CNotifier
{
  DECLARE_DYNAMIC(CUnitHierarchy);
  DECLARE_OBSERVER_MAP(CUnitHierarchy);

public:
	CUnitHierarchy();
	virtual ~CUnitHierarchy();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // Creation
  void Create();
  // Deletes the object
  virtual void Delete();

  // The unit hierarchy lock
  void Lock(){ VERIFY(m_lockLock.Lock()); }
  void Unlock(){ VERIFY(m_lockLock.Unlock()); }

  // Returns TRUE if the hierarchy is available (update is not in progress)
  // !!! must be locked
  BOOL IsAvailable(){ return !m_bUpdating; }
  // Returns the general commander
  // !!! the unit hierarchy must be locked to do so
  CUnitHierarchyUnit *GetGeneralCommander(){ return m_pGeneralCommander; }

  // Find a unit by it's ZUnit ID
  // can search in subtree if givven the parent
  CUnitHierarchyUnit *FindZUnit(DWORD dwZUnitID, CUnitHierarchyUnit *pParent = NULL);

  // Removes the unit from hierarchy (and all units under)
  void DeleteUnit(CUnitHierarchyUnit *pUnit);

  // Starts the refreshing of the unit hierarchy
  // No lock required
  // returns FALSe if the server is down
  BOOL Refresh();

  enum{
    E_HierarchyUpdated = 1,  // The hierarchy was updated (when the packet arrived from server)
                             // If the dwParam is (DWORD)-1, than server error occured
    E_ChangedScriptName = 2, // The script change request is done (dwParam is number of errors)
    E_ChangedName = 3, // The name change request is done (dwParam is number of errors)
  };

  // returns FALSE if the server is down
  BOOL ChangeScript(CString strNewScript, DWORD *pZUnitIDs, DWORD dwZUnitCount);
  BOOL ChangeName(CString strNewName, DWORD *pZUnitIDs, DWORD dwZUnitCount);
  // Replies with E_ChangeScriptName (E_ChangeName) on success
  // or with E_HierarchyUpdated with param (DWORD)-1 -> server error

protected:
  void OnPacketAvailable();
  void OnNetworkError();

private:
  void Clear();

  // The network virtual connection
  CVirtualConnection m_Connection;
  enum{ ID_Connection = 0x0100, };

  // The general commander record
  CUnitHierarchyUnit *m_pGeneralCommander;

  // Remeber the name or script name we're changing to
  CString m_strChangeName, m_strChangeScriptName;

  // Lock for this object
  CMutex m_lockLock;
  BOOL m_bUpdating;
};

#endif // !defined(AFX_UNITHIERARCHY_H__2D1E3F05_7E75_11D4_B0F4_004F49068BD6__INCLUDED_)
