/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da �tverce mapy na serveru hry
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
// Informace o jednotk�ch v MapSquarech
//////////////////////////////////////////////////////////////////////

// pole ukazatel� na MapSquary mapy jednotek MainLoop threadu
signed char **CSMapSquare::m_aMainLoopUnitMapMapSquares = NULL;
// memory pool MapSquar� mapy jednotek MainLoop threadu
CMemoryPool CSMapSquare::m_cMainLoopUnitMapMapSquareMemoryPool ( 5 );
// z�sobn�k index� MapSquar� mapy jednotek MainLoop threadu
CSelfPooledStack<DWORD> CSMapSquare::m_cMainLoopUnitMapMapSquareIndexStack ( 50 );

// pole ukazatel� na MapSquary mapy jednotek FindPathLoop threadu
signed char **CSMapSquare::m_aFindPathLoopUnitMapMapSquares = NULL;
// memory pool MapSquar� mapy jednotek FindPathLoop threadu
CMemoryPool CSMapSquare::m_cFindPathLoopUnitMapMapSquareMemoryPool ( 5 );
// z�sobn�k index� MapSquar� mapy jednotek FindPathLoop threadu
CSelfPooledStack<DWORD> CSMapSquare::m_cFindPathLoopUnitMapMapSquareIndexStack ( 50 );

// offsety okoln�ch pozic
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
// Inicializace a zni�en� dat objektu
//////////////////////////////////////////////////////////////////////

// vytvo�� MapSquare ze souboru mapy "cMapFile"
void CSMapSquare::Create ( CArchiveFile cMapFile ) 
{
	ASSERT ( CheckEmptyData () );

	SMapSquareHeader sMapSquareHeader;

	// na�te hlavi�ku MapSquaru
	LOAD_ASSERT ( cMapFile.Read ( &sMapSquareHeader, sizeof ( sMapSquareHeader ) ) == sizeof ( sMapSquareHeader ) );

	// zjist� velikost popisu MapSquaru
	DWORD dwMapSquareDescriptionSize = ( sMapSquareHeader.m_dwBL1MapexesNum + 
		sMapSquareHeader.m_dwBL2MapexesNum + sMapSquareHeader.m_dwBL3MapexesNum ) * 
		sizeof ( SMapexInstanceHeader ) + ( sMapSquareHeader.m_dwULMapexesNum ) * 
		sizeof ( SULMapexInstanceHeader );

	// p�esko�� popis MapSquaru
	LOAD_ASSERT ( cMapFile.GetPosition () + dwMapSquareDescriptionSize <= 
		cMapFile.GetLength () );
	(void)cMapFile.Seek ( dwMapSquareDescriptionSize, CFile::current );
}

// zni�� MapSquare
void CSMapSquare::Delete () 
{
	// zni�� seznam jednotek
	m_cUnitList.RemoveAll ();
}

//////////////////////////////////////////////////////////////////////
// Operace s jednotkami
//////////////////////////////////////////////////////////////////////

// aktualizuje MapSquare po odebr�n� jednotky "pUnit"
void CSMapSquare::UnitDeleted ( CSUnit *pUnit ) 
{
	ASSERT ( pUnit != NULL );

	// zjist� index civilizace odebran� jednotky
	DWORD dwCivilizationIndex = pUnit->GetCivilizationIndex ();

	ASSERT ( m_cCivilizations.GetAt ( dwCivilizationIndex ) );

// aktualizuje p��znak um�st�n� civilizace odeb�ran� jednotky v MapSquaru

	// ukazatel na jednotku MapSquaru
	CSUnit *pMapSquareUnit;

	// zjist� pozici prvn� jednotky v seznamu jednotek
	POSITION posUnit = m_cUnitList.GetHeadPosition ();

	// projede jednotky MapSquaru
	while ( m_cUnitList.GetNext ( posUnit, pMapSquareUnit ) )
	{
		// zjist�, je-li to jednotka civilizace odeb�ran� jednotky
		if ( pMapSquareUnit->GetCivilizationIndex () == dwCivilizationIndex )
		{	// jedn� se o jednotku civilizace odeb�ran� jednotky
			// nen� t�eba mazat p��znak um�st�n� civilizace v MapSquaru
			return;
		}
	}
	// civilizace odeb�ran� jednotky se v MapSquaru ji� nenal�z�
	m_cCivilizations.ClearAt ( dwCivilizationIndex );
}

