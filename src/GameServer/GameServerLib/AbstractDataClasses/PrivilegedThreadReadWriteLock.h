/************************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Obecné tøídy a šablony
 *   Autor: Karel Zikmund
 * 
 *   Popis: Zámek pro ètecí a zapisovací operace 
 *          s privilegovaným threadem (pro více zapisovatelù 
 *          a více ètenáøù)
 * 
 ***********************************************************/

#ifndef __PRIVILEGED_THREAD_READ_WRITE_LOCK__HEADER_INCLUDED__
#define __PRIVILEGED_THREAD_READ_WRITE_LOCK__HEADER_INCLUDED__

#include <afxmt.h>

#ifndef DWORD_MAX
	#define DWORD_MAX ((DWORD)0xffffffff)
#endif //DWORD_MAX

//////////////////////////////////////////////////////////////////////
// Tøída zámku s privilegovaným threadem - umožòuje pøístup více 
//		ètenáøùm a více zapisovatelùm. Najednou mùže èíst více ètenáøù, 
//		ale zapisovat mùže vždy jediný zapisovatel a navíc bez ètenáøù. 
//		Privilegovaní zapisovatelé a ètenáøi mohou být splolu libovolnì 
//		zkombinováni.
class CPrivilegedThreadReadWriteLock 
{
// Metody
public:
// Konstrukce a destukce

	// konstruktor
	CPrivilegedThreadReadWriteLock ();
	// destruktor
	~CPrivilegedThreadReadWriteLock ();

// Operace se zámkem

	// zamkne zámek pro ètenáøe (s timeoutem "dwTimeOut" v milisekundách, INFINITE=èeká 
	//		dokud se zámek neuvolní; skuteèná doba timeoutu mùže být i delší) 
	//		(FALSE=timeout)
	BOOL ReaderLock ( DWORD dwTimeOut = INFINITE );
	// odemkne zámek pro ètenáøe
	void ReaderUnlock ();
	// zamkne zámek pro zapisovatele (s timeoutem "dwTimeOut" v milisekundách, 
	//		INFINITE=èeká dokud se zámek neuvolní; skuteèná doba timeoutu mùže být 
	//		až dvojnásobná) (FALSE=timeout)
	BOOL WriterLock ( DWORD dwTimeOut = INFINITE );
	// odemkne zámek pro zapisovatele
#ifdef _DEBUG
	void WriterUnlock ();
#else //!_DEBUG
	inline void WriterUnlock ();
#endif //!_DEBUG
	// zamkne zámek pro ètenáøe z privilegovaného threadu (s timeoutem "dwTimeOut" 
	//		v milisekundách, INFINITE=èeká dokud se zámek neuvolní) (FALSE=timeout)
	BOOL PrivilegedThreadReaderLock ( DWORD dwTimeOut = INFINITE );
	// odemkne zámek pro ètenáøe zprivilegovaného threadu
	void PrivilegedThreadReaderUnlock ();
	// zamkne zámek pro zapisovatele z privilegovaného threadu (s timeoutem "dwTimeOut" 
	//		v milisekundách, INFINITE=èeká dokud se zámek neuvolní; skuteèná doba timeoutu 
	//		mùže být i delší) (FALSE=timeout)
	BOOL PrivilegedThreadWriterLock ( DWORD dwTimeOut = INFINITE );
	// odemkne zámek pro zapisovatele z privilegovaného threadu
	void PrivilegedThreadWriterUnlock ();

#ifdef _DEBUG
// Debuggovací metody
public:
	// nastaví ID privilegovaného threadu
	inline void SetPrivilegedThreadID ( DWORD dwPrivilegedThreadID );
private:
	// zjistí, jedná-li se o privilegovaný thread (TRUE=jedná se o privilegovaný thread)
	inline BOOL IsPrivilegedThread ();
#endif //_DEBUG

// Data
private:
	// zámek zapisovatelù
	CMutex m_mutexWriterLock;
	// poèet ètenáøù
	DWORD m_dwReaderCount;
	// poèet ètenáøù z privilegovaného threadu
	DWORD m_dwPrivilegedThreadReaderCount;
	// poèet zapisovatelù z privilegovaného threadu
	DWORD m_dwPrivilegedThreadWriterCount;
	// událost povolení zapisovatele
	CEvent m_eventWriterAllowed;
	// událost povolení ètenáøe
	CEvent m_eventReaderAllowed;
	// událost povolení zapisovatele z privilegovaného threadu
	CEvent m_eventPrivilegedThreadWriterAllowed;

#ifdef _DEBUG
// Debuggovací data
private:
	// ID privilegovaného threadu
	DWORD m_dwPrivilegedThreadID;

