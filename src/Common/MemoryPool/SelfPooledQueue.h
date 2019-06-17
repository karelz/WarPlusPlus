/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Abstraktn� datov� typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: �ablona fronty s vlastn�m memory poolem
 * 
 ***********************************************************/

#ifndef __SELF_POOLED_QUEUE__HEADER_INCLUDED__
#define __SELF_POOLED_QUEUE__HEADER_INCLUDED__

#include "TypedMemoryPool.h"

//////////////////////////////////////////////////////////////////////
// �ablona t��dy fronty s vlastn�m memory poolem
template <class Element> 
class CSelfPooledQueue 
{
// Datov� typy
private:
	// prvek fronty
	struct SQueueMember 
	{
		// ukazatel na dal�� prvek fronty
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

	// zjist�, je-li fronta pr�zdn�
	BOOL IsEmpty () const { return ( m_pFirst == NULL ); };
	// z�sk� prvn� prvek "cElement" z fronty (FALSE=fronta je pr�zdn�)
	inline BOOL Get ( Element &cElement ) const;
	// vr�t� pozici prvn�ho prvku fronty
	POSITION GetHeadPosition () const { return (POSITION)&m_pFirst; };
	// vypln� "cElement" dal��m prvkem fornty na pozici "rPosition" (FALSE=konec fronty)
	static inline BOOL GetNext ( POSITION &rPosition, Element &cElement );
	// vypln� "pElement" ukazatelem na dal�� prvek fronty na pozici "rPosition" 
	//		(FALSE=konec seznamu)
	static inline BOOL GetNext ( POSITION &rPosition, Element *&pElement );
	// p�esko�� prvek fronty na pozici "rPosition" (FALSE=konec seznamu)
	static inline BOOL GetNext ( POSITION &rPosition );
	// vr�t� po�et prvk� fronty
	DWORD GetSize () const;

