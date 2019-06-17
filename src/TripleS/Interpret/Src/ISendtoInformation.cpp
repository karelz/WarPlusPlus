// IInformation.cpp: implementation of the CISendtoInformation class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\Interpret.h"

#include "ISendtoInformation.h"
#include "ISendtoWaitingAll.h"
#include "ISendtoWaiting1st.h"

#include "GameServer\Civilization\Src\ZUnit.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DEFINE_MULTITHREAD_POOL(CISendtoInformation, 4);

CISendtoInformation::CISendtoInformation( CZUnit *pSender)
{
    m_pTargetUnitList = NULL;
    m_pNotificationListFirst = m_pNotificationListLast = NULL;
    m_pCommandListFirst = m_pCommandListLast = NULL;
    
    m_pWaiting = NULL;
    m_eSendtoWaitingType = NONE;

    m_eEventType = EVENT_COMMAND;

    m_pSender = pSender;
}

CISendtoInformation::~CISendtoInformation()
{
    if (!g_bGameLoading && m_pWaiting != NULL) m_pWaiting = m_pWaiting->Release();
    
    CZUnitListItem *pPom;

    while (m_pTargetUnitList != NULL)
    {
        pPom = m_pTargetUnitList;
        m_pTargetUnitList = m_pTargetUnitList->m_pNext;

        delete pPom;
    }

    if ( m_eEventType == EVENT_COMMAND)
    {
        CZUnitCommandQueueItem *pP;
        while (m_pCommandListFirst != NULL)
        {
            pP = m_pCommandListFirst;
            m_pCommandListFirst = (CZUnitCommandQueueItem*)m_pCommandListFirst->m_pNext;

            if (g_bGameLoading) delete pP;
            else pP->Release();
        }
    }
    else
    {
        ASSERT( m_eEventType == EVENT_NOTIFICATION);

        CZUnitNotifyQueueItem *pP;
        while (m_pNotificationListFirst != NULL)
        {
            pP = m_pNotificationListFirst;
            m_pNotificationListFirst = m_pNotificationListFirst->m_pNext;

            if (g_bGameLoading) delete pP;
            else pP->Release();
        }
    }
}

void CISendtoInformation::SetWaiting( CIWaitingInterface *pWaiting, ESendtoWaitingType eWaitingType)
{    
    m_pWaiting = pWaiting->AddRef();
    m_eSendtoWaitingType = eWaitingType;
}

void CISendtoInformation::AddEvent( CStringTableItem *pName, CIBag *pBag)
{
    if ( m_eEventType == EVENT_COMMAND)
    {
        CZUnitCommandQueueItem *pCommand = new CZUnitCommandQueueItem;
        pCommand->Create( pName, pBag, m_pWaiting);

        if (m_pCommandListLast == NULL) m_pCommandListFirst = m_pCommandListLast = pCommand;
        else m_pCommandListLast->m_pNext = pCommand;
    }
    else 
    {
        ASSERT( m_eEventType == EVENT_NOTIFICATION);

        CZUnitNotifyQueueItem *pNot = new CZUnitNotifyQueueItem;
        pNot->Create( pName, pBag);

        if (m_pNotificationListLast == NULL) m_pNotificationListFirst = m_pNotificationListLast = pNot;
        else m_pNotificationListLast->m_pNext = pNot;
    }
}

