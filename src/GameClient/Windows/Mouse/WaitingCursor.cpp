// WaitingCursor.cpp: implementation of the CWaitingCursor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WaitingCursor.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include "Mouse.h"

CWaitingCursor::CWaitingCursor()
{
  m_pCursor = NULL;
  if(g_pMouse != NULL){
    m_pCursor = g_pMouse->SetDefaultCursor(g_pMouse->GetWaitingCursor());
    g_pMouse->RestoreCursor();
  }
}

CWaitingCursor::~CWaitingCursor()
{
  if(g_pMouse != NULL){
    g_pMouse->SetDefaultCursor(m_pCursor);
    g_pMouse->RestoreCursor();
  }
}
