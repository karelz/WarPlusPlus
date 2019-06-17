// MultithreadNotifier.h: interface for the CMultithreadNotifier class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MULTITHREADNOTIFIER_H__F3565164_1CF1_11D3_9FBA_D572359E1138__INCLUDED_)
#define AFX_MULTITHREADNOTIFIER_H__F3565164_1CF1_11D3_9FBA_D572359E1138__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Common\ReadWriteLock\ReadWriteLock.h"
#include "Notifier.h"

class CMultithreadNotifier : public CNotifier
{
public:
	CMultithreadNotifier();
	virtual ~CMultithreadNotifier();

  DECLARE_DYNAMIC(CMultithreadNotifier);

#ifdef _DEBUG
public:
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

public:
  // some create

protected:
	// unlocks the semaphore (internal)
  virtual void ReadUnlock();
  virtual void WriteUnlock();
	// locks the semaphore (internal)
  virtual void ReadLock();
  virtual void WriteLock();

  // the lock
  CReadWriteLock m_Lock;

  friend class CNotifier;
};

#endif // !defined(AFX_MULTITHREADNOTIFIER_H__F3565164_1CF1_11D3_9FBA_D572359E1138__INCLUDED_)
