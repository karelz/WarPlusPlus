// MapexEditView.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "MapexEditView.h"

#include "..\DataObjects\EMap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMapexEditView

CMapexEditView::CMapexEditView()
{
}

CMapexEditView::~CMapexEditView()
{
}


BEGIN_MESSAGE_MAP(CMapexEditView, CWnd)
	//{{AFX_MSG_MAP(CMapexEditView)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapexEditView message handlers

void CMapexEditView::Create(CEMapex *pMapex, CRect &rcBound, DWORD dwStyles, CWnd *pParent, UINT nID)
{
  CWnd::Create(NULL, "", dwStyles, rcBound, pParent, nID);

  ASSERT(pMapex != NULL);
  m_pMapex = pMapex;

  ReallocateBuffers();

  m_bMouseDrag = FALSE;

  m_Clipper.Create(this);
}

void CMapexEditView::OnDestroy() 
{
	CWnd::OnDestroy();
	
  m_TransBuffer.Delete();
  m_Buffer.Delete();

  m_Clipper.Delete();
}

void CMapexEditView::OnMapexSizeChanged()
{
  ReallocateBuffers();

  Invalidate(FALSE);
}

void CMapexEditView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
  CDDrawSurface *pMapexGraphics = m_pMapex->GetGraphics()->GetFrame(0);
  g_pDDPrimarySurface->SetClipper(&m_Clipper);

  CRect rcClient;
  GetClientRect(&rcClient);
  CRect rcInner(rcClient);
  rcInner.DeflateRect(2, 2);

  CBrush brush;
  brush.CreateSolidBrush(RGB(255,255, 255));
  CRect rcMapex;
  rcMapex.left = rcInner.left;
  rcMapex.top = rcInner.top;
  rcMapex.bottom = rcMapex.top + m_Buffer.GetHeight();
  rcMapex.right = rcMapex.left + m_Buffer.GetWidth();
  rcMapex.IntersectRect(&rcMapex, &rcInner);

  CRect rcFill;
  rcFill.left = rcMapex.right; rcFill.top = rcInner.top;
  rcFill.right = rcInner.right; rcFill.bottom = rcMapex.bottom;
  dc.FillRect(&rcFill, &brush);
  rcFill.left = rcInner.left; rcFill.top = rcMapex.bottom;
  rcFill.right = rcInner.right; rcFill.bottom = rcInner.bottom;
  dc.FillRect(&rcFill, &brush);

  dc.DrawEdge(&rcClient, EDGE_SUNKEN, BF_RECT);

  ClientToScreen(&rcMapex);

  if((m_eViewType == VIEW_BOTH) || (m_eViewType == VIEW_GRID)){
    m_TransBuffer.Fill(RGB32(255, 255, 255));
    // draw land types
    CSize size = m_pMapex->GetSize();
    int i, j;
    CRect rcCell;
    CLandType *pLandType;
    CEMap *pMap = AppGetActiveMap();
    for(i = 0; i < size.cy; i++){
      rcCell.top = i * 16; rcCell.bottom = rcCell.top + 16;
      for(j = 0; j < size.cx; j++){
        rcCell.left = j * 16; rcCell.right = rcCell.left + 16;
        
        // get the land type for this cell
        pLandType = pMap->GetLandType(m_pMapex->GetLandType(j, i));
        if(pLandType == NULL){
          pLandType = pMap->GetLandType(0);
        }
        
        // if has animation -> draw it
        if(pLandType->GetAnimation() != NULL){
          if(pLandType->GetAnimation()->GetFrame(0) != NULL){
            m_TransBuffer.Paste(rcCell.TopLeft(), pLandType->GetAnimation()->GetFrame(0));
            continue;
          }
        }
        // no animation or frame -> fill it with color
        m_TransBuffer.Fill(pLandType->GetColor(), &rcCell);
      }
    }
    
    // draw grid lines
    DWORD dwLen;
    DWORD dwPitch, dwOff;
    LPBYTE pData = (LPBYTE)m_TransBuffer.Lock(m_TransBuffer.GetAllRect(), 0, dwPitch);
    dwLen = m_TransBuffer.GetWidth();
    if(g_pDirectDraw->Is32BitMode()){
      for(i = 1; i <= size.cy; i++){
        dwOff = (i * 16 - 1) * dwPitch;
        for(j = 0; j < (int)dwLen; j++){
          pData[dwOff + j * 4] = 0;
          pData[dwOff + j * 4 + 1] = 255;
          pData[dwOff + j * 4 + 2] = 0;
        }
      }
    }
    else{
      for(i = 1; i <= size.cy; i++){
        dwOff = (i * 16 - 1) * dwPitch;
        for(j = 0; j < (int)dwLen; j++){
          pData[dwOff + j * 3] = 0;
          pData[dwOff + j * 3 + 1] = 255;
          pData[dwOff + j * 3 + 2] = 0;
        }
      }
    }
    dwLen = m_TransBuffer.GetHeight();
    if(g_pDirectDraw->Is32BitMode()){
      for(i = 1; i <= size.cx; i++){
        dwOff = (i * 16 - 1) * 4;
        for(j = 0; j < (int)dwLen; j++){
          pData[dwOff + j * dwPitch] = 0;
          pData[dwOff + j * dwPitch + 1] = 255;
          pData[dwOff + j * dwPitch + 2] = 0;
        }
      }
    }
    else{
      for(i = 1; i <= size.cx; i++){
        dwOff = (i * 16 - 1) * 3;
        for(j = 0; j < (int)dwLen; j++){
          pData[dwOff + j * dwPitch] = 0;
          pData[dwOff + j * dwPitch + 1] = 255;
          pData[dwOff + j * dwPitch + 2] = 0;
        }
      }
    }
    m_TransBuffer.Unlock(pData);

    // set transparency
    LPBYTE pAlpha = m_TransBuffer.GetAlphaChannel();
    if(m_eViewType == VIEW_BOTH){
      memset(pAlpha, 80, m_TransBuffer.GetWidth() * m_TransBuffer.GetHeight());
    }
    if(m_eViewType == VIEW_GRID){
      memset(pAlpha, 255, m_TransBuffer.GetWidth() * m_TransBuffer.GetHeight());
    }
    m_TransBuffer.ReleaseAlphaChannel(TRUE);
  }

  m_Buffer.Fill(RGB32(255, 255, 255));
  if((m_eViewType == VIEW_BOTH) || (m_eViewType == VIEW_GRAPHICS)){
    m_Buffer.Paste(0, 0, pMapexGraphics);
  }
  if((m_eViewType == VIEW_BOTH) || (m_eViewType == VIEW_GRID)){
    m_Buffer.Paste(0, 0, &m_TransBuffer);
  }
  CRect rcBuffer(0, 0, rcMapex.Width(), rcMapex.Height());

  g_pDDPrimarySurface->Paste(rcMapex.TopLeft(), &m_Buffer, &rcBuffer);
}