//////////////////////////////////////////////////////////////////////
// Operace s jednotkami v MapSquarech
//////////////////////////////////////////////////////////////////////

// ukazatel na MapSquare mapu jednotek
typedef signed char *TUnitMapMapSquarePointer;

// vytvo�� mapy jednotek MainLoop threadu a FindPathLoop threadu
void CSMapSquare::CreateUnitMaps () 
{
	ASSERT ( m_aMainLoopUnitMapMapSquares == NULL );
	ASSERT ( m_aFindPathLoopUnitMapMapSquares == NULL );
	ASSERT ( ( g_cMap.GetMapSizeMapSquare ().cx > 0 ) && 
		( g_cMap.GetMapSizeMapSquare ().cy > 0 ) );

	// po�et MapSquar� mapy jednotek
	DWORD dwUnitMapMapSquareCount = g_cMap.GetMapSizeMapSquare ().cx * 
		g_cMap.GetMapSizeMapSquare ().cy;

	// alokuje pole ukazatel� na MapSquary mapy jednotek MainLoop threadu
	m_aMainLoopUnitMapMapSquares = new TUnitMapMapSquarePointer[dwUnitMapMapSquareCount];
	// alokuje pole ukazatel� na MapSquary mapy jednotek FindPathLoop threadu
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

	// inicializuje memory pool MapSquar� mapy jednotek MainLoop threadu
	m_cMainLoopUnitMapMapSquareMemoryPool.Create ( MAP_SQUARE_SIZE * MAP_SQUARE_SIZE );
	// inicializuje memory pool MapSquar� mapy jednotek FindPathLoop threadu
	m_cFindPathLoopUnitMapMapSquareMemoryPool.Create ( MAP_SQUARE_SIZE * MAP_SQUARE_SIZE );
}

// zni�� mapy jednotek MainLoop threadu a FindPathLoop threadu
void CSMapSquare::DeleteUnitMaps () 
{
	// zjist�, je-li pole ukazatel� na MapSquary mapy jednotek MainLoop threadu platn�
	if ( m_aMainLoopUnitMapMapSquares != NULL )
	{	// pole ukazatel� na MapSquary mapy jednotek MainLoop threadu je platn�
		// zni�� pole ukazatel� na MapSquary mapy jednotek MainLoop threadu
		delete [] m_aMainLoopUnitMapMapSquares;
		m_aMainLoopUnitMapMapSquares = NULL;
		// zni�� memory pool MapSquar� mapy jednotek MainLoop threadu
		m_cMainLoopUnitMapMapSquareMemoryPool.Delete ();
	}

	// zjist�, je-li pole ukazatel� na MapSquary mapy jednotek FindPathLoop threadu platn�
	if ( m_aFindPathLoopUnitMapMapSquares != NULL )
	{	// pole ukazatel� na MapSquary mapy jednotek FindPathLoop threadu je platn�
		// zni�� pole ukazatel� na MapSquary mapy jednotek FindPathLoop threadu
		delete [] m_aFindPathLoopUnitMapMapSquares;
		m_aFindPathLoopUnitMapMapSquares = NULL;
		// zni�� memory pool MapSquar� mapy jednotek FindPathLoop threadu
		m_cFindPathLoopUnitMapMapSquareMemoryPool.Delete ();
	}
}

