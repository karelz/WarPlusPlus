// HelpBrowserScroll.h: interface for the CHelpBrowserScroll class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HELPBROWSERSCROLL_H__AC9A7049_7BFD_11D4_B0F0_004F49068BD6__INCLUDED_)
#define AFX_HELPBROWSERSCROLL_H__AC9A7049_7BFD_11D4_B0F0_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CHelpBrowserScroll : public CScrollControl  
{
public:
	BOOL Create(CRect &rcBound, CScrollControlLayout *pLayout, CWindow *pParent, BOOL bHorizontal);
	CHelpBrowserScroll();
	virtual ~CHelpBrowserScroll();

  enum Events{
    E_ACTIVATED = 0x0EFFFFFFF
  };
protected:
	virtual BOOL OnActivate();
};

#endif // !defined(AFX_HELPBROWSERSCROLL_H__AC9A7049_7BFD_11D4_B0F0_004F49068BD6__INCLUDED_)
