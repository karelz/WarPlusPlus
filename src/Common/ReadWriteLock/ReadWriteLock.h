/************************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Obecné tøídy a šablony
 *   Autor: Karel Zikmund
 * 
 *   Popis: Zámek pro ètecí a zapisovací operace (pro více 
 *          zapisovatelù a více ètenáøù)
 * 
 ***********************************************************/

#ifndef __READ_WRITE_LOCK__HEADER_INCLUDED__
#define __READ_WRITE_LOCK__HEADER_INCLUDED__

#include <afxmt.h>

//////////////////////////////////////////////////////////////////////
// Tøída zámku - umožòuje pøístup více ètenáøùm a více zapisovatelùm. 
//		Najednou mùže èíst více ètenáøù, ale zapisovat mùže vždy jediný 
//		zapisovatel a navíc bez ètenáøù.
class CReadWriteLock 
{
// Metody
public:
// Konstrukce a destukce

	// konstruktor
	CReadWriteLock ();
	// destruktor
	~CReadWriteLock ();

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

// Data
protected:
	// zámek zapisovatelù
	CCriticalSection m_mutexWriterLock;
	// poèet ètenáøù
	DWORD m_dwReaderCount;
	// událost odchodu posledního ètenáøe
	CEvent m_eventLastReaderLeft;

#ifdef _DEBUG
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
inline CReadWriteLock::CReadWriteLock () 
{
	// inicializuje poèet zamèených ètenáøù
	m_dwReaderCount = 0;

#ifdef _DEBUG
	// inicializuje seznam ètenáøù
	m_pFirstReader = NULL;
#endif //_DEBUG
}

// destruktor
inline CReadWriteLock::~CReadWriteLock () 
{
	ASSERT ( m_dwReaderCount == 0 );

	ASSERT ( m_pFirstReader == NULL );
}

//////////////////////////////////////////////////////////////////////
// Operace se zámkem
//////////////////////////////////////////////////////////////////////

#ifndef _DEBUG

// odemkne zámek pro zapisovatele
inline void CReadWriteLock::WriterUnlock () 
{
	// odemkne zámek zapisovatelù
	VERIFY ( m_mutexWriterLock.Unlock () );
}

#endif //!_DEBUG

#endif //__READ_WRITE_LOCK__HEADER_INCLUDED__
