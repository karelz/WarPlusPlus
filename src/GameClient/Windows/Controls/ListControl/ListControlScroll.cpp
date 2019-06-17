// ListControlScroll.cpp: implementation of the CListControlScroll class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ListControlScroll.h"

#include "ListControl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CListControlScroll, CScrollControl)

CListControlScroll::CListControlScroll()
{
}

CListControlScroll::~CListControlScroll()
{
}

#ifdef _DEBUG

void CListControlScroll::AssertValid() const
{
  CScrollControl::AssertValid();
}

void CListControlScroll::Dump(CDumpContext &dc) const
{
  CScrollControl::Dump(dc);
}

#endif


BOOL CListControlScroll::Create(CRect &rcBound, CScrollControlLayout *pLayout, CWindow *pParent)
{
  ASSERT_VALID(pLayout);

  if(!CScrollControl::Create(rcBound, pLayout, pParent))
    return FALSE;

  return TRUE;
}

void CListControlScroll::Delete()
{
  CScrollControl::Delete();
}

BOOL CListControlScroll::OnActivate()
{
  InlayEvent(E_ACTIVATED, 0);
  CScrollControl::OnActivate();
  return TRUE;
}
