/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Abstraktn� datov� typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: �ablona prioritn� fronty s vlastn�m memory poolem
 * 
 ***********************************************************/

#ifndef __SELF_POOLED_PRIORITY_QUEUE__HEADER_INCLUDED__
#define __SELF_POOLED_PRIORITY_QUEUE__HEADER_INCLUDED__

#include "TypedMemoryPool.h"

//////////////////////////////////////////////////////////////////////
// �ablona t��dy prioritn� fronty s vlastn�m memory poolem
template <class Priority, class Element> 
class CSelfPooledPriorityQueue 
{
// Datov� typy
private:
	// prvek fronty
	struct SPriorityQueueMember 
	{
		// priorita
		Priority cPriority;
		// ukazatel na dal�� prvek fronty
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

	// zjist�, je-li fronta pr�zdn�
	BOOL IsEmpty () const { return ( m_pFirst == NULL ); };

	// vyjme z fronty prvn� prvek a vr�t� jeho hodnotu
	inline Element RemoveFirst ();
	// vyjme z fronty prvn� prvek "cElement" s prioritou "cPriority" (FALSE=fronta byla 
	//		pr�zdn�, prvek nebyl vyjmut)
	inline BOOL RemoveFirst ( Priority &cPriority, Element &cElement );
	// vyjme z fronty prvn� prvek "cElement" (FALSE=fronta byla pr�zdn�, prvek nebyl 
	//		vyjmut)
	inline BOOL RemoveFirst ( Element &cElement );
	// p�id� do fronty prvek "cElement" s prioritou "cPriority"
	void Add ( Priority cPriority, Element cElement );
	// sma�e z fronty prvek "cElement" (FALSE=prvek nebyl ve front� nalezen)
	BOOL RemoveElement ( Element &cElement );
	// vyjme z fronty v�echny prvky
	inline void RemoveAll ();

// Data
protected:
	// prvn� prvek fronty
	struct SPriorityQueueMember *m_pFirst;
	// memory pool prvk� fronty
	CTypedMemoryPool<struct SPriorityQueueMember> m_cPool;
};

//////////////////////////////////////////////////////////////////////
// Metody �ablony CSelfPooledPriorityQueue

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
template <class Priority, class Element> 
inline CSelfPooledPriorityQueue<Priority, Element>::CSelfPooledPriorityQueue ( 
	DWORD dwBlockSize ) : m_cPool ( dwBlockSize ) 
{
	// zne�kodn� data objektu
	m_pFirst = NULL;
}

// destruktor
template <class Priority, class Element> 
inline CSelfPooledPriorityQueue<Priority, Element>::~CSelfPooledPriorityQueue () 
{
	// zkontroluje ne�kodn� data objektu
	ASSERT ( m_pFirst == NULL );
}

//////////////////////////////////////////////////////////////////////
// Operace s frontou
//////////////////////////////////////////////////////////////////////

// vyjme prvn� prvek fronty a vr�t� jeho hodnotu
template <class Priority, class Element> 
inline Element CSelfPooledPriorityQueue<Priority, Element>::RemoveFirst () 
{
	ASSERT ( m_pFirst != NULL );

	// uschov� si ukazatel na prvn� prvek fronty
	struct SPriorityQueueMember *pMember = m_pFirst;

	// vypoj� prvek z fronty
	m_pFirst = pMember->pNext;
	// zjist�, je-li fronta pr�zdn�
	if ( m_pFirst == NULL )
	{	// fronta je pr�zdn�
		ASSERT ( m_pLast == pMember );
		// aktualizuje ukazatel na posledn� prvek fronty
		m_pLast = NULL;
	}

	// zjist� hodnotu prvku fronty
	Element cElement = pMember->cElement;

	// sma�e prvek fronty
	m_cPool.Free ( pMember );

	// vr�t� hodnotu mazan�ho prvku fronty
	return cElement;
}

// vyjme z fronty prvn� prvek "cElement" s prioritou "cPriority" (FALSE=fronta byla 
//		pr�zdn�, prvek nebyl vyjmut)
template <class Priority, class Element> 
inline BOOL CSelfPooledPriorityQueue<Priority, Element>::RemoveFirst ( 
	Priority &cPriority, Element &cElement ) 
{
	// zjsit�, je-li fronta pr�zdn�
	if ( m_pFirst == NULL )
	{	// fronta je pr�zdn�
		// vr�t� p��znak pr�zdn� fronty
		return FALSE;
	}

	// uschov� si ukazatel na prvn� prvek fronty
	struct SPriorityQueueMember *pMember = m_pFirst;

	// vypoj� prvek z fronty
	m_pFirst = pMember->pNext;

	// vypln� prioritu prvku fronty
	cPriority = pMember->cPriority;
	// vypln� hodnotu prvku fronty
	cElement = pMember->cElement;
	// sma�e prvek fronty
	m_cPool.Free ( pMember );

	// vr�t� p��znak platn�ho prvku fronty
	return TRUE;
}

// vyjme z fronty prvn� prvek "cElement" (FALSE=fronta byla pr�zdn�, prvek nebyl vyjmut)
template <class Priority, class Element> 
inline BOOL CSelfPooledPriorityQueue<Priority, Element>::RemoveFirst ( 
	Element &cElement ) 
{
	// zjsit�, je-li fronta pr�zdn�
	if ( m_pFirst == NULL )
	{	// fronta je pr�zdn�
		// vr�t� p��znak pr�zdn� fronty
		return FALSE;
	}

	// uschov� si ukazatel na prvn� prvek fronty
	struct SPriorityQueueMember *pMember = m_pFirst;

	// vypoj� prvek z fronty
	m_pFirst = pMember->pNext;

	// vypln� hodnotu prvku fronty
	cElement = pMember->cElement;
	// sma�e prvek fronty
	m_cPool.Free ( pMember );

	// vr�t� p��znak platn�ho prvku fronty
	return TRUE;
}

// p�id� do fronty prvek "cElement" s prioritou "cPriority"
template <class Priority, class Element> 
void CSelfPooledPriorityQueue<Priority, Element>::Add ( Priority cPriority, 
	Element cElement ) 
{
	// alokuje nov� prvek fronty
	struct SPriorityQueueMember *pNewMember = m_cPool.Allocate ();

	// inicializuje nov� alokovan� prvek fronty
	pNewMember->cPriority = cPriority;
	pNewMember->cElement = cElement;

	// ukazatel na prvek fronty
	struct SPriorityQueueMember **pMember = &m_pFirst;

	// najde m�sto um�st�n� prvku do fronty
	while ( (*pMember) != NULL )
	{
		// zjist�, je-li priorita p�id�van�ho prvku ni���
		if ( (*pMember)->cPriority >= cPriority )
		{	// priorita p�id�van�ho prvku je ni���
			// nech� vlo�it prvek na nalezen� m�sto
			break;
		}

		// posune se na dal�� prvek fronty
		pMember = &(*pMember)->pNext;
	}

	// p�id� prvek do fronty na uveden� m�sto
	pNewMember->pNext = *pMember;
	*pMember = pNewMember;
}

// sma�e z fronty prvek "cElement" (FALSE=prvek nebyl ve front� nalezen)
template <class Priority, class Element> 
BOOL CSelfPooledPriorityQueue<Priority, Element>::RemoveElement ( Element &cElement ) 
{
	// ukazatel na odkaz na prvek fronty
	struct SPriorityQueueMember **pMember = &m_pFirst;

	// projede frontu
	while ( *pMember != NULL )
	{
		// zjist�, jedn�-li se o mazan� prvek
		if ( (*pMember)->cElement == cElement )
		{	// jedn� se o mazan� prvek

			// uschov� si ukazatel na mazan� prvek
			struct SPriorityQueueMember *pDeletedMember = *pMember;

			// vypoj� prvek z fronty
			*pMember = (*pMember)->pNext;

			// sma�e prvek fronty
			m_cPool.Free ( pDeletedMember );

			// vr�t� p��znak smaz�n� prvku fronty
			return TRUE;
		}

		// nech� zpracovat dal�� prvek fronty
		pMember = &(*pMember)->pNext;
	}

	// vr�t� p��znak nenalezen� prvku
	return FALSE;
}

// vyjme z fronty v�echny prvky
template <class Priority, class Element> 
inline void CSelfPooledPriorityQueue<Priority, Element>::RemoveAll () 
{
	// sma�e v�echny alokovan� prvky
	m_cPool.FreeAll ();

	// aktualizuje ukazatel na prvn� prvek fronty
	m_pFirst = NULL;
}

#endif //__SELF_POOLED_PRIORITY_QUEUE__HEADER_INCLUDED__
