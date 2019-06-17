// ListControlScroll.h: interface for the CListControlScroll class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LISTCONTROLSCROLL_H__22FDD4EC_5081_11D3_A02E_DD87D6C1E031__INCLUDED_)
#define AFX_LISTCONTROLSCROLL_H__22FDD4EC_5081_11D3_A02E_DD87D6C1E031__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\ScrollControl\ScrollControl.h"

class CListControl;

class CListControlScroll : public CScrollControl  
{
  DECLARE_DYNAMIC(CListControlScroll);

public:
	virtual void Delete();
	BOOL Create(CRect &rcBound, CScrollControlLayout *pLayout, CWindow *pParent);
	CListControlScroll();
	virtual ~CListControlScroll();

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

#endif // !defined(AFX_LISTCONTROLSCROLL_H__22FDD4EC_5081_11D3_A02E_DD87D6C1E031__INCLUDED_)
