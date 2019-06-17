/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída ètverce mapy na serveru hry
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SMap.h"
#include "SMapSquare.h"

#include "Common\Map\Map.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Informace o jednotkách v MapSquarech
//////////////////////////////////////////////////////////////////////

// pole ukazatelù na MapSquary mapy jednotek MainLoop threadu
signed char **CSMapSquare::m_aMainLoopUnitMapMapSquares = NULL;
// memory pool MapSquarù mapy jednotek MainLoop threadu
CMemoryPool CSMapSquare::m_cMainLoopUnitMapMapSquareMemoryPool ( 5 );
// zásobník indexù MapSquarù mapy jednotek MainLoop threadu
CSelfPooledStack<DWORD> CSMapSquare::m_cMainLoopUnitMapMapSquareIndexStack ( 50 );

// pole ukazatelù na MapSquary mapy jednotek FindPathLoop threadu
signed char **CSMapSquare::m_aFindPathLoopUnitMapMapSquares = NULL;
// memory pool MapSquarù mapy jednotek FindPathLoop threadu
CMemoryPool CSMapSquare::m_cFindPathLoopUnitMapMapSquareMemoryPool ( 5 );
// zásobník indexù MapSquarù mapy jednotek FindPathLoop threadu
CSelfPooledStack<DWORD> CSMapSquare::m_cFindPathLoopUnitMapMapSquareIndexStack ( 50 );

// offsety okolních pozic
CPointDW CSMapSquare::m_aSurroundingPositionOffset[9] = 
{
	CPointDW ( (DWORD)-1, (DWORD)-1 ), 
	CPointDW ( 0, (DWORD)-1 ), 
	CPointDW ( 1, (DWORD)-1 ), 
	CPointDW ( (DWORD)-1, 0 ), 
	CPointDW ( 1, 0 ), 
	CPointDW ( (DWORD)-1, 1 ), 
	CPointDW ( 0, 1 ), 
	CPointDW ( 1, 1 ), 
	CPointDW ( 0, 0 )
};

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSMapSquare::CSMapSquare () 
{
}

// destruktor
CSMapSquare::~CSMapSquare () 
{
	ASSERT ( CheckEmptyData () );
}

//////////////////////////////////////////////////////////////////////
// Inicializace a znièení dat objektu
//////////////////////////////////////////////////////////////////////

// vytvoøí MapSquare ze souboru mapy "cMapFile"
void CSMapSquare::Create ( CArchiveFile cMapFile ) 
{
	ASSERT ( CheckEmptyData () );

	SMapSquareHeader sMapSquareHeader;

	// naète hlavièku MapSquaru
	LOAD_ASSERT ( cMapFile.Read ( &sMapSquareHeader, sizeof ( sMapSquareHeader ) ) == sizeof ( sMapSquareHeader ) );

	// zjistí velikost popisu MapSquaru
	DWORD dwMapSquareDescriptionSize = ( sMapSquareHeader.m_dwBL1MapexesNum + 
		sMapSquareHeader.m_dwBL2MapexesNum + sMapSquareHeader.m_dwBL3MapexesNum ) * 
		sizeof ( SMapexInstanceHeader ) + ( sMapSquareHeader.m_dwULMapexesNum ) * 
		sizeof ( SULMapexInstanceHeader );

	// pøeskoèí popis MapSquaru
	LOAD_ASSERT ( cMapFile.GetPosition () + dwMapSquareDescriptionSize <= 
		cMapFile.GetLength () );
	(void)cMapFile.Seek ( dwMapSquareDescriptionSize, CFile::current );
}

// znièí MapSquare
void CSMapSquare::Delete () 
{
	// znièí seznam jednotek
	m_cUnitList.RemoveAll ();
}

//////////////////////////////////////////////////////////////////////
// Operace s jednotkami
//////////////////////////////////////////////////////////////////////

