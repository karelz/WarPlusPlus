/************************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Obecn� t��dy a �ablony
 *   Autor: Karel Zikmund
 * 
 *   Popis: Z�mek pro �tec� a zapisovac� operace 
 *          s privilegovan�m threadem (pro v�ce zapisovatel� 
 *          a v�ce �ten���)
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "PrivilegedThreadReadWriteLock.h"

//////////////////////////////////////////////////////////////////////
// Operace se z�mkem
//////////////////////////////////////////////////////////////////////

// zamkne z�mek pro �ten��e (s timeoutem "dwTimeOut" v milisekund�ch, INFINITE=�ek� dokud 
//		se z�mek neuvoln�; skute�n� doba timeoutu m��e b�t i del��) (FALSE=timeout)
BOOL CPrivilegedThreadReadWriteLock::ReaderLock ( DWORD dwTimeOut /*= INFINITE*/ ) 
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

	ASSERT ( !IsPrivilegedThread () );

	// zamkne z�mek zapisovatel�
	if ( !m_mutexWriterLock.Lock ( dwTimeOut ) )
	{	// nepoda�ilo se zamknout z�mek zapisovatel�
		// vr�t� p��znak nezam�en� z�mku
		return FALSE;
	}

	// po�k� na odchod posledn�ho zapisovatele z privilegovan�ho threadu
	for ( ; ; )
	{
		// zru�� ud�lost povolen� �ten��e
		VERIFY ( m_eventReaderAllowed.ResetEvent () );

		// zv��� po�et �ten���
		DWORD dwReaderCount = ::InterlockedIncrement ( (long *)&m_dwReaderCount );
		ASSERT ( dwReaderCount != 0 );

		// zjist�, ode�el-li posledn� zapisovatel z privilegovan�ho threadu
		if ( m_dwPrivilegedThreadWriterCount == 0 )
		{	// ode�el posledn� zapisovatel z privilegovan�ho threadu
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

		// sn�� po�et �ten���
		dwReaderCount = ::InterlockedDecrement ( (long *)&m_dwReaderCount );

		// zkontroluje, byl-li z�mek �ten��� skute�n� zam�en
		ASSERT ( dwReaderCount != DWORD_MAX );

		// po�k� na ud�lost povolen� �ten��e (tj. na odchod posledn�ho zapisovatele 
		//		z privilegovan�ho threadu)
		if ( !m_eventReaderAllowed.Lock ( dwTimeOut ) )
		{	// ud�lost povolen� �ten��e nenastala
			// odemkne z�mek zapisovatel�
			VERIFY ( m_mutexWriterLock.Unlock () );
			// vr�t� p��znak nezam�en� z�mku
			return FALSE;
		}
	}
	// nemo�n� p��pad
}

// odemkne z�mek pro �ten��e
void CPrivilegedThreadReadWriteLock::ReaderUnlock () 
{
	ASSERT ( !IsPrivilegedThread () );

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

	// zjist�, jedn�-li se o posledn�ho �ten��e
	if ( dwReaderCount == 0 )
	{	// jedn� se o odchod posledn�ho �ten��e
		// nastav� ud�lost povolen� zapisovatele z privilegovan�ho threadu
		VERIFY ( m_eventPrivilegedThreadWriterAllowed.SetEvent () );
		// zjist�, je-li z�mek zam�en z privilegovan�ho threadu
		if ( ( m_dwPrivilegedThreadReaderCount == 0 ) && 
			( m_dwPrivilegedThreadWriterCount == 0 ) )
		{	// z�mek nen� zam�en z privilegovan�ho threadu
			// nastav� ud�lost povolen� zapisovatele
			VERIFY ( m_eventWriterAllowed.SetEvent () );
		}
	}

}

