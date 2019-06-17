// FontObject.h: interface for the CFontObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FONTOBJECT_H__C6DEDCF4_87AC_11D2_AB85_85B649344A60__INCLUDED_)
#define AFX_FONTOBJECT_H__C6DEDCF4_87AC_11D2_AB85_85B649344A60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFontObject : public CObject  
{
  DECLARE_DYNAMIC(CFontObject);

public:
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

public:
  // draws text string to the surface at some coords in some color
  virtual void PaintText(int nX, int nY, CString strText, CDDrawSurface *pDestSurface, DWORD dwColor = 0x0FFFFFFFF){ ASSERT(FALSE); };
  // returns size of character
	virtual CSize GetCharSize(char Character);
  // returns size of text string
	virtual CSize GetTextSize(CString strText);
  // returns default color of the font
	virtual DWORD GetColor();
  // sets defuult color for the font
	virtual void SetColor(DWORD dwColor);
  // deletes the object and its contents
	virtual void Delete();
  // creates font object
	virtual BOOL Create(DWORD dwColor);
  // constructor
	CFontObject();
  // destructor
	virtual ~CFontObject();

protected:
  // default color of the font
	DWORD m_dwColor;
};

extern CFontObject *g_pSystemFont;

#endif // !defined(AFX_FONTOBJECT_H__C6DEDCF4_87AC_11D2_AB85_85B649344A60__INCLUDED_)
