/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Abstraktní datové typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Memory pool
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "Common\AfxDebugPlus\AfxDebugPlus.h"
#include "MemoryPool.h"

//////////////////////////////////////////////////////////////////////
// Inicializace a znièení dat objektu
//////////////////////////////////////////////////////////////////////

// znièení memory poolu
void CMemoryPool::Delete () 
{
	ASSERT ( m_dwBlockSize > 0 );

	// zjistí, je-li memory pool platný
	if ( m_dwElementSize == 0 )
	{	// memory pool není platný
		// zkontroluje neplatnost memory poolu
		ASSERT ( m_pFirstBlock == NULL );
		ASSERT ( m_pFirstFreeBlock == NULL );
		ASSERT ( m_pEmptyBlock == NULL );
		ASSERT ( m_dwElementSize == 0 );
		// ukonèí nièení neplatného memory poolu
		return;
	}

	// zkontroluje platnost dat memory poolu
	ASSERT ( m_dwElementSize > 0 );

	// znièí nepotøebnou pamì (tj. volný blok)
	FreeExtra ();
	ASSERT ( m_pEmptyBlock == NULL );

#ifdef _DEBUG
// zkontroluje, je-li memory pool prázdný

	// ukazatel na blok
	SBlockHeader *pBlock = m_pFirstBlock;
	// poèet alokovaných prvkù
	DWORD dwElementCount = 0;
	// pøíznak nalezení prvního volného bloku
	BOOL bFirstFreeBlockFound = ( m_pFirstFreeBlock == NULL );

	// spoèítá poèet alokovaných prvkù
	while ( pBlock != NULL )
	{
		ASSERT ( ( pBlock->dwElementCount > 0 ) && ( pBlock->dwElementCount <= m_dwBlockSize ) );

	// zkontroluje spoják volných prvkù bloku

		// ukazatel na volný prvek bloku
		BYTE *pElement = pBlock->pFirstFreeElement;
		// projede spoják volných prvkù bloku
		for ( DWORD dwIndex = m_dwBlockSize - pBlock->dwElementCount; dwIndex-- > 0; )
		{
			// zjistí, je-li to skuteènì prvek bloku
			if ( ( pElement == NULL ) || ( pElement < (BYTE *)(pBlock + 1) ) || 
				( pElement >= ((BYTE *)(pBlock + 1)) + m_dwBlockSize * m_dwElementSize ) || 
				( (pElement - (BYTE *)(pBlock + 1)) % m_dwElementSize != 0 ) )
			{	// spoják pokraèuje na chybné adrese
				ASSERT ( FALSE );
			}
			// vezme další prvek spojáku
			pElement = *(BYTE **)pElement;
		}
		// zkontroluje ukonèení spojáku
		if ( pElement != NULL )
		{	// spoják není ukonèen
			ASSERT ( FALSE );
		}

	// zpracuje neprázdný blok prvkù

		// zjistí, jedná-li se o první volný blok
		if ( pBlock == m_pFirstFreeBlock )
		{	// jedná se o první volný blok
			bFirstFreeBlockFound = TRUE;
		}
		// pøiète alokované prvky bloku
		dwElementCount += pBlock->dwElementCount;
		// pøejde na další blok
		pBlock = pBlock->pNextBlock;
	}

	// zkontroluje, je-li ukazatel na první volný blok platný
	ASSERT ( bFirstFreeBlockFound );

	// zjistí, jsou-li alokovány nìjaké prvky
	if ( dwElementCount != 0 )
	{	// jsou alokovány nìjaké prvky
		TRACE2 ( "Memory leaks detected in memory pool: %d elements allocated of size %d\n", dwElementCount, m_dwElementSize );
		// index dumpovaného prvku
		DWORD dwElementIndex = 0;

	// vydumpuje alokované prvky

		// projede spoják blokù
		for ( pBlock = m_pFirstBlock; pBlock != NULL; pBlock = pBlock->pNextBlock )
		{
			// ukazatel na prvek bloku
			BYTE *pElement = (BYTE *)(pBlock + 1);
			// projede prvky bloku
			for ( DWORD dwIndex = m_dwBlockSize; dwIndex-- > 0; 
				pElement += m_dwElementSize )
			{	// zjistí, je-li prvek bloku volný
				// pøíznak, je-li prvek volný
				BOOL bIsFree = FALSE;
				// projede spoják volných prvkù
				for ( BYTE *pFreeElement = pBlock->pFirstFreeElement; pFreeElement != NULL; 
					pFreeElement = *(BYTE **)pFreeElement )
				{
					// zjistí, jedná-li se o hledaný prvek bloku
					if ( pFreeElement == pElement )
					{	// jedná se o hledaný prvek bloku
						bIsFree = TRUE;
						break;
					}
				}
				// zjistí, je-li prvek bloku volný
				if ( !bIsFree )
				{	// prvek bloku je alokovaný
					// vydumpuje prvek bloku
					TRACE1_NEXT ( "   Dumping element #%d of memory pool: ", 
						++dwElementIndex );
					for ( int i = 0; i < (int)m_dwElementSize; i++ )
					{
						TRACE1_NEXT ( " %02X", (int)pElement[i] );
					}
					TRACE0_NEXT ( "\n" );
				}
			}
		}
	}
#endif //_DEBUG

	// zneškodní data memory poolu
	m_dwElementSize = 0;
	m_pFirstBlock = NULL;
	m_pFirstFreeBlock = NULL;
}

