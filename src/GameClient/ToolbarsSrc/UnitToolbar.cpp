// UnitToolbar.cpp: implementation of the CUnitToolbar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\Gameclient.h"
#include "UnitToolbar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define UT_EMPTY_WIDTH 110
#define UT_EMPTY_HEIGHT 40

#define UT_BORDER_LEFT   10
#define UT_BORDER_TOP    5
#define UT_BORDER_RIGHT  5
#define UT_BORDER_BOTTOM 5

#define UT_SEPARATOR_SIZE 5

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CUnitToolbar, CFrameWindow);

BEGIN_OBSERVER_MAP(CUnitToolbar, CFrameWindow)
  case -1:
    break;
  default:
    if((dwEvent == CButtonObject::E_RELEASED) && 
      (dwNotID >= ID_Buttons) && (dwNotID < ID_Buttons + 200)){
      OnButton(dwNotID - ID_Buttons);
      return FALSE;
    }
    break;
END_OBSERVER_MAP(CUnitToolbar, CFrameWindow)

CUnitToolbar::CUnitToolbar()
{
  // empty the toolbar info (no unit selected...)
  m_pToolbarInfo = NULL;
  m_aButtons = NULL;
  m_dwButtonCount = 0;
  m_pDesktopWindow = NULL;

  m_bSizing = FALSE;

  m_bLine = TRUE;
  m_dwFixedNumber = 1;
}

CUnitToolbar::~CUnitToolbar()
{
  ASSERT(m_pToolbarInfo == NULL);
  ASSERT(m_aButtons == NULL);
  ASSERT(m_dwButtonCount == 0);
}

#ifdef _DEBUG

void CUnitToolbar::AssertValid() const
{
  CFrameWindow::AssertValid();
}

void CUnitToolbar::Dump(CDumpContext &dc) const
{
  CFrameWindow::Dump(dc);
}

#endif


// Creation
// Creates the window (toolbar) empty
void CUnitToolbar::Create(CRect &rcWindow, CWindow *pParent, CDataArchive MapViewArchive, CWindow *pDesktopWindow)
{
  // Copy the desktop window
  m_pDesktopWindow = pDesktopWindow;
  
  // Create layout for this window
  m_Layout.Create(MapViewArchive.CreateFile("UnitToolbar\\Frame.window"));
  // Create the layout for buttons
  m_ButtonLayout.Create(MapViewArchive.CreateFile("UnitToolbar\\ToolbarButton.button"));
  // Create the default icon
  m_DefaultButtonIcon.Create(MapViewArchive.CreateFile("UnitToolbar\\DefaultIcon.tga"));

  // Create the frame window
  VERIFY(CFrameWindow::Create(rcWindow, &m_Layout, pParent, TRUE));
  m_bTransparent = FALSE;
  SetBuffering();
  StopBufferingOnSize();

  // Create the empty text control
  {
    CRect rcBound(10, 10, 100, 35);
    CString strText;
    strText.LoadString(IDS_UNITTOOLBAR_EMPTY);
    m_EmptyText.Create(rcBound, NULL, strText, this);
  }

  // Create one button to determine its size
  {
    CIconButton but;
    but.Create(CPoint(0, 0), &m_DefaultButtonIcon, &m_ButtonLayout, this, FALSE, m_pDesktopWindow);
    CRect rc;
    rc = but.GetWindowPosition();
    m_szButton = rc.Size();
    but.Delete();
  }
    
  // Set it empty
  SetEmpty();
}

// Deletes the toolbar
void CUnitToolbar::Delete()
{
  // Clear it (this deletes any buttons)
  Clear();

  // Forget the toolbar info
  m_pToolbarInfo = NULL;
  // Delete the empty text control
  m_EmptyText.Delete();

  m_Layout.Delete();
  m_ButtonLayout.Delete();
  m_DefaultButtonIcon.Delete();

  // Delete the window
  CFrameWindow::Delete();

  m_pDesktopWindow = NULL;
}


