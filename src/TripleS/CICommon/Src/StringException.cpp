/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac, Interpret, CodeManager, Civilization, GameServer
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace vyjimky CStringException.
 * 
 ***********************************************************/

#include "stdafx.h"
#include "StringException.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStringException::CStringException(LPCTSTR lpszFormat, ...)
{
  va_list args;
  va_start(args, lpszFormat);
  m_strMessage.FormatV(lpszFormat, args);
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