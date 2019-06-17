// UnitTypeCache.h: interface for the CUnitTypeCache class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNITTYPECACHE_H__C4CF22E3_09E8_11D4_802E_0000B4A08F9A__INCLUDED_)
#define AFX_UNITTYPECACHE_H__C4CF22E3_09E8_11D4_802E_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Cache/IDAbstractCache.h"
#include "../DataObjects/CUnitType.h"

class CUnitTypeCache : public CIDAbstractCache
{
// Konstrukce, destrukce
public:
	// Konstruktor
	CUnitTypeCache();

	// Destruktor
	virtual ~CUnitTypeCache();

// Konstanty
public:
	enum EUnitTypeCacheSizes {
		UnitTypeCacheMaxSize=400000,
		UnitTypeCachePreferredSize=250000
	};

// Metody
public:
	// Pocatecni inicializace
	void Create(CCacheLoadQueue *pQueue=NULL);

	CCUnitType *GetType(DWORD dwID) {
		return (CCUnitType *)GetObject(dwID);
	}

// debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// Data
private:
};

#endif // !defined(AFX_UNITTYPECACHE_H__C4CF22E3_09E8_11D4_802E_0000B4A08F9A__INCLUDED_)
