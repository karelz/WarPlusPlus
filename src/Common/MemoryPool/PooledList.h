/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Abstraktní datové typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Šablona poolovaného seznamu
 * 
 ***********************************************************/

#ifndef __POOLED_LIST__HEADER_INCLUDED__
#define __POOLED_LIST__HEADER_INCLUDED__

#include "TypedMemoryPool.h"

//////////////////////////////////////////////////////////////////////
// Šablona tøídy poolovaného seznamu
template <class Element, int nStaticCopyNumber> 
class CPooledList 
{
// Datové typy
private:
	// prvek seznamu
	struct SListMember 
	{
		// ukazatel na další prvek seznamu
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

	// zjistí, je-li seznam prázdný
	BOOL IsEmpty () const { return ( m_pFirst == NULL ); };
	// zjistí, je-li seznam od pozice "position" prázdný
	static inline BOOL IsEmpty ( POSITION position );
	// vrátí pozici prvního prvku seznamu
	POSITION GetHeadPosition () const { return (POSITION)&m_pFirst; };
	// vyplní "cElement" dalším prvkem seznamu na pozici "rPosition" (FALSE=konec seznamu)
	static inline BOOL GetNext ( POSITION &rPosition, Element &cElement );
	// vyplní "pElement" ukazatelem na další prvek seznamu na pozici "rPosition" 
	//		(FALSE=konec seznamu)
	static inline BOOL GetNext ( POSITION &rPosition, Element *&pElement );
	// pøeskoèí prvek seznamu na pozici "rPosition" (FALSE=konec seznamu)
	static inline BOOL GetNext ( POSITION &rPosition );
	// najde prvek "cElement" v seznamu a vrátí jeho pozici (NULL=nenalezeno)
	POSITION Find ( Element cElement ) const;
	// vrátí poèet prvkù v seznamu
	DWORD GetSize () const;

