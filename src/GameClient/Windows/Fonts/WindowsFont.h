// WindowsFont.h: interface for the CWindowsFont class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINDOWSFONT_H__46C4B6D3_8827_11D2_AB86_8C39C64B4D60__INCLUDED_)
#define AFX_WINDOWSFONT_H__46C4B6D3_8827_11D2_AB86_8C39C64B4D60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FontObject.h"

class CWindowsFont : public CFontObject  
{
  DECLARE_DYNAMIC(CWindowsFont)

public:
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

public:
  // draws text string to surface in some color
	virtual void PaintText(int nX, int nY, CString strText, CDDrawSurface *pDestSurface, DWORD dwColor = 0x0FFFFFFFF);
  // returns size of the text string
	virtual CSize GetTextSize(CString strText);
  // returns size of one character
	virtual CSize GetCharSize(char Character);
  // deletes the object
	virtual void Delete();
  // creates the object for givven font face and point size
	virtual BOOL Create(CString strFace, DWORD dwPointSize, DWORD dwColor = 0x0FFFFFFFF);
  // constructor
	CWindowsFont();
  // destructor
	virtual ~CWindowsFont();

private:
  // point size of the Windows font
	DWORD m_dwPointSize;
  // face name of the Windows font
	CString m_strFaceName;
  // pointer to the font
	CFont * m_pFont;
};

#endif // !defined(AFX_WINDOWSFONT_H__46C4B6D3_8827_11D2_AB86_8C39C64B4D60__INCLUDED_)
