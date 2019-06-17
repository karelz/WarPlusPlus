/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Abstraktní datové typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Šablona zásobníku s vlastním memory poolem
 * 
 ***********************************************************/

#ifndef __SELF_POOLED_STACK__HEADER_INCLUDED__
#define __SELF_POOLED_STACK__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Šablona tøídy zásobníku s vlastním memory poolem
template <class Element> 
class CSelfPooledStack 
{
// Datové typy
private:
	// hlavièka bloku dat
	struct SBlockHeader 
	{
		// poèet použitých položek v bloku dat
		DWORD dwElementCount;
		// ukazatel na další blok dat
		struct SBlockHeader *pNext;
	};

// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	inline CSelfPooledStack ( DWORD dwBlockSize = 0 );
	// destruktor
	inline ~CSelfPooledStack ();

	// nastaví velikost bloku dat (lze volat jen jednou, pokud byl konstruktor volán 
	//		s parametrem 0)
	inline void SetBlockSize ( DWORD dwBlockSize );

// Operace se zásobníkem

	// zjistí, je-li zásobník prázdný
	inline BOOL IsEmpty () const;
	// pøidá prvek "cElement" na zásobník
	inline void Push ( Element cElement );
	// pøidá prvek "pElement" na zásobník
	inline void Push ( Element *pElement );
	// odebere prvek "cElement" ze zásobníku (FALSE=zásobník je prázdný)
	inline BOOL Pop ( Element &cElement );
	// odebere prvek "pElement" ze zásobníku (FALSE=zásobník je prázdný)
	inline BOOL Pop ( Element *pElement );

	// odebere všechny prvky zásobníku
	void FreeAll ();

// Data
protected:
	// první blok seznamu blokù
	struct SBlockHeader *m_pFirstBlock;
	// prázdný blok
	struct SBlockHeader *m_pFreeBlock;
	// velikost bloku dat
	DWORD m_dwBlockSize;
};

//////////////////////////////////////////////////////////////////////
// Metody šablony CSelfPooledStack

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
template <class Element> 
inline CSelfPooledStack<Element>::CSelfPooledStack ( DWORD dwBlockSize ) 
{
	// uschová si velikost bloku dat
	m_dwBlockSize = dwBlockSize;

	// zneškodní data objektu
	m_pFirstBlock = NULL;
	m_pFreeBlock = NULL;
}

// destruktor
template <class Element> 
inline CSelfPooledStack<Element>::~CSelfPooledStack () 
{
	ASSERT ( m_pFirstBlock == NULL );
	// zjistí, je-li naalokovaný volný blok dat
	if ( m_pFreeBlock != NULL )
	{	// volný blok dat je naalokovaný
		// znièí naalokovaný volný blok dat
		delete [] (BYTE *)m_pFreeBlock;
	}
}

// nastaví velikost bloku dat (lze volat jen jednou, pokud byl konstruktor volán 
//		s parametrem 0)
template <class Element> 
inline void CSelfPooledStack<Element>::SetBlockSize ( DWORD dwBlockSize ) 
{
	ASSERT ( m_dwBlockSize == 0 );
	ASSERT ( dwBlockSize > 0 );

	// uschová si velikost bloku dat
	m_dwBlockSize = dwBlockSize;
}

//////////////////////////////////////////////////////////////////////
// Operace se zásobníkem
//////////////////////////////////////////////////////////////////////

// zjistí, je-li zásobník prázdný
template <class Element> 
inline BOOL CSelfPooledStack<Element>::IsEmpty () const 
{
	ASSERT ( m_dwBlockSize > 0 );
	ASSERT ( ( m_pFirstBlock == NULL ) || ( m_pFirstBlock->dwElementCount != 0 ) );

	return ( m_pFirstBlock == NULL );
}

// pøidá prvek "cElement" na zásobník
template <class Element> 
inline void CSelfPooledStack<Element>::Push ( Element cElement ) 
{
	ASSERT ( m_dwBlockSize > 0 );
	ASSERT ( ( m_pFirstBlock == NULL ) || ( m_pFirstBlock->dwElementCount != 0 ) );

	// zjistí, je-li potøeba naalokovat nový blok dat
	if ( ( m_pFirstBlock == NULL ) || ( m_pFirstBlock->dwElementCount == m_dwBlockSize ) )
	{	// je potøeba naalokovat nový blok dat
		// zjistí, je-li k dispozici volný blok dat
		if ( m_pFreeBlock != NULL )
		{	// volný blok dat je k dispozici
			ASSERT ( m_pFreeBlock->dwElementCount == 0 );
			// zapojí volný blok do seznamu blokù
			m_pFreeBlock->pNext = m_pFirstBlock;
			m_pFirstBlock = m_pFreeBlock;
			// zneškodní volný blok
			m_pFreeBlock = NULL;
		}
		else
		{
			// naalokuje nový blok dat
			struct SBlockHeader *pBlock = 
				(struct SBlockHeader *)new BYTE[sizeof ( SBlockHeader ) + m_dwBlockSize * 
				sizeof ( Element )];
			// inicializuje nový blok dat
			pBlock->dwElementCount = 0;
			// zapojí nový blok do seznamu blokù
			pBlock->pNext = m_pFirstBlock;
			m_pFirstBlock = pBlock;
		}
	}
	// není potøeba alokovat nový blok dat

	// pøidá prvek na zásobník
	((Element *)( m_pFirstBlock + 1 ))[m_pFirstBlock->dwElementCount++] = cElement;
}