// zamkne z�mek pro zapisovatele (s timeoutem "dwTimeOut" v milisekund�ch, INFINITE=�ek� 
//		dokud se z�mek neuvoln�; skute�n� doba timeoutu m��e b�t a� dvojn�sobn�) 
//		(FALSE=timeout)
BOOL CPrivilegedThreadReadWriteLock::WriterLock ( DWORD dwTimeOut /*= INFINITE*/ ) 
{
	ASSERT ( !IsPrivilegedThread () );

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

	// zru�� ud�lost povolen� zapisovatele
	VERIFY ( m_eventWriterAllowed.ResetEvent () );

	// po�k� na odchod posledn�ho
	while ( ( m_dwReaderCount != 0 ) || ( m_dwPrivilegedThreadReaderCount != 0 ) || 
		( m_dwPrivilegedThreadWriterCount != 0 ) )
	{
		// po�k� na ud�lost povolen� zapisovatele (tj. na odchod posledn�ho)
		if ( !m_eventWriterAllowed.Lock ( dwTimeOut ) )
		{	// ud�lost povolen� zapisovatele nenastala
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
void CPrivilegedThreadReadWriteLock::WriterUnlock () 
{
	ASSERT ( !IsPrivilegedThread () );

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

// zamkne z�mek pro �ten��e z privilegovan�ho threadu (s timeoutem "dwTimeOut" 
//		v milisekund�ch, INFINITE=�ek� dokud se z�mek neuvoln�) (FALSE=timeout)
BOOL CPrivilegedThreadReadWriteLock::PrivilegedThreadReaderLock ( 
	DWORD dwTimeOut /*= INFINITE*/ ) 
{
	ASSERT ( IsPrivilegedThread () );

	// zjist�, jedn�-li se o prvn� z�mek z privilegovan�ho threadu
	if ( ( m_dwPrivilegedThreadReaderCount != 0 ) || 
		( m_dwPrivilegedThreadWriterCount != 0 ) )
	{	// nejedn� se o prvn� z�mek z privilegovan�ho threadu
		// zv��� po�et �ten��� z privilegovan�ho threadu
		m_dwPrivilegedThreadReaderCount++;
		// vr�t� p��znak �sp�n�ho zam�en� z�mku pro �ten��e z privilegovan�ho threadu
		return TRUE;
	}
	// jedn� se o prvn� z�mek z privilegovan�ho threadu

	// zamkne z�mek zapisovatel�
	if ( !m_mutexWriterLock.Lock ( dwTimeOut ) )
	{	// nepoda�ilo se zamknout z�mek zapisovatel�
		// vr�t� p��znak nezam�en� z�mku
		return FALSE;
	}

	// zv��� po�et �ten��� z privilegovan�ho threadu
	m_dwPrivilegedThreadReaderCount++;

	// odemkne z�mek zapisovatel�
	VERIFY ( m_mutexWriterLock.Unlock () );

	// vr�t� p��znak �sp�n�ho zam�en� z�mku pro �ten��e z privilegovan�ho threadu
	return TRUE;
}
// odemkne z�mek pro �ten��e zprivilegovan�ho threadu
void CPrivilegedThreadReadWriteLock::PrivilegedThreadReaderUnlock () 
{
	ASSERT ( IsPrivilegedThread () );

	// sn�� po�et �ten��� z privilegovan�ho threadu
	m_dwPrivilegedThreadReaderCount--;

	// zjist�, jedn�-li se o posledn� z�mek z privilegovan�ho threadu a je-li z�mek zam�en 
	//		pro �ten��e
	if ( ( m_dwPrivilegedThreadReaderCount == 0 ) && 
		( m_dwPrivilegedThreadWriterCount == 0 ) && ( m_dwReaderCount == 0 ) )
	{	// jedn� se o posledn� z�mek z privilegovan�ho threadu a z�mek nen� zam�en 
		//		pro �ten��e
		// nastav� ud�lost povolen� zapisovatele
		VERIFY ( m_eventWriterAllowed.SetEvent () );
	}
}

// zamkne z�mek pro zapisovatele z privilegovan�ho threadu (s timeoutem "dwTimeOut" 
//		v milisekund�ch, INFINITE=�ek� dokud se z�mek neuvoln�; skute�n� doba timeoutu 
//		m��e b�t i del��) (FALSE=timeout)
BOOL CPrivilegedThreadReadWriteLock::PrivilegedThreadWriterLock ( 
	DWORD dwTimeOut /*= INFINITE*/ ) 
{
	ASSERT ( IsPrivilegedThread () );

	// zjist�, jedn�-li se o prvn� z�mek z privilegovan�ho threadu
	if ( ( m_dwPrivilegedThreadReaderCount != 0 ) || 
		( m_dwPrivilegedThreadWriterCount != 0 ) )
	{	// nejedn� se o prvn� z�mek z privilegovan�ho threadu
		// zru�� ud�lost povolen� zapisovatele z privilegovan�ho threadu
		VERIFY ( m_eventPrivilegedThreadWriterAllowed.ResetEvent () );

		// zv��� po�et zapisovatel� z privilegovan�ho threadu
		m_dwPrivilegedThreadWriterCount++;

		// zjist�, je-li z�mek zam�en pro �ten��e
		if ( m_dwReaderCount != 0 )
		{	// z�mek je zam�en pro �ten��e
			// po�k� na ud�lost povolen� zapisovatele z privilegovan�ho threadu (tj. 
			//		na odchod posledn�ho �ten��e)
			if ( !m_eventPrivilegedThreadWriterAllowed.Lock ( dwTimeOut ) )
			{	// ud�lost povolen� zapisovatele z privilegovan�ho threadu nenastala
				// sn�� po�et zapisovatel� z privilegovan�ho threadu
				m_dwPrivilegedThreadWriterCount--;

				// zjist�, jedn�-li se o posledn�ho zapisovatele z privilegovan�ho threadu
				if ( m_dwPrivilegedThreadWriterCount == 0 )
				{	// jedn� se o posledn�ho zapisovatele z privilegovan�ho threadu
					// nastav� ud�lost povolen� �ten��e
					VERIFY ( m_eventReaderAllowed.SetEvent () );
				}
				// vr�t� p��znak nezam�en� z�mku
				return FALSE;
			}
		}

		// vr�t� p��znak �sp�n�ho zam�en� z�mku pro zapisovatele z privilegovan�ho threadu
		return TRUE;
	}
	// jedn� se o prvn� z�mek z privilegovan�ho threadu

	// zamkne z�mek zapisovatel�
	if ( !m_mutexWriterLock.Lock ( dwTimeOut ) )
	{	// nepoda�ilo se zamknout z�mek zapisovatel�
		// vr�t� p��znak nezam�en� z�mku
		return FALSE;
	}

	// zv��� po�et zapisovatel� z privilegovan�ho threadu
	m_dwPrivilegedThreadWriterCount++;

	// odemkne z�mek zapisovatel�
	VERIFY ( m_mutexWriterLock.Unlock () );

	// vr�t� p��znak �sp�n�ho zam�en� z�mku pro zapisovatele z privilegovan�ho threadu
	return TRUE;
}

// odemkne z�mek pro zapisovatele z privilegovan�ho threadu
void CPrivilegedThreadReadWriteLock::PrivilegedThreadWriterUnlock () 
{
	ASSERT ( IsPrivilegedThread () );

	// sn�� po�et zapisovatel� z privilegovan�ho threadu
	m_dwPrivilegedThreadWriterCount--;

	// zjist�, jedn�-li se o posledn�ho zapisovatele z privilegovan�ho threadu
	if ( m_dwPrivilegedThreadWriterCount == 0 )
	{	// jedn� se o posledn�ho zapisovatele z privilegovan�ho threadu
		// nastav� ud�lost povolen� �ten��e
		VERIFY ( m_eventReaderAllowed.SetEvent () );
		// zjist�, je-li z�mek zam�en pro �ten� mimo i z privilegovan�ho threadu
		if ( ( m_dwPrivilegedThreadReaderCount == 0 ) && ( m_dwReaderCount == 0 ) )
		{	// z�mek nen� zam�en pro �ten� ani mimo ani z privilegovan�ho threadu
			// nastav� ud�lost povolen� zapisovatele
			VERIFY ( m_eventWriterAllowed.SetEvent () );
		}
	}
}
