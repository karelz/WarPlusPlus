/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Abstraktni datove typy
 *   Autor: Karel Zikmund
 * 
 *   Popis: Chytra prioritni fronta
 * 
 ***********************************************************/

#ifndef __SMART_PRIORITY_QUEUE__HEADER_INCLUDED__
#define __SMART_PRIORITY_QUEUE__HEADER_INCLUDED__

#include "TypedMemoryPool.h"

/////////////////////////////////////////////////////////////////////////////
// sablona na chytrou prioritni frontu
template <class Element>
class CSmartPriorityQueue
{
// konstruktor a destruktor
public:
  // konstruktor
  CSmartPriorityQueue(DWORD dwInitialSize, DWORD dwGrowBySize);
  // destruktor
  ~CSmartPriorityQueue();

// operace
public:
  // zatrideni prvku do fronty
  void Add(const Element &cElement, DWORD dwPriority);
  // odebrani nejmensiho prvku
  bool Remove(Element &cElement);
  // vyprazdneni cele fronty
  void Clear();
  // zjisti, jestli je fronta prazdna
  bool IsEmpty();

// interni datove struktury
protected:
  // prvek fronty (spojoveho seznamu)
  struct SQueueItem
  {
    // data 
    Element m_cElement;
    // ukazatel na dalsi prvek
    SQueueItem *m_pNext;
  };

  struct SPrioritySlot
  {
    // ukazatel na prvni prvek spojoveho seznamu
    SQueueItem *m_pFirst;
    // ukazatel na konec spojoveho seznamu
    SQueueItem **m_ppLast;
  };
  
// datove polozky
protected:
  // memory pool na polozky fronty
  CTypedMemoryPool<SQueueItem> m_cPool;
  // pole priorit
  SPrioritySlot *m_aPriorities;
  // velikost pole priorit
  DWORD m_dwPrioritiesSize;
  // priorita, ve ktere je ulozen nejmensi prvek
  DWORD m_dwFirstUsedPriority;
  // priorita, ve ktere je ulozen nejvetsi prvek
  DWORD m_dwLastUsedPriority;
  // velikost, o kterou ma pole rust
  DWORD m_dwGrowBySize;
};


/////////////////////////////////////////////////////////////////////////////
// konstruktor a destruktor

template <class Element>
CSmartPriorityQueue<Element>::CSmartPriorityQueue(DWORD dwInitialSize, DWORD dwGrowBySize) :
  m_cPool(1000)
{
  // naalokujeme pole
  m_aPriorities = (SPrioritySlot *) malloc(dwInitialSize * sizeof(SPrioritySlot));

  // kdyz se nepovedla se alokace, hodime exception
  if (m_aPriorities == NULL)
    ::AfxThrowMemoryException();

  // velikost si ulozime do member polozky
  m_dwPrioritiesSize = dwInitialSize;

  // inicializujeme pole
  for (DWORD i = 0; i < m_dwPrioritiesSize; i++)
  {
    // zapamatujeme si prvek pole
    SPrioritySlot &sSlot = m_aPriorities[i];
    // inicializujeme ukazatel na prvni a posledni prvek
    sSlot.m_pFirst = NULL;
    sSlot.m_ppLast = &(sSlot.m_pFirst);
  }

  // inicializujeme index priority, kde je ulozen nejmensi prvek
  m_dwFirstUsedPriority = m_dwPrioritiesSize;
  // inicializujeme index priority, kde je ulozen nejvetsi prvek
  m_dwLastUsedPriority = 0;

  // okopirujeme doporucenou velikost, o kolik ma pole rust v pripade,
  // ze je prilis male
  m_dwGrowBySize = dwGrowBySize;
}


template <class Element>
CSmartPriorityQueue<Element>::~CSmartPriorityQueue()
{
  // ujistime se, ze fronta je prazdna
  ASSERT(IsEmpty());

  // uvolnime pole
  free(m_aPriorities);
}


