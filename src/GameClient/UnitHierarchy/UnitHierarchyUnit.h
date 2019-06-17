// UnitHierarchyUnit.h: interface for the CUnitHierarchyUnit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNITHIERARCHYUNIT_H__2D1E3F04_7E75_11D4_B0F4_004F49068BD6__INCLUDED_)
#define AFX_UNITHIERARCHYUNIT_H__2D1E3F04_7E75_11D4_B0F4_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUnitHierarchyUnit : public CObject  
{
  DECLARE_DYNAMIC(CUnitHierarchyUnit);

public:
	CUnitHierarchyUnit();
	virtual ~CUnitHierarchyUnit();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // Creates the object from network structure
  void Create(CVirtualConnection *pVirtualConnection, CUnitHierarchyUnit *pCommander);
  // deletes the object
  virtual void Delete();

  CString GetName(){ return m_strName; }
  CString GetScriptName(){ return m_strScriptName; }
  CUnitHierarchyUnit *GetCommander(){ return m_pCommander; }
  DWORD GetZUnitID(){ return m_dwZUnitID; }

  DWORD GetSUnitID(){ return m_dwSUnitID; }
  DWORD GetSUnitTypeID(){ return m_dwSUnitTypeID; }

  CUnitHierarchyUnit *GetNextSibbling(){ return m_pNextSibbling; }
  CUnitHierarchyUnit *GetFirstInferior(){ return m_pFirstInferior; }

  BOOL IsSelected(){ return m_bSelected; }
  void SetSelected(BOOL bSelected = TRUE){ m_bSelected = bSelected; }
  
  BOOL IsExpanded(){ return m_bExpanded; }
  void SetExpanded(BOOL bExpanded = TRUE){ m_bExpanded = bExpanded; }

  DWORD GetHierarchyLevel(){ return m_dwHierarchyLevel; }

private:
  // ID of the ZUnit on the server
  DWORD m_dwZUnitID;
  // Our commander
  CUnitHierarchyUnit *m_pCommander;

  // the ID of the fysical unit or 0x0FFFFFFFF
  DWORD m_dwSUnitID;
  // ID of the fysical unit type
  DWORD m_dwSUnitTypeID;

  // Name of the script assigned
  CString m_strScriptName;
  // Name of the unit
  CString m_strName;

  // Next sibbling unit
  CUnitHierarchyUnit *m_pNextSibbling;
  // First inferior unit
  CUnitHierarchyUnit *m_pFirstInferior;

  // Selected flag
  BOOL m_bSelected;
  // Expanded flag
  BOOL m_bExpanded;

  // Hierarchy level (the general commander has 0)
  DWORD m_dwHierarchyLevel;

  friend class CUnitHierarchy;
};

#endif // !defined(AFX_UNITHIERARCHYUNIT_H__2D1E3F04_7E75_11D4_B0F4_004F49068BD6__INCLUDED_)
