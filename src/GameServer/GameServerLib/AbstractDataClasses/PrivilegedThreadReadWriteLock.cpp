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

#include "StdAfx.h"

#include "PrivilegedThreadReadWriteLock.h"

//////////////////////////////////////////////////////////////////////
// Operace se zámkem
//////////////////////////////////////////////////////////////////////

// zamkne zámek pro ètenáøe (s timeoutem "dwTimeOut" v milisekundách, INFINITE=èeká dokud 
//		se zámek neuvolní; skuteèná doba timeoutu mùže být i delší) (FALSE=timeout)
BOOL CPrivilegedThreadReadWriteLock::ReaderLock ( DWORD dwTimeOut /*= INFINITE*/ ) 
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

	ASSERT ( !IsPrivilegedThread () );

	// zamkne zámek zapisovatelù
	if ( !m_mutexWriterLock.Lock ( dwTimeOut ) )
	{	// nepodaøilo se zamknout zámek zapisovatelù
		// vrátí pøíznak nezamèení zámku
		return FALSE;
	}

	// poèká na odchod posledního zapisovatele z privilegovaného threadu
	for ( ; ; )
	{
		// zruší událost povolení ètenáøe
		VERIFY ( m_eventReaderAllowed.ResetEvent () );

		// zvýší poèet ètenáøù
		DWORD dwReaderCount = ::InterlockedIncrement ( (long *)&m_dwReaderCount );
		ASSERT ( dwReaderCount != 0 );

		// zjistí, odešel-li poslední zapisovatel z privilegovaného threadu
		if ( m_dwPrivilegedThreadWriterCount == 0 )
		{	// odešel poslední zapisovatel z privilegovaného threadu
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

		// sníží poèet ètenáøù
		dwReaderCount = ::InterlockedDecrement ( (long *)&m_dwReaderCount );

		// zkontroluje, byl-li zámek ètenáøù skuteènì zamèen
		ASSERT ( dwReaderCount != DWORD_MAX );

		// poèká na událost povolení ètenáøe (tj. na odchod posledního zapisovatele 
		//		z privilegovaného threadu)
		if ( !m_eventReaderAllowed.Lock ( dwTimeOut ) )
		{	// událost povolení ètenáøe nenastala
			// odemkne zámek zapisovatelù
			VERIFY ( m_mutexWriterLock.Unlock () );
			// vrátí pøíznak nezamèení zámku
			return FALSE;
		}
	}
	// nemožný pøípad
}

// odemkne zámek pro ètenáøe
void CPrivilegedThreadReadWriteLock::ReaderUnlock () 
{
	ASSERT ( !IsPrivilegedThread () );

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

	// zjistí, jedná-li se o posledního ètenáøe
	if ( dwReaderCount == 0 )
	{	// jedná se o odchod posledního ètenáøe
		// nastaví událost povolení zapisovatele z privilegovaného threadu
		VERIFY ( m_eventPrivilegedThreadWriterAllowed.SetEvent () );
		// zjistí, je-li zámek zamèen z privilegovaného threadu
		if ( ( m_dwPrivilegedThreadReaderCount == 0 ) && 
			( m_dwPrivilegedThreadWriterCount == 0 ) )
		{	// zámek není zamèen z privilegovaného threadu
			// nastaví událost povolení zapisovatele
			VERIFY ( m_eventWriterAllowed.SetEvent () );
		}
	}

}

