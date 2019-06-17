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
	// odemkne z�mek zapisovatel�
	// VERIFY ( m_mutexWriterLock.Unlock () );
/////////////////////////////////////////////////////// VYKRACENO

/////////////////////////////////////////////////////// VYKRACENO
	// zamkne z�mek zapisovatel�
	// if ( !m_mutexWriterLock.Lock ( dwTimeOut ) )
	// {	// nepoda�ilo se zamknout z�mek zapisovatel�
	//	// vr�t� p��znak nezam�en� z�mku
	//	return FALSE;
	// }
/////////////////////////////////////////////////////// VYKRACENO

	// zv��� po�et �ten���
	DWORD dwReaderCount = ::InterlockedIncrement ( (long *)&m_dwReaderCount );
	ASSERT ( dwReaderCount != 0 );

	// odemkne z�mek zapisovatel�
	VERIFY ( m_mutexWriterLock.Unlock () );
}

void CCacheRWLock::ReaderUnlockWriterLock()
{	
	ReaderUnlock();
	WriterLock();
}

// zamkne z�mek pro �ten��e (s timeoutem "dwTimeOut" v milisekund�ch, INFINITE=�ek� 
//		dokud se z�mek neuvoln�) (FALSE=timeout)
BOOL CCacheRWLock::ReaderLock ( DWORD dwTimeOut /*= INFINITE*/ ) 
{
	// zamkne z�mek zapisovatel�
	if ( !m_mutexWriterLock.Lock ( dwTimeOut ) )
	{	// nepoda�ilo se zamknout z�mek zapisovatel�
		// vr�t� p��znak nezam�en� z�mku
		return FALSE;
	}

	// zv��� po�et �ten���
	DWORD dwReaderCount = ::InterlockedIncrement ( (long *)&m_dwReaderCount );
	ASSERT ( dwReaderCount != 0 );

	// odemkne z�mek zapisovatel�
	VERIFY ( m_mutexWriterLock.Unlock () );

	// vr�t� p��znak �sp�n�ho zam�en� z�mku pro �ten��e
	return TRUE;
}

// odemkne z�mek pro �ten��e
void CCacheRWLock::ReaderUnlock () 
{

	// sn�� po�et �ten���
	DWORD dwReaderCount = ::InterlockedDecrement ( (long *)&m_dwReaderCount );

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
BOOL CCacheRWLock::WriterLock ( DWORD dwTimeOut /*= INFINITE*/ ) 
{

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


	// vr�t� p��znak �sp�n�ho zam�en� z�mku pro zapisovatele
	return TRUE;
}

// odemkne z�mek pro zapisovatele
void CCacheRWLock::WriterUnlock () 
{
	// odemkne z�mek zapisovatel�
	VERIFY ( m_mutexWriterLock.Unlock () );
}

