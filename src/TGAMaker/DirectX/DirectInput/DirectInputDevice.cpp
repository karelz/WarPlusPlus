// DirectInputDevice.cpp : implementation file
//

#include "stdafx.h"
#include <dinput.h>
#include "DirectInputDevice.h"
#include "DirectInputException.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDirectInputDevice

IMPLEMENT_DYNAMIC(CDirectInputDevice, CObject)

CDirectInputDevice::CDirectInputDevice()
{
  m_lpDID = NULL;
  
  m_pguid = &GUID_SysKeyboard;
}

CDirectInputDevice::~CDirectInputDevice()
{
  Delete();
}

/////////////////////////////////////////////////////////////////////////////
// CDirectInputDevice message handlers

BOOL CDirectInputDevice::Create(LPDIRECTINPUTDEVICE lpDID, CWnd *pWnd)
{
#ifdef _DEBUG
  if(pWnd != NULL){
    ASSERT_VALID(pWnd);
  }
#endif
  ASSERT_POINTER(lpDID, 1);
  
  m_lpDID = lpDID;

  return TRUE;
}

void CDirectInputDevice::Delete()
{
  if(m_lpDID != NULL)
    m_lpDID->Release();
  m_lpDID = NULL;
}

LPDIRECTINPUTDEVICE CDirectInputDevice::GetLP()
{
  ASSERT_VALID(this);
  
  m_lpDID->AddRef();

  return m_lpDID;
}

BOOL CDirectInputDevice::PreCreate(const GUID *&rguid)
{
  rguid = m_pguid;

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CDirectInputDevice::AssertValid() const
{
  CObject::AssertValid();
  ASSERT_POINTER(m_lpDID, 1);
}

void CDirectInputDevice::Dump(CDumpContext & dc) const
{
  CObject::Dump(dc);
  dc << "LPDIRECTINPUTDEVICE : " << m_lpDID << "\n";
}

#endif

void CDirectInputDevice::Acquire()
{
  ASSERT_VALID(this);

  m_hResult = m_lpDID->Acquire();
//  DIRECTINPUT_ERROR(m_hResult);
  // it sometimes returns errors, but don't know why
}

void CDirectInputDevice::Unacquire()
{
  ASSERT_VALID(this);

  m_hResult = m_lpDID->Unacquire();
//  DIRECTINPUT_ERROR(m_hResult);
  // No detection of error - for some reason it returns error
  // event in cases it shouldn't
}

void CDirectInputDevice::SetEventNotification(HANDLE hEvent)
{
  ASSERT_VALID(this);

  m_hResult = m_lpDID->SetEventNotification(hEvent);
  DIRECTINPUT_ERROR(m_hResult);
}

void CDirectInputDevice::SetProperty(REFGUID rguid, LPCDIPROPHEADER pdiph)
{
  ASSERT_VALID(this);
  
  m_hResult = m_lpDID->SetProperty(rguid, pdiph);
  DIRECTINPUT_ERROR(m_hResult);
}

void CDirectInputDevice::GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph)
{
  ASSERT_VALID(this);

  m_hResult = m_lpDID->GetProperty(rguidProp, pdiph);
  DIRECTINPUT_ERROR(m_hResult);
}

HRESULT CDirectInputDevice::GetDeviceData(LPDIDEVICEOBJECTDATA rgdod, LPDWORD dwElements, DWORD dwFlags)
{
  ASSERT_VALID(this);
  
  m_hResult = m_lpDID->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), rgdod,
    dwElements, dwFlags);

  return m_hResult;
}

BOOL CDirectInputDevice::IsValid()
{
  if(m_lpDID == NULL) return FALSE;
  return TRUE;
}
