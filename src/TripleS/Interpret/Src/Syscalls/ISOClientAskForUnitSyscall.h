#ifndef _ISOCLIENT_ASKFORUNIT_SYSCALL_H_
#define _ISOCLIENT_ASKFORUNIT_SYSCALL_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\ISyscall.h"

class CISOClientAskForUnitSyscall : public CISyscall
{
public:
  CISOClientAskForUnitSyscall();
  virtual ~CISOClientAskForUnitSyscall();

  virtual bool Create( CIProcess *pProcess, CIDataStackItem *pRet );
  virtual void Delete();
  virtual void Cancel( CIProcess *pProcess );

  void FinishWaiting( CZUnit *pUnit );

  typedef enum {
    UnitType_Any = 0,
    UnitType_Enemy = 1,
    UnitType_My = 2,
  } EUnitType;

private:
  // Lock the data in the object
  CCriticalSection m_lockLock;

  // Here return the result
  CIDataStackItem *m_pRet;
};

#endif