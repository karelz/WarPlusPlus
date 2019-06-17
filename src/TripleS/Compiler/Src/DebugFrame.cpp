/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Okno pro vypis debugovacich vypisu prekladace,
 *          definovano je-li definovano makro _DEBUG a je-li
 *          nastaven priznak pro debugovani pred zacatkem
 *          kompilace (CCompiler::SetDebug()).
 * 
 ***********************************************************/

#include "stdafx.h"
#include "CompilerKernel.h"
#include "DebugFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDebugFrame

#ifdef _DEBUG

IMPLEMENT_DYNCREATE(CDebugFrame, CFrameWnd)

CDebugFrame::CDebugFrame()
{
    m_pwndDebugView = NULL;
}

CDebugFrame::~CDebugFrame()
{
}

bool CDebugFrame::Pause()
{
    ASSERT(m_pwndDebugView!=NULL);

    MSG msg;
    HWND hWnd=GetSafeHwnd();
    HWND hMainWnd=::AfxGetMainWnd()->GetSafeHwnd();
    HWND hREView=m_pwndDebugView->GetSafeHwnd();

    while (true)
    {
        if (PeekMessage(&msg, hWnd, 0, (UINT)-1, PM_REMOVE))
        {
            if (msg.message==WM_KEYDOWN && msg.wParam==VK_RETURN) return true;
            if (msg.message==WM_KEYDOWN && msg.wParam==VK_ESCAPE) return false;
            
            ::TranslateMessage(&msg); 
            ::DispatchMessage(&msg);

        }

        if (PeekMessage(&msg, hMainWnd, 0, (UINT)-1, PM_REMOVE))
        {   
            ::TranslateMessage(&msg); 
            ::DispatchMessage(&msg);
        }

        if (PeekMessage(&msg, hREView, 0, (UINT)-1, PM_REMOVE))
        {   
            ::TranslateMessage(&msg); 
            ::DispatchMessage(&msg);
        }
    }
}

BEGIN_MESSAGE_MAP(CDebugFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CDebugFrame)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#endif

/////////////////////////////////////////////////////////////////////////////
// CDebugFrame message handlers



