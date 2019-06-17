// StaticText.h: interface for the CStaticText class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STATICTEXT_H__22FDD4E3_5081_11D3_A02E_DD87D6C1E031__INCLUDED_)
#define AFX_STATICTEXT_H__22FDD4E3_5081_11D3_A02E_DD87D6C1E031__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\Window\Window.h"
#include "StaticTextLayout.h"

class CStaticText : public CWindow
{
  DECLARE_DYNAMIC(CStaticText)

public:
	static CSize GetControlSize(CStaticTextLayout *pLayout, CString strText);
  // draws the window -> called from the windows system
	virtual void Draw(CDDrawSurface *pSurface, CRect *pRectBound);

  enum EAlignType{  // alignment of the text in the window
    AlignLeft = 0,  // align text to the left
    AlignRight = 1, // align text to the right
    AlignCenter = 2, // align text to center
    AlignNoYResize = 16, // no y resizing enabled
    AlignYResize = 0     // resize the window, so that all text line will fit in
  };

	// returns the alignment
  EAlignType GetAlign();
  // sets the alignment
	void SetAlign(EAlignType eAlign);
  // returns the text in the control
	CString GetText();
  // sets the text to this control
	void SetText(CString strText);
  // deletes the object
	virtual void Delete();
  // creates the static text control
	BOOL Create(CRect &rcBound, CStaticTextLayout *pLayout, CString strText, CWindow *pParent);

  // constructor
	CStaticText();
  // destructor
	virtual ~CStaticText();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
	virtual void ComputeLines();
	CStringArray m_aLines;
  // alignment
	EAlignType m_eAlign;
  // text to draw
  CString m_strText;
private:
  // layout object for this statictext
	CStaticTextLayout * m_pLayout;
};

#endif // !defined(AFX_STATICTEXT_H__22FDD4E3_5081_11D3_A02E_DD87D6C1E031__INCLUDED_)
