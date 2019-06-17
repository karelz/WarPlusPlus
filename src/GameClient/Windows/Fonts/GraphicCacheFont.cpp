// GraphicCacheFont.cpp: implementation of the CGraphicCacheFont class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GraphicCacheFont.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CGraphicCacheFont, CFontObject);

CGraphicCacheFont::CGraphicCacheFont()
{
  m_pGraphicFont = NULL;
}

CGraphicCacheFont::~CGraphicCacheFont()
{
}

#ifdef _DEBUG

void CGraphicCacheFont::AssertValid() const
{
  CFontObject::AssertValid();

  ASSERT_VALID(m_pGraphicFont);
}

void CGraphicCacheFont::Dump(CDumpContext &dc) const
{
  CFontObject::Dump(dc);
}

#endif


BOOL CGraphicCacheFont::Create(CGraphicFont *pGraphicFont, DWORD dwTextColor, DWORD dwBackgroundColor)
{
  ASSERT_VALID(pGraphicFont);

  // copy the arguments to our variables
  m_pGraphicFont = pGraphicFont;
  m_dwTextColor = dwTextColor;
  m_dwBackgroundColor = dwBackgroundColor;

  // create the cache buffer
  m_Cache.SetWidth(m_pGraphicFont->m_dwBufferWidth);
  m_Cache.SetHeight((m_pGraphicFont->m_dwHeight + m_pGraphicFont->m_dwLineSpace) * m_pGraphicFont->m_dwBufferLines);
  m_Cache.SetAlphaChannel(FALSE);
  m_Cache.Create();

  // fill the cache background
  m_Cache.Fill(m_dwBackgroundColor);

  // draw all characters to the cache
  int i;
  CString strText;
  for(i = 0; i < (int)(m_pGraphicFont->m_dwCharsCount); i++){
		WORD ascii;
    ascii = (WORD)m_pGraphicFont->m_pCharHeader[i].m_wASCIICode;
		strText = (char)ascii;

    m_pGraphicFont->PaintText(m_pGraphicFont->m_pPositions[ascii].x,
			m_pGraphicFont->m_pPositions[ascii].y, strText, &m_Cache, m_dwTextColor);
  }

  return CFontObject::Create(m_dwTextColor);
}

void CGraphicCacheFont::Delete()
{
  // delete the cache
  m_Cache.Delete();

  m_pGraphicFont = NULL;

  CFontObject::Delete();
}

void CGraphicCacheFont::PaintText(int nX, int nY, CString strText, CDDrawSurface *pDestSurface, DWORD dwColor)
{
  int nXPos, i, nLen;
  LPCSTR pStr;
  CSize size;

  nLen = strText.GetLength();
  if(nLen == 0) return;

  pStr = strText;

  // try if the text is not out of the clip rect of the
  // destination surface
  // if so we don't have to draw anything
  size = GetTextSize(strText);
  CRect rc(nX, nY, nX + size.cx, nY + size.cy);
  CRect rcClip; pDestSurface->GetClipRect(&rcClip);
  rc.IntersectRect(&rc, &rcClip);
  if(rc.IsRectEmpty()) return;

	CRect rcSource;

  nXPos = nX;
  for(i = 0; i < nLen; i++){
    CGraphicFont::SCharHeader *pCharHeader = m_pGraphicFont->m_pCharacters[(unsigned char)pStr[i]];
    if(pCharHeader == NULL) continue;
    if((nXPos + (int)(pCharHeader->m_dwWidth) >= rcClip.left) && (nXPos < rcClip.right)){
      rcSource.left = m_pGraphicFont->m_pPositions[(unsigned char)pStr[i]].x;
			rcSource.top = m_pGraphicFont->m_pPositions[(unsigned char)pStr[i]].y;
			rcSource.right = rcSource.left + pCharHeader->m_dwWidth + m_pGraphicFont->m_dwCharSpace;
			rcSource.bottom = rcSource.top + m_pGraphicFont->m_dwHeight + m_pGraphicFont->m_dwLineSpace;

      pDestSurface->Paste(nXPos, nY, &m_Cache, &rcSource);
      nXPos += pCharHeader->m_dwWidth;
    }
    else
      nXPos += pCharHeader->m_dwWidth;
    nXPos += m_pGraphicFont->m_dwCharSpace;
  }  
}
