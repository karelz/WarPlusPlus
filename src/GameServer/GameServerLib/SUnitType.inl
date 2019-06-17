/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída typu jednotky pro jednu civilizaci
 * 
 ***********************************************************/

#ifndef __SERVER_UNIT_TYPE__INLINE_INCLUDED__
#define __SERVER_UNIT_TYPE__INLINE_INCLUDED__

#include "SUnitType.h"
#include "SMap.h"

//////////////////////////////////////////////////////////////////////
// Operace se zámkem typu jednotky
//////////////////////////////////////////////////////////////////////

// zamkne zámek pro ètenáøe (s timeoutem "dwTimeOut" v milisekundách, INFINITE=èeká 
//		dokud se zámek neuvolní) (nemá smysl volat z MainLoop threadu) (FALSE=timeout)
inline BOOL CSUnitType::ReaderLock ( DWORD dwTimeOut ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );

	return m_cLock.ReaderLock ( dwTimeOut );
};

// odemkne zámek pro ètenáøe
inline void CSUnitType::ReaderUnlock () 
{
	m_cLock.ReaderUnlock ();
};

// zamkne zámek pro zapisovatele (s timeoutem "dwTimeOut" v milisekundách, 
//		INFINITE=èeká dokud se zámek neuvolní - skuteèná doba timeoutu mùže být až 
//		dvojnásobná) (nelze volat mimo MainLoop thread) (FALSE=timeout)
inline BOOL CSUnitType::WriterLock ( DWORD dwTimeOut ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );

	return m_cLock.WriterLock ( dwTimeOut );
};

// odemkne zámek pro zapisovatele
inline void CSUnitType::WriterUnlock () 
{
	m_cLock.WriterUnlock ();
};

#endif //__SERVER_UNIT_TYPE__INLINE_INCLUDED__
