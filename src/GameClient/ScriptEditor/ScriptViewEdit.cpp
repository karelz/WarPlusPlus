// ScriptViewEdit.cpp: implementation of the CScriptViewEdit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\gameclient.h"
#include "ScriptViewEdit.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CScriptViewEdit, CScriptEdit);

BEGIN_OBSERVER_MAP(CScriptViewEdit, CScriptEdit)
  BEGIN_MOUSE()
    ON_RBUTTONUP()
  END_MOUSE()
END_OBSERVER_MAP(CScriptViewEdit, CScriptEdit)

CScriptViewEdit::CScriptViewEdit()
{

}

CScriptViewEdit::~CScriptViewEdit()
{

}


#ifdef _DEBUG

void CScriptViewEdit::AssertValid() const
{
  CScriptEdit::AssertValid();
}

void CScriptViewEdit::Dump(CDumpContext &dc) const
{
  CScriptEdit::Dump(dc);
}

#endif


void CScriptViewEdit::OnRButtonUp(CPoint point)
{
  point.Offset(GetWindowScreenPosition().TopLeft());
  InlayEvent(E_CONTEXTMENU, MOUSE_MAKEPOINT(point.x, point.y));
}