void CMapexEditView::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
}

BOOL CMapexEditView::OnEraseBkgnd(CDC* pDC) 
{
  return TRUE;
}

void CMapexEditView::OnLButtonDown(UINT nFlags, CPoint point) 
{
  CRect rcClient;
  GetClientRect(&rcClient);
  rcClient.DeflateRect(2, 2);
  if(rcClient.PtInRect(point)){
    DWORD dwX, dwY;
    CSize sizeMapex = m_pMapex->GetSize();
    dwX = (point.x - rcClient.left) / 16;
    dwY = (point.y - rcClient.top) / 16;
    if(dwX >= (DWORD)sizeMapex.cx) goto Quit;
    if(dwY >= (DWORD)sizeMapex.cy) goto Quit;

    m_pMapex->SetLandType(dwX, dwY, m_pCurrentLandType->GetID());
    Invalidate(FALSE);

    m_dwLastXPos = dwX; m_dwLastYPos = dwY;
    m_bMouseDrag = TRUE;
  }

Quit:
	CWnd::OnLButtonDown(nFlags, point);
}

void CMapexEditView::SetCurrentLandType(CLandType *pLandType)
{
  m_pCurrentLandType = pLandType;
}

void CMapexEditView::OnMouseMove(UINT nFlags, CPoint point) 
{
  CRect rcClient;
  GetClientRect(&rcClient);
  rcClient.DeflateRect(2, 2);
  if(rcClient.PtInRect(point) && m_bMouseDrag){
    DWORD dwX, dwY;
    CSize sizeMapex = m_pMapex->GetSize();
    dwX = (point.x - rcClient.left) / 16;
    dwY = (point.y - rcClient.top) / 16;
    if(dwX >= (DWORD)sizeMapex.cx) goto Quit;
    if(dwY >= (DWORD)sizeMapex.cy) goto Quit;
    if((dwX == m_dwLastXPos) && (dwY == m_dwLastYPos)) goto Quit;

    m_pMapex->SetLandType(dwX, dwY, m_pCurrentLandType->GetID());
    Invalidate(FALSE);

    m_dwLastXPos = dwX; m_dwLastYPos = dwY;
  }

Quit:
	CWnd::OnMouseMove(nFlags, point);
}

void CMapexEditView::OnLButtonUp(UINT nFlags, CPoint point) 
{
  m_bMouseDrag = FALSE;
  
	CWnd::OnLButtonUp(nFlags, point);
}

void CMapexEditView::SetViewType(EViewType eType)
{
  m_eViewType = eType;
  Invalidate(FALSE);
}

CMapexEditView::EViewType CMapexEditView::GetViewType()
{
  return m_eViewType;
}

void CMapexEditView::OnMapexGraphicsChanged()
{
  ReallocateBuffers();

  Invalidate();
}

void CMapexEditView::ReallocateBuffers()
{
  // we have to reallocate the buffer
  m_Buffer.Delete();
  m_TransBuffer.Delete();

  DWORD dwWidth, dwHeight;
  dwWidth = m_pMapex->GetSize().cx * 16;
  dwHeight = m_pMapex->GetSize().cy * 16;
  CDDrawSurface *pSurface = m_pMapex->GetGraphics()->GetFrame(0);
  if(dwWidth < pSurface->GetWidth()) dwWidth = pSurface->GetWidth();
  if(dwHeight < pSurface->GetHeight()) dwHeight = pSurface->GetHeight();

  m_Buffer.SetWidth(dwWidth);
  m_Buffer.SetHeight(dwHeight);
  m_Buffer.Create();

  m_TransBuffer.SetWidth(m_pMapex->GetSize().cx * 16);
  m_TransBuffer.SetHeight(m_pMapex->GetSize().cy * 16);
  m_TransBuffer.SetAlphaChannel(TRUE);
  m_TransBuffer.Create();
}