// aktualizuje MapSquare po odebrání jednotky "pUnit"
void CSMapSquare::UnitDeleted ( CSUnit *pUnit ) 
{
	ASSERT ( pUnit != NULL );

	// zjistí index civilizace odebrané jednotky
	DWORD dwCivilizationIndex = pUnit->GetCivilizationIndex ();

	ASSERT ( m_cCivilizations.GetAt ( dwCivilizationIndex ) );

// aktualizuje pøíznak umístìní civilizace odebírané jednotky v MapSquaru

	// ukazatel na jednotku MapSquaru
	CSUnit *pMapSquareUnit;

	// zjistí pozici první jednotky v seznamu jednotek
	POSITION posUnit = m_cUnitList.GetHeadPosition ();

	// projede jednotky MapSquaru
	while ( m_cUnitList.GetNext ( posUnit, pMapSquareUnit ) )
	{
		// zjistí, je-li to jednotka civilizace odebírané jednotky
		if ( pMapSquareUnit->GetCivilizationIndex () == dwCivilizationIndex )
		{	// jedná se o jednotku civilizace odebírané jednotky
			// není tøeba mazat pøíznak umístìní civilizace v MapSquaru
			return;
		}
	}
	// civilizace odebírané jednotky se v MapSquaru již nenalézá
	m_cCivilizations.ClearAt ( dwCivilizationIndex );
}

//////////////////////////////////////////////////////////////////////
// Operace s jednotkami v MapSquarech
//////////////////////////////////////////////////////////////////////

// ukazatel na MapSquare mapu jednotek
typedef signed char *TUnitMapMapSquarePointer;

// vytvoøí mapy jednotek MainLoop threadu a FindPathLoop threadu
void CSMapSquare::CreateUnitMaps () 
{
	ASSERT ( m_aMainLoopUnitMapMapSquares == NULL );
	ASSERT ( m_aFindPathLoopUnitMapMapSquares == NULL );
	ASSERT ( ( g_cMap.GetMapSizeMapSquare ().cx > 0 ) && 
		( g_cMap.GetMapSizeMapSquare ().cy > 0 ) );

	// poèet MapSquarù mapy jednotek
	DWORD dwUnitMapMapSquareCount = g_cMap.GetMapSizeMapSquare ().cx * 
		g_cMap.GetMapSizeMapSquare ().cy;

	// alokuje pole ukazatelù na MapSquary mapy jednotek MainLoop threadu
	m_aMainLoopUnitMapMapSquares = new TUnitMapMapSquarePointer[dwUnitMapMapSquareCount];
	// alokuje pole ukazatelù na MapSquary mapy jednotek FindPathLoop threadu
	m_aFindPathLoopUnitMapMapSquares = 
		new TUnitMapMapSquarePointer[dwUnitMapMapSquareCount];

	// inicializuje ukazatele na MapSquary mapy jednotek
	for ( ; dwUnitMapMapSquareCount-- > 0; )
	{
		// inicializuje ukazatel na MapSquare mapy jednotek MainLoop threadu
		m_aMainLoopUnitMapMapSquares[dwUnitMapMapSquareCount] = NULL;
		// inicializuje ukazatel na MapSquare mapy jednotek FindPathLoop threadu
		m_aFindPathLoopUnitMapMapSquares[dwUnitMapMapSquareCount] = NULL;
	}

	// inicializuje memory pool MapSquarù mapy jednotek MainLoop threadu
	m_cMainLoopUnitMapMapSquareMemoryPool.Create ( MAP_SQUARE_SIZE * MAP_SQUARE_SIZE );
	// inicializuje memory pool MapSquarù mapy jednotek FindPathLoop threadu
	m_cFindPathLoopUnitMapMapSquareMemoryPool.Create ( MAP_SQUARE_SIZE * MAP_SQUARE_SIZE );
}

// znièí mapy jednotek MainLoop threadu a FindPathLoop threadu
void CSMapSquare::DeleteUnitMaps () 
{
	// zjistí, je-li pole ukazatelù na MapSquary mapy jednotek MainLoop threadu platné
	if ( m_aMainLoopUnitMapMapSquares != NULL )
	{	// pole ukazatelù na MapSquary mapy jednotek MainLoop threadu je platné
		// znièí pole ukazatelù na MapSquary mapy jednotek MainLoop threadu
		delete [] m_aMainLoopUnitMapMapSquares;
		m_aMainLoopUnitMapMapSquares = NULL;
		// znièí memory pool MapSquarù mapy jednotek MainLoop threadu
		m_cMainLoopUnitMapMapSquareMemoryPool.Delete ();
	}

	// zjistí, je-li pole ukazatelù na MapSquary mapy jednotek FindPathLoop threadu platné
	if ( m_aFindPathLoopUnitMapMapSquares != NULL )
	{	// pole ukazatelù na MapSquary mapy jednotek FindPathLoop threadu je platné
		// znièí pole ukazatelù na MapSquary mapy jednotek FindPathLoop threadu
		delete [] m_aFindPathLoopUnitMapMapSquares;
		m_aFindPathLoopUnitMapMapSquares = NULL;
		// znièí memory pool MapSquarù mapy jednotek FindPathLoop threadu
		m_cFindPathLoopUnitMapMapSquareMemoryPool.Delete ();
	}
}

