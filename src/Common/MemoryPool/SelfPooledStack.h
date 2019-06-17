/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Abstraktn� datov� typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: �ablona z�sobn�ku s vlastn�m memory poolem
 * 
 ***********************************************************/

#ifndef __SELF_POOLED_STACK__HEADER_INCLUDED__
#define __SELF_POOLED_STACK__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// �ablona t��dy z�sobn�ku s vlastn�m memory poolem
template <class Element> 
class CSelfPooledStack 
{
// Datov� typy
private:
	// hlavi�ka bloku dat
	struct SBlockHeader 
	{
		// po�et pou�it�ch polo�ek v bloku dat
		DWORD dwElementCount;
		// ukazatel na dal�� blok dat
		struct SBlockHeader *pNext;
	};

// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	inline CSelfPooledStack ( DWORD dwBlockSize = 0 );
	// destruktor
	inline ~CSelfPooledStack ();

	// nastav� velikost bloku dat (lze volat jen jednou, pokud byl konstruktor vol�n 
	//		s parametrem 0)
	inline void SetBlockSize ( DWORD dwBlockSize );

// Operace se z�sobn�kem

	// zjist�, je-li z�sobn�k pr�zdn�
	inline BOOL IsEmpty () const;
	// p�id� prvek "cElement" na z�sobn�k
	inline void Push ( Element cElement );
	// p�id� prvek "pElement" na z�sobn�k
	inline void Push ( Element *pElement );
	// odebere prvek "cElement" ze z�sobn�ku (FALSE=z�sobn�k je pr�zdn�)
	inline BOOL Pop ( Element &cElement );
	// odebere prvek "pElement" ze z�sobn�ku (FALSE=z�sobn�k je pr�zdn�)
	inline BOOL Pop ( Element *pElement );

	// odebere v�echny prvky z�sobn�ku
	void FreeAll ();

// Data
protected:
	// prvn� blok seznamu blok�
	struct SBlockHeader *m_pFirstBlock;
	// pr�zdn� blok
	struct SBlockHeader *m_pFreeBlock;
	// velikost bloku dat
	DWORD m_dwBlockSize;
};

//////////////////////////////////////////////////////////////////////
// Metody �ablony CSelfPooledStack

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
template <class Element> 
inline CSelfPooledStack<Element>::CSelfPooledStack ( DWORD dwBlockSize ) 
{
	// uschov� si velikost bloku dat
	m_dwBlockSize = dwBlockSize;

	// zne�kodn� data objektu
	m_pFirstBlock = NULL;
	m_pFreeBlock = NULL;
}

// destruktor
template <class Element> 
inline CSelfPooledStack<Element>::~CSelfPooledStack () 
{
	ASSERT ( m_pFirstBlock == NULL );
	// zjist�, je-li naalokovan� voln� blok dat
	if ( m_pFreeBlock != NULL )
	{	// voln� blok dat je naalokovan�
		// zni�� naalokovan� voln� blok dat
		delete [] (BYTE *)m_pFreeBlock;
	}
}

// nastav� velikost bloku dat (lze volat jen jednou, pokud byl konstruktor vol�n 
//		s parametrem 0)
template <class Element> 
inline void CSelfPooledStack<Element>::SetBlockSize ( DWORD dwBlockSize ) 
{
	ASSERT ( m_dwBlockSize == 0 );
	ASSERT ( dwBlockSize > 0 );

	// uschov� si velikost bloku dat
	m_dwBlockSize = dwBlockSize;
}

//////////////////////////////////////////////////////////////////////
// Operace se z�sobn�kem
//////////////////////////////////////////////////////////////////////

// zjist�, je-li z�sobn�k pr�zdn�
template <class Element> 
inline BOOL CSelfPooledStack<Element>::IsEmpty () const 
{
	ASSERT ( m_dwBlockSize > 0 );
	ASSERT ( ( m_pFirstBlock == NULL ) || ( m_pFirstBlock->dwElementCount != 0 ) );

	return ( m_pFirstBlock == NULL );
}

