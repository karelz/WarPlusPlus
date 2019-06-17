// DirectInput.cpp : implementation file
//

#include "stdafx.h"
#include <objbase.h>
#include <initguid.h>  // small hack - next #include will define guids
#include <dinput.h>    // not only as declarations but also as definitions
#include "DirectInput.h"
#include "..\..\Events\Events.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDirectInput

CDirectInput *g_pDirectInput = NULL;

IMPLEMENT_DYNAMIC(CDirectInput, CObserver)

BEGIN_OBSERVER_MAP(CDirectInput, CObserver)
  BEGIN_ABORT()
    ON_ABORT()
  END_ABORT()
END_OBSERVER_MAP(CDirectInput, CObserver)

CDirectInput::CDirectInput()
{
  m_lpDI = NULL;
}

CDirectInput::~CDirectInput()
{
  Delete();
}

/////////////////////////////////////////////////////////////////////////////
// CDirectInput message handlers

BOOL CDirectInput::Create(HINSTANCE hInstance, CWnd *pWnd)
{
#ifdef _DEBUG
  if(pWnd != NULL){
    ASSERT_VALID(pWnd);
    ASSERT_KINDOF(CWnd, pWnd);
  }
#endif
  
  m_pWnd = pWnd;
  
  m_hResult = DirectInputCreate(hInstance, DIRECTINPUT_VERSION, &m_lpDI, NULL);
  DIRECTINPUT_ERROR(m_hResult);

  // we can assume only one instance of CDirectInput
  // so we can set a global pointer to it
  ASSERT(g_pDirectInput == NULL); // if it's not null -> error
  g_pDirectInput = this;

  return TRUE;
}

void CDirectInput::Delete()
{
  // as in Create we can assume only one instance of this
  // so null the pointer to it
  g_pDirectInput = NULL;
  
  if(m_lpDI != NULL)
    m_lpDI->Release();
  m_lpDI = NULL;
}

LPDIRECTINPUT CDirectInput::GetLP()
{
  ASSERT_VALID(this);

  m_lpDI->AddRef();

  return m_lpDI;
}

BOOL CDirectInput::CreateDevice(CDirectInputDevice & DIDevice)
{
  const GUID *guid;
  LPDIRECTINPUTDEVICE lpDID;

  ASSERT_VALID(this);

  if(!DIDevice.PreCreate(guid)) return FALSE;

  m_hResult = m_lpDI->CreateDevice(*guid, &lpDID, NULL);
  DIRECTINPUT_ERROR(m_hResult);

  if(!DIDevice.Create(lpDID, m_pWnd)) return FALSE;

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CDirectInput::AssertValid() const
{
  CObserver::AssertValid();
  ASSERT_POINTER(m_lpDI, 1);
}

void CDirectInput::Dump(CDumpContext & dc) const
{
  CObserver::Dump(dc);
  dc << "LPDIRECTINPUT : " << m_lpDI << "\n";
}

#endif

void CDirectInput::OnAbort(DWORD dwExitCode)
{
  Delete();
}
