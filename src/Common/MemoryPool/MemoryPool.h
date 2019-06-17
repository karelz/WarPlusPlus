/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Abstraktní datové typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Memory pool
 * 
 ***********************************************************/

#ifndef __MEMORY_POOL__HEADER_INCLUDED__
#define __MEMORY_POOL__HEADER_INCLUDED__

#include <memory.h>

//////////////////////////////////////////////////////////////////////
// Tøída memory poolu.
class CMemoryPool 
{
// Datové typy
private:
	// hlavièka bloku dat
	struct SBlockHeader
	{
		// první volná položka v bloku dat (NULL=blok je plný)
		BYTE *pFirstFreeElement;
		// poèet použitých položek v bloku dat
		DWORD dwElementCount;
		// ukazatel na další blok dat
		SBlockHeader *pNextBlock;
	};
// Metody
public:
// Konstrukce a destrukce

	// konstruktor memory poolu s bloky po "dwBlockSize" prvcích
	inline CMemoryPool ( DWORD dwBlockSize );
	// destruktor
	inline ~CMemoryPool ();

// Inicializace a znièení dat objektu

	// vytvoøení memory poolu prvkù velikosti "dwElementSize"
	inline void Create ( DWORD dwElementSize );
	// znièení memory poolu
	void Delete ();

	// znièí nepotøebnou pamì
	void FreeExtra ();

// Operace s prvky

	// alokuje nový prvek
	//		výjimky: CMemoryException
	void *Allocate ();
	// dealokuje prvek
	void Free ( void *pElement );
	// dealokuje všechny prvky
	void FreeAll ();

// Data
private:
	// ukazatel na první blok dat
	SBlockHeader *m_pFirstBlock;
	// ukazatel na první volný blok dat (nezahrnuje úplnì prázdné bloky)
	SBlockHeader *m_pFirstFreeBlock;
	// ukazatel na prázdný blok
	SBlockHeader *m_pEmptyBlock;
	// velikost prvku memory poolu
	DWORD m_dwElementSize;
	// poèet datových položek v bloku
	DWORD m_dwBlockSize;
};

//////////////////////////////////////////////////////////////////////
// Inline metody tøídy memory poolu

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor memory poolu s bloky po "dwBlockSize" prvcích
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
// Inicializace a znièení dat objektu
//////////////////////////////////////////////////////////////////////

// vytvoøení memory poolu prvkù velikosti "dwElementSize"
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