// znièí nepotøebnou pamì
void CMemoryPool::FreeExtra () 
{
	ASSERT ( m_dwElementSize > 0 );
	ASSERT ( m_dwBlockSize > 0 );

	// zjistí, je-li alokován prázdný blok
	if ( m_pEmptyBlock != NULL )
	{	// je alokován prázdný blok
		ASSERT ( m_pEmptyBlock->dwElementCount == 0 );
		ASSERT ( m_pEmptyBlock->pNextBlock == NULL );
#ifdef _DEBUG
	// zkontroluje, je-li pole prvkù skuteènì prázdné

		// ukazatel na volný prvek bloku
		BYTE *pElement = m_pEmptyBlock->pFirstFreeElement;
		// projede spoják volných prvkù bloku
		for ( DWORD dwIndex = m_dwBlockSize; dwIndex-- > 0; )
		{
			// zjistí, je-li to skuteènì prvek bloku
			if ( ( pElement == NULL ) || ( pElement < (BYTE *)(m_pEmptyBlock + 1) ) || 
				( pElement >= ((BYTE *)(m_pEmptyBlock + 1)) + m_dwBlockSize * m_dwElementSize ) || 
				( (pElement - (BYTE *)(m_pEmptyBlock + 1)) % m_dwElementSize != 0 ) )
			{	// spoják pokraèuje na chybné adrese
				ASSERT ( FALSE );
			}
			// vezme další prvek spojáku
			pElement = *(BYTE **)pElement;
		}
		// zkontroluje ukonèení spojáku
		if ( pElement != NULL )
		{	// spoják není ukonèen
			ASSERT ( FALSE );
		}
#endif //_DEBUG

		// znièí alokovanou pamì
		delete [](BYTE *)m_pEmptyBlock;
		m_pEmptyBlock = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Operace s prvky
//////////////////////////////////////////////////////////////////////

// alokuje nový prvek
//		výjimky: CMemoryException
void *CMemoryPool::Allocate () 
{
	// zkontroluje platnost dat memory poolu
	ASSERT ( m_dwElementSize > 0 );
	ASSERT ( m_dwBlockSize > 0 );

	// zjistí, je-li nìjaký blok volný
	if ( m_pFirstFreeBlock == NULL )
	{	// žádný blok není volný
		// zjistí, je-li nìjaký blok úplnì prázdný
		if ( m_pEmptyBlock == NULL )
		{	// žádný blok není úplnì prázdný
			// alokuje nový blok
			m_pFirstFreeBlock = (SBlockHeader *)new BYTE[sizeof(SBlockHeader) + 
				m_dwBlockSize * m_dwElementSize];
			// inicializuje novì alokovaný blok
			m_pFirstFreeBlock->pNextBlock = m_pFirstBlock;
			m_pFirstBlock = m_pFirstFreeBlock;
			m_pFirstFreeBlock->dwElementCount = 0;

			// ukazatel na prvek v poli prvkù bloku
			BYTE *pElement = (BYTE *)(m_pFirstFreeBlock + 1);
			// inicializuje ukazatel na první volný prvek
			m_pFirstFreeBlock->pFirstFreeElement = pElement;

			// inicializuje seznam nepoužitých prvkù novì alokovaného bloku
			for ( DWORD dwIndex = m_dwBlockSize; dwIndex-- > 0; )
			{
				*(BYTE **)pElement = pElement + m_dwElementSize;
				pElement += m_dwElementSize;
			}
			// ukonèí inicializaci seznamu
			*(BYTE **)(pElement - m_dwElementSize) = NULL;

			// máme pøipravený novì alokovaný volný blok
		}
		else
		{	// máme úplnì prázdný blok
			ASSERT ( m_pEmptyBlock->pNextBlock == NULL );
			// zmìní prázdný blok na volný blok
			m_pEmptyBlock->pNextBlock = m_pFirstBlock;
			m_pFirstBlock = m_pFirstFreeBlock = m_pEmptyBlock;
			m_pEmptyBlock = NULL;
		}
	}
	// máme volný blok
	ASSERT ( m_pFirstFreeBlock->dwElementCount < m_dwBlockSize );

	// schová si volný prvek
	BYTE *pElement = m_pFirstFreeBlock->pFirstFreeElement;
	// aktualizuje poèet použitých prvkù
	++m_pFirstFreeBlock->dwElementCount;
	// aktualizuje první volný prvek
	if ( ( m_pFirstFreeBlock->pFirstFreeElement = *(BYTE **)pElement ) == NULL )
	{	// jedná se o poslední volný prvek bloku
		// pokusí se najít volný blok
		do
		{
			// pøejde na další blok
			m_pFirstFreeBlock = m_pFirstFreeBlock->pNextBlock;
			// zjistí, je-li blok volný (a není-li to již poslední blok)
		} while ( ( m_pFirstFreeBlock != NULL ) && 
			( m_pFirstFreeBlock->dwElementCount == m_dwBlockSize ) );
	}

	// vrátí prvek
	return (void *)pElement;
}

// dealokuje prvek
void CMemoryPool::Free ( void *pElement ) 
{
	// zkontroluje platnost dat memory poolu
	ASSERT ( m_dwElementSize > 0 );
	ASSERT ( m_dwBlockSize > 0 );

	// zkontroluje parametry
	ASSERT ( pElement != NULL );

// pomocné promìnné

	// ukazatel na odkaz na blok
	SBlockHeader **pBlock = &m_pFirstBlock;

	// najde blok, ve kterém je prvek alokován
	while ( *pBlock != NULL )
	{
		// zjistí, je-li prvek z tohoto bloku
		if ( ( (BYTE *)(*pBlock) <= pElement ) && 
			( ((BYTE *)((*pBlock) + 1)) + m_dwBlockSize * m_dwElementSize > pElement ) )
		{	// prvek je z tohoto bloku
			// zkontroluje, dealokuje-li se platný ukazatel
			ASSERT ( (*pBlock)->dwElementCount > 0 );
			ASSERT ( (BYTE *)((*pBlock) + 1) <= pElement );
			ASSERT ( ( (BYTE *)pElement - (BYTE *)((*pBlock) + 1) ) % m_dwElementSize == 0 );
#ifdef _DEBUG
		// zkontroluje, není-li dealokovaný prvek již dealokovaný

			// pomocná promìnná pro prùjezd spojáku volných prvkù
			BYTE *pFreeElement = (*pBlock)->pFirstFreeElement;

			// projede spoják volných prvkù
			while ( pFreeElement != NULL )
			{
				// zjistí, je-li to dealokovaný prvek
				if ( pFreeElement == pElement )
				{	// dealokovaný prvek je volný
					TRACE1 ( "Error deallocating element in the memory pool - element already deallocated! (element size %d)\n", m_dwElementSize );
					ASSERT ( FALSE );
					return;
				}
				pFreeElement = *(BYTE **)pFreeElement;
			}

			// inicializuje dealokovaný prvek
			memset ( (void *)pElement, 0xfd, m_dwElementSize );
#endif //_DEBUG
			// pøidá prvek do seznamu volných prvkù
			*(BYTE **)pElement = (*pBlock)->pFirstFreeElement;
			(*pBlock)->pFirstFreeElement = (BYTE *)pElement;

			// aktualizuje poèet použitých prvkù bloku
			if ( --(*pBlock)->dwElementCount == 0 )
			{	// blok je prázdný
				// získá ukazatel na prázdný blok
				SBlockHeader *pEmptyBlock = *pBlock;

				// zjistí, je-li to také první volný blok
				if ( pEmptyBlock == m_pFirstFreeBlock )
				{	// jedná se o první volný blok
					// pokusí se najít volný blok
					do
					{
						// pøejde na další blok
						m_pFirstFreeBlock = m_pFirstFreeBlock->pNextBlock;
						// zjistí, je-li blok volný (a není-li to již poslední blok)
					} while ( ( m_pFirstFreeBlock != NULL ) && 
						( m_pFirstFreeBlock->dwElementCount == m_dwBlockSize ) );
				}

				// vyjme blok ze seznamu používaných blokù
				*pBlock = pEmptyBlock->pNextBlock;
				// aktualizuje ukazatel na další blok
				pEmptyBlock->pNextBlock = NULL;

				// zjistí, je-li to první prázdný blok
				if ( m_pEmptyBlock == NULL )
				{	// jedná se o první prázdný blok
					m_pEmptyBlock = pEmptyBlock;
				}
				else
				{	// jedná se o další prázdný blok
					// znièí prázdný blok
					delete [] (BYTE *)pEmptyBlock;
				}
			}
			// ukonèí dealokaci prvku
			return;
		}
		// pøejde na další blok
		pBlock = &(*pBlock)->pNextBlock;
	}

	// nenašel alokovaný prvek
	TRACE1 ( "Error deallocating element in the memory pool - element was not allocated here! (element size %d)\n", m_dwElementSize );
	ASSERT ( FALSE );
}

// dealokuje všechny prvky
void CMemoryPool::FreeAll () 
{
	// ukazatel na nièený blok
	struct SBlockHeader *pBlock = m_pFirstBlock;

	// zjistí, jedná-li se o první prázdný blok
	if ( ( pBlock != NULL ) && ( m_pEmptyBlock == NULL ) )
	{	// jedná se o první prázdný blok
		// aktualizuje ukazatel na prázdný blok
		m_pEmptyBlock = pBlock;

		// aktualizuje ukazatel na další nièený blok
		pBlock = pBlock->pNextBlock;

		// aktualizuje prázdný blok
		m_pEmptyBlock->pNextBlock = NULL;
		m_pEmptyBlock->dwElementCount = 0;

		// ukazatel na prvek v poli prvkù bloku
		BYTE *pElement = (BYTE *)(m_pEmptyBlock + 1);
		// inicializuje ukazatel na první volný prvek
		m_pEmptyBlock->pFirstFreeElement = pElement;

		// inicializuje seznam nepoužitých prvkù novì alokovaného bloku
		for ( DWORD dwIndex = m_dwBlockSize; dwIndex-- > 0; )
		{
			*(BYTE **)pElement = pElement + m_dwElementSize;
			pElement += m_dwElementSize;
		}
		// ukonèí inicializaci seznamu
		*(BYTE **)(pElement - m_dwElementSize) = NULL;
	}

	// aktualizuje ukazatel na první prázdný blok
	m_pFirstFreeBlock = NULL;
	// aktualizuje ukazatel na první blok
	m_pFirstBlock = NULL;

	// znièí všechny bloky
	while ( pBlock != NULL )
	{
		// ukazatel na další nièený blok
		struct SBlockHeader *pNextBlock = pBlock->pNextBlock;

		// znièí blok
		delete [] (BYTE *)pBlock;

		// aktualizuje ukazatel na další nièený blok
		pBlock = pNextBlock;
	}
}
