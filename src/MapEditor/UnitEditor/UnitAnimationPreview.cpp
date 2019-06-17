// UnitAnimationPreview.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "UnitAnimationPreview.h"

#include "..\DataObjects\EUnitAppearance.h"
#include "..\DataObjects\EUnitType.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnitAnimationPreview

CUnitAnimationPreview::CUnitAnimationPreview()
{
  m_pUnitAnimation = NULL;
  m_nTimer = 0;
}

CUnitAnimationPreview::~CUnitAnimationPreview()
{
}


BEGIN_MESSAGE_MAP(CUnitAnimationPreview, CStatic)
	//{{AFX_MSG_MAP(CUnitAnimationPreview)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnitAnimationPreview message handlers

void CUnitAnimationPreview::Create(CRect &rcBound, DWORD dwStyle, CWnd *pParent, UINT nID, CEUnitAppearanceType *pAppearanceType, DWORD dwDirection)
{
  m_pAppearanceType = pAppearanceType;
  m_dwDirection = dwDirection;
 
  m_Buffer.SetWidth(rcBound.Width());
  m_Buffer.SetHeight(rcBound.Height());
  m_Buffer.Create();

  CStatic::Create("", dwStyle, rcBound, pParent, nID);

  m_Clipper.Create(this);
}

void CUnitAnimationPreview::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
  g_pDDPrimarySurface->SetClipper(&m_Clipper);
  
  CRect rcClient;
  GetClientRect(&rcClient);
  ClientToScreen(&rcClient);

  if(m_pUnitAnimation != NULL){
    if(m_pUnitAnimation->GetFrame(m_dwFrame) != NULL){
      CRect rc(0, 0, rcClient.Width(), rcClient.Height());
      CRect rcSource(m_pUnitAnimation->GetFrame(m_dwFrame)->GetAllRect());
      
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
      m_Buffer.BltStretch(&rcSource, m_pUnitAnimation->GetFrame(m_dwFrame), NULL);
      CRect rcSrc(0, 0, rcClient.Width(), rcClient.Height() - 1);
      g_pDDPrimarySurface->Paste(rcClient.TopLeft(), &m_Buffer, &rcSrc);
      return;
    }
  }
  GetClientRect(&rcClient);
  CBrush brush;
  brush.CreateSolidBrush(RGB(192, 192, 192));
  dc.FillRect(&rcClient, &brush);
  brush.DeleteObject();
}

void CUnitAnimationPreview::SetAnimation(CEUnitAnimation *pAnimation)
{
  m_pUnitAnimation = pAnimation;

  if(m_nTimer != 0) KillTimer(m_nTimer);
  m_nTimer = 0;
  if(pAnimation != NULL){
    if(pAnimation->GetSpeed() > 0){
      m_nTimer = SetTimer(1, pAnimation->GetSpeed(), NULL);
    }
  }
  m_dwFrame = 0;

  Invalidate();
}

void CUnitAnimationPreview::OnDestroy() 
{
	CStatic::OnDestroy();
	
  if(m_nTimer != 0){
    KillTimer(m_nTimer);
    m_nTimer = 0;
  }

  m_Buffer.Delete();
  m_Clipper.Delete();
}

void CUnitAnimationPreview::OnTimer(UINT nIDEvent) 
{
  if(m_pUnitAnimation != NULL){
    m_dwFrame++;
    if(m_dwFrame >= m_pUnitAnimation->GetFrameNum())
      m_dwFrame = 0;
    Invalidate();
  }
  
	CStatic::OnTimer(nIDEvent);
}

void CUnitAnimationPreview::OnLButtonDown(UINT nFlags, CPoint point) 
{
  // load a graphics for it
  CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
    "Podporované formáty|*.anim;*.tif;*.tga|Animace (*.anim)|*.anim|Targa (*.tga)|*.tga|TIFF (*.tif)|*.tif|Všechny soubory|*.*||",
    this);

  CString str = "Nastavit vzhled jednotky - ";
  switch(m_dwDirection){
  case 0: str += "N"; break;
  case 1: str += "NE"; break;
  case 2: str += "E"; break;
  case 3: str += "SE"; break;
  case 4: str += "S"; break;
  case 5: str += "SW"; break;
  case 6: str += "W"; break;
  case 7: str += "NW"; break;
  }
  dlg.m_ofn.lpstrTitle = str;

  if(dlg.DoModal() == IDOK){
    if(m_pUnitAnimation == NULL){
      m_pUnitAnimation = m_pAppearanceType->GetInstance()->AddDirection(m_dwDirection);
    }
    try{
      m_pUnitAnimation->SetGraphics(m_pAppearanceType->GetUnitType()->GetID(),
        m_pAppearanceType->GetID(), m_dwDirection, dlg.GetPathName(), m_pAppearanceType->GetArchive());
      // set the anim to us - called for timing stuff to be done
      SetAnimation(m_pUnitAnimation);
    }
    catch(CException *e){
      SetAnimation(NULL);
      e->ReportError();
      e->Delete();
    }
  }
  
	CStatic::OnLButtonDown(nFlags, point);
}

void CUnitAnimationPreview::OnRButtonDown(UINT nFlags, CPoint point) 
{
  CString str = "Opravdu chcete smazat smìr ";
  switch(m_dwDirection){
  case 0: str += "N"; break;
  case 1: str += "NE"; break;
  case 2: str += "E"; break;
  case 3: str += "SE"; break;
  case 4: str += "S"; break;
  case 5: str += "SW"; break;
  case 6: str += "W"; break;
  case 7: str += "NW"; break;
  }
  str += " ?";
  if(AfxMessageBox(str, MB_YESNO) == IDYES){
    m_pAppearanceType->GetInstance()->RemoveDirection(m_dwDirection, m_pAppearanceType->GetArchive());
    SetAnimation(NULL);
  }
  
	CStatic::OnRButtonDown(nFlags, point);
}