EInterpretStatus CISendtoInformation::Send( CInterpret *pInterpret, CIProcess *pCurrentProcess)
{
	ASSERT( pInterpret != NULL);
	ASSERT( pCurrentProcess != NULL);

    CZUnitListItem *pPom;
	EInterpretStatus eStatus = STATUS_SUCCESS;

    if (m_pTargetUnitList == NULL) return eStatus;

    if ( m_eEventType == EVENT_COMMAND)
    {
		// kontrola posilani prikazu: je zakazano posilat si prikaz z horni fronty do dolni, 
		// protoze to okamzite zabiji a pak cykli.
		for( pPom = m_pTargetUnitList; pPom != NULL; pPom = pPom->m_pNext)
		{
			if ( m_pSender != pPom->m_pUnit) continue;
			if ( !m_pSender->CanSendCommandToMyselfFromProcess( pCurrentProcess))
			{
				return STATUS_FORBIDDEN_SEND_COMMAND;
			}
		}

		// Ok, rozesleme zpravy
        CZUnitCommandQueueItem *pCurCmd;

        // pro vsechny jednotky
        while (m_pTargetUnitList != NULL)
        {
            // pro vsechny prikazy 
            pCurCmd = m_pCommandListFirst;
            while ( pCurCmd != NULL)
            {
                // zvyseni counteru u cekani
                if (m_pWaiting != NULL && m_eSendtoWaitingType == WAIT_ALL)
                {
                    ((CISendtoWaitingAll*)m_pWaiting)->IncreaseEventCount();
                }

                // odeslani (spusti se az se zase rozbehne interpret..)
                eStatus = m_pTargetUnitList->m_pUnit->AppendEvent( m_pSender, pInterpret, EVENT_UNKNOWN, 
                            pCurCmd->m_stiEventName, &pCurCmd->m_Bag, pCurCmd->m_pWaitingInterface, true);
                if ( eStatus != STATUS_SUCCESS) return eStatus;

                // dalsi eventa
                pCurCmd = (CZUnitCommandQueueItem*)pCurCmd->m_pNext;
            }

            // smazat jednotku a hura na dalsi
            pPom = m_pTargetUnitList;
            m_pTargetUnitList = m_pTargetUnitList->m_pNext;

            delete pPom;
        }
    }
    else
    {
        ASSERT( m_eEventType == EVENT_NOTIFICATION);

        CZUnitNotifyQueueItem *pCurNot;

        // pro vsechny jednotky
        while (m_pTargetUnitList != NULL)
        {
            // pro vsechny prikazy 
            pCurNot = m_pNotificationListFirst;
            while ( pCurNot != NULL)
            {
                // odeslani
                m_pTargetUnitList->m_pUnit->AppendEvent( m_pSender, pInterpret, EVENT_NOTIFICATION,
                    pCurNot->m_stiEventName, &pCurNot->m_Bag, NULL, true);

                // dalsi eventa
                pCurNot = pCurNot->m_pNext;
            }

            // smazat jednotku a hura na dalsi
            pPom = m_pTargetUnitList;
            m_pTargetUnitList = m_pTargetUnitList->m_pNext;

            delete pPom;
        }
    }

	return eStatus;
}

void CISendtoInformation::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    // pointer na sebe
	storage << (DWORD)this;

	// odesilatel
	storage << (DWORD)m_pSender;

	// waiting interface
    storage << (DWORD)m_eSendtoWaitingType;
    PersistentSaveWaiting( m_pWaiting, storage);

    // spojak jednotek
	int nCount;
	CZUnitListItem *pUnitListItem;

	for ( nCount=0, pUnitListItem = m_pTargetUnitList;  pUnitListItem != NULL;  pUnitListItem = pUnitListItem->m_pNext)
	{
		nCount++;
	}

	storage << nCount;

	for (pUnitListItem = m_pTargetUnitList;  pUnitListItem != NULL;  pUnitListItem = pUnitListItem->m_pNext)
	{
		storage << (DWORD)pUnitListItem->m_pUnit;
	}

    // typ zprav
    storage << (DWORD)m_eEventType;
    
	// zpravy
	CZUnitNotifyQueueItem *pQI;
	if (m_eEventType == EVENT_COMMAND)
	{
		for ( nCount=0, pQI = m_pCommandListFirst;  pQI != NULL;  pQI = pQI->m_pNext) nCount++;
		storage << nCount;
		for ( pQI = m_pCommandListFirst;  pQI != NULL;  pQI = pQI->m_pNext) pQI->PersistentSave(storage);
	}
	else
	{
		for ( nCount=0, pQI = m_pNotificationListFirst;  pQI != NULL;  pQI = pQI->m_pNext) nCount++;
		storage << nCount;
		for ( pQI = m_pNotificationListFirst;  pQI != NULL;  pQI = pQI->m_pNext) pQI->PersistentSave(storage);
	}
}

