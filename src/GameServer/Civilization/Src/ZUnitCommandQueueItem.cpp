/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: ....
 *          
 ***********************************************************/

#include "stdafx.h"
#include "..\ZCivilization.h"

//////////////////////////////////////////////////////////////////////

DEFINE_MULTITHREAD_POOL( CZUnitCommandQueueItem, 30);

//////////////////////////////////////////////////////////////////////

CZUnitCommandQueueItem::CZUnitCommandQueueItem()
: CZUnitNotifyQueueItem()
{
	m_pWaitingInterface = NULL;
}

void CZUnitCommandQueueItem::Create(CStringTableItem *stiEventName, CIBag *pBag, 
                       CIWaitingInterface *pWaitingInterface)
{
    CZUnitNotifyQueueItem::Create( stiEventName, pBag);
    m_pWaitingInterface = pWaitingInterface;
    if (m_pWaitingInterface != NULL) m_pWaitingInterface->AddRef();
}

//////////////////////////////////////////////////////////////////////

void CZUnitCommandQueueItem::Create( CZUnitCommandQueueItem *pAnotherOne)
{
    CZUnitNotifyQueueItem::Create( pAnotherOne);
    m_pWaitingInterface =  pAnotherOne->m_pWaitingInterface;
    if (m_pWaitingInterface != NULL) m_pWaitingInterface->AddRef();
}

//////////////////////////////////////////////////////////////////////

void CZUnitCommandQueueItem::Delete()
{
	CZUnitNotifyQueueItem::Delete();
    if (m_pWaitingInterface != NULL) 
	{
		m_pWaitingInterface->Release();
		m_pWaitingInterface = NULL;
	}
}

//////////////////////////////////////////////////////////////////////

void CZUnitCommandQueueItem::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
	CZUnitNotifyQueueItem::PersistentSave( storage);
	storage << (DWORD)m_pWaitingInterface;
}

//////////////////////////////////////////////////////////////////////

void CZUnitCommandQueueItem::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
	CZUnitNotifyQueueItem::PersistentLoad( storage);
	storage >> (DWORD&)m_pWaitingInterface;
}

//////////////////////////////////////////////////////////////////////

void CZUnitCommandQueueItem::PersistentTranslatePointers( CPersistentStorage &storage)
{
    CZUnitNotifyQueueItem::PersistentTranslatePointers( storage);
	m_pWaitingInterface = (CIWaitingInterface*) storage.TranslatePointer( m_pWaitingInterface);
}

//////////////////////////////////////////////////////////////////////

void CZUnitCommandQueueItem::PersistentInit()
{
	CZUnitNotifyQueueItem::PersistentInit();
}

//////////////////////////////////////////////////////////////////////

