/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Abstraktní datové typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Typovanı memory pool
 * 
 ***********************************************************/

#ifndef __TYPED_MEMORY_POOL__HEADER_INCLUDED__
#define __TYPED_MEMORY_POOL__HEADER_INCLUDED__

#include <memory.h>
#include "Common\AfxDebugPlus\AfxDebugPlus.h"

//////////////////////////////////////////////////////////////////////
// Šablona typovaného memory poolu pro data "Element".
template <class Element>
class CTypedMemoryPool 
{
// Datové typy
private:
	// hlavièka bloku dat
	struct SBlockHeader
	{
		// první volná poloka v bloku dat (NULL=blok je plnı)
		Element *pFirstFreeElement;
		// poèet pouitıch poloek v bloku dat
		DWORD dwElementCount;
		// ukazatel na další blok dat
		SBlockHeader *pNextBlock;
	};
// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CTypedMemoryPool ( DWORD dwBlockSize );
	// destruktor
	~CTypedMemoryPool ();

	// znièí nepotøebnou pamì
	void FreeExtra ();

// Operace s prvky

	// alokuje novı prvek
	//		vıjimky: CMemoryException
	Element *Allocate ();
	// dealokuje prvek
	void Free ( Element *pElement );
	// dealokuje všechny prvky
	void FreeAll ();

// Data
private:
	// ukazatel na první blok dat
	SBlockHeader *m_pFirstBlock;
	// ukazatel na první volnı blok dat (nezahrnuje úplnì prázdné bloky)
	SBlockHeader *m_pFirstFreeBlock;
	// ukazatel na prázdnı blok
	SBlockHeader *m_pEmptyBlock;
	// poèet datovıch poloek v bloku
	DWORD m_dwBlockSize;
};

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
template <class Element> 
inline CTypedMemoryPool<Element>::CTypedMemoryPool ( DWORD dwBlockSize ) 
{
	ASSERT ( dwBlockSize > 0 );
	ASSERT ( sizeof ( Element ) >= sizeof ( Element * ) );

	// inicializuje data memory poolu
	m_pFirstBlock = NULL;
	m_pFirstFreeBlock = NULL;
	m_pEmptyBlock = NULL;

	// inicializuje velikost bloku
	m_dwBlockSize = dwBlockSize;
}

