#include "stdafx.h"
#include "..\..\Interpret.h"

#include "ISOClientAskForPositionSyscall.h"

CISOClientAskForPositionSyscall::CISOClientAskForPositionSyscall()
{
  m_pRet = NULL;
  m_pPosition = NULL;
}

CISOClientAskForPositionSyscall::~CISOClientAskForPositionSyscall()
{
  ASSERT(m_pRet == NULL);
  ASSERT(m_pPosition == NULL);
}

bool CISOClientAskForPositionSyscall::Create ( CIProcess *pProcess, CIDataStackItem *pRet, CISOPosition *pPosition )
{
  ASSERT(pPosition != NULL);

  // Just copy params
  m_pRet = pRet;
  m_pPosition = pPosition;
  m_pPosition->AddRef();

  return CISyscall::Create( pProcess );
}

void CISOClientAskForPositionSyscall::Delete ( )
{
  VERIFY(m_lockLock.Lock());

  if(m_pRet != NULL){
    m_pRet = NULL;
    m_pPosition->Release();
    m_pPosition = NULL;
  }

  VERIFY(m_lockLock.Unlock());

  CISyscall::Delete();
}

void CISOClientAskForPositionSyscall::Cancel ( CIProcess *pProcess )
{
  VERIFY(m_lockLock.Lock());

  if(m_pRet != NULL){
    m_pRet = NULL;
    m_pPosition->Release();
    m_pPosition = NULL;
  }

  VERIFY(m_lockLock.Unlock());

  CISyscall::Cancel( pProcess );
}

void CISOClientAskForPositionSyscall::FinishWaiting ( bool bReturn, DWORD dwXPosition, DWORD dwYPosition )
{
  VERIFY(m_lockLock.Lock());

  if(m_pRet != NULL){
    m_pRet->Set( bReturn );
    if(bReturn){
      CPointDW pt(dwXPosition, dwYPosition);
      m_pPosition->Set( &pt, 0 );
    }

    m_pRet = NULL;
    m_pPosition->Release();
    m_pPosition = NULL;
  }

  VERIFY(m_lockLock.Unlock());

  CISyscall::FinishWaiting(STATUS_SUCCESS);
}
