// CachedObject.h: interface for the CCachedObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CACHEDOBJECT_H__87AA8E67_F3AB_11D3_BFF9_0000B4A08F9A__INCLUDED_)
#define AFX_CACHEDOBJECT_H__87AA8E67_F3AB_11D3_BFF9_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CacheRWLock.h"

class CAbstractCache;

class CCachedObject : public CObject
{
	friend class CAbstractCache;

  DECLARE_DYNAMIC(CCachedObject);

	/////////////////////////////////////////////////////
	// Konstrukce & destrukce
public:

	// Konstruktor
	CCachedObject();

	// Destruktor (nezvlada multithreading, predpoklada odemcenost objektu na vsechno)
	virtual ~CCachedObject();


#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

	/////////////////////////////////////////////////////
	// Overridables
public:
	// Celkova velikost spotrebovane pameti pro objekt v bajtech
	virtual DWORD GetSize() { return 0; }

protected:
	// Nahraje data objektu (dostane objekt zamceny na zapis)
	virtual void Load() {}

	// Smaze data objektu (dostane objekt zamceny na zapis)
	virtual void Unload() {}

	/////////////////////////////////////////////////////
	// Zamykani
public:
	// Zamkne objekt na cteni
	virtual void Lock();

	// Zkusi zamknout objekt na cteni, kdyz se nepovede vrati FALSE
	virtual BOOL TestLock();

	// Odemkne objekt po cteni
	virtual void Unlock();

	// Nahraje objekt a zaroven ho uzamkne pro cteni
	virtual void LoadAndLock();

    // Nahraje objekt a zaroven ho uzamkne pro cteni
    // za predpokladu, ze objekt jiz BYL zamknut pro zapis
    void LoadAfterBeingWriteLocked();

	// Zamkne objekt na zapis
	virtual void LockWrite();

	// Zkusi zamknout objekt na zapis, kdyz se nepovede vrati FALSE
	virtual BOOL TestLockWrite();

	// Odemkne objekt po zapisu
	virtual void UnlockWrite();

	// Nahraje objekt a zaroven ho uzamkne pro zapis
	virtual void LoadAndLockWrite();

protected:
	// Zkusi zamknout objekt na zapis, kdyz se nepovede vrati FALSE
	virtual BOOL TestWriteLock();

	// Odemkne objekt po zapisu
	virtual void WriteUnlock();

	/////////////////////////////////////////////////////
	// Hinty
public:
	// Rika, ze data objektu budou potreba
	virtual void HintDataNeeded();

	// Rika, ze data objektu potreba uz nebudou
	virtual void HintDataNotNeeded();

	/////////////////////////////////////////////////////
	// Queries
public:
	// Je objekt nahrany? !Pouzivejte JEN pokud mate objekt zamceny!
	virtual BOOL IsLoaded() { return m_bLoaded; }

	/////////////////////////////////////////////////////
	// Data
private:
	// Zamek
	CCacheRWLock m_RWLock;

	// Priznak, ze je objekt 'loaded'
	BOOL m_bLoaded;
	
	// Ukazatel na cache, ktera tento objekt spravuje
	CAbstractCache *m_pAbstractCache;

	// Jeden extra ukazatel pro data cache
	void *m_pInternalData;
};

#endif // !defined(AFX_CACHEDOBJECT_H__87AA8E67_F3AB_11D3_BFF9_0000B4A08F9A__INCLUDED_)
