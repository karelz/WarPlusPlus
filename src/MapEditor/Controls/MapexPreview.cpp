// MapexPreview.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "MapexPreview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMapexPreview

CMapexPreview::CMapexPreview()
{
  m_pMapex = NULL;
}

CMapexPreview::~CMapexPreview()
{
}


BEGIN_MESSAGE_MAP(CMapexPreview, CStatic)
	//{{AFX_MSG_MAP(CMapexPreview)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapexPreview message handlers

void CMapexPreview::Create(CRect &rcBound, DWORD dwStyle, CWnd *pParent, UINT nID)
{
  m_Buffer.SetWidth(rcBound.Width());
  m_Buffer.SetHeight(rcBound.Height());
  m_Buffer.Create();

  CStatic::Create("", dwStyle, rcBound, pParent, nID);

  m_Clipper.Create(this);
}

void CMapexPreview::SetMapex(CEMapex *pMapex)
{
  m_pMapex = pMapex;
  Invalidate();
}

void CMapexPreview::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

  g_pDDPrimarySurface->SetClipper(&m_Clipper);
  
  CRect rcClient;
  GetClientRect(&rcClient);
  CBrush brush;
  brush.CreateSolidBrush(RGB(192, 192, 192));
  dc.FillRect(&rcClient, &brush);
  brush.DeleteObject();
  ClientToScreen(&rcClient);

  if(m_pMapex != NULL){
    if(m_pMapex->GetGraphics() != NULL){
      if(m_pMapex->GetGraphics()->GetFrame(0) != NULL){
        CRect rc(0, 0, rcClient.Width(), rcClient.Height());
        CRect rcSource(m_pMapex->GetGraphics()->GetFrame(0)->GetAllRect());

        if(rcSource.Width() > rc.Width()){
          rcSource.bottom = (int)((double)rcSource.bottom *
            ((double) rc.right / (double)rcSource.right));
          rcSource.right = rc.right;
        }
        if(rcSource.Height() > rc.Height()){
          rcSource.right = (int)((double)rcSource.right *
            ((double) rc.bottom / (double)rcSource.bottom));
          rcSource.bottom = rc.bottom;
        }

        m_Buffer.Fill(RGB32(192, 192, 192), NULL);
        m_Buffer.BltStretch(&rcSource, m_pMapex->GetGraphics()->GetFrame(0), NULL);
        g_pDDPrimarySurface->Paste(rcClient.TopLeft(), &m_Buffer, &rcSource);

        return;
      }
    }
  }
  else{
    GetClientRect(&rcClient);
    CBrush brush;
    brush.CreateSolidBrush(RGB(192, 192, 192));
    dc.FillRect(&rcClient, &brush);
  }
}

void CMapexPreview::OnDestroy() 
{
	CStatic::OnDestroy();
	
  m_Buffer.Delete();
  m_Clipper.Delete();
}