// zamkne zámek pro zapisovatele (s timeoutem "dwTimeOut" v milisekundách, INFINITE=èeká 
//		dokud se zámek neuvolní; skuteèná doba timeoutu mùže být až dvojnásobná) 
//		(FALSE=timeout)
BOOL CPrivilegedThreadReadWriteLock::WriterLock ( DWORD dwTimeOut /*= INFINITE*/ ) 
{
	ASSERT ( !IsPrivilegedThread () );

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

	// zruší událost povolení zapisovatele
	VERIFY ( m_eventWriterAllowed.ResetEvent () );

	// poèká na odchod posledního
	while ( ( m_dwReaderCount != 0 ) || ( m_dwPrivilegedThreadReaderCount != 0 ) || 
		( m_dwPrivilegedThreadWriterCount != 0 ) )
	{
		// poèká na událost povolení zapisovatele (tj. na odchod posledního)
		if ( !m_eventWriterAllowed.Lock ( dwTimeOut ) )
		{	// událost povolení zapisovatele nenastala
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
void CPrivilegedThreadReadWriteLock::WriterUnlock () 
{
	ASSERT ( !IsPrivilegedThread () );

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

// zamkne zámek pro ètenáøe z privilegovaného threadu (s timeoutem "dwTimeOut" 
//		v milisekundách, INFINITE=èeká dokud se zámek neuvolní) (FALSE=timeout)
BOOL CPrivilegedThreadReadWriteLock::PrivilegedThreadReaderLock ( 
	DWORD dwTimeOut /*= INFINITE*/ ) 
{
	ASSERT ( IsPrivilegedThread () );

	// zjistí, jedná-li se o první zámek z privilegovaného threadu
	if ( ( m_dwPrivilegedThreadReaderCount != 0 ) || 
		( m_dwPrivilegedThreadWriterCount != 0 ) )
	{	// nejedná se o první zámek z privilegovaného threadu
		// zvýší poèet ètenáøù z privilegovaného threadu
		m_dwPrivilegedThreadReaderCount++;
		// vrátí pøíznak úspìšného zamèení zámku pro ètenáøe z privilegovaného threadu
		return TRUE;
	}
	// jedná se o první zámek z privilegovaného threadu

	// zamkne zámek zapisovatelù
	if ( !m_mutexWriterLock.Lock ( dwTimeOut ) )
	{	// nepodaøilo se zamknout zámek zapisovatelù
		// vrátí pøíznak nezamèení zámku
		return FALSE;
	}

	// zvýší poèet ètenáøù z privilegovaného threadu
	m_dwPrivilegedThreadReaderCount++;

	// odemkne zámek zapisovatelù
	VERIFY ( m_mutexWriterLock.Unlock () );

	// vrátí pøíznak úspìšného zamèení zámku pro ètenáøe z privilegovaného threadu
	return TRUE;
}
// odemkne zámek pro ètenáøe zprivilegovaného threadu
void CPrivilegedThreadReadWriteLock::PrivilegedThreadReaderUnlock () 
{
	ASSERT ( IsPrivilegedThread () );

	// sníží poèet ètenáøù z privilegovaného threadu
	m_dwPrivilegedThreadReaderCount--;

	// zjistí, jedná-li se o poslední zámek z privilegovaného threadu a je-li zámek zamèen 
	//		pro ètenáøe
	if ( ( m_dwPrivilegedThreadReaderCount == 0 ) && 
		( m_dwPrivilegedThreadWriterCount == 0 ) && ( m_dwReaderCount == 0 ) )
	{	// jedná se o poslední zámek z privilegovaného threadu a zámek není zamèen 
		//		pro ètenáøe
		// nastaví událost povolení zapisovatele
		VERIFY ( m_eventWriterAllowed.SetEvent () );
	}
}

// zamkne zámek pro zapisovatele z privilegovaného threadu (s timeoutem "dwTimeOut" 
//		v milisekundách, INFINITE=èeká dokud se zámek neuvolní; skuteèná doba timeoutu 
//		mùže být i delší) (FALSE=timeout)
BOOL CPrivilegedThreadReadWriteLock::PrivilegedThreadWriterLock ( 
	DWORD dwTimeOut /*= INFINITE*/ ) 
{
	ASSERT ( IsPrivilegedThread () );

	// zjistí, jedná-li se o první zámek z privilegovaného threadu
	if ( ( m_dwPrivilegedThreadReaderCount != 0 ) || 
		( m_dwPrivilegedThreadWriterCount != 0 ) )
	{	// nejedná se o první zámek z privilegovaného threadu
		// zruší událost povolení zapisovatele z privilegovaného threadu
		VERIFY ( m_eventPrivilegedThreadWriterAllowed.ResetEvent () );

		// zvýší poèet zapisovatelù z privilegovaného threadu
		m_dwPrivilegedThreadWriterCount++;

		// zjistí, je-li zámek zamèen pro ètenáøe
		if ( m_dwReaderCount != 0 )
		{	// zámek je zamèen pro ètenáøe
			// poèká na událost povolení zapisovatele z privilegovaného threadu (tj. 
			//		na odchod posledního ètenáøe)
			if ( !m_eventPrivilegedThreadWriterAllowed.Lock ( dwTimeOut ) )
			{	// událost povolení zapisovatele z privilegovaného threadu nenastala
				// sníží poèet zapisovatelù z privilegovaného threadu
				m_dwPrivilegedThreadWriterCount--;

				// zjistí, jedná-li se o posledního zapisovatele z privilegovaného threadu
				if ( m_dwPrivilegedThreadWriterCount == 0 )
				{	// jedná se o posledního zapisovatele z privilegovaného threadu
					// nastaví událost povolení ètenáøe
					VERIFY ( m_eventReaderAllowed.SetEvent () );
				}
				// vrátí pøíznak nezamèení zámku
				return FALSE;
			}
		}

		// vrátí pøíznak úspìšného zamèení zámku pro zapisovatele z privilegovaného threadu
		return TRUE;
	}
	// jedná se o první zámek z privilegovaného threadu

	// zamkne zámek zapisovatelù
	if ( !m_mutexWriterLock.Lock ( dwTimeOut ) )
	{	// nepodaøilo se zamknout zámek zapisovatelù
		// vrátí pøíznak nezamèení zámku
		return FALSE;
	}

	// zvýší poèet zapisovatelù z privilegovaného threadu
	m_dwPrivilegedThreadWriterCount++;

	// odemkne zámek zapisovatelù
	VERIFY ( m_mutexWriterLock.Unlock () );

	// vrátí pøíznak úspìšného zamèení zámku pro zapisovatele z privilegovaného threadu
	return TRUE;
}

// odemkne zámek pro zapisovatele z privilegovaného threadu
void CPrivilegedThreadReadWriteLock::PrivilegedThreadWriterUnlock () 
{
	ASSERT ( IsPrivilegedThread () );

	// sníží poèet zapisovatelù z privilegovaného threadu
	m_dwPrivilegedThreadWriterCount--;

	// zjistí, jedná-li se o posledního zapisovatele z privilegovaného threadu
	if ( m_dwPrivilegedThreadWriterCount == 0 )
	{	// jedná se o posledního zapisovatele z privilegovaného threadu
		// nastaví událost povolení ètenáøe
		VERIFY ( m_eventReaderAllowed.SetEvent () );
		// zjistí, je-li zámek zamèen pro ètení mimo i z privilegovaného threadu
		if ( ( m_dwPrivilegedThreadReaderCount == 0 ) && ( m_dwReaderCount == 0 ) )
		{	// zámek není zamèen pro ètení ani mimo ani z privilegovaného threadu
			// nastaví událost povolení zapisovatele
			VERIFY ( m_eventWriterAllowed.SetEvent () );
		}
	}
}
