// Keyboard.cpp: implementation of the CKeyboard class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Keyboard.h"
#include "..\Window\Window.h"
#include "KeyAccelerators.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CKeyboard *g_pKeyboard = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CKeyboard, CNotifier)

BEGIN_OBSERVER_MAP(CKeyboard, CNotifier)
  BEGIN_ABORT()
    ON_ABORT()
  END_ABORT()
  BEGIN_NOTIFIER(KeyboardNotifierID)
    EVENT(E_KEYDOWN)
      OnKeyDown(dwParam);
      return TRUE;
    EVENT(E_KEYUP)
      OnKeyUp(dwParam);
      return TRUE;
    EVENT(E_CHAR)
      OnChar(dwParam);
      return TRUE;
  END_NOTIFIER()
END_OBSERVER_MAP(CKeyboard, CNotifier)

CKeyboard::CKeyboard()
{
  m_pWindow = NULL;
}

CKeyboard::~CKeyboard()
{

}

////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CKeyboard::AssertValid() const
{
  CNotifier::AssertValid();
}

void CKeyboard::Dump(CDumpContext &dc) const
{
  CNotifier::Dump(dc);
}

#endif


BOOL CKeyboard::Init(CKeyboardWindow * pKeyboardWindow)
{
  ASSERT(g_pKeyboard == NULL);

  CKeyboard *pKeyboard;
  pKeyboard = new CKeyboard();
  return pKeyboard->Create(pKeyboardWindow);
}

void CKeyboard::Close()
{
  if(g_pKeyboard == NULL) return;

  CKeyboard *pKeyboard;
  pKeyboard = g_pKeyboard;
  pKeyboard->Delete();
  delete pKeyboard;
}

BOOL CKeyboard::Create(CKeyboardWindow * pKeyboardWindow)
{
  g_pKeyboard = this;
  m_pWindow = pKeyboardWindow;
  m_pWindow->m_pKeyboard = this;

  g_AbortNotifier.Connect(this);

  // connect us to the notifier
  m_KeyboardNotifier.Connect(this, KeyboardNotifierID);

  return TRUE;
}

void CKeyboard::Delete()
{
  m_KeyboardNotifier.Disconnect(this);

  g_AbortNotifier.Disconnect(this);
  
  CNotifier::Delete();
  g_pKeyboard = NULL;
  if(m_pWindow){
    m_pWindow->m_pKeyboard = NULL;
  }
  m_pWindow = NULL;
}

void CKeyboard::Connect(CWindow *pWindow, DWORD dwNotID)
{
  CNotifier::Connect(pWindow, dwNotID);

  SObserverNode *pNode;
  SKeyboardConnectionData *pConData;
  pNode = FindObserverNode(pWindow);

  // additional info about this connection
  pConData = new SKeyboardConnectionData();
  pConData->m_eType = SKeyboardConnectionData::Con_Window;
  pNode->m_pConnectionData = pConData;
}

void CKeyboard::ConnectGlobal(CObserver * pObserver, DWORD dwNotID)
{
  CNotifier::Connect(pObserver, dwNotID);

  SObserverNode *pNode;
  SKeyboardConnectionData *pConData;
  pNode = FindObserverNode(pObserver);

  // additional info about this connection
  pConData = new SKeyboardConnectionData();
  pConData->m_eType = SKeyboardConnectionData::Con_Global;
  pNode->m_pConnectionData = pConData;
}

BOOL CKeyboard::ShiftPressed()
{
  if(GetAsyncKeyState(VK_SHIFT) & 0x8000) return TRUE;
  else return FALSE;
}

BOOL CKeyboard::CtrlPressed()
{
  if(GetAsyncKeyState(VK_CONTROL) & 0x8000) return TRUE;
  else return FALSE;
}

BOOL CKeyboard::AltPressed()
{
  if(GetAsyncKeyState(VK_MENU) & 0x8000) return TRUE;
  else return FALSE;
}

void CKeyboard::DeleteNode(SObserverNode *pNode)
{
  SKeyboardConnectionData *pConData;

  //remove and delete additional info about the connection
  pConData = (SKeyboardConnectionData *)pNode->m_pConnectionData;
  pNode->m_pConnectionData = NULL;

  if(pConData != NULL)
    delete pConData;

  CNotifier::DeleteNode(pNode);
}

void CKeyboard::OnAbort(DWORD dwExitCode)
{
  Delete();
}

void CKeyboard::OnKeyDown(DWORD dwKey)
{
  SendKeyEvent(E_KEYDOWN, dwKey);
}

void CKeyboard::OnKeyUp(DWORD dwKey)
{
  SendKeyEvent(E_KEYUP, dwKey);
}

void CKeyboard::OnChar(DWORD dwKey)
{
  SendKeyEvent(E_CHAR, dwKey);
}

void CKeyboard::SendKeyEvent(DWORD dwEvent, DWORD dwParam)
{
  // send the event to the window which has the focus
  if(g_pDesktopWindow){
    if(g_pDesktopWindow->m_pFocusWindow){
      // send it only if it's enabled
      if(g_pDesktopWindow->m_pFocusWindow->m_bInternalyEnabled){
        // if the user processed the message do nothing
        if(SendEvent(dwEvent, dwParam, g_pDesktopWindow->m_pFocusWindow))
          return;
      }
    }
  }

  // elseway -> try to find an accelerator connection
  SKeyInfo *pInfo = (SKeyInfo *)dwParam;
  if((dwEvent == E_KEYDOWN) && (g_pKeyAccelerators != NULL)){
    if(g_pKeyAccelerators->DoAccelerator(pInfo))
      return; // if returned TRUE -> deleted the info
  }

  // other way we have to delete the info by ourself
  delete pInfo;
}
