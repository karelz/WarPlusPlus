// CacheRWLock.cpp: implementation of the CCacheRWLock class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CacheRWLock.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCacheRWLock::CCacheRWLock() : CReadWriteLock()
{
}

CCacheRWLock::~CCacheRWLock()
{

}

BOOL CCacheRWLock::ReaderTestLock() 
{ 
	// Zkusime zamknout s nulovym timeoutem
	return ReaderLock(0);
}

BOOL CCacheRWLock::WriterTestLock()
{ 
	// Zkusime zamknout s nulovym timeoutem
	return WriterLock(0);
}

BOOL CCacheRWLock::ReaderIsLocked()
{
	// Jestlize alespon jeden ctenar cte, je readlock zamceny
	return m_dwReaderCount>0;
}

void CCacheRWLock::WriterUnlockReaderLock()
{	
/////////////////////////////////////////////////////// VYKRACENO
	// odemkne zámek zapisovatelù
	// VERIFY ( m_mutexWriterLock.Unlock () );
/////////////////////////////////////////////////////// VYKRACENO

/////////////////////////////////////////////////////// VYKRACENO
	// zamkne zámek zapisovatelù
	// if ( !m_mutexWriterLock.Lock ( dwTimeOut ) )
	// {	// nepodaøilo se zamknout zámek zapisovatelù
	//	// vrátí pøíznak nezamèení zámku
	//	return FALSE;
	// }
/////////////////////////////////////////////////////// VYKRACENO

	// zvýší poèet ètenáøù
	DWORD dwReaderCount = ::InterlockedIncrement ( (long *)&m_dwReaderCount );
	ASSERT ( dwReaderCount != 0 );

	// odemkne zámek zapisovatelù
	VERIFY ( m_mutexWriterLock.Unlock () );
}

void CCacheRWLock::ReaderUnlockWriterLock()
{	
	ReaderUnlock();
	WriterLock();
}

// zamkne zámek pro ètenáøe (s timeoutem "dwTimeOut" v milisekundách, INFINITE=èeká 
//		dokud se zámek neuvolní) (FALSE=timeout)
BOOL CCacheRWLock::ReaderLock ( DWORD dwTimeOut /*= INFINITE*/ ) 
{
	// zamkne zámek zapisovatelù
	if ( !m_mutexWriterLock.Lock ( dwTimeOut ) )
	{	// nepodaøilo se zamknout zámek zapisovatelù
		// vrátí pøíznak nezamèení zámku
		return FALSE;
	}

	// zvýší poèet ètenáøù
	DWORD dwReaderCount = ::InterlockedIncrement ( (long *)&m_dwReaderCount );
	ASSERT ( dwReaderCount != 0 );

	// odemkne zámek zapisovatelù
	VERIFY ( m_mutexWriterLock.Unlock () );

	// vrátí pøíznak úspìšného zamèení zámku pro ètenáøe
	return TRUE;
}

// odemkne zámek pro ètenáøe
void CCacheRWLock::ReaderUnlock () 
{

	// sníží poèet ètenáøù
	DWORD dwReaderCount = ::InterlockedDecrement ( (long *)&m_dwReaderCount );

	// otestuje, jedná-li se o posledního ètenáøe
	if ( dwReaderCount == 0 )
	{	// poslední ètenáø odchází
		// nastaví událost odchodu posledního ètenáøe
		VERIFY ( m_eventLastReaderLeft.SetEvent () );
	}
}

// zamkne zámek pro zapisovatele (s timeoutem "dwTimeOut" v milisekundách, 
//		INFINITE=èeká dokud se zámek neuvolní - skuteèná doba timeoutu mùže být
//		až dvojnásobná) (FALSE=timeout)
BOOL CCacheRWLock::WriterLock ( DWORD dwTimeOut /*= INFINITE*/ ) 
{

	// zamkne zámek zapisovatelù
	if ( !m_mutexWriterLock.Lock ( dwTimeOut ) )
	{	// nepodaøilo se zamknout zámek zapisovatelù
		// vrátí pøíznak nezamèení zámku
		return FALSE;
	}

	// poèká na odchod posledního ètenáøe
	while ( m_dwReaderCount != 0 )
	{
		// poèká na událost odchodu posledního ètenáøe
		if ( !m_eventLastReaderLeft.Lock ( dwTimeOut ) )
		{	// událost odchodu posledního ètenáøe nenastala
			// odemkne zámek zapisovatelù
			VERIFY ( m_mutexWriterLock.Unlock () );
			// vrátí pøíznak nezamèení zámku
			return FALSE;
		}
	}


	// vrátí pøíznak úspìšného zamèení zámku pro zapisovatele
	return TRUE;
}

// odemkne zámek pro zapisovatele
void CCacheRWLock::WriterUnlock () 
{
	// odemkne zámek zapisovatelù
	VERIFY ( m_mutexWriterLock.Unlock () );
}

