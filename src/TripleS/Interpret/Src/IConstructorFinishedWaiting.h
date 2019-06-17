// IConstructorFinishedWaiting.h: interface for the CIConstructorFinishedWaiting class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ICONSTRUCTORFINISHEDWAITING_H__04FDFD86_EC32_11D3_AFF2_004F49068BD6__INCLUDED_)
#define AFX_ICONSTRUCTORFINISHEDWAITING_H__04FDFD86_EC32_11D3_AFF2_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IWaitingInterface.h"

class CIConstructorFinishedWaiting : public CIWaitingInterface  
{
protected:
	CCriticalSection m_Lock;

public:
    CIConstructorFinishedWaiting() { m_eWaitingType = CONSTRUCTOR_WAITING;}
    virtual ~CIConstructorFinishedWaiting()  {}

	virtual void ReadUnlock()			{	m_Lock.Unlock(); }
	virtual void WriteUnlock()			{	m_Lock.Unlock(); }
	virtual void ReadLock()				{	m_Lock.Lock(); }
	virtual void WriteLock()			{	m_Lock.Lock(); }

    virtual void FinishWaiting( EInterpretStatus eStatus)
    {
        ASSERT( eStatus == STATUS_SUCCESS || eStatus== STATUS_UNIT_IS_DEAD ||
			    eStatus == STATUS_INTERPRET_SHUTDOWN);

        InlayEvent( WAITING_FINISHED, eStatus);
    }
};

#endif // !defined(AFX_ICONSTRUCTORFINISHEDWAITING_H__04FDFD86_EC32_11D3_AFF2_004F49068BD6__INCLUDED_)
