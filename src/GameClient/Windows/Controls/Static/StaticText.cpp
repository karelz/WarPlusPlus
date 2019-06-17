// StaticText.cpp: implementation of the CStaticText class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StaticText.h"

#include "..\Layouts.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CStaticText, CWindow)

CStaticText::CStaticText()
{
  m_pLayout = NULL;
  m_eAlign = AlignLeft;
}

CStaticText::~CStaticText()
{

}

#ifdef _DEBUG

void CStaticText::AssertValid() const
{
  CWindow::AssertValid();
}

void CStaticText::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);

  dc << "Text : " << m_strText << "\n";
}

#endif


BOOL CStaticText::Create(CRect &rcBound, CStaticTextLayout *pLayout, CString strText, CWindow *pParent)
{
  if(pLayout == NULL){
    m_pLayout = CLayouts::m_pDefaults->GetStaticTextLayout();
  }
  else{
    m_pLayout = pLayout;
  }
  ASSERT_VALID(m_pLayout);

  m_bTabStop = FALSE;
  
  m_strText = strText;
  ComputeLines();

  m_eAlign = (EAlignType)m_pLayout->m_eAlign;

  // resize the window to fit the lines
  CRect rcWindow(rcBound);
  if(!(m_eAlign & AlignNoYResize)){
    CSize CharSize = m_pLayout->GetFont()->GetCharSize('A');
    if(m_aLines.GetSize() == 0){
      rcWindow.bottom = rcWindow.top + CharSize.cy;
    }
    else{
      rcWindow.bottom = rcWindow.top + m_aLines.GetSize() * CharSize.cy;
    }
  }

  m_bTransparent = TRUE;

  if(!CWindow::Create(&rcWindow, pParent))
    return FALSE;

  return TRUE;
}

void CStaticText::Delete()
{
  CWindow::Delete();

  m_pLayout = NULL;
  m_strText.Empty();
  m_aLines.RemoveAll();
}

void CStaticText::SetText(CString strText)
{
  ASSERT_VALID(this);

  m_strText = strText;
  ComputeLines();
  UpdateRect();
}

CString CStaticText::GetText()
{
  ASSERT_VALID(this);

  return m_strText;
}

void CStaticText::SetAlign(EAlignType eAlign)
{
  ASSERT_VALID(this);

  m_eAlign = eAlign;
  UpdateRect();
}

CStaticText::EAlignType CStaticText::GetAlign()
{
  ASSERT_VALID(this);

  return m_eAlign;
}

void CStaticText::Draw(CDDrawSurface *pSurface, CRect *pRectBound)
{
  CFontObject *pFont = m_pLayout->GetFont();
  CPoint ptTopLeft(0, 0);

  int i;
  for(i = 0; i < m_aLines.GetSize(); i++){
    CSize sizeText = pFont->GetTextSize(m_aLines[i]);
    switch(m_eAlign){
    case AlignLeft:
      break;
    case AlignRight:
      ptTopLeft.x = GetClientRect()->right - sizeText.cx;
      break;
    case AlignCenter:
      ptTopLeft.x = (GetClientRect()->Width() - sizeText.cx) / 2;
      break;
    }
    
    pFont->PaintText(ptTopLeft.x, ptTopLeft.y, m_aLines[i], pSurface,
      m_pLayout->m_dwTextColor);

    ptTopLeft.y += sizeText.cy;
  }
}

void CStaticText::ComputeLines()
{
  m_aLines.RemoveAll();

  int nPos, nLength;
  CString strLine;

  nLength = m_strText.GetLength();
  nPos = 0;
  strLine.Empty();

  while(nPos < nLength){
    if(m_strText[nPos] == '\n'){
      m_aLines.Add(strLine);
      strLine.Empty();
    }
    else{
      strLine += m_strText[nPos];
    }
    nPos++;
  }
  if(!strLine.IsEmpty()) m_aLines.Add(strLine);
}

CSize CStaticText::GetControlSize(CStaticTextLayout *pLayout, CString strText)
{
  CStaticText ThisControl;

  if(pLayout == NULL){
    ThisControl.m_pLayout = CLayouts::m_pDefaults->GetStaticTextLayout();
  }
  else{
    ThisControl.m_pLayout = pLayout;
  }
  ASSERT_VALID(ThisControl.m_pLayout);

  ThisControl.m_strText = strText;

  ThisControl.ComputeLines();

  CSize sizeText(0, 0) , sizeLine;
  int i;
  for(i = 0; i < ThisControl.m_aLines.GetSize(); i++){
    sizeLine = ThisControl.m_pLayout->GetFont()->GetTextSize(ThisControl.m_aLines[i]);
    if(sizeText.cx < sizeLine.cx) sizeText.cx = sizeLine.cx;
    sizeText.cy += sizeLine.cy;
  }

  ThisControl.m_aLines.RemoveAll();
  ThisControl.m_pLayout = NULL;
  ThisControl.m_strText.Empty();

  return sizeText;
}
