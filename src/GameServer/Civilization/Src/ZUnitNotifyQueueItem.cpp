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

DEFINE_MULTITHREAD_POOL( CZUnitNotifyQueueItem, 30);

//////////////////////////////////////////////////////////////////////

CZUnitNotifyQueueItem::CZUnitNotifyQueueItem()
{
	m_stiEventName = NULL;
	m_pNext = NULL;
    m_nRefCount = 0;
}

void CZUnitNotifyQueueItem::Create( CStringTableItem *stiEventName, CIBag *pBag)
{
    m_stiEventName = stiEventName->AddRef();

    ASSERT( pBag->GetReturnRegister() == NULL);
    m_Bag.Create(*pBag);
    
    m_pNext = NULL;

    AddRef();
}

void CZUnitNotifyQueueItem::Create( CZUnitNotifyQueueItem *pAnotherOne)
{
    m_stiEventName = pAnotherOne->m_stiEventName->AddRef();

    m_Bag.Create( pAnotherOne->m_Bag);

    m_pNext = NULL;

    AddRef();
}

void CZUnitNotifyQueueItem::Delete()
{
    if (m_stiEventName != NULL) 
	{
		m_stiEventName->Release();
		m_stiEventName = NULL;
	}
    m_Bag.Delete();
	m_pNext = NULL;
}

//
//	SAVE & LOAD
//

void CZUnitNotifyQueueItem::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    storage << m_nRefCount;
    storage << (DWORD)m_stiEventName;
    m_Bag.PersistentSave( storage);
}

void CZUnitNotifyQueueItem::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    storage >> m_nRefCount;
	storage >> (DWORD&)m_stiEventName;
	m_Bag.PersistentLoad( storage);
	m_pNext = NULL;
}

void CZUnitNotifyQueueItem::PersistentTranslatePointers( CPersistentStorage &storage)
{
	m_stiEventName = (CStringTableItem *) storage.TranslatePointer( m_stiEventName);
    m_Bag.PersistentTranslatePointers( storage);
}

void CZUnitNotifyQueueItem::PersistentInit()
{
	// empty
}
