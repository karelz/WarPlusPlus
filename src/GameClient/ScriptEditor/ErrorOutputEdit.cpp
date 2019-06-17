// ErrorOutputEdit.cpp: implementation of the CErrorOutputEdit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\gameclient.h"
#include "ErrorOutputEdit.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CErrorOutputEdit, CScriptEdit);

BEGIN_OBSERVER_MAP(CErrorOutputEdit, CScriptEdit)
  BEGIN_MOUSE()
    ON_LBUTTONDBLCLK()
  END_MOUSE()

  BEGIN_KEYBOARD()
    ON_KEYDOWN()
  END_KEYBOARD()
END_OBSERVER_MAP(CErrorOutputEdit, CScriptEdit)

CErrorOutputEdit::CErrorOutputEdit()
{

}

CErrorOutputEdit::~CErrorOutputEdit()
{

}

// Debug functions
#ifdef _DEBUG

void CErrorOutputEdit::AssertValid() const
{
  CScriptEdit::AssertValid();
}

void CErrorOutputEdit::Dump(CDumpContext &dc) const
{
  CScriptEdit::Dump(dc);
}

#endif


// Creation
void CErrorOutputEdit::Create(CRect &rcBound, CScriptEditLayout *pLayout, CWindow *pParent)
{
  // wants double clicks
  m_bDoubleClk = TRUE;

  // create the script edit window
  CScriptEdit::Create(rcBound, pLayout, pParent);
}

// Delete
void CErrorOutputEdit::Delete()
{
  // delete the base class
  CScriptEdit::Delete();
}

void CErrorOutputEdit::OnLButtonDblClk(CPoint point)
{
  // just inlay event
  InlayEvent(E_LineSelected, 0);
}

BOOL CErrorOutputEdit::OnKeyDown(DWORD dwKey, DWORD dwFlags)
{
  switch(dwKey){
  case VK_RETURN:
    if(dwFlags == 0){
      InlayEvent(E_LineSelected, 0);
      return TRUE;
    }
    break;
  }

  return FALSE;
}