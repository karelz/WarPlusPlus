// UnitCache.h: interface for the CUnitCache class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNITCACHE_H__C4CF22E3_09E8_11D4_802E_0000B4A08F9A__INCLUDED_)
#define AFX_UNITCACHE_H__C4CF22E3_09E8_11D4_802E_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CUnit.h"

class CUnitCache : public CObject
{
// Konstrukce, destrukce
public:
	// Konstruktor
	CUnitCache();

	// Destruktor
	virtual ~CUnitCache();

// Metody
public:
	// Pocatecni inicializace
	void Create();
  virtual void Delete();

  // returns unit by ID
  // !!! It AddRefs the returned unit
  CCUnit *GetUnit(DWORD dwUnitID){ ASSERT_VALID(this); CCUnit *pUnit;
    VERIFY ( m_lockCache.Lock () );
    if ( !m_Map.Lookup ( dwUnitID, pUnit ) )
    {
      VERIFY ( m_lockCache.Unlock () );
      return NULL;
    }
    ASSERT_VALID ( pUnit );
    pUnit->AddRef ();
    VERIFY ( m_lockCache.Unlock () );
    return pUnit;
  }

  // Adds unit
  void AddUnit(CCUnit *pUnit);
  // Removes the unit
  void RemoveUnit(CCUnit *pUnit);

  // Returns first unit in the cache
  // !!! After you call this, you MUST call CloseUnits
  // (it locks the cache... ;-) )
  CCUnit *OpenUnits();
  // returns next unit in the cache (must be called after OpenUnits and before CloseUnits)
  CCUnit *GetNextUnit(CCUnit *pUnit){ ASSERT_VALID(pUnit); return pUnit->m_pUnitCacheNext; }
  // closes the list of units
  void CloseUnits();

// debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// Data
private:
  // map
  CMap<DWORD, DWORD, CCUnit *, CCUnit *&> m_Map;
  // cache lock
  CMutex m_lockCache;

  // list of all units
  CCUnit *m_pUnits;
};

#endif // !defined(AFX_UNITCACHE_H__C4CF22E3_09E8_11D4_802E_0000B4A08F9A__INCLUDED_)
