// ISendtoWaiting1st.h: interface for the CISendtoWaiting1st class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISENDTOWAITING1ST_H__D90A65F5_F516_11D3_B001_004F49068BD6__INCLUDED_)
#define AFX_ISENDTOWAITING1ST_H__D90A65F5_F516_11D3_B001_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IWaitingInterface.h"

class CISendtoWaiting1st : public CIWaitingInterface  
{
protected:
	CCriticalSection m_Lock;

public:
    CISendtoWaiting1st() { m_bWaitingFinished = false; m_eWaitingType = EVENT_WAITING_1ST;}
    virtual ~CISendtoWaiting1st() {}

	virtual void ReadUnlock()			{	m_Lock.Unlock(); }
	virtual void WriteUnlock()			{	m_Lock.Unlock(); }
	virtual void ReadLock()				{	m_Lock.Lock(); }
	virtual void WriteLock()			{	m_Lock.Lock(); }

    virtual void FinishWaiting( EInterpretStatus eStatus)
	{
		WriteLock();
		if (!m_bWaitingFinished)
		{
			InlayEvent( WAITING_FINISHED, eStatus);
			m_bWaitingFinished = true;
		}
		WriteUnlock();
	}

protected:
    bool m_bWaitingFinished;
};

#endif // !defined(AFX_ISENDTOWAITING1ST_H__D90A65F5_F516_11D3_B001_004F49068BD6__INCLUDED_)