// Sets the unit toolbar to empty (no unit)
void CUnitToolbar::SetEmpty()
{
  // First clear all controls
  Clear();

  // Show the empty text
  m_EmptyText.ShowWindow();
  // set empty size
  CRect rcWnd = GetWindowPosition();
  rcWnd.right = rcWnd.left + UT_EMPTY_WIDTH;
  rcWnd.bottom = rcWnd.top + UT_EMPTY_HEIGHT;
  SetWindowPosition(&rcWnd);
}

// Clears all controls on the toolbar
void CUnitToolbar::Clear()
{
  // Hide the empty text control
  m_EmptyText.HideWindow();

  // Delete any button objects
  {
    DWORD dwButton;
    for(dwButton = 0; dwButton < m_dwButtonCount; dwButton++){
      if(m_aButtons[dwButton] != NULL){
        m_aButtons[dwButton]->Delete();
        delete m_aButtons[dwButton];
        m_aButtons[dwButton] = 0;
      }
    }

    if(m_aButtons != NULL){
      delete m_aButtons;
      m_aButtons = NULL;
    }

    m_dwButtonCount = 0;
  }

}


// Sets new toolbar info
void CUnitToolbar::SetToolbarInfo(CToolbarInfo *pToolbarInfo)
{
  // if it was the same as the new one
  if(m_pToolbarInfo == pToolbarInfo) return;

  // It's a new one
  // Copy it
  m_pToolbarInfo = pToolbarInfo;

  if(m_pToolbarInfo == NULL){
    SetEmpty();
    return;
  }

  // Create buttons
  {
    // First free the old ones
    Clear();

    // allocate new array
    m_dwButtonCount = m_pToolbarInfo->GetButtonCount();
    m_aButtons = (CIconButton **)new BYTE[sizeof(CImageButton *) * m_dwButtonCount];

    // Create the buttons
    DWORD dwButton;
    CToolbarButtonInfo *pInfo;
    for(dwButton = 0; dwButton < m_dwButtonCount; dwButton++){
      // Get the button info
      pInfo = m_pToolbarInfo->GetButtonInfo(dwButton);
      if(pInfo->IsSeparator()){
        // If separator -> leave it empty
        m_aButtons[dwButton] = NULL;
        continue;
      }

      // normal button
      // create new button
      m_aButtons[dwButton] = new CIconButton();

      // create it
      {
        CAnimation *pAnim;
        if(pInfo->GetIcon() == NULL)
          pAnim = &m_DefaultButtonIcon;
        else
          pAnim = pInfo->GetIcon();

		    m_aButtons[dwButton]->HideWindow ();
        VERIFY(m_aButtons[dwButton]->Create(CPoint(0, 0), pAnim, &m_ButtonLayout, this, FALSE, m_pDesktopWindow));
        m_aButtons[dwButton]->SetToolTip(pInfo->GetCaption());
        // Set accelerators
        if ( pInfo->GetKeyAccChar () != 0 )
        {
          m_aButtons[dwButton]->SetKeyAcc ( pInfo->GetKeyAccChar (), pInfo->GetKeyAccFlags () );
        }
        m_aButtons[dwButton]->Connect(this, ID_Buttons + dwButton);
      }
    }
  }

  CSize szWindow;

  // Now place buttons
  m_szOneLine = ComputeOneLineSize();

  // Determine the ideal number of fixed lines
  szWindow = ComputeWindowSize(m_dwFixedNumber);
  while((CompareWithSquare(szWindow) <= 0) && m_dwFixedNumber > 1){
    m_dwFixedNumber--;
    szWindow = ComputeWindowSize(m_dwFixedNumber);
  }

  // Compute min and max
  {
    CSize sz;
    if(m_bLine){
      m_dwMin = szWindow.cx;
      if(m_dwFixedNumber > 1){
        sz = ComputeWindowSize(m_dwFixedNumber - 1);
        m_dwMax = sz.cx;
      }
      else{
        m_dwMax = 0x0100000;
      }
    }
    else{
      m_dwMin = szWindow.cy;
      if(m_dwFixedNumber > 1){
        sz = ComputeWindowSize(m_dwFixedNumber - 1);
        m_dwMax = sz.cy;
      }
      else{
        m_dwMax = 0x0100000;
      }
    }
  }

  // Place buttons
  PlaceButtons();

  {
    DWORD dwButton;
    for(dwButton = 0; dwButton < m_dwButtonCount; dwButton++){
      m_aButtons[dwButton]->ShowWindow();
    }
  }

  // And resize the window
  CRect rcWindow = GetWindowPosition();
  rcWindow.right = rcWindow.left + szWindow.cx;
  rcWindow.bottom = rcWindow.top + szWindow.cy;
  m_rcLastPosition = rcWindow;
  m_bSizing = TRUE;
  SetWindowPosition(&rcWindow);
  m_bSizing = FALSE;
}