// zjist�, lze-li jednotku "pUnit" um�stit na pozici "pointPosition" z MainLoop threadu 
//		(je-li nastaven p��znak "bMapSquaresLocked", nezamyk� MapSquary)
BOOL CSMapSquare::MainLoopCanPlaceUnit ( CSUnit *pUnit, CPointDW pointPosition, 
	BOOL bMapSquaresLocked ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );

	// ���ka jednotky
	DWORD dwUnitWidth = pUnit->GetMoveWidth ();
	// offset posunut� �tverce jednotky
	DWORD dwUnitRectOffset = dwUnitWidth / 2;
	// �tverec jednotky
	CPointDW pointUnitLeftTop ( pointPosition.x - dwUnitRectOffset, pointPosition.y - 
		dwUnitRectOffset );
	CPointDW pointUnitRightBottom ( pointPosition.x - dwUnitRectOffset + dwUnitWidth - 1, 
		pointPosition.y - dwUnitRectOffset + dwUnitWidth - 1 );

	// zjist�, zasahuje-li �tverec jednotky mimo mapu
	if ( ( pointUnitRightBottom.x > g_cMap.GetRightBottomCornerMapCell ().x ) || 
		( pointUnitRightBottom.y > g_cMap.GetRightBottomCornerMapCell ().y ) )
	{	// �tverec jednotky zasahuje mimo mapu
		// vr�t� p��znak neum�st�n� jednotky
		return FALSE;
	}

	// zjist�, jsou-li zam�eny MapSquary
	if ( !bMapSquaresLocked )
	{	// MapSquary nejsou zam�eny
		// zamkne MapSquary
		VERIFY ( g_cMap.m_mutexMapSquaresLock.Lock () );
	}

	// zkontroluje osazen� �tverce jednotky jin�mi jednotkami
	for ( DWORD dwPositionX = pointUnitLeftTop.x; dwPositionX <= pointUnitRightBottom.x; 
		dwPositionX++ )
	{
		for ( DWORD dwPositionY = pointUnitLeftTop.y; dwPositionY <= 
			pointUnitRightBottom.y; dwPositionY++ )
		{
			// index MapSquaru mapy jednotek
			DWORD dwUnitMapMapSquareIndex = dwPositionX / MAP_SQUARE_SIZE + ( dwPositionY / 
				MAP_SQUARE_SIZE ) * g_cMap.GetMapSizeMapSquare ().cx;

			// zjist�, je-li MapSquare mapy jednotek platn�
			if ( m_aMainLoopUnitMapMapSquares[dwUnitMapMapSquareIndex] == NULL )
			{	// MapSquare mapy jednotek nen� platn�
				// sou�adnice MapSquaru mapy jednotek
				CPointDW pointUnitMapMapSquareIndex ( dwPositionX / MAP_SQUARE_SIZE, 
					dwPositionY / MAP_SQUARE_SIZE );
				// alokuje nov� MapSquare mapy jednotek
				signed char *pUnitMapMapSquare = 
					m_aMainLoopUnitMapMapSquares[dwUnitMapMapSquareIndex] = 
					(signed char *)m_cMainLoopUnitMapMapSquareMemoryPool.Allocate ();
				m_cMainLoopUnitMapMapSquareIndexStack.Push ( dwUnitMapMapSquareIndex );

				// inicializuje nov� MapSquare mapy jednotek
				memset ( pUnitMapMapSquare, 1, MAP_SQUARE_SIZE * MAP_SQUARE_SIZE );

			// nakresl� MapSquare mapy jednotek

				// �tverec MapSquaru mapy jednotek
				CPointDW pointUnitMapMapSquareLeftTop ( pointUnitMapMapSquareIndex.x * 
					MAP_SQUARE_SIZE, pointUnitMapMapSquareIndex.y * MAP_SQUARE_SIZE );
				CPointDW pointUnitMapMapSquareRightBottom ( pointUnitMapMapSquareLeftTop.x + 
					MAP_SQUARE_SIZE - 1, pointUnitMapMapSquareLeftTop.y + MAP_SQUARE_SIZE - 1 );

				// projede okoln� MapSquary
				for ( DWORD dwSurroundingPositionIndex = 9; 
					dwSurroundingPositionIndex-- > 0; )
				{
					// ukazatel na MapSquare
					CSMapSquare *pMapSquare = g_cMap.GetMapSquareFromIndexNull ( 
						pointUnitMapMapSquareIndex + 
						m_aSurroundingPositionOffset[dwSurroundingPositionIndex] );
					// zjist�, je-li MapSquare platn�
					if ( pMapSquare == NULL )
					{	// MapSquare nen� platn�
						// pokra�uje ve zpracov�n� dal��ho MapSquaru
						continue;
					}

					// ukazatel na jednotku MapSquaru
					CSUnit *pMapSquareUnit;
					// pozice jednotky MapSquaru v seznamu jednotek MapSquaru
					POSITION posMapSquareUnit = pMapSquare->m_cUnitList.GetHeadPosition ();

					// vykresl� jednotky MapSquaru
					while ( CSUnitList::GetNext ( posMapSquareUnit, pMapSquareUnit ) )
					{
						// zjist�, jedn�-li se o um�s�ovanou jednotku
						if ( pMapSquareUnit == pUnit )
						{	// jedn� se o um�s�ovanou jednotku
							// nech� vykreslit dal�� jednotku MapSquaru
							continue;
						}

						// zjist�, jedn�-li se o vzdu�nou jednotku
						if ( pMapSquareUnit->GetVerticalPosition () >= AIR_VERTICAL_POSITION )
						{	// jedn� se o vzdu�nou jednotku
							// nech� vykreslit dal�� jednotku MapSquaru
							continue;
						}

						// ���ka jednotky MapSquaru
						DWORD dwMapSquareUnitWidth = pMapSquareUnit->GetMoveWidth ();
						// pozice jednotky MapSquaru
						CPointDW pointMapSquareUnitPosition = pMapSquareUnit->GetPosition ();
						// offset posunut� �tverce jednotky MapSquaru
						DWORD dwMapSquareUnitRectOffset = dwMapSquareUnitWidth / 2;
						// �tverec jednotky MapSquaru
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

						// o��zne �tverec jednotky MapSquaru �tvercem MapSquaru mapy jednotek
						pointMapSquareUnitLeftTop.x = max ( pointMapSquareUnitLeftTop.x, 
							pointUnitMapMapSquareLeftTop.x );
						pointMapSquareUnitLeftTop.y = max ( pointMapSquareUnitLeftTop.y, 
							pointUnitMapMapSquareLeftTop.y );
						pointMapSquareUnitRightBottom.x = min ( pointMapSquareUnitRightBottom.x, 
							pointUnitMapMapSquareRightBottom.x );
						pointMapSquareUnitRightBottom.y = min ( pointMapSquareUnitRightBottom.y, 
							pointUnitMapMapSquareRightBottom.y );

						// vykresl� �tverec jednotky MapSquaru do MapSquaru mapy jednotek
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
								// ozna�� m�sto v map� jednotek za obsazen�
								pUnitMapMapSquare[dwPosX + dwPosY * MAP_SQUARE_SIZE] = 0;
							}
						}
						// �tverec jednotky MapSquaru byl vykreslen
					}
					// jednotky MapSquaru byly vykresleny
				}
				// v�echny okoln� MapSquary byly vykresleny
			}
			// MapSquare mapy jednotek je platn�

			ASSERT ( m_aMainLoopUnitMapMapSquares[dwUnitMapMapSquareIndex] != NULL );

			// index MapCellu v MapSquaru mapy jednotek
			DWORD dwMapCellIndex = dwPositionX % MAP_SQUARE_SIZE + ( dwPositionY % 
				MAP_SQUARE_SIZE ) * MAP_SQUARE_SIZE;

			// zjist�, je-li MapCell obsazen jinou jednotkou
			if ( !m_aMainLoopUnitMapMapSquares[dwUnitMapMapSquareIndex][dwMapCellIndex] )
			{	// MapCell je ji� obsazen jinou jednotkou
				ASSERT ( m_aMainLoopUnitMapMapSquares[dwUnitMapMapSquareIndex][dwMapCellIndex] 
					== 0 );
				// zjist�, byly-li zam�eny MapSquary
				if ( !bMapSquaresLocked )
				{	// MapSquary nebyly zam�eny
					// odemkne MapSquary
					VERIFY ( g_cMap.m_mutexMapSquaresLock.Unlock () );
				}

				// vr�t� p��znak neum�st�n� jednotky
				return FALSE;
			}
			// MapCell �tverce jednotky nen� obsazen jinou jednotkou
			ASSERT ( m_aMainLoopUnitMapMapSquares[dwUnitMapMapSquareIndex][dwMapCellIndex] 
				== 1 );
		}
	}
	// ve �tverci jednotky nen� jin� jednotka

	// zjist�, byly-li zam�eny MapSquary
	if ( !bMapSquaresLocked )
	{	// MapSquary nebyly zam�eny
		// odemkne MapSquary
		VERIFY ( g_cMap.m_mutexMapSquaresLock.Unlock () );
	}

	// vr�t� p��znak um�st�n� jednotky
	return TRUE;
}

