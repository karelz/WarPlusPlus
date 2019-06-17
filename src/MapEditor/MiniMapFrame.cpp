// MiniMapFrame.cpp : implementation file
//

#include "stdafx.h"
#include "MapEditor.h"
#include "MiniMapFrame.h"

#include "DataObjects\EMap.h"
#include "MapEditorDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MINIMAPFRAME_VIEWRECTCOLOR RGB32(255, 255, 255)

/////////////////////////////////////////////////////////////////////////////
// CMiniMapFrame

IMPLEMENT_DYNCREATE(CMiniMapFrame, CMiniFrameWnd)

CMiniMapFrame::CMiniMapFrame()
{
  m_nMouseDrag = 0;
}

CMiniMapFrame::~CMiniMapFrame()
{
}


BEGIN_MESSAGE_MAP(CMiniMapFrame, CMiniFrameWnd)
	//{{AFX_MSG_MAP(CMiniMapFrame)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMiniMapFrame message handlers

int CMiniMapFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMiniFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_Clipper.Create(this);

  m_hMoveCursor = AfxGetApp()->LoadCursor(IDC_MOVECURSOR);

  m_VerticalLine.SetWidth(1); m_VerticalLine.SetHeight(50);
  m_VerticalLine.Create(); m_VerticalLine.Fill(MINIMAPFRAME_VIEWRECTCOLOR);
  m_HorizontalLine.SetWidth(50); m_HorizontalLine.SetHeight(1);
  m_HorizontalLine.Create(); m_HorizontalLine.Fill(MINIMAPFRAME_VIEWRECTCOLOR);

  return 0;
}

void CMiniMapFrame::OnDestroy() 
{
	CMiniFrameWnd::OnDestroy();
	
  CWinApp *pApp = AfxGetApp();

  m_VerticalLine.Delete(); m_HorizontalLine.Delete();

  CRect rcMiniMap;
  GetWindowRect(&rcMiniMap);
  pApp->WriteProfileInt("Layout", "MiniMapX", rcMiniMap.left);
  pApp->WriteProfileInt("Layout", "MiniMapY", rcMiniMap.top);

  m_Clipper.Delete();
}

void CMiniMapFrame::OnClose() 
{
  ShowWindow(SW_HIDE);
}

void CMiniMapFrame::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
  CRect rcClient;
  GetClientRect(&rcClient);
  ClientToScreen(&rcClient);
  CRect rcClip(0, 0, rcClient.Width(), rcClient.Height());

  g_pDDPrimarySurface->SetClipper(&m_Clipper);
  CEMap *pMap = AppGetActiveMap();
  CDDrawSurface *pMiniMap = pMap->GetMiniMap();
  CDDrawSurface *pMiniMapBuffer = pMap->GetMiniMapBuffer();

  if((!pMiniMap->IsValid()) || (!pMiniMapBuffer->IsValid())){
    dc.FillSolidRect(&rcClient, RGB(192, 192, 192));
    pMap->ReleaseMiniMap();
    return;
  }
  
  // draw the visible rect
  CRect rcVisible = pMap->GetVisibleRect();
  CRect rcLine;
  int nZoom = (int)pMap->GetMiniMapZoom();
  CRect rcMiniMap = pMap->GetMiniMapRect();
  rcVisible.OffsetRect(-rcMiniMap.left, -rcMiniMap.top);
  rcVisible.left /= nZoom;
  rcVisible.right /= nZoom;
  rcVisible.top /= nZoom;
  rcVisible.bottom /= nZoom;
  pMiniMapBuffer->Paste(0, 0, pMiniMap);
  {
    int x, xl;
    xl = rcVisible.Width() + 1; x = rcVisible.left;
    rcLine.left = 0; rcLine.top = 0; rcLine.bottom = m_HorizontalLine.GetHeight();
    while(xl > 0){
      if(xl >= (int)m_HorizontalLine.GetWidth()) rcLine.right = rcLine.left + m_HorizontalLine.GetWidth();
      else rcLine.right = rcLine.left + xl;
      pMiniMapBuffer->Paste(x, rcVisible.top, &m_HorizontalLine, &rcLine);
      pMiniMapBuffer->Paste(x, rcVisible.bottom, &m_HorizontalLine, &rcLine);
      x += rcLine.Width();
      xl -= rcLine.Width();
    }

    int y, yl;
    yl = rcVisible.Height() + 1; y = rcVisible.top;
    rcLine.left = 0; rcLine.right = m_VerticalLine.GetWidth(); rcLine.top = 0;
    while(yl > 0){
      if(yl >= (int)m_VerticalLine.GetHeight()) rcLine.bottom = rcLine.top + m_VerticalLine.GetHeight();
      else rcLine.bottom = rcLine.top + yl;
      pMiniMapBuffer->Paste(rcVisible.left, y, &m_VerticalLine, &rcLine);
      pMiniMapBuffer->Paste(rcVisible.right, y, &m_VerticalLine, &rcLine);
      y += rcLine.Height();
      yl -= rcLine.Height();
    }
  }

  g_pDDPrimarySurface->Paste(rcClient.TopLeft(), pMiniMapBuffer, &rcClip);

  pMap->ReleaseMiniMap();
}

