// GraphicCacheFont.h: interface for the CGraphicCacheFont class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRAPHICCACHEFONT_H__9987D753_C9B6_11D3_A8A9_00105ACA8325__INCLUDED_)
#define AFX_GRAPHICCACHEFONT_H__9987D753_C9B6_11D3_A8A9_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FontObject.h"

#include "GraphicFont.h"

class CGraphicCacheFont : public CFontObject  
{
  DECLARE_DYNAMIC(CGraphicCacheFont);

public:
  // paints the text from the cache to dest surface (it's quite quick, cause no alpha channel computations)
  virtual void PaintText(int nX, int nY, CString strText, CDDrawSurface *pDestSurface, DWORD dwColor = 0x0FFFFFFFF);

  // returns the size of given text (in pixels)
  virtual CSize GetTextSize(CString strText){
    return m_pGraphicFont->GetTextSize(strText);
  }
  // returns size of given character (in pixels)
  virtual CSize GetCharSize(char Character){
    return m_pGraphicFont->GetCharSize(Character);
  }

  // deletes the object
  virtual void Delete();
  // creates the font chache for given graphic font, text color and background color
	BOOL Create(CGraphicFont *pBaseFont, DWORD dwTextColor, DWORD dwBackgroundColor);
  // constructor & destructor
	CGraphicCacheFont();
	virtual ~CGraphicCacheFont();

  // debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
  // pointer to the original graphic font which we are cacheing
	CGraphicFont * m_pGraphicFont;

  // text color
  DWORD m_dwTextColor;
  // background color
  DWORD m_dwBackgroundColor;

  // the cache itself, graphic buffer where is the whole font
  CScratchSurface m_Cache;
};

#endif // !defined(AFX_GRAPHICCACHEFONT_H__9987D753_C9B6_11D3_A8A9_00105ACA8325__INCLUDED_)