// ukon�� um�s�ov�n� jednotky z MainLoop threadu
void CSMapSquare::MainLoopFinishPlacingUnit () 
{
	ASSERT ( g_cMap.IsMainLoopThread () );

	// index MapSquaru mapy jednotek MainLoop threadu
	DWORD dwIndex;

	// zni�� MapSquary mapy jednotek MainLoop threadu
	while ( m_cMainLoopUnitMapMapSquareIndexStack.Pop ( dwIndex ) )
	{
		ASSERT ( dwIndex < g_cMap.GetMapSizeMapSquare ().cx * 
			g_cMap.GetMapSizeMapSquare ().cy );
		ASSERT ( m_aMainLoopUnitMapMapSquares[dwIndex] != NULL );

		// zni�� MapSquare mapy jednotek MainLoop threadu
		m_cMainLoopUnitMapMapSquareMemoryPool.Free ( 
			m_aMainLoopUnitMapMapSquares[dwIndex] );
		m_aMainLoopUnitMapMapSquares[dwIndex] = NULL;
	}
}

// zjist�, lze-li jednotku "pUnit" um�stit na pozici "pointPosition" z FindPathLoop threadu 
//		(je-li nastaven p��znak "bMapSquaresLocked", nezamyk� MapSquary)
BOOL CSMapSquare::FindPathLoopCanPlaceUnit ( CSUnit *pUnit, CPointDW pointPosition, 
	BOOL bMapSquaresLocked ) 
{
	ASSERT ( g_cMap.m_pFindPathLoopThread->m_nThreadID == GetCurrentThreadId () );

	// ���ka jednotky
	DWORD dwUnitWidth = pUnit->GetMoveWidth ();
	// offset posunut� �tverce jednotky
	DWORD dwUnitRectOffset = dwUnitWidth / 2;
	// �tverec jednotky
	CPointDW pointUnitLeftTop ( pointPosition.x - dwUnitRectOffset, pointPosition.y - 
		dwUnitRectOffset );
	CPointDW pointUnitRightBottom ( pointPosition.x - dwUnitRectOffset + dwUnitWidth - 1, 
		pointPosition.y - dwUnitRectOffset + dwUnitWidth - 1 );

	// zjist�, zasahuje-li �tverec jednotky mimo mapu
	if ( ( pointUnitRightBottom.x > g_cMap.GetRightBottomCornerMapCell ().x ) || 
		( pointUnitRightBottom.y > g_cMap.GetRightBottomCornerMapCell ().y ) )
	{	// �tverec jednotky zasahuje mimo mapu
		// vr�t� p��znak neum�st�n� jednotky
		return FALSE;
	}

	// zjist�, jsou-li zam�eny MapSquary
	if ( !bMapSquaresLocked )
	{	// MapSquary nejsou zam�eny
		// zamkne MapSquary
		VERIFY ( g_cMap.m_mutexMapSquaresLock.Lock () );
	}

	// zkontroluje osazen� �tverce jednotky jin�mi jednotkami
	for ( DWORD dwPositionX = pointUnitLeftTop.x; dwPositionX <= pointUnitRightBottom.x; 
		dwPositionX++ )
	{
		for ( DWORD dwPositionY = pointUnitLeftTop.y; dwPositionY <= 
			pointUnitRightBottom.y; dwPositionY++ )
		{
			// index MapSquaru mapy jednotek
			DWORD dwUnitMapMapSquareIndex = dwPositionX / MAP_SQUARE_SIZE + ( dwPositionY / 
				MAP_SQUARE_SIZE ) * g_cMap.GetMapSizeMapSquare ().cx;

			// zjist�, je-li MapSquare mapy jednotek platn�
			if ( m_aFindPathLoopUnitMapMapSquares[dwUnitMapMapSquareIndex] == NULL )
			{	// MapSquare mapy jednotek nen� platn�
				// sou�adnice MapSquaru mapy jednotek
				CPointDW pointUnitMapMapSquareIndex ( dwPositionX / MAP_SQUARE_SIZE, 
					dwPositionY / MAP_SQUARE_SIZE );
				// alokuje nov� MapSquare mapy jednotek
				signed char *pUnitMapMapSquare = 
					m_aFindPathLoopUnitMapMapSquares[dwUnitMapMapSquareIndex] = 
					(signed char *)m_cFindPathLoopUnitMapMapSquareMemoryPool.Allocate ();
				m_cFindPathLoopUnitMapMapSquareIndexStack.Push ( dwUnitMapMapSquareIndex );

				// inicializuje nov� MapSquare mapy jednotek
				memset ( pUnitMapMapSquare, 1, MAP_SQUARE_SIZE * MAP_SQUARE_SIZE );

			// nakresl� MapSquare mapy jednotek

				// �tverec MapSquaru mapy jednotek
				CPointDW pointUnitMapMapSquareLeftTop ( pointUnitMapMapSquareIndex.x * 
					MAP_SQUARE_SIZE, pointUnitMapMapSquareIndex.y * MAP_SQUARE_SIZE );
				CPointDW pointUnitMapMapSquareRightBottom ( pointUnitMapMapSquareLeftTop.x + 
					MAP_SQUARE_SIZE - 1, pointUnitMapMapSquareLeftTop.y + MAP_SQUARE_SIZE - 1 );

				// projede okoln� MapSquary
				for ( DWORD dwSurroundingPositionIndex = 9; 
					dwSurroundingPositionIndex-- > 0; )
				{
					// ukazatel na MapSquare
					CSMapSquare *pMapSquare = g_cMap.GetMapSquareFromIndexNull ( 
						pointUnitMapMapSquareIndex + 
						m_aSurroundingPositionOffset[dwSurroundingPositionIndex] );
					// zjist�, je-li MapSquare platn�
					if ( pMapSquare == NULL )
					{	// MapSquare nen� platn�
						// pokra�uje ve zpracov�n� dal��ho MapSquaru
						continue;
					}

					// ukazatel na jednotku MapSquaru
					CSUnit *pMapSquareUnit;
					// pozice jednotky MapSquaru v seznamu jednotek MapSquaru
					POSITION posMapSquareUnit = pMapSquare->m_cUnitList.GetHeadPosition ();

					// vykresl� jednotky MapSquaru
					while ( CSUnitList::GetNext ( posMapSquareUnit, pMapSquareUnit ) )
					{
						// zjist�, jedn�-li se o um�s�ovanou jednotku
						if ( pMapSquareUnit == pUnit )
						{	// jedn� se o um�s�ovanou jednotku
							// nech� vykreslit dal�� jednotku MapSquaru
							continue;
						}

						// zjist�, jedn�-li se o vzdu�nou jednotku
						if ( pMapSquareUnit->GetVerticalPosition () >= AIR_VERTICAL_POSITION )
						{	// jedn� se o vzdu�nou jednotku
							// nech� vykreslit dal�� jednotku MapSquaru
							continue;
						}

						// ���ka jednotky MapSquaru
						DWORD dwMapSquareUnitWidth = pMapSquareUnit->GetMoveWidth ();
						// pozice jednotky MapSquaru
						CPointDW pointMapSquareUnitPosition = pMapSquareUnit->GetPosition ();
						// offset posunut� �tverce jednotky MapSquaru
						DWORD dwMapSquareUnitRectOffset = dwMapSquareUnitWidth / 2;
						// �tverec jednotky MapSquaru
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

						// o��zne �tverec jednotky MapSquaru �tvercem MapSquaru mapy jednotek
						pointMapSquareUnitLeftTop.x = max ( pointMapSquareUnitLeftTop.x, 
							pointUnitMapMapSquareLeftTop.x );
						pointMapSquareUnitLeftTop.y = max ( pointMapSquareUnitLeftTop.y, 
							pointUnitMapMapSquareLeftTop.y );
						pointMapSquareUnitRightBottom.x = min ( pointMapSquareUnitRightBottom.x, 
							pointUnitMapMapSquareRightBottom.x );
						pointMapSquareUnitRightBottom.y = min ( pointMapSquareUnitRightBottom.y, 
							pointUnitMapMapSquareRightBottom.y );

						// vykresl� �tverec jednotky MapSquaru do MapSquaru mapy jednotek
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
								// ozna�� m�sto v map� jednotek za obsazen�
								pUnitMapMapSquare[dwPosX + dwPosY * MAP_SQUARE_SIZE] = 0;
							}
						}
						// �tverec jednotky MapSquaru byl vykreslen
					}
					// jednotky MapSquaru byly vykresleny
				}
				// v�echny okoln� MapSquary byly vykresleny
			}
			// MapSquare mapy jednotek je platn�

			ASSERT ( m_aFindPathLoopUnitMapMapSquares[dwUnitMapMapSquareIndex] != NULL );

			// index MapCellu v MapSquaru mapy jednotek
			DWORD dwMapCellIndex = dwPositionX % MAP_SQUARE_SIZE + ( dwPositionY % 
				MAP_SQUARE_SIZE ) * MAP_SQUARE_SIZE;

			// zjist�, je-li MapCell obsazen jinou jednotkou
			if ( !m_aFindPathLoopUnitMapMapSquares[dwUnitMapMapSquareIndex][dwMapCellIndex] )
			{	// MapCell je ji� obsazen jinou jednotkou
				ASSERT ( m_aFindPathLoopUnitMapMapSquares[dwUnitMapMapSquareIndex][dwMapCellIndex] 
					== 0 );
				// zjist�, byly-li zam�eny MapSquary
				if ( !bMapSquaresLocked )
				{	// MapSquary nebyly zam�eny
					// odemkne MapSquary
					VERIFY ( g_cMap.m_mutexMapSquaresLock.Unlock () );
				}

				// vr�t� p��znak neum�st�n� jednotky
				return FALSE;
			}
			// MapCell �tverce jednotky nen� obsazen jinou jednotkou
			ASSERT ( m_aFindPathLoopUnitMapMapSquares[dwUnitMapMapSquareIndex][dwMapCellIndex] 
				== 1 );
		}
	}
	// ve �tverci jednotky nen� jin� jednotka

	// zjist�, byly-li zam�eny MapSquary
	if ( !bMapSquaresLocked )
	{	// MapSquary nebyly zam�eny
		// odemkne MapSquary
		VERIFY ( g_cMap.m_mutexMapSquaresLock.Unlock () );
	}

	// vr�t� p��znak um�st�n� jednotky
	return TRUE;
}