// zjistí, lze-li jednotku "pUnit" umístit na pozici "pointPosition" z MainLoop threadu 
//		(je-li nastaven pøíznak "bMapSquaresLocked", nezamyká MapSquary)
BOOL CSMapSquare::MainLoopCanPlaceUnit ( CSUnit *pUnit, CPointDW pointPosition, 
	BOOL bMapSquaresLocked ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );

	// šíøka jednotky
	DWORD dwUnitWidth = pUnit->GetMoveWidth ();
	// offset posunutí ètverce jednotky
	DWORD dwUnitRectOffset = dwUnitWidth / 2;
	// ètverec jednotky
	CPointDW pointUnitLeftTop ( pointPosition.x - dwUnitRectOffset, pointPosition.y - 
		dwUnitRectOffset );
	CPointDW pointUnitRightBottom ( pointPosition.x - dwUnitRectOffset + dwUnitWidth - 1, 
		pointPosition.y - dwUnitRectOffset + dwUnitWidth - 1 );

	// zjistí, zasahuje-li ètverec jednotky mimo mapu
	if ( ( pointUnitRightBottom.x > g_cMap.GetRightBottomCornerMapCell ().x ) || 
		( pointUnitRightBottom.y > g_cMap.GetRightBottomCornerMapCell ().y ) )
	{	// ètverec jednotky zasahuje mimo mapu
		// vrátí pøíznak neumístìní jednotky
		return FALSE;
	}

	// zjistí, jsou-li zamèeny MapSquary
	if ( !bMapSquaresLocked )
	{	// MapSquary nejsou zamèeny
		// zamkne MapSquary
		VERIFY ( g_cMap.m_mutexMapSquaresLock.Lock () );
	}

	// zkontroluje osazení ètverce jednotky jinými jednotkami
	for ( DWORD dwPositionX = pointUnitLeftTop.x; dwPositionX <= pointUnitRightBottom.x; 
		dwPositionX++ )
	{
		for ( DWORD dwPositionY = pointUnitLeftTop.y; dwPositionY <= 
			pointUnitRightBottom.y; dwPositionY++ )
		{
			// index MapSquaru mapy jednotek
			DWORD dwUnitMapMapSquareIndex = dwPositionX / MAP_SQUARE_SIZE + ( dwPositionY / 
				MAP_SQUARE_SIZE ) * g_cMap.GetMapSizeMapSquare ().cx;

			// zjistí, je-li MapSquare mapy jednotek platný
			if ( m_aMainLoopUnitMapMapSquares[dwUnitMapMapSquareIndex] == NULL )
			{	// MapSquare mapy jednotek není platný
				// souøadnice MapSquaru mapy jednotek
				CPointDW pointUnitMapMapSquareIndex ( dwPositionX / MAP_SQUARE_SIZE, 
					dwPositionY / MAP_SQUARE_SIZE );
				// alokuje nový MapSquare mapy jednotek
				signed char *pUnitMapMapSquare = 
					m_aMainLoopUnitMapMapSquares[dwUnitMapMapSquareIndex] = 
					(signed char *)m_cMainLoopUnitMapMapSquareMemoryPool.Allocate ();
				m_cMainLoopUnitMapMapSquareIndexStack.Push ( dwUnitMapMapSquareIndex );

				// inicializuje nový MapSquare mapy jednotek
				memset ( pUnitMapMapSquare, 1, MAP_SQUARE_SIZE * MAP_SQUARE_SIZE );

			// nakreslí MapSquare mapy jednotek

				// ètverec MapSquaru mapy jednotek
				CPointDW pointUnitMapMapSquareLeftTop ( pointUnitMapMapSquareIndex.x * 
					MAP_SQUARE_SIZE, pointUnitMapMapSquareIndex.y * MAP_SQUARE_SIZE );
				CPointDW pointUnitMapMapSquareRightBottom ( pointUnitMapMapSquareLeftTop.x + 
					MAP_SQUARE_SIZE - 1, pointUnitMapMapSquareLeftTop.y + MAP_SQUARE_SIZE - 1 );

				// projede okolní MapSquary
				for ( DWORD dwSurroundingPositionIndex = 9; 
					dwSurroundingPositionIndex-- > 0; )
				{
					// ukazatel na MapSquare
					CSMapSquare *pMapSquare = g_cMap.GetMapSquareFromIndexNull ( 
						pointUnitMapMapSquareIndex + 
						m_aSurroundingPositionOffset[dwSurroundingPositionIndex] );
					// zjistí, je-li MapSquare platný
					if ( pMapSquare == NULL )
					{	// MapSquare není platný
						// pokraèuje ve zpracování dalšího MapSquaru
						continue;
					}

					// ukazatel na jednotku MapSquaru
					CSUnit *pMapSquareUnit;
					// pozice jednotky MapSquaru v seznamu jednotek MapSquaru
					POSITION posMapSquareUnit = pMapSquare->m_cUnitList.GetHeadPosition ();

					// vykreslí jednotky MapSquaru
					while ( CSUnitList::GetNext ( posMapSquareUnit, pMapSquareUnit ) )
					{
						// zjistí, jedná-li se o umísovanou jednotku
						if ( pMapSquareUnit == pUnit )
						{	// jedná se o umísovanou jednotku
							// nechá vykreslit další jednotku MapSquaru
							continue;
						}

						// zjistí, jedná-li se o vzdušnou jednotku
						if ( pMapSquareUnit->GetVerticalPosition () >= AIR_VERTICAL_POSITION )
						{	// jedná se o vzdušnou jednotku
							// nechá vykreslit další jednotku MapSquaru
							continue;
						}

						// šíøka jednotky MapSquaru
						DWORD dwMapSquareUnitWidth = pMapSquareUnit->GetMoveWidth ();
						// pozice jednotky MapSquaru
						CPointDW pointMapSquareUnitPosition = pMapSquareUnit->GetPosition ();
						// offset posunutí ètverce jednotky MapSquaru
						DWORD dwMapSquareUnitRectOffset = dwMapSquareUnitWidth / 2;
						// ètverec jednotky MapSquaru
						CPointDW pointMapSquareUnitLeftTop ( pointMapSquareUnitPosition.x - 
							dwMapSquareUnitRectOffset, pointMapSquareUnitPosition.y - 
							dwMapSquareUnitRectOffset );
						CPointDW pointMapSquareUnitRightBottom ( pointMapSquareUnitLeftTop.x + 
							dwMapSquareUnitWidth - 1, pointMapSquareUnitLeftTop.y + 
							dwMapSquareUnitWidth - 1 );
						ASSERT ( ( pointMapSquareUnitRightBottom.x <= 
							g_cMap.GetRightBottomCornerMapCell ().x ) && 
							( pointMapSquareUnitRightBottom.y <= 
							g_cMap.GetRightBottomCornerMapCell ().y ) );

						// oøízne ètverec jednotky MapSquaru ètvercem MapSquaru mapy jednotek
						pointMapSquareUnitLeftTop.x = max ( pointMapSquareUnitLeftTop.x, 
							pointUnitMapMapSquareLeftTop.x );
						pointMapSquareUnitLeftTop.y = max ( pointMapSquareUnitLeftTop.y, 
							pointUnitMapMapSquareLeftTop.y );
						pointMapSquareUnitRightBottom.x = min ( pointMapSquareUnitRightBottom.x, 
							pointUnitMapMapSquareRightBottom.x );
						pointMapSquareUnitRightBottom.y = min ( pointMapSquareUnitRightBottom.y, 
							pointUnitMapMapSquareRightBottom.y );

						// vykreslí ètverec jednotky MapSquaru do MapSquaru mapy jednotek
						DWORD dwPositionX = pointMapSquareUnitLeftTop.x;
						DWORD dwPosX = pointMapSquareUnitLeftTop.x - 
							pointUnitMapMapSquareLeftTop.x;
						for ( ; dwPositionX++ <= pointMapSquareUnitRightBottom.x; dwPosX++ )
						{
							DWORD dwPositionY = pointMapSquareUnitLeftTop.y;
							DWORD dwPosY = pointMapSquareUnitLeftTop.y - 
								pointUnitMapMapSquareLeftTop.y;
							for ( ; dwPositionY++ <= pointMapSquareUnitRightBottom.y; dwPosY++ )
							{
								// oznaèí místo v mapì jednotek za obsazené
								pUnitMapMapSquare[dwPosX + dwPosY * MAP_SQUARE_SIZE] = 0;
							}
						}
						// ètverec jednotky MapSquaru byl vykreslen
					}
					// jednotky MapSquaru byly vykresleny
				}
				// všechny okolní MapSquary byly vykresleny
			}
			// MapSquare mapy jednotek je platný

			ASSERT ( m_aMainLoopUnitMapMapSquares[dwUnitMapMapSquareIndex] != NULL );

			// index MapCellu v MapSquaru mapy jednotek
			DWORD dwMapCellIndex = dwPositionX % MAP_SQUARE_SIZE + ( dwPositionY % 
				MAP_SQUARE_SIZE ) * MAP_SQUARE_SIZE;

			// zjistí, je-li MapCell obsazen jinou jednotkou
			if ( !m_aMainLoopUnitMapMapSquares[dwUnitMapMapSquareIndex][dwMapCellIndex] )
			{	// MapCell je již obsazen jinou jednotkou
				ASSERT ( m_aMainLoopUnitMapMapSquares[dwUnitMapMapSquareIndex][dwMapCellIndex] 
					== 0 );
				// zjistí, byly-li zamèeny MapSquary
				if ( !bMapSquaresLocked )
				{	// MapSquary nebyly zamèeny
					// odemkne MapSquary
					VERIFY ( g_cMap.m_mutexMapSquaresLock.Unlock () );
				}

				// vrátí pøíznak neumístìní jednotky
				return FALSE;
			}
			// MapCell ètverce jednotky není obsazen jinou jednotkou
			ASSERT ( m_aMainLoopUnitMapMapSquares[dwUnitMapMapSquareIndex][dwMapCellIndex] 
				== 1 );
		}
	}
	// ve ètverci jednotky není jiná jednotka

	// zjistí, byly-li zamèeny MapSquary
	if ( !bMapSquaresLocked )
	{	// MapSquary nebyly zamèeny
		// odemkne MapSquary
		VERIFY ( g_cMap.m_mutexMapSquaresLock.Unlock () );
	}

	// vrátí pøíznak umístìní jednotky
	return TRUE;
}

