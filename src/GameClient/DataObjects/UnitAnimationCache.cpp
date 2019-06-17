// UnitAnimationCache.cpp: implementation of the CUnitAnimationCache class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UnitAnimationCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CUnitAnimationCache, CAbstractCache)

CUnitAnimationCache::CUnitAnimationCache()
{

}

CUnitAnimationCache::~CUnitAnimationCache()
{

}


#ifdef _DEBUG

void CUnitAnimationCache::AssertValid() const
{
  CAbstractCache::AssertValid();
}

void CUnitAnimationCache::Dump(CDumpContext &dc) const
{
  CAbstractCache::Dump(dc);
}

#endif

void CUnitAnimationCache::Create()
{
  CAbstractCache::Create(&m_LoadQueue, MaxCacheSize, PrefferedCacheSize);
  m_LoadQueue.RunQueue();
}

void CUnitAnimationCache::Delete()
{
}