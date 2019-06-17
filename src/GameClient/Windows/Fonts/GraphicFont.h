// GraphicFont.h: interface for the CGraphicFont class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRAPHICFONT_H__C6DEDCF5_87AC_11D2_AB85_85B649344A60__INCLUDED_)
#define AFX_GRAPHICFONT_H__C6DEDCF5_87AC_11D2_AB85_85B649344A60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FontObject.h"

class CGraphicFont : public CFontObject  
{
  DECLARE_DYNAMIC(CGraphicFont)

public:
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

public:
  // returns size of the text sstring
	virtual CSize GetTextSize(CString strText);
  // returns size of one character
	virtual CSize GetCharSize(char Character);
  // draws text string to surface in some color
	virtual void PaintText(int nX, int nY, CString strText, CDDrawSurface *pDestSurface, DWORD dwColor = 0x0FFFFFFFF);
  // deletes the object
	virtual void Delete();
  // creates the object from file
	virtual BOOL Create(CArchiveFile File, DWORD dwColor = 0);
  // constructor
	CGraphicFont();
  // destructor
	virtual ~CGraphicFont();

private:
	// sizes of the buffer, used for the characters
	DWORD m_dwBufferWidth;
	DWORD m_dwBufferLines;

  // space between two lines of the text
	DWORD m_dwLineSpace;
  // space between two characters
	DWORD m_dwCharSpace;
  // last color used to draw the character
  // if we are to draw with different color
  // we have to refill the buffer surface with the new color
	DWORD m_dwPrevColor;
  // draws one character
  // fills the buffer surface with its color
  // sets its alpha channel for the alpha channel for the character
  // pastes the buffer surface to the destination
	int PaintCharacter(char Character, DWORD dwX, DWORD dwY, CDDrawSurface *pDestSurface,
    DWORD dwColor);
  // pointer to the aplha channel of the characters
  // this is whole image of the font
	BYTE * m_pAlphaChannel;
  struct tagSCharHeader{
    DWORD m_wASCIICode; // ASCII code of this character
    DWORD m_dwWidth;   // width of the character
    DWORD m_dwPosition;  // x coord of the character in the alpha channel image
  };
  typedef tagSCharHeader SCharHeader;

  struct tagSFNTHeader{
    char m_ID[16];
    DWORD m_dwWidth;  // width of the alpha channel image
    DWORD m_dwHeight; // height of the alpha channel image
    DWORD m_dwCharsCount; // number of chars in the font
    DWORD m_dwCharSpace;  // space between two characters
    DWORD m_dwLineSpace;  // space between two lines
    BYTE m_Reserved[36];  
  };
  typedef tagSFNTHeader SFNTHeader;

  // array of characters
  // organized by their ASCII codes
	SCharHeader * m_pCharacters[255];

  CPoint m_pPositions[256];
  DWORD m_pPrevColors[256];

  // count of characters in the font
	DWORD m_dwCharsCount;
  // maximal width of the character in the font
	DWORD m_dwMaxWidth;
  // headers of characters
	SCharHeader * m_pCharHeader;

	// width of the font image
  DWORD m_dwWidth;
  // height of the font image
	DWORD m_dwHeight;
  // buffer surface for drawing one character
	CScratchSurface m_CharSurface;

  friend class CGraphicCacheFont;
};

#endif // !defined(AFX_GRAPHICFONT_H__C6DEDCF5_87AC_11D2_AB85_85B649344A60__INCLUDED_)
