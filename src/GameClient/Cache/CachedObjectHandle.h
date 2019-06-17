// CachedObjectHandle.h: interface for the CCachedObjectHandle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CACHEDOBJECTHANDLE_H__F09B4E6A_F452_11D3_BFFB_0000B4A08F9A__INCLUDED_)
#define AFX_CACHEDOBJECTHANDLE_H__F09B4E6A_F452_11D3_BFFB_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CachedObject.h"

// Drzak na CCachedObject, pouzitelny pro zarazovani objektu do struktur cache
class CCachedObjectHandle  
{
	/////////////////////////////////////////////////////
	// Konstrukce a destrukce
public:
	// Konstruktor
	CCachedObjectHandle(CCachedObject *pCachedObject);

	// Destruktor
	virtual ~CCachedObjectHandle();

	/////////////////////////////////////////////////////
	// Prace s daty
public:
	// Vrati referenci objekt
	CCachedObject *GetCachedObject() { return m_pCachedObject; }

	// Rekne objektu, aby se nahral
	void Load();

    // Rekne objektu, aby se nahral, za predpokladu, ze jiz BYL
    // zamknut na zapis
    void LoadAfterBeingWriteLocked();

    // Zamkne objekt na zapis
    void LockWrite();

	// Nastavi dalsi objekt v rade
	void NextHandle(CCachedObjectHandle *pNextHandle) {
		m_pNextHandle=pNextHandle;
	}

	// Zjisti dalsiho ze struktury cache
	CCachedObjectHandle *NextHandle() {
		return m_pNextHandle;
	}

	// Nastavi predchozi objekt v rade
	void PrevHandle(CCachedObjectHandle *pPrevHandle) {
		m_pPrevHandle=pPrevHandle;
	}

	// Zjisti predchoziho ze struktury cache
	CCachedObjectHandle *PrevHandle() {
		return m_pPrevHandle;
	}	

	// Nastavi dalsiho do fronty na load
	void NextLoad(CCachedObjectHandle *pNextLoad) {
		m_pNextLoad=pNextLoad;
	}

	// Zjisti dalsiho z fronty na load
	CCachedObjectHandle *NextLoad() {
		return m_pNextLoad;
	}

	// Bylo na nas pristoupeno
	void Hit() { m_dwHitCounter++; }

	// Reset pocitadla
	void ResetHits() { m_dwHitCounter=0; }

	// Vrati pocet pristupu
	DWORD NumHits() { return m_dwHitCounter; }

	/////////////////////////////////////////////////////
	// Data
private:
	// Cachovany objekt
	CCachedObject *m_pCachedObject;

	// Ukaatel na dalsi a predchozi handle ve strukture cache
	CCachedObjectHandle *m_pNextHandle, *m_pPrevHandle;

	// Ukazatel na dalsi handle ve spojaku na loadeni
	CCachedObjectHandle *m_pNextLoad;
	
	// Pocitadlo pristupu
	DWORD m_dwHitCounter;
};

#endif // !defined(AFX_CACHEDOBJECTHANDLE_H__F09B4E6A_F452_11D3_BFFB_0000B4A08F9A__INCLUDED_)