// destruktor
template <class Element> 
CTypedMemoryPool<Element>::~CTypedMemoryPool () 
{
	// uvolní nepotøebnou pamì (tj. prázdnı blok)
	FreeExtra ();
	ASSERT ( m_pEmptyBlock == NULL );

#ifdef _DEBUG
// zkontroluje, je-li memory pool prázdnı

// pomocné promìnné

	// ukazatel na blok
	SBlockHeader *pBlock = m_pFirstBlock;
	// poèet alokovanıch prvkù
	DWORD dwElementCount = 0;
	// pøíznak nalezení prvního volného bloku
	BOOL bFirstFreeBlockFound = ( m_pFirstFreeBlock == NULL );

// zkontroluje, je-li memory pool prázdnı

	// spoèítá poèet alokovanıch prvkù
	while ( pBlock != NULL )
	{
		ASSERT ( ( pBlock->dwElementCount > 0 ) && ( pBlock->dwElementCount <= m_dwBlockSize ) );

	// zkontroluje spoják volnıch prvkù bloku

		// ukazatel na volnı prvek bloku
		Element *pElement = pBlock->pFirstFreeElement;
		// projede spoják volnıch prvkù bloku
		for ( DWORD dwIndex = m_dwBlockSize - pBlock->dwElementCount; dwIndex-- > 0; )
		{
			// zjistí, je-li to skuteènì prvek bloku
			if ( ( pElement == NULL ) || ( pElement < (Element *)(pBlock + 1) ) || 
				( pElement >= ((Element *)(pBlock + 1)) + m_dwBlockSize ) || 
				( ((char *)pElement - (char *)(pBlock + 1)) % sizeof ( Element ) != 0 ) )
			{	// spoják pokraèuje na chybné adrese
				ASSERT ( FALSE );
			}
			// vezme další prvek spojáku
			pElement = *(Element **)pElement;
		}
		// zkontroluje ukonèení spojáku
		if ( pElement != NULL )
		{	// spoják není ukonèen
			ASSERT ( FALSE );
		}

	// zpracuje neprázdnı blok prvkù

		// zjistí, jedná-li se o první volnı blok
		if ( pBlock == m_pFirstFreeBlock )
		{	// jedná se o první volnı blok
			bFirstFreeBlockFound = TRUE;
		}
		// pøiète alokované prvky bloku
		dwElementCount += pBlock->dwElementCount;
		// pøejde na další blok
		pBlock = pBlock->pNextBlock;
	}

	// zkontroluje, je-li ukazatel na první volnı blok platnı
	ASSERT ( bFirstFreeBlockFound );

	// zjistí, jsou-li alokovány nìjaké prvky
	if ( dwElementCount != 0 )
	{	// jsou alokovány nìjaké prvky
		TRACE2 ( "Memory leaks detected in memory pool: %d elements allocated of size %d\n", dwElementCount, sizeof ( Element ) );
		// index dumpovaného prvku
		DWORD dwElementIndex = 0;

	// vydumpuje alokované prvky

		// projede spoják blokù
		for ( pBlock = m_pFirstBlock; pBlock != NULL; pBlock = pBlock->pNextBlock )
		{
			// ukazatel na prvek bloku
			Element *pElement = (Element *)(pBlock + 1);
			// projede prvky bloku
			for ( DWORD dwIndex = m_dwBlockSize; dwIndex-- > 0; pElement++ )
			{	// zjistí, je-li prvek bloku volnı
				// pøíznak, je-li prvek volnı
				BOOL bIsFree = FALSE;
				// projede spoják volnıch prvkù
				for ( Element *pFreeElement = pBlock->pFirstFreeElement; pFreeElement != NULL; 
					pFreeElement = *(Element **)pFreeElement )
				{
					// zjistí, jedná-li se o hledanı prvek bloku
					if ( pFreeElement == pElement )
					{	// jedná se o hledanı prvek bloku
						bIsFree = TRUE;
						break;
					}
				}
				// zjistí, je-li prvek bloku volnı
				if ( !bIsFree )
				{	// prvek bloku je alokovanı
					// vydumpuje prvek bloku
					TRACE1_NEXT ( "   Dumping element #%d of memory pool: ", ++dwElementIndex );
					for ( int i = 0; i < sizeof ( Element ); i++ )
					{
						TRACE1_NEXT ( " %02X", (int)((unsigned char*)pElement)[i] );
					}
					TRACE0_NEXT ( "\n" );
				}
			}
		}
		ASSERT ( FALSE );
	}
#endif //_DEBUG
}