	// vyjme prvn� prvek fronty a vr�t� jeho hodnotu
	inline Element RemoveFirst ();
	// vyjme z fronty prvn� prvek "cElement" (FALSE=fronta byla pr�zdn�, prvek nebyl 
	//		vyjmut)
	inline BOOL RemoveFirst ( Element &cElement );
	// p�id� do fronty prvek "cElement"
	inline void Add ( Element cElement );
	// sma�e z fronty prvek "cElement" (FALSE=prvek nebyl ve front� nalezen)
	BOOL RemoveElement ( Element &cElement );
	// sma�e prvek fronty "position" a vr�t� jeho hodnotu ("position" je platn� a 
	//		odkazuje na dal�� prvek seznamu)
	Element RemovePosition ( POSITION position );
	// vyjme z fronty v�echny prvky
	inline void RemoveAll ();

// Data
protected:
	// prvn� prvek fronty
	struct SQueueMember *m_pFirst;
	// posledn� prvek fronty
	struct SQueueMember *m_pLast;
	// memory pool prvk� fronty
	CTypedMemoryPool<struct SQueueMember> m_cPool;
};

//////////////////////////////////////////////////////////////////////
// Metody �ablony CSelfPooledQueue

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
template <class Element> 
inline CSelfPooledQueue<Element>::CSelfPooledQueue ( DWORD dwBlockSize ) : 
	m_cPool ( dwBlockSize ) 
{
	// zne�kodn� data objektu
	m_pFirst = NULL;
	m_pLast = NULL;
}

// destruktor
template <class Element> 
inline CSelfPooledQueue<Element>::~CSelfPooledQueue () 
{
	// zkontroluje ne�kodn� data objektu
	ASSERT ( m_pFirst == NULL );
	ASSERT ( m_pLast == NULL );
}

//////////////////////////////////////////////////////////////////////
// Operace s frontou
//////////////////////////////////////////////////////////////////////

// z�sk� prvn� prvek "cElement" z fronty (FALSE=fronta je pr�zdn�)
template <class Element> 
inline BOOL CSelfPooledQueue<Element>::Get ( Element &cElement ) const 
{
	// zjsit�, je-li fronta pr�zdn�
	if ( m_pFirst == NULL )
	{	// fronta je pr�zdn�
		ASSERT ( m_pLast == NULL );
		// vr�t� p��znak pr�zdn� fronty
		return FALSE;
	}

	ASSERT ( m_pLast != NULL );

	// vypln� hodnotu prvku fronty
	cElement = m_pFirst->cElement;

	// vr�t� p��znak platn�ho prvku fronty
	return TRUE;
}

// vypln� "cElement" dal��m prvkem fronty na pozici "rPosition" (FALSE=konec fronty)
template <class Element> 
inline BOOL CSelfPooledQueue<Element>::GetNext ( POSITION &rPosition, Element &cElement ) 
{
	ASSERT ( rPosition != NULL );

	// zjist�, je-li "rPosition" za posledn�m prvkem fronty
	if ( *(struct SQueueMember **)rPosition == NULL )
	{	// "rPosition" je za posledn�m prvkem fronty
		// vr�t� p��znak konce fronty
		return FALSE;
	}

	// zjist� ukazatel na prvek fronty
	cElement = (*(struct SQueueMember **)rPosition)->cElement;
	// posune iter�tor ve front�
	rPosition = (POSITION)&(*(struct SQueueMember **)rPosition)->pNext;
	// vr�t� p��znak platn�ho prvku fronty
	return TRUE;
}

// vypln� "pElement" ukazatelem na dal�� prvek fronty na pozici "rPosition" 
//		(FALSE=konec seznamu)
template <class Element> 
inline BOOL CSelfPooledQueue<Element>::GetNext ( POSITION &rPosition, 
	Element *&pElement ) 
{
	ASSERT ( rPosition != NULL );

	// zjist�, je-li "rPosition" za posledn�m prvkem fronty
	if ( *(struct SQueueMember **)rPosition == NULL )
	{	// "rPosition" je za posledn�m prvkem fronty
		// vr�t� p��znak konce fronty
		return FALSE;
	}

	// vypln� ukazatel na prvek fronty
	pElement = &(*(struct SQueueMember **)rPosition)->cElement;
	// posune iter�tor ve front�
	rPosition = (POSITION)&(*(struct SQueueMember **)rPosition)->pNext;
	// vr�t� p��znak platn�ho prvku fronty
	return TRUE;
}

// p�esko�� prvek fronty na pozici "rPosition" (FALSE=konec seznamu)
template <class Element> 
inline BOOL CSelfPooledQueue<Element>::GetNext ( POSITION &rPosition ) 
{
	ASSERT ( rPosition != NULL );

	// zjist�, je-li "rPosition" za posledn�m prvkem fronty
	if ( *(struct SQueueMember **)rPosition == NULL )
	{	// "rPosition" je za posledn�m prvkem fronty
		// vr�t� p��znak konce fronty
		return FALSE;
	}

	// posune iter�tor ve front�
	rPosition = (POSITION)&(*(struct SQueueMember **)rPosition)->pNext;
	// vr�t� p��znak platn�ho prvku fronty
	return TRUE;
}

// vr�t� po�et prvk� fronty
template <class Element> 
DWORD CSelfPooledQueue<Element>::GetSize () const 
{
	// ukazatel na odkaz na prvek seznamu
	struct SQueueMember * const *pMember = &m_pFirst;

	// spo��t� po�et prvk� v seznamu
	for ( DWORD dwMemberCount = 0; *pMember != NULL; dwMemberCount++ )
	{
		// posune ukazatel na dal�� prvek seznamu
		pMember = &(*pMember)->pNext;
	}

	// vr�t� po�et prvk� v seznamu
	return dwMemberCount;
}

// vyjme prvn� prvek fronty a vr�t� jeho hodnotu
template <class Element> 
inline Element CSelfPooledQueue<Element>::RemoveFirst () 
{
	ASSERT ( ( m_pFirst != NULL ) && ( m_pLast != NULL ) );

	// uschov� si ukazatel na prvn� prvek fronty
	struct SQueueMember *pMember = m_pFirst;

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

// vyjme z fronty prvn� prvek "cElement" (FALSE=fronta byla pr�zdn�, prvek nebyl vyjmut)
template <class Element> 
inline BOOL CSelfPooledQueue<Element>::RemoveFirst ( Element &cElement ) 
{
	// zjsit�, je-li fronta pr�zdn�
	if ( m_pFirst == NULL )
	{	// fronta je pr�zdn�
		ASSERT ( m_pLast == NULL );
		// vr�t� p��znak pr�zdn� fronty
		return FALSE;
	}

	ASSERT ( m_pLast != NULL );

	// uschov� si ukazatel na prvn� prvek fronty
	struct SQueueMember *pMember = m_pFirst;

	// vypoj� prvek z fronty
	m_pFirst = pMember->pNext;
	// zjist�, je-li fronta pr�zdn�
	if ( m_pFirst == NULL )
	{	// fronta je pr�zdn�
		ASSERT ( m_pLast == pMember );
		// aktualizuje ukazatel na posledn� prvek fronty
		m_pLast = NULL;
	}

	// vypln� hodnotu prvku fronty
	cElement = pMember->cElement;
	// sma�e prvek fronty
	m_cPool.Free ( pMember );

	// vr�t� p��znak platn�ho prvku fronty
	return TRUE;
}

// p�id� do fronty prvek "cElement"
template <class Element> 
inline void CSelfPooledQueue<Element>::Add ( Element cElement ) 
{
	// alokuje nov� prvek fronty
	struct SQueueMember *pNewMember = m_cPool.Allocate ();

	// inicializuje nov� alokovan� prvek fronty
	pNewMember->pNext = NULL;
	pNewMember->cElement = cElement;

	// zjist�, je-li fronta pr�zdn�
	if ( m_pLast == NULL )
	{	// fronta je pr�zdn�
		ASSERT ( m_pFirst == NULL );
		// p�id� prvek do fronty
		m_pFirst = m_pLast = pNewMember;
	}
	else
	{	// fronta nen� pr�zdn�
		ASSERT ( m_pLast->pNext == NULL );
		ASSERT ( m_pFirst != NULL );
		// p�id� prvek na konec fronty
		m_pLast->pNext = pNewMember;
		m_pLast = pNewMember;
	}
}

// sma�e z fronty prvek "cElement" (FALSE=prvek nebyl ve front� nalezen)
template <class Element> 
BOOL CSelfPooledQueue<Element>::RemoveElement ( Element &cElement ) 
{
	// z�sk� ukazatel na prvn� prvek fronty
	struct SQueueMember *pMember = m_pFirst;
	// ukazatel na p�edchoz� prvek
	struct SQueueMember *pPreviousMember = NULL;

	// projede frontu
	while ( pMember != NULL )
	{
		// zjist�, jedn�-li se o mazan� prvek
		if ( pMember->cElement == cElement )
		{	// jedn� se o mazan� prvek
		// vypoj� prvek z fronty

			// zjist�, jedn�-li se o posledn� prvek fronty
			if ( pMember == m_pLast )
			{	// jedn� se o posledn� prvek fronty
				ASSERT ( pMember->pNext == NULL );
				// aktualizuje ukazatel na posledn� prvek fronty
				m_pLast = pPreviousMember;
			}
			// zjist�, jedn�-li se o prvn� prvek fronty
			if ( pMember == m_pFirst )
			{	// jedn� se o prvn� prvek fronty
				// aktualizuje ukazatel na prvn� prvek fronty
				m_pFirst = pMember->pNext;
			}
			else
			{	// nejedn� se o prvn� prvek fronty
				// vypoj� prvek z fronty
				pPreviousMember->pNext = pMember->pNext;
			}

			// sma�e prvek fronty
			m_cPool.Free ( pMember );

			// vr�t� p��znak smaz�n� prvku fronty
			return TRUE;
		}

		// nech� zpracovat dal�� prvek fronty
		pPreviousMember = pMember;
		pMember = pMember->pNext;
	}

	// vr�t� p��znak nenalezen� prvku
	return FALSE;
}

// sma�e prvek fronty "position" a vr�t� jeho hodnotu ("position" je platn� a 
//		odkazuje na dal�� prvek seznamu)
template <class Element> 
inline Element CSelfPooledQueue<Element>::RemovePosition ( POSITION position ) 
{
	ASSERT ( position != NULL );
	ASSERT ( *(struct SQueueMember **)position != NULL );

	// uschov� si ukazatel na prvek fronty
	struct SQueueMember *pMember = *(struct SQueueMember **)position;

	// vypoj� prvek z fronty
	*(struct SQueueMember **)position = pMember->pNext;
	// zjist� hodnotu prvku fronty
	Element cElement = pMember->cElement;

	// sma�e prvek fronty
	m_cPool.Free ( pMember );

	// zkontroluje ma�e-li se posledn� prvek fronty
	if ( pMember == m_pLast )
	{	// jedn� se o konec fronty
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

	// vr�t� hodnotu mazan�ho prvku fronty
	return cElement;
}

// vyjme z fronty v�echny prvky
template <class Element> 
inline void CSelfPooledQueue<Element>::RemoveAll () 
{
	// sma�e v�echny alokovan� prvky
	m_cPool.FreeAll ();

	// aktualizuje ukazatel na prvn� prvek fronty
	m_pFirst = NULL;
	// aktualizuje ukazatel na posledn� prvek fronty
	m_pLast = NULL;
}

#endif //__SELF_POOLED_QUEUE__HEADER_INCLUDED__