	// prvek seznamu ètenáøù
	struct SReaderListMember 
	{
		// další prvek seznamu ètenáøù
		struct SReaderListMember *pNext;
		// ID threadu ètenáøe
		DWORD dwThreadID;
	};
	// zámek seznamu ètenáøù
	CMutex m_mutexReaderListLock;
	// ukazatel na první prvek seznamu ètenáøù
	struct SReaderListMember *m_pFirstReader;
#endif //_DEBUG
};

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
inline CPrivilegedThreadReadWriteLock::CPrivilegedThreadReadWriteLock () 
{
	// inicializuje data objektu
	m_dwReaderCount = 0;
	m_dwPrivilegedThreadReaderCount = 0;
	m_dwPrivilegedThreadWriterCount = 0;

#ifdef _DEBUG
	// inicializuje ID privilegovaného threadu
	m_dwPrivilegedThreadID = DWORD_MAX;
	// inicializuje seznam ètenáøù
	m_pFirstReader = NULL;
#endif //_DEBUG
}

// destruktor
inline CPrivilegedThreadReadWriteLock::~CPrivilegedThreadReadWriteLock () 
{
	ASSERT ( m_dwReaderCount == 0 );
	ASSERT ( m_dwPrivilegedThreadReaderCount == 0 );
	ASSERT ( m_dwPrivilegedThreadWriterCount == 0 );

	ASSERT ( m_pFirstReader == NULL );
}

//////////////////////////////////////////////////////////////////////
// Operace se zámkem
//////////////////////////////////////////////////////////////////////

#ifndef _DEBUG

// odemkne zámek pro zapisovatele
inline void CPrivilegedThreadReadWriteLock::WriterUnlock () 
{
	ASSERT ( !IsPrivilegedThread () );

	// odemkne zámek zapisovatelù
	VERIFY ( m_mutexWriterLock.Unlock () );
}

#endif //!_DEBUG

#ifdef _DEBUG

//////////////////////////////////////////////////////////////////////
// Debuggovací metody
//////////////////////////////////////////////////////////////////////

// nastaví ID privilegovaného threadu
inline void CPrivilegedThreadReadWriteLock::SetPrivilegedThreadID ( 
	DWORD dwPrivilegedThreadID ) 
{
//	ASSERT ( m_dwPrivilegedThreadID == DWORD_MAX );
	ASSERT ( dwPrivilegedThreadID != DWORD_MAX );

	// nastaví ID privilegovaného threadu
	m_dwPrivilegedThreadID = dwPrivilegedThreadID;
}

// zjistí, jedná-li se o privilegovaný thread (TRUE=jedná se o privilegovaný thread)
inline BOOL CPrivilegedThreadReadWriteLock::IsPrivilegedThread () 
{
	ASSERT ( m_dwPrivilegedThreadID != DWORD_MAX );

	// vrátí pøíznak privilegovaného threadu
	return ( m_dwPrivilegedThreadID == GetCurrentThreadId () );
}

#endif //_DEBUG

#endif //__PRIVILEGED_THREAD_READ_WRITE_LOCK__HEADER_INCLUDED__