// ukonèí umísování jednotky z MainLoop threadu
void CSMapSquare::MainLoopFinishPlacingUnit () 
{
	ASSERT ( g_cMap.IsMainLoopThread () );

	// index MapSquaru mapy jednotek MainLoop threadu
	DWORD dwIndex;

	// znièí MapSquary mapy jednotek MainLoop threadu
	while ( m_cMainLoopUnitMapMapSquareIndexStack.Pop ( dwIndex ) )
	{
		ASSERT ( dwIndex < g_cMap.GetMapSizeMapSquare ().cx * 
			g_cMap.GetMapSizeMapSquare ().cy );
		ASSERT ( m_aMainLoopUnitMapMapSquares[dwIndex] != NULL );

		// znièí MapSquare mapy jednotek MainLoop threadu
		m_cMainLoopUnitMapMapSquareMemoryPool.Free ( 
			m_aMainLoopUnitMapMapSquares[dwIndex] );
		m_aMainLoopUnitMapMapSquares[dwIndex] = NULL;
	}
}

// zjistí, lze-li jednotku "pUnit" umístit na pozici "pointPosition" z FindPathLoop threadu 
//		(je-li nastaven pøíznak "bMapSquaresLocked", nezamyká MapSquary)
BOOL CSMapSquare::FindPathLoopCanPlaceUnit ( CSUnit *pUnit, CPointDW pointPosition, 
	BOOL bMapSquaresLocked ) 
{
	ASSERT ( g_cMap.m_pFindPathLoopThread->m_nThreadID == GetCurrentThreadId () );

	// šíøka jednotky
	DWORD dwUnitWidth = pUnit->GetMoveWidth ();
	// offset posunutí ètverce jednotky
	DWORD dwUnitRectOffset = dwUnitWidth / 2;
	// ètverec jednotky
	CPointDW pointUnitLeftTop ( pointPosition.x - dwUnitRectOffset, pointPosition.y - 
		dwUnitRectOffset );
	CPointDW pointUnitRightBottom ( pointPosition.x - dwUnitRectOffset + dwUnitWidth - 1, 
		pointPosition.y - dwUnitRectOffset + dwUnitWidth - 1 );

	// zjistí, zasahuje-li ètverec jednotky mimo mapu
	if ( ( pointUnitRightBottom.x > g_cMap.GetRightBottomCornerMapCell ().x ) || 
		( pointUnitRightBottom.y > g_cMap.GetRightBottomCornerMapCell ().y ) )
	{	// ètverec jednotky zasahuje mimo mapu
		// vrátí pøíznak neumístìní jednotky
		return FALSE;
	}

	// zjistí, jsou-li zamèeny MapSquary
	if ( !bMapSquaresLocked )
	{	// MapSquary nejsou zamèeny
		// zamkne MapSquary
		VERIFY ( g_cMap.m_mutexMapSquaresLock.Lock () );
	}

	// zkontroluje osazení ètverce jednotky jinými jednotkami
	for ( DWORD dwPositionX = pointUnitLeftTop.x; dwPositionX <= pointUnitRightBottom.x; 
		dwPositionX++ )
	{
		for ( DWORD dwPositionY = pointUnitLeftTop.y; dwPositionY <= 
			pointUnitRightBottom.y; dwPositionY++ )
		{
			// index MapSquaru mapy jednotek
			DWORD dwUnitMapMapSquareIndex = dwPositionX / MAP_SQUARE_SIZE + ( dwPositionY / 
				MAP_SQUARE_SIZE ) * g_cMap.GetMapSizeMapSquare ().cx;

			// zjistí, je-li MapSquare mapy jednotek platný
			if ( m_aFindPathLoopUnitMapMapSquares[dwUnitMapMapSquareIndex] == NULL )
			{	// MapSquare mapy jednotek není platný
				// souøadnice MapSquaru mapy jednotek
				CPointDW pointUnitMapMapSquareIndex ( dwPositionX / MAP_SQUARE_SIZE, 
					dwPositionY / MAP_SQUARE_SIZE );
				// alokuje nový MapSquare mapy jednotek
				signed char *pUnitMapMapSquare = 
					m_aFindPathLoopUnitMapMapSquares[dwUnitMapMapSquareIndex] = 
					(signed char *)m_cFindPathLoopUnitMapMapSquareMemoryPool.Allocate ();
				m_cFindPathLoopUnitMapMapSquareIndexStack.Push ( dwUnitMapMapSquareIndex );

				// inicializuje nový MapSquare mapy jednotek
				memset ( pUnitMapMapSquare, 1, MAP_SQUARE_SIZE * MAP_SQUARE_SIZE );

			// nakreslí MapSquare mapy jednotek

				// ètverec MapSquaru mapy jednotek
				CPointDW pointUnitMapMapSquareLeftTop ( pointUnitMapMapSquareIndex.x * 
					MAP_SQUARE_SIZE, pointUnitMapMapSquareIndex.y * MAP_SQUARE_SIZE );
				CPointDW pointUnitMapMapSquareRightBottom ( pointUnitMapMapSquareLeftTop.x + 
					MAP_SQUARE_SIZE - 1, pointUnitMapMapSquareLeftTop.y + MAP_SQUARE_SIZE - 1 );

				// projede okolní MapSquary
				for ( DWORD dwSurroundingPositionIndex = 9; 
					dwSurroundingPositionIndex-- > 0; )
				{
					// ukazatel na MapSquare
					CSMapSquare *pMapSquare = g_cMap.GetMapSquareFromIndexNull ( 
						pointUnitMapMapSquareIndex + 
						m_aSurroundingPositionOffset[dwSurroundingPositionIndex] );
					// zjistí, je-li MapSquare platný
					if ( pMapSquare == NULL )
					{	// MapSquare není platný
						// pokraèuje ve zpracování dalšího MapSquaru
						continue;
					}

					// ukazatel na jednotku MapSquaru
					CSUnit *pMapSquareUnit;
					// pozice jednotky MapSquaru v seznamu jednotek MapSquaru
					POSITION posMapSquareUnit = pMapSquare->m_cUnitList.GetHeadPosition ();

					// vykreslí jednotky MapSquaru
					while ( CSUnitList::GetNext ( posMapSquareUnit, pMapSquareUnit ) )
					{
						// zjistí, jedná-li se o umísovanou jednotku
						if ( pMapSquareUnit == pUnit )
						{	// jedná se o umísovanou jednotku
							// nechá vykreslit další jednotku MapSquaru
							continue;
						}

						// zjistí, jedná-li se o vzdušnou jednotku
						if ( pMapSquareUnit->GetVerticalPosition () >= AIR_VERTICAL_POSITION )
						{	// jedná se o vzdušnou jednotku
							// nechá vykreslit další jednotku MapSquaru
							continue;
						}

						// šíøka jednotky MapSquaru
						DWORD dwMapSquareUnitWidth = pMapSquareUnit->GetMoveWidth ();
						// pozice jednotky MapSquaru
						CPointDW pointMapSquareUnitPosition = pMapSquareUnit->GetPosition ();
						// offset posunutí ètverce jednotky MapSquaru
						DWORD dwMapSquareUnitRectOffset = dwMapSquareUnitWidth / 2;
						// ètverec jednotky MapSquaru
						CPointDW pointMapSquareUnitLeftTop ( pointMapSquareUnitPosition.x - 
							dwMapSquareUnitRectOffset, pointMapSquareUnitPosition.y - 
							dwMapSquareUnitRectOffset );
						CPointDW pointMapSquareUnitRightBottom ( pointMapSquareUnitLeftTop.x + 
							dwMapSquareUnitWidth - 1, pointMapSquareUnitLeftTop.y + 
							dwMapSquareUnitWidth - 1 );
						ASSERT ( ( pointMapSquareUnitRightBottom.x <= 
							g_cMap.GetRightBottomCornerMapCell ().x ) && 
							( pointMapSquareUnitRightBottom.y <= 
							g_cMap.GetRightBottomCornerMapCell ().y ) );

						// oøízne ètverec jednotky MapSquaru ètvercem MapSquaru mapy jednotek
						pointMapSquareUnitLeftTop.x = max ( pointMapSquareUnitLeftTop.x, 
							pointUnitMapMapSquareLeftTop.x );
						pointMapSquareUnitLeftTop.y = max ( pointMapSquareUnitLeftTop.y, 
							pointUnitMapMapSquareLeftTop.y );
						pointMapSquareUnitRightBottom.x = min ( pointMapSquareUnitRightBottom.x, 
							pointUnitMapMapSquareRightBottom.x );
						pointMapSquareUnitRightBottom.y = min ( pointMapSquareUnitRightBottom.y, 
							pointUnitMapMapSquareRightBottom.y );

						// vykreslí ètverec jednotky MapSquaru do MapSquaru mapy jednotek
						DWORD dwPositionX = pointMapSquareUnitLeftTop.x;
						DWORD dwPosX = pointMapSquareUnitLeftTop.x - 
							pointUnitMapMapSquareLeftTop.x;
						for ( ; dwPositionX++ <= pointMapSquareUnitRightBottom.x; dwPosX++ )
						{
							DWORD dwPositionY = pointMapSquareUnitLeftTop.y;
							DWORD dwPosY = pointMapSquareUnitLeftTop.y - 
								pointUnitMapMapSquareLeftTop.y;
							for ( ; dwPositionY++ <= pointMapSquareUnitRightBottom.y; dwPosY++ )
							{
								// oznaèí místo v mapì jednotek za obsazené
								pUnitMapMapSquare[dwPosX + dwPosY * MAP_SQUARE_SIZE] = 0;
							}
						}
						// ètverec jednotky MapSquaru byl vykreslen
					}
					// jednotky MapSquaru byly vykresleny
				}
				// všechny okolní MapSquary byly vykresleny
			}
			// MapSquare mapy jednotek je platný

			ASSERT ( m_aFindPathLoopUnitMapMapSquares[dwUnitMapMapSquareIndex] != NULL );

			// index MapCellu v MapSquaru mapy jednotek
			DWORD dwMapCellIndex = dwPositionX % MAP_SQUARE_SIZE + ( dwPositionY % 
				MAP_SQUARE_SIZE ) * MAP_SQUARE_SIZE;

			// zjistí, je-li MapCell obsazen jinou jednotkou
			if ( !m_aFindPathLoopUnitMapMapSquares[dwUnitMapMapSquareIndex][dwMapCellIndex] )
			{	// MapCell je již obsazen jinou jednotkou
				ASSERT ( m_aFindPathLoopUnitMapMapSquares[dwUnitMapMapSquareIndex][dwMapCellIndex] 
					== 0 );
				// zjistí, byly-li zamèeny MapSquary
				if ( !bMapSquaresLocked )
				{	// MapSquary nebyly zamèeny
					// odemkne MapSquary
					VERIFY ( g_cMap.m_mutexMapSquaresLock.Unlock () );
				}

				// vrátí pøíznak neumístìní jednotky
				return FALSE;
			}
			// MapCell ètverce jednotky není obsazen jinou jednotkou
			ASSERT ( m_aFindPathLoopUnitMapMapSquares[dwUnitMapMapSquareIndex][dwMapCellIndex] 
				== 1 );
		}
	}
	// ve ètverci jednotky není jiná jednotka

	// zjistí, byly-li zamèeny MapSquary
	if ( !bMapSquaresLocked )
	{	// MapSquary nebyly zamèeny
		// odemkne MapSquary
		VERIFY ( g_cMap.m_mutexMapSquaresLock.Unlock () );
	}

	// vrátí pøíznak umístìní jednotky
	return TRUE;
}

