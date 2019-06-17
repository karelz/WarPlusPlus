// CachedObjectHandle.cpp: implementation of the CCachedObjectHandle class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CachedObjectHandle.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCachedObjectHandle::CCachedObjectHandle(CCachedObject *pCachedObject)
{
	m_pCachedObject=pCachedObject;
	m_pNextLoad=(CCachedObjectHandle *)NULL;
	m_pNextHandle=(CCachedObjectHandle *)NULL;
	m_pPrevHandle=(CCachedObjectHandle *)NULL;
	ResetHits();
}

CCachedObjectHandle::~CCachedObjectHandle()
{
    m_pCachedObject=NULL;
}

void CCachedObjectHandle::Load()
{
	if(m_pCachedObject) {
		m_pCachedObject->LoadAndLock();
		m_pCachedObject->Unlock();
	}
}

void CCachedObjectHandle::LoadAfterBeingWriteLocked()
{
	if(m_pCachedObject) {
		m_pCachedObject->LoadAfterBeingWriteLocked();
		m_pCachedObject->Unlock();
	}
}

void CCachedObjectHandle::LockWrite()
{
	if(m_pCachedObject) {
		m_pCachedObject->LockWrite();
    }
}