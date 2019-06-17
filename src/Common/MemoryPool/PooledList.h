/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Abstraktn� datov� typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: �ablona poolovan�ho seznamu
 * 
 ***********************************************************/

#ifndef __POOLED_LIST__HEADER_INCLUDED__
#define __POOLED_LIST__HEADER_INCLUDED__

#include "TypedMemoryPool.h"

//////////////////////////////////////////////////////////////////////
// �ablona t��dy poolovan�ho seznamu
template <class Element, int nStaticCopyNumber> 
class CPooledList 
{
// Datov� typy
private:
	// prvek seznamu
	struct SListMember 
	{
		// ukazatel na dal�� prvek seznamu
		struct SListMember *pNext;
		// data prvku seznamu
		Element cElement;
	};

// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	inline CPooledList ();
	// destruktor
	inline ~CPooledList ();

// Operace se seznamem

	// zjist�, je-li seznam pr�zdn�
	BOOL IsEmpty () const { return ( m_pFirst == NULL ); };
	// zjist�, je-li seznam od pozice "position" pr�zdn�
	static inline BOOL IsEmpty ( POSITION position );
	// vr�t� pozici prvn�ho prvku seznamu
	POSITION GetHeadPosition () const { return (POSITION)&m_pFirst; };
	// vypln� "cElement" dal��m prvkem seznamu na pozici "rPosition" (FALSE=konec seznamu)
	static inline BOOL GetNext ( POSITION &rPosition, Element &cElement );
	// vypln� "pElement" ukazatelem na dal�� prvek seznamu na pozici "rPosition" 
	//		(FALSE=konec seznamu)
	static inline BOOL GetNext ( POSITION &rPosition, Element *&pElement );
	// p�esko�� prvek seznamu na pozici "rPosition" (FALSE=konec seznamu)
	static inline BOOL GetNext ( POSITION &rPosition );
	// najde prvek "cElement" v seznamu a vr�t� jeho pozici (NULL=nenalezeno)
	POSITION Find ( Element cElement ) const;
	// vr�t� po�et prvk� v seznamu
	DWORD GetSize () const;

	// vyjme prvn� prvek seznamu a vr�t� jeho hodnotu
	inline Element RemoveFirst ();
	// vyjme za seznamu prvn� prvek "cElement" (FALSE=seznam byl pr�zdn�, prvek nebyl 
	//		vyjmut)
	inline BOOL RemoveFirst ( Element &cElement );
	// p�id� do seznamu prvek "cElement"
	inline void Add ( Element cElement );
	// vlo�� do seznamu prvek "cElement" na pozici "position"
	static inline void Insert ( POSITION position, Element cElement );
	// sma�e prvek seznamu "position" a vr�t� jeho hodnotu ("position" je platn� a 
	//		odkazuje na dal�� prvek seznamu)
	static inline Element RemovePosition ( POSITION position );
	// p�esune prvek seznamu "position" na za��tek seznamu
	inline void MoveFirst ( POSITION position );
	// p�esune prvek seznamu "position" do seznamu "cList"
	static inline void Move ( POSITION position, 
		CPooledList<Element, nStaticCopyNumber> &cList );
	// p�esune seznam do seznamu "cList"
	inline void MoveList ( CPooledList<Element, nStaticCopyNumber> &cList );
	// vyjme ze seznamu v�echny prvky
	void RemoveAll ();

// Data
protected:
	// prvn� prvek seznamu
	struct SListMember *m_pFirst;
	// memory pool prvk� seznamu
	static CTypedMemoryPool<struct SListMember> m_cPool;
};

//////////////////////////////////////////////////////////////////////
// Metody �ablony CPooledList

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
template <class Element, int nStaticCopyNumber> 
inline CPooledList<Element, nStaticCopyNumber>::CPooledList () 
{
	// zne�kodn� data objektu
	m_pFirst = NULL;
}

// destruktor
template <class Element, int nStaticCopyNumber> 
inline CPooledList<Element, nStaticCopyNumber>::~CPooledList () 
{
	// zkontroluje ne�kodn� data objektu
	ASSERT ( m_pFirst == NULL );
}

//////////////////////////////////////////////////////////////////////
// Operace se seznamem
//////////////////////////////////////////////////////////////////////

// zjist�, je-li seznam od pozice "position" pr�zdn�
template <class Element, int nStaticCopyNumber> 
inline BOOL CPooledList<Element, nStaticCopyNumber>::IsEmpty ( POSITION position ) 
{
	ASSERT ( position != NULL );

	// vr�t� p��znak, je-li "position" za posledn�m prvkem seznamu
	return ( *(struct SListMember **)position == NULL );
}

// vypln� "cElement" dal��m prvkem seznamu na pozici "rPosition" (FALSE=konec seznamu)
template <class Element, int nStaticCopyNumber> 
inline BOOL CPooledList<Element, nStaticCopyNumber>::GetNext ( POSITION &rPosition, 
	Element &cElement ) 
{
	ASSERT ( rPosition != NULL );

	// zjist�, je-li "rPosition" za posledn�m prvkem seznamu
	if ( *(struct SListMember **)rPosition == NULL )
	{	// "rPosition" je za posledn�m prvkem seznamu
		// vr�t� p��znak konce seznamu
		return FALSE;
	}

	// vypln� hodnotu prvku seznamu
	cElement = (*(struct SListMember **)rPosition)->cElement;
	// posune iter�tor v seznamu
	rPosition = (POSITION)&(*(struct SListMember **)rPosition)->pNext;
	// vr�t� p��znak platn�ho prvku seznamu
	return TRUE;
}

