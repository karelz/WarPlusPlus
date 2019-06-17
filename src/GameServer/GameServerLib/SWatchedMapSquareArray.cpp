/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da pole sledovan�ch MapSquar�
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
	// zne�kodn� data objektu
	m_pMapSquares = NULL;
	m_nMapSquareCount = 0;
	m_nAllocatedMapSquareCount = 0;
}

// destruktor
CSWatchedMapSquareArray::~CSWatchedMapSquareArray () 
{
	// zni�� p��padn� alokovan� data
	if ( m_pMapSquares != NULL )
	{	// pole MapSquar� je alokov�no
		ASSERT ( m_nAllocatedMapSquareCount > 0 );
		// zni�� pole MapSquar�
		free ( m_pMapSquares );
	}
}

//////////////////////////////////////////////////////////////////////
// Operace s polem sledovan�ch MapSquar�
//////////////////////////////////////////////////////////////////////

// odebere MapSquare ��slo "nIndex"
void CSWatchedMapSquareArray::RemoveAt ( int nIndex ) 
{
	ASSERT ( nIndex < m_nMapSquareCount );

	// posune konec pole od pozice "nIndex", aby byl smaz�n mazan� MapSquare
	(void) memmove ( m_pMapSquares + nIndex, m_pMapSquares + nIndex + 1, 
		( m_nMapSquareCount - nIndex - 1 ) * sizeof ( CSMapSquare * ) );
	// zmen�� velikost pole MapSquar�
	m_nMapSquareCount--;

	// ukon�� maz�n� MapSquaru
	return ;
}

// vr�t� p��znak nalezen� MapSquaru "pMapSquare" (TRUE=nalezeno)
BOOL CSWatchedMapSquareArray::Find ( CSMapSquare *pMapSquare ) const 
{
	// zjist�, je-li pole MapSquar� pr�zdn�
	if ( m_nMapSquareCount == 0 )
	{	// pole MapSquar� je pr�zdn�
		return FALSE;
	}

// algoritmus bin�rn�ho vyhled�v�n� v uspo��dan�m poli

	// prvn� prvek prohled�van�ho �seku pole
	int nFirst = 0;
	// poslen� prvek prohled�van�ho �seku pole
	int nLast = m_nMapSquareCount - 1;

	// najde um�st�n� prvku v poli
	while ( nFirst != nLast )
	{
		// prost�edn� prvek prohled�van�ho �seku pole
		int nMiddle = ( nLast + nFirst ) / 2;
		// zjist�, jedn�-li se o hledan� prvek
		if ( m_pMapSquares[nMiddle] >= pMapSquare )
		{	// jedn� se v�t�� prvek (p��padn� o hledan� prvek)
			// posune konec �seku na v�t�� nebo rovn� prvek
			nLast = nMiddle;
		}
		else
		{	// jedn� se o men�� prvek
			// posune za��tek �seku za men�� prvek
			nFirst = nMiddle + 1;
		}
	}

	// otestuje, je-li algoritmus bin�rn�ho vyhled�v�n� v po��dku
	ASSERT ( ( m_pMapSquares[nFirst] >= pMapSquare ) ? 
		( ( nFirst == 0 ) || ( m_pMapSquares[nFirst - 1] < pMapSquare ) ) : 
		( ( nFirst + 1 == m_nMapSquareCount ) || ( m_pMapSquares[nFirst + 1] >= pMapSquare ) ) );

	// vr�t� p��znak nalezen� prvku
	return ( m_pMapSquares[nFirst] == pMapSquare );
}

