#ifndef _ISOCLIENT_ASKFORPOSITION_SYSCALL_H_
#define _ISOCLIENT_ASKFORPOSITION_SYSCALL_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\ISyscall.h"
#include "..\SysObjs\ISOPosition.h"

class CISOClientAskForPositionSyscall : public CISyscall
{
public:
  CISOClientAskForPositionSyscall();
  virtual ~CISOClientAskForPositionSyscall();

  virtual bool Create( CIProcess *pProcess, CIDataStackItem *pRet, CISOPosition *pPosition );
  virtual void Delete();
  virtual void Cancel( CIProcess *pProcess );

  void FinishWaiting( bool bReturn, DWORD dwXPosition, DWORD dwYPosition );

private:
  // Lock the data in the object
  CMutex m_lockLock;

  // Here return the result
  CIDataStackItem *m_pRet;
  // Here return the position if some
  CISOPosition *m_pPosition;
};

#endif