/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Abstraktn� datov� typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Memory pool
 * 
 ***********************************************************/

#ifndef __MEMORY_POOL__HEADER_INCLUDED__
#define __MEMORY_POOL__HEADER_INCLUDED__

#include <memory.h>

//////////////////////////////////////////////////////////////////////
// T��da memory poolu.
class CMemoryPool 
{
// Datov� typy
private:
	// hlavi�ka bloku dat
	struct SBlockHeader
	{
		// prvn� voln� polo�ka v bloku dat (NULL=blok je pln�)
		BYTE *pFirstFreeElement;
		// po�et pou�it�ch polo�ek v bloku dat
		DWORD dwElementCount;
		// ukazatel na dal�� blok dat
		SBlockHeader *pNextBlock;
	};
// Metody
public:
// Konstrukce a destrukce

	// konstruktor memory poolu s bloky po "dwBlockSize" prvc�ch
	inline CMemoryPool ( DWORD dwBlockSize );
	// destruktor
	inline ~CMemoryPool ();

// Inicializace a zni�en� dat objektu

	// vytvo�en� memory poolu prvk� velikosti "dwElementSize"
	inline void Create ( DWORD dwElementSize );
	// zni�en� memory poolu
	void Delete ();

	// zni�� nepot�ebnou pam�
	void FreeExtra ();

// Operace s prvky

	// alokuje nov� prvek
	//		v�jimky: CMemoryException
	void *Allocate ();
	// dealokuje prvek
	void Free ( void *pElement );
	// dealokuje v�echny prvky
	void FreeAll ();

// Data
private:
	// ukazatel na prvn� blok dat
	SBlockHeader *m_pFirstBlock;
	// ukazatel na prvn� voln� blok dat (nezahrnuje �pln� pr�zdn� bloky)
	SBlockHeader *m_pFirstFreeBlock;
	// ukazatel na pr�zdn� blok
	SBlockHeader *m_pEmptyBlock;
	// velikost prvku memory poolu
	DWORD m_dwElementSize;
	// po�et datov�ch polo�ek v bloku
	DWORD m_dwBlockSize;
};

//////////////////////////////////////////////////////////////////////
// Inline metody t��dy memory poolu

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor memory poolu s bloky po "dwBlockSize" prvc�ch
inline CMemoryPool::CMemoryPool ( DWORD dwBlockSize ) 
{
	// inicializuje data memory poolu
	m_pFirstBlock = NULL;
	m_pFirstFreeBlock = NULL;
	m_pEmptyBlock = NULL;
	m_dwElementSize = 0;

	// zkontroluje parametry
	ASSERT ( dwBlockSize > 0 );

	// inicializuje velikost bloku
	m_dwBlockSize = dwBlockSize;
}

// destruktor
inline CMemoryPool::~CMemoryPool () 
{
	ASSERT ( m_dwBlockSize > 0 );

	// zkontroluje data memory poolu
	ASSERT ( m_pFirstBlock == NULL );
	ASSERT ( m_pFirstFreeBlock == NULL );
	ASSERT ( m_pEmptyBlock == NULL );
	ASSERT ( m_dwElementSize == 0 );
}

//////////////////////////////////////////////////////////////////////
// Inicializace a zni�en� dat objektu
//////////////////////////////////////////////////////////////////////

// vytvo�en� memory poolu prvk� velikosti "dwElementSize"
inline void CMemoryPool::Create ( DWORD dwElementSize ) 
{
	ASSERT ( m_dwBlockSize > 0 );

	// zkontroluje data memory poolu
	ASSERT ( m_pFirstBlock == NULL );
	ASSERT ( m_pFirstFreeBlock == NULL );
	ASSERT ( m_pEmptyBlock == NULL );
	ASSERT ( m_dwElementSize == 0 );

	// zkontroluje parametry
	ASSERT ( dwElementSize > 0 );
	ASSERT ( dwElementSize >= sizeof ( BYTE * ) );

	// inicializuje velikost prvku
	m_dwElementSize = dwElementSize;
}

#endif //__MEMORY_POOL__HEADER_INCLUDED__
