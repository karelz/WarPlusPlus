// ZUnitNotifyQueueItem.h: interface for the CZUnitNotifyQueueItem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZUNITNOTIFYQUEUEITEM_H__9A7DDAA3_F27D_11D3_AFFC_004F49068BD6__INCLUDED_)
#define AFX_ZUNITNOTIFYQUEUEITEM_H__9A7DDAA3_F27D_11D3_AFFC_004F49068BD6__INCLUDED_

#include "TripleS\Interpret\Src\IBag.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CZUnitNotifyQueueItem  
{
    DECLARE_MULTITHREAD_POOL( CZUnitNotifyQueueItem);

    LONG m_nRefCount;

public:
	// prazdny konstruktor pouzit jen tehdy, jedna-li se o load ze souboru
	CZUnitNotifyQueueItem();
    virtual ~CZUnitNotifyQueueItem() { ASSERT( m_stiEventName == NULL); }

    void Create( CStringTableItem *stiEventName, CIBag *pBag);
    void Create( CZUnitNotifyQueueItem *pAnotherOne);
    virtual void Delete();

    inline CZUnitNotifyQueueItem* AddRef() 
    { 
        ::InterlockedIncrement( &m_nRefCount);
        return this;
    }

    inline CZUnitNotifyQueueItem* Release()
    {
        if ( ::InterlockedDecrement( &m_nRefCount) == 0)
        {
            Delete();
            delete this;
        }
        return NULL;
    }

public:
    CStringTableItem *m_stiEventName;
    CIBag m_Bag;
    CZUnitNotifyQueueItem *m_pNext;

public:
//
//	SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();
};

#endif // !defined(AFX_ZUNITNOTIFYQUEUEITEM_H__9A7DDAA3_F27D_11D3_AFFC_004F49068BD6__INCLUDED_)