void CUnitToolbar::OnMove(CPoint point)
{
  m_rcLastPosition = GetWindowPosition();
}

void CUnitToolbar::OnSize(CSize size)
{
  if(m_dwButtonCount == 0){
    CFrameWindow::OnSize(size);
    CRect rc = GetWindowPosition();
    m_rcLastPosition = rc;
    return;
  }

  if(m_bSizing){
    CFrameWindow::OnSize(size);
    CRect rc = GetWindowPosition();
    m_rcLastPosition = rc;
    return;
  }

  if(size == m_rcLastPosition.Size()){
    goto Resize;
  }

  if(size.cx != m_rcLastPosition.Size().cx){
    // The x move
    if(m_bLine){
      if((m_dwMin <= (DWORD)size.cx) && (m_dwMax > (DWORD)size.cx)) goto Resize;

      if(m_dwMin > (DWORD)size.cx){
        CSize sz;
        while(m_dwMin > (DWORD)size.cx){
          m_dwFixedNumber++;
          sz = ComputeWindowSize(m_dwFixedNumber);
          m_dwMax = m_dwMin;
          m_dwMin = sz.cx;
          if(CompareWithSquare(sz) <= 0){
            m_bLine = FALSE;
            m_dwFixedNumber = size.cx / m_szButton.cx;
            if(m_dwFixedNumber == 0) m_dwFixedNumber = 1;
            sz = ComputeWindowSize(m_dwFixedNumber);
            m_dwMin = sz.cy;
            if(m_dwFixedNumber <= 1)
              m_dwMax = 0x0100000;
            else{
              sz = ComputeWindowSize(m_dwFixedNumber - 1);
              m_dwMax = sz.cy;
            }
            goto Resize;
          }
        }

        goto Resize;
      }

      if(m_dwMax < (DWORD)size.cx){
        CSize sz;
        while(m_dwMax < (DWORD)size.cx){
          m_dwFixedNumber--;
          m_dwMin = m_dwMax;
          if(m_dwFixedNumber <= 1){
            m_dwMax = 0x0100000;
            goto Resize;
          }
          sz = ComputeWindowSize(m_dwFixedNumber - 1);
          m_dwMax = sz.cx;
        }

        goto Resize;
      }
    }
    else{
      DWORD dwFixedNumber = m_dwFixedNumber;
      dwFixedNumber = size.cx / m_szButton.cx;
      if(dwFixedNumber == 0) dwFixedNumber = 1;

      if(dwFixedNumber == m_dwFixedNumber) goto Resize;
      m_dwFixedNumber = dwFixedNumber;
      CSize sz;
      sz = ComputeWindowSize(m_dwFixedNumber);
      if(CompareWithSquare(sz) <= 0){
        m_bLine = TRUE;
        m_dwMax = 1;
        m_dwMin = 0;
        m_dwFixedNumber = m_dwButtonCount;
        while(m_dwMax < (DWORD)size.cx){
          m_dwMin = m_dwMax;
          if(m_dwFixedNumber <= 1){
            m_dwMax = 0x0100000;
            goto Resize;
          }
          m_dwFixedNumber--;
          sz = ComputeWindowSize(m_dwFixedNumber);
          m_dwMax = sz.cx;
        }

        goto Resize;
      }
      m_dwMin = sz.cy;
      if(m_dwFixedNumber <= 1)
        m_dwMax = 0x0100000;
      else{
        sz = ComputeWindowSize(m_dwFixedNumber - 1);
        m_dwMax = sz.cy;
      }

      goto Resize;
    }
  }
  else{
    // The y move
    if(!m_bLine){
      if((m_dwMin <= (DWORD)size.cy) && (m_dwMax > (DWORD)size.cy)) goto Resize;

      if(m_dwMin > (DWORD)size.cy){
        CSize sz;
        while(m_dwMin > (DWORD)size.cy){
          m_dwFixedNumber++;
          sz = ComputeWindowSize(m_dwFixedNumber);
          m_dwMax = m_dwMin;
          m_dwMin = sz.cy;
          if(CompareWithSquare(sz) <= 0){
            m_bLine = TRUE;
            m_dwFixedNumber = size.cy / m_szButton.cy;
            if(m_dwFixedNumber == 0) m_dwFixedNumber = 1;
            sz = ComputeWindowSize(m_dwFixedNumber);
            m_dwMin = sz.cx;
            if(m_dwFixedNumber <= 1)
              m_dwMax = 0x0100000;
            else{
              sz = ComputeWindowSize(m_dwFixedNumber - 1);
              m_dwMax = sz.cx;
            }
            goto Resize;
          }
        }

        goto Resize;
      }

      if(m_dwMax < (DWORD)size.cy){
        CSize sz;
        while(m_dwMax < (DWORD)size.cy){
          m_dwFixedNumber--;
          m_dwMin = m_dwMax;
          if(m_dwFixedNumber <= 1){
            m_dwMax = 0x0100000;
            goto Resize;
          }
          sz = ComputeWindowSize(m_dwFixedNumber - 1);
          m_dwMax = sz.cy;
        }

        goto Resize;
      }
    }
    else{
      DWORD dwFixedNumber = m_dwFixedNumber;
      dwFixedNumber = size.cy / m_szButton.cy;
      if(dwFixedNumber <= 0) dwFixedNumber = 1;

      if(dwFixedNumber == m_dwFixedNumber) goto Resize;
      m_dwFixedNumber = dwFixedNumber;
      CSize sz;
      sz = ComputeWindowSize(m_dwFixedNumber);
      if(CompareWithSquare(sz) <= 0){
        m_bLine = FALSE;
        m_dwMax = 1;
        m_dwMin = 0;
        m_dwFixedNumber = m_dwButtonCount;
        while(m_dwMax < (DWORD)size.cy){
          m_dwMin = m_dwMax;
          if(m_dwFixedNumber <= 1){
            m_dwMax = 0x0100000;
            goto Resize;
          }
          m_dwFixedNumber--;
          sz = ComputeWindowSize(m_dwFixedNumber);
          m_dwMax = sz.cy;
        }

        goto Resize;
      }
      m_dwMin = sz.cx;
      if(m_dwFixedNumber <= 1)
        m_dwMax = 0x0100000;
      else{
        sz = ComputeWindowSize(m_dwFixedNumber - 1);
        m_dwMax = sz.cx;
      }

      goto Resize;
    }
  }

  if ( m_dwFixedNumber == 0 )
	  return;
Resize:;
  {
    CSize szWindow;
    szWindow = ComputeWindowSize(m_dwFixedNumber);
    PlaceButtons();
    // And resize the window
    CRect rcWindow = GetWindowPosition();
    if(rcWindow.TopLeft() == m_rcLastPosition.TopLeft()){
      rcWindow.right = rcWindow.left + szWindow.cx;
      rcWindow.bottom = rcWindow.top + szWindow.cy;
    }
    else{
      rcWindow.left = rcWindow.right - szWindow.cx;
      rcWindow.top = rcWindow.bottom - szWindow.cy;
    }
    m_bSizing = TRUE;
    CFrameWindow::SetWindowPosition(&rcWindow);
    m_bSizing = FALSE;
    m_rcLastPosition = rcWindow;
  }

}