template <class Element> 
// znièí nepotøebnou pamì
void CTypedMemoryPool<Element>::FreeExtra () 
{
	// zjistí, je-li alokován prázdnı blok
	if ( m_pEmptyBlock != NULL )
	{	// je alokován prázdnı blok
		ASSERT ( m_pEmptyBlock->dwElementCount == 0 );
		ASSERT ( m_pEmptyBlock->pNextBlock == NULL );
#ifdef _DEBUG
	// zkontroluje, je-li pole prvkù skuteènì prázdné

		// ukazatel na volnı prvek bloku
		Element *pElement = m_pEmptyBlock->pFirstFreeElement;
		// projede spoják volnıch prvkù bloku
		for ( DWORD dwIndex = m_dwBlockSize; dwIndex-- > 0; )
		{
			// zjistí, je-li to skuteènì prvek bloku
			if ( ( pElement == NULL ) || ( pElement < (Element *)(m_pEmptyBlock + 1) ) || 
				( pElement >= ((Element *)(m_pEmptyBlock + 1)) + m_dwBlockSize ) || 
				( ((char *)pElement - (char *)(m_pEmptyBlock + 1)) % sizeof ( Element ) != 0 ) )
			{	// spoják pokraèuje na chybné adrese
				ASSERT ( FALSE );
			}
			// vezme další prvek spojáku
			pElement = *(Element **)pElement;
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

// alokuje novı prvek
//		vıjimky: CMemoryException
template <class Element> 
Element *CTypedMemoryPool<Element>::Allocate () 
{
	// zjistí, je-li nìjakı blok volnı
	if ( m_pFirstFreeBlock == NULL )
	{	// ádnı blok není volnı
		// zjistí, je-li nìjakı blok úplnì prázdnı
		if ( m_pEmptyBlock == NULL )
		{	// ádnı blok není úplnì prázdnı
			// alokuje novı blok
			m_pFirstFreeBlock = (SBlockHeader *)new BYTE[sizeof ( SBlockHeader ) + 
				m_dwBlockSize * sizeof ( Element )];
			// inicializuje novì alokovanı blok
			m_pFirstFreeBlock->pNextBlock = m_pFirstBlock;
			m_pFirstBlock = m_pFirstFreeBlock;
			m_pFirstFreeBlock->dwElementCount = 0;

		// pomocné promìnné pro prùjezd polem prázdnıch prvkù

			// ukazatel na prvek v poli prvkù bloku
			Element *pElement = (Element *)(m_pFirstFreeBlock + 1);
			// inicializuje ukazatel na první volnı prvek
			m_pFirstFreeBlock->pFirstFreeElement = pElement;

			// inicializuje seznam nepouitıch prvkù novì alokovaného bloku
			for ( DWORD dwIndex = m_dwBlockSize; dwIndex-- > 0; )
			{
				*(Element **)pElement = pElement + 1;
				pElement++;
			}
			// ukonèí inicializaci seznamu
			*(Element **)(pElement - 1) = NULL;

			// máme pøipravenı novì alokovanı volnı blok
		}
		else
		{	// máme úplnì prázdnı blok
			ASSERT ( m_pEmptyBlock->pNextBlock == NULL );
			// zmìní prázdnı blok na volnı blok
			m_pEmptyBlock->pNextBlock = m_pFirstBlock;
			m_pFirstBlock = m_pFirstFreeBlock = m_pEmptyBlock;
			m_pEmptyBlock = NULL;
		}
	}
	// máme volnı blok
	ASSERT ( m_pFirstFreeBlock->dwElementCount < m_dwBlockSize );

	// schová si volnı prvek
	Element *pElement = m_pFirstFreeBlock->pFirstFreeElement;
	// aktualizuje poèet pouitıch prvkù
	++m_pFirstFreeBlock->dwElementCount;
	// aktualizuje první volnı prvek
	if ( ( m_pFirstFreeBlock->pFirstFreeElement = *(Element **)pElement ) == NULL )
	{	// jedná se o poslední volnı prvek bloku
		// pokusí se najít volnı blok
		do
		{
			// pøejde na další blok
			m_pFirstFreeBlock = m_pFirstFreeBlock->pNextBlock;
			// zjistí, je-li blok volnı (a není-li to ji poslední blok)
		} while ( ( m_pFirstFreeBlock != NULL ) && 
			( m_pFirstFreeBlock->dwElementCount == m_dwBlockSize ) );
	}

	// vrátí prvek
	return pElement;
}

// dealokuje prvek
template <class Element> 
void CTypedMemoryPool<Element>::Free ( Element *pElement ) 
{
	ASSERT ( pElement != NULL );

// pomocné promìnné

	// ukazatel na odkaz na blok
	SBlockHeader **pBlock = &m_pFirstBlock;

	// najde blok, ve kterém je prvek alokován
	while ( *pBlock != NULL )
	{
		// zjistí, je-li prvek z tohoto bloku
		if ( ( (Element *)(*pBlock) < pElement ) && 
			( ((Element *)((*pBlock) + 1)) + m_dwBlockSize > pElement ) )
		{	// prvek je z tohoto bloku
			// zkontroluje, dealokuje-li se platnı ukazatel
			ASSERT ( (*pBlock)->dwElementCount > 0 );
			ASSERT ( (BYTE *)((*pBlock) + 1) <= (BYTE *)pElement );
			ASSERT ( ( (BYTE *)pElement - (BYTE *)((*pBlock) + 1) ) % sizeof ( Element ) == 0 );
#ifdef _DEBUG
		// zkontroluje, není-li dealokovanı prvek volnı

			// pomocná promìnná pro prùjezd spojáku volnıch prvkù
			Element *pFreeElement = (*pBlock)->pFirstFreeElement;

			// projede spoják volnıch prvkù
			while ( pFreeElement != NULL )
			{
				// zjistí, je-li to dealokovanı prvek
				if ( pFreeElement == pElement )
				{	// dealokovanı prvek je volnı
					TRACE1 ( "Error deallocating element in the memory pool - element already deallocated! (element size %d)\n", sizeof ( Element ) );
					ASSERT ( FALSE );
					return;
				}
				pFreeElement = *(Element **)pFreeElement;
			}

			// inicializuje dealokovanı prvek
			memset ( (void *)pElement, 0xfd, sizeof ( Element ) );
#endif //_DEBUG
			// pøidá prvek do seznamu volnıch prvkù
			*(Element **)pElement = (*pBlock)->pFirstFreeElement;
			(*pBlock)->pFirstFreeElement = pElement;

			// aktualizuje poèet pouitıch prvkù bloku
			if ( --(*pBlock)->dwElementCount == 0 )
			{	// blok je prázdnı
				// získá ukazatel na prázdnı blok
				SBlockHeader *pEmptyBlock = *pBlock;

				// zjistí, je-li to také první volnı blok
				if ( pEmptyBlock == m_pFirstFreeBlock )
				{	// jedná se o první volnı blok
					// pokusí se najít volnı blok
					do
					{
						// pøejde na další blok
						m_pFirstFreeBlock = m_pFirstFreeBlock->pNextBlock;
						// zjistí, je-li blok volnı (a není-li to ji poslední blok)
					} while ( ( m_pFirstFreeBlock != NULL ) && 
						( m_pFirstFreeBlock->dwElementCount == m_dwBlockSize ) );
				}

				// vyjme blok ze seznamu pouívanıch blokù
				*pBlock = pEmptyBlock->pNextBlock;
				// aktualizuje ukazatel na další blok
				pEmptyBlock->pNextBlock = NULL;

				// zjistí, je-li to první prázdnı blok
				if ( m_pEmptyBlock == NULL )
				{	// jedná se o první prázdnı blok
					m_pEmptyBlock = pEmptyBlock;
				}
				else
				{	// jedná se o další prázdnı blok
					// znièí prázdnı blok
					delete [] (BYTE *)pEmptyBlock;
				}
			}
			// ukonèí dealokaci prvku
			return;
		}
		// pøejde na další blok
		pBlock = &(*pBlock)->pNextBlock;
	}

	// nenašel alokovanı prvek
	TRACE1 ( "Error deallocating element in the memory pool - element was not allocated here! (element size %d)\n", sizeof ( Element ) );
	ASSERT ( FALSE );
}

// dealokuje všechny prvky
template <class Element> 
void CTypedMemoryPool<Element>::FreeAll () 
{
	// ukazatel na nièenı blok
	struct SBlockHeader *pBlock = m_pFirstBlock;

	// zjistí, jedná-li se o první prázdnı blok
	if ( ( pBlock != NULL ) && ( m_pEmptyBlock == NULL ) )
	{	// jedná se o první prázdnı blok
		// aktualizuje ukazatel na prázdnı blok
		m_pEmptyBlock = pBlock;

		// aktualizuje ukazatel na další nièenı blok
		pBlock = pBlock->pNextBlock;

		// aktualizuje prázdnı blok
		m_pEmptyBlock->pNextBlock = NULL;
		m_pEmptyBlock->dwElementCount = 0;

		// ukazatel na prvek v poli prvkù bloku
		Element *pElement = (Element *)(m_pEmptyBlock + 1);
		// inicializuje ukazatel na první volnı prvek
		m_pEmptyBlock->pFirstFreeElement = pElement;

		// inicializuje seznam nepouitıch prvkù novì alokovaného bloku
		for ( DWORD dwIndex = m_dwBlockSize; dwIndex-- > 0; )
		{
			*(Element **)pElement = pElement + 1;
			pElement++;
		}
		// ukonèí inicializaci seznamu
		*(Element **)(pElement - 1) = NULL;
	}

	// aktualizuje ukazatel na první prázdnı blok
	m_pFirstFreeBlock = NULL;
	// aktualizuje ukazatel na první blok
	m_pFirstBlock = NULL;

	// znièí všechny bloky
	while ( pBlock != NULL )
	{
		// ukazatel na další nièenı blok
		struct SBlockHeader *pNextBlock = pBlock->pNextBlock;

		// znièí blok
		delete [] (BYTE *)pBlock;

		// aktualizuje ukazatel na další nièenı blok
		pBlock = pNextBlock;
	}
}

#endif //__TYPED_MEMORY_POOL__HEADER_INCLUDED__
