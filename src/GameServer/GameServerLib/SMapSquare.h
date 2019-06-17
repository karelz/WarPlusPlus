/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída ètverce mapy na serveru hry
 * 
 ***********************************************************/

#ifndef __SERVER_MAP_SQUARE__HEADER_INCLUDED__
#define __SERVER_MAP_SQUARE__HEADER_INCLUDED__

#include "GameServerCompileSettings.h"

#include "SCivilizationBitArray.h"
#include "SUnitList.h"
#include "SUnit.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace tøíd

// tøída mapy
class CSMap;
// tøída grafu pro hledání cesty
class CSFindPathGraph;

//////////////////////////////////////////////////////////////////////
// Tøída ètverce mapy na serveru hry.
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

// Inicializace a znièení dat objektu

	// vytvoøí MapSquare ze souboru mapy "cMapFile"
	void Create ( CArchiveFile cMapFile );
	// znièí MapSquare
	void Delete ();

// Operace s jednotkami v MapSquaru

	// vloí jednotku "pUnit" do MapSquaru
	inline void InsertUnit ( CSUnit *pUnit );
	// odebere jednotku "pUnit" z MapSquaru
	inline void DeleteUnit ( CSUnit *pUnit );
	// zjistí, je-li civilizace "dwCivilizationIndex" v MapSquaru sama
	inline BOOL IsExclusive ( DWORD dwCivilizationIndex ) 
		{ return m_cCivilizations.IsExclusive ( dwCivilizationIndex ); };
	// zjist ukazatel na seznam jednotek MapSquaru
	CSUnitList *GetUnitList () { return &m_cUnitList; };
	// aktualizuje MapSquare po pøidání jednotky "pUnit"
	inline void UnitInserted ( CSUnit *pUnit );
	// aktualizuje MapSquare po odebrání jednotky "pUnit"
	void UnitDeleted ( CSUnit *pUnit );

// Operace s jednotkami v MapSquarech

	// vytvoøí mapy jednotek MainLoop threadu a FindPathLoop threadu
	static void CreateUnitMaps ();
	// znièí mapu jednotek MainLoop threadu a FindPathLoop threadu
	static void DeleteUnitMaps ();

	// zjistí, lze-li jednotku "pUnit" umístit na pozici "pointPosition" z MainLoop 
	//		threadu (je-li nastaven pøíznak "bMapSquaresLocked", nezamyká MapSquary)
	static BOOL MainLoopCanPlaceUnit ( CSUnit *pUnit, CPointDW pointPosition, 
		BOOL bMapSquaresLocked );
	// ukonèí umísování jednotky z MainLoop threadu
	static void MainLoopFinishPlacingUnit ();

	// zjistí, lze-li jednotku "pUnit" umístit na pozici "pointPosition" z FindPathLoop 
	//		threadu (je-li nastaven pøíznak "bMapSquaresLocked", nezamyká MapSquary)
	static BOOL FindPathLoopCanPlaceUnit ( CSUnit *pUnit, CPointDW pointPosition, 
		BOOL bMapSquaresLocked );
	// ukonèí umísování jednotky z FindPathLoop threadu
	static void FindPathLoopFinishPlacingUnit ();

// Informace o MapSquarech

	// vrátí index MapSquaru
	CPointDW GetIndex ();

// Debuggovací informace
private:
#ifdef _DEBUG
	// zkontroluje správná data objektu (TRUE=OK)
	BOOL CheckValidData ();
	// zkontroluje neškodná data objektu (TRUE=OK)
	BOOL CheckEmptyData ();
#endif //_DEBUG

// Data
private:
// Informace o MapSquaru

	// pole pøíznakù umístìní civilizací v MapSquaru
	CSCivilizationBitArray m_cCivilizations;
	// pole pøíznakù civilizací sledujících tento MapSquare
	CSCivilizationBitArray m_cWatchingCivilizations;
	// seznam jednotek umístìnıch v MapSquaru
	CSUnitList m_cUnitList;

// Informace o jednotkách v MapSquarech

	// pole ukazatelù na MapSquary mapy jednotek MainLoop threadu
	static signed char **m_aMainLoopUnitMapMapSquares;
	// memory pool MapSquarù mapy jednotek MainLoop threadu
	static CMemoryPool m_cMainLoopUnitMapMapSquareMemoryPool;
	// zásobník indexù MapSquarù mapy jednotek MainLoop threadu
	static CSelfPooledStack<DWORD> m_cMainLoopUnitMapMapSquareIndexStack;

	// pole ukazatelù na MapSquary mapy jednotek FindPathLoop threadu
	static signed char **m_aFindPathLoopUnitMapMapSquares;
	// memory pool MapSquarù mapy jednotek FindPathLoop threadu
	static CMemoryPool m_cFindPathLoopUnitMapMapSquareMemoryPool;
	// zásobník indexù MapSquarù mapy jednotek FindPathLoop threadu
	static CSelfPooledStack<DWORD> m_cFindPathLoopUnitMapMapSquareIndexStack;

	// offsety okolních pozic
	static CPointDW m_aSurroundingPositionOffset[9];
};

//////////////////////////////////////////////////////////////////////
// Inline metody tøídy CSMapSquare

//////////////////////////////////////////////////////////////////////
// Operace s jednotkami
//////////////////////////////////////////////////////////////////////

// vloí jednotku "pUnit" do MapSquaru
inline void CSMapSquare::InsertUnit ( CSUnit *pUnit ) 
{
	ASSERT ( pUnit != NULL );

	// pøidá jednotku do seznamu jednotek MapSquaru
	m_cUnitList.Add ( pUnit );
	// nastaví pøíznak umístìní civilizace v MapSquaru
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

	// aktualizuje MapSquare po odebrání jednotky "pUnit"
	UnitDeleted ( pUnit );
}

// aktualizuje MapSquare po pøidání jednotky "pUnit"
inline void CSMapSquare::UnitInserted ( CSUnit *pUnit ) 
{
	ASSERT ( pUnit != NULL );

	// nastaví pøíznak umístìní civilizace v MapSquaru
	m_cCivilizations.SetAt ( pUnit->GetCivilizationIndex () );
}

#endif //__SERVER_MAP_SQUARE__HEADER_INCLUDED__
