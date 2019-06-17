// PopupMenu.h: interface for the CPopupMenu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POPUPMENU_H__EE303F71_DB21_11D3_A8BA_00105ACA8325__INCLUDED_)
#define AFX_POPUPMENU_H__EE303F71_DB21_11D3_A8BA_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\WINDOW\FrameWindow.h"
#include "..\Buttons\TextButton.h"

class CPopupMenu : public CFrameWindow
{
  DECLARE_DYNAMIC(CPopupMenu);
  DECLARE_OBSERVER_MAP(CPopupMenu);

  // internal IDs and constants
  enum{
    _SELF_ID_ = 0x0FE000000,
    _MENU_ITEM_ID_ = 0x0FE000001,
    _MENU_SEPARATOR_ = 0x0FFFFFFFF,

    SeparatorHeight = 10
  };

  class CMenuItem : public CTextButton
  {
    DECLARE_OBSERVER_MAP(CMenuItem);

  public:
    // constructor & destructor
    CMenuItem();
    virtual ~CMenuItem();

    // creates the item
    void Create(CString &strText, DWORD dwID);
    // creates separator item
    void CreateSeparator();
    // deletes the item
    virtual void Delete();

    // creates the button (menu item)
    virtual void CreateButton(CPoint &pt, CPopupMenu *pParent, CObserver *pObserver);
    // deletes the button (menu item)
    virtual void DeleteButton();

    // returns the button size (used after default creation)
    virtual CSize GetButtonSize();
    // sets new button position
    virtual void SetButtonPosition(CRect &rcButton);

    virtual void OnRelease();
    // reaction on keyboard event
    BOOL OnKeyDown(DWORD dwKey, DWORD dwFlags);

  protected:
    // text and ID of the menuitem
    CString m_strText;
    DWORD m_dwID;

    // the parent menu
    CPopupMenu *m_pParentMenu;
  };

public:
	CPopupMenu();
	virtual ~CPopupMenu();

  // Create the menu
  // first call some AddItem or so to add some items to the menu
  // pt - top left point of the menu
  // pObserver - observer which will recieve all commands from the menu
  void Create(CPoint &pt, CObserver *pObserver);
  // Delete the menu, you don't have to call this (it will delete itself automaticaly)
  void Delete();

  // add item ( to the end of menu )
  // strText, dwID - name and Notifier ID for the item
  // returns index of the item in the menu
  DWORD AddItem(CString strText, DWORD dwID);
  DWORD AddItem(UINT nText, DWORD dwID);

  // add separator (small space)
  DWORD AddSeparator();

  // insert item
  // strText, dwID - as for AddItem
  // dwIndex - index to which insert (all items below will be shifted)
  DWORD InsertItem(DWORD dwIndex, CString strText, DWORD dwID);
  // delete item
  void DeleteItem(DWORD dwIndex);
  // delete all items
  void DeleteAll();

  // closes the menu
  void CloseMenu();


#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
  // array of menu items
  CTypedPtrArray<CPtrArray, CMenuItem *> m_aItems;

  // the window, which had the focus before us
  CWindow *m_pFocusWindow;

  // internal events
  enum{
    _E_CLOSE_MENU = 1
  };

  // delete the menu window
  void DeleteWindow();

  // reactions on internal events
  void OnCloseMenu();
  void OnItemRelease();

  // reactions on some window system notifications
  virtual void OnDeactivate();
  virtual void Draw(CDDrawSurface *pDDSurface, CRect *pBoundRect);
};

#endif // !defined(AFX_POPUPMENU_H__EE303F71_DB21_11D3_A8BA_00105ACA8325__INCLUDED_)
