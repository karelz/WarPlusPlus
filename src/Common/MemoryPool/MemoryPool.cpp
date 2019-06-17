/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Abstraktn� datov� typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Memory pool
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "Common\AfxDebugPlus\AfxDebugPlus.h"
#include "MemoryPool.h"

//////////////////////////////////////////////////////////////////////
// Inicializace a zni�en� dat objektu
//////////////////////////////////////////////////////////////////////

// zni�en� memory poolu
void CMemoryPool::Delete () 
{
	ASSERT ( m_dwBlockSize > 0 );

	// zjist�, je-li memory pool platn�
	if ( m_dwElementSize == 0 )
	{	// memory pool nen� platn�
		// zkontroluje neplatnost memory poolu
		ASSERT ( m_pFirstBlock == NULL );
		ASSERT ( m_pFirstFreeBlock == NULL );
		ASSERT ( m_pEmptyBlock == NULL );
		ASSERT ( m_dwElementSize == 0 );
		// ukon�� ni�en� neplatn�ho memory poolu
		return;
	}

	// zkontroluje platnost dat memory poolu
	ASSERT ( m_dwElementSize > 0 );

	// zni�� nepot�ebnou pam� (tj. voln� blok)
	FreeExtra ();
	ASSERT ( m_pEmptyBlock == NULL );

#ifdef _DEBUG
// zkontroluje, je-li memory pool pr�zdn�

	// ukazatel na blok
	SBlockHeader *pBlock = m_pFirstBlock;
	// po�et alokovan�ch prvk�
	DWORD dwElementCount = 0;
	// p��znak nalezen� prvn�ho voln�ho bloku
	BOOL bFirstFreeBlockFound = ( m_pFirstFreeBlock == NULL );

	// spo��t� po�et alokovan�ch prvk�
	while ( pBlock != NULL )
	{
		ASSERT ( ( pBlock->dwElementCount > 0 ) && ( pBlock->dwElementCount <= m_dwBlockSize ) );

	// zkontroluje spoj�k voln�ch prvk� bloku

		// ukazatel na voln� prvek bloku
		BYTE *pElement = pBlock->pFirstFreeElement;
		// projede spoj�k voln�ch prvk� bloku
		for ( DWORD dwIndex = m_dwBlockSize - pBlock->dwElementCount; dwIndex-- > 0; )
		{
			// zjist�, je-li to skute�n� prvek bloku
			if ( ( pElement == NULL ) || ( pElement < (BYTE *)(pBlock + 1) ) || 
				( pElement >= ((BYTE *)(pBlock + 1)) + m_dwBlockSize * m_dwElementSize ) || 
				( (pElement - (BYTE *)(pBlock + 1)) % m_dwElementSize != 0 ) )
			{	// spoj�k pokra�uje na chybn� adrese
				ASSERT ( FALSE );
			}
			// vezme dal�� prvek spoj�ku
			pElement = *(BYTE **)pElement;
		}
		// zkontroluje ukon�en� spoj�ku
		if ( pElement != NULL )
		{	// spoj�k nen� ukon�en
			ASSERT ( FALSE );
		}

	// zpracuje nepr�zdn� blok prvk�

		// zjist�, jedn�-li se o prvn� voln� blok
		if ( pBlock == m_pFirstFreeBlock )
		{	// jedn� se o prvn� voln� blok
			bFirstFreeBlockFound = TRUE;
		}
		// p�i�te alokovan� prvky bloku
		dwElementCount += pBlock->dwElementCount;
		// p�ejde na dal�� blok
		pBlock = pBlock->pNextBlock;
	}

	// zkontroluje, je-li ukazatel na prvn� voln� blok platn�
	ASSERT ( bFirstFreeBlockFound );

	// zjist�, jsou-li alokov�ny n�jak� prvky
	if ( dwElementCount != 0 )
	{	// jsou alokov�ny n�jak� prvky
		TRACE2 ( "Memory leaks detected in memory pool: %d elements allocated of size %d\n", dwElementCount, m_dwElementSize );
		// index dumpovan�ho prvku
		DWORD dwElementIndex = 0;

	// vydumpuje alokovan� prvky

		// projede spoj�k blok�
		for ( pBlock = m_pFirstBlock; pBlock != NULL; pBlock = pBlock->pNextBlock )
		{
			// ukazatel na prvek bloku
			BYTE *pElement = (BYTE *)(pBlock + 1);
			// projede prvky bloku
			for ( DWORD dwIndex = m_dwBlockSize; dwIndex-- > 0; 
				pElement += m_dwElementSize )
			{	// zjist�, je-li prvek bloku voln�
				// p��znak, je-li prvek voln�
				BOOL bIsFree = FALSE;
				// projede spoj�k voln�ch prvk�
				for ( BYTE *pFreeElement = pBlock->pFirstFreeElement; pFreeElement != NULL; 
					pFreeElement = *(BYTE **)pFreeElement )
				{
					// zjist�, jedn�-li se o hledan� prvek bloku
					if ( pFreeElement == pElement )
					{	// jedn� se o hledan� prvek bloku
						bIsFree = TRUE;
						break;
					}
				}
				// zjist�, je-li prvek bloku voln�
				if ( !bIsFree )
				{	// prvek bloku je alokovan�
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

	// zne�kodn� data memory poolu
	m_dwElementSize = 0;
	m_pFirstBlock = NULL;
	m_pFirstFreeBlock = NULL;
}

// zni�� nepot�ebnou pam�
void CMemoryPool::FreeExtra () 
{
	ASSERT ( m_dwElementSize > 0 );
	ASSERT ( m_dwBlockSize > 0 );

	// zjist�, je-li alokov�n pr�zdn� blok
	if ( m_pEmptyBlock != NULL )
	{	// je alokov�n pr�zdn� blok
		ASSERT ( m_pEmptyBlock->dwElementCount == 0 );
		ASSERT ( m_pEmptyBlock->pNextBlock == NULL );
#ifdef _DEBUG
	// zkontroluje, je-li pole prvk� skute�n� pr�zdn�

		// ukazatel na voln� prvek bloku
		BYTE *pElement = m_pEmptyBlock->pFirstFreeElement;
		// projede spoj�k voln�ch prvk� bloku
		for ( DWORD dwIndex = m_dwBlockSize; dwIndex-- > 0; )
		{
			// zjist�, je-li to skute�n� prvek bloku
			if ( ( pElement == NULL ) || ( pElement < (BYTE *)(m_pEmptyBlock + 1) ) || 
				( pElement >= ((BYTE *)(m_pEmptyBlock + 1)) + m_dwBlockSize * m_dwElementSize ) || 
				( (pElement - (BYTE *)(m_pEmptyBlock + 1)) % m_dwElementSize != 0 ) )
			{	// spoj�k pokra�uje na chybn� adrese
				ASSERT ( FALSE );
			}
			// vezme dal�� prvek spoj�ku
			pElement = *(BYTE **)pElement;
		}
		// zkontroluje ukon�en� spoj�ku
		if ( pElement != NULL )
		{	// spoj�k nen� ukon�en
			ASSERT ( FALSE );
		}
#endif //_DEBUG

		// zni�� alokovanou pam�
		delete [](BYTE *)m_pEmptyBlock;
		m_pEmptyBlock = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Operace s prvky
//////////////////////////////////////////////////////////////////////

// alokuje nov� prvek
//		v�jimky: CMemoryException
void *CMemoryPool::Allocate () 
{
	// zkontroluje platnost dat memory poolu
	ASSERT ( m_dwElementSize > 0 );
	ASSERT ( m_dwBlockSize > 0 );

	// zjist�, je-li n�jak� blok voln�
	if ( m_pFirstFreeBlock == NULL )
	{	// ��dn� blok nen� voln�
		// zjist�, je-li n�jak� blok �pln� pr�zdn�
		if ( m_pEmptyBlock == NULL )
		{	// ��dn� blok nen� �pln� pr�zdn�
			// alokuje nov� blok
			m_pFirstFreeBlock = (SBlockHeader *)new BYTE[sizeof(SBlockHeader) + 
				m_dwBlockSize * m_dwElementSize];
			// inicializuje nov� alokovan� blok
			m_pFirstFreeBlock->pNextBlock = m_pFirstBlock;
			m_pFirstBlock = m_pFirstFreeBlock;
			m_pFirstFreeBlock->dwElementCount = 0;

			// ukazatel na prvek v poli prvk� bloku
			BYTE *pElement = (BYTE *)(m_pFirstFreeBlock + 1);
			// inicializuje ukazatel na prvn� voln� prvek
			m_pFirstFreeBlock->pFirstFreeElement = pElement;

			// inicializuje seznam nepou�it�ch prvk� nov� alokovan�ho bloku
			for ( DWORD dwIndex = m_dwBlockSize; dwIndex-- > 0; )
			{
				*(BYTE **)pElement = pElement + m_dwElementSize;
				pElement += m_dwElementSize;
			}
			// ukon�� inicializaci seznamu
			*(BYTE **)(pElement - m_dwElementSize) = NULL;

			// m�me p�ipraven� nov� alokovan� voln� blok
		}
		else
		{	// m�me �pln� pr�zdn� blok
			ASSERT ( m_pEmptyBlock->pNextBlock == NULL );
			// zm�n� pr�zdn� blok na voln� blok
			m_pEmptyBlock->pNextBlock = m_pFirstBlock;
			m_pFirstBlock = m_pFirstFreeBlock = m_pEmptyBlock;
			m_pEmptyBlock = NULL;
		}
	}
	// m�me voln� blok
	ASSERT ( m_pFirstFreeBlock->dwElementCount < m_dwBlockSize );

	// schov� si voln� prvek
	BYTE *pElement = m_pFirstFreeBlock->pFirstFreeElement;
	// aktualizuje po�et pou�it�ch prvk�
	++m_pFirstFreeBlock->dwElementCount;
	// aktualizuje prvn� voln� prvek
	if ( ( m_pFirstFreeBlock->pFirstFreeElement = *(BYTE **)pElement ) == NULL )
	{	// jedn� se o posledn� voln� prvek bloku
		// pokus� se naj�t voln� blok
		do
		{
			// p�ejde na dal�� blok
			m_pFirstFreeBlock = m_pFirstFreeBlock->pNextBlock;
			// zjist�, je-li blok voln� (a nen�-li to ji� posledn� blok)
		} while ( ( m_pFirstFreeBlock != NULL ) && 
			( m_pFirstFreeBlock->dwElementCount == m_dwBlockSize ) );
	}

	// vr�t� prvek
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

// pomocn� prom�nn�

	// ukazatel na odkaz na blok
	SBlockHeader **pBlock = &m_pFirstBlock;

	// najde blok, ve kter�m je prvek alokov�n
	while ( *pBlock != NULL )
	{
		// zjist�, je-li prvek z tohoto bloku
		if ( ( (BYTE *)(*pBlock) <= pElement ) && 
			( ((BYTE *)((*pBlock) + 1)) + m_dwBlockSize * m_dwElementSize > pElement ) )
		{	// prvek je z tohoto bloku
			// zkontroluje, dealokuje-li se platn� ukazatel
			ASSERT ( (*pBlock)->dwElementCount > 0 );
			ASSERT ( (BYTE *)((*pBlock) + 1) <= pElement );
			ASSERT ( ( (BYTE *)pElement - (BYTE *)((*pBlock) + 1) ) % m_dwElementSize == 0 );
#ifdef _DEBUG
		// zkontroluje, nen�-li dealokovan� prvek ji� dealokovan�

			// pomocn� prom�nn� pro pr�jezd spoj�ku voln�ch prvk�
			BYTE *pFreeElement = (*pBlock)->pFirstFreeElement;

			// projede spoj�k voln�ch prvk�
			while ( pFreeElement != NULL )
			{
				// zjist�, je-li to dealokovan� prvek
				if ( pFreeElement == pElement )
				{	// dealokovan� prvek je voln�
					TRACE1 ( "Error deallocating element in the memory pool - element already deallocated! (element size %d)\n", m_dwElementSize );
					ASSERT ( FALSE );
					return;
				}
				pFreeElement = *(BYTE **)pFreeElement;
			}

			// inicializuje dealokovan� prvek
			memset ( (void *)pElement, 0xfd, m_dwElementSize );
#endif //_DEBUG
			// p�id� prvek do seznamu voln�ch prvk�
			*(BYTE **)pElement = (*pBlock)->pFirstFreeElement;
			(*pBlock)->pFirstFreeElement = (BYTE *)pElement;

			// aktualizuje po�et pou�it�ch prvk� bloku
			if ( --(*pBlock)->dwElementCount == 0 )
			{	// blok je pr�zdn�
				// z�sk� ukazatel na pr�zdn� blok
				SBlockHeader *pEmptyBlock = *pBlock;

				// zjist�, je-li to tak� prvn� voln� blok
				if ( pEmptyBlock == m_pFirstFreeBlock )
				{	// jedn� se o prvn� voln� blok
					// pokus� se naj�t voln� blok
					do
					{
						// p�ejde na dal�� blok
						m_pFirstFreeBlock = m_pFirstFreeBlock->pNextBlock;
						// zjist�, je-li blok voln� (a nen�-li to ji� posledn� blok)
					} while ( ( m_pFirstFreeBlock != NULL ) && 
						( m_pFirstFreeBlock->dwElementCount == m_dwBlockSize ) );
				}

				// vyjme blok ze seznamu pou��van�ch blok�
				*pBlock = pEmptyBlock->pNextBlock;
				// aktualizuje ukazatel na dal�� blok
				pEmptyBlock->pNextBlock = NULL;

				// zjist�, je-li to prvn� pr�zdn� blok
				if ( m_pEmptyBlock == NULL )
				{	// jedn� se o prvn� pr�zdn� blok
					m_pEmptyBlock = pEmptyBlock;
				}
				else
				{	// jedn� se o dal�� pr�zdn� blok
					// zni�� pr�zdn� blok
					delete [] (BYTE *)pEmptyBlock;
				}
			}
			// ukon�� dealokaci prvku
			return;
		}
		// p�ejde na dal�� blok
		pBlock = &(*pBlock)->pNextBlock;
	}

	// nena�el alokovan� prvek
	TRACE1 ( "Error deallocating element in the memory pool - element was not allocated here! (element size %d)\n", m_dwElementSize );
	ASSERT ( FALSE );
}

// dealokuje v�echny prvky
void CMemoryPool::FreeAll () 
{
	// ukazatel na ni�en� blok
	struct SBlockHeader *pBlock = m_pFirstBlock;

	// zjist�, jedn�-li se o prvn� pr�zdn� blok
	if ( ( pBlock != NULL ) && ( m_pEmptyBlock == NULL ) )
	{	// jedn� se o prvn� pr�zdn� blok
		// aktualizuje ukazatel na pr�zdn� blok
		m_pEmptyBlock = pBlock;

		// aktualizuje ukazatel na dal�� ni�en� blok
		pBlock = pBlock->pNextBlock;

		// aktualizuje pr�zdn� blok
		m_pEmptyBlock->pNextBlock = NULL;
		m_pEmptyBlock->dwElementCount = 0;

		// ukazatel na prvek v poli prvk� bloku
		BYTE *pElement = (BYTE *)(m_pEmptyBlock + 1);
		// inicializuje ukazatel na prvn� voln� prvek
		m_pEmptyBlock->pFirstFreeElement = pElement;

		// inicializuje seznam nepou�it�ch prvk� nov� alokovan�ho bloku
		for ( DWORD dwIndex = m_dwBlockSize; dwIndex-- > 0; )
		{
			*(BYTE **)pElement = pElement + m_dwElementSize;
			pElement += m_dwElementSize;
		}
		// ukon�� inicializaci seznamu
		*(BYTE **)(pElement - m_dwElementSize) = NULL;
	}

	// aktualizuje ukazatel na prvn� pr�zdn� blok
	m_pFirstFreeBlock = NULL;
	// aktualizuje ukazatel na prvn� blok
	m_pFirstBlock = NULL;

	// zni�� v�echny bloky
	while ( pBlock != NULL )
	{
		// ukazatel na dal�� ni�en� blok
		struct SBlockHeader *pNextBlock = pBlock->pNextBlock;

		// zni�� blok
		delete [] (BYTE *)pBlock;

		// aktualizuje ukazatel na dal�� ni�en� blok
		pBlock = pNextBlock;
	}
}
