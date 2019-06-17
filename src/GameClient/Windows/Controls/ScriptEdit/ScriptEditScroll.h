// ScriptEditScroll.h: interface for the CScriptEditScroll class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTEDITSCROLL_H__7C4CD786_5A18_11D3_A037_B2F8384EE131__INCLUDED_)
#define AFX_SCRIPTEDITSCROLL_H__7C4CD786_5A18_11D3_A037_B2F8384EE131__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\ScrollControl.h"

class CScriptEditScroll : public CScrollControl  
{
  DECLARE_DYNAMIC(CScriptEditScroll)

public:
	BOOL Create(CRect &rcBound, CScrollControlLayout *pLayout, CWindow *pParent, BOOL bHorizontal);
	CScriptEditScroll();
	virtual ~CScriptEditScroll();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  enum Events{
    E_ACTIVATED = 0x0EFFFFFFF
  };
protected:
	virtual BOOL OnActivate();
};

#endif // !defined(AFX_SCRIPTEDITSCROLL_H__7C4CD786_5A18_11D3_A037_B2F8384EE131__INCLUDED_)
