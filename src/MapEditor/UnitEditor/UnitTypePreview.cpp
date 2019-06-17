// UnitTypePreview.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "UnitTypePreview.h"

#include "..\DataObjects\EUnitAppearance.h"
#include "..\DataObjects\EUnitAnimation.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnitTypePreview

CUnitTypePreview::CUnitTypePreview()
{
  m_pUnitType = NULL;
}

CUnitTypePreview::~CUnitTypePreview()
{
}


BEGIN_MESSAGE_MAP(CUnitTypePreview, CStatic)
	//{{AFX_MSG_MAP(CUnitTypePreview)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnitTypePreview message handlers

void CUnitTypePreview::Create(CRect &rcBound, DWORD dwStyle, CWnd *pParent, UINT nID)
{
  m_Buffer.SetWidth(rcBound.Width());
  m_Buffer.SetHeight(rcBound.Height());
  m_Buffer.Create();

  CStatic::Create("", dwStyle, rcBound, pParent, nID);

  m_Clipper.Create(this);
}

void CUnitTypePreview::SetUnitType(CEUnitType *pUnitType)
{
  m_pUnitType = pUnitType;
  Invalidate();
}

void CUnitTypePreview::OnPaint() 
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

  CWaitCursor wait;

  if(m_pUnitType != NULL){
    CEUnitAppearanceType *pAppType;
    pAppType = m_pUnitType->GetDefaultAppearanceType();
    if(pAppType != NULL){
      CEUnitAppearance *pApp;
      pApp = pAppType->GetDefaultInstance();
      if(pApp != NULL){
        CEUnitAnimation *pAnim = NULL;
        int i;
        for(i = 0; i < 8; i++){
          if(pApp->GetDirection(i) != NULL){
            pAnim = pApp->GetDirection(i);
            break;
          }
        }
        if(pAnim != NULL){
          if(pAnim->GetFrame(0) != NULL){
            CRect rc(0, 0, rcClient.Width(), rcClient.Height());
            CRect rcSource(pAnim->GetFrame(0)->GetAllRect());

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
            m_Buffer.BltStretch(&rcSource, pAnim->GetFrame(0), NULL);
            g_pDDPrimarySurface->Paste(rcClient.TopLeft(), &m_Buffer, &rcSource);

            return;
          }
        }
      }
    }
  }
  
	// Do not call CStatic::OnPaint() for painting messages
}

void CUnitTypePreview::OnDestroy() 
{
	CStatic::OnDestroy();
	
  m_Buffer.Delete();
  m_Clipper.Delete();
}
