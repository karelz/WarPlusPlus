/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da typu jednotky pro jednu civilizaci
 * 
 ***********************************************************/

#ifndef __SERVER_UNIT_TYPE__INLINE_INCLUDED__
#define __SERVER_UNIT_TYPE__INLINE_INCLUDED__

#include "SUnitType.h"
#include "SMap.h"

//////////////////////////////////////////////////////////////////////
// Operace se z�mkem typu jednotky
//////////////////////////////////////////////////////////////////////

// zamkne z�mek pro �ten��e (s timeoutem "dwTimeOut" v milisekund�ch, INFINITE=�ek� 
//		dokud se z�mek neuvoln�) (nem� smysl volat z MainLoop threadu) (FALSE=timeout)
inline BOOL CSUnitType::ReaderLock ( DWORD dwTimeOut ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );

	return m_cLock.ReaderLock ( dwTimeOut );
};

// odemkne z�mek pro �ten��e
inline void CSUnitType::ReaderUnlock () 
{
	m_cLock.ReaderUnlock ();
};

// zamkne z�mek pro zapisovatele (s timeoutem "dwTimeOut" v milisekund�ch, 
//		INFINITE=�ek� dokud se z�mek neuvoln� - skute�n� doba timeoutu m��e b�t a� 
//		dvojn�sobn�) (nelze volat mimo MainLoop thread) (FALSE=timeout)
inline BOOL CSUnitType::WriterLock ( DWORD dwTimeOut ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );

	return m_cLock.WriterLock ( dwTimeOut );
};

// odemkne z�mek pro zapisovatele
inline void CSUnitType::WriterUnlock () 
{
	m_cLock.WriterUnlock ();
};

#endif //__SERVER_UNIT_TYPE__INLINE_INCLUDED__
