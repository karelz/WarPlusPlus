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

#include "StdAfx.h"

#include "ReadWriteLock.h"

#include "Common\AfxDebugPlus\AfxDebugPlus.h"

#ifndef DWORD_MAX
	#define DWORD_MAX ((DWORD)0xffffffff)
#endif //DWORD_MAX

//////////////////////////////////////////////////////////////////////
// Operace se zámkem
//////////////////////////////////////////////////////////////////////

// zamkne zámek pro ètenáøe (s timeoutem "dwTimeOut" v milisekundách, INFINITE=èeká 
//		dokud se zámek neuvolní) (FALSE=timeout)
BOOL CReadWriteLock::ReaderLock ( DWORD dwTimeOut /*= INFINITE*/ ) 
{
#ifdef _DEBUG
	// zamkne seznam ètenáøù
	VERIFY ( m_mutexReaderListLock.Lock () );

	// zjistí ID threadu ètenáøe
	DWORD dwThreadID = GetCurrentThreadId ();

	// ukazatel na ètenáøe v seznamu ètenáøù
	struct SReaderListMember *pReader = m_pFirstReader;

	// zkontroluje, nejedná-li se o druhý zámek od jednoho threadu
	while ( pReader != NULL )
	{
		// zkontroluje, nejedná-li se o druhý zámek od jednoho threadu
		ASSERT ( pReader->dwThreadID != dwThreadID );
		// nechá zpracovat dalšího ètenáøe
		pReader = pReader->pNext;
	}

	// odemkne seznam ètenáøù
	VERIFY ( m_mutexReaderListLock.Unlock () );
#endif //_DEBUG

	// zamkne zámek zapisovatelù
	if ( !m_mutexWriterLock.Lock ( dwTimeOut ) )
	{	// nepodaøilo se zamknout zámek zapisovatelù
		// vrátí pøíznak nezamèení zámku
		return FALSE;
	}

	// zvýší poèet ètenáøù
	DWORD dwReaderCount = ::InterlockedIncrement ( (long *)&m_dwReaderCount );
	ASSERT ( dwReaderCount != 0 );

#ifdef _DEBUG
	// zamkne seznam ètenáøù
	VERIFY ( m_mutexReaderListLock.Lock () );

	// pøidá ètenáøe do seznamu ètenáøù
	pReader = new struct SReaderListMember;
	pReader->pNext = m_pFirstReader;
	pReader->dwThreadID = dwThreadID;
	m_pFirstReader = pReader;

	// odemkne seznam ètenáøù
	VERIFY ( m_mutexReaderListLock.Unlock () );
#endif //_DEBUG

	// odemkne zámek zapisovatelù
	VERIFY ( m_mutexWriterLock.Unlock () );

	// vrátí pøíznak úspìšného zamèení zámku pro ètenáøe
	return TRUE;
}

// odemkne zámek pro ètenáøe
void CReadWriteLock::ReaderUnlock () 
{
#ifdef _DEBUG
	// zamkne seznam ètenáøù
	VERIFY ( m_mutexReaderListLock.Lock () );

	// zjistí ID threadu ètenáøe
	DWORD dwThreadID = GetCurrentThreadId ();

	// ukazatel na ètenáøe v seznamu ètenáøù
	struct SReaderListMember **pReader = &m_pFirstReader;

	// najde ètenáøe v seznamu ètenáøù
	while ( *pReader != NULL )
	{
		// zkontroluje, jedná-li se o hledaného ètenáøe
		if ( (*pReader)->dwThreadID == dwThreadID )
		{
			break;
		}
		// nechá zpracovat dalšího ètenáøe
		pReader = &(*pReader)->pNext;
	}

	// zkontroluje, byl-li ètenáø nalezen v seznamu
	ASSERT ( *pReader != NULL );

	// uschová ukazatel na mazaného ètenáøe
	struct SReaderListMember *pDeletedReader = *pReader;
	// vypojí ètenáøe ze seznamu ètenáøù
	*pReader = (*pReader)->pNext;
	// smaže ètenáøe
	delete pDeletedReader;

	// odemkne seznam ètenáøù
	VERIFY ( m_mutexReaderListLock.Unlock () );
#endif //_DEBUG

	// sníží poèet ètenáøù
	DWORD dwReaderCount = ::InterlockedDecrement ( (long *)&m_dwReaderCount );

	// zkontroluje, byl-li zámek ètenáøù skuteènì zamèen
	ASSERT ( dwReaderCount != DWORD_MAX );

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
BOOL CReadWriteLock::WriterLock ( DWORD dwTimeOut /*= INFINITE*/ ) 
{
#ifdef _DEBUG
	// zamkne seznam ètenáøù
	VERIFY ( m_mutexReaderListLock.Lock () );

	// zjistí ID threadu ètenáøe
	DWORD dwThreadID = GetCurrentThreadId ();

	// ukazatel na ètenáøe v seznamu ètenáøù
	struct SReaderListMember *pReader = m_pFirstReader;

	// zkontroluje, nejedná-li se o druhý zámek od jednoho threadu
	while ( pReader != NULL )
	{
		// zkontroluje, nejedná-li se o druhý zámek od jednoho threadu
		ASSERT ( pReader->dwThreadID != dwThreadID );
		// nechá zpracovat dalšího ètenáøe
		pReader = pReader->pNext;
	}

	// odemkne seznam ètenáøù
	VERIFY ( m_mutexReaderListLock.Unlock () );
#endif //_DEBUG

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

#ifdef _DEBUG
	// zamkne seznam ètenáøù
	VERIFY ( m_mutexReaderListLock.Lock () );

	// pøidá ètenáøe do seznamu ètenáøù
	pReader = new struct SReaderListMember;
	pReader->pNext = m_pFirstReader;
	pReader->dwThreadID = dwThreadID;
	m_pFirstReader = pReader;

	// odemkne seznam ètenáøù
	VERIFY ( m_mutexReaderListLock.Unlock () );
#endif //_DEBUG

	// vrátí pøíznak úspìšného zamèení zámku pro zapisovatele
	return TRUE;
}

#ifdef _DEBUG

// odemkne zámek pro zapisovatele
void CReadWriteLock::WriterUnlock () 
{
	// zamkne seznam ètenáøù
	VERIFY ( m_mutexReaderListLock.Lock () );

	// zjistí ID threadu ètenáøe
	DWORD dwThreadID = GetCurrentThreadId ();

	// ukazatel na ètenáøe v seznamu ètenáøù
	struct SReaderListMember **pReader = &m_pFirstReader;

	// najde ètenáøe v seznamu ètenáøù
	while ( *pReader != NULL )
	{
		// zkontroluje, jedná-li se o hledaného ètenáøe
		if ( (*pReader)->dwThreadID == dwThreadID )
		{
			break;
		}
		// nechá zpracovat dalšího ètenáøe
		pReader = &(*pReader)->pNext;
	}

	// zkontroluje, byl-li ètenáø nalezen v seznamu
	ASSERT ( *pReader != NULL );

	// uschová ukazatel na mazaného ètenáøe
	struct SReaderListMember *pDeletedReader = *pReader;
	// vypojí ètenáøe ze seznamu ètenáøù
	*pReader = (*pReader)->pNext;
	// smaže ètenáøe
	delete pDeletedReader;

	// odemkne seznam ètenáøù
	VERIFY ( m_mutexReaderListLock.Unlock () );

	// odemkne zámek zapisovatelù
	VERIFY ( m_mutexWriterLock.Unlock () );
}

#endif //_DEBUG
