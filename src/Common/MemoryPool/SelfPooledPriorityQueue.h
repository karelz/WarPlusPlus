/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Abstraktní datové typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Šablona prioritní fronty s vlastním memory poolem
 * 
 ***********************************************************/

#ifndef __SELF_POOLED_PRIORITY_QUEUE__HEADER_INCLUDED__
#define __SELF_POOLED_PRIORITY_QUEUE__HEADER_INCLUDED__

#include "TypedMemoryPool.h"

//////////////////////////////////////////////////////////////////////
// Šablona tøídy prioritní fronty s vlastním memory poolem
template <class Priority, class Element> 
class CSelfPooledPriorityQueue 
{
// Datové typy
private:
	// prvek fronty
	struct SPriorityQueueMember 
	{
		// priorita
		Priority cPriority;
		// ukazatel na další prvek fronty
		struct SPriorityQueueMember *pNext;
		// data prvku fronty
		Element cElement;
	};

// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	inline CSelfPooledPriorityQueue ( DWORD dwBlockSize );
	// destruktor
	inline ~CSelfPooledPriorityQueue ();

// Operace s frontou

	// zjistí, je-li fronta prázdná
	BOOL IsEmpty () const { return ( m_pFirst == NULL ); };

	// vyjme z fronty první prvek a vrátí jeho hodnotu
	inline Element RemoveFirst ();
	// vyjme z fronty první prvek "cElement" s prioritou "cPriority" (FALSE=fronta byla 
	//		prázdná, prvek nebyl vyjmut)
	inline BOOL RemoveFirst ( Priority &cPriority, Element &cElement );
	// vyjme z fronty první prvek "cElement" (FALSE=fronta byla prázdná, prvek nebyl 
	//		vyjmut)
	inline BOOL RemoveFirst ( Element &cElement );
	// pøidá do fronty prvek "cElement" s prioritou "cPriority"
	void Add ( Priority cPriority, Element cElement );
	// smaže z fronty prvek "cElement" (FALSE=prvek nebyl ve frontì nalezen)
	BOOL RemoveElement ( Element &cElement );
	// vyjme z fronty všechny prvky
	inline void RemoveAll ();

// Data
protected:
	// první prvek fronty
	struct SPriorityQueueMember *m_pFirst;
	// memory pool prvkù fronty
	CTypedMemoryPool<struct SPriorityQueueMember> m_cPool;
};

//////////////////////////////////////////////////////////////////////
// Metody šablony CSelfPooledPriorityQueue

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
template <class Priority, class Element> 
inline CSelfPooledPriorityQueue<Priority, Element>::CSelfPooledPriorityQueue ( 
	DWORD dwBlockSize ) : m_cPool ( dwBlockSize ) 
{
	// zneškodní data objektu
	m_pFirst = NULL;
}

// destruktor
template <class Priority, class Element> 
inline CSelfPooledPriorityQueue<Priority, Element>::~CSelfPooledPriorityQueue () 
{
	// zkontroluje neškodná data objektu
	ASSERT ( m_pFirst == NULL );
}

//////////////////////////////////////////////////////////////////////
// Operace s frontou
//////////////////////////////////////////////////////////////////////

