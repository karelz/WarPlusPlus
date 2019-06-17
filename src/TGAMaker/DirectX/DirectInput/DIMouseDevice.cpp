// DIMouseDevice.cpp: implementation of the CDIMouseDevice class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <dinput.h>
#include "DIMouseDevice.h"

#include "DirectInputException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CDIMouseDevice, CDirectInputDevice);

CDIMouseDevice::CDIMouseDevice()
{

}

CDIMouseDevice::~CDIMouseDevice()
{

}

BOOL CDIMouseDevice::PreCreate(const GUID *&rguid)
{
  rguid = &GUID_SysMouse;

  return TRUE;
}

BOOL CDIMouseDevice::Create(LPDIRECTINPUTDEVICE lpDID, CWnd *pWnd)
{
  if(!CDirectInputDevice::Create(lpDID, pWnd)) return FALSE;

  m_pWnd = pWnd;

  m_hResult = m_lpDID->SetDataFormat(&c_dfDIMouse);
  DIRECTINPUT_ERROR(m_hResult);

  m_hResult = m_lpDID->SetCooperativeLevel(pWnd->GetSafeHwnd(),
    DISCL_FOREGROUND | DISCL_EXCLUSIVE);
  DIRECTINPUT_ERROR(m_hResult);

  return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CDIMouseDevice::AssertValid() const
{
  CDirectInputDevice::AssertValid();
}

void CDIMouseDevice::Dump(CDumpContext & dc) const
{
  CDirectInputDevice::Dump(dc);
}

#endif