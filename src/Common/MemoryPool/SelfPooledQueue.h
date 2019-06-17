/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Abstraktní datové typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Šablona fronty s vlastním memory poolem
 * 
 ***********************************************************/

#ifndef __SELF_POOLED_QUEUE__HEADER_INCLUDED__
#define __SELF_POOLED_QUEUE__HEADER_INCLUDED__

#include "TypedMemoryPool.h"

//////////////////////////////////////////////////////////////////////
// Šablona tøídy fronty s vlastním memory poolem
template <class Element> 
class CSelfPooledQueue 
{
// Datové typy
private:
	// prvek fronty
	struct SQueueMember 
	{
		// ukazatel na další prvek fronty
		struct SQueueMember *pNext;
		// data prvku fronty
		Element cElement;
	};

// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	inline CSelfPooledQueue ( DWORD dwBlockSize );
	// destruktor
	inline ~CSelfPooledQueue ();

// Operace s frontou

	// zjistí, je-li fronta prázdná
	BOOL IsEmpty () const { return ( m_pFirst == NULL ); };
	// získá první prvek "cElement" z fronty (FALSE=fronta je prázdná)
	inline BOOL Get ( Element &cElement ) const;
	// vrátí pozici prvního prvku fronty
	POSITION GetHeadPosition () const { return (POSITION)&m_pFirst; };
	// vyplní "cElement" dalším prvkem fornty na pozici "rPosition" (FALSE=konec fronty)
	static inline BOOL GetNext ( POSITION &rPosition, Element &cElement );
	// vyplní "pElement" ukazatelem na další prvek fronty na pozici "rPosition" 
	//		(FALSE=konec seznamu)
	static inline BOOL GetNext ( POSITION &rPosition, Element *&pElement );
	// pøeskoèí prvek fronty na pozici "rPosition" (FALSE=konec seznamu)
	static inline BOOL GetNext ( POSITION &rPosition );
	// vrátí poèet prvkù fronty
	DWORD GetSize () const;

	// vyjme první prvek fronty a vrátí jeho hodnotu
	inline Element RemoveFirst ();
	// vyjme z fronty první prvek "cElement" (FALSE=fronta byla prázdná, prvek nebyl 
	//		vyjmut)
	inline BOOL RemoveFirst ( Element &cElement );
	// pøidá do fronty prvek "cElement"
	inline void Add ( Element cElement );
	// smaže z fronty prvek "cElement" (FALSE=prvek nebyl ve frontì nalezen)
	BOOL RemoveElement ( Element &cElement );
	// smaže prvek fronty "position" a vrátí jeho hodnotu ("position" je platná a 
	//		odkazuje na další prvek seznamu)
	Element RemovePosition ( POSITION position );
	// vyjme z fronty všechny prvky
	inline void RemoveAll ();

// Data
protected:
	// první prvek fronty
	struct SQueueMember *m_pFirst;
	// poslední prvek fronty
	struct SQueueMember *m_pLast;
	// memory pool prvkù fronty
	CTypedMemoryPool<struct SQueueMember> m_cPool;
};

//////////////////////////////////////////////////////////////////////
// Metody šablony CSelfPooledQueue

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
template <class Element> 
inline CSelfPooledQueue<Element>::CSelfPooledQueue ( DWORD dwBlockSize ) : 
	m_cPool ( dwBlockSize ) 
{
	// zneškodní data objektu
	m_pFirst = NULL;
	m_pLast = NULL;
}

// destruktor
template <class Element> 
inline CSelfPooledQueue<Element>::~CSelfPooledQueue () 
{
	// zkontroluje neškodná data objektu
	ASSERT ( m_pFirst == NULL );
	ASSERT ( m_pLast == NULL );
}

//////////////////////////////////////////////////////////////////////
// Operace s frontou
//////////////////////////////////////////////////////////////////////

// získá první prvek "cElement" z fronty (FALSE=fronta je prázdná)
template <class Element> 
inline BOOL CSelfPooledQueue<Element>::Get ( Element &cElement ) const 
{
	// zjsití, je-li fronta prázdná
	if ( m_pFirst == NULL )
	{	// fronta je prázdná
		ASSERT ( m_pLast == NULL );
		// vrátí pøíznak prázdné fronty
		return FALSE;
	}

	ASSERT ( m_pLast != NULL );

	// vyplní hodnotu prvku fronty
	cElement = m_pFirst->cElement;

	// vrátí pøíznak platného prvku fronty
	return TRUE;
}

