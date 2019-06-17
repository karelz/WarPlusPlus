/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da �tverce mapy na serveru hry
 * 
 ***********************************************************/

#ifndef __SERVER_MAP_SQUARE__HEADER_INCLUDED__
#define __SERVER_MAP_SQUARE__HEADER_INCLUDED__

#include "GameServerCompileSettings.h"

#include "SCivilizationBitArray.h"
#include "SUnitList.h"
#include "SUnit.h"

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace t��d

// t��da mapy
class CSMap;
// t��da grafu pro hled�n� cesty
class CSFindPathGraph;

//////////////////////////////////////////////////////////////////////
// T��da �tverce mapy na serveru hry.
class CSMapSquare 
{
	friend class CSMap;
	friend class  CSFindPathGraph;

// Metody
protected:
// Konstrukce a destrukce

	// konstruktor
	CSMapSquare ();
	// destruktor
	~CSMapSquare ();

// Inicializace a zni�en� dat objektu

	// vytvo�� MapSquare ze souboru mapy "cMapFile"
	void Create ( CArchiveFile cMapFile );
	// zni�� MapSquare
	void Delete ();

// Operace s jednotkami v MapSquaru

	// vlo�� jednotku "pUnit" do MapSquaru
	inline void InsertUnit ( CSUnit *pUnit );
	// odebere jednotku "pUnit" z MapSquaru
	inline void DeleteUnit ( CSUnit *pUnit );
	// zjist�, je-li civilizace "dwCivilizationIndex" v MapSquaru sama
	inline BOOL IsExclusive ( DWORD dwCivilizationIndex ) 
		{ return m_cCivilizations.IsExclusive ( dwCivilizationIndex ); };
	// zjist ukazatel na seznam jednotek MapSquaru
	CSUnitList *GetUnitList () { return &m_cUnitList; };
	// aktualizuje MapSquare po p�id�n� jednotky "pUnit"
	inline void UnitInserted ( CSUnit *pUnit );
	// aktualizuje MapSquare po odebr�n� jednotky "pUnit"
	void UnitDeleted ( CSUnit *pUnit );

// Operace s jednotkami v MapSquarech

	// vytvo�� mapy jednotek MainLoop threadu a FindPathLoop threadu
	static void CreateUnitMaps ();
	// zni�� mapu jednotek MainLoop threadu a FindPathLoop threadu
	static void DeleteUnitMaps ();

	// zjist�, lze-li jednotku "pUnit" um�stit na pozici "pointPosition" z MainLoop 
	//		threadu (je-li nastaven p��znak "bMapSquaresLocked", nezamyk� MapSquary)
	static BOOL MainLoopCanPlaceUnit ( CSUnit *pUnit, CPointDW pointPosition, 
		BOOL bMapSquaresLocked );
	// ukon�� um�s�ov�n� jednotky z MainLoop threadu
	static void MainLoopFinishPlacingUnit ();

	// zjist�, lze-li jednotku "pUnit" um�stit na pozici "pointPosition" z FindPathLoop 
	//		threadu (je-li nastaven p��znak "bMapSquaresLocked", nezamyk� MapSquary)
	static BOOL FindPathLoopCanPlaceUnit ( CSUnit *pUnit, CPointDW pointPosition, 
		BOOL bMapSquaresLocked );
	// ukon�� um�s�ov�n� jednotky z FindPathLoop threadu
	static void FindPathLoopFinishPlacingUnit ();

// Informace o MapSquarech

	// vr�t� index MapSquaru
	CPointDW GetIndex ();

// Debuggovac� informace
private:
#ifdef _DEBUG
	// zkontroluje spr�vn� data objektu (TRUE=OK)
	BOOL CheckValidData ();
	// zkontroluje ne�kodn� data objektu (TRUE=OK)
	BOOL CheckEmptyData ();
#endif //_DEBUG

// Data
private:
// Informace o MapSquaru

	// pole p��znak� um�st�n� civilizac� v MapSquaru
	CSCivilizationBitArray m_cCivilizations;
	// pole p��znak� civilizac� sleduj�c�ch tento MapSquare
	CSCivilizationBitArray m_cWatchingCivilizations;
	// seznam jednotek um�st�n�ch v MapSquaru
	CSUnitList m_cUnitList;

// Informace o jednotk�ch v MapSquarech

	// pole ukazatel� na MapSquary mapy jednotek MainLoop threadu
	static signed char **m_aMainLoopUnitMapMapSquares;
	// memory pool MapSquar� mapy jednotek MainLoop threadu
	static CMemoryPool m_cMainLoopUnitMapMapSquareMemoryPool;
	// z�sobn�k index� MapSquar� mapy jednotek MainLoop threadu
	static CSelfPooledStack<DWORD> m_cMainLoopUnitMapMapSquareIndexStack;

	// pole ukazatel� na MapSquary mapy jednotek FindPathLoop threadu
	static signed char **m_aFindPathLoopUnitMapMapSquares;
	// memory pool MapSquar� mapy jednotek FindPathLoop threadu
	static CMemoryPool m_cFindPathLoopUnitMapMapSquareMemoryPool;
	// z�sobn�k index� MapSquar� mapy jednotek FindPathLoop threadu
	static CSelfPooledStack<DWORD> m_cFindPathLoopUnitMapMapSquareIndexStack;

	// offsety okoln�ch pozic
	static CPointDW m_aSurroundingPositionOffset[9];
};

//////////////////////////////////////////////////////////////////////
// Inline metody t��dy CSMapSquare

//////////////////////////////////////////////////////////////////////
// Operace s jednotkami
//////////////////////////////////////////////////////////////////////

// vlo�� jednotku "pUnit" do MapSquaru
inline void CSMapSquare::InsertUnit ( CSUnit *pUnit ) 
{
	ASSERT ( pUnit != NULL );

	// p�id� jednotku do seznamu jednotek MapSquaru
	m_cUnitList.Add ( pUnit );
	// nastav� p��znak um�st�n� civilizace v MapSquaru
	m_cCivilizations.SetAt ( pUnit->GetCivilizationIndex () );
}

// odebere jednotku "pUnit" z MapSquaru
inline void CSMapSquare::DeleteUnit ( CSUnit *pUnit ) 
{
	ASSERT ( pUnit != NULL );

	// najde jednotku v seznamu jednotek MapSquaru
	POSITION posUnit = m_cUnitList.Find ( pUnit );
	ASSERT ( posUnit != NULL );
	ASSERT ( !m_cUnitList.IsEmpty ( posUnit ) );

	// odebere jednotku "pUnit" ze seznamu jednotek MapSquaru
	VERIFY ( m_cUnitList.RemovePosition ( posUnit ) == pUnit );

	// aktualizuje MapSquare po odebr�n� jednotky "pUnit"
	UnitDeleted ( pUnit );
}

// aktualizuje MapSquare po p�id�n� jednotky "pUnit"
inline void CSMapSquare::UnitInserted ( CSUnit *pUnit ) 
{
	ASSERT ( pUnit != NULL );

	// nastav� p��znak um�st�n� civilizace v MapSquaru
	m_cCivilizations.SetAt ( pUnit->GetCivilizationIndex () );
}

#endif //__SERVER_MAP_SQUARE__HEADER_INCLUDED__