/////////////////////////////////////////////////////////////////////////////
// zatrideni prvku do fronty
template <class Element>
void CSmartPriorityQueue<Element>::Add(const Element &cElement, DWORD dwPriority)
{
  // zjistime, jestli je pole priorit dost velke
  if (dwPriority >= m_dwPrioritiesSize)
  {
    // neni, jdeme realokovat

    // nova velikost pole (v poctech prvku)
    DWORD dwNewSize = dwPriority + 1 + m_dwGrowBySize;

    m_aPriorities = (SPrioritySlot *)realloc(
      m_aPriorities, 
      dwNewSize * sizeof(SPrioritySlot));

    // povedla se realokace?
    if (m_aPriorities == NULL)
      ::AfxThrowMemoryException();

    // inicializujeme nove vyrobeny kus pole
    for (DWORD i = m_dwPrioritiesSize; i < dwNewSize; i++)
    {
      // zapamatujeme si prvek pole
      SPrioritySlot &sSlot = m_aPriorities[i];
      // inicializujeme ukazatel na prvni a posledni prvek
      sSlot.m_pFirst = NULL;
      sSlot.m_ppLast = &(sSlot.m_pFirst);
    }
    for ( i = 0; i < m_dwPrioritiesSize; i ++ )
    {
      // zapamatujeme si prvek pole
      SPrioritySlot &sSlot = m_aPriorities[i];
      // inicializujeme ukazatel na prvni a posledni prvek
      if ( sSlot.m_pFirst == NULL )
        sSlot.m_ppLast = &(sSlot.m_pFirst);
    }

    // kdyz byla fronta prazdna a tedy index prvniho neprazdneho
    // slotu byl "za koncem", tak ho tam zase posuneme
    if (m_dwFirstUsedPriority == m_dwPrioritiesSize)
      m_dwFirstUsedPriority = dwNewSize;

    // zapamatujeme si novou velikost pole
    m_dwPrioritiesSize = dwNewSize;

    // a to je vse...
  }

  // do spravne priority pridame prvek
  // zapamatujeme si pozici v poli
  SPrioritySlot &sSlot = m_aPriorities[dwPriority];

  // alokujeme novy prvek
  SQueueItem *pNew = m_cPool.Allocate();
  // okopirujeme do ni element
  pNew->m_cElement = cElement;
  // vynulujeme ukazatel na dalsi prvek
  pNew->m_pNext = NULL;

  // paranoia -- ukazatel na posledni prvek musi ukazovat na NULL
  ASSERT(*(sSlot.m_ppLast) == NULL);
  // zaradime novy prvek na konec spojaku
  *(sSlot.m_ppLast) = pNew;
  // posuneme ukazatel na posledni prvek, aby ukazoval do noveho prvku na m_pNext
  sSlot.m_ppLast = &(pNew->m_pNext);

  // upravime indexy nejnizsi a nejvyssi pouzite priority
  if (m_dwFirstUsedPriority > dwPriority)
    m_dwFirstUsedPriority = dwPriority;
  if (m_dwLastUsedPriority < dwPriority)
    m_dwLastUsedPriority = dwPriority;
}
 

/////////////////////////////////////////////////////////////////////////////
// odebrani nejmensiho prvku
template <class Element>
bool CSmartPriorityQueue<Element>::Remove(Element &cElement)
{
  // najdeme nejmensi prvek
  for ( ; m_dwFirstUsedPriority <= m_dwLastUsedPriority; m_dwFirstUsedPriority++)
  {
    // zapamatujeme si pozici v poli
    SPrioritySlot &sSlot = m_aPriorities[m_dwFirstUsedPriority];

    // kdyz je v teto priorite nejaky prvek, tak ho vratime
    if (sSlot.m_pFirst != NULL)
    {
      // okopirujeme element do navratove hodnoty
      cElement = sSlot.m_pFirst->m_cElement;

      // zapamatujeme si prvni prvek
      SQueueItem *pDel = sSlot.m_pFirst;
      // smazeme prvni prvek ze spojaku
      sSlot.m_pFirst = sSlot.m_pFirst->m_pNext;
      // osetrime konec spojaku
      if (sSlot.m_pFirst == NULL)
      {
        // ukazatel na posledni bude ukazovat na ukazatel na prvni
        sSlot.m_ppLast = &(sSlot.m_pFirst);
        // muzeme o jednicku posunout spodni mez pole priorit
        m_dwFirstUsedPriority++;
      }

      // fyzicky smazeme prvek
      m_cPool.Free(pDel);

      // paranoia -- odkaz na posledni musi stale ukazovat na NULL
      ASSERT(*(sSlot.m_ppLast) == NULL);

      // vratime uspech
      return true;
    }

    // paranoia -- odkaz na posledni musi ukazovat na m_pFirst
    ASSERT(sSlot.m_ppLast == &(sSlot.m_pFirst));
  }

  // nenasli jsme zadny prvek
  // nastavime aspon meze
  m_dwFirstUsedPriority = m_dwPrioritiesSize;
  m_dwLastUsedPriority = 0;
  
  // vratime neuspech
  return false;
}


