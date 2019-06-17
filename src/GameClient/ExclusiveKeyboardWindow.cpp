// ExclusiveKeyboardWindow.cpp : implementation file
//

#include "stdafx.h"
#include "gameclient.h"
#include "ExclusiveKeyboardWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExclusiveKeyboardWindow

CExclusiveKeyboardWindow::CExclusiveKeyboardWindow()
{
}

CExclusiveKeyboardWindow::~CExclusiveKeyboardWindow()
{
}


BEGIN_MESSAGE_MAP(CExclusiveKeyboardWindow, CWnd)
	//{{AFX_MSG_MAP(CExclusiveKeyboardWindow)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CExclusiveKeyboardWindow message handlers

void CExclusiveKeyboardWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if ( m_pSendToWnd != NULL )
	{
		m_pSendToWnd->PostMessage ( WM_KEYDOWN, (WPARAM)nChar, nRepCnt | (nFlags << 16 ) );
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CExclusiveKeyboardWindow::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if ( m_pSendToWnd != NULL )
	{
		m_pSendToWnd->PostMessage ( WM_KEYUP, (WPARAM)nChar, nRepCnt | (nFlags << 16 ) );
	}
	
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CExclusiveKeyboardWindow::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if ( m_pSendToWnd != NULL )
	{
//		m_pSendToWnd->PostMessage ( WM_CHAR,  (WPARAM)nChar, nRepCnt | (nFlags << 16 ) );
	}

	CWnd::OnChar(nChar, nRepCnt, nFlags);
}
