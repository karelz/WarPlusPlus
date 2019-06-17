// ISendtoInformation.h: interface for the CISendtoInformation class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISENDTOINFORMATION_H__D90A65F3_F516_11D3_B001_004F49068BD6__INCLUDED_)
#define AFX_ISENDTOINFORMATION_H__D90A65F3_F516_11D3_B001_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameServer\Civilization\Src\ZUnit.h"

class CISendtoInformation  
{
    DECLARE_MULTITHREAD_POOL(CISendtoInformation);

public:
	CISendtoInformation() {}; // pro load&save
	CISendtoInformation( CZUnit *pSender);
	virtual ~CISendtoInformation();

    typedef enum { NONE, WAIT_ALL, WAIT_1ST } ESendtoWaitingType;

// METODY
    void SetWaiting( CIWaitingInterface *pWaiting, ESendtoWaitingType eWaitingType);
    void SetNotification() { m_eEventType = EVENT_NOTIFICATION; }
    void AddUnit( CZUnit *pUnit) { m_pTargetUnitList = new CZUnitListItem( pUnit, m_pTargetUnitList); }
    void AddEvent( CStringTableItem *pName, CIBag *pBag);
    EInterpretStatus Send( CInterpret *pInterpret, CIProcess *pCurrentProcess);

// DATA
protected:
    CZUnit *m_pSender;
    CIWaitingInterface *m_pWaiting;
    ESendtoWaitingType m_eSendtoWaitingType;

    // spojak jednotek
    CZUnitListItem *m_pTargetUnitList;

    // typ zprav
    EEventType m_eEventType;
    union
    {
        struct
        {
            CZUnitCommandQueueItem *m_pCommandListFirst;
            CZUnitCommandQueueItem *m_pCommandListLast;
        };
        struct
        {
            CZUnitNotifyQueueItem *m_pNotificationListFirst;
            CZUnitNotifyQueueItem *m_pNotificationListLast;
        };
    };

public:
	//
// SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();
};

#endif // !defined(AFX_ISENDTOINFORMATION_H__D90A65F3_F516_11D3_B001_004F49068BD6__INCLUDED_)
