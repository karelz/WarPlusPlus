// ResourcePreview.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "ResourcePreview.h"
#include "Common\Map\Map.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourcePreview

CResourcePreview::CResourcePreview()
{
  m_pResource = NULL;
}

CResourcePreview::~CResourcePreview()
{
}


BEGIN_MESSAGE_MAP(CResourcePreview, CStatic)
	//{{AFX_MSG_MAP(CResourcePreview)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourcePreview message handlers

void CResourcePreview::Create(CRect &rcBound, DWORD dwStyle, CWnd *pParent, UINT nID)
{
  m_Buffer.SetWidth(RESOURCE_ICON_WIDTH);
  m_Buffer.SetHeight(RESOURCE_ICON_HEIGHT);
  m_Buffer.Create();

  CStatic::Create("", dwStyle, rcBound, pParent, nID);

  m_Clipper.Create(this);
}

void CResourcePreview::SetResource(CEResource *pResource)
{
  m_pResource = pResource;
  Invalidate();
}

void CResourcePreview::OnDestroy() 
{
	CStatic::OnDestroy();
  m_Buffer.Delete();
  m_Clipper.Delete();
}

void CResourcePreview::OnPaint() 
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

  if(m_pResource != NULL){
    m_Buffer.Fill(RGB32(192, 192, 192));
    m_Buffer.Paste(0, 0, m_pResource->GetIcon());
    g_pDDPrimarySurface->Paste(rcClient.TopLeft(), &m_Buffer);
  }
}
