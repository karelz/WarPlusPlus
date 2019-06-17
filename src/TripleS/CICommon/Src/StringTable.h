/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac, Interpret, CodeManager, Civilization, GameServer
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace trid CStringTable a CStringTableItem,
 *          globalnich tabulek stringu.
 * 
 ***********************************************************/

#if !defined(AFX_STRINGTABLE_H__247E9183_32C6_11D3_AF0C_004F49068BD6__INCLUDED_)
#define AFX_STRINGTABLE_H__247E9183_32C6_11D3_AF0C_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Common\MemoryPool\MemoryPool.h"

#include "Common\PersistentStorage\PersistentObject.h"

class CStringTable;

// class CStringTableItem 
// ======================
// Reprezentuje jednu polozku v tabulce stringu (CStringTable).
class CStringTableItem : public CPersistentObject
{
//	DECLARE_MULTITHREAD_POOL( CStringTableItem);
public:
  inline void* operator new( size_t nSize);
  inline void  operator delete( void* p);
#ifdef _DEBUG
  inline void* operator new( size_t nSize, LPCSTR, int);
  inline void  operator delete( void* p, LPCSTR, int);
#endif //_DEBUG
//    CStringTableItem() {}
//    virtual ~CStringTableItem() {}

    // inicializace 
    void Initialize(CString &strString);

    // pridani reference, vraci samo sebe
    CStringTableItem* AddRef();
    // snizeni poctu referenci, vraci NULL
    CStringTableItem* Release(); 

    // operatory
    operator LPCTSTR () const;
    operator CString () const;

// DATA
public:
    // retezec
    CString m_strString;
    // pocitadlo referenci
    LONG m_nRefCount;

#ifdef _DEBUG
public:
	virtual void Dump(CDumpContext& dc) const;
#endif
};

// class CStringTable
// ==================
// Trida pro reprezentaci tabulky stringu. Retezce se do tabulky hasuji 
// pomoci MFC tridy CTypedPtrMap.
// Instance teto tridy se nebudou vytvaret, bude pouze jedna globalni g_StringTable. 
class CStringTable : public CPersistentObject
{
    friend CStringTableItem;

public:
	CStringTable();
	virtual ~CStringTable();

    // Pridavani do tabulky - CString, LPCTSTR.
    // bAddRef urcuje, zda se ma stringu zvednou pocitadlo referenci.
	CStringTableItem* AddItem(CString &strString, bool bAddRef=true);
	CStringTableItem* AddItem(LPCTSTR string, bool bAddRef=true);

    // Vyprazdneni tabulky, bez ohledu na reference count
    void RemoveAll();


protected:
    // Ubirani z tabulky - vola se z CStringTableItem::Release, kdyz counter klesne na 0.
    void ReleaseItem(CStringTableItem* pItem);

protected:
    // velikost hashovaci tabulky
    enum { HASHTABLE_SIZE = 2047 };
    
    // hasovaci tabulka
	CTypedPtrMap<CMapStringToPtr, CString, CStringTableItem*> m_apSTITable;

	static CMultiThreadTypedMemoryPool<CStringTableItem> *m_pPool;
	static DWORD m_dwPoolReferenceCount;
	CCriticalSection m_lockPool;

	// pocet polozek
	long m_nItemCount;

    // zamek nad pridavanim do tabulky 
    CCriticalSection m_StringTableLock;

public:
//
// SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();
};

//
// Globalni stringtable
//
extern CStringTable g_StringTable;


void *CStringTableItem::operator new( size_t nSize){ return CStringTable::m_pPool->Allocate();    }
void CStringTableItem::operator delete( void *p ){  CStringTable::m_pPool->Free( (CStringTableItem*)p); }
#ifdef _DEBUG
void *CStringTableItem::operator new( size_t nSize, LPCSTR, int){ return CStringTable::m_pPool->Allocate();    }
void CStringTableItem::operator delete( void *p, LPCSTR, int ){ CStringTable::m_pPool->Free( (CStringTableItem*)p); }
#endif //_DEBUG

#endif // !defined(AFX_STRINGTABLE_H__247E9183_32C6_11D3_AF0C_004F49068BD6__INCLUDED_)