// vyplní "cElement" dalším prvkem fronty na pozici "rPosition" (FALSE=konec fronty)
template <class Element> 
inline BOOL CSelfPooledQueue<Element>::GetNext ( POSITION &rPosition, Element &cElement ) 
{
	ASSERT ( rPosition != NULL );

	// zjistí, je-li "rPosition" za posledním prvkem fronty
	if ( *(struct SQueueMember **)rPosition == NULL )
	{	// "rPosition" je za posledním prvkem fronty
		// vrátí pøíznak konce fronty
		return FALSE;
	}

	// zjistí ukazatel na prvek fronty
	cElement = (*(struct SQueueMember **)rPosition)->cElement;
	// posune iterátor ve frontì
	rPosition = (POSITION)&(*(struct SQueueMember **)rPosition)->pNext;
	// vrátí pøíznak platného prvku fronty
	return TRUE;
}

// vyplní "pElement" ukazatelem na další prvek fronty na pozici "rPosition" 
//		(FALSE=konec seznamu)
template <class Element> 
inline BOOL CSelfPooledQueue<Element>::GetNext ( POSITION &rPosition, 
	Element *&pElement ) 
{
	ASSERT ( rPosition != NULL );

	// zjistí, je-li "rPosition" za posledním prvkem fronty
	if ( *(struct SQueueMember **)rPosition == NULL )
	{	// "rPosition" je za posledním prvkem fronty
		// vrátí pøíznak konce fronty
		return FALSE;
	}

	// vyplní ukazatel na prvek fronty
	pElement = &(*(struct SQueueMember **)rPosition)->cElement;
	// posune iterátor ve frontì
	rPosition = (POSITION)&(*(struct SQueueMember **)rPosition)->pNext;
	// vrátí pøíznak platného prvku fronty
	return TRUE;
}

// pøeskoèí prvek fronty na pozici "rPosition" (FALSE=konec seznamu)
template <class Element> 
inline BOOL CSelfPooledQueue<Element>::GetNext ( POSITION &rPosition ) 
{
	ASSERT ( rPosition != NULL );

	// zjistí, je-li "rPosition" za posledním prvkem fronty
	if ( *(struct SQueueMember **)rPosition == NULL )
	{	// "rPosition" je za posledním prvkem fronty
		// vrátí pøíznak konce fronty
		return FALSE;
	}

	// posune iterátor ve frontì
	rPosition = (POSITION)&(*(struct SQueueMember **)rPosition)->pNext;
	// vrátí pøíznak platného prvku fronty
	return TRUE;
}

// vrátí poèet prvkù fronty
template <class Element> 
DWORD CSelfPooledQueue<Element>::GetSize () const 
{
	// ukazatel na odkaz na prvek seznamu
	struct SQueueMember * const *pMember = &m_pFirst;

	// spoèítá poèet prvkù v seznamu
	for ( DWORD dwMemberCount = 0; *pMember != NULL; dwMemberCount++ )
	{
		// posune ukazatel na další prvek seznamu
		pMember = &(*pMember)->pNext;
	}

	// vrátí poèet prvkù v seznamu
	return dwMemberCount;
}

// vyjme první prvek fronty a vrátí jeho hodnotu
template <class Element> 
inline Element CSelfPooledQueue<Element>::RemoveFirst () 
{
	ASSERT ( ( m_pFirst != NULL ) && ( m_pLast != NULL ) );

	// uschová si ukazatel na první prvek fronty
	struct SQueueMember *pMember = m_pFirst;

	// vypojí prvek z fronty
	m_pFirst = pMember->pNext;
	// zjistí, je-li fronta prázdná
	if ( m_pFirst == NULL )
	{	// fronta je prázdná
		ASSERT ( m_pLast == pMember );
		// aktualizuje ukazatel na poslední prvek fronty
		m_pLast = NULL;
	}

	// zjistí hodnotu prvku fronty
	Element cElement = pMember->cElement;

	// smaže prvek fronty
	m_cPool.Free ( pMember );

	// vrátí hodnotu mazaného prvku fronty
	return cElement;
}

// vyjme z fronty první prvek "cElement" (FALSE=fronta byla prázdná, prvek nebyl vyjmut)
template <class Element> 
inline BOOL CSelfPooledQueue<Element>::RemoveFirst ( Element &cElement ) 
{
	// zjsití, je-li fronta prázdná
	if ( m_pFirst == NULL )
	{	// fronta je prázdná
		ASSERT ( m_pLast == NULL );
		// vrátí pøíznak prázdné fronty
		return FALSE;
	}

	ASSERT ( m_pLast != NULL );

	// uschová si ukazatel na první prvek fronty
	struct SQueueMember *pMember = m_pFirst;

	// vypojí prvek z fronty
	m_pFirst = pMember->pNext;
	// zjistí, je-li fronta prázdná
	if ( m_pFirst == NULL )
	{	// fronta je prázdná
		ASSERT ( m_pLast == pMember );
		// aktualizuje ukazatel na poslední prvek fronty
		m_pLast = NULL;
	}

	// vyplní hodnotu prvku fronty
	cElement = pMember->cElement;
	// smaže prvek fronty
	m_cPool.Free ( pMember );

	// vrátí pøíznak platného prvku fronty
	return TRUE;
}

