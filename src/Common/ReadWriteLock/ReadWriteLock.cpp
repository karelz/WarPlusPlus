/************************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Obecn� t��dy a �ablony
 *   Autor: Karel Zikmund
 * 
 *   Popis: Z�mek pro �tec� a zapisovac� operace (pro v�ce 
 *          zapisovatel� a v�ce �ten���)
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "ReadWriteLock.h"

#include "Common\AfxDebugPlus\AfxDebugPlus.h"

#ifndef DWORD_MAX
	#define DWORD_MAX ((DWORD)0xffffffff)
#endif //DWORD_MAX

//////////////////////////////////////////////////////////////////////
// Operace se z�mkem
//////////////////////////////////////////////////////////////////////

// zamkne z�mek pro �ten��e (s timeoutem "dwTimeOut" v milisekund�ch, INFINITE=�ek� 
//		dokud se z�mek neuvoln�) (FALSE=timeout)
BOOL CReadWriteLock::ReaderLock ( DWORD dwTimeOut /*= INFINITE*/ ) 
{
#ifdef _DEBUG
	// zamkne seznam �ten���
	VERIFY ( m_mutexReaderListLock.Lock () );

	// zjist� ID threadu �ten��e
	DWORD dwThreadID = GetCurrentThreadId ();

	// ukazatel na �ten��e v seznamu �ten���
	struct SReaderListMember *pReader = m_pFirstReader;

	// zkontroluje, nejedn�-li se o druh� z�mek od jednoho threadu
	while ( pReader != NULL )
	{
		// zkontroluje, nejedn�-li se o druh� z�mek od jednoho threadu
		ASSERT ( pReader->dwThreadID != dwThreadID );
		// nech� zpracovat dal��ho �ten��e
		pReader = pReader->pNext;
	}

	// odemkne seznam �ten���
	VERIFY ( m_mutexReaderListLock.Unlock () );
#endif //_DEBUG

	// zamkne z�mek zapisovatel�
	if ( !m_mutexWriterLock.Lock ( dwTimeOut ) )
	{	// nepoda�ilo se zamknout z�mek zapisovatel�
		// vr�t� p��znak nezam�en� z�mku
		return FALSE;
	}

	// zv��� po�et �ten���
	DWORD dwReaderCount = ::InterlockedIncrement ( (long *)&m_dwReaderCount );
	ASSERT ( dwReaderCount != 0 );

#ifdef _DEBUG
	// zamkne seznam �ten���
	VERIFY ( m_mutexReaderListLock.Lock () );

	// p�id� �ten��e do seznamu �ten���
	pReader = new struct SReaderListMember;
	pReader->pNext = m_pFirstReader;
	pReader->dwThreadID = dwThreadID;
	m_pFirstReader = pReader;

	// odemkne seznam �ten���
	VERIFY ( m_mutexReaderListLock.Unlock () );
#endif //_DEBUG

	// odemkne z�mek zapisovatel�
	VERIFY ( m_mutexWriterLock.Unlock () );

	// vr�t� p��znak �sp�n�ho zam�en� z�mku pro �ten��e
	return TRUE;
}

// odemkne z�mek pro �ten��e
void CReadWriteLock::ReaderUnlock () 
{
#ifdef _DEBUG
	// zamkne seznam �ten���
	VERIFY ( m_mutexReaderListLock.Lock () );

	// zjist� ID threadu �ten��e
	DWORD dwThreadID = GetCurrentThreadId ();

	// ukazatel na �ten��e v seznamu �ten���
	struct SReaderListMember **pReader = &m_pFirstReader;

	// najde �ten��e v seznamu �ten���
	while ( *pReader != NULL )
	{
		// zkontroluje, jedn�-li se o hledan�ho �ten��e
		if ( (*pReader)->dwThreadID == dwThreadID )
		{
			break;
		}
		// nech� zpracovat dal��ho �ten��e
		pReader = &(*pReader)->pNext;
	}

	// zkontroluje, byl-li �ten�� nalezen v seznamu
	ASSERT ( *pReader != NULL );

	// uschov� ukazatel na mazan�ho �ten��e
	struct SReaderListMember *pDeletedReader = *pReader;
	// vypoj� �ten��e ze seznamu �ten���
	*pReader = (*pReader)->pNext;
	// sma�e �ten��e
	delete pDeletedReader;

	// odemkne seznam �ten���
	VERIFY ( m_mutexReaderListLock.Unlock () );
#endif //_DEBUG

	// sn�� po�et �ten���
	DWORD dwReaderCount = ::InterlockedDecrement ( (long *)&m_dwReaderCount );

	// zkontroluje, byl-li z�mek �ten��� skute�n� zam�en
	ASSERT ( dwReaderCount != DWORD_MAX );

	// otestuje, jedn�-li se o posledn�ho �ten��e
	if ( dwReaderCount == 0 )
	{	// posledn� �ten�� odch�z�
		// nastav� ud�lost odchodu posledn�ho �ten��e
		VERIFY ( m_eventLastReaderLeft.SetEvent () );
	}
}