// ukonèí umísování jednotky z FindPathLoop threadu
void CSMapSquare::FindPathLoopFinishPlacingUnit () 
{
	ASSERT ( g_cMap.m_pFindPathLoopThread->m_nThreadID == GetCurrentThreadId () );

	// index MapSquaru mapy jednotek FindPathLoop threadu
	DWORD dwIndex;

	// znièí MapSquary mapy jednotek FindPathLoop threadu
	while ( m_cFindPathLoopUnitMapMapSquareIndexStack.Pop ( dwIndex ) )
	{
		ASSERT ( dwIndex < g_cMap.GetMapSizeMapSquare ().cx * 
			g_cMap.GetMapSizeMapSquare ().cy );
		ASSERT ( m_aFindPathLoopUnitMapMapSquares[dwIndex] != NULL );

		// znièí MapSquare mapy jednotek FindPathLoop threadu
		m_cFindPathLoopUnitMapMapSquareMemoryPool.Free ( 
			m_aFindPathLoopUnitMapMapSquares[dwIndex] );
		m_aFindPathLoopUnitMapMapSquares[dwIndex] = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Informace o MapSquare
//////////////////////////////////////////////////////////////////////

// vrátí index MapSquaru
CPointDW CSMapSquare::GetIndex () 
{
	// index MapSquaru
	CPointDW pointIndex;

	// zjistí index MapSquaru v poli MapSquarù
	DWORD dwMapSquareIndex = ( (DWORD)this - (DWORD)g_cMap.m_pMapSquares ) / 
		sizeof ( CSMapSquare );
	ASSERT ( dwMapSquareIndex < g_cMap.m_sizeMapMapSquare.cx * 
		g_cMap.m_sizeMapMapSquare.cy );
	ASSERT ( &g_cMap.m_pMapSquares[dwMapSquareIndex] == this );

	// vypoète index MapSquaru
	pointIndex.x = dwMapSquareIndex % g_cMap.m_sizeMapMapSquare.cx;
	pointIndex.y = dwMapSquareIndex / g_cMap.m_sizeMapMapSquare.cx;
	ASSERT ( pointIndex.y < g_cMap.m_sizeMapMapSquare.cy );

	// vrátí index MapSquaru
	return pointIndex;
}

//////////////////////////////////////////////////////////////////////
// Debuggovací informace
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// zkontroluje správná data objektu (TRUE=OK)
BOOL CSMapSquare::CheckValidData () 
{
	return TRUE;
}

// zkontroluje neškodná data objektu (TRUE=OK)
BOOL CSMapSquare::CheckEmptyData () 
{
	ASSERT ( m_cUnitList.IsEmpty () );

	return TRUE;
}

#endif //_DEBUG
