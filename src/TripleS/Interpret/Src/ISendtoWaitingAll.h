// ISendtoWaitingAll.h: interface for the CISendtoWaitingAll class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISENDTOWAITINGALL_H__D90A65F4_F516_11D3_B001_004F49068BD6__INCLUDED_)
#define AFX_ISENDTOWAITINGALL_H__D90A65F4_F516_11D3_B001_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IWaitingInterface.h"

class CISendtoWaitingAll : public CIWaitingInterface 
{
protected:
	CCriticalSection m_Lock;

public:
    CISendtoWaitingAll()  { m_nEventCount = 0; m_eWaitingType = EVENT_WAITING_ALL;}
    virtual ~CISendtoWaitingAll() {}

	virtual void ReadUnlock()			{	m_Lock.Unlock(); }
	virtual void WriteUnlock()			{	m_Lock.Unlock(); }
	virtual void ReadLock()				{	m_Lock.Lock(); }
	virtual void WriteLock()			{	m_Lock.Lock(); }

    void IncreaseEventCount()   {   m_nEventCount++; }  // neni treba zamykat, dela se v jednom threadu

    virtual void FinishWaiting( EInterpretStatus eStatus)
	{
		WriteLock();
		if ( --m_nEventCount == 0)  
			// kdyz vsichni dali cancel, tj. v m_nEventCount je 0, tak vrati -1 => neposle se zprava, OK
		{                                                   
			InlayEvent( WAITING_FINISHED, eStatus);
		}
		WriteUnlock();
	}

protected:
    long m_nEventCount;
};

#endif // !defined(AFX_ISENDTOWAITINGALL_H__D90A65F4_F516_11D3_B001_004F49068BD6__INCLUDED_)
