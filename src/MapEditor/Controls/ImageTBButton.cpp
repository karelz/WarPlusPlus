// ImageTBButton.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "ImageTBButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImageTBButton

CImageTBButton::CImageTBButton()
{
  m_pImage = NULL;
  m_dwColor = 0;
  m_bSelected = FALSE;
}

CImageTBButton::~CImageTBButton()
{
  m_Clipper.Delete();
  m_Buffer.Delete();
}


BEGIN_MESSAGE_MAP(CImageTBButton, CWnd)
	//{{AFX_MSG_MAP(CImageTBButton)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CImageTBButton message handlers

void CImageTBButton::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
  CRect rc;
  GetClientRect(&rc);
//  rc.right--;

  if(!m_bSelected){
    CBrush brush;
    brush.CreateSolidBrush(RGB(192, 192, 192));

    dc.FrameRect(&rc, &brush);
    brush.DeleteObject();
  }
  else{
    CBrush brush;
    brush.CreateSolidBrush(RGB(255, 255, 0));

    dc.FrameRect(&rc, &brush);
    brush.DeleteObject();
//    dc.DrawEdge(&rc, EDGE_ETCHED, BF_RECT);
  }

  BeforePaint();

  g_pDDPrimarySurface->SetClipper(&m_Clipper);
  rc.DeflateRect(1, 1);
  ClientToScreen(&rc);

  if(m_pImage != NULL){
    CRect rcBuf(0, 0, rc.Width(), rc.Height());
    CRect rcSrc = *(m_pImage->GetAllRect());
    if(rcBuf.Width() < rcSrc.Width()){
      int nHlp = rcSrc.top + (rcSrc.Width() * rcBuf.Height() / rcBuf.Width());
      if(nHlp < rcSrc.bottom) rcSrc.bottom = nHlp;
    }
    m_Buffer.BltStretch(&rcBuf, m_pImage, &rcSrc);
    g_pDDPrimarySurface->Paste(rc.TopLeft(), &m_Buffer, &rcBuf);
  }
  else{
    g_pDDPrimarySurface->Fill(m_dwColor, &rc);
  }
}

void CImageTBButton::Create(CRect &rcBound, CWnd *pParent, UINT nID)
{
  CWnd::Create(NULL, "", WS_VISIBLE | WS_CHILD, rcBound, pParent, nID);
  m_Clipper.Create(this);
  m_Buffer.SetWidth(rcBound.Width());
  m_Buffer.SetHeight(rcBound.Height());
  m_Buffer.Create();
}

void CImageTBButton::Insert(CToolBarCtrl *pToolBar, int nIndex, UINT nID)
{
  // insert separator
  TBBUTTON but;
  but.fsState = TBSTATE_ENABLED;
  but.fsStyle = TBSTYLE_SEP;
  but.iBitmap = NULL;
  but.iString = NULL;
  but.idCommand = nID;
  pToolBar->InsertButton(nIndex, &but);

  CRect rcItem;
  pToolBar->GetItemRect(nIndex, &rcItem);

  // resize it
  TBBUTTONINFO butinfo;
  butinfo.cbSize = sizeof(butinfo);
  butinfo.cx = rcItem.Height() + 1; // set same width as height
  butinfo.dwMask = TBIF_SIZE;
  pToolBar->SetButtonInfo(nID, &butinfo);

  // get the rect
  pToolBar->GetItemRect(nIndex, &rcItem);

  // replace separator with our window
  Create(rcItem, pToolBar, nID);
}

BOOL CImageTBButton::GetSelected()
{
  return m_bSelected;
}

void CImageTBButton::SetSelected(BOOL bSelected)
{
  if(m_bSelected != bSelected){
    m_bSelected = bSelected;
    Invalidate();
  }
}

void CImageTBButton::OnDestroy() 
{
	CWnd::OnDestroy();
	
  m_Clipper.Delete();	
}

void CImageTBButton::OnLButtonDown(UINT nFlags, CPoint point) 
{
  GetParent()->SendMessage(WM_COMMAND, (WPARAM)GetDlgCtrlID(), (LPARAM)GetSafeHwnd());
	CWnd::OnLButtonDown(nFlags, point);
}

void CImageTBButton::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
  NMHDR nmhdr;
  nmhdr.hwndFrom = GetSafeHwnd();
  nmhdr.idFrom = GetDlgCtrlID();
  nmhdr.code = N_DOUBLECLICKED;

  GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nmhdr);
	CWnd::OnLButtonDblClk(nFlags, point);
}

void CImageTBButton::BeforePaint()
{

}