// pøidá prvek "pElement" na zásobník
template <class Element> 
inline void CSelfPooledStack<Element>::Push ( Element *pElement ) 
{
	ASSERT ( m_dwBlockSize > 0 );
	ASSERT ( ( m_pFirstBlock == NULL ) || ( m_pFirstBlock->dwElementCount != 0 ) );

	// zjistí, je-li potøeba naalokovat nový blok dat
	if ( ( m_pFirstBlock == NULL ) || ( m_pFirstBlock->dwElementCount == m_dwElementCount ) )
	{	// je potøeba naalokovat nový blok dat
		// zjistí, je-li k dispozici volný blok dat
		if ( m_pFreeBlock != NULL )
		{	// volný blok dat je k dispozici
			ASSERT ( m_pFreeBlock->dwElementCount == 0 );
			// zapojí volný blok do seznamu blokù
			m_pFreeBlock->pNext = m_pFirstBlock;
			m_pFirstBlock = m_pFreeBlock;
			// zneškodní volný blok
			m_pFreeBlock = NULL;
		}
		else
		{
			// naalokuje nový blok dat
			struct SBlockHeader *pBlock = 
				(struct SBlockHeader *)new BYTE[sizeof ( SBLockHeader ) + m_dwBlockSize * 
				sizeof ( Element )];
			// inicializuje nový blok dat
			pBlock->dwElementCount = 0;
			// zapojí nový blok do seznamu blokù
			pBlock->pNext = m_pFirstBlock;
			m_pFirstBlock = pBlock;
		}
	}
	// není potøeba alokovat nový blok dat

	// pøidá prvek na zásobník
	((Element *)( m_pFirstBlock + 1 ))[m_pFirstBlock->dwElementCount++] = *pElement;
}

// odebere prvek "cElement" ze zásobníku (FALSE=zásobník je prázdný)
template <class Element> 
inline BOOL CSelfPooledStack<Element>::Pop ( Element &cElement ) 
{
	ASSERT ( m_dwBlockSize > 0 );
	ASSERT ( ( m_pFirstBlock == NULL ) || ( m_pFirstBlock->dwElementCount != 0 ) );

	// zjistí, je-li zásobník prázdný
	if ( m_pFirstBlock == NULL )
	{	// zásobník je prázdný
		// vrátí pøíznak prázdného zásobníku
		return FALSE;
	}
	// zásobník není prázdný

	// odebere prvek ze zásobníku
	cElement = ((Element *)( m_pFirstBlock + 1 ))[--m_pFirstBlock->dwElementCount];

	// zkontroluje, je-li první blok dat prázdný
	if ( m_pFirstBlock->dwElementCount == 0 )
	{	// první blok dat je prázdný
		// vyjme první blok dat ze seznamu blokù dat
		struct SBlockHeader *pBlock = m_pFirstBlock;
		m_pFirstBlock = pBlock->pNext;
		// zjistí, existuje-li volný blok dat
		if ( m_pFreeBlock == NULL )
		{	// neexistuje volný blok dat
			// pøesune blok dat do volného bloku dat
			m_pFreeBlock = pBlock;
		}
		else
		{	// existuje volný blok dat
			// znièí blok dat
			delete [] (BYTE *)pBlock;
		}
	}
	return TRUE;
}

// odebere prvek "pElement" ze zásobníku (FALSE=zásobník je prázdný)
template <class Element> 
inline BOOL CSelfPooledStack<Element>::Pop ( Element *pElement ) 
{
	ASSERT ( m_dwBlockSize > 0 );
	ASSERT ( ( m_pFirstBlock == NULL ) || ( m_pFirstBlock->dwElementCount != 0 ) );

	// zjistí, je-li zásobník prázdný
	if ( m_pFirstBlock == NULL )
	{	// zásobník je prázdný
		// vrátí pøíznak prázdného zásobníku
		return FALSE;
	}
	// zásobník není prázdný

	// odebere prvek ze zásobníku
	*pElement = ((Element *)( m_pFirstBlock + 1 ))[--m_pFirstBlock->dwElementCount];

	// zkontroluje, je-li první blok dat prázdný
	if ( m_pFirstBlock->dwElementCount == 0 )
	{	// první blok dat je prázdný
		// vyjme první blok dat ze seznamu blokù dat
		struct SBlockHeader *pBlock = m_pFirstBlock;
		m_pFirstBlock = pBlock->pNext;
		// zjistí, existuje-li volný blok dat
		if ( m_pFreeBlock == NULL )
		{	// neexistuje volný blok dat
			// pøesune blok dat do volného bloku dat
			m_pFreeBlock = pBlock;
		}
		else
		{	// existuje volný blok dat
			// znièí blok dat
			delete [] (BYTE *)pBlock;
		}
	}
}

// odebere všechny prvky zásobníku
template <class Element> 
void CSelfPooledStack<Element>::FreeAll () 
{
	ASSERT ( m_dwBlockSize > 0 );

	// ukazatel na nièený blok dat
	struct SBlockHeader *pBlock = m_pFirstBlock;

	// zjistí, jedná-li se o první volný blok dat
	if ( ( pBlock != NULL ) && ( m_pFreeBlock == NULL ) )
	{	// jedná se o první volný blok dat
		// pøesune blok dat do volného bloku dat
		m_pFreeBlock = pBlock;

		// aktualizuje ukazatel na nièený blok data
		pBlock = pBlock->pNext;

		// aktualizuje volný blok dat
		m_pFreeBlock->dwElementCount = 0;
	}

	// aktualizuje ukazatel na první blok dat
	m_pFirstBlock = NULL;

	// znièí bloky dat
	while ( pBlock != NULL )
	{
		// ukazatel na další nièený blok dat
		struct SBlockHeader *pNextBlock = pBlock->pNext;

		// znièí blok dat
		delete [] (BYTE *)pBlock;

		// aktualizuje ukazatel na další nièený blok dat
		pBlock = pNextBlock;
	}
}

#endif //__SELF_POOLED_STACK__HEADER_INCLUDED__
