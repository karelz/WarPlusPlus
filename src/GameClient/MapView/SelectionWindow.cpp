// SelectionWindow.cpp: implementation of the CSelectionWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SelectionWindow.h"

#include "..\GameClientGlobal.h"
#include "..\DataObjects\CMap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define SELECTIONWINDOW_BORDER_LEFT 6
#define SELECTIONWINDOW_BORDER_RIGHT 8
#define SELECTIONWINDOW_BORDER_TOP 15
#define SELECTIONWINDOW_BORDER_BOTTOM 5

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CSelectionWindow, CFrameWindow);

BEGIN_OBSERVER_MAP(CSelectionWindow, CFrameWindow)
  BEGIN_NOTIFIER(ID_Selection)
    EVENT(CUnitSelection::E_SelectionChanged)
      FillList(); return FALSE;
  END_NOTIFIER()

  THECOMMAND(ID_CloseButton, OnClose)
END_OBSERVER_MAP(CSelectionWindow, CFrameWindow)

// Constructor
CSelectionWindow::CSelectionWindow()
{
  m_pSelection = NULL;
  m_pDesktopWindow = NULL;
}

// Destructor
CSelectionWindow::~CSelectionWindow()
{
  ASSERT(m_pSelection == NULL);
}

// Debug functions
#ifdef _DEBUG

void CSelectionWindow::AssertValid() const
{
  CFrameWindow::AssertValid();
  ASSERT(m_pSelection != NULL);
}

void CSelectionWindow::Dump(CDumpContext &dc) const
{
  CSelectionWindow::Dump(dc);
}

#endif


// Creates the window
void CSelectionWindow::Create(CRect &rcRect, CWindow *pParent, CUnitSelection *pSelection,
                              CDataArchive MapViewArchive, CWindow *pDesktopWindow)
{
  ASSERT(pParent != NULL);
  ASSERT(pSelection != NULL);

  // Copy the desktop window
  m_pDesktopWindow = pDesktopWindow;

  // Copy pointer to selection
  m_pSelection = pSelection;
  // And connect us to it
  m_pSelection->Connect(this, ID_Selection);

  m_Layout.Create(MapViewArchive.CreateFile("Selection\\Frame.window"));
  // Create the window itself
  VERIFY(CFrameWindow::Create(rcRect, &m_Layout, pParent, TRUE));
  m_bTransparent = FALSE;
  SetBuffering();
  StopBufferingOnSize();

  // Create the layout for the list
  m_ListLayout.Create(MapViewArchive.CreateFile("Selection\\List.listctrl"));
  // Create the layout of the close button
  m_CloseButtonLayout.Create(MapViewArchive.CreateFile("Selection\\Close.button"));

  // Create the list control
  CRect rcList;
  GetListRect(&rcList);
  m_wndList.Create(rcList, &m_ListLayout, this, 1, FALSE);

  // Create the close button
  {
    m_wndCloseButton.SetTopMost();
    m_wndCloseButton.Create(CPoint(GetClientRect()->Width() - 23, 0), &m_CloseButtonLayout,
      this, FALSE, pDesktopWindow);
    m_wndCloseButton.Connect(this, ID_CloseButton);
  }
}

// Deletes the window
void CSelectionWindow::Delete()
{
  // Disconnect from the selection
  if(m_pSelection != NULL){
    m_pSelection->Disconnect(this);
    m_pSelection = NULL;
  }

  // Delete the close button list
  m_wndCloseButton.Delete();
  m_CloseButtonLayout.Delete();

  // Delete the list
  m_wndList.Delete();
  m_ListLayout.Delete();

  m_Layout.Delete();

  // Delete the window itself
  CFrameWindow::Delete();
}
// returns the rectangle of the list control in the window
void CSelectionWindow::GetListRect(CRect *pRect)
{
  CRect rcWnd;
  rcWnd = GetWindowPosition();

  pRect->left = SELECTIONWINDOW_BORDER_LEFT;
  pRect->top = SELECTIONWINDOW_BORDER_TOP;
  pRect->right = rcWnd.Width() - SELECTIONWINDOW_BORDER_RIGHT;
  pRect->bottom = rcWnd.Height() - SELECTIONWINDOW_BORDER_BOTTOM;
}

// Reaction on size change
void CSelectionWindow::OnSize(CSize size)
{
  CFrameWindow::OnSize(size);

  CRect rcList;
  GetListRect(&rcList);
  m_wndList.SetWindowPos(&rcList);

  CPoint pt(GetClientRect()->Width() - 23, 0);
  m_wndCloseButton.SetWindowPosition(&pt);
}

// On close window
void CSelectionWindow::OnClose()
{
  InlayEvent(E_Close, 0);
}

// Fills the list from the selection
void CSelectionWindow::FillList()
{
  // First clear the list
  m_wndList.RemoveAll();

  // Lock the selection
  m_pSelection->Lock();

  // Go through the selection and add the units
  CUnitSelection::SUnitNode *pNode = m_pSelection->GetFirstNode();
  CCCivilization *pCivilization;
  CCGeneralUnitType *pUnitType;
  CString str;
  int nItem, i = 0;
  while(pNode != NULL){
    // Get the civilization
    pCivilization = g_pMap->GetCivilization(pNode->m_dwCivilizationID);
    // Get the unit type
    pUnitType = g_pMap->GetGeneralUnitType(pNode->m_dwUnitTypeID);

    // Format the item string
    if(pCivilization == g_pCivilization){
      str = pUnitType->GetName();
    }
    else{
      str.Format("%s [%s]", pUnitType->GetName(), pCivilization->GetName());
    }

    // Insert the item
    nItem = m_wndList.InsertItem(i++, str);
    m_wndList.SetItemData(nItem, pNode->m_dwUnitID);

    // Go to the next selected unit
    pNode = pNode->m_pNext;
  }

  // Unlock the selection
  m_pSelection->Unlock();
}

void CSelectionWindow::DisablePartial()
{
  m_wndList.EnableWindow(FALSE);
}

void CSelectionWindow::EnableAll()
{
  m_wndList.EnableWindow();
}