// vypln� "pElement" ukazatelem na dal�� prvek seznamu na pozici "rPosition" 
//		(FALSE=konec seznamu)
template <class Element, int nStaticCopyNumber> 
inline BOOL CPooledList<Element, nStaticCopyNumber>::GetNext ( POSITION &rPosition, 
	Element *&pElement ) 
{
	ASSERT ( rPosition != NULL );

	// zjist�, je-li "rPosition" za posledn�m prvkem seznamu
	if ( *(struct SListMember **)rPosition == NULL )
	{	// "rPosition" je za posledn�m prvkem seznamu
		// vr�t� p��znak konce seznamu
		return FALSE;
	}

	// vypln� ukazatel na prvek seznamu
	pElement = &(*(struct SListMember **)rPosition)->cElement;
	// posune iter�tor v seznamu
	rPosition = (POSITION)&(*(struct SListMember **)rPosition)->pNext;
	// vr�t� p��znak platn�ho prvku seznamu
	return TRUE;
}

// p�esko�� prvek na pozici "rPosition" (FALSE=konec seznamu)
template <class Element, int nStaticCopyNumber> 
inline BOOL CPooledList<Element, nStaticCopyNumber>::GetNext ( POSITION &rPosition ) 
{
	ASSERT ( rPosition != NULL );

	// zjist�, je-li "rPosition" za posledn�m prvkem seznamu
	if ( *(struct SListMember **)rPosition == NULL )
	{	// "rPosition" je za posledn�m prvkem seznamu
		// vr�t� p��znak konce seznamu
		return FALSE;
	}

	// posune iter�tor v seznamu
	rPosition = (POSITION)&(*(struct SListMember **)rPosition)->pNext;
	// vr�t� p��znak platn�ho prvku seznamu
	return TRUE;
}

// najde prvek "cElement" v seznamu a vr�t� jej� pozici (NULL=nenalezeno)
template <class Element, int nStaticCopyNumber> 
POSITION CPooledList<Element, nStaticCopyNumber>::Find ( Element cElement ) const 
{
	// ukazatel na odkaz na prvek seznamu
	struct SListMember * const *pMember = &m_pFirst;

	// projede seznam
	while ( *pMember != NULL )
	{
		// zjist�, jedn�-li se o hledan� prvek
		if ( (*pMember)->cElement == cElement )
		{	// jedn� se o hledan� prvek
			return (POSITION)pMember;
		}

		// posune ukazatel na dal�� prvek seznamu
		pMember = &(*pMember)->pNext;
	}
	// prvek nebyl nalezen
	return NULL;
}

// vr�t� po�et prvk� v seznamu
template <class Element, int nStaticCopyNumber> 
DWORD CPooledList<Element, nStaticCopyNumber>::GetSize () const 
{
	// ukazatel na odkaz na prvek seznamu
	struct SListMember * const *pMember = &m_pFirst;

	// spo��t� po�et prvk� v seznamu
	for ( DWORD dwMemberCount = 0; *pMember != NULL; dwMemberCount++ )
	{
		// posune ukazatel na dal�� prvek seznamu
		pMember = &(*pMember)->pNext;
	}

	// vr�t� po�et prvk� v seznamu
	return dwMemberCount;
}

// vyjme prvn� prvek seznamu a vr�t� jeho hodnotu
template <class Element, int nStaticCopyNumber> 
inline Element CPooledList<Element, nStaticCopyNumber>::RemoveFirst () 
{
	ASSERT ( m_pFirst != NULL );

	// uschov� si ukazatel na prvn� prvek seznamu
	struct SListMember *pMember = m_pFirst;

	// vypoj� prvek ze seznamu
	m_pFirst = pMember->pNext;
	// zjist� hodnotu prvku seznamu
	Element cElement = pMember->cElement;

	// sma�e prvek seznamu
	m_cPool.Free ( pMember );

	// vr�t� hodnotu mazan�ho prvku seznamu
	return cElement;
}

// vyjme za seznamu prvn� prvek "cElement" (FALSE=seznam byl pr�zdn�, prvek nebyl vyjmut)
template <class Element, int nStaticCopyNumber> 
inline BOOL CPooledList<Element, nStaticCopyNumber>::RemoveFirst ( Element &cElement ) 
{
	// zjist�, je-li seznam pr�zdn�
	if ( m_pFirst == NULL )
	{	// seznam je pr�zdn�
		// vr�t� p��znak pr�zdn� ho seznamu
		return FALSE;
	}

	// uschov� si ukazatel na prvn� prvek seznamu
	struct SListMember *pMember = m_pFirst;

	// vypoj� prvek ze seznamu
	m_pFirst = pMember->pNext;

	// vypln� hodnotu prvku seznamu
	cElement = pMember->cElement;
	// sma�e prvek seznamu
	m_cPool.Free ( pMember );

	// vr�t� p��znak platn�ho prvku seznamu
	return TRUE;
}

