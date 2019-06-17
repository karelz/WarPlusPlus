/***********************************************************
 * 
 * Projekt: Strategick· hra
 *    »·st: AbstraktnÌ datovÈ typy
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
// T¯Ìda multi thread memory poolu.
class CMultiThreadMemoryPool : private CMemoryPool 
{
// Metody
public:
// Konstrukce a destrukce

	// konstruktor memory poolu s bloky po "nBlockSize" prvcÌch
	CMultiThreadMemoryPool ( DWORD dwBlockSize ) : m_mutexLock (), 
		CMemoryPool ( dwBlockSize ) {};

// Inicializace a zniËenÌ dat objektu

	// vytvo¯enÌ memory poolu prvk˘ velikosti "dwElementSize"
	inline void Create ( DWORD dwElementSize );
	// zniËenÌ memory poolu
	inline void Delete ();

// Operace s prvky

	// alokuje nov˝ prvek
	//		v˝jimky: CMemoryException
	inline void *Allocate ();
	// dealokuje prvek
	inline void Free ( void *pElement );
	// zniËÌ nepot¯ebnou pamÏù
	inline void FreeExtra ();
	// dealokuje vöechny prvky
	inline void FreeAll ();

// Data
private:
	// z·mek memory poolu
	CCriticalSection m_mutexLock;
};

//////////////////////////////////////////////////////////////////////
// Inicializace a zniËenÌ dat objektu
//////////////////////////////////////////////////////////////////////

// vytvo¯enÌ memory poolu prvk˘ velikosti "dwElementSize"
inline void CMultiThreadMemoryPool::Create ( DWORD dwElementSize ) 
{
	// vytvo¯Ì memory pool velikosti "dwElementSize"
	CMemoryPool::Create ( dwElementSize );
}

// zniËenÌ memory poolu
inline void CMultiThreadMemoryPool::Delete () 
{
	// zniËÌ memory pool
	CMemoryPool::Delete ();
}

//////////////////////////////////////////////////////////////////////
// Operace s prvky
//////////////////////////////////////////////////////////////////////

// alokuje nov˝ prvek
//		v˝jimky: CMemoryException
inline void *CMultiThreadMemoryPool::Allocate () 
{
	// zamkne z·mek memory poolu
	VERIFY ( m_mutexLock.Lock () );

	// alokuje nov˝ prvek
	void *pElement = CMemoryPool::Allocate ();

	// odemkne z·mek memory poolu
	VERIFY ( m_mutexLock.Unlock () );

	// vr·tÌ alokovan˝ prvek
	return pElement;
}

// dealokuje prvek
inline void CMultiThreadMemoryPool::Free ( void *pElement ) 
{
	// zamkne z·mek memory poolu
	VERIFY ( m_mutexLock.Lock () );

	// dealokuje prvek
	CMemoryPool::Free ( pElement );

	// odemkne z·mek memory poolu
	VERIFY ( m_mutexLock.Unlock () );
}

// zniËÌ nepot¯ebnou pamÏù
inline void CMultiThreadMemoryPool::FreeExtra () 
{
	// zamkne z·mek memory poolu
	VERIFY ( m_mutexLock.Lock () );

	// zniËÌ nepot¯ebnou pamÏù
	CMemoryPool::FreeExtra ();

	// odemkne z·mek memory poolu
	VERIFY ( m_mutexLock.Unlock () );
}

// dealokuje vöechny prvky
inline void CMultiThreadMemoryPool::FreeAll () 
{
	// zamkne z·mek memory poolu
	VERIFY ( m_mutexLock.Lock () );

	// dealokuje vöechny prvky
	CMemoryPool::FreeAll ();

	// odemkne z·mek memory poolu
	VERIFY ( m_mutexLock.Unlock () );
}

#endif //__MULTI_THREAD_MEMORY_POOL__HEADER_INCLUDED__
