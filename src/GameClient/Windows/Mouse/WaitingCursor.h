// WaitingCursor.h: interface for the CWaitingCursor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAITINGCURSOR_H__F7E2F251_7D22_11D4_B0F4_004F49068BD6__INCLUDED_)
#define AFX_WAITINGCURSOR_H__F7E2F251_7D22_11D4_B0F4_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Cursor.h"

class CWaitingCursor  
{
public:
	CWaitingCursor();
	virtual ~CWaitingCursor();

private:
  CCursor *m_pCursor;
};

#endif // !defined(AFX_WAITINGCURSOR_H__F7E2F251_7D22_11D4_B0F4_004F49068BD6__INCLUDED_)
