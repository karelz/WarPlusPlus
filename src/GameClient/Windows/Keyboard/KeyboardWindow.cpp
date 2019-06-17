// KeyboardWindow.cpp : implementation file
//

#include "stdafx.h"
#include "KeyboardWindow.h"
#include "Keyboard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeyboardWindow

IMPLEMENT_DYNCREATE(CKeyboardWindow, CEventWindow)

CKeyboardWindow::CKeyboardWindow()
{
  m_pKeyboard = NULL;
}

CKeyboardWindow::~CKeyboardWindow()
{
  if ( m_pKeyboard != NULL )
    m_pKeyboard->m_pWindow = NULL;
}


BEGIN_MESSAGE_MAP(CKeyboardWindow, CEventWindow)
	//{{AFX_MSG_MAP(CKeyboardWindow)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
  ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyboardWindow message handlers

#ifdef _DEBUG

void CKeyboardWindow::AssertValid() const
{
  CEventWindow::AssertValid();
}

void CKeyboardWindow::Dump(CDumpContext &dc) const
{
  CEventWindow::Dump(dc);
}

#endif

void CKeyboardWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  if(m_pKeyboard != NULL){
    CKeyboard::SKeyInfo *pInfo = new CKeyboard::SKeyInfo();
    pInfo->nChar = nChar; pInfo->dwFlags = 0x0;
    if(GetKeyState(VK_LSHIFT) & 0x8000) pInfo->dwFlags |= CKeyboard::LShiftDown;
    if(GetKeyState(VK_RSHIFT) & 0x8000) pInfo->dwFlags |= CKeyboard::RShiftDown;
    if(GetKeyState(VK_LCONTROL) & 0x8000) pInfo->dwFlags |= CKeyboard::LCtrlDown;
    if(GetKeyState(VK_RCONTROL) & 0x8000) pInfo->dwFlags |= CKeyboard::RCtrlDown;
    if(GetKeyState(VK_LMENU) & 0x8000) pInfo->dwFlags |= CKeyboard::LAltDown;
    if(GetKeyState(VK_RMENU) & 0x8000) pInfo->dwFlags |= CKeyboard::RAltDown;
    m_pKeyboard->m_KeyboardNotifier.InlayEvent(CKeyboard::E_KEYDOWN, (DWORD)pInfo, m_pKeyboard);
  }

  CEventWindow::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CKeyboardWindow::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  if(m_pKeyboard != NULL){
    CKeyboard::SKeyInfo *pInfo = new CKeyboard::SKeyInfo();
    pInfo->nChar = nChar; pInfo->dwFlags = 0x0;
    if(GetKeyState(VK_LSHIFT) & 0x8000) pInfo->dwFlags |= CKeyboard::LShiftDown;
    if(GetKeyState(VK_RSHIFT) & 0x8000) pInfo->dwFlags |= CKeyboard::RShiftDown;
    if(GetKeyState(VK_LCONTROL) & 0x8000) pInfo->dwFlags |= CKeyboard::LCtrlDown;
    if(GetKeyState(VK_RCONTROL) & 0x8000) pInfo->dwFlags |= CKeyboard::RCtrlDown;
    if(GetKeyState(VK_LMENU) & 0x8000) pInfo->dwFlags |= CKeyboard::LAltDown;
    if(GetKeyState(VK_RMENU) & 0x8000) pInfo->dwFlags |= CKeyboard::RAltDown;
    m_pKeyboard->m_KeyboardNotifier.InlayEvent(CKeyboard::E_KEYUP, (DWORD)pInfo, m_pKeyboard);
  }
  
	CEventWindow::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CKeyboardWindow::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  if(m_pKeyboard != NULL){
    CKeyboard::SKeyInfo *pInfo = new CKeyboard::SKeyInfo();
    pInfo->nChar = nChar; pInfo->dwFlags = 0x0;
    if(GetKeyState(VK_LSHIFT) & 0x8000) pInfo->dwFlags |= CKeyboard::LShiftDown;
    if(GetKeyState(VK_RSHIFT) & 0x8000) pInfo->dwFlags |= CKeyboard::RShiftDown;
    if(GetKeyState(VK_LCONTROL) & 0x8000) pInfo->dwFlags |= CKeyboard::LCtrlDown;
    if(GetKeyState(VK_RCONTROL) & 0x8000) pInfo->dwFlags |= CKeyboard::RCtrlDown;
    if(GetKeyState(VK_LMENU) & 0x8000) pInfo->dwFlags |= CKeyboard::LAltDown;
    if(GetKeyState(VK_RMENU) & 0x8000) pInfo->dwFlags |= CKeyboard::RAltDown;
    m_pKeyboard->m_KeyboardNotifier.InlayEvent(CKeyboard::E_CHAR, (DWORD)pInfo, m_pKeyboard);
  }

  CEventWindow::OnKeyDown(nChar, nRepCnt, nFlags);
}