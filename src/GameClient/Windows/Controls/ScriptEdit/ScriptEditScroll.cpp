// ScriptEditScroll.cpp: implementation of the CScriptEditScroll class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptEditScroll.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CScriptEditScroll, CScrollControl)

CScriptEditScroll::CScriptEditScroll()
{

}

CScriptEditScroll::~CScriptEditScroll()
{
  Delete();
}

#ifdef _DEBUG

void CScriptEditScroll::AssertValid() const
{
  CScrollControl::AssertValid();
}

void CScriptEditScroll::Dump(CDumpContext &dc) const
{
  CScrollControl::Dump(dc);
}

#endif


BOOL CScriptEditScroll::OnActivate()
{
  InlayEvent(E_ACTIVATED, 0);
  CScrollControl::OnActivate();
  return TRUE;
}

BOOL CScriptEditScroll::Create(CRect &rcBound, CScrollControlLayout *pLayout, CWindow *pParent, BOOL bHorizontal)
{
  ASSERT_VALID(pLayout);

  if(!CScrollControl::Create(rcBound, pLayout, pParent, bHorizontal))
    return FALSE;

  return TRUE;
}
