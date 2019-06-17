// IDAbstractCache.cpp: implementation of the CIDAbstractCache class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDAbstractCache.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CIDAbstractCache, CAbstractCache);

CIDAbstractCache::CIDAbstractCache(DWORD dwMaxSize /* =10240 */, DWORD dwPreferredSize /* =5120 */) : CAbstractCache(dwMaxSize, dwPreferredSize)
{
}

CIDAbstractCache::CIDAbstractCache(CCacheLoadQueue *pQueue, DWORD dwMaxSize /* =10240 */, DWORD dwPreferredSize /* =5120 */) : CAbstractCache(pQueue, dwMaxSize, dwPreferredSize)
{
}

CIDAbstractCache::~CIDAbstractCache()
{
}

CIDCachedObject *CIDAbstractCache::GetObject(DWORD dwID)
{
	CCachedObjectHandle *pObj;

	m_RWLock.ReaderLock();
	
	if(((CIDCachedObject*)m_pObjects->GetCachedObject())->GetID()==dwID) {
		m_RWLock.ReaderUnlock();
		return (CIDCachedObject*)m_pObjects->GetCachedObject();
	}

	for(pObj=m_pObjects->NextHandle(); pObj!=m_pObjects; pObj=pObj->NextHandle()) {
		if(((CIDCachedObject*)pObj->GetCachedObject())->GetID()==dwID) {
			m_RWLock.ReaderUnlock();
			return (CIDCachedObject*)pObj->GetCachedObject();
		}
	}
	m_RWLock.ReaderUnlock();
	return NULL;
}