// ISOSet.h: interface for the CISOSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISOSET_H__380C8AF5_DFDE_11D3_AFE0_004F49068BD6__INCLUDED_)
#define AFX_ISOSET_H__380C8AF5_DFDE_11D3_AFE0_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ISOSetParent.h"
#include "Common\AfxDebugPlus\AfxDebugPlus.h"

template<class Typ> class CISOSet : public CISOSetParent
{
    friend CInterpret;

protected:
    struct tagSetItem
    {
        Typ Item;
        struct tagSetItem *pNext;
    };
    typedef struct tagSetItem SSetItem;

public:
    // konstrukce
    CISOSet() : CISOSetParent() 
    {
        _SCI_m_aMethods[0] = _SCI_M_M_GetItemCount;
        _SCI_m_aMethods[1] = _SCI_M_M_IsEmpty;
        _SCI_m_aMethods[2] = _SCI_M_M_SetEmpty;
    }

    // destrukce
    virtual ~CISOSet() {}

    // alokace a dealokace prvku
    static SSetItem* AllocSSetItem()  { return new SSetItem; }
    static void FreeSSetItem( SSetItem* pSetItem)  {  delete pSetItem; }

    // vytvoreni mnoziny
    virtual bool CreateEmpty( CInterpret *pInterpret, CType *pType)
    {
		if (!CISystemObject::Create( pInterpret)) return false;

        m_DataType = *pType;
        m_pItems = NULL;
        m_nItemCount = 0;
        m_bObjUnitStruct = pType->IsObjectUnitStruct();
        m_pForItems = NULL;

        return true;
    }
    
    // vytvoreni mnoziny ze sjednoceni
    virtual bool CreateFromUnion( CInterpret *pInterpret, CISOSetParent *pSet1, CISOSetParent *pSet2)
    {
		if (!CISystemObject::Create( pInterpret)) return false;

        ASSERT( pSet1->m_DataType == pSet2->m_DataType);

        SSetItem *pNewSI, *pSI;

        if (!CreateFrom( pInterpret, pSet1)) return false;

        pSI = ((CISOSet<Typ>*)pSet2)->m_pItems;
        while (pSI != NULL)
        {
            if (!Contains(pSI->Item))
            {
                pNewSI = AllocSSetItem();
                pNewSI->Item = pSI->Item;
                if (m_bObjUnitStruct  &&  *(CIObjUnitStruct**)&pNewSI->Item != NULL) 
                {
                    (*(CIObjUnitStruct**)&pNewSI->Item)->AddRef();
                }
                pNewSI->pNext = m_pItems;
                m_pItems = pNewSI;

                m_nItemCount++;
            }

            pSI = pSI->pNext;
        }

		return true;
    }

#define _help_SetOp1( cond) \
		if (!CISystemObject::Create( pInterpret)) return false;	\
	\
        ASSERT( pSet1->m_DataType == pSet2->m_DataType);    \
    \
        SSetItem *pNewSI, *pSI; \
    \
        m_DataType = pSet1->m_DataType; \
        m_pItems = NULL;    \
        m_nItemCount = 0;   \
        m_bObjUnitStruct = pSet1->m_DataType.IsObjectUnitStruct();  \
        m_pForItems = NULL; \
    \
        pSI = ((CISOSet<Typ>*)pSet1)->m_pItems; \
        while ( pSI != NULL)    \
        {   \
            if ( cond )    \
            {   \
                pNewSI = AllocSSetItem();   \
                pNewSI->Item = pSI->Item;   \
                if (m_bObjUnitStruct && *(CIObjUnitStruct**)&pNewSI->Item != NULL) \
                {  \
                    (*(CIObjUnitStruct**)&pNewSI->Item)->AddRef(); \
                }  \
                pNewSI->pNext = m_pItems;   \
                m_pItems = pNewSI;  \
    \
                m_nItemCount++; \
            }   \
            pSI = pSI->pNext;   \
        }   \
		return true; \
        \


    // vytvoreni mnoziny z pruniku
    virtual bool CreateFromIntersection( CInterpret *pInterpret, CISOSetParent *pSet1, CISOSetParent *pSet2)
    {
       _help_SetOp1( ((CISOSet<Typ>*)pSet2)->Contains( (Typ)pSI->Item));
    }

    // vytvoreni mnoziny z rozdilu
    virtual bool CreateFromDifference( CInterpret *pInterpret, CISOSetParent *pSet1, CISOSetParent *pSet2)
    {
       _help_SetOp1( !((CISOSet<Typ>*)pSet2)->Contains( pSI->Item));
    }

#undef _help_SetOp1

	// vytvoreni duplikatu dle jineho objektu
	virtual bool CreateFrom( CInterpret *pInterpret, CIDuplicSystemObject *pSrc)
    {
	    ASSERT( pSrc->GetObjectType() == SO_SET);

		if (!CISystemObject::Create( pInterpret)) return false;

        CISOSet<Typ> *pSet = (CISOSet<Typ>*)pSrc;
        SSetItem *pNewSI, *pSI;

        m_DataType = pSet->m_DataType;
        m_pItems = NULL;
        m_nItemCount = 0;
        m_bObjUnitStruct = pSet->m_bObjUnitStruct;
        m_pForItems = NULL;

        pSI = ((CISOSet<Typ>*)pSet)->m_pItems;
        while (pSI != NULL)
        {
            pNewSI = AllocSSetItem();
            pNewSI->Item = pSI->Item;
            if (m_bObjUnitStruct  && *(CIObjUnitStruct**)&pNewSI->Item != NULL) 
            {
                (*(CIObjUnitStruct**)&pNewSI->Item)->AddRef();
            }
            pNewSI->pNext = m_pItems;
            m_pItems = pNewSI;

            m_nItemCount++;

            pSI = pSI->pNext;
        }
		return true;
    }

    // zruseni mnoziny
    virtual void Delete()
    {
        if ( g_bGameLoading) 
        {
            while (m_pItems != NULL)
            {
                SSetItem *pSetItem = m_pItems;
                m_pItems = m_pItems->pNext;

                delete pSetItem;
            }

    		if (m_pForItems != NULL)
            {
                delete m_pForItems;
                m_pForItems = NULL;
            }

            return;
        }

        RemoveAll();
        SetForEnd();
    }

    // vyprazdneni mnoziny
    void RemoveAll()
    {
        SSetItem *pSI;

        while (m_pItems != NULL)
        {
            pSI = m_pItems;
            m_pItems = m_pItems->pNext;

            if (m_bObjUnitStruct && *(CIObjUnitStruct**)&pSI->Item != NULL) 
            {
                (*(CIObjUnitStruct**)&pSI->Item)->Release();
            }
            FreeSSetItem(pSI);
        }
        m_nItemCount = 0;
    }

    // dotaz zda je prvek v mnozine 
    virtual EInterpretStatus Contains( CIDataStackItem &DSI, bool &bResult)
    {
        Typ Item = NULL;
        DSI.Get( &Item);
		if ( m_bObjUnitStruct && Item == NULL) return STATUS_INVALID_REFERENCE;
        
		bResult = Contains( Item);
		return STATUS_SUCCESS;
    }
    
    virtual bool Contains( Typ Item)
    {
        SSetItem *pSI = m_pItems;

        while (pSI != NULL)
        {
            if ( Item == pSI->Item) return true;
            pSI = pSI->pNext;
        }

        return false;
    }

    // pridani prvku do mnoziny - pri interpretaci
    virtual EInterpretStatus AddItemToSet( CIDataStackItem &DSI)
    {
        Typ Item = NULL;

        DSI.Get( &Item);
		
		if ( m_bObjUnitStruct && Item == NULL) return STATUS_INVALID_REFERENCE;
        
		AddItemToSet(Item);
		
		return STATUS_SUCCESS;
    }

    // pridani prvku do mnoziny - z C++
    virtual void AddItemToSet( Typ Item)
    {
        if (!Contains(Item))
        {
            SSetItem *pNewSI;

            pNewSI = AllocSSetItem();
            pNewSI->Item = Item;
            if (m_bObjUnitStruct && *(CIObjUnitStruct**)&pNewSI->Item != NULL) 
            {
                (*(CIObjUnitStruct**)&pNewSI->Item)->AddRef();
            }

            pNewSI->pNext = m_pItems;
            m_pItems = pNewSI;

            m_nItemCount++;
        }
    }

    // vyjmuti prvku z mnoziny
    virtual EInterpretStatus RemoveItemFromSet( CIDataStackItem &DSI)
    {
        Typ Item = NULL;

        if (m_pItems == NULL) return STATUS_SUCCESS;

        DSI.Get( &Item);

		if (m_bObjUnitStruct && Item == NULL) return STATUS_INVALID_REFERENCE;

		RemoveItemFromSet( Item);

		return STATUS_SUCCESS;
	}


// vyjmuti prvku z mnoziny - z C++
    virtual void RemoveItemFromSet( Typ &Item)
	{
        SSetItem *pSI, *pRem;

        pSI = m_pItems;
        if ( Item == pSI->Item)
        {
            m_pItems = m_pItems->pNext;

            if (m_bObjUnitStruct && *(CIObjUnitStruct**)&pSI->Item != NULL) 
            {
                (*(CIObjUnitStruct**)&pSI->Item)->Release();
            }
            FreeSSetItem( pSI);
            m_nItemCount--;
        }
        else
        {
            while (pSI->pNext != NULL)
            {
                if ( Item == pSI->pNext->Item) 
                {
                    pRem = pSI->pNext;
                    pSI->pNext = pSI->pNext->pNext;

                    if (m_bObjUnitStruct  &&  *(CIObjUnitStruct**)&pRem->Item != NULL) 
                    {
                        (*(CIObjUnitStruct**)&pRem->Item)->Release();
                    }
                    FreeSSetItem( pRem);
                    m_nItemCount--;

                    return;
                }
                pSI = pSI->pNext;
            }
        }

    }
    

    // sjednoceni mnozin
    virtual void UnionWith( CISOSetParent *pSet)
    {
        SSetItem *pSI, *pNewSI;

        pSI = ((CISOSet<Typ>*)pSet)->m_pItems;
        while (pSI != NULL)
        {
            if (!Contains(pSI->Item))
            {
                pNewSI = AllocSSetItem();
                pNewSI->Item = pSI->Item;
                if (m_bObjUnitStruct  && *(CIObjUnitStruct**)&pNewSI->Item != NULL) 
                {
                    (*(CIObjUnitStruct**)&pNewSI->Item)->AddRef();
                }
                pNewSI->pNext = m_pItems;
                m_pItems = pNewSI;

                m_nItemCount++;
            }

            pSI = pSI->pNext;
        }
    }

#define _help_SetOp2( cond1, cond2)  \
        SSetItem *pSI, *pPom;   \
    \
        if (m_pItems == NULL) return;   \
    \
        pSI = m_pItems;     \
        while (pSI != NULL  && cond1 )    \
        {                       \
            pPom = pSI;         \
            pSI = pSI->pNext;   \
            m_pItems = pSI;     \
        \
            if (m_bObjUnitStruct  &&  *(CIObjUnitStruct**)&pPom->Item != NULL) \
            {  \
                (*(CIObjUnitStruct**)&pPom->Item)->Release();     \
            }  \
            FreeSSetItem( pPom);        \
            m_nItemCount--;     \
        }           \
        \
        pSI = m_pItems;     \
        \
        if (pSI != NULL)    \
        {   \
            while ( pSI->pNext != NULL)     \
            {       \
                if ( cond2 )       \
                {       \
                    pPom = pSI->pNext;      \
                    pSI->pNext = pSI->pNext->pNext;     \
            \
                    if (m_bObjUnitStruct && *(CIObjUnitStruct**)&pPom->Item != NULL)    \
                    {   \
                        (*(CIObjUnitStruct**)&pPom->Item)->Release();     \
                    }   \
                    FreeSSetItem( pPom);    \
                        \
                    m_nItemCount--; \
        \
                    if (pSI->pNext == NULL) break;  \
                }   \
                else pSI = pSI->pNext;   \
            }   \
        }   \

    // prunik mnozin
    virtual void IntersectWith( CISOSetParent *pSet)
    {
        _help_SetOp2( !((CISOSet<Typ>*)pSet)->Contains( pSI->Item),
                      !((CISOSet<Typ>*)pSet)->Contains( pSI->pNext->Item)); 
    }

    // rozdil mnozin
    virtual void DifferWith( CISOSetParent *pSet)
    {
        _help_SetOp2( ((CISOSet<Typ>*)pSet)->Contains( pSI->Item),
                      ((CISOSet<Typ>*)pSet)->Contains( pSI->pNext->Item));
    }


// FOR cyklus pro mnozinu
    virtual bool SetForFirst( CIDataStackItem &Var, CIDataStackItem &BoolRes)
    {
        SSetItem *pSI;

        if (m_pForItems != NULL) return false;

        if (m_nItemCount == 0) BoolRes.Set(false);
        else
        {
            BoolRes.Set(true);

            // okopirovani obsahu mnoziny
            m_pForItems = new Typ[ m_nItemCount ];
            
            m_nForItemsCount = 0;
            pSI = m_pItems;
            while (pSI != NULL)
            {
                m_pForItems[m_nForItemsCount++] = pSI->Item;
                if (m_bObjUnitStruct  &&  *(CIObjUnitStruct**)&pSI->Item != NULL)
                {
                    (*(CIObjUnitStruct**)&pSI->Item)->AddRef();
                }
                pSI = pSI->pNext;
            }
            
            ASSERT (m_nForItemsCount == m_nItemCount);
            m_nForItemCurrent = 0;

            // vraceni prvniho prvku
            Var.Set( m_pForItems[ m_nForItemCurrent++]);
        }
        return true;
    }

    virtual void SetForNext( CIDataStackItem &Var, CIDataStackItem &BoolRes)
    {
        ASSERT(m_pForItems != NULL);

        if (m_nForItemCurrent < m_nForItemsCount)
        {
            BoolRes.Set(true);
            Var.Set( m_pForItems[ m_nForItemCurrent++]);
        }
        else BoolRes.Set(false);
    }

    virtual void SetForEnd()
    {
        if ( m_pForItems == NULL) return;

        if (m_bObjUnitStruct) 
        {
            int i;
            for (i=0; i<m_nForItemsCount; i++) 
            {
                if (*(CIObjUnitStruct**)&m_pForItems[i] != NULL)
                {
                    (*(CIObjUnitStruct**)&m_pForItems[i])->Release();
                }
            }
        }

        delete [] m_pForItems;

        m_pForItems = NULL;
    }

// METODY
public:
    virtual ESyscallResult InvokeMethod( CIProcess *pCallingProcess, long nMethodID, CIBag *pBag, CISyscall **ppSyscall)
    { 
	    if (nMethodID < 0 || nMethodID >= _SCI_METHOD_COUNT || 
		    _SCI_m_aMethods[nMethodID] == NULL) 
	    { 
		    TRACE_INTERPRET3("%3d: object %s doesn't have any method with id %d!\n", pCallingProcess->m_nProcessID, GetStringFromSOType(m_nSOType), nMethodID); 
		    ASSERT(false); 
		    return SYSCALL_ERROR; 
	    } 
	    else return (this->*_SCI_m_aMethods[nMethodID])(pCallingProcess, pBag, ppSyscall); 
    } 

protected:

    DECLARE_SYSCALL_METHOD0(M_GetItemCount);
	  DECLARE_SYSCALL_METHOD0(M_IsEmpty);
    DECLARE_SYSCALL_METHOD0(M_SetEmpty);

// 0:
    ESyscallResult M_GetItemCount( CIProcess*, CISyscall**, CIDataStackItem *pRet)
    {
        pRet->Set( m_nItemCount);
        return SYSCALL_SUCCESS;
    }

// 1:
    ESyscallResult M_IsEmpty( CIProcess*, CISyscall**, CIDataStackItem *pRet)
    {
        ASSERT( (m_nItemCount == 0 && m_pItems == NULL) ||
                (m_nItemCount != 0 && m_pItems != NULL));

        pRet->Set( m_nItemCount == 0);
        return SYSCALL_SUCCESS;
    }

// 2:
    ESyscallResult M_SetEmpty( CIProcess*, CISyscall**, CIDataStackItem *pRet)
    {
        RemoveAll();
        ASSERT( m_nItemCount == 0 && m_pItems == NULL );
        return SYSCALL_SUCCESS;
    }

    enum {_SCI_METHOD_COUNT = 3 }; 
    ESyscallResult (CISOSet<Typ>::* _SCI_m_aMethods[_SCI_METHOD_COUNT])( CIProcess*, CIBag*, CISyscall**);

// polozky
protected:
    // spojak polozek
    SSetItem *m_pItems;
    // priznak, zda na polozky volat AddRef a Release
    bool m_bObjUnitStruct;
    
    // okopirovany obsah mnoziny pri for-cyklu
    Typ *m_pForItems;
    int m_nForItemsCount, m_nForItemCurrent;

/*
object Set
{
0: int GetItemCount();
1: bool IsEmpty();
}
*/

public:
// 
// SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage)
	{
        BRACE_BLOCK(storage);

        CISOSetParent::PersistentSave(storage);

		// priznak "objektovosti"
		storage << (BYTE)m_bObjUnitStruct;

		// items
		int i;
		SSetItem *pItem;

		for ( i=0, pItem=m_pItems; i<m_nItemCount; i++, pItem = pItem->pNext)
		{
			ASSERT( pItem != NULL);
			if (m_bObjUnitStruct) storage << (DWORD)pItem->Item;
			else storage << pItem->Item;
		}
		ASSERT( pItem == NULL);

		// for cyklus
		storage << (DWORD)m_pForItems;
		if (m_pForItems != NULL)
		{
			storage << m_nForItemsCount;
			storage << m_nForItemCurrent;

			storage.Write( m_pForItems, m_nForItemsCount * sizeof(Typ));
		}
	}

	void PersistentLoad( CPersistentStorage &storage)
	{
        BRACE_BLOCK(storage);

        CISOSetParent::PersistentLoad(storage);

		// priznak "objektovosti"
		storage >> (BYTE&)m_bObjUnitStruct;

		// items
		SSetItem *pItem, *pLastItem;
		int i;

        m_pItems = NULL;
		for (i=0; i<m_nItemCount; i++)
		{
			pItem = new SSetItem;
			pItem->pNext = NULL;

			storage >> (DWORD&)pItem->Item;
			
			if (i==0) m_pItems = pLastItem = pItem;
			else
			{
				pLastItem->pNext = pItem;
				pLastItem = pItem;
			}
		}

		// for cyklus
		storage >> (DWORD&)m_pForItems;
		if (m_pForItems != NULL)
		{
			storage >> m_nForItemsCount;
			storage >> m_nForItemCurrent;

			m_pForItems = new Typ[ m_nForItemsCount];
			storage.Read( m_pForItems, m_nForItemsCount * sizeof(Typ));
		}
		else
		{
			m_nForItemCurrent = m_nForItemsCount = 0;
		}

	}

	void PersistentTranslatePointers( CPersistentStorage &storage)
	{
		CISOSetParent::PersistentTranslatePointers(storage);

		SSetItem *pItem;
		void *pPom;
		int i;

		if (m_bObjUnitStruct)
		{
			for ( pItem = m_pItems;  pItem != NULL;  pItem = pItem->pNext)
			{
				pPom = storage.TranslatePointer( *(void**)&pItem->Item);
				pItem->Item = *(Typ*)&pPom;
			}
		}

		// for cyklus
		for (i=0; i<m_nForItemsCount; i++)
		{
			pPom = storage.TranslatePointer( *(void**)&m_pForItems[i]);
			m_pForItems[i] = *(Typ*)&pPom;
		}
	}

	void PersistentInit()
	{
		CISOSetParent::PersistentInit();
	}
};

#endif // !defined(AFX_ISOSet_H__380C8AF5_DFDE_11D3_AFE0_004F49068BD6__INCLUDED_)
