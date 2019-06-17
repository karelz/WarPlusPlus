// MapViewToolbar.cpp: implementation of the CMapViewToolbar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\gameclient.h"
#include "MapViewToolbar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CMapViewToolbar, CWindow);

BEGIN_OBSERVER_MAP(CMapViewToolbar, CWindow)
  case -1:
    break;
END_OBSERVER_MAP(CMapViewToolbar, CWindow)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMapViewToolbar::CMapViewToolbar()
{
  m_eState = State_None;
}

CMapViewToolbar::~CMapViewToolbar()
{
}

// Debug functions
#ifdef _DEBUG

void CMapViewToolbar::AssertValid() const
{
  CWindow::AssertValid();
}

void CMapViewToolbar::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);
}

#endif

// Creates the toolbar
void CMapViewToolbar::Create(CWindow *pParent, CDataArchive MapViewArchive, CObserver *pObserver)
{
  ASSERT(pParent != NULL);
  ASSERT(pObserver != NULL);

  // Create the background image
  VERIFY(m_Background.Create(MapViewArchive.CreateFile("Toolbar\\Background.tga")));

  VERIFY(m_HourGlass.Create(MapViewArchive.CreateFile("Toolbar\\HourGlass.tga")));
  VERIFY(m_HourGlassWarning.Create(MapViewArchive.CreateFile("Toolbar\\HourGlassWarning.tga")));
  VERIFY(m_Question.Create(MapViewArchive.CreateFile("Toolbar\\Question.tga")));

  CSize sizeWindow;
  // Create the window itself
  {
    CRect rcParent = pParent->GetWindowPosition();
    CRect rc(rcParent.right - m_Background.GetAllRect()->Width(),
      rcParent.bottom - m_Background.GetAllRect()->Height(),
      rcParent.right, rcParent.bottom);

    m_bDoubleClk = FALSE;
    m_bTransparent = TRUE;

    VERIFY(CWindow::Create(&rc, pParent));

    sizeWindow = rc.Size();
  }

  {
    // Create the Script editor button
    m_ScriptEditorLayout.Create(MapViewArchive.CreateFile("Toolbar\\ScriptEditor.button"));
    m_wndScriptEditor.Create(CPoint(sizeWindow.cx - 2 * 17, sizeWindow.cy - 17), &m_ScriptEditorLayout, this, FALSE, pParent);
    m_wndScriptEditor.SetToolTip(IDS_MAPVIEW_SCRIPTEDITOR_TT);
    m_wndScriptEditor.Connect(pObserver, IDC_SCRIPTEDITOR);

    // Create the Minimap button
    m_MinimapLayout.Create(MapViewArchive.CreateFile("Toolbar\\Minimap.button"));
    m_wndMinimap.Create(CPoint(sizeWindow.cx - 3 * 17, sizeWindow.cy - 17), &m_MinimapLayout, this, FALSE, pParent);
    m_wndMinimap.SetToolTip(IDS_MAPVIEW_MINIMAP_TT);
    m_wndMinimap.Connect(pObserver, IDC_MINIMAP);

    // Create the Selection window button
    m_SelectionLayout.Create(MapViewArchive.CreateFile("Toolbar\\Selection.button"));
    m_wndSelection.Create(CPoint(sizeWindow.cx - 4 * 17, sizeWindow.cy - 17), &m_SelectionLayout, this, FALSE, pParent);
    m_wndSelection.SetToolTip(IDS_MAPVIEW_SELECTION_TT);
    m_wndSelection.Connect(pObserver, IDC_SELECTION);
  }
}

// Deletes the toolbar
void CMapViewToolbar::Delete()
{
  // Delete the selection button
  m_wndSelection.Delete();
  m_SelectionLayout.Delete();

  // Delete the minimap button
  m_wndMinimap.Delete();
  m_MinimapLayout.Delete();

  // Delete the script editor button
  m_wndScriptEditor.Delete();
  m_ScriptEditorLayout.Delete();

  // Delete the window itself
  CWindow::Delete();

  m_HourGlass.Delete();
  m_HourGlassWarning.Delete();
  m_Question.Delete();

  // Delete the background image
  m_Background.Delete();
}

// Draws the window
void CMapViewToolbar::Draw(CDDrawSurface *pDDSurface, CRect *pBoundRect)
{
  CRect rcWindow = GetWindowPosition();

  pDDSurface->Paste(0, 0, &m_Background);

  switch(m_eState){
  case State_None:
    break;
  case State_HourGlass:
    pDDSurface->Paste(rcWindow.Width() - 17, rcWindow.Height() - 17, &m_HourGlass);
    break;
  case State_HourGlassWarning:
    pDDSurface->Paste(rcWindow.Width() - 17, rcWindow.Height() - 17, &m_HourGlassWarning);
    break;
  case State_Question:
    pDDSurface->Paste(rcWindow.Width() - 17, rcWindow.Height() - 17, &m_Question);
    break;
  }
}

void CMapViewToolbar::DisablePartial()
{
  m_wndScriptEditor.EnableWindow(FALSE);
}

void CMapViewToolbar::EnableAll()
{
  m_wndScriptEditor.EnableWindow();
}