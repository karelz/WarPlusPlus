// ListControl.h: interface for the CListControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LISTCONTROL_H__22FDD4E8_5081_11D3_A02E_DD87D6C1E031__INCLUDED_)
#define AFX_LISTCONTROL_H__22FDD4E8_5081_11D3_A02E_DD87D6C1E031__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\Window.h"
#include "ListControlLayout.h"
#include "ListControlScroll.h"

class CListControl : public CFrameWindow  
{
  DECLARE_DYNAMIC(CListControl)
  DECLARE_OBSERVER_MAP(CListControl)

public:
  // Returns the index of the first visible item
  virtual int GetFirstVisible () { return m_nFirstVisible; }
  // Sets the first visible item
  virtual void SetFirstVisible ( int nFirstVisible ) { m_nFirstVisible = nFirstVisible;
    UpdateVisibility(); UpdateScroll(); UpdateRect(); }

  // Sets new windows position of the listbox
  // Use this function if you're changing the size of the listbox
  virtual void SetWindowPos(CRect *pRect);
  // returns first selected item index
  // if none returns -1
	virtual int GetSelectedItem();
  // returns next selected item (if none -> -1)
  virtual int GetNextSelectedItem(int nItem);
  // returns number of selected items
	virtual int GetSelectedCount();
  // sets the state of item
	virtual void SetItemSelected(int nIndex, BOOL bSelected);
  // returns TRUE if the item is selected
	virtual BOOL IsItemSelected(int nIndex);
  // returns index of item which has data value
  // returns the first one
  // returns -1 if not found
	virtual int FindItemData(DWORD dwData);
  //r eturns number of items in the list box
	virtual int GetItemsCount();
  // sets item text
	virtual void SetItemText(int nIndex, CString strText);
  // draws the window
	virtual void Draw(CDDrawSurface *pSurface, CRect *pRectBound);
  // sets item data
  void SetItemData(int nIndex, DWORD dwData){
    ASSERT_VALID(this); ASSERT((nIndex >= 0) && (nIndex < m_aItems.GetSize()));
    m_aItems[nIndex]->SetData(dwData);
  }
  // returns item data at given index
  DWORD GetItemData(int nIndex){
    ASSERT_VALID(this); ASSERT((nIndex >= 0) && (nIndex < m_aItems.GetSize()));
    return m_aItems[nIndex]->GetData();
  }
  // returns item text at given index
  virtual CString GetItemText(int nIndex){
    ASSERT_VALID(this); ASSERT((nIndex >= 0) && (nIndex < m_aItems.GetSize()));
    return m_aItems[nIndex]->GetText();
  }

  // sets column width
  virtual void SetColumnWidth(DWORD dwColumn, DWORD dwWidth){
    ASSERT_VALID(this); ASSERT(dwColumn < m_dwColumns);
    m_aColumnWidths[dwColumn] = dwWidth;
    UpdateRect();
  }
  // returns the column width
  virtual DWORD GetColumnWidth(DWORD dwColumn){
    ASSERT_VALID(this); ASSERT(dwColumn < m_dwColumns);
    return m_aColumnWidths[dwColumn];
  }

  // sets subitem text
  virtual void SetItemText(int nIndex, DWORD dwSubitem, CString strText);
  virtual CString  GetItemText(int nIndex, DWORD dwSubitem){
    ASSERT_VALID(this); ASSERT((nIndex >= 0) && (nIndex < m_aItems.GetSize()));
	ASSERT(dwSubitem < m_dwColumns - 1);
    return m_aItems[nIndex]->GetText(dwSubitem);
  }
  // inserts new item at given position
	virtual int InsertItem(int nIndex, CString strText);
  // deletes one item
	virtual void DeleteItem(int nIndex);
  // removes all items
  virtual void RemoveAll();
  // adds new item to the end of the list
	virtual int AddItem(CString strText);
  // deletes the object
	virtual void Delete();
  // creates the list control
	BOOL Create(CRect &rcBound, CListControlLayout *pLayout, CWindow *pParent, DWORD dwColumns = 1, BOOL bMultiSelect = FALSE);

  // constructor
	CListControl();
  // destructor
	virtual ~CListControl();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // one item in the list control
  class CItem{
  public:
    // Is the item selectable
    virtual BOOL IsSelectable(){ return TRUE; }
    // return TRUE if selected
	  virtual BOOL IsSelected();
    // selects this item
	  virtual void SetSelected(BOOL bSelected);
    // returns TRUE if has focus
	  virtual BOOL HasFocus();
    // sets the focus to this item
	  virtual void SetFocus(BOOL bFocus);
    // draws item
	  virtual void Draw(CDDrawSurface *pSurface, CRect *pRectBound);

    // draws the background of the item
    virtual void DrawBackground(CDDrawSurface *pSurface, CDDrawSurface *pBkg, CRect *pRectBound);

    // returns text color to be draw in current state
    // and surface to be used as background for the item
    virtual DWORD GetTextColor(CDDrawSurface **pBkg);