// ukon�� um�s�ov�n� jednotky z FindPathLoop threadu
void CSMapSquare::FindPathLoopFinishPlacingUnit () 
{
	ASSERT ( g_cMap.m_pFindPathLoopThread->m_nThreadID == GetCurrentThreadId () );

	// index MapSquaru mapy jednotek FindPathLoop threadu
	DWORD dwIndex;

	// zni�� MapSquary mapy jednotek FindPathLoop threadu
	while ( m_cFindPathLoopUnitMapMapSquareIndexStack.Pop ( dwIndex ) )
	{
		ASSERT ( dwIndex < g_cMap.GetMapSizeMapSquare ().cx * 
			g_cMap.GetMapSizeMapSquare ().cy );
		ASSERT ( m_aFindPathLoopUnitMapMapSquares[dwIndex] != NULL );

		// zni�� MapSquare mapy jednotek FindPathLoop threadu
		m_cFindPathLoopUnitMapMapSquareMemoryPool.Free ( 
			m_aFindPathLoopUnitMapMapSquares[dwIndex] );
		m_aFindPathLoopUnitMapMapSquares[dwIndex] = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Informace o MapSquare
//////////////////////////////////////////////////////////////////////

// vr�t� index MapSquaru
CPointDW CSMapSquare::GetIndex () 
{
	// index MapSquaru
	CPointDW pointIndex;

	// zjist� index MapSquaru v poli MapSquar�
	DWORD dwMapSquareIndex = ( (DWORD)this - (DWORD)g_cMap.m_pMapSquares ) / 
		sizeof ( CSMapSquare );
	ASSERT ( dwMapSquareIndex < g_cMap.m_sizeMapMapSquare.cx * 
		g_cMap.m_sizeMapMapSquare.cy );
	ASSERT ( &g_cMap.m_pMapSquares[dwMapSquareIndex] == this );

	// vypo�te index MapSquaru
	pointIndex.x = dwMapSquareIndex % g_cMap.m_sizeMapMapSquare.cx;
	pointIndex.y = dwMapSquareIndex / g_cMap.m_sizeMapMapSquare.cx;
	ASSERT ( pointIndex.y < g_cMap.m_sizeMapMapSquare.cy );

	// vr�t� index MapSquaru
	return pointIndex;
}

//////////////////////////////////////////////////////////////////////
// Debuggovac� informace
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// zkontroluje spr�vn� data objektu (TRUE=OK)
BOOL CSMapSquare::CheckValidData () 
{
	return TRUE;
}

// zkontroluje ne�kodn� data objektu (TRUE=OK)
BOOL CSMapSquare::CheckEmptyData () 
{
	ASSERT ( m_cUnitList.IsEmpty () );

	return TRUE;
}

#endif //_DEBUG
