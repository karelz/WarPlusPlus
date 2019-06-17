/***********************************************************
 * 
 * Projekt: Strategick· hra
 *    »·st: AbstraktnÌ datovÈ typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Multi threadov˝ typovan˝ memory pool
 * 
 ***********************************************************/

#ifndef __MULTI_THREAD_TYPED_MEMORY_POOL__HEADER_INCLUDED__
#define __MULTI_THREAD_TYPED_MEMORY_POOL__HEADER_INCLUDED__

#include <memory.h>
#include <afxmt.h>
#include "TypedMemoryPool.h"

//////////////////////////////////////////////////////////////////////
// äablona memory poolu pro data "Element".
template <class Element>
class CMultiThreadTypedMemoryPool : private CTypedMemoryPool<Element> 
{
// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CMultiThreadTypedMemoryPool ( DWORD dwBlockSize ) : m_mutexLock (), 
		CTypedMemoryPool<Element> ( dwBlockSize ) {};

// Operace s prvky

	// alokuje nov˝ prvek
	//		v˝jimky: CMemoryException
	inline Element *Allocate ();
	// dealokuje prvek
	inline void Free ( Element *pElement );
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
// Operace s prvky
//////////////////////////////////////////////////////////////////////

// alokuje nov˝ prvek
//		v˝jimky: CMemoryException
template <class Element>
inline Element *CMultiThreadTypedMemoryPool<Element>::Allocate () 
{
	// zamkne z·mek memory poolu
	VERIFY ( m_mutexLock.Lock () );

	// alokuje nov˝ prvek
	Element *pElement = CTypedMemoryPool<Element>::Allocate ();

	// odemkne z·mek memory poolu
	VERIFY ( m_mutexLock.Unlock () );

	// vr·tÌ alokovan˝ prvek
	return pElement;
}

// dealokuje prvek
template <class Element> 
inline void CMultiThreadTypedMemoryPool<Element>::Free ( Element *pElement ) 
{
	// zamkne z·mek memory poolu
	VERIFY ( m_mutexLock.Lock () );

	// dealokuje prvek
	CTypedMemoryPool<Element>::Free ( pElement );

	// odemkne z·mek memory poolu
	VERIFY ( m_mutexLock.Unlock () );
}

// zniËÌ nepot¯ebnou pamÏù
template <class Element> 
inline void CMultiThreadTypedMemoryPool<Element>::FreeExtra () 
{
	// zamkne z·mek memory poolu
	VERIFY ( m_mutexLock.Lock () );

	// zniËÌ nepot¯ebnou pamÏù
	CTypedMemoryPool<Element>::FreeExtra ( pElement );

	// odemkne z·mek memory poolu
	VERIFY ( m_mutexLock.Unlock () );
}

// dealokuje vöechny prvky
template <class Element> 
inline void CMultiThreadTypedMemoryPool<Element>::FreeAll () 
{
	// zamkne z·mek memory poolu
	VERIFY ( m_mutexLock.Lock () );

	// dealokuje prvek
	CTypedMemoryPool<Element>::FreeAll ( pElement );

	// odemkne z·mek memory poolu
	VERIFY ( m_mutexLock.Unlock () );
}

#endif //__MULTI_THREAD_TYPED_MEMORY_POOL__HEADER_INCLUDED__
