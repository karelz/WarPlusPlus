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

#ifndef __PRIVILEGED_THREAD_READ_WRITE_LOCK__HEADER_INCLUDED__
#define __PRIVILEGED_THREAD_READ_WRITE_LOCK__HEADER_INCLUDED__

#include <afxmt.h>

#ifndef DWORD_MAX
	#define DWORD_MAX ((DWORD)0xffffffff)
#endif //DWORD_MAX

//////////////////////////////////////////////////////////////////////
// T��da z�mku s privilegovan�m threadem - umo��uje p��stup v�ce 
//		�ten���m a v�ce zapisovatel�m. Najednou m��e ��st v�ce �ten���, 
//		ale zapisovat m��e v�dy jedin� zapisovatel a nav�c bez �ten���. 
//		Privilegovan� zapisovatel� a �ten��i mohou b�t splolu libovoln� 
//		zkombinov�ni.
class CPrivilegedThreadReadWriteLock 
{
// Metody
public:
// Konstrukce a destukce

	// konstruktor
	CPrivilegedThreadReadWriteLock ();
	// destruktor
	~CPrivilegedThreadReadWriteLock ();

// Operace se z�mkem

	// zamkne z�mek pro �ten��e (s timeoutem "dwTimeOut" v milisekund�ch, INFINITE=�ek� 
	//		dokud se z�mek neuvoln�; skute�n� doba timeoutu m��e b�t i del��) 
	//		(FALSE=timeout)
	BOOL ReaderLock ( DWORD dwTimeOut = INFINITE );
	// odemkne z�mek pro �ten��e
	void ReaderUnlock ();
	// zamkne z�mek pro zapisovatele (s timeoutem "dwTimeOut" v milisekund�ch, 
	//		INFINITE=�ek� dokud se z�mek neuvoln�; skute�n� doba timeoutu m��e b�t 
	//		a� dvojn�sobn�) (FALSE=timeout)
	BOOL WriterLock ( DWORD dwTimeOut = INFINITE );
	// odemkne z�mek pro zapisovatele
#ifdef _DEBUG
	void WriterUnlock ();
#else //!_DEBUG
	inline void WriterUnlock ();
#endif //!_DEBUG
	// zamkne z�mek pro �ten��e z privilegovan�ho threadu (s timeoutem "dwTimeOut" 
	//		v milisekund�ch, INFINITE=�ek� dokud se z�mek neuvoln�) (FALSE=timeout)
	BOOL PrivilegedThreadReaderLock ( DWORD dwTimeOut = INFINITE );
	// odemkne z�mek pro �ten��e zprivilegovan�ho threadu
	void PrivilegedThreadReaderUnlock ();
	// zamkne z�mek pro zapisovatele z privilegovan�ho threadu (s timeoutem "dwTimeOut" 
	//		v milisekund�ch, INFINITE=�ek� dokud se z�mek neuvoln�; skute�n� doba timeoutu 
	//		m��e b�t i del��) (FALSE=timeout)
	BOOL PrivilegedThreadWriterLock ( DWORD dwTimeOut = INFINITE );
	// odemkne z�mek pro zapisovatele z privilegovan�ho threadu
	void PrivilegedThreadWriterUnlock ();

#ifdef _DEBUG
// Debuggovac� metody
public:
	// nastav� ID privilegovan�ho threadu
	inline void SetPrivilegedThreadID ( DWORD dwPrivilegedThreadID );
private:
	// zjist�, jedn�-li se o privilegovan� thread (TRUE=jedn� se o privilegovan� thread)
	inline BOOL IsPrivilegedThread ();
#endif //_DEBUG

// Data
private:
	// z�mek zapisovatel�
	CMutex m_mutexWriterLock;
	// po�et �ten���
	DWORD m_dwReaderCount;
	// po�et �ten��� z privilegovan�ho threadu
	DWORD m_dwPrivilegedThreadReaderCount;
	// po�et zapisovatel� z privilegovan�ho threadu
	DWORD m_dwPrivilegedThreadWriterCount;
	// ud�lost povolen� zapisovatele
	CEvent m_eventWriterAllowed;
	// ud�lost povolen� �ten��e
	CEvent m_eventReaderAllowed;
	// ud�lost povolen� zapisovatele z privilegovan�ho threadu
	CEvent m_eventPrivilegedThreadWriterAllowed;

#ifdef _DEBUG
// Debuggovac� data
private:
	// ID privilegovan�ho threadu
	DWORD m_dwPrivilegedThreadID;

	// prvek seznamu �ten���
	struct SReaderListMember 
	{
		// dal�� prvek seznamu �ten���
		struct SReaderListMember *pNext;
		// ID threadu �ten��e
		DWORD dwThreadID;
	};
	// z�mek seznamu �ten���
	CMutex m_mutexReaderListLock;
	// ukazatel na prvn� prvek seznamu �ten���
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
	// inicializuje ID privilegovan�ho threadu
	m_dwPrivilegedThreadID = DWORD_MAX;
	// inicializuje seznam �ten���
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
// Operace se z�mkem
//////////////////////////////////////////////////////////////////////

#ifndef _DEBUG

// odemkne z�mek pro zapisovatele
inline void CPrivilegedThreadReadWriteLock::WriterUnlock () 
{
	ASSERT ( !IsPrivilegedThread () );

	// odemkne z�mek zapisovatel�
	VERIFY ( m_mutexWriterLock.Unlock () );
}

#endif //!_DEBUG

#ifdef _DEBUG

//////////////////////////////////////////////////////////////////////
// Debuggovac� metody
//////////////////////////////////////////////////////////////////////

// nastav� ID privilegovan�ho threadu
inline void CPrivilegedThreadReadWriteLock::SetPrivilegedThreadID ( 
	DWORD dwPrivilegedThreadID ) 
{
//	ASSERT ( m_dwPrivilegedThreadID == DWORD_MAX );
	ASSERT ( dwPrivilegedThreadID != DWORD_MAX );

	// nastav� ID privilegovan�ho threadu
	m_dwPrivilegedThreadID = dwPrivilegedThreadID;
}

// zjist�, jedn�-li se o privilegovan� thread (TRUE=jedn� se o privilegovan� thread)
inline BOOL CPrivilegedThreadReadWriteLock::IsPrivilegedThread () 
{
	ASSERT ( m_dwPrivilegedThreadID != DWORD_MAX );

	// vr�t� p��znak privilegovan�ho threadu
	return ( m_dwPrivilegedThreadID == GetCurrentThreadId () );
}

#endif //_DEBUG

#endif //__PRIVILEGED_THREAD_READ_WRITE_LOCK__HEADER_INCLUDED__
