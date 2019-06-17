// PopupMenu.cpp: implementation of the CPopupMenu class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PopupMenu.h"

#include "..\..\Keyboard.h"
#include "..\Layouts.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CPopupMenu, CFrameWindow);

BEGIN_OBSERVER_MAP(CPopupMenu, CFrameWindow)
  BEGIN_NOTIFIER(_SELF_ID_)
    EVENT(_E_CLOSE_MENU)
      OnCloseMenu(); return FALSE;
  END_NOTIFIER()
  BEGIN_NOTIFIER(_MENU_ITEM_ID_)
    ON_BUTTONRELEASED(OnItemRelease)
  END_NOTIFIER()
END_OBSERVER_MAP(CPopupMenu, CFrameWindow)

CPopupMenu::CPopupMenu()
{
}

CPopupMenu::~CPopupMenu()
{
}


#ifdef _DEBUG

void CPopupMenu::AssertValid() const
{
  CFrameWindow::AssertValid();
}

void CPopupMenu::Dump(CDumpContext &dc) const
{
  CFrameWindow::Dump(dc);
}

#endif

// CPopupMenu::CMenuItem implementation

BEGIN_OBSERVER_MAP(CPopupMenu::CMenuItem, CTextButton)
  BEGIN_KEYBOARD()
    ON_KEYDOWN()
  END_KEYBOARD()
END_OBSERVER_MAP(CPopupMenu::CMenuItem, CTextButton)

// constructor
CPopupMenu::CMenuItem::CMenuItem()
{
  m_dwID = 0;
  m_pParentMenu = NULL;
}

// destructor
CPopupMenu::CMenuItem::~CMenuItem()
{
  Delete();
}

// create function
void CPopupMenu::CMenuItem::Create(CString &strText, DWORD dwID)
{
  // copy the item info
  m_strText = strText;
  m_dwID = dwID;
}

void CPopupMenu::CMenuItem::CreateSeparator()
{
  m_dwID = CPopupMenu::_MENU_SEPARATOR_;
}

// delete function
void CPopupMenu::CMenuItem::Delete()
{
  DeleteButton();
}

// create instance (the button)
void CPopupMenu::CMenuItem::CreateButton(CPoint &pt, CPopupMenu *pParent, CObserver *pObserver)
{
  ASSERT_VALID(pParent);

  m_pParentMenu = pParent;

  if(m_dwID == CPopupMenu::_MENU_SEPARATOR_){
    // do nothing
  }
  else{
    // create the new button and connect it to the observer
    CTextButton::Create(pt, NULL, m_strText, pParent);
    Connect(pObserver, m_dwID);
  }
}

// delete instance (the button)
void CPopupMenu::CMenuItem::DeleteButton()
{
  m_pParentMenu = NULL;

  if(m_dwID != CPopupMenu::_MENU_SEPARATOR_)
    CTextButton::Delete();
}

void CPopupMenu::CMenuItem::OnRelease()
{
  CTextButton::OnRelease();

  if(m_pParentMenu != NULL)
    m_pParentMenu->CloseMenu();
}

// reaction on keyboard event
BOOL CPopupMenu::CMenuItem::OnKeyDown(DWORD dwKey, DWORD dwFlags)
{
  switch(dwKey){
  case VK_ESCAPE: // escape
    if(m_pParentMenu != NULL)
      m_pParentMenu->CloseMenu();
    return TRUE;
  case VK_DOWN:
    SetFocusToNext();
    return TRUE;
  case VK_UP:
    SetFocusToPrev();
  }
  return CTextButton::OnKeyDown(dwKey, dwFlags);
}

// get the button size (the window size)
CSize CPopupMenu::CMenuItem::GetButtonSize()
{
  if(m_dwID == CPopupMenu::_MENU_SEPARATOR_){
    return CSize(1, CPopupMenu::SeparatorHeight);
  }
  return GetVirtualSize();
}

// set the button position
void CPopupMenu::CMenuItem::SetButtonPosition(CRect &rcButton)
{
  CRect rc(rcButton);
  rc.bottom = rc.top + GetWindowPosition().Height();
  if(m_dwID != CPopupMenu::_MENU_SEPARATOR_){
    SetWindowPosition(&rc);

    CTextButton::OnSize(rc.Size());
  }
}


// main CPopupMenu implementation

