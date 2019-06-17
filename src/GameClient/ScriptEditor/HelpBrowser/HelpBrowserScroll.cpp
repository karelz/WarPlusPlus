// HelpBrowserScroll.cpp: implementation of the CHelpBrowserScroll class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HelpBrowserScroll.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHelpBrowserScroll::CHelpBrowserScroll()
{

}

CHelpBrowserScroll::~CHelpBrowserScroll()
{

}

BOOL CHelpBrowserScroll::OnActivate()
{
  InlayEvent(E_ACTIVATED, 0);
  CScrollControl::OnActivate();
  return TRUE;
}

BOOL CHelpBrowserScroll::Create(CRect &rcBound, CScrollControlLayout *pLayout, CWindow *pParent, BOOL bHorizontal)
{
  ASSERT_VALID(pLayout);

  if(!CScrollControl::Create(rcBound, pLayout, pParent, bHorizontal))
    return FALSE;

  return TRUE;
}
