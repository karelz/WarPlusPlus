// DesktopWindow.cpp: implementation of the CDesktopWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DesktopWindow.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CDesktopWindow, CWindow);

CDesktopWindow::CDesktopWindow()
{
  m_bHasImage = FALSE;
}

CDesktopWindow::~CDesktopWindow()
{

}

#ifdef _DEBUG

void CDesktopWindow::AssertValid() const
{
  CWindow::AssertValid();
}

void CDesktopWindow::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);
}

#endif

void CDesktopWindow::Draw(CDDrawSurface *pSurface, CRect *pRect)
{
  // just fill it with black
  pSurface->Fill(0, pRect);

  if(m_bHasImage){
    CRect *pScreen = g_pDDPrimarySurface->GetScreenRect();
    CRect *pImage = m_Image.GetAllRect();
    pSurface->Paste((pScreen->Width() - pImage->Width()) / 2, (pScreen->Height() - pImage->Height()) / 2, &m_Image);
  }
}

void CDesktopWindow::Create()
{
  ASSERT_VALID(g_pDDPrimarySurface);

  VERIFY(CWindow::Create(g_pDDPrimarySurface->GetScreenRect(), NULL));
}

void CDesktopWindow::Delete()
{
  if(m_bHasImage){
    m_bHasImage = FALSE;
    m_Image.Delete();
  }
  CWindow::Delete();
}

void CDesktopWindow::SetImage(CArchiveFile file)
{
  if(m_bHasImage){
    m_bHasImage = FALSE;
    m_Image.Delete();
  }

  m_Image.Create(file);
  m_bHasImage = TRUE;
}

void CDesktopWindow::ClearImage()
{
  if(m_bHasImage){
    m_bHasImage = FALSE;
    m_Image.Delete();
  }
}