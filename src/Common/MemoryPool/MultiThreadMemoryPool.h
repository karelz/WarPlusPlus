/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Abstraktn� datov� typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Multi thread memory pool
 * 
 ***********************************************************/

#ifndef __MULTI_THREAD_MEMORY_POOL__HEADER_INCLUDED__
#define __MULTI_THREAD_MEMORY_POOL__HEADER_INCLUDED__

#include <memory.h>
#include <afxmt.h>
#include "MemoryPool.h"

//////////////////////////////////////////////////////////////////////
// T��da multi thread memory poolu.
class CMultiThreadMemoryPool : private CMemoryPool 
{
// Metody
public:
// Konstrukce a destrukce

	// konstruktor memory poolu s bloky po "nBlockSize" prvc�ch
	CMultiThreadMemoryPool ( DWORD dwBlockSize ) : m_mutexLock (), 
		CMemoryPool ( dwBlockSize ) {};

// Inicializace a zni�en� dat objektu

	// vytvo�en� memory poolu prvk� velikosti "dwElementSize"
	inline void Create ( DWORD dwElementSize );
	// zni�en� memory poolu
	inline void Delete ();

// Operace s prvky

	// alokuje nov� prvek
	//		v�jimky: CMemoryException
	inline void *Allocate ();
	// dealokuje prvek
	inline void Free ( void *pElement );
	// zni�� nepot�ebnou pam�
	inline void FreeExtra ();
	// dealokuje v�echny prvky
	inline void FreeAll ();

// Data
private:
	// z�mek memory poolu
	CCriticalSection m_mutexLock;
};

//////////////////////////////////////////////////////////////////////
// Inicializace a zni�en� dat objektu
//////////////////////////////////////////////////////////////////////

// vytvo�en� memory poolu prvk� velikosti "dwElementSize"
inline void CMultiThreadMemoryPool::Create ( DWORD dwElementSize ) 
{
	// vytvo�� memory pool velikosti "dwElementSize"
	CMemoryPool::Create ( dwElementSize );
}

// zni�en� memory poolu
inline void CMultiThreadMemoryPool::Delete () 
{
	// zni�� memory pool
	CMemoryPool::Delete ();
}

//////////////////////////////////////////////////////////////////////
// Operace s prvky
//////////////////////////////////////////////////////////////////////

// alokuje nov� prvek
//		v�jimky: CMemoryException
inline void *CMultiThreadMemoryPool::Allocate () 
{
	// zamkne z�mek memory poolu
	VERIFY ( m_mutexLock.Lock () );

	// alokuje nov� prvek
	void *pElement = CMemoryPool::Allocate ();

	// odemkne z�mek memory poolu
	VERIFY ( m_mutexLock.Unlock () );

	// vr�t� alokovan� prvek
	return pElement;
}

// dealokuje prvek
inline void CMultiThreadMemoryPool::Free ( void *pElement ) 
{
	// zamkne z�mek memory poolu
	VERIFY ( m_mutexLock.Lock () );

	// dealokuje prvek
	CMemoryPool::Free ( pElement );

	// odemkne z�mek memory poolu
	VERIFY ( m_mutexLock.Unlock () );
}

// zni�� nepot�ebnou pam�
inline void CMultiThreadMemoryPool::FreeExtra () 
{
	// zamkne z�mek memory poolu
	VERIFY ( m_mutexLock.Lock () );

	// zni�� nepot�ebnou pam�
	CMemoryPool::FreeExtra ();

	// odemkne z�mek memory poolu
	VERIFY ( m_mutexLock.Unlock () );
}

// dealokuje v�echny prvky
inline void CMultiThreadMemoryPool::FreeAll () 
{
	// zamkne z�mek memory poolu
	VERIFY ( m_mutexLock.Lock () );

	// dealokuje v�echny prvky
	CMemoryPool::FreeAll ();

	// odemkne z�mek memory poolu
	VERIFY ( m_mutexLock.Unlock () );
}

#endif //__MULTI_THREAD_MEMORY_POOL__HEADER_INCLUDED__
