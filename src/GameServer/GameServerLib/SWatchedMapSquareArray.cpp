/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída pole sledovanıch MapSquarù
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SWatchedMapSquareArray.h"
#include "SMapSquare.h"

#include <memory.h>

#define WATCHED_MAP_SQUARE_BLOCK_SIZE	20

/////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSWatchedMapSquareArray::CSWatchedMapSquareArray () 
{
	// zneškodní data objektu
	m_pMapSquares = NULL;
	m_nMapSquareCount = 0;
	m_nAllocatedMapSquareCount = 0;
}

// destruktor
CSWatchedMapSquareArray::~CSWatchedMapSquareArray () 
{
	// znièí pøípadnì alokovaná data
	if ( m_pMapSquares != NULL )
	{	// pole MapSquarù je alokováno
		ASSERT ( m_nAllocatedMapSquareCount > 0 );
		// znièí pole MapSquarù
		free ( m_pMapSquares );
	}
}

//////////////////////////////////////////////////////////////////////
// Operace s polem sledovanıch MapSquarù
//////////////////////////////////////////////////////////////////////

// odebere MapSquare èíslo "nIndex"
void CSWatchedMapSquareArray::RemoveAt ( int nIndex ) 
{
	ASSERT ( nIndex < m_nMapSquareCount );

	// posune konec pole od pozice "nIndex", aby byl smazán mazanı MapSquare
	(void) memmove ( m_pMapSquares + nIndex, m_pMapSquares + nIndex + 1, 
		( m_nMapSquareCount - nIndex - 1 ) * sizeof ( CSMapSquare * ) );
	// zmenší velikost pole MapSquarù
	m_nMapSquareCount--;

	// ukonèí mazání MapSquaru
	return ;
}

// vrátí pøíznak nalezení MapSquaru "pMapSquare" (TRUE=nalezeno)
BOOL CSWatchedMapSquareArray::Find ( CSMapSquare *pMapSquare ) const 
{
	// zjistí, je-li pole MapSquarù prázdné
	if ( m_nMapSquareCount == 0 )
	{	// pole MapSquarù je prázdné
		return FALSE;
	}

// algoritmus binárního vyhledávání v uspoøádaném poli

	// první prvek prohledávaného úseku pole
	int nFirst = 0;
	// poslení prvek prohledávaného úseku pole
	int nLast = m_nMapSquareCount - 1;

	// najde umístìní prvku v poli
	while ( nFirst != nLast )
	{
		// prostøední prvek prohledávaného úseku pole
		int nMiddle = ( nLast + nFirst ) / 2;
		// zjistí, jedná-li se o hledanı prvek
		if ( m_pMapSquares[nMiddle] >= pMapSquare )
		{	// jedná se vìtší prvek (pøípadnì o hledanı prvek)
			// posune konec úseku na vìtší nebo rovnı prvek
			nLast = nMiddle;
		}
		else
		{	// jedná se o menší prvek
			// posune zaèátek úseku za menší prvek
			nFirst = nMiddle + 1;
		}
	}

	// otestuje, je-li algoritmus binárního vyhledávání v poøádku
	ASSERT ( ( m_pMapSquares[nFirst] >= pMapSquare ) ? 
		( ( nFirst == 0 ) || ( m_pMapSquares[nFirst - 1] < pMapSquare ) ) : 
		( ( nFirst + 1 == m_nMapSquareCount ) || ( m_pMapSquares[nFirst + 1] >= pMapSquare ) ) );

	// vrátí pøíznak nalezení prvku
	return ( m_pMapSquares[nFirst] == pMapSquare );
}

