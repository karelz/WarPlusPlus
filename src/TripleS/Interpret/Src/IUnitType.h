/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CIUnitType uchovavajici informace
 *          o jednom typu jednotky.
 *          
 ***********************************************************/

#if !defined(AFX_IUNITTYPE_H__574E728A_986E_11D3_AF75_004F49068BD6__INCLUDED_)
#define AFX_IUNITTYPE_H__574E728A_986E_11D3_AF75_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIUnitType;

class CIUnitTypeList
{
    DECLARE_MULTITHREAD_POOL( CIUnitTypeList);
public:
    CIUnitTypeList( CIUnitType *pIUnitType, CIUnitTypeList *pNext) 
    { 
        ASSERT( pIUnitType != NULL);
        m_pIUnitType = pIUnitType;
        m_pNext = pNext;
    }

    CIUnitType *m_pIUnitType;
    CIUnitTypeList *m_pNext;
};

class CIUnitType : public CIStructureType
{
public:
	CIUnitType();
	virtual ~CIUnitType();

    // natazeni jednotky z disku
    // throws CFileException, CStringException
    void Load(CFile &file, CIUnitType *pParentUnit);

    // Cleaning
    void Clean();

public:
    // query metody
    EImplementation GetEvent(CStringTableItem *stiName, CIEvent* &pEvent);
    EImplementation GetMethod(CStringTableItem *stiName, CIMethod* &pMethod);
    EImplementation GetMember(CStringTableItem *stiName, CIMember* &pMember);

    // update
    void UpdateUnitTypeFrom(CIUnitType *pFrom);
    // update offsetu
    void ChangeHeapOffset( int nOffsetDelta);

public:
    // jmeno  - zdedene z CIStructure
    
    // jmeno predka
    CStringTableItem *m_stiParentName;
    // ukazatel na typ predka
    CIUnitType *m_pParent;

    // polozky (members) - zdedene z CIStructure

    // konstruktor 
    CIConstructor *m_pConstructor;

    // destruktor 
    CIDestructor *m_pDestructor;

    // pocet metod
    short m_nMethodCount;
    // metody
    CTypedPtrMap<CMapPtrToPtr, CStringTableItem*, CIMethod*> m_tpMethods;

    // pocet eventu
    short m_nEventCount;
    // eventy
    CTypedPtrMap<CMapPtrToPtr, CStringTableItem*, CIEvent*> m_tpEvents;
    
    // tabulky
    CICodeStringTable *m_pCodeStringTable;
    CFloatTable *m_pFloatTable;

public:
    // priznak, zda byl objekt spravne natazen
    bool m_bLoaded;

#ifdef _DEBUG
public:
	virtual void AssertValid() const;
//	virtual void Dump(CDumpContext& dc) const;
#endif

};

#endif // !defined(AFX_IUNITTYPE_H__574E728A_986E_11D3_AF75_004F49068BD6__INCLUDED_)