CSize CUnitToolbar::ComputeWindowSize(DWORD dwFixedCount)
{
  DWORD dwButtonWidth, dwButtonHeight, dwOneLine;

  if(m_bLine){
    dwButtonWidth = m_szButton.cx;
    dwButtonHeight = m_szButton.cy;
    dwOneLine = m_szOneLine.cx;
  }
  else{
    dwButtonWidth = m_szButton.cy;
    dwButtonHeight = m_szButton.cy;
    dwOneLine = m_szOneLine.cy;
  }

  DWORD dwBreakWidth, dwCurrentWidth, dwItemWidth;
  dwBreakWidth = dwOneLine / dwFixedCount;
  DWORD dwMaxWidth, dwLines;

  DWORD dwButton;
  dwCurrentWidth = 0;
  dwLines = 1;
  dwMaxWidth = 0;
  for(dwButton = 0; dwButton < m_dwButtonCount;){
    if(m_aButtons[dwButton] == NULL){
      // separator
      dwItemWidth = UT_SEPARATOR_SIZE;
    }
    else{
      dwItemWidth = dwButtonWidth;
    }

    if((dwCurrentWidth + dwItemWidth) > dwBreakWidth){
      BOOL bBefore = FALSE;
      if(m_aButtons[dwButton] == NULL) bBefore = FALSE;
      else{
        if(dwButton == 0) bBefore = FALSE;
        else{
          if(m_aButtons[dwButton - 1] != NULL) bBefore = FALSE;
          else{
            if(dwButton >= m_dwButtonCount - 1) bBefore = FALSE;
            else{
              if(m_aButtons[dwButton + 1] == NULL) bBefore = FALSE;
              bBefore = TRUE;
            }
          }
        }
      }
      if(bBefore){
        // Leave the separator to next line
        if(dwLines >= dwFixedCount){
          // Last line -> no break
          dwCurrentWidth += dwItemWidth;
          dwButton++;
          continue;
        }

        // Break the line
        if(dwMaxWidth < dwCurrentWidth) dwMaxWidth = dwCurrentWidth;
        dwCurrentWidth = 0;
        dwLines++;
        continue;
      }
      else{
        // Break it after the separator
        dwCurrentWidth += dwItemWidth;
        if(dwLines >= dwFixedCount){
          // Last line -> no break
        }
        else{
          if(dwMaxWidth < dwCurrentWidth) dwMaxWidth = dwCurrentWidth;
          dwCurrentWidth = 0;
          dwLines++;
        }
        dwButton++;
      }
    }
    else{
      dwCurrentWidth += dwItemWidth;
      if(dwMaxWidth < dwCurrentWidth) dwMaxWidth = dwCurrentWidth;
      dwButton++;
    }
  }
  if(dwCurrentWidth > dwMaxWidth) dwMaxWidth = dwCurrentWidth;

  CSize szWindow;
  if(m_bLine){
    szWindow.cx = dwMaxWidth + UT_BORDER_LEFT + UT_BORDER_RIGHT;
    szWindow.cy = dwLines * dwButtonHeight + UT_BORDER_TOP + UT_BORDER_BOTTOM;
  }
  else{
    szWindow.cx = dwLines * dwButtonHeight + UT_BORDER_LEFT + UT_BORDER_RIGHT;
    szWindow.cy = dwMaxWidth + UT_BORDER_TOP + UT_BORDER_BOTTOM;
  }

  return szWindow;
}