// p�id� prvek "cElement" na z�sobn�k
template <class Element> 
inline void CSelfPooledStack<Element>::Push ( Element cElement ) 
{
	ASSERT ( m_dwBlockSize > 0 );
	ASSERT ( ( m_pFirstBlock == NULL ) || ( m_pFirstBlock->dwElementCount != 0 ) );

	// zjist�, je-li pot�eba naalokovat nov� blok dat
	if ( ( m_pFirstBlock == NULL ) || ( m_pFirstBlock->dwElementCount == m_dwBlockSize ) )
	{	// je pot�eba naalokovat nov� blok dat
		// zjist�, je-li k dispozici voln� blok dat
		if ( m_pFreeBlock != NULL )
		{	// voln� blok dat je k dispozici
			ASSERT ( m_pFreeBlock->dwElementCount == 0 );
			// zapoj� voln� blok do seznamu blok�
			m_pFreeBlock->pNext = m_pFirstBlock;
			m_pFirstBlock = m_pFreeBlock;
			// zne�kodn� voln� blok
			m_pFreeBlock = NULL;
		}
		else
		{
			// naalokuje nov� blok dat
			struct SBlockHeader *pBlock = 
				(struct SBlockHeader *)new BYTE[sizeof ( SBlockHeader ) + m_dwBlockSize * 
				sizeof ( Element )];
			// inicializuje nov� blok dat
			pBlock->dwElementCount = 0;
			// zapoj� nov� blok do seznamu blok�
			pBlock->pNext = m_pFirstBlock;
			m_pFirstBlock = pBlock;
		}
	}
	// nen� pot�eba alokovat nov� blok dat

	// p�id� prvek na z�sobn�k
	((Element *)( m_pFirstBlock + 1 ))[m_pFirstBlock->dwElementCount++] = cElement;
}

// p�id� prvek "pElement" na z�sobn�k
template <class Element> 
inline void CSelfPooledStack<Element>::Push ( Element *pElement ) 
{
	ASSERT ( m_dwBlockSize > 0 );
	ASSERT ( ( m_pFirstBlock == NULL ) || ( m_pFirstBlock->dwElementCount != 0 ) );

	// zjist�, je-li pot�eba naalokovat nov� blok dat
	if ( ( m_pFirstBlock == NULL ) || ( m_pFirstBlock->dwElementCount == m_dwElementCount ) )
	{	// je pot�eba naalokovat nov� blok dat
		// zjist�, je-li k dispozici voln� blok dat
		if ( m_pFreeBlock != NULL )
		{	// voln� blok dat je k dispozici
			ASSERT ( m_pFreeBlock->dwElementCount == 0 );
			// zapoj� voln� blok do seznamu blok�
			m_pFreeBlock->pNext = m_pFirstBlock;
			m_pFirstBlock = m_pFreeBlock;
			// zne�kodn� voln� blok
			m_pFreeBlock = NULL;
		}
		else
		{
			// naalokuje nov� blok dat
			struct SBlockHeader *pBlock = 
				(struct SBlockHeader *)new BYTE[sizeof ( SBLockHeader ) + m_dwBlockSize * 
				sizeof ( Element )];
			// inicializuje nov� blok dat
			pBlock->dwElementCount = 0;
			// zapoj� nov� blok do seznamu blok�
			pBlock->pNext = m_pFirstBlock;
			m_pFirstBlock = pBlock;
		}
	}
	// nen� pot�eba alokovat nov� blok dat

	// p�id� prvek na z�sobn�k
	((Element *)( m_pFirstBlock + 1 ))[m_pFirstBlock->dwElementCount++] = *pElement;
}

