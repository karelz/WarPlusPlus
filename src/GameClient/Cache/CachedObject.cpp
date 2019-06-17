// CachedObject.cpp: implementation of the CCachedObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CachedObject.h"

#include "AbstractCache.h"

IMPLEMENT_DYNAMIC(CCachedObject, CObject);

CCachedObject::CCachedObject()
{
	m_bLoaded=FALSE;
	m_pAbstractCache=NULL;
	m_pInternalData=NULL;
}

CCachedObject::~CCachedObject()
{
	// V kazdem pripade po nas nesmi zustat data
	if(IsLoaded()) 
		Unload();
}

#ifdef _DEBUG

void CCachedObject::AssertValid() const
{
  CObject::AssertValid();
}

void CCachedObject::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
  dc << "CCachedObject " << (m_bLoaded?"(LOADED)":"(UNLOADED)") << ", " << (m_pAbstractCache?", member of cache: ":"not in cache");
  if(m_pAbstractCache) {
    m_pAbstractCache->Dump(dc);
  }
  dc << "\n";
}

#endif

void CCachedObject::HintDataNeeded()
{    
	if(m_pAbstractCache) {
		m_pAbstractCache->HintDataNeeded(this);
	}
}

void CCachedObject::HintDataNotNeeded()
{
	if(m_pAbstractCache) {
		m_pAbstractCache->HintDataNotNeeded(this);
	}
}

void CCachedObject::Lock()
{
	m_RWLock.ReaderLock();

	if(m_pAbstractCache) {
		// Notifikace pro cache, ze na objekt bylo pristoupeno
		m_pAbstractCache->NotifyObjectAccessed(this);
	}
}

void CCachedObject::LockWrite()
{
	m_RWLock.WriterLock();

	if(m_pAbstractCache) {
		// Notifikace pro cache, ze na objekt bylo pristoupeno
		m_pAbstractCache->NotifyObjectAccessed(this);
	}
}

BOOL CCachedObject::TestLock()
{
	BOOL success=m_RWLock.ReaderTestLock();

	if(success && m_pAbstractCache) {
		// Notifikace pro cache, ze na objekt bylo pristoupeno
		m_pAbstractCache->NotifyObjectAccessed(this);
	}

	return success;
}

BOOL CCachedObject::TestLockWrite()
{
	BOOL success=m_RWLock.WriterTestLock();

	if(success && m_pAbstractCache) {
		// Notifikace pro cache, ze na objekt bylo pristoupeno
		m_pAbstractCache->NotifyObjectAccessed(this);
	}

	return success;
}

BOOL CCachedObject::TestWriteLock()
{
	// Tuto metodu vola cache, takze neni treba ji notifikovat
	// ze na objekt bylo pristoupeno
	return m_RWLock.WriterTestLock();
}

void CCachedObject::Unlock()
{
	m_RWLock.ReaderUnlock();
}

void CCachedObject::UnlockWrite()
{
	m_RWLock.WriterUnlock();
}

void CCachedObject::WriteUnlock()
{
	// Tuto metodu vola cache, takze neni treba ji notifikovat
	// ze na objekt bylo pristoupeno
	m_RWLock.WriterUnlock();
}

void CCachedObject::LoadAndLock()
{
	// Zamkneme se na cteni
	Lock();

    if(IsLoaded() || m_pAbstractCache==NULL) { // Jestlize jsme nahrani tak staci skoncit
		return;
	}

	// Jenze jestlize nahrani nejsme, tak je treba se prezamknout
	m_RWLock.ReaderUnlockWriterLock();

    // Je zamceno na zapis, provedeme nahrani
    LoadAfterBeingWriteLocked();
}

void CCachedObject::LoadAfterBeingWriteLocked()
{
	// Nahrajeme objekt
	if(!IsLoaded() && m_pAbstractCache!=NULL) { // Testujeme pro jistotu
		// Nejprve rekneme cachi o nasem umyslu se nahrat		
		m_pAbstractCache->NotifyObjectLoad(this);

		// Odhad velikosti pred loadem
		DWORD dwPrevSize=GetSize();

		// A pak se nahrajeme
		Load();
		m_bLoaded=TRUE;

		// A pak rekneme cachi, ze jsme se nahrali
		m_pAbstractCache->NotifyObjectLoaded(this, dwPrevSize);
	}

	// Odemkneme na zapis, zamkneme na cteni
	m_RWLock.WriterUnlockReaderLock();
}

void CCachedObject::LoadAndLockWrite()
{
	// Zamkneme se na cteni
	LockWrite();

	if(IsLoaded() || m_pAbstractCache==NULL) { // Jestlize jsme nahrani tak staci skoncit
		return;
	}

	// Nahrajeme objekt
	if(!IsLoaded()) { // Testujeme pro jistotu
		// Nejprve rekneme cachi o nasem umyslu se nahrat		
		m_pAbstractCache->NotifyObjectLoad(this);

		// Odhad velikosti pred loadem
		DWORD dwPrevSize=GetSize();

		// A pak se nahrajeme
		Load();
		m_bLoaded=TRUE;

		// A pak rekneme cachi, ze jsme se nahrali
		m_pAbstractCache->NotifyObjectLoaded(this, dwPrevSize);
	}
}
