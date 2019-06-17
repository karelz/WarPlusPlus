/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Abstraktn� datov� typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Multi threadov� typovan� memory pool
 * 
 ***********************************************************/

#ifndef __MULTI_THREAD_TYPED_MEMORY_POOL__HEADER_INCLUDED__
#define __MULTI_THREAD_TYPED_MEMORY_POOL__HEADER_INCLUDED__

#include <memory.h>
#include <afxmt.h>
#include "TypedMemoryPool.h"

//////////////////////////////////////////////////////////////////////
// �ablona memory poolu pro data "Element".
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

	// alokuje nov� prvek
	//		v�jimky: CMemoryException
	inline Element *Allocate ();
	// dealokuje prvek
	inline void Free ( Element *pElement );
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
// Operace s prvky
//////////////////////////////////////////////////////////////////////

// alokuje nov� prvek
//		v�jimky: CMemoryException
template <class Element>
inline Element *CMultiThreadTypedMemoryPool<Element>::Allocate () 
{
	// zamkne z�mek memory poolu
	VERIFY ( m_mutexLock.Lock () );

	// alokuje nov� prvek
	Element *pElement = CTypedMemoryPool<Element>::Allocate ();

	// odemkne z�mek memory poolu
	VERIFY ( m_mutexLock.Unlock () );

	// vr�t� alokovan� prvek
	return pElement;
}

// dealokuje prvek
template <class Element> 
inline void CMultiThreadTypedMemoryPool<Element>::Free ( Element *pElement ) 
{
	// zamkne z�mek memory poolu
	VERIFY ( m_mutexLock.Lock () );

	// dealokuje prvek
	CTypedMemoryPool<Element>::Free ( pElement );

	// odemkne z�mek memory poolu
	VERIFY ( m_mutexLock.Unlock () );
}

// zni�� nepot�ebnou pam�
template <class Element> 
inline void CMultiThreadTypedMemoryPool<Element>::FreeExtra () 
{
	// zamkne z�mek memory poolu
	VERIFY ( m_mutexLock.Lock () );

	// zni�� nepot�ebnou pam�
	CTypedMemoryPool<Element>::FreeExtra ( pElement );

	// odemkne z�mek memory poolu
	VERIFY ( m_mutexLock.Unlock () );
}

// dealokuje v�echny prvky
template <class Element> 
inline void CMultiThreadTypedMemoryPool<Element>::FreeAll () 
{
	// zamkne z�mek memory poolu
	VERIFY ( m_mutexLock.Lock () );

	// dealokuje prvek
	CTypedMemoryPool<Element>::FreeAll ( pElement );

	// odemkne z�mek memory poolu
	VERIFY ( m_mutexLock.Unlock () );
}

#endif //__MULTI_THREAD_TYPED_MEMORY_POOL__HEADER_INCLUDED__
