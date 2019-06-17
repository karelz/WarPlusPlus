// DirectInputException.cpp: implementation of the CDirectInputException class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <dinput.h>
#include "DirectInputException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDirectInputException::CDirectInputException(HRESULT hResult)
{
  m_hResult = hResult;

  char txt[256];

  GetErrorMessage(txt, 256, NULL);
  TRACE("DirectInput exception occured : '%s'\n", txt);
}

CDirectInputException::CDirectInputException(CDirectInputException &source)
{
  m_hResult = source.m_hResult;
}

CDirectInputException::~CDirectInputException()
{

}

BOOL CDirectInputException::GetErrorMessage(LPSTR lpszError, UINT nMaxError, PUINT pnHelpContext)
{
  CString txt, msg;

  switch(m_hResult){
  case DI_BUFFEROVERFLOW: txt = "The device buffer overflowed and some input was lost. This value is equal to the S_FALSE standard COM return value."; break;
//  case DI_NOTATTACHED: txt = "The device exists but is not currently attached. This value is equal to the S_FALSE standard COM return value."; break;
  case DI_OK: txt = "The operation completed successfully. This value is equal to the S_OK standard COM return value."; break;
  case DI_POLLEDDEVICE: txt = "The device is a polled device. As a result, device buffering will not collect any data and event notifications will not be signaled until the IDirectInputDevice::GetDeviceState method is called."; break;
//  case DI_PROPNOEFFECT: txt = "The change in device properties had no effect. This value is equal to the S_FALSE standard COM return value."; break;
  case DIERR_ACQUIRED: txt = "The operation cannot be performed while the device is acquired."; break;
  case DIERR_ALREADYINITIALIZED: txt = "This object is already initialized"; break;
  case DIERR_BADDRIVERVER: txt = "The object could not be created due to an incompatible driver version or mismatched or incomplete driver components."; break;
  case DIERR_BETADIRECTINPUTVERSION: txt = "The application was written for an unsupported prerelease version of DirectInput."; break;
  case DIERR_DEVICENOTREG: txt = "The device or device instance is not registered with DirectInput. This value is equal to the REGDB_E_CLASSNOTREG standard COM return value."; break;
  case DIERR_GENERIC: txt = "An undetermined error occurred inside the DirectInput subsystem. This value is equal to the E_FAIL standard COM return value."; break;
  case DIERR_HANDLEEXISTS: txt = "The device already has an event notification associated with it. This value is equal to the E_ACCESSDENIED standard COM return value."; break;
  case DIERR_INPUTLOST: txt = "Access to the input device has been lost. It must be re-acquired."; break;
  case DIERR_INVALIDPARAM: txt = "An invalid parameter was passed to the returning function, or the object was not in a state that permitted the function to be called. This value is equal to the E_INVALIDARG standard COM return value."; break;
  case DIERR_NOAGGREGATION: txt = "This object does not support aggregation. This value is equal to the CLASS_E_NOAGGREGATION standard COM return value."; break;
  case DIERR_NOINTERFACE: txt = "The specified interface is not supported by the object. This value is equal to the E_NOINTERFACE standard COM return value."; break;
  case DIERR_NOTACQUIRED: txt = "The operation cannot be performed unless the device is acquired."; break;
  case DIERR_NOTINITIALIZED: txt = "This object has not been initialized."; break;
  case DIERR_OBJECTNOTFOUND: txt = "The requested object does not exist."; break;
  case DIERR_OLDDIRECTINPUTVERSION: txt = "The application requires a newer version of DirectInput."; break;
//  case DIERR_OTHERAPPHASPRIO: txt = "Another application has a higher priority level, preventing this call from succeeding. This value is equal to the E_ACCESSDENIED standard COM return value."; break;
  case DIERR_OUTOFMEMORY: txt = "The DirectInput subsystem couldn't allocate sufficient memory to complete the caller's request. This value is equal to the E_OUTOFMEMORY standard COM return value."; break;
//  case DIERR_READONLY: txt = "The specified property cannot be changed. This value is equal to the E_ACCESSDENIED standard COM return value."; break;
  case DIERR_UNSUPPORTED: txt = "The function called is not supported at this time. This value is equal tot the E_NOTIMPL standard COM return value."; break;
  case E_PENDING: txt = "Data is not yet available."; break;
//  case JOYERR_NOCANDO: txt = "The joystick driver cannot update the device information from the registry."; break;
//  case JOYERR_NOERROR: txt = "The request completed successfully."; break;
//  case JOYERR_REGISTRYNOTVALID: txt = "One or more registry joystick entries contain invalid data."; break;
//  case JOYERR_UNPLUGGED: txt = "The specified joystick is not connected to the computer."; break;
//  case MMSYSERR_BADDEVICEID: txt = "The specified joystick identifier is invalid."; break;
//  case MMSYSERR_INVALPARAM: txt = "An invalid parameter was passed."; break;
//  case MMSYSERR_NODRIVER: txt = "The joystick driver is not present."; break;
  default: return FALSE;
  }
  msg.Format("DirectInput error occured:\r\n     %s",txt);

  TRACE("DirectInput exception : %s\n", txt);
  
  strncpy(lpszError, msg, nMaxError);
  return TRUE;
}