// create function
void CPopupMenu::Create(CPoint &pt, CObserver *pObserver)
{
  ASSERT_VALID(pObserver);
  ASSERT_VALID(g_pDesktopWindow);
  
  // get the focus window
  m_pFocusWindow = GetFocusWindow();

  // first just create the menu window
  // sizes will be determined later
  CRect rcWindow(pt.x, pt.y, pt.x + 1, pt.y + 1);
  CFrameWindow::Create(rcWindow, CLayouts::m_pDefaults->GetPopupMenuLayout(), g_pDesktopWindow, FALSE);

  // if there are no items -> do nothing
  if(m_aItems.GetSize() == 0) return;

  // create all items and get the maximum size
  CSize sizeMax(0, 0), sizeBut(0, 0);
  CPoint ptBut;
  int i;
  for(i = 0; i < m_aItems.GetSize(); i++){
    m_aItems[i]->CreateButton(ptBut, this, pObserver);
    sizeBut = m_aItems[i]->GetButtonSize();

    if(sizeBut.cx > sizeMax.cx) sizeMax.cx = sizeBut.cx;
    if(sizeBut.cy > sizeMax.cy) sizeMax.cy = sizeBut.cy;
  }

  CRect rcButton(m_pLayout->m_nEdgeLeft, 0, m_pLayout->m_nEdgeLeft + sizeMax.cx, 0);
  int nHeight = 0;
  // set correct positions for the items
  for(i = 0; i < m_aItems.GetSize(); i++){
    sizeBut = m_aItems[i]->GetButtonSize();
    rcButton.top = nHeight + m_pLayout->m_nEdgeTop;
    rcButton.bottom = rcButton.top + sizeBut.cy;
    m_aItems[i]->SetButtonPosition(rcButton);

    nHeight += sizeBut.cy;
  }

  // now set the correct size of the menu window
  rcWindow.left = pt.x; rcWindow.top = pt.y;
  rcWindow.right = pt.x + sizeMax.cx + m_pLayout->m_nEdgeLeft + m_pLayout->m_nEdgeRight;
  rcWindow.bottom = pt.y + nHeight + m_pLayout->m_nEdgeTop + m_pLayout->m_nEdgeBottom;

  // move the window to be visible whole
  {
    CSize sz = rcWindow.Size();
    CRect rcDesktop = g_pDesktopWindow->GetWindowPosition();
    if(rcWindow.left < rcDesktop.left){ rcWindow.left = rcDesktop.left; rcWindow.right = rcWindow.left + sz.cx; }
    if(rcWindow.right > rcDesktop.right){ rcWindow.right = rcDesktop.right; rcWindow.left = rcWindow.right - sz.cx; }
    if(rcWindow.top < rcDesktop.top){ rcWindow.top = rcDesktop.top; rcWindow.bottom = rcWindow.top + sz.cy; }
    if(rcWindow.bottom > rcDesktop.bottom){ rcWindow.bottom = rcDesktop.bottom; rcWindow.top = rcWindow.bottom - sz.cy; }
  }

  SetWindowPosition(&rcWindow);

  // connect us to us
  Connect(this, _SELF_ID_);

  // set us active
  Activate();
}

// delete function
void CPopupMenu::Delete()
{
  DeleteWindow();
  DeleteAll();
}

DWORD CPopupMenu::AddItem(UINT nText, DWORD dwID)
{
  CString str;

  str.LoadString(nText);
  return AddItem(str, dwID);
}

// add item to the end of menu
DWORD CPopupMenu::AddItem(CString strText, DWORD dwID)
{
  int nIndex;
  CMenuItem *pItem;

  pItem = new CMenuItem();
  pItem->Create(strText, dwID);

  nIndex = m_aItems.Add(pItem);

  return (DWORD)nIndex;
}

DWORD CPopupMenu::AddSeparator()
{
  int nIndex;
  CMenuItem *pItem;

  pItem = new CMenuItem();
  pItem->CreateSeparator();

  nIndex = m_aItems.Add(pItem);

  return (DWORD)nIndex;
}

// insert item
DWORD CPopupMenu::InsertItem(DWORD dwIndex, CString strText, DWORD dwID)
{
  ASSERT(dwIndex <= (DWORD)(m_aItems.GetSize()));
  CMenuItem *pItem;

  pItem = new CMenuItem();
  pItem->Create(strText, dwID);

  m_aItems.InsertAt(dwIndex, pItem);

  return dwIndex;
}

// delete item
void CPopupMenu::DeleteItem(DWORD dwIndex)
{
  ASSERT(dwIndex < (DWORD)(m_aItems.GetSize()));
  CMenuItem *pItem;

  pItem = m_aItems[dwIndex];
  ASSERT(pItem != NULL);

  pItem->Delete();
  delete pItem;

  m_aItems.RemoveAt(dwIndex);
}

// delete all items
void CPopupMenu::DeleteAll()
{
  int i;
  CMenuItem *pItem;

  for(i = 0; i < m_aItems.GetSize(); i++){
    pItem = m_aItems[i];
    ASSERT(pItem != NULL);

    pItem->Delete();
    delete pItem;
  }
  m_aItems.RemoveAll();
}

// used to delete the menu window (internaly)
void CPopupMenu::DeleteWindow()
{
  // disconnect us from us
  Disconnect(this);

  // delete all buttons (items)
  int i;
  for(i = 0; i < m_aItems.GetSize(); i++){
    m_aItems[i]->DeleteButton();
  }

  // delete the window
  CFrameWindow::Delete();
}

// draw the window
void CPopupMenu::Draw(CDDrawSurface *pDDSurface, CRect *pBoundRect)
{
  CFrameWindow::Draw(pDDSurface, pBoundRect);
}

// reaction on the deactivation
// -> close the menu
void CPopupMenu::OnDeactivate()
{
  if(m_pFocusWindow != NULL) m_pFocusWindow->SetFocus();
  InlayEvent(_E_CLOSE_MENU, 0, this);
  CFrameWindow::OnDeactivate();
}

// reaction on close menu event
void CPopupMenu::OnCloseMenu()
{
  DeleteWindow();
}

void CPopupMenu::OnItemRelease()
{
  if(m_pFocusWindow != NULL) m_pFocusWindow->SetFocus();
  DeleteWindow();
}

void CPopupMenu::CloseMenu()
{
  if(m_pFocusWindow != NULL) m_pFocusWindow->SetFocus();
  InlayEvent(_E_CLOSE_MENU, 0, this);
}