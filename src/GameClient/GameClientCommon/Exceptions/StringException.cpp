// StringException.cpp: implementation of the CStringException class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StringException.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStringException::CStringException(CString strMessage)
{
  m_strMessage = strMessage;
}

CStringException::CStringException(UINT nResourceID)
{
  m_strMessage.LoadString(nResourceID);
}

CStringException::CStringException(CStringException &source)
{
  m_strMessage = source.m_strMessage;
}

CStringException::~CStringException()
{
}

BOOL CStringException::GetErrorMessage(LPSTR lpszError, UINT nMaxError, PUINT pnHelpContext)
{
  strncpy(lpszError, m_strMessage, nMaxError);
  return TRUE;
}