// p�id� do seznamu prvek "cElement"
template <class Element, int nStaticCopyNumber> 
inline void CPooledList<Element, nStaticCopyNumber>::Add ( Element cElement ) 
{
	// alokuje nov� prvek seznamu
	struct SListMember *pNewMember = m_cPool.Allocate ();
	// inicializuje nov� alokovan� prvek seznamu
	(*pNewMember).pNext = m_pFirst;
	(*pNewMember).cElement = cElement;

	// aktualizuje ukazatel na prvn� prvek seznamu
	m_pFirst = pNewMember;
}

// vlo�� do seznamu prvek "cElement" na pozici "position"
template <class Element, int nStaticCopyNumber> 
inline void CPooledList<Element, nStaticCopyNumber>::Insert ( POSITION position, 
	Element cElement ) 
{
	ASSERT ( position != NULL );

	// alokuje nov� prvek seznamu
	struct SListMember *pNewMember = m_cPool.Allocate ();
	// inicializuje nov� alokovan� prvek seznamu
	(*pNewMember).pNext = *(struct SListMember **)position;
	(*pNewMember).cElement = cElement;

	// zapoj� prvek do seznamu
	*(struct SListMember **)position = pNewMember;
}

// sma�e prvek seznamu "position" a vr�t� jeho hodnotu ("position" je platn� a odkazuje 
//		na dal�� prvek seznamu)
template <class Element, int nStaticCopyNumber> 
inline Element CPooledList<Element, nStaticCopyNumber>::RemovePosition ( 
	POSITION position ) 
{
	ASSERT ( position != NULL );
	ASSERT ( *(struct SListMember **)position != NULL );

	// uschov� si ukazatel na prvek seznamu
	struct SListMember *pMember = *(struct SListMember **)position;

	// vypoj� prvek ze seznamu
	*(struct SListMember **)position = pMember->pNext;
	// zjist� hodnotu prvku seznamu
	Element cElement = pMember->cElement;

	// sma�e prvek seznamu
	m_cPool.Free ( pMember );

	// vr�t� hodnotu mazan�ho prvku seznamu
	return cElement;
}

// p�esune prvek seznamu "position" na za��tek seznamu
template <class Element, int nStaticCopyNumber> 
inline void CPooledList<Element, nStaticCopyNumber>::MoveFirst ( POSITION position ) 
{
	ASSERT ( position != NULL );
	ASSERT ( *(struct SListMember **)position != NULL );

	// uschov� si ukazatel na prvek seznamu
	struct SListMember *pMember = *(struct SListMember **)position;

	// vypoj� prvek ze seznamu
	*(struct SListMember **)position = pMember->pNext;

	// p�id� vyjmut� prvek ze seznamu na za��tek seznamu
	pMember->pNext = m_pFirst;
	m_pFirst = pMember;
}

// p�esune prvek seznamu "position" do seznamu "cList"
template <class Element, int nStaticCopyNumber> 
inline void CPooledList<Element, nStaticCopyNumber>::Move ( POSITION position, 
	CPooledList<Element, nStaticCopyNumber> &cList ) 
{
	ASSERT ( position != NULL );
	ASSERT ( *(struct SListMember **)position != NULL );

	// uschov� si ukazatel na prvek seznamu
	struct SListMember *pMember = *(struct SListMember **)position;

	// vypoj� prvek ze seznamu
	*(struct SListMember **)position = pMember->pNext;

	// p�id� vyjmut� prvek ze seznamu do seznamu "cList"
	pMember->pNext = cList.m_pFirst;
	cList.m_pFirst = pMember;
}

// p�esune seznam do seznamu "cList"
template <class Element, int nStaticCopyNumber> 
inline void CPooledList<Element, nStaticCopyNumber>::MoveList ( 
	CPooledList<Element, nStaticCopyNumber> &cList ) 
{
	ASSERT ( cList.IsEmpty () );

	// p�epoj� prvky seznamu pod seznam "cList"
	cList.m_pFirst = m_pFirst;
	// sma�e seznam
	m_pFirst = NULL;
}

// vyjme ze seznamu v�echny prvky
template <class Element, int nStaticCopyNumber> 
void CPooledList<Element, nStaticCopyNumber>::RemoveAll () 
{
	// uschov� si ukazatel na prvn� prvek seznamu
	struct SListMember *pMember = m_pFirst;

	// sma�e prvky seznamu
	while ( pMember != NULL )
	{
		// uschov� si ukazatel na mazan� prvek seznamu
		struct SListMember *pDeletedMember = pMember;

		// posune ukazatel na dal�� prvek seznamu
		pMember = pMember->pNext;

		// sma�e prvek seznamu
		m_cPool.Free ( pDeletedMember );
	}

	// aktualizuje ukazatel na prvn� prvek seznamu
	m_pFirst = NULL;
}

#endif //__POOLED_LIST__HEADER_INCLUDED__
