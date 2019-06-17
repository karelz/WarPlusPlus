#include "stdafx.h"
#include "MapexCache.h"

IMPLEMENT_DYNAMIC(CMapexCache, CObject);

// constructor
CMapexCache::CMapexCache() :
  CAbstractCache(40000000, 30000000)
{
}

// destructor
CMapexCache::~CMapexCache()
{
}

// Debug functions
#ifdef _DEBUG

void CMapexCache::AssertValid() const
{
  CAbstractCache::AssertValid();
}

void CMapexCache::Dump(CDumpContext &dc) const
{
  CAbstractCache::Dump(dc);
}

#endif