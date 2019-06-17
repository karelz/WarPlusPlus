// UnitCache.cpp: implementation of the CUnitCache class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UnitCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUnitCache::CUnitCache()
{
  m_pUnits = NULL;
}

CUnitCache::~CUnitCache()
{
  ASSERT(m_pUnits == NULL);
  ASSERT(m_Map.IsEmpty());
}

void CUnitCache::Create()
{
  // Init the hashing table of all units
  m_Map.InitHashTable(103);
}

void CUnitCache::Delete()
{
  CCUnit *pUnit, *pDelUnit;

  // Delete all units in the cache
  for(pUnit = m_pUnits; pUnit != NULL;){
    pDelUnit = pUnit;
    pUnit = pUnit->m_pUnitCacheNext;
    pDelUnit->Delete();
    delete pDelUnit;
  }

  // and clear cache structures
  m_pUnits = NULL;
  m_Map.RemoveAll();
}

#ifdef _DEBUG

void CUnitCache::AssertValid() const
{
	CObject::AssertValid();
}

void CUnitCache::Dump(CDumpContext &dc) const
{
	CObject::Dump(dc);
}

#endif

void CUnitCache::AddUnit(CCUnit *pUnit)
{
  ASSERT_VALID(this);
  ASSERT(pUnit != NULL);
  ASSERT_VALID(pUnit);

  // Lock us
  VERIFY(m_lockCache.Lock());

  // add the unit to the list of all units
  pUnit->m_pUnitCacheNext = m_pUnits;
  if(m_pUnits != NULL) m_pUnits->m_pUnitCachePrev = pUnit;
  pUnit->m_pUnitCachePrev = NULL;
  m_pUnits = pUnit;

  // add the unit to the hash table
  m_Map.SetAt(pUnit->GetID(), pUnit);

  // Unlock us
  VERIFY(m_lockCache.Unlock());
}

void CUnitCache::RemoveUnit(CCUnit *pUnit)
{
  ASSERT_VALID(this);
  ASSERT(pUnit != NULL);
  ASSERT_VALID(pUnit);

  // Lock us
  VERIFY(m_lockCache.Lock());

  // Test if the unit really has reference count == 0
  // If not -> leave it
  if(pUnit->m_dwReferenceCount > 0){
    VERIFY(m_lockCache.Unlock());
    return;
  }

  // remove the unit from the hash table
  m_Map.RemoveKey(pUnit->GetID());

  // remove the unit from the list
  if(pUnit->m_pUnitCachePrev != NULL)
    pUnit->m_pUnitCachePrev->m_pUnitCacheNext = pUnit->m_pUnitCacheNext;
  if(pUnit->m_pUnitCacheNext != NULL)
    pUnit->m_pUnitCacheNext->m_pUnitCachePrev = pUnit->m_pUnitCachePrev;
  if(m_pUnits == pUnit)
    m_pUnits = pUnit->m_pUnitCacheNext;

  // delete the unit
  ASSERT(pUnit->m_dwReferenceCount == 0);
  pUnit->Delete();
  delete pUnit;

  // Unlock us
  VERIFY(m_lockCache.Unlock());
}

CCUnit * CUnitCache::OpenUnits()
{
  // Lock us
  VERIFY(m_lockCache.Lock());

  // return the first unit
  return m_pUnits;
}

void CUnitCache::CloseUnits()
{
  VERIFY(m_lockCache.Unlock());
}