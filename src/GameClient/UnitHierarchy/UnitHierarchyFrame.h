// UnitHierarchyFrame.h: interface for the CUnitHierarchyFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNITHIERARCHYFRAME_H__E44B2BF3_7FF6_11D4_B0F8_004F49068BD6__INCLUDED_)
#define AFX_UNITHIERARCHYFRAME_H__E44B2BF3_7FF6_11D4_B0F8_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUnitHierarchy;
class CUnitHierarchyUnit;

class CUnitHierarchyFrame : public CCaptionWindow  
{
  DECLARE_DYNAMIC(CUnitHierarchyFrame);
  DECLARE_OBSERVER_MAP(CUnitHierarchyFrame)

public:
	CUnitHierarchyFrame();
	virtual ~CUnitHierarchyFrame();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  void Create(CRect rcWindow, CWindow *pParent, CCaptionWindowLayout *pLayout, CTreeControlLayout *pTreeLayout,
    CUnitHierarchy *pUnitHierarchy);
  virtual void Delete();

protected:
  virtual void OnSize(CSize size);

  void OnClose();
  void OnRefresh();
  void OnHierarchyUpdated(int nStatus);
  void OnChangeScript();
  void OnChangeName();

  void OnChangedScriptName(DWORD dwErrors);
  void OnChangedName(DWORD dwErrors);

  void OnItemExpanded(int nItem);
  void OnItemCollapsed(int nItem);
  void OnItemSelected(int nItem);
  void OnItemDeselected(int nItem);

  void OnServerError();

private:
  void GetTreeRect(CRect *pRect);
  // the hierarchy must be locked
  void SetAvailable(BOOL bAvailable);
  void FillTree(CUnitHierarchyUnit *pUnit, int nParent);
  // the hierarchy must be locked
  void UpdateTree();

  CImageButton m_CloseButton;

  CTextButton m_Refresh, m_ChangeScript, m_ChangeName;

  enum{
    IDC_CLOSE = 0x0100,
    IDC_TREE = 0x0101,
    IDC_REFRESH = 0x0102,
    IDC_CHANGESCRIPT = 0x0103,
    IDC_CHANGENAME = 0x0104,

    IDC_HIERARCHY = 0x0200,
  };

  CTreeControl m_wndTree;
  CUnitHierarchy *m_pUnitHierarchy;

  BOOL m_bAvailable;

  enum EState{
    State_None = 0,
    State_ScriptChange = 1,
    State_NameChange = 2,
  };
  EState m_eState;
};

#endif // !defined(AFX_UNITHIERARCHYFRAME_H__E44B2BF3_7FF6_11D4_B0F8_004F49068BD6__INCLUDED_)