// pøidá do fronty prvek "cElement"
template <class Element> 
inline void CSelfPooledQueue<Element>::Add ( Element cElement ) 
{
	// alokuje nový prvek fronty
	struct SQueueMember *pNewMember = m_cPool.Allocate ();

	// inicializuje novì alokovaný prvek fronty
	pNewMember->pNext = NULL;
	pNewMember->cElement = cElement;

	// zjistí, je-li fronta prázdná
	if ( m_pLast == NULL )
	{	// fronta je prázdná
		ASSERT ( m_pFirst == NULL );
		// pøidá prvek do fronty
		m_pFirst = m_pLast = pNewMember;
	}
	else
	{	// fronta není prázdná
		ASSERT ( m_pLast->pNext == NULL );
		ASSERT ( m_pFirst != NULL );
		// pøidá prvek na konec fronty
		m_pLast->pNext = pNewMember;
		m_pLast = pNewMember;
	}
}

// smaže z fronty prvek "cElement" (FALSE=prvek nebyl ve frontì nalezen)
template <class Element> 
BOOL CSelfPooledQueue<Element>::RemoveElement ( Element &cElement ) 
{
	// získá ukazatel na první prvek fronty
	struct SQueueMember *pMember = m_pFirst;
	// ukazatel na pøedchozí prvek
	struct SQueueMember *pPreviousMember = NULL;

	// projede frontu
	while ( pMember != NULL )
	{
		// zjistí, jedná-li se o mazaný prvek
		if ( pMember->cElement == cElement )
		{	// jedná se o mazaný prvek
		// vypojí prvek z fronty

			// zjistí, jedná-li se o poslední prvek fronty
			if ( pMember == m_pLast )
			{	// jedná se o poslední prvek fronty
				ASSERT ( pMember->pNext == NULL );
				// aktualizuje ukazatel na poslední prvek fronty
				m_pLast = pPreviousMember;
			}
			// zjistí, jedná-li se o první prvek fronty
			if ( pMember == m_pFirst )
			{	// jedná se o první prvek fronty
				// aktualizuje ukazatel na první prvek fronty
				m_pFirst = pMember->pNext;
			}
			else
			{	// nejedná se o první prvek fronty
				// vypojí prvek z fronty
				pPreviousMember->pNext = pMember->pNext;
			}

			// smaže prvek fronty
			m_cPool.Free ( pMember );

			// vrátí pøíznak smazání prvku fronty
			return TRUE;
		}

		// nechá zpracovat další prvek fronty
		pPreviousMember = pMember;
		pMember = pMember->pNext;
	}

	// vrátí pøíznak nenalezení prvku
	return FALSE;
}

// smaže prvek fronty "position" a vrátí jeho hodnotu ("position" je platná a 
//		odkazuje na další prvek seznamu)
template <class Element> 
inline Element CSelfPooledQueue<Element>::RemovePosition ( POSITION position ) 
{
	ASSERT ( position != NULL );
	ASSERT ( *(struct SQueueMember **)position != NULL );

	// uschová si ukazatel na prvek fronty
	struct SQueueMember *pMember = *(struct SQueueMember **)position;

	// vypojí prvek z fronty
	*(struct SQueueMember **)position = pMember->pNext;
	// zjistí hodnotu prvku fronty
	Element cElement = pMember->cElement;

	// smaže prvek fronty
	m_cPool.Free ( pMember );

	// zkontroluje maže-li se poslední prvek fronty
	if ( pMember == m_pLast )
	{	// jedná se o konec fronty
		if ( m_pFirst == NULL )
		{
			m_pLast = NULL;
		}
		else
		{
			m_pLast = m_pFirst;
			while ( m_pLast->pNext != NULL )
			{
				m_pLast = m_pLast->pNext;
			}
		}
	}

	// vrátí hodnotu mazaného prvku fronty
	return cElement;
}

// vyjme z fronty všechny prvky
template <class Element> 
inline void CSelfPooledQueue<Element>::RemoveAll () 
{
	// smaže všechny alokované prvky
	m_cPool.FreeAll ();

	// aktualizuje ukazatel na první prvek fronty
	m_pFirst = NULL;
	// aktualizuje ukazatel na poslední prvek fronty
	m_pLast = NULL;
}

#endif //__SELF_POOLED_QUEUE__HEADER_INCLUDED__
