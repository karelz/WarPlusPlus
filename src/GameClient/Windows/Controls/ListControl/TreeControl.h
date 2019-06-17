// TreeControl.h: interface for the CTreeControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TREECONTROL_H__4D86CBF3_7F3C_11D4_B0F5_004F49068BD6__INCLUDED_)
#define AFX_TREECONTROL_H__4D86CBF3_7F3C_11D4_B0F5_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ListControl.h"
#include "TreeControlLayout.h"

class CTreeControl : public CListControl  
{
  DECLARE_OBSERVER_MAP(CTreeControl);
  DECLARE_DYNAMIC(CTreeControl);

public:
	CTreeControl();
	virtual ~CTreeControl();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // Adds item as last child of givven parent
  // if parent is -1 -> inserted to the root
  virtual int AddItem(int nItem, CString strText);

  // Inserts item under some parent and after given item
  virtual int InsertItem(int nParent, int nAfter, CString strText);
  // Delete an item and whole its subtree
  virtual void DeleteItem(int nItem);

  // returns index of parent item, if none -> -1
  virtual int GetParentItem(int nItem);
  // returns index of first child item, if none -> -1
  virtual int GetFirstChild(int nItem);
  // returns index of next sibbling item, if none -> -1
  virtual int GetNextSibbling(int nItem);

  // collapse item
  virtual void CollapseItem(int nItem);
  // expands item
  virtual void ExpandItem(int nItem);

  // returns true if the item is expanded
  virtual BOOL IsItemExpanded(int nItem);

  // deletes the object
	virtual void Delete();
  // creates the list control
	BOOL Create(CRect &rcBound, CTreeControlLayout *pLayout, CWindow *pParent, DWORD dwColumns = 1, BOOL bMultiSelect = FALSE);

  enum{
    E_ItemExpanded = 100,
    E_ItemCollapsed = 101,
  };

  class CItem : public CListControl::CItem
  {
  public:
    enum EState{
      Visible = 0,
      Hidden = 1,

      Expanded = 0,
      Collapsed = 2,
    };

    virtual BOOL IsSelectable(){ return IsVisible(); }
    virtual BOOL IsVisible(){ return !(m_dwTreeState & Hidden); }
    virtual BOOL IsHidden(){ return (m_dwTreeState & Hidden); }
    virtual BOOL IsExpanded(){ return !(m_dwTreeState & Collapsed); }
    virtual BOOL IsCollapsed(){ return (m_dwTreeState & Collapsed); }
    virtual DWORD GetTreeState(){ return m_dwTreeState; }

    virtual void Draw(CDDrawSurface *pSurface, CRect *pRectBound);

    virtual CItem* GetParent(){ return m_pParent; }

    typedef CListControl::CItem CListItem;
    virtual DWORD GetHeight(){ if(IsHidden()) return 0; else return m_dwHeight; }

    CItem(){ m_dwTreeState = Hidden | Collapsed; m_pParent = NULL; m_dwNestedLevel = 0; }
    virtual ~CItem(){ }

  protected:
    // The item state in the tree control
    DWORD m_dwTreeState;
    // Number of level in which this item resides (root = 0)
    DWORD m_dwNestedLevel;

    // index of our parent item (if root = NULL)
    CItem *m_pParent;

    CTreeControl *GetControl(){ return (CTreeControl *)m_pControl; }

    friend class CTreeControl;
  };
  friend class CItem;

protected:
  // New item creation
  virtual CListControl::CItem *NewItem();

  virtual int FindItem(CListControl::CItem *pItem){ return CListControl::FindItem(pItem); }

  CTreeControlLayout *GetLayout(){ return (CTreeControlLayout *)m_pLayout; }

  void OnLButtonDown(CPoint point);
  void OnLButtonDblClk(CPoint pt);

  CItem *GetItem(int nItem){ if(nItem == -1) return NULL; else return (CTreeControl::CItem *)m_aItems[nItem]; }

private:
  // Our layout
  CTreeControlLayout *m_pLayout;
};

#endif // !defined(AFX_TREECONTROL_H__4D86CBF3_7F3C_11D4_B0F5_004F49068BD6__INCLUDED_)
