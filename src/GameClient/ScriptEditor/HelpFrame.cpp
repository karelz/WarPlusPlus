// HelpFrame.cpp: implementation of the CHelpFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HelpFrame.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CHelpFrame, CCaptionWindow)

BEGIN_OBSERVER_MAP(CHelpFrame, CCaptionWindow)
  BEGIN_NOTIFIER(IDC_CLOSE)
    ON_BUTTONCOMMAND(OnClose)
  END_NOTIFIER()
END_OBSERVER_MAP(CHelpFrame, CCaptionWindow)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHelpFrame::CHelpFrame()
{

}

CHelpFrame::~CHelpFrame()
{

}
#ifdef _DEBUG

void CHelpFrame::AssertValid() const
{
  CCaptionWindow::AssertValid();
}

void CHelpFrame::Dump(CDumpContext &dc) const
{
  CCaptionWindow::Dump(dc);
}

#endif

void CHelpFrame::Create(CRect rcWindow, CWindow *pParent, CCaptionWindowLayout *pFrameLayout, CDataArchive *pArchive)
{
  CCaptionWindow::Create(rcWindow, "", pFrameLayout, pParent, TRUE);

  // first create the close button
  {
    CDialogWindowLayout *pLayout = CLayouts::m_pDefaults->GetDialogWindowLayout();

    // create the point and button itself there
    CPoint pt(rcWindow.Width() - pLayout->m_dwCloseRightMargin,
      pLayout->m_dwCloseTopMargin);
    m_CloseButton.Create(pt, &(pLayout->m_CloseButtonLayout), this, FALSE);

    // connect it
    m_CloseButton.Connect(this, IDC_CLOSE);
  }

  CRect rcHelp;
  GetBrowserRect(&rcHelp);
  m_wndHelpBrowser.Create(rcHelp, this, pArchive);
}

void CHelpFrame::Delete()
{
  m_wndHelpBrowser.Delete();

  m_CloseButton.Delete();
  CCaptionWindow::Delete();
}

void CHelpFrame::OnSize(CSize size)
{
  CRect rcBrowser;
  GetBrowserRect(&rcBrowser);
  m_wndHelpBrowser.SetPosition(&rcBrowser);

  CDialogWindowLayout *pLayout = CLayouts::m_pDefaults->GetDialogWindowLayout();
  CPoint pt(size.cx - pLayout->m_dwCloseRightMargin,
    pLayout->m_dwCloseTopMargin);
  m_CloseButton.SetWindowPosition(&pt);

  CCaptionWindow::OnSize(size);
}

void CHelpFrame::OnClose()
{
  HideWindow();
}

void CHelpFrame::GetBrowserRect(CRect *pRect)
{
  CRect rcBound(GetWindowPosition());

  pRect->left = 7;
  pRect->top = 26;
  pRect->right = rcBound.Width() - 7;
  pRect->bottom = rcBound.Height() - 7;
}

CSize CHelpFrame::GetMaximumSize()
{
  CSize sz = m_wndHelpBrowser.GetMaximumSize();
  sz.cx += 7 + 7;
  sz.cy += 26 + 7;
  return sz;
}