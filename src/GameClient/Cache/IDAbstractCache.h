// IDAbstractCache.h: interface for the CIDAbstractCache class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IDABSTRACTCACHE_H__14F8F164_1047_11D4_8040_0000B4A08F9A__INCLUDED_)
#define AFX_IDABSTRACTCACHE_H__14F8F164_1047_11D4_8040_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AbstractCache.h"
#include "IDCachedObject.h"

class CIDAbstractCache : public CAbstractCache
{
  DECLARE_DYNAMIC(CIDAbstractCache);

// Konstrukce a destrukce
public:
	// Konstruktor
	CIDAbstractCache(DWORD dwMaxSize=10240, DWORD dwPreferredSize=5120);

	// Konstruktor se zadanim fronty
	CIDAbstractCache(CCacheLoadQueue *pQueue, DWORD dwMaxSize=10240, DWORD dwPreferredSize=5120);

	// Destruktor
	virtual ~CIDAbstractCache();

// Metody
public:
	// Vrati objekt daneho ID (at loaded/unloaded)
	CIDCachedObject *GetObject(DWORD dwID);

	// Zacne prochazet objekty
	void StartBrowse() {}

	// Vrati dalsi objekt pri browseni
	CIDCachedObject *GetNextObject() { return NULL; }

	// Prestane prochazet objekty
	void EndBrowse() {}

// Data
private:
	BOOL m_bBrowsing;
};

#endif // !defined(AFX_IDABSTRACTCACHE_H__14F8F164_1047_11D4_8040_0000B4A08F9A__INCLUDED_)
