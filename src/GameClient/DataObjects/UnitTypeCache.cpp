// UnitTypeCache.cpp: implementation of the CUnitTypeCache class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UnitTypeCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUnitTypeCache::CUnitTypeCache() : CIDAbstractCache()
{
}

CUnitTypeCache::~CUnitTypeCache()
{
}

void CUnitTypeCache::Create(CCacheLoadQueue *pQueue)
{	
	CIDAbstractCache::Create(pQueue, EUnitTypeCacheSizes::UnitTypeCacheMaxSize, EUnitTypeCacheSizes::UnitTypeCachePreferredSize);
}

#ifdef _DEBUG

void CUnitTypeCache::AssertValid() const
{
	CAbstractCache::AssertValid();
}

void CUnitTypeCache::Dump(CDumpContext &dc) const
{
	CAbstractCache::Dump(dc);
}

#endif