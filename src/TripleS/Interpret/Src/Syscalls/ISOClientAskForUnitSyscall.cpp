#include "stdafx.h"
#include "..\..\Interpret.h"

#include "ISOClientAskForUnitSyscall.h"
#include "TripleS\CodeManager\CodeManager.h"

CISOClientAskForUnitSyscall::CISOClientAskForUnitSyscall()
{
  m_pRet = NULL;
}

CISOClientAskForUnitSyscall::~CISOClientAskForUnitSyscall()
{
  ASSERT(m_pRet == NULL);
}

bool CISOClientAskForUnitSyscall::Create ( CIProcess *pProcess, CIDataStackItem *pRet )
{
  // Just copy params
  m_pRet = pRet;

  return CISyscall::Create ( pProcess );
}

void CISOClientAskForUnitSyscall::Delete ( )
{
  VERIFY(m_lockLock.Lock());

  if(m_pRet != NULL){
    m_pRet = NULL;
  }

  VERIFY(m_lockLock.Unlock());

  CISyscall::Delete ( );
}

void CISOClientAskForUnitSyscall::Cancel ( CIProcess *pProcess )
{
  VERIFY(m_lockLock.Lock());

  if(m_pRet != NULL){
    m_pRet = NULL;
  }

  VERIFY(m_lockLock.Unlock());

  CISyscall::Cancel ( pProcess );
}

void CISOClientAskForUnitSyscall::FinishWaiting ( CZUnit *pUnit )
{
  VERIFY(m_lockLock.Lock());

  if(m_pRet != NULL){
    m_pRet->Set( pUnit, CCodeManager::m_pDefaultUnitType );
    m_pRet = NULL;
  }

  VERIFY(m_lockLock.Unlock());

  CISyscall::FinishWaiting(STATUS_SUCCESS);
}
