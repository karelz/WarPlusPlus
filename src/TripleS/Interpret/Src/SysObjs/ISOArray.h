// ISOArray.h: interface for the CISOArray class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISOARRAY_H__380C8AF5_DFDE_11D3_AFE0_004F49068BD6__INCLUDED_)
#define AFX_ISOARRAY_H__380C8AF5_DFDE_11D3_AFE0_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ISOArrayParent.h"

class CISyscall;

template<class Typ> class CISOArray : public CISOArrayParent
{
public:
    CISOArray() : CISOArrayParent() 
    {
        _SCI_m_aMethods[0] = _SCI_M_M_GetLength;
        _SCI_m_aMethods[1] = _SCI_M_M_Resize;
		m_pItems = NULL;
    }

    virtual ~CISOArray() {}

    // vytvoreni objektu
    virtual bool Create(CInterpret *pInterpret, CType *pType, int nArraySize)
    {
		if (!CISystemObject::Create( pInterpret)) return false;

        ASSERT(pType != NULL);
        ASSERT(nArraySize != 0);

        try
        {
            m_DataType = *pType;
            m_pItems = new Typ[ nArraySize];
            ::ZeroMemory( m_pItems, nArraySize * sizeof(Typ));
            m_nArraySize = nArraySize;
            m_bObjUnitStruct = pType->IsObjectUnitStruct();

            // vicerozmerne pole
            if (pType->IsArray())
            {
                ASSERT(pType->m_nArraySize != NULL);
                ASSERT(pType->m_pOfType != NULL);
        
                int i,j;

                for ( i = 0; i < nArraySize; i++)
                {
                    switch (pType->m_pOfType->m_nType)
                    {
                    case T_CHAR:    *(CISOArrayParent**)&m_pItems[i] = new CISOArray<char>;             break;
                    case T_INT:     *(CISOArrayParent**)&m_pItems[i] = new CISOArray<int>;              break;
                    case T_FLOAT:   *(CISOArrayParent**)&m_pItems[i] = new CISOArray<double>;           break;
                    case T_BOOL:    *(CISOArrayParent**)&m_pItems[i] = new CISOArray<bool>;             break;
                    case T_OBJECT:  *(CISOArrayParent**)&m_pItems[i] = new CISOArray<CISystemObject*>;  break;
                    case T_UNIT:    *(CISOArrayParent**)&m_pItems[i] = new CISOArray<CZUnit*>;          break;
                    case T_STRUCT:  *(CISOArrayParent**)&m_pItems[i] = new CISOArray<CIStructure*>;     break;
                    default:  ASSERT(false);
                    }

                    if ( ! (*(CISOArrayParent**)&m_pItems[i])->Create(m_pInterpret, pType->m_pOfType, pType->m_nArraySize))
                    {
                        // vse vytvorene uvolnit (zpatky do konzistentniho stavu :-)
                        delete *(CISOArray<CISystemObject*> **) &m_pItems[i];
                        m_pItems[i] = NULL;

                        for (j=0; j<i; j++)
                        {
                            (*(CISOArrayParent**)&m_pItems[i])->Release();  // counter klesne na 0 a ulovni se to samo
                            m_pItems[j] = NULL;
                        }
                        return false;
                    }
                    (*(CISOArrayParent**)&m_pItems[i])->AddRef();
                }
            }
        }
        catch (CException *e)
        {
            e->Delete();
            return false;
        }

        return true;
    }

	// vytvoreni duplikatu podle jineho objektu
	virtual bool CreateFrom( CInterpret *pInterpret, CIDuplicSystemObject *pSrc)
    {
		ASSERT( pSrc->GetObjectType() == SO_ARRAY);
		ASSERT( m_pItems == NULL); // not created

		CISOArrayParent *pFrom = (CISOArrayParent*)pSrc;

		ASSERT( pFrom != NULL);
		ASSERT( pFrom->m_nArraySize != 0);

        try
        {
			if (!Create( pInterpret, &pFrom->m_DataType, pFrom->m_nArraySize)) return false;

			::CopyMemory( m_pItems, ((CISOArray<Typ>*)pFrom)->m_pItems, m_nArraySize * sizeof(Typ));

			if ( m_bObjUnitStruct)
			{
				int i;
				for (i=0; i<m_nArraySize; i++) 
				{
					if (m_pItems[i] != NULL) (*(CIObjUnitStruct**)&m_pItems[i])->AddRef();
				}
			}
        }
        catch (CException *e)
        {
			ASSERT(false);
            e->Delete();
			Delete();
        }

        return true;
    }

    // zruseni objektu
    virtual void Delete()
    {
        if (g_bGameLoading)
        {
            delete m_pItems;
            m_pItems = NULL;
            return;
        }

        int i;
        if (m_bObjUnitStruct)
        {
            for ( i = 0; i < m_nArraySize; i++)
            {
                if (m_pItems[i] != NULL) (*(CISOArrayParent**)&m_pItems[i])->Release();  
            }
        }
            
        delete [] m_pItems;
        m_nArraySize = 0;
		m_pItems = NULL;
    }

	// indexace []
    virtual bool SetAt(int i, CIDataStackItem *pDSI)
    {
	    if (i<0 || i>=m_nArraySize) return false;
	    else 
	    {
            if (m_bObjUnitStruct)
            {
                if (m_pItems[i] != NULL) (*(CIObjUnitStruct**)&m_pItems[i])->Release();

				if (pDSI == NULL)
				{
					*(CIObjUnitStruct**)&m_pItems[i] = NULL;
				}
				else 
				{
					pDSI->Get( &m_pItems[i]);
					if (m_pItems[i] != NULL) (*(CIObjUnitStruct**)&m_pItems[i])->AddRef();
				}
            }
            else 
			{
				ASSERT( pDSI != NULL);
				pDSI->Get( &m_pItems[i]);
			}
		    return true;
	    }
    }
    virtual bool GetAt(int i, CIDataStackItem &DSI)
    { 
	    if (i<0 || i>=m_nArraySize) return false;
	    else 
	    {
            DSI.Set( m_pItems[i]);
		    return true;
	    }
    }

	bool SetAt(int i, Typ Item)
	{
		if (i<0 || i>=m_nArraySize) return false;
	    else 
	    {
            if (m_bObjUnitStruct)
            {
                if (m_pItems[i] != NULL) (*(CIObjUnitStruct**)&m_pItems[i])->Release();
                m_pItems[i] = Item;
                (*(CIObjUnitStruct**)&m_pItems[i])->AddRef();
            }
            else m_pItems[i] = Item;
		    return true;
	    }
	}

    bool GetAt(int i, Typ &Item)
    { 
	    if (i<0 || i>=m_nArraySize) return false;
	    else 
	    {
            Item = m_pItems[i];
		    return true;
	    }
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

    DECLARE_SYSCALL_METHOD0(M_GetLength);
	DECLARE_SYSCALL_METHOD1V(M_Resize, Int, int*);

// 0:
    ESyscallResult M_GetLength( CIProcess*, CISyscall**, CIDataStackItem *pRet)
    {
        pRet->Set( m_nArraySize);
        return SYSCALL_SUCCESS;
    }

// 1:
    ESyscallResult M_Resize( CIProcess*, CISyscall**, int *pNewSize)
    {
        if (*pNewSize <= 0 || *pNewSize > MAX_ARRAY_SIZE) return SYSCALL_ERROR;

        Typ *pNewItems;
        int nSize = (*pNewSize < m_nArraySize) ? *pNewSize : m_nArraySize;

        pNewItems = new Typ[ *pNewSize];
		::ZeroMemory( pNewItems + nSize, (*pNewSize - nSize)*sizeof(Typ));
        ::CopyMemory( pNewItems, m_pItems, nSize * sizeof(Typ));

        delete [] m_pItems;

        m_pItems = pNewItems;
        m_nArraySize = *pNewSize;

        return SYSCALL_SUCCESS;
    }


    enum {_SCI_METHOD_COUNT = 2 }; 
    ESyscallResult (CISOArray<Typ>::* _SCI_m_aMethods[_SCI_METHOD_COUNT])( CIProcess*, CIBag*, CISyscall**);


// polozky
protected:
    // samotne pole
    Typ *m_pItems;
    // priznak, zda na polozky volat AddRef a Release
    bool m_bObjUnitStruct;
/*
object Array
{
0: int GetLength();
1: void Resize(int newsize);
}
*/

public:
// 
// SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage)
	{
        BRACE_BLOCK(storage);

        CISOArrayParent::PersistentSave(storage);

		storage.Write( m_pItems, m_nArraySize * sizeof(Typ));
		storage << (BYTE)m_bObjUnitStruct;
	}

	void PersistentLoad( CPersistentStorage &storage)
	{
        BRACE_BLOCK(storage);

		CISOArrayParent::PersistentLoad(storage);

		m_pItems = new Typ[ m_nArraySize];
		storage.Read( m_pItems, m_nArraySize * sizeof(Typ));
		
		storage >> (BYTE&)m_bObjUnitStruct;
	}

	void PersistentTranslatePointers( CPersistentStorage &storage)
	{
		CISOArrayParent::PersistentTranslatePointers(storage);
		int i;
		void *pPom;

		if (m_bObjUnitStruct)
		{
			for (i=0; i<m_nArraySize; i++)
			{
				pPom = storage.TranslatePointer( *(void**)&(m_pItems[i]));
				m_pItems[i] = *(Typ*)&pPom;
			}
		}
	}

	void PersistentInit()
	{
		CISOArrayParent::PersistentInit();
	}
};

#endif // !defined(AFX_ISOARRAY_H__380C8AF5_DFDE_11D3_AFE0_004F49068BD6__INCLUDED_)
