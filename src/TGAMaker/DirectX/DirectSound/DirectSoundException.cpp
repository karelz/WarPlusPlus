// DirectSoundException.cpp: implementation of the CDirectSoundException class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <mmreg.h>
#include <dsound.h>
#include "DirectSoundException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDirectSoundException::CDirectSoundException(HRESULT hResult)
{
  m_hResult = hResult;

#ifdef _DEBUG  
  char txt[256];

  GetErrorMessage(txt, 255, 0);
  TRACE("DirectSound exception : '%s'.\n", txt);
#endif
}

CDirectSoundException::CDirectSoundException(CDirectSoundException &source)
{
  m_hResult = source.m_hResult;
}

CDirectSoundException::~CDirectSoundException()
{
}

BOOL CDirectSoundException::GetErrorMessage(LPSTR lpszError, UINT nMaxError, PUINT pnHelpContext)
{
  CString txt, msg;

  switch(m_hResult){
  case DS_OK: txt = "No error."; break;
  case DSERR_ALLOCATED: txt = "The request failed because resources, such as a priority level, were already in use by another caller."; break;
  case DSERR_ALREADYINITIALIZED: txt = "The object is already initialized."; break;
  case DSERR_BADFORMAT: txt = "The specified wave format is not supported."; break;
  case DSERR_BUFFERLOST: txt = "The buffer memory has been lost and must be restored."; break;
  case DSERR_CONTROLUNAVAIL: txt = "The control (volume, pan, and so forth) requested by the caller is not available."; break;
  case DSERR_GENERIC: txt = "An undetermined error occurred inside the DirectSound subsystem."; break;
  case DSERR_INVALIDCALL: txt = "This function is not valid for the current state of this object."; break;
  case DSERR_INVALIDPARAM: txt = "An invalid parameter was passed to the returning function."; break;
  case DSERR_NOAGGREGATION: txt = "The object does not support aggregation."; break;
  case DSERR_NODRIVER: txt = "No sound driver is available for use."; break;
  case DSERR_OTHERAPPHASPRIO: txt = "This value is obsolete and is not used."; break;
  case DSERR_OUTOFMEMORY: txt = "The DirectSound subsystem could not allocate sufficient memory to complete the caller's request."; break;
  case DSERR_PRIOLEVELNEEDED: txt = "The caller does not have the priority level required for the function to succeed.";  break;
  case DSERR_UNINITIALIZED: txt = "The IDirectSound::Initialize method has not been called or has not been called successfully before other methods were called."; break;
  case DSERR_UNSUPPORTED: txt = "The function called is not supported at this time."; break;
  default: return FALSE;
  }
  msg.Format("DirectSound error ocured: \r\n     %s", txt);
  strncpy(lpszError, msg, nMaxError);

  return TRUE;
}
