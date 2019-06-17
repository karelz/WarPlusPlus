// ZUnitCommandQueueItem.h: interface for the CZUnitCommandQueueItem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZUNITCOMMANDQUEUEITEM_H__9A7DDAA4_F27D_11D3_AFFC_004F49068BD6__INCLUDED_)
#define AFX_ZUNITCOMMANDQUEUEITEM_H__9A7DDAA4_F27D_11D3_AFFC_004F49068BD6__INCLUDED_

#include "TripleS\Interpret\Src\IBag.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CZUnitCommandQueueItem  :  public CZUnitNotifyQueueItem
{
    DECLARE_MULTITHREAD_POOL(CZUnitCommandQueueItem);
public:
	// prazdny konstruktor pouzit jen tehdy, jedna-li se o load ze souboru
	CZUnitCommandQueueItem();
    virtual ~CZUnitCommandQueueItem() {}

    void Create( CStringTableItem *stiEventName, CIBag *pBag, CIWaitingInterface *pWaitingInterface);
    void Create( CZUnitCommandQueueItem *pAnotherOne);
    virtual void Delete();

public:
    CIWaitingInterface *m_pWaitingInterface;

public:
//
//	SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();

};

#endif // !defined(AFX_ZUNITCOMMANDQUEUEITEM_H__9A7DDAA4_F27D_11D3_AFFC_004F49068BD6__INCLUDED_)