void CMiniMapFrame::OnRButtonDown(UINT nFlags, CPoint point) 
{
  if(m_nMouseDrag == 0){
    CEMap *pMap = AppGetActiveMap();
    if(pMap == NULL) return;

    m_nMouseDrag = 2;
    SetCapture();
    SetCursor(m_hMoveCursor);

    CRect rcMap, rcClient;
    rcMap = pMap->GetMiniMapRect();
    GetClientRect(&rcClient);
    m_ptDragStart.x = rcMap.left + (point.x - rcClient.left) * (rcMap.Width() / 200);
    m_ptDragStart.y = rcMap.top + (point.y - rcClient.top) * (rcMap.Height() / 200);
    m_rcMap = rcMap;
  }
	
	CMiniFrameWnd::OnRButtonDown(nFlags, point);
}

void CMiniMapFrame::OnRButtonUp(UINT nFlags, CPoint point) 
{
  if(m_nMouseDrag == 2){
    ReleaseCapture();
    m_nMouseDrag = 0;
  }
  
	CMiniFrameWnd::OnRButtonUp(nFlags, point);
}

void CMiniMapFrame::OnMouseMove(UINT nFlags, CPoint point) 
{
  if(m_nMouseDrag == 1){
    SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));

    SetViewPosition(point);
  }

  if(m_nMouseDrag == 2){
    SetCursor(m_hMoveCursor);
    CEMap *pMap = AppGetActiveMap();
    if(pMap == NULL) return;

    CPoint ptPos;
    CRect rcClient;
    GetClientRect(&rcClient);
    ptPos.x = m_rcMap.left + (point.x - rcClient.left) * (m_rcMap.Width() / 200);
    ptPos.y = m_rcMap.top + (point.y - rcClient.top) * (m_rcMap.Height() / 200);

    CRect rcMap = m_rcMap;
    rcMap.OffsetRect(m_ptDragStart.x - ptPos.x, m_ptDragStart.y - ptPos.y);

    pMap->SetMiniMapRect(rcMap);
  }

  if(m_nMouseDrag == 0){
    SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
  }

	CMiniFrameWnd::OnMouseMove(nFlags, point);
}

BOOL CMiniMapFrame::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
  return TRUE;
}

void CMiniMapFrame::OnLButtonDown(UINT nFlags, CPoint point) 
{
  if(m_nMouseDrag == 0){
    m_nMouseDrag = 1;
    SetCapture();
    SetViewPosition(point);
  }
  
	CMiniFrameWnd::OnLButtonDown(nFlags, point);
}

void CMiniMapFrame::OnLButtonUp(UINT nFlags, CPoint point) 
{
  if(m_nMouseDrag == 1){
    ReleaseCapture();
    m_nMouseDrag = 0;
  }
  
	CMiniFrameWnd::OnLButtonUp(nFlags, point);
}

void CMiniMapFrame::SetViewPosition(CPoint point)
{
  CRect rcClient;
  GetClientRect(&rcClient);

  CEMap *pMap = AppGetActiveMap();
  CRect rcViewMap = pMap->GetVisibleRect();

  CPoint pt;
  CRect rcMap = pMap->GetMiniMapRect();
  pt.x = rcMap.left + (point.x - rcClient.left) * pMap->GetMiniMapZoom();
  pt.y = rcMap.top + (point.y - rcClient.top) * pMap->GetMiniMapZoom();

  CRect rcViewRect(rcViewMap);
  rcViewRect.left = pt.x - (rcViewRect.Width() / 2);
  rcViewRect.top = pt.y - (rcViewRect.Height() / 2);
  rcViewRect.right = rcViewRect.left + rcViewMap.Width();
  rcViewRect.bottom = rcViewRect.top + rcViewMap.Height();

  CMapEditorDoc *pDoc = AppGetActiveDocument();
  pDoc->SetVisibleRect(rcViewRect);
}