// odebere prvek "cElement" ze z�sobn�ku (FALSE=z�sobn�k je pr�zdn�)
template <class Element> 
inline BOOL CSelfPooledStack<Element>::Pop ( Element &cElement ) 
{
	ASSERT ( m_dwBlockSize > 0 );
	ASSERT ( ( m_pFirstBlock == NULL ) || ( m_pFirstBlock->dwElementCount != 0 ) );

	// zjist�, je-li z�sobn�k pr�zdn�
	if ( m_pFirstBlock == NULL )
	{	// z�sobn�k je pr�zdn�
		// vr�t� p��znak pr�zdn�ho z�sobn�ku
		return FALSE;
	}
	// z�sobn�k nen� pr�zdn�

	// odebere prvek ze z�sobn�ku
	cElement = ((Element *)( m_pFirstBlock + 1 ))[--m_pFirstBlock->dwElementCount];

	// zkontroluje, je-li prvn� blok dat pr�zdn�
	if ( m_pFirstBlock->dwElementCount == 0 )
	{	// prvn� blok dat je pr�zdn�
		// vyjme prvn� blok dat ze seznamu blok� dat
		struct SBlockHeader *pBlock = m_pFirstBlock;
		m_pFirstBlock = pBlock->pNext;
		// zjist�, existuje-li voln� blok dat
		if ( m_pFreeBlock == NULL )
		{	// neexistuje voln� blok dat
			// p�esune blok dat do voln�ho bloku dat
			m_pFreeBlock = pBlock;
		}
		else
		{	// existuje voln� blok dat
			// zni�� blok dat
			delete [] (BYTE *)pBlock;
		}
	}
	return TRUE;
}

// odebere prvek "pElement" ze z�sobn�ku (FALSE=z�sobn�k je pr�zdn�)
template <class Element> 
inline BOOL CSelfPooledStack<Element>::Pop ( Element *pElement ) 
{
	ASSERT ( m_dwBlockSize > 0 );
	ASSERT ( ( m_pFirstBlock == NULL ) || ( m_pFirstBlock->dwElementCount != 0 ) );

	// zjist�, je-li z�sobn�k pr�zdn�
	if ( m_pFirstBlock == NULL )
	{	// z�sobn�k je pr�zdn�
		// vr�t� p��znak pr�zdn�ho z�sobn�ku
		return FALSE;
	}
	// z�sobn�k nen� pr�zdn�

	// odebere prvek ze z�sobn�ku
	*pElement = ((Element *)( m_pFirstBlock + 1 ))[--m_pFirstBlock->dwElementCount];

	// zkontroluje, je-li prvn� blok dat pr�zdn�
	if ( m_pFirstBlock->dwElementCount == 0 )
	{	// prvn� blok dat je pr�zdn�
		// vyjme prvn� blok dat ze seznamu blok� dat
		struct SBlockHeader *pBlock = m_pFirstBlock;
		m_pFirstBlock = pBlock->pNext;
		// zjist�, existuje-li voln� blok dat
		if ( m_pFreeBlock == NULL )
		{	// neexistuje voln� blok dat
			// p�esune blok dat do voln�ho bloku dat
			m_pFreeBlock = pBlock;
		}
		else
		{	// existuje voln� blok dat
			// zni�� blok dat
			delete [] (BYTE *)pBlock;
		}
	}
}

// odebere v�echny prvky z�sobn�ku
template <class Element> 
void CSelfPooledStack<Element>::FreeAll () 
{
	ASSERT ( m_dwBlockSize > 0 );

	// ukazatel na ni�en� blok dat
	struct SBlockHeader *pBlock = m_pFirstBlock;

	// zjist�, jedn�-li se o prvn� voln� blok dat
	if ( ( pBlock != NULL ) && ( m_pFreeBlock == NULL ) )
	{	// jedn� se o prvn� voln� blok dat
		// p�esune blok dat do voln�ho bloku dat
		m_pFreeBlock = pBlock;

		// aktualizuje ukazatel na ni�en� blok data
		pBlock = pBlock->pNext;

		// aktualizuje voln� blok dat
		m_pFreeBlock->dwElementCount = 0;
	}

	// aktualizuje ukazatel na prvn� blok dat
	m_pFirstBlock = NULL;

	// zni�� bloky dat
	while ( pBlock != NULL )
	{
		// ukazatel na dal�� ni�en� blok dat
		struct SBlockHeader *pNextBlock = pBlock->pNext;

		// zni�� blok dat
		delete [] (BYTE *)pBlock;

		// aktualizuje ukazatel na dal�� ni�en� blok dat
		pBlock = pNextBlock;
	}
}

#endif //__SELF_POOLED_STACK__HEADER_INCLUDED__
