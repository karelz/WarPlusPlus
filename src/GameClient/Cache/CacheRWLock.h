// CacheRWLock.h: interface for the CCacheRWLock class.
//
//////////////////////////////////////////////////////////////////////

/************************************************************
 * Tento zamek byl prevzat od Zikiho a mirne modifikovan,
 * takze podporuje nove revolucni ficury (stylu WriteUnlockReadLock ;)
 *
 * Lepsi dokumentaci najdete u objektu CReadWriteLock
 ***********************************************************/


#if !defined(AFX_CACHERWLOCK_H__DBBE1D46_F3EF_11D3_BFFA_0000B4A08F9A__INCLUDED_)
#define AFX_CACHERWLOCK_H__DBBE1D46_F3EF_11D3_BFFA_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxmt.h>
#include "../../Common/ReadWriteLock/ReadWriteLock.h"

// RW Lock pouzivany pri cachovani
class CCacheRWLock : public CReadWriteLock
{
	/////////////////////////////////////////////////
	// Konstrukce a destrukce
public:
	// Konstruktor
	CCacheRWLock();

	// Destruktor
	virtual ~CCacheRWLock();

// Operace se zámkem

	// zamkne zámek pro ètenáøe (s timeoutem "dwTimeOut" v milisekundách, INFINITE=èeká 
	//		dokud se zámek neuvolní) (FALSE=timeout)
	BOOL ReaderLock ( DWORD dwTimeOut = INFINITE );
	// odemkne zámek pro ètenáøe
	void ReaderUnlock ();
	// zamkne zámek pro zapisovatele (s timeoutem "dwTimeOut" v milisekundách, 
	//		INFINITE=èeká dokud se zámek neuvolní - skuteèná doba timeoutu mùže být
	//		až dvojnásobná) (FALSE=timeout)
	BOOL WriterLock ( DWORD dwTimeOut = INFINITE );
	// odemkne zámek pro zapisovatele
	void WriterUnlock ();

	/////////////////////////////////////////////////
	// Zamykani
public:

	// Pokus o zamknuti na cteni, pokud nevyjde, vraci false
	virtual BOOL ReaderTestLock();

	// Pokus o zamknuti na zapis, pokud nevyjde, vraci false
	virtual BOOL WriterTestLock();

	/////////////////////////////////////////////////
	// Queries
public:
	// Je objekt zamceny na cteni?
	BOOL ReaderIsLocked();

	/////////////////////////////////////////////////
	// Speciality a naminky
public:
	// Predpokladame, ze je zamek zamknuty na write *volanym*
	// Atomicky dojde k odemknuti write locku a okamzitemu zamknuti read locku
	void WriterUnlockReaderLock();

	// Predpokladame, ze je zamek zamknuty na read *volanym*
	// Atomicky dojde k odemknuti read locku a okamzitemu zamknuti write locku
	// !!!!!!! Dodelat !!!!!!!!
	void ReaderUnlockWriterLock();
};

#endif // !defined(AFX_CACHERWLOCK_H__DBBE1D46_F3EF_11D3_BFFA_0000B4A08F9A__INCLUDED_)