    enum EState{
      NonSelected = 0,   // non selected item (normal)
      Selected = 1,      // selected (but there is no focus)
      Focus = 2,
      SelectedFocus = 3  // selected and has the focus
    };

    // returns the height of the item
    virtual DWORD GetHeight(){ return m_dwHeight; }
    // sets and returns the item data
    virtual void SetData(DWORD dwData){ m_dwData = dwData; }
    virtual DWORD GetData(){ return m_dwData; }

    // returns the item text
    virtual CString GetText(){ return m_strText; }
    // returns the subitem text
    virtual CString GetText(DWORD dwSubItem){ ASSERT(dwSubItem < m_pControl->m_dwColumns - 1); return m_aSubItems[dwSubItem]; }
    // sets the text value for this item
	  virtual void SetText(CString strText);
    // sets the subitem text
    virtual void SetText(DWORD dwSubItem, CString strText);
    // sets new state of the item
	  virtual void SetState(EState eState);
    // returns the state of the item
	  virtual EState GetState();
    // deletes the object
	  virtual void Delete();
    // creates the object
	  void Create(CListControl *pControl);
    // constructor
    CItem();
    // destructor
    virtual ~CItem();

  protected:
    // text of the item
    CString m_strText;
    // text of subitem
    CStringArray m_aSubItems;
    // the user defined data
    DWORD m_dwData;
    // the control object of the owner
    CListControl *m_pControl;
    // the state of the item
    EState m_eState;
    // height of the item in pixels
    DWORD m_dwHeight;
  };

  enum Events{
    E_SELCHANGE = 1,  // selection has changed
    E_SELECTED = 2,    // some item was selected (doubleclicked)
    E_ItemSelected = 10,  // dwParam - the item
    E_ItemDeselected = 11, // dwParam - the item
  };

protected:
  // clips the point into the inner rect of the list
  // returns the index of item under the point
  // if there is no item (free space) returns -1
  // -1 can be returned only if its below the last item
  int ItemFromPoint(CPoint point);

  virtual CItem *NewItem();
  int NextSelectable(int nPosition);
  int PrevSelectable(int nPosition);

  // reactions on mouse events
	void OnMouseMove(CPoint point);
	void OnLButtonUp(CPoint point);
	void OnLButtonDown(CPoint point);
	void OnLButtonDblClk(CPoint point);

  // selects all items between two indeces
	void ShiftSelection(int nPrevPos, int nNewPos);
  // deselects item
	void DeselectItem(int nPosition);
  // selects item
	void SelectItem(int nPosition);
  // deselects all items
	void ClearSelection();
  // scrolls the list to the position (make it visible) (no redraw)
	void ScrollToPosition(int nPosition);

  // reactions on keyboard events
	BOOL OnKeyUp(UINT nChar, DWORD dwFlags);
	BOOL OnKeyDown(UINT nChar, DWORD dwFlags);

  // sets tcaret position (no redraw)
	virtual void SetCaretPosition(int nPosition);

  // reactions on notifs from scroll bar
	void OnPosition(int nNewPosition);
	void OnPageDown();
	void OnPageUp();
	void OnStepDown();
	void OnStepUp();

  // reaction on size changes
	virtual void OnSize ( CSize size );

  // updates the last visible value
	virtual void UpdateVisibility();
	// returns the index of item from pointer to it
  int FindItem(CItem *pItem);
  // updates the item graphics
	virtual void UpdateItem(CItem *pItem);
	virtual void UpdateItem(int nIndex);

  enum{ ScrollID = 0x01 }; // ID of the scroll notifier

  // reaction on the scroll activation -> activates the list control
	virtual void OnScrollActivated();
  // the scroll bar object
	CListControlScroll m_Scroll;
  // The horizontal scroll bar
  CListControlScroll m_HorizontalScroll;

  // reactions on events
	void OnAnimsRepaint(CAnimationInstance *pAnim);

  // reactions on notifications
	virtual void OnLoseFocus();
	virtual void OnSetFocus();
	virtual BOOL OnActivate();


  // TRUE if multi selection is enabled
  BOOL m_bMultiSelect;
  // number of selected items
  int m_nSelectedItems;
  // position of the focused item (index)
	int m_nCaretPosition;
  // first visible item (index)
	int m_nFirstVisible;
  // last visible item (index)
  int m_nLastVisible;
  // updates the scroll bar
	virtual void UpdateScroll();

  // the items array
  CTypedPtrArray<CPtrArray, CItem *> m_aItems;

  // animations
  CAnimationInstance m_Selection;
  CAnimationInstance m_Focus;
  CAnimationInstance m_SelectionFocus;

  // number of columns
  DWORD m_dwColumns;
  DWORD *m_aColumnWidths;
    
  // the layout object
	CListControlLayout * m_pLayout;

private:
	BOOL m_bMouseDrag;

  friend CListControl::CItem;
};

#endif // !defined(AFX_LISTCONTROL_H__22FDD4E8_5081_11D3_A02E_DD87D6C1E031__INCLUDED_)