void CISendtoInformation::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    // pointer na sebe
	CISendtoInformation *pThis;
	storage >> (DWORD&)pThis;
	storage.RegisterPointer( pThis, this);

	// odesilatel
	storage >> (DWORD&)m_pSender;

	// waiting interface
    storage >> (DWORD&)m_eSendtoWaitingType;
    m_pWaiting = PersistentLoadWaiting( storage);

    // spojak jednotek
	int nCount, i;
	CZUnitListItem *pNewULI, *pEndULI;

	storage >> nCount;

	if (nCount == 0)
	{
		m_pTargetUnitList = NULL;
	}
	else
	{
		for (i=0; i<nCount; i++)
		{
			pNewULI = new CZUnitListItem();
			storage >> (DWORD&)pNewULI->m_pUnit;
			pNewULI->m_pNext = NULL;

			if (i==0) pEndULI = m_pTargetUnitList = pNewULI;
			else
			{
				pEndULI->m_pNext = pNewULI;
				pEndULI = pNewULI;
			}
		}
	}

    // typ zprav
    storage << (DWORD)m_eEventType;
    
	// zpravy
	CZUnitNotifyQueueItem *pQI;

	if (m_eEventType == EVENT_COMMAND)
	{
		storage >> nCount;
		if (nCount == NULL) m_pCommandListFirst = m_pCommandListLast = NULL;
		else
		{
			for ( i=0; i<nCount; i++)
			{
				pQI = new CZUnitCommandQueueItem;
				pQI->PersistentLoad( storage);

				if (i==0) m_pCommandListFirst = m_pCommandListLast = (CZUnitCommandQueueItem*)pQI;
				else
				{
					m_pCommandListLast->m_pNext = pQI;
					m_pCommandListLast = (CZUnitCommandQueueItem*)pQI;
				}
			}
		}
	}
	else
	{
		storage >> nCount;
		if (nCount == NULL) m_pNotificationListFirst = m_pNotificationListLast = NULL;
		else
		{
			for ( i=0; i<nCount; i++)
			{
				pQI = new CZUnitNotifyQueueItem;
				pQI->PersistentLoad( storage);

				if (i==0) m_pNotificationListFirst = m_pNotificationListLast = pQI;
				else
				{
					m_pNotificationListLast->m_pNext = pQI;
					m_pNotificationListLast = pQI;
				}
			}
		}
	}
}

void CISendtoInformation::PersistentTranslatePointers( CPersistentStorage &storage)
{
	// odesilatel
	m_pSender = (CZUnit*) storage.TranslatePointer( m_pSender);

	// waiting interface
    PersistentTranslatePointersWaiting( m_pWaiting, storage);

    // spojak jednotek
	CZUnitListItem *pULI;

	for ( pULI=m_pTargetUnitList;  pULI != NULL; pULI = pULI->m_pNext)
	{
		pULI->m_pUnit = (CZUnit*) storage.TranslatePointer( pULI->m_pUnit);
	}

	// zpravy
	CZUnitNotifyQueueItem *pQI;

	if (m_eEventType == EVENT_COMMAND)
	{
		for (pQI = m_pCommandListFirst;  pQI != NULL;  pQI = pQI->m_pNext) pQI->PersistentTranslatePointers( storage);
	}
	else
	{
		for (pQI = m_pNotificationListFirst;  pQI != NULL;  pQI = pQI->m_pNext) pQI->PersistentTranslatePointers( storage);
	}
}

void CISendtoInformation::PersistentInit()
{
	PersistentInitWaiting( m_pWaiting);
}