// vyjme první prvek fronty a vrátí jeho hodnotu
template <class Priority, class Element> 
inline Element CSelfPooledPriorityQueue<Priority, Element>::RemoveFirst () 
{
	ASSERT ( m_pFirst != NULL );

	// uschová si ukazatel na první prvek fronty
	struct SPriorityQueueMember *pMember = m_pFirst;

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

// vyjme z fronty první prvek "cElement" s prioritou "cPriority" (FALSE=fronta byla 
//		prázdná, prvek nebyl vyjmut)
template <class Priority, class Element> 
inline BOOL CSelfPooledPriorityQueue<Priority, Element>::RemoveFirst ( 
	Priority &cPriority, Element &cElement ) 
{
	// zjsití, je-li fronta prázdná
	if ( m_pFirst == NULL )
	{	// fronta je prázdná
		// vrátí pøíznak prázdné fronty
		return FALSE;
	}

	// uschová si ukazatel na první prvek fronty
	struct SPriorityQueueMember *pMember = m_pFirst;

	// vypojí prvek z fronty
	m_pFirst = pMember->pNext;

	// vyplní prioritu prvku fronty
	cPriority = pMember->cPriority;
	// vyplní hodnotu prvku fronty
	cElement = pMember->cElement;
	// smaže prvek fronty
	m_cPool.Free ( pMember );

	// vrátí pøíznak platného prvku fronty
	return TRUE;
}

// vyjme z fronty první prvek "cElement" (FALSE=fronta byla prázdná, prvek nebyl vyjmut)
template <class Priority, class Element> 
inline BOOL CSelfPooledPriorityQueue<Priority, Element>::RemoveFirst ( 
	Element &cElement ) 
{
	// zjsití, je-li fronta prázdná
	if ( m_pFirst == NULL )
	{	// fronta je prázdná
		// vrátí pøíznak prázdné fronty
		return FALSE;
	}

	// uschová si ukazatel na první prvek fronty
	struct SPriorityQueueMember *pMember = m_pFirst;

	// vypojí prvek z fronty
	m_pFirst = pMember->pNext;

	// vyplní hodnotu prvku fronty
	cElement = pMember->cElement;
	// smaže prvek fronty
	m_cPool.Free ( pMember );

	// vrátí pøíznak platného prvku fronty
	return TRUE;
}

// pøidá do fronty prvek "cElement" s prioritou "cPriority"
template <class Priority, class Element> 
void CSelfPooledPriorityQueue<Priority, Element>::Add ( Priority cPriority, 
	Element cElement ) 
{
	// alokuje nový prvek fronty
	struct SPriorityQueueMember *pNewMember = m_cPool.Allocate ();

	// inicializuje novì alokovaný prvek fronty
	pNewMember->cPriority = cPriority;
	pNewMember->cElement = cElement;

	// ukazatel na prvek fronty
	struct SPriorityQueueMember **pMember = &m_pFirst;

	// najde místo umístìní prvku do fronty
	while ( (*pMember) != NULL )
	{
		// zjistí, je-li priorita pøidávaného prvku nižší
		if ( (*pMember)->cPriority >= cPriority )
		{	// priorita pøidávaného prvku je nižší
			// nechá vložit prvek na nalezené místo
			break;
		}

		// posune se na další prvek fronty
		pMember = &(*pMember)->pNext;
	}

	// pøidá prvek do fronty na uvedené místo
	pNewMember->pNext = *pMember;
	*pMember = pNewMember;
}

// smaže z fronty prvek "cElement" (FALSE=prvek nebyl ve frontì nalezen)
template <class Priority, class Element> 
BOOL CSelfPooledPriorityQueue<Priority, Element>::RemoveElement ( Element &cElement ) 
{
	// ukazatel na odkaz na prvek fronty
	struct SPriorityQueueMember **pMember = &m_pFirst;

	// projede frontu
	while ( *pMember != NULL )
	{
		// zjistí, jedná-li se o mazaný prvek
		if ( (*pMember)->cElement == cElement )
		{	// jedná se o mazaný prvek

			// uschová si ukazatel na mazaný prvek
			struct SPriorityQueueMember *pDeletedMember = *pMember;

			// vypojí prvek z fronty
			*pMember = (*pMember)->pNext;

			// smaže prvek fronty
			m_cPool.Free ( pDeletedMember );

			// vrátí pøíznak smazání prvku fronty
			return TRUE;
		}

		// nechá zpracovat další prvek fronty
		pMember = &(*pMember)->pNext;
	}

	// vrátí pøíznak nenalezení prvku
	return FALSE;
}

// vyjme z fronty všechny prvky
template <class Priority, class Element> 
inline void CSelfPooledPriorityQueue<Priority, Element>::RemoveAll () 
{
	// smaže všechny alokované prvky
	m_cPool.FreeAll ();

	// aktualizuje ukazatel na první prvek fronty
	m_pFirst = NULL;
}

#endif //__SELF_POOLED_PRIORITY_QUEUE__HEADER_INCLUDED__
