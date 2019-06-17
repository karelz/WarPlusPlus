// MultithreadNotifier.cpp: implementation of the CMultithreadNotifier class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MultithreadNotifier.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CMultithreadNotifier, CNotifier);

CMultithreadNotifier::CMultithreadNotifier()
{

}

CMultithreadNotifier::~CMultithreadNotifier()
{

}

#ifdef _DEBUG

void CMultithreadNotifier::AssertValid() const
{
  CNotifier::AssertValid();
}

void CMultithreadNotifier::Dump(CDumpContext &dc) const
{
  CNotifier::Dump(dc);
}

#endif


void CMultithreadNotifier::Lock()
{
  m_semaphoreMonitor.Lock();
}

void CMultithreadNotifier::Unlock()
{
  m_semaphoreMonitor.Unlock();
}
