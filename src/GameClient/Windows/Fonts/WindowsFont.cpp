// WindowsFont.cpp: implementation of the CWindowsFont class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WindowsFont.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWindowsFont, CFontObject);

CWindowsFont::CWindowsFont()
{
  m_pFont = NULL;
  m_dwPointSize = 0;
}

CWindowsFont::~CWindowsFont()
{

}

//////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CWindowsFont::AssertValid() const
{
  CFontObject::AssertValid();
}

void CWindowsFont::Dump(CDumpContext &dc) const
{
  CFontObject::Dump(dc);

  dc << "Face name : " << m_strFaceName << " with point size " << m_dwPointSize << "\n";
}

#endif


BOOL CWindowsFont::Create(CString strFace, DWORD dwPointSize, DWORD dwColor)
{
  if(!CFontObject::Create(dwColor)) return FALSE;

  m_strFaceName = strFace; m_dwPointSize = dwPointSize;
  m_pFont = new CFont();

  if(!m_pFont->CreatePointFont(dwPointSize, strFace)) return FALSE;

  return TRUE;
}

void CWindowsFont::Delete()
{
  if(m_pFont){
    delete m_pFont;
    m_pFont = NULL;
  }
  
  CFontObject::Delete();
}

CSize CWindowsFont::GetCharSize(char Character)
{
  CDC *pDC;

  // get the DC for the entire screen
  // we don't know the DC of destination
  pDC = CDC::FromHandle(::GetWindowDC(NULL));

  return pDC->GetTextExtent(&Character, 1);
}

CSize CWindowsFont::GetTextSize(CString strText)
{
  CDC *pDC;

  pDC = CDC::FromHandle(::GetWindowDC(NULL));

  return pDC->GetTextExtent(strText);
}

void CWindowsFont::PaintText(int nX, int nY, CString strText, CDDrawSurface *pDestSurface, DWORD dwColor)
{
  COLORREF oldColor, newColor;
  int nOldBkMode;

  if(strText.GetLength() == 0) return;
  if(dwColor == 0x0FFFFFFFF) dwColor = m_dwColor;

  newColor = RGB((dwColor & 0x0FF0000) >> 16, (dwColor & 0x0FF00) >> 8,
    (dwColor & 0x0FF));

  CRect rc(nX, nY, nX + pDestSurface->GetWidth(), nY + pDestSurface->GetHeight());
  rc.OffsetRect(pDestSurface->GetTransformation());
//  CRect rcClip;
//  pDestSurface->GetClipRect(&rcClip);
//  rc.IntersectRect(&rc, &rcClip);

  CDC *pDC = pDestSurface->GetDC();

  oldColor = pDC->SetTextColor(newColor);
  nOldBkMode = pDC->SetBkMode(TRANSPARENT);

  pDC->DrawText(strText, &rc, DT_LEFT);
  
  pDC->SetBkMode(nOldBkMode);
  pDC->SetTextColor(oldColor);

  pDestSurface->ReleaseDC(pDC);
}