// p�id� MapSquare "pMapSquare" do sledovan�ch MapSquar�
//		v�jimky: CMemoryException
void CSWatchedMapSquareArray::Add ( CSMapSquare *pMapSquare ) 
{
	// zjist�, je-li pole MapSquar� pr�zdn�
	if ( m_nMapSquareCount == 0 )
	{	// pole MapSquar� je pr�zdn�
		// nech� zv�t�it pole MapSquar� o jeden prvek
		AddNew ();
		// um�st� p�id�van� MapSquare do pole sledovan�ch MapSquar�
		m_pMapSquares[0] = pMapSquare;
		// ukon�� p�id�v�n� MapSquaru
		return;
	}

// algoritmus bin�rn�ho vyhled�v�n� v uspo��dan�m poli

	// prvn� prvek prohled�van�ho �seku pole
	int nFirst = 0;
	// poslen� prvek prohled�van�ho �seku pole
	int nLast = m_nMapSquareCount - 1;

	// najde um�st�n� prvku v poli
	while ( nFirst != nLast )
	{
		// prost�edn� prvek prohled�van�ho �seku pole
		int nMiddle = ( nLast + nFirst ) / 2;
		// zjist�, jedn�-li se o hledan� prvek
		if ( m_pMapSquares[nMiddle] >= pMapSquare )
		{	// jedn� se v�t�� prvek (p��padn� o hledan� prvek)
			// posune konec �seku na v�t�� nebo rovn� prvek
			nLast = nMiddle;
		}
		else
		{	// jedn� se o men�� prvek
			// posune za��tek �seku za men�� prvek
			nFirst = nMiddle + 1;
		}
	}

	// otestuje, je-li algoritmus bin�rn�ho vyhled�v�n� v po��dku
	ASSERT ( ( m_pMapSquares[nFirst] >= pMapSquare ) ? 
		( ( nFirst == 0 ) || ( m_pMapSquares[nFirst - 1] < pMapSquare ) ) : 
		( ( nFirst + 1 == m_nMapSquareCount ) || ( m_pMapSquares[nFirst + 1] >= pMapSquare ) ) );

	// nech� zv�t�it pole MapSquar� o jeden prvek
	AddNew ();
	// zjist�, jedn�-li se o posledn� prvek
	if ( m_pMapSquares[nFirst] < pMapSquare )
	{	// jedn� se o posledn� prvek
		ASSERT ( nFirst + 2 == m_nMapSquareCount );
		// aktualizuje pozici v poli, kam se m� p�idat nov� prvek
		nFirst++;
	}
	else
	{
		ASSERT ( nFirst + 2 <= m_nMapSquareCount );
		// posune konec pole od pozice "nFirst", aby bylo mo�n� vlo�it MapSquare
		(void) memmove ( m_pMapSquares + nFirst + 1, m_pMapSquares + nFirst, 
			( m_nMapSquareCount - nFirst - 1 ) * sizeof ( CSMapSquare * ) );
	}

	// p�id� prvek do pole MapSquar�
	m_pMapSquares[nFirst] = pMapSquare;

	// ukon�� p�id�v�n� MapSquaru
	return;
}

// odebere MapSquare "pMapSquare" ze sledovan�ch MapSquar�
void CSWatchedMapSquareArray::RemoveElement ( CSMapSquare *pMapSquare ) 
{
	ASSERT ( m_nMapSquareCount > 0 );

// algoritmus bin�rn�ho vyhled�v�n� v uspo��dan�m poli

	// prvn� prvek prohled�van�ho �seku pole
	int nFirst = 0;
	// poslen� prvek prohled�van�ho �seku pole
	int nLast = m_nMapSquareCount - 1;

	// najde um�st�n� prvku v poli
	while ( nFirst != nLast )
	{
		// prost�edn� prvek prohled�van�ho �seku pole
		int nMiddle = ( nLast + nFirst ) / 2;
		// zjist�, jedn�-li se o hledan� prvek
		if ( m_pMapSquares[nMiddle] >= pMapSquare )
		{	// jedn� se v�t�� prvek (p��padn� o hledan� prvek)
			// posune konec �seku na v�t�� nebo rovn� prvek
			nLast = nMiddle;
		}
		else
		{	// jedn� se o men�� prvek
			// posune za��tek �seku za men�� prvek
			nFirst = nMiddle + 1;
		}
	}

	// otestuje, je-li algoritmus bin�rn�ho vyhled�v�n� v po��dku
	ASSERT ( ( m_pMapSquares[nFirst] >= pMapSquare ) ? 
		( ( nFirst == 0 ) || ( m_pMapSquares[nFirst - 1] < pMapSquare ) ) : 
		( ( nFirst + 1 == m_nMapSquareCount ) || ( m_pMapSquares[nFirst + 1] >= pMapSquare ) ) );

	// otestuje, byl-li skute�n� nalezen hledan� prvek
	ASSERT ( m_pMapSquares[nFirst] == pMapSquare );

	// posune konec pole od pozice "nFirst", aby byl smaz�n mazan� MapSquare
	(void) memmove ( m_pMapSquares + nFirst, m_pMapSquares + nFirst + 1, 
		( m_nMapSquareCount - nFirst - 1 ) * sizeof ( CSMapSquare * ) );
	// zmen�� velikost pole MapSquar�
	m_nMapSquareCount--;

	// ukon�� maz�n� MapSquaru
	return ;
}

// zv�t�� pole sledovan�ch MapSquar� o jeden MapSquare
//		v�jimky: CMemoryException
void CSWatchedMapSquareArray::AddNew () 
{
	ASSERT ( WATCHED_MAP_SQUARE_BLOCK_SIZE > 0 );

	// zjist�, je-li pot�eba zv�t�it alokovanou pam�
	if ( m_nMapSquareCount == m_nAllocatedMapSquareCount )
	{	// je pot�eba zv�t�it alokovanou pam�
		// zv�t�� alokovanou pam�
		if ( ( m_pMapSquares = (CSMapSquare **) realloc ( m_pMapSquares, sizeof ( CSMapSquare * ) * 
			( m_nAllocatedMapSquareCount += WATCHED_MAP_SQUARE_BLOCK_SIZE ) ) ) == NULL )
		{	// nepovedlo se zv�t�it alokovanou pam�
			AfxThrowMemoryException ();
		}
	}
	// zv�t�� velikost pole
	m_nMapSquareCount++;
}
