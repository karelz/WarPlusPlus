/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da pole sledovan�ch MapSquar�
 * 
 ***********************************************************/

#ifndef __SERVER_WATCHED_MAP_SQUARE_ARRAY__HEADER_INCLUDED__
#define __SERVER_WATCHED_MAP_SQUARE_ARRAY__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace

// t��da �tverce mapy na serveru hry
class CSMapSquare;

//////////////////////////////////////////////////////////////////////
// T��da pole sledovan�ch MapSquar�
class CSWatchedMapSquareArray 
{
// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CSWatchedMapSquareArray ();
	// destruktor
	~CSWatchedMapSquareArray ();

// Operace s polem sledovan�ch MapSquar�

	// vr�t� po�et MapSquar� pole
	int GetCount () const { return m_nMapSquareCount; };
	// vr�t� MapSquare pole ��slo "nIndex"
	CSMapSquare *GetAt ( int nIndex ) const 
		{ ASSERT ( nIndex < m_nMapSquareCount ); return m_pMapSquares[nIndex]; };
	// odebere MapSquare ��slo "nIndex"
	void RemoveAt ( int nIndex );
	// sma�e v�echny MapSquary
	void RemoveAll () { m_nMapSquareCount = 0; };
	// vr�t� p��znak nalezen� MapSquaru "pMapSquare" (TRUE=nalezeno)
	BOOL Find ( CSMapSquare *pMapSquare ) const;
	// p�id� MapSquare "pMapSquare" do sledovan�ch MapSquar�
	//		v�jimky: CMemoryException
	void Add ( CSMapSquare *pMapSquare );
	// odebere MapSquare "pMapSquare" ze sledovan�ch MapSquar�
	void RemoveElement ( CSMapSquare *pMapSquare );
protected:
	// zv�t�� pole sledovan�ch MapSquar� o jeden MapSquare
	//		v�jimky: CMemoryException
	void AddNew ();

// Data
private:
	// ukazatel na pole sledovan�ch MapSquar�
	CSMapSquare **m_pMapSquares;
	// po�et sledovan�ch MapSquar�
	int m_nMapSquareCount;
	// veliksot alokovan�ho pole sledovan�ch MapSquar�
	int m_nAllocatedMapSquareCount;
};

#endif //__SERVER_WATCHED_MAP_SQUARE_ARRAY__HEADER_INCLUDED__