// zamkne z�mek pro zapisovatele (s timeoutem "dwTimeOut" v milisekund�ch, 
//		INFINITE=�ek� dokud se z�mek neuvoln� - skute�n� doba timeoutu m��e b�t
//		a� dvojn�sobn�) (FALSE=timeout)
BOOL CReadWriteLock::WriterLock ( DWORD dwTimeOut /*= INFINITE*/ ) 
{
#ifdef _DEBUG
	// zamkne seznam �ten���
	VERIFY ( m_mutexReaderListLock.Lock () );

	// zjist� ID threadu �ten��e
	DWORD dwThreadID = GetCurrentThreadId ();

	// ukazatel na �ten��e v seznamu �ten���
	struct SReaderListMember *pReader = m_pFirstReader;

	// zkontroluje, nejedn�-li se o druh� z�mek od jednoho threadu
	while ( pReader != NULL )
	{
		// zkontroluje, nejedn�-li se o druh� z�mek od jednoho threadu
		ASSERT ( pReader->dwThreadID != dwThreadID );
		// nech� zpracovat dal��ho �ten��e
		pReader = pReader->pNext;
	}

	// odemkne seznam �ten���
	VERIFY ( m_mutexReaderListLock.Unlock () );
#endif //_DEBUG

	// zamkne z�mek zapisovatel�
	if ( !m_mutexWriterLock.Lock ( dwTimeOut ) )
	{	// nepoda�ilo se zamknout z�mek zapisovatel�
		// vr�t� p��znak nezam�en� z�mku
		return FALSE;
	}

	// po�k� na odchod posledn�ho �ten��e
	while ( m_dwReaderCount != 0 )
	{
		// po�k� na ud�lost odchodu posledn�ho �ten��e
		if ( !m_eventLastReaderLeft.Lock ( dwTimeOut ) )
		{	// ud�lost odchodu posledn�ho �ten��e nenastala
			// odemkne z�mek zapisovatel�
			VERIFY ( m_mutexWriterLock.Unlock () );
			// vr�t� p��znak nezam�en� z�mku
			return FALSE;
		}
	}

#ifdef _DEBUG
	// zamkne seznam �ten���
	VERIFY ( m_mutexReaderListLock.Lock () );

	// p�id� �ten��e do seznamu �ten���
	pReader = new struct SReaderListMember;
	pReader->pNext = m_pFirstReader;
	pReader->dwThreadID = dwThreadID;
	m_pFirstReader = pReader;

	// odemkne seznam �ten���
	VERIFY ( m_mutexReaderListLock.Unlock () );
#endif //_DEBUG

	// vr�t� p��znak �sp�n�ho zam�en� z�mku pro zapisovatele
	return TRUE;
}

#ifdef _DEBUG

// odemkne z�mek pro zapisovatele
void CReadWriteLock::WriterUnlock () 
{
	// zamkne seznam �ten���
	VERIFY ( m_mutexReaderListLock.Lock () );

	// zjist� ID threadu �ten��e
	DWORD dwThreadID = GetCurrentThreadId ();

	// ukazatel na �ten��e v seznamu �ten���
	struct SReaderListMember **pReader = &m_pFirstReader;

	// najde �ten��e v seznamu �ten���
	while ( *pReader != NULL )
	{
		// zkontroluje, jedn�-li se o hledan�ho �ten��e
		if ( (*pReader)->dwThreadID == dwThreadID )
		{
			break;
		}
		// nech� zpracovat dal��ho �ten��e
		pReader = &(*pReader)->pNext;
	}

	// zkontroluje, byl-li �ten�� nalezen v seznamu
	ASSERT ( *pReader != NULL );

	// uschov� ukazatel na mazan�ho �ten��e
	struct SReaderListMember *pDeletedReader = *pReader;
	// vypoj� �ten��e ze seznamu �ten���
	*pReader = (*pReader)->pNext;
	// sma�e �ten��e
	delete pDeletedReader;

	// odemkne seznam �ten���
	VERIFY ( m_mutexReaderListLock.Unlock () );

	// odemkne z�mek zapisovatel�
	VERIFY ( m_mutexWriterLock.Unlock () );
}

#endif //_DEBUG