CSize CUnitToolbar::ComputeOneLineSize()
{
  DWORD dwButton, dwSize;
  CSize sz;

  dwSize = 0;
  for(dwButton = 0; dwButton < m_dwButtonCount; dwButton++){
    if(m_aButtons[dwButton] == NULL) dwSize += UT_SEPARATOR_SIZE;
    else dwSize += m_szButton.cx;
  }
  sz.cx = dwSize;

  dwSize = 0;
  for(dwButton = 0; dwButton < m_dwButtonCount; dwButton++){
    if(m_aButtons[dwButton] == NULL) dwSize += UT_SEPARATOR_SIZE;
    else dwSize += m_szButton.cy;
  }
  sz.cy = dwSize;
  return sz;
}

int CUnitToolbar::CompareWithSquare(CSize sz)
{
  if(m_bLine){
    if(sz.cx > sz.cy) return 1;
    if(sz.cx < sz.cy) return -1;
    return 0;
  }
  else{
    if(sz.cy > sz.cx) return 1;
    if(sz.cy < sz.cx) return -1;
    return 0;
  }
  return 0;
}

void CUnitToolbar::PlaceButtons()
{
  DWORD dwButtonWidth, dwButtonHeight, dwOneLine;

  if(m_bLine){
    dwButtonWidth = m_szButton.cx;
    dwButtonHeight = m_szButton.cy;
    dwOneLine = m_szOneLine.cx;
  }
  else{
    dwButtonWidth = m_szButton.cy;
    dwButtonHeight = m_szButton.cy;
    dwOneLine = m_szOneLine.cy;
  }

  DWORD dwBreakWidth, dwCurrentWidth, dwItemWidth;
  dwBreakWidth = dwOneLine / m_dwFixedNumber;
  DWORD dwMaxWidth, dwLines;
  CPoint pt;

  DWORD dwButton;
  dwCurrentWidth = 0;
  dwLines = 1;
  dwMaxWidth = 0;
  for(dwButton = 0; dwButton < m_dwButtonCount;){
    if(m_bLine){
      pt.x = dwCurrentWidth + UT_BORDER_LEFT;
      pt.y = (dwLines - 1) * dwButtonHeight + UT_BORDER_TOP;
    }
    else{
      pt.y = dwCurrentWidth + UT_BORDER_TOP;
      pt.x = (dwLines - 1) * dwButtonHeight + UT_BORDER_LEFT;
    }

    if(m_aButtons[dwButton] == NULL){
      // separator
      dwItemWidth = UT_SEPARATOR_SIZE;
    }
    else{
      dwItemWidth = dwButtonWidth;
    }

    if((dwCurrentWidth + dwItemWidth) > dwBreakWidth){
      BOOL bBefore = FALSE;
      if(m_aButtons[dwButton] == NULL) bBefore = FALSE;
      else{
        if(dwButton == 0) bBefore = FALSE;
        else{
          if(m_aButtons[dwButton - 1] != NULL) bBefore = FALSE;
          else{
            if(dwButton >= m_dwButtonCount - 1) bBefore = FALSE;
            else{
              if(m_aButtons[dwButton + 1] == NULL) bBefore = FALSE;
              bBefore = TRUE;
            }
          }
        }
      }
      if(bBefore){
        // Leave the separator to next line
        if(dwLines >= m_dwFixedNumber){
          // Last line -> no break
          if(m_aButtons[dwButton] != NULL){
            m_aButtons[dwButton]->SetWindowPosition(&pt);
          }
          dwCurrentWidth += dwItemWidth;
          dwButton++;
          continue;
        }

        // Break the line
        if(dwMaxWidth < dwCurrentWidth) dwMaxWidth = dwCurrentWidth;
        dwCurrentWidth = 0;
        dwLines++;
        continue;
      }
      else{
        // Break it after the separator
        if(m_aButtons[dwButton] != NULL){
          m_aButtons[dwButton]->SetWindowPosition(&pt);
        }
        dwCurrentWidth += dwItemWidth;
        if(dwLines >= m_dwFixedNumber){
          // Last line -> no break
        }
        else{
          if(dwMaxWidth < dwCurrentWidth) dwMaxWidth = dwCurrentWidth;
          dwCurrentWidth = 0;
          dwLines++;
        }
        dwButton++;
      }
    }
    else{
      if(m_aButtons[dwButton] != NULL){
        m_aButtons[dwButton]->SetWindowPosition(&pt);
      }
      dwCurrentWidth += dwItemWidth;
      if(dwMaxWidth < dwCurrentWidth) dwMaxWidth = dwCurrentWidth;
      dwButton++;
    }
  }
}

void CUnitToolbar::OnButton(DWORD dwButton)
{
  ASSERT(dwButton < m_dwButtonCount);
  ASSERT(m_aButtons[dwButton] != NULL);

  InlayEvent(E_Command, (DWORD)(m_pToolbarInfo->GetButtonInfo(dwButton)));
}

void CUnitToolbar::DisablePartial()
{
  // go through the buttons and disable them
  DWORD dwButton;
  for(dwButton = 0; dwButton < m_dwButtonCount; dwButton++){
    if(m_aButtons[dwButton] != NULL){
      m_aButtons[dwButton]->EnableWindow(FALSE);
    }
  }
}

void CUnitToolbar::EnableAll()
{
  // go through the buttons and disable them
  DWORD dwButton;
  for(dwButton = 0; dwButton < m_dwButtonCount; dwButton++){
    if(m_aButtons[dwButton] != NULL){
      m_aButtons[dwButton]->EnableWindow();
    }
  }
}