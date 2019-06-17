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

#ifndef __READ_WRITE_LOCK__HEADER_INCLUDED__
#define __READ_WRITE_LOCK__HEADER_INCLUDED__

#include <afxmt.h>

//////////////////////////////////////////////////////////////////////
// T��da z�mku - umo��uje p��stup v�ce �ten���m a v�ce zapisovatel�m. 
//		Najednou m��e ��st v�ce �ten���, ale zapisovat m��e v�dy jedin� 
//		zapisovatel a nav�c bez �ten���.
class CReadWriteLock 
{
// Metody
public:
// Konstrukce a destukce

	// konstruktor
	CReadWriteLock ();
	// destruktor
	~CReadWriteLock ();

// Operace se z�mkem

	// zamkne z�mek pro �ten��e (s timeoutem "dwTimeOut" v milisekund�ch, INFINITE=�ek� 
	//		dokud se z�mek neuvoln�) (FALSE=timeout)
	BOOL ReaderLock ( DWORD dwTimeOut = INFINITE );
	// odemkne z�mek pro �ten��e
	void ReaderUnlock ();
	// zamkne z�mek pro zapisovatele (s timeoutem "dwTimeOut" v milisekund�ch, 
	//		INFINITE=�ek� dokud se z�mek neuvoln� - skute�n� doba timeoutu m��e b�t
	//		a� dvojn�sobn�) (FALSE=timeout)
	BOOL WriterLock ( DWORD dwTimeOut = INFINITE );
	// odemkne z�mek pro zapisovatele
	void WriterUnlock ();

// Data
protected:
	// z�mek zapisovatel�
	CCriticalSection m_mutexWriterLock;
	// po�et �ten���
	DWORD m_dwReaderCount;
	// ud�lost odchodu posledn�ho �ten��e
	CEvent m_eventLastReaderLeft;

#ifdef _DEBUG
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
inline CReadWriteLock::CReadWriteLock () 
{
	// inicializuje po�et zam�en�ch �ten���
	m_dwReaderCount = 0;

#ifdef _DEBUG
	// inicializuje seznam �ten���
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
// Operace se z�mkem
//////////////////////////////////////////////////////////////////////

#ifndef _DEBUG

// odemkne z�mek pro zapisovatele
inline void CReadWriteLock::WriterUnlock () 
{
	// odemkne z�mek zapisovatel�
	VERIFY ( m_mutexWriterLock.Unlock () );
}

#endif //!_DEBUG

#endif //__READ_WRITE_LOCK__HEADER_INCLUDED__