/////////////////////////////////////////////////////////////////////////////
// vyprazdneni cele fronty
template <class Element>
void CSmartPriorityQueue<Element>::Clear()
{
  // zahodime cely obsah memory poolu
  m_cPool.FreeAll();
  
  // projdeme pole, vynulujeme jeho obsah
  for ( ; m_dwFirstUsedPriority <= m_dwLastUsedPriority; m_dwFirstUsedPriority++)
  {
    // zapamatujeme si pozici v poli
    SPrioritySlot &sSlot = m_aPriorities[m_dwFirstUsedPriority];
    sSlot.m_pFirst = NULL;
    sSlot.m_ppLast = &(sSlot.m_pFirst);
  }

#ifdef _DEBUG
  for ( DWORD i = 0; i < m_dwPrioritiesSize; i ++ )
  {
    ASSERT ( m_aPriorities [ i ].m_pFirst == NULL );
    ASSERT ( m_aPriorities [ i ].m_ppLast == &(m_aPriorities [ i ].m_pFirst ) );
  }
#endif

  // upravime meze v poli
  m_dwFirstUsedPriority = m_dwPrioritiesSize;
  m_dwLastUsedPriority = 0;
}


/*
// vyprazdneni cele fronty
// hehe, byl jsem snazivej a blbej a napsal jsem to puvodne poctive prvek po prvku...
// proste trubka...   :-(
template <class Element>
void CSmartPriorityQueue<Element>::Clear()
{
  for ( ; m_dwFirstUsedPriority <= m_dwLastUsedPriority; m_dwFirstUsedPriority++)
  {
    // zapamatujeme si pozici v poli
    SPrioritySlot &sSlot = m_aPriorities[m_dwFirstUsedPriority];
    // dokud je neco ve spojaku teto priority
    while (sSlot.m_pFirst != NULL)
    {
      // zapamatujeme si prvni prvek
      SQueueItem *pDel = sSlot.m_pFirst;
      // vypojime prvni prvek ze spojaku
      sSlot.m_pFirst = sSlot.m_pFirst->m_pNext;
      // smazeme zapamatovany prvek
      m_cPool.Free(pDel);
    }
    // upravime ukazatel na konec spojaku
    sSlot.m_ppLast = &(sSlot.m_pFirst);
  }
  
  // upravime meze v poli
  m_dwFirstUsedPriority = m_dwPrioritiesSize;
  m_dwLastUsedPriority = 0;
}
*/
 

/////////////////////////////////////////////////////////////////////////////
// zjisteni, je-li je fronta prazdna
template <class Element>
bool CSmartPriorityQueue<Element>::IsEmpty()
{
  // najdeme prvni obsazenou prioritu
  for ( ; m_dwFirstUsedPriority <= m_dwLastUsedPriority; m_dwFirstUsedPriority++)
  {
    if (m_aPriorities[m_dwFirstUsedPriority].m_pFirst != NULL)
      break;
  }

  // kdyz jsme dosli az za nejvetsi obsazenou prioritu, tak je fronta prazdna
  if (m_dwFirstUsedPriority > m_dwLastUsedPriority)
  {
    m_dwFirstUsedPriority = m_dwPrioritiesSize;
    m_dwLastUsedPriority = 0;
    return true;
  }
  else
  {
    // fronta neni prazdna
    return false;
  }
}


#endif __SMART_PRIORITY_QUEUE__HEADER_INCLUDED__
