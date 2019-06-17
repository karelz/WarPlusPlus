// AbstractCache.h: interface for the CAbstractCache class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ABSTRACTCACHE_H__87AA8E66_F3AB_11D3_BFF9_0000B4A08F9A__INCLUDED_)
#define AFX_ABSTRACTCACHE_H__87AA8E66_F3AB_11D3_BFF9_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CachedObject.h"
#include "CachedObjectHandle.h"
#include "CacheLoadQueue.h"
#include "CacheRWLock.h"

class CAbstractCache : public CObject  
{
  DECLARE_DYNAMIC(CAbstractCache)

	///////////////////////////////////////////////////////////////////////
	// Construction & destruction
public:	
	// Konstruktor
	CAbstractCache(DWORD dwMaxSize=10240, DWORD dwPreferredSize=5120);

	// Konstruktor se zadanim fronty
	CAbstractCache(CCacheLoadQueue *pQueue, DWORD dwMaxSize=10240, DWORD dwPreferredSize=5120);

	// Destruktor
	virtual ~CAbstractCache();

	///////////////////////////////////////////////////////////////////////
	// Diagnostics
public:
#ifdef _DEBUG 
	// AssertValid
	virtual void AssertValid() const;

	// Dump
	virtual void Dump(CDumpContext &dc) const;
#endif // _DEBUG

	///////////////////////////////////////////////////////////////////////
	// Queries
public:
	// Zjisteni maximalni velikosti cache
	DWORD GetMaxSize() const { return m_dwMaxSize; }
	
	// Nastaveni maximalni velikosti cache
	void SetMaxSize(DWORD dwMaxSize);

	// Zjisteni preferovane velikosti cache
	DWORD GetPreferredSize() const { return m_dwPreferredSize; }
	
	// Nastaveni preferovane velikosti cache
	void SetPreferredSize(DWORD dwPreferredSize);

	// Vraceni mnozstvi pameti, aktualne zabrane cachi
	virtual DWORD MemoryUsed() const;

	// Vraceni max. mnozstvi pameti, ktere kdy cache potrebovala
	DWORD MaxSoFar() const;

	///////////////////////////////////////////////////////////////////////
	// Pridavani/ruseni objektu
public:
	// Vlozi objekt do cache a zacne jej spravovat
	// !predpokladame, ze kazdy objekt bude vlozen max. JEDNOU!
	// a dale, ze vkladani probiha z JEDNOHO THREADU
	virtual void InsertObject(CCachedObject *pCachedObject);

	// Odstrani objekt ze struktur cache
	// Je na uzivateli, aby si na nem zavolal destruktor apod.
	// POZOR - volejte tuto funkci z JEDNOHO THREADU
	virtual CCachedObject *RemoveObject(CCachedObject *pCachedObject);

	///////////////////////////////////////////////////////////////////////
	// Hinty
public:
	// Budou potreba data daneho objektu
	virtual void HintDataNeeded(CCachedObject *pCachedObject);	

	// Uz nebudou potreba data daneho objektu
	virtual void HintDataNotNeeded(CCachedObject *pCachedObject);	

	///////////////////////////////////////////////////////////////////////
	// Notifikace
public:
	// Objekt rika cachi, ze na nej uzivatel pristoupil
	virtual void NotifyObjectAccessed(CCachedObject *pCachedObject);

	// Objekt rika cachi, ze ho uzivatel chce mit loadnuty
	virtual void NotifyObjectLoad(CCachedObject *pCachedObject);

	// Objekt rika cachi, ze se prave loadnul, a ze pred loadem odhadoval svou velikost na dwSizeEstimated
	virtual void NotifyObjectLoaded(CCachedObject *pCachedObject, DWORD dwSizeEstimated);

	///////////////////////////////////////////////////////////////////////
	// Pomocne fce
protected:
	// Volano z konstruktoru, naplneni dat
	void Create(DWORD dwMaxSize, DWORD dwPreferredSize);

	// Volano z konstruktoru, naplneni dat
	void Create(CCacheLoadQueue *pQueue, DWORD dwMaxSize, DWORD dwPreferredSize);

	// Projede spojak objektu v cachi a snazi se vyhazovat, dokud neni
	// MemoryUsed pod zadanym limitem (nezamyka nic, zamykat struktury rucne!)
	// Nikdy nevyrucha objekt pCachedObject
	void Cleanup(DWORD dwLimit, CCachedObject *pNoClean=NULL);

	///////////////////////////////////////////////////////////////////////
	// Data
protected:
	// Maximalni velikost cache
	DWORD m_dwMaxSize;

	// Preferovana velikost cache
	DWORD m_dwPreferredSize;

	// Aktualni velikost cache
	DWORD m_dwActualSize;

	// Pocet objektu v cachi
	DWORD m_dwNumObjects;

	// Hlavicka spojaku objektu
	CCachedObjectHandle *m_pObjects;

	// Fronta na loadeni
	CCacheLoadQueue *m_pQueue;

	// Zamek na strukturu v cachi
	CCacheRWLock m_RWLock;

	///////////////////////////////////////////////////////////////////////
	// Statistika
protected:
	// Maximalni velikost cache, ktere zatim bylo dosazeno
	DWORD m_dwMaxSoFar; 
};

#endif // !defined(AFX_ABSTRACTCACHE_H__87AA8E66_F3AB_11D3_BFF9_0000B4A08F9A__INCLUDED_)