// pøidá MapSquare "pMapSquare" do sledovanıch MapSquarù
//		vıjimky: CMemoryException
void CSWatchedMapSquareArray::Add ( CSMapSquare *pMapSquare ) 
{
	// zjistí, je-li pole MapSquarù prázdné
	if ( m_nMapSquareCount == 0 )
	{	// pole MapSquarù je prázdné
		// nechá zvìtšit pole MapSquarù o jeden prvek
		AddNew ();
		// umístí pøidávanı MapSquare do pole sledovanıch MapSquarù
		m_pMapSquares[0] = pMapSquare;
		// ukonèí pøidávání MapSquaru
		return;
	}

// algoritmus binárního vyhledávání v uspoøádaném poli

	// první prvek prohledávaného úseku pole
	int nFirst = 0;
	// poslení prvek prohledávaného úseku pole
	int nLast = m_nMapSquareCount - 1;

	// najde umístìní prvku v poli
	while ( nFirst != nLast )
	{
		// prostøední prvek prohledávaného úseku pole
		int nMiddle = ( nLast + nFirst ) / 2;
		// zjistí, jedná-li se o hledanı prvek
		if ( m_pMapSquares[nMiddle] >= pMapSquare )
		{	// jedná se vìtší prvek (pøípadnì o hledanı prvek)
			// posune konec úseku na vìtší nebo rovnı prvek
			nLast = nMiddle;
		}
		else
		{	// jedná se o menší prvek
			// posune zaèátek úseku za menší prvek
			nFirst = nMiddle + 1;
		}
	}

	// otestuje, je-li algoritmus binárního vyhledávání v poøádku
	ASSERT ( ( m_pMapSquares[nFirst] >= pMapSquare ) ? 
		( ( nFirst == 0 ) || ( m_pMapSquares[nFirst - 1] < pMapSquare ) ) : 
		( ( nFirst + 1 == m_nMapSquareCount ) || ( m_pMapSquares[nFirst + 1] >= pMapSquare ) ) );

	// nechá zvìtšit pole MapSquarù o jeden prvek
	AddNew ();
	// zjistí, jedná-li se o poslední prvek
	if ( m_pMapSquares[nFirst] < pMapSquare )
	{	// jedná se o poslední prvek
		ASSERT ( nFirst + 2 == m_nMapSquareCount );
		// aktualizuje pozici v poli, kam se má pøidat novı prvek
		nFirst++;
	}
	else
	{
		ASSERT ( nFirst + 2 <= m_nMapSquareCount );
		// posune konec pole od pozice "nFirst", aby bylo moné vloit MapSquare
		(void) memmove ( m_pMapSquares + nFirst + 1, m_pMapSquares + nFirst, 
			( m_nMapSquareCount - nFirst - 1 ) * sizeof ( CSMapSquare * ) );
	}

	// pøidá prvek do pole MapSquarù
	m_pMapSquares[nFirst] = pMapSquare;

	// ukonèí pøidávání MapSquaru
	return;
}

// odebere MapSquare "pMapSquare" ze sledovanıch MapSquarù
void CSWatchedMapSquareArray::RemoveElement ( CSMapSquare *pMapSquare ) 
{
	ASSERT ( m_nMapSquareCount > 0 );

// algoritmus binárního vyhledávání v uspoøádaném poli

	// první prvek prohledávaného úseku pole
	int nFirst = 0;
	// poslení prvek prohledávaného úseku pole
	int nLast = m_nMapSquareCount - 1;

	// najde umístìní prvku v poli
	while ( nFirst != nLast )
	{
		// prostøední prvek prohledávaného úseku pole
		int nMiddle = ( nLast + nFirst ) / 2;
		// zjistí, jedná-li se o hledanı prvek
		if ( m_pMapSquares[nMiddle] >= pMapSquare )
		{	// jedná se vìtší prvek (pøípadnì o hledanı prvek)
			// posune konec úseku na vìtší nebo rovnı prvek
			nLast = nMiddle;
		}
		else
		{	// jedná se o menší prvek
			// posune zaèátek úseku za menší prvek
			nFirst = nMiddle + 1;
		}
	}

	// otestuje, je-li algoritmus binárního vyhledávání v poøádku
	ASSERT ( ( m_pMapSquares[nFirst] >= pMapSquare ) ? 
		( ( nFirst == 0 ) || ( m_pMapSquares[nFirst - 1] < pMapSquare ) ) : 
		( ( nFirst + 1 == m_nMapSquareCount ) || ( m_pMapSquares[nFirst + 1] >= pMapSquare ) ) );

	// otestuje, byl-li skuteènì nalezen hledanı prvek
	ASSERT ( m_pMapSquares[nFirst] == pMapSquare );

	// posune konec pole od pozice "nFirst", aby byl smazán mazanı MapSquare
	(void) memmove ( m_pMapSquares + nFirst, m_pMapSquares + nFirst + 1, 
		( m_nMapSquareCount - nFirst - 1 ) * sizeof ( CSMapSquare * ) );
	// zmenší velikost pole MapSquarù
	m_nMapSquareCount--;

	// ukonèí mazání MapSquaru
	return ;
}

// zvìtší pole sledovanıch MapSquarù o jeden MapSquare
//		vıjimky: CMemoryException
void CSWatchedMapSquareArray::AddNew () 
{
	ASSERT ( WATCHED_MAP_SQUARE_BLOCK_SIZE > 0 );

	// zjistí, je-li potøeba zvìtšit alokovanou pamì
	if ( m_nMapSquareCount == m_nAllocatedMapSquareCount )
	{	// je potøeba zvìtšit alokovanou pamì
		// zvìtší alokovanou pamì
		if ( ( m_pMapSquares = (CSMapSquare **) realloc ( m_pMapSquares, sizeof ( CSMapSquare * ) * 
			( m_nAllocatedMapSquareCount += WATCHED_MAP_SQUARE_BLOCK_SIZE ) ) ) == NULL )
		{	// nepovedlo se zvìtšit alokovanou pamì
			AfxThrowMemoryException ();
		}
	}
	// zvìtší velikost pole
	m_nMapSquareCount++;
}