	// vyjme první prvek seznamu a vrátí jeho hodnotu
	inline Element RemoveFirst ();
	// vyjme za seznamu první prvek "cElement" (FALSE=seznam byl prázdná, prvek nebyl 
	//		vyjmut)
	inline BOOL RemoveFirst ( Element &cElement );
	// pøidá do seznamu prvek "cElement"
	inline void Add ( Element cElement );
	// vloží do seznamu prvek "cElement" na pozici "position"
	static inline void Insert ( POSITION position, Element cElement );
	// smaže prvek seznamu "position" a vrátí jeho hodnotu ("position" je platná a 
	//		odkazuje na další prvek seznamu)
	static inline Element RemovePosition ( POSITION position );
	// pøesune prvek seznamu "position" na zaèátek seznamu
	inline void MoveFirst ( POSITION position );
	// pøesune prvek seznamu "position" do seznamu "cList"
	static inline void Move ( POSITION position, 
		CPooledList<Element, nStaticCopyNumber> &cList );
	// pøesune seznam do seznamu "cList"
	inline void MoveList ( CPooledList<Element, nStaticCopyNumber> &cList );
	// vyjme ze seznamu všechny prvky
	void RemoveAll ();

// Data
protected:
	// první prvek seznamu
	struct SListMember *m_pFirst;
	// memory pool prvkù seznamu
	static CTypedMemoryPool<struct SListMember> m_cPool;
};

//////////////////////////////////////////////////////////////////////
// Metody šablony CPooledList

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
template <class Element, int nStaticCopyNumber> 
inline CPooledList<Element, nStaticCopyNumber>::CPooledList () 
{
	// zneškodní data objektu
	m_pFirst = NULL;
}

// destruktor
template <class Element, int nStaticCopyNumber> 
inline CPooledList<Element, nStaticCopyNumber>::~CPooledList () 
{
	// zkontroluje neškodná data objektu
	ASSERT ( m_pFirst == NULL );
}

//////////////////////////////////////////////////////////////////////
// Operace se seznamem
//////////////////////////////////////////////////////////////////////

// zjistí, je-li seznam od pozice "position" prázdný
template <class Element, int nStaticCopyNumber> 
inline BOOL CPooledList<Element, nStaticCopyNumber>::IsEmpty ( POSITION position ) 
{
	ASSERT ( position != NULL );

	// vrátí pøíznak, je-li "position" za posledním prvkem seznamu
	return ( *(struct SListMember **)position == NULL );
}

// vyplní "cElement" dalším prvkem seznamu na pozici "rPosition" (FALSE=konec seznamu)
template <class Element, int nStaticCopyNumber> 
inline BOOL CPooledList<Element, nStaticCopyNumber>::GetNext ( POSITION &rPosition, 
	Element &cElement ) 
{
	ASSERT ( rPosition != NULL );

	// zjistí, je-li "rPosition" za posledním prvkem seznamu
	if ( *(struct SListMember **)rPosition == NULL )
	{	// "rPosition" je za posledním prvkem seznamu
		// vrátí pøíznak konce seznamu
		return FALSE;
	}

	// vyplní hodnotu prvku seznamu
	cElement = (*(struct SListMember **)rPosition)->cElement;
	// posune iterátor v seznamu
	rPosition = (POSITION)&(*(struct SListMember **)rPosition)->pNext;
	// vrátí pøíznak platného prvku seznamu
	return TRUE;
}

// vyplní "pElement" ukazatelem na další prvek seznamu na pozici "rPosition" 
//		(FALSE=konec seznamu)
template <class Element, int nStaticCopyNumber> 
inline BOOL CPooledList<Element, nStaticCopyNumber>::GetNext ( POSITION &rPosition, 
	Element *&pElement ) 
{
	ASSERT ( rPosition != NULL );

	// zjistí, je-li "rPosition" za posledním prvkem seznamu
	if ( *(struct SListMember **)rPosition == NULL )
	{	// "rPosition" je za posledním prvkem seznamu
		// vrátí pøíznak konce seznamu
		return FALSE;
	}

	// vyplní ukazatel na prvek seznamu
	pElement = &(*(struct SListMember **)rPosition)->cElement;
	// posune iterátor v seznamu
	rPosition = (POSITION)&(*(struct SListMember **)rPosition)->pNext;
	// vrátí pøíznak platného prvku seznamu
	return TRUE;
}

// pøeskoèí prvek na pozici "rPosition" (FALSE=konec seznamu)
template <class Element, int nStaticCopyNumber> 
inline BOOL CPooledList<Element, nStaticCopyNumber>::GetNext ( POSITION &rPosition ) 
{
	ASSERT ( rPosition != NULL );

	// zjistí, je-li "rPosition" za posledním prvkem seznamu
	if ( *(struct SListMember **)rPosition == NULL )
	{	// "rPosition" je za posledním prvkem seznamu
		// vrátí pøíznak konce seznamu
		return FALSE;
	}

	// posune iterátor v seznamu
	rPosition = (POSITION)&(*(struct SListMember **)rPosition)->pNext;
	// vrátí pøíznak platného prvku seznamu
	return TRUE;
}

// najde prvek "cElement" v seznamu a vrátí její pozici (NULL=nenalezeno)
template <class Element, int nStaticCopyNumber> 
POSITION CPooledList<Element, nStaticCopyNumber>::Find ( Element cElement ) const 
{
	// ukazatel na odkaz na prvek seznamu
	struct SListMember * const *pMember = &m_pFirst;

	// projede seznam
	while ( *pMember != NULL )
	{
		// zjistí, jedná-li se o hledaný prvek
		if ( (*pMember)->cElement == cElement )
		{	// jedná se o hledaný prvek
			return (POSITION)pMember;
		}

		// posune ukazatel na další prvek seznamu
		pMember = &(*pMember)->pNext;
	}
	// prvek nebyl nalezen
	return NULL;
}

// vrátí poèet prvkù v seznamu
template <class Element, int nStaticCopyNumber> 
DWORD CPooledList<Element, nStaticCopyNumber>::GetSize () const 
{
	// ukazatel na odkaz na prvek seznamu
	struct SListMember * const *pMember = &m_pFirst;

	// spoèítá poèet prvkù v seznamu
	for ( DWORD dwMemberCount = 0; *pMember != NULL; dwMemberCount++ )
	{
		// posune ukazatel na další prvek seznamu
		pMember = &(*pMember)->pNext;
	}

	// vrátí poèet prvkù v seznamu
	return dwMemberCount;
}

// vyjme první prvek seznamu a vrátí jeho hodnotu
template <class Element, int nStaticCopyNumber> 
inline Element CPooledList<Element, nStaticCopyNumber>::RemoveFirst () 
{
	ASSERT ( m_pFirst != NULL );

	// uschová si ukazatel na první prvek seznamu
	struct SListMember *pMember = m_pFirst;

	// vypojí prvek ze seznamu
	m_pFirst = pMember->pNext;
	// zjistí hodnotu prvku seznamu
	Element cElement = pMember->cElement;

	// smaže prvek seznamu
	m_cPool.Free ( pMember );

	// vrátí hodnotu mazaného prvku seznamu
	return cElement;
}

// vyjme za seznamu první prvek "cElement" (FALSE=seznam byl prázdný, prvek nebyl vyjmut)
template <class Element, int nStaticCopyNumber> 
inline BOOL CPooledList<Element, nStaticCopyNumber>::RemoveFirst ( Element &cElement ) 
{
	// zjistí, je-li seznam prázdný
	if ( m_pFirst == NULL )
	{	// seznam je prázdná
		// vrátí pøíznak prázdné ho seznamu
		return FALSE;
	}

	// uschová si ukazatel na první prvek seznamu
	struct SListMember *pMember = m_pFirst;

	// vypojí prvek ze seznamu
	m_pFirst = pMember->pNext;

	// vyplní hodnotu prvku seznamu
	cElement = pMember->cElement;
	// smaže prvek seznamu
	m_cPool.Free ( pMember );

	// vrátí pøíznak platného prvku seznamu
	return TRUE;
}

// pøidá do seznamu prvek "cElement"
template <class Element, int nStaticCopyNumber> 
inline void CPooledList<Element, nStaticCopyNumber>::Add ( Element cElement ) 
{
	// alokuje nový prvek seznamu
	struct SListMember *pNewMember = m_cPool.Allocate ();
	// inicializuje novì alokovaný prvek seznamu
	(*pNewMember).pNext = m_pFirst;
	(*pNewMember).cElement = cElement;

	// aktualizuje ukazatel na první prvek seznamu
	m_pFirst = pNewMember;
}

// vloží do seznamu prvek "cElement" na pozici "position"
template <class Element, int nStaticCopyNumber> 
inline void CPooledList<Element, nStaticCopyNumber>::Insert ( POSITION position, 
	Element cElement ) 
{
	ASSERT ( position != NULL );

	// alokuje nový prvek seznamu
	struct SListMember *pNewMember = m_cPool.Allocate ();
	// inicializuje novì alokovaný prvek seznamu
	(*pNewMember).pNext = *(struct SListMember **)position;
	(*pNewMember).cElement = cElement;

	// zapojí prvek do seznamu
	*(struct SListMember **)position = pNewMember;
}

// smaže prvek seznamu "position" a vrátí jeho hodnotu ("position" je platná a odkazuje 
//		na další prvek seznamu)
template <class Element, int nStaticCopyNumber> 
inline Element CPooledList<Element, nStaticCopyNumber>::RemovePosition ( 
	POSITION position ) 
{
	ASSERT ( position != NULL );
	ASSERT ( *(struct SListMember **)position != NULL );

	// uschová si ukazatel na prvek seznamu
	struct SListMember *pMember = *(struct SListMember **)position;

	// vypojí prvek ze seznamu
	*(struct SListMember **)position = pMember->pNext;
	// zjistí hodnotu prvku seznamu
	Element cElement = pMember->cElement;

	// smaže prvek seznamu
	m_cPool.Free ( pMember );

	// vrátí hodnotu mazaného prvku seznamu
	return cElement;
}

// pøesune prvek seznamu "position" na zaèátek seznamu
template <class Element, int nStaticCopyNumber> 
inline void CPooledList<Element, nStaticCopyNumber>::MoveFirst ( POSITION position ) 
{
	ASSERT ( position != NULL );
	ASSERT ( *(struct SListMember **)position != NULL );

	// uschová si ukazatel na prvek seznamu
	struct SListMember *pMember = *(struct SListMember **)position;

	// vypojí prvek ze seznamu
	*(struct SListMember **)position = pMember->pNext;

	// pøidá vyjmutý prvek ze seznamu na zaèátek seznamu
	pMember->pNext = m_pFirst;
	m_pFirst = pMember;
}

// pøesune prvek seznamu "position" do seznamu "cList"
template <class Element, int nStaticCopyNumber> 
inline void CPooledList<Element, nStaticCopyNumber>::Move ( POSITION position, 
	CPooledList<Element, nStaticCopyNumber> &cList ) 
{
	ASSERT ( position != NULL );
	ASSERT ( *(struct SListMember **)position != NULL );

	// uschová si ukazatel na prvek seznamu
	struct SListMember *pMember = *(struct SListMember **)position;

	// vypojí prvek ze seznamu
	*(struct SListMember **)position = pMember->pNext;

	// pøidá vyjmutý prvek ze seznamu do seznamu "cList"
	pMember->pNext = cList.m_pFirst;
	cList.m_pFirst = pMember;
}

// pøesune seznam do seznamu "cList"
template <class Element, int nStaticCopyNumber> 
inline void CPooledList<Element, nStaticCopyNumber>::MoveList ( 
	CPooledList<Element, nStaticCopyNumber> &cList ) 
{
	ASSERT ( cList.IsEmpty () );

	// pøepojí prvky seznamu pod seznam "cList"
	cList.m_pFirst = m_pFirst;
	// smaže seznam
	m_pFirst = NULL;
}

// vyjme ze seznamu všechny prvky
template <class Element, int nStaticCopyNumber> 
void CPooledList<Element, nStaticCopyNumber>::RemoveAll () 
{
	// uschová si ukazatel na první prvek seznamu
	struct SListMember *pMember = m_pFirst;

	// smaže prvky seznamu
	while ( pMember != NULL )
	{
		// uschová si ukazatel na mazaný prvek seznamu
		struct SListMember *pDeletedMember = pMember;

		// posune ukazatel na další prvek seznamu
		pMember = pMember->pNext;

		// smaže prvek seznamu
		m_cPool.Free ( pDeletedMember );
	}

	// aktualizuje ukazatel na první prvek seznamu
	m_pFirst = NULL;
}

#endif //__POOLED_LIST__HEADER_INCLUDED__
