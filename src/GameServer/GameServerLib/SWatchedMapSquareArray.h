/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída pole sledovaných MapSquarù
 * 
 ***********************************************************/

#ifndef __SERVER_WATCHED_MAP_SQUARE_ARRAY__HEADER_INCLUDED__
#define __SERVER_WATCHED_MAP_SQUARE_ARRAY__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída ètverce mapy na serveru hry
class CSMapSquare;

//////////////////////////////////////////////////////////////////////
// Tøída pole sledovaných MapSquarù
class CSWatchedMapSquareArray 
{
// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CSWatchedMapSquareArray ();
	// destruktor
	~CSWatchedMapSquareArray ();

// Operace s polem sledovaných MapSquarù

	// vrátí poèet MapSquarù pole
	int GetCount () const { return m_nMapSquareCount; };
	// vrátí MapSquare pole èíslo "nIndex"
	CSMapSquare *GetAt ( int nIndex ) const 
		{ ASSERT ( nIndex < m_nMapSquareCount ); return m_pMapSquares[nIndex]; };
	// odebere MapSquare èíslo "nIndex"
	void RemoveAt ( int nIndex );
	// smaže všechny MapSquary
	void RemoveAll () { m_nMapSquareCount = 0; };
	// vrátí pøíznak nalezení MapSquaru "pMapSquare" (TRUE=nalezeno)
	BOOL Find ( CSMapSquare *pMapSquare ) const;
	// pøidá MapSquare "pMapSquare" do sledovaných MapSquarù
	//		výjimky: CMemoryException
	void Add ( CSMapSquare *pMapSquare );
	// odebere MapSquare "pMapSquare" ze sledovaných MapSquarù
	void RemoveElement ( CSMapSquare *pMapSquare );
protected:
	// zvìtší pole sledovaných MapSquarù o jeden MapSquare
	//		výjimky: CMemoryException
	void AddNew ();

// Data
private:
	// ukazatel na pole sledovaných MapSquarù
	CSMapSquare **m_pMapSquares;
	// poèet sledovaných MapSquarù
	int m_nMapSquareCount;
	// veliksot alokovaného pole sledovaných MapSquarù
	int m_nAllocatedMapSquareCount;
};

#endif //__SERVER_WATCHED_MAP_SQUARE_ARRAY__HEADER_INCLUDED__
