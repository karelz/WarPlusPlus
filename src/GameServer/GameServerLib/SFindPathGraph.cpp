/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída grafu pro hledání cesty
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SFindPathGraph.h"

#include "SMap.h"
#include "Common\Map\MFindPathGraphs.h"
#include "SPath.h"

IMPLEMENT_DYNAMIC ( CSFindPathGraph, CObject )

#define FIND_LOCAL_PATH_TOLERACE		((double)1.3)

#define FindLocalPathValuation( StartDistance, EndDistance ) (DWORD)( (DWORD)EndDistance + ((DWORD)StartDistance >> 2) )
#define FIND_LOCAL_PATH_MAX_VALUATION FindLocalPathValuation ( 250, 750 )
#define FindLocalPathAbs( a ) (DWORD)( ((int)(a) < 0) ? -(int)(a) : (int)(a) )
inline DWORD FindLocalPathDistance( CPointDW &cFirstPosition, CPointDW &cSecondPosition )
{ 
  DWORD dwXDist = FindLocalPathAbs ( (int)cSecondPosition.x - (int)cFirstPosition.x );
  DWORD dwYDist = FindLocalPathAbs ( (int)cSecondPosition.y - (int)cFirstPosition.y );
  return (dwXDist < dwYDist ) ? (dwXDist * 14 + (dwYDist - dwXDist) * 10) :
    (dwYDist * 14 + (dwXDist - dwYDist) * 10);
}

//////////////////////////////////////////////////////////////////////
// Grafy pro hledání cesty
//////////////////////////////////////////////////////////////////////

// poèet grafù pro hledání cesty
DWORD CSFindPathGraph::m_dwFindPathGraphCount = DWORD_MAX;
// ukazatel na pole grafù pro hledání cesty
CSFindPathGraph *CSFindPathGraph::m_aFindPathGraph = NULL;
// prázdnı hint cesty
struct CSFindPathGraph::SPathHint CSFindPathGraph::m_sEmptyPathHint ( 0 );

//////////////////////////////////////////////////////////////////////
// Data pro hledání cesty
//////////////////////////////////////////////////////////////////////

// pole vzdáleností MapCellù
CArray2DOnDemand<DWORD> CSFindPathGraph::m_cMapCellDistance ( 10 );
// prioritní fronta pozic na mapì
CSmartPriorityQueue<CPointDW> CSFindPathGraph::m_cPositionPriorityQueue ( FIND_LOCAL_PATH_MAX_VALUATION + 1, FIND_LOCAL_PATH_MAX_VALUATION + 1);
// pole offsetù pozic okolních pozic na mapì
CPointDW CSFindPathGraph::m_aSurroundingPositionOffset[8] = 
{
	CPointDW ( (DWORD)-1, (DWORD)-1 ),
	CPointDW ( 0, (DWORD)-1 ),
	CPointDW ( 1, (DWORD)-1 ),
	CPointDW ( 1, 0 ),
	CPointDW ( 1, 1 ),
	CPointDW ( 0, 1 ),
	CPointDW ( (DWORD)-1, 1 ),
	CPointDW ( (DWORD)-1, 0 )
};

//////////////////////////////////////////////////////////////////////
// Data pro hledání cesty mezi branami
//////////////////////////////////////////////////////////////////////

// poèet bran mostù nejvìtšího grafu pro hledání cesty
DWORD CSFindPathGraph::m_dwMaxBridgeGateCount = 0;
// pole vzdáleností bran mostù
struct CSFindPathGraph::SBridgeGateDistance *CSFindPathGraph::m_aBridgeGateDistance = 
	NULL;
// seznam bran mostù
CSelfPooledList<DWORD> CSFindPathGraph::m_cBridgeGateList ( 200 );

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSFindPathGraph::CSFindPathGraph () 
{
	m_dwFindPathGraphID = 0;

	m_aAreaTable = NULL;
	m_aBridgeGateTable = NULL;
	m_aPathHintTable = NULL;

	m_pMap = NULL;
}

// destruktor
CSFindPathGraph::~CSFindPathGraph () 
{
	ASSERT ( m_dwFindPathGraphID == 0 );

	ASSERT ( m_aAreaTable == NULL );
	ASSERT ( m_aBridgeGateTable == NULL );
	ASSERT ( m_aPathHintTable == NULL );

	ASSERT ( m_pMap == NULL );
}

//////////////////////////////////////////////////////////////////////
// Inicializace a znièení dat objektu
//////////////////////////////////////////////////////////////////////

// vytvoøí graf pro hledání cesty s ID "dwID" z archivu "cArchive"
void CSFindPathGraph::Create ( DWORD dwID, CDataArchive cArchive ) 
{
	ASSERT ( m_dwFindPathGraphID == 0 );

	ASSERT ( m_aAreaTable == NULL );
	ASSERT ( m_aBridgeGateTable == NULL );
	ASSERT ( m_aPathHintTable == NULL );

	ASSERT ( m_pMap == NULL );

	ASSERT ( m_aBridgeGateDistance == NULL );

	// uschová si ID grafu pro hledání cesty
	m_dwFindPathGraphID = dwID;

// naète hlavièkovı soubor grafu pro hledání cesty

	// velikost tabulky oblastí
	DWORD dwAreaTableSize;
	// velikost tabulky bran mostù
	DWORD dwBridgeGateTableSize;
	// velikost tabulky hintù cest
	DWORD dwPathHintTableSize;
	// poèet bran mostù
	DWORD dwBridgeGateCount;

	// otevøe hlavièkovı soubor grafu pro hledání cesty
	CArchiveFile cHeaderFile = cArchive.CreateFile ( FIND_PATH_GRAPH_HEADER_FILE_NAME, 
		CFile::modeRead | CFile::shareDenyWrite );

	// naète hlavièku grafu pro hledání cesty
	{
		SMFindPathGraphHeader sFindPathGraphHeader;

		// naète hlavièku grafu pro hledání cesty
		LOAD_ASSERT ( cHeaderFile.Read ( &sFindPathGraphHeader, sizeof ( sFindPathGraphHeader ) ) == sizeof ( sFindPathGraphHeader ) );

		// zjistí velikost tabulky oblastí
		dwAreaTableSize = sFindPathGraphHeader.m_dwAreaTableSize;
		LOAD_ASSERT ( ( dwAreaTableSize <= 4 * dwAreaTableSize ) && 
			( dwAreaTableSize <= 2 * dwAreaTableSize ) );
		// zjistí velikost tabulky bran mostù
		dwBridgeGateTableSize = sFindPathGraphHeader.m_dwBridgeGateTableSize;
		LOAD_ASSERT ( dwBridgeGateTableSize % 4 == 0 );
		LOAD_ASSERT ( ( dwBridgeGateTableSize <= 4 * dwBridgeGateTableSize ) && 
			( dwBridgeGateTableSize <= 2 * dwBridgeGateTableSize ) );
		dwBridgeGateCount = dwBridgeGateTableSize / 4;
		// zjistí velikost tabulky hintù cest
		dwPathHintTableSize = sFindPathGraphHeader.m_dwPathHintTableSize;
		LOAD_ASSERT ( ( dwPathHintTableSize <= 4 * dwPathHintTableSize ) && 
			( dwPathHintTableSize <= 2 * dwPathHintTableSize ) );

		// zjistí, je-li poèet bran mostù vìtší
		if ( dwBridgeGateCount > m_dwMaxBridgeGateCount )
		{	// poèet bran mostù je vìtší
			// aktualizuje poèet bran mostù
			m_dwMaxBridgeGateCount = dwBridgeGateCount;
		}
	}

	// naète tabulku oblastí
	if ( dwAreaTableSize != 0 )
	{	// tabulka oblastí není prázdná
		// alokuje tabulku oblastí
		m_aAreaTable = new DWORD[dwAreaTableSize];
		// naète tabulku oblastí
		LOAD_ASSERT ( cHeaderFile.Read ( m_aAreaTable, dwAreaTableSize * sizeof ( DWORD ) ) == dwAreaTableSize * sizeof ( DWORD ) );
	}
	// tabulka oblastí je naètena

	// alokuje a naète tabulku bran mostù
	if ( dwBridgeGateTableSize != 0 )
	{	// tabulka bran mostù není prázdná
		// alokuje tabulku bran mostù
		m_aBridgeGateTable = new struct SBridgeGate[dwBridgeGateCount];
		ASSERT ( sizeof ( struct SBridgeGate ) == 4 * sizeof ( DWORD ) );
		// naète tabulku bran mostù
		LOAD_ASSERT ( cHeaderFile.Read ( m_aBridgeGateTable, dwBridgeGateTableSize * sizeof ( DWORD ) ) == dwBridgeGateTableSize * sizeof ( DWORD ) );
	}

	// alokuje a naète tabulku hintù cest
	if ( dwPathHintTableSize != 0 )
	{	// tabulka hintù cest není prázdná
		// alokuje tabulku hintù cest
		m_aPathHintTable = new DWORD[dwPathHintTableSize];
		// naète tabulku hintù cest
		LOAD_ASSERT ( cHeaderFile.Read ( m_aPathHintTable, dwPathHintTableSize * sizeof ( DWORD ) ) == dwPathHintTableSize * sizeof ( DWORD ) );
	}

	// zkontroluje konec hlavièkového souboru
	LOAD_ASSERT ( cHeaderFile.GetPosition () == cHeaderFile.GetLength () );
	cHeaderFile.Close ();

// inicializuje tabulky

	// inicializuje tabulku oblastí
	for ( DWORD dwAreaIndex = 0; dwAreaIndex < dwAreaTableSize; )
	{
		// ukazatel na oblast
		struct SArea *pArea = (struct SArea *)( m_aAreaTable + dwAreaIndex );
		LOAD_ASSERT ( ( pArea->dwBridgeGateCount >= 1 ) && ( pArea->dwBridgeGateCount < 
			( pArea->dwBridgeGateCount * pArea->dwBridgeGateCount + 2 ) ) );
		// aktualizuje index další oblasti
		dwAreaIndex += pArea->dwBridgeGateCount * pArea->dwBridgeGateCount + 2;
		LOAD_ASSERT ( dwAreaIndex <= dwAreaTableSize );

		// ukazatel na ukazatele na brány mostù oblasti
		struct SBridgeGate **pBridgeGate = (struct SBridgeGate **)( pArea + 1 );
		// inicializuje ukazatele na brány mostù oblasti
		for ( DWORD dwBridgeGateIndex = pArea->dwBridgeGateCount; dwBridgeGateIndex-- > 0; 
			pBridgeGate++ )
		{
			// inicializuje ukazatel na bránu mostu
			LOAD_ASSERT ( ( *(DWORD *)pBridgeGate < dwBridgeGateCount ) );
			*pBridgeGate = m_aBridgeGateTable + *(DWORD *)pBridgeGate;
		}

		// ukazatel na cestu mezi branami mostù oblasti
		struct SBridgeGatePath *pBridgeGatePath = (struct SBridgeGatePath *)pBridgeGate;
		// inicializuje cesty mezi branami mostù oblasti
		for ( DWORD dwIndex = pArea->dwBridgeGateCount * ( pArea->dwBridgeGateCount - 1 ) / 
			2; dwIndex-- > 0; pBridgeGatePath++ )
		{
			LOAD_ASSERT ( pBridgeGatePath->dwBridgeGateDistance <= INFINITE_DISTANCE );
			// zjistí, je-li ukazatel na hint cesty platnı
			if ( (DWORD)pBridgeGatePath->pPathHint == INVALID_PATH_HINT )
			{	// ukazatel na hint cesty není platnı
				// aktualizuje ukazatel na prázdnı hunt cesty
				pBridgeGatePath->pPathHint = &m_sEmptyPathHint;
			}
			else
			{	// ukazatel na hint cesty je platnı
				LOAD_ASSERT ( (DWORD)pBridgeGatePath->pPathHint + 1 < 
					dwPathHintTableSize );
				// inicializuje ukazatel na hint cesty
				pBridgeGatePath->pPathHint = (struct SPathHint *)( m_aPathHintTable + 
					(DWORD)pBridgeGatePath->pPathHint );
				LOAD_ASSERT ( pBridgeGatePath->pPathHint->dwCheckPointCount < 2 * 
					pBridgeGatePath->pPathHint->dwCheckPointCount );
				LOAD_ASSERT ( ((DWORD *)pBridgeGatePath->pPathHint) + 2 * 
					pBridgeGatePath->pPathHint->dwCheckPointCount < m_aPathHintTable + 
					dwPathHintTableSize );
			}
			// ukazatel na hint cesty je inicializován
		}
		// cesty mezi branami mostù jsou inicializovány
		ASSERT ( (DWORD *)pBridgeGatePath == m_aAreaTable + dwAreaIndex );
	}
	// tabulka oblastí je inicializována
	ASSERT ( dwAreaIndex == dwAreaTableSize );

	// ukazatel na bránu mostu
	struct SBridgeGate *pBridgeGate = m_aBridgeGateTable;
	// inicializuje tabulku bran mostù
	for ( DWORD dwBridgeGateIndex = dwBridgeGateCount; dwBridgeGateIndex-- > 0; 
		pBridgeGate++ )
	{
		LOAD_ASSERT ( ( pBridgeGate->dwX < g_cMap.GetMapSizeMapCell ().cx ) && 
			( pBridgeGate->dwY < g_cMap.GetMapSizeMapCell ().cy ) );
		// inicializuje ukazatel na oblast mostu
		LOAD_ASSERT ( (DWORD)pBridgeGate->pBridgeArea < dwAreaTableSize );
		pBridgeGate->pBridgeArea = (struct SArea *)( m_aAreaTable + 
			(DWORD)pBridgeGate->pBridgeArea );
		// inicializuje ukazatel na oblast
		LOAD_ASSERT ( (DWORD)pBridgeGate->pArea < dwAreaTableSize );
		pBridgeGate->pArea = (struct SArea *)( m_aAreaTable + (DWORD)pBridgeGate->pArea );
	}
	// tabulka bran mostù je inicializována
	ASSERT ( pBridgeGate == m_aBridgeGateTable + dwBridgeGateCount );

	// inicializuje tabulku hintù cest
	for ( DWORD dwPathHintIndex = 0; dwPathHintIndex < dwPathHintTableSize; )
	{
		// ukazatel na hint cesty
		struct SPathHint *pPathHint = (struct SPathHint *)( m_aPathHintTable + 
			dwPathHintIndex );
		LOAD_ASSERT ( pPathHint->dwCheckPointCount < 2 * pPathHint->dwCheckPointCount );
		// aktualizuje index dalšího hintu cesty
		dwPathHintIndex += 2 * pPathHint->dwCheckPointCount + 1;
		LOAD_ASSERT ( dwPathHintIndex <= dwPathHintTableSize );

		// ukazatel na kontrolní bod hintu cesty
		struct SPathHintCheckPoint *pPathHintCheckPoint = 
			(struct SPathHintCheckPoint *)( pPathHint + 1 );
		// zkontroluje kontrolní body hintu cesty
		for ( DWORD dwIndex = pPathHint->dwCheckPointCount; dwIndex-- > 0; 
			pPathHintCheckPoint++ )
		{
			LOAD_ASSERT ( ( pPathHintCheckPoint->dwX < g_cMap.GetMapSizeMapCell ().cx ) && 
				( pPathHintCheckPoint->dwY < g_cMap.GetMapSizeMapCell ().cy ) );
		}
		// kontrolní body hintu cesty jsou v poøádku
	}
	// tabulka hintù cest je inicializována
	ASSERT ( dwPathHintIndex == dwPathHintTableSize );

// naète mapu grafu pro hledání cesty

	// otevøe datovı soubor grafu pro hledání cesty
	CArchiveFile cDataFile = cArchive.CreateFile ( _T("beta.version.data"), 
		CFile::modeRead | CFile::shareDenyWrite );

	// zjistí velikost mapy
	DWORD dwMapSize = g_cMap.GetMapSizeMapCell ().cx * g_cMap.GetMapSizeMapCell ().cy;
	// alokuje mapu grafu pro hledání cesty
	m_pMap = new signed char[dwMapSize];
	// naète mapu grafu pro hledání cesty
	LOAD_ASSERT ( cDataFile.Read ( m_pMap, dwMapSize ) == dwMapSize );

	// zkontroluje konec datového souboru
	LOAD_ASSERT ( cDataFile.GetPosition () == cDataFile.GetLength () );
	cDataFile.Close ();

// zkontroluje oznaèení bran mostù na mapì

	// ukazatel na bránu mostu
	pBridgeGate = m_aBridgeGateTable;

	// projede brány mostù
	for ( dwBridgeGateIndex = dwBridgeGateCount; dwBridgeGateIndex-- > 0; pBridgeGate++ )
	{
		// zkontroluje oznaèení brány mostu na mapì
		LOAD_ASSERT ( IsBridgeGateMapCell ( GetMapCellAt ( CPointDW ( pBridgeGate->dwX, 
			pBridgeGate->dwY ) ) ) );
	}
}

// znièí graf pro hledání cesty
void CSFindPathGraph::Delete () 
{
	// zneškodní ID grafu pro hledání cesty
	m_dwFindPathGraphID = 0;

	// znièí tabulku oblastí
	if ( m_aAreaTable != NULL )
	{	// tabulka oblastí není prázdná
		// znièí tabulku oblastí
		delete [] m_aAreaTable;
		m_aAreaTable = NULL;
	}

	// znièí tabulku bran mostù
	if ( m_aBridgeGateTable != NULL )
	{	// tabulka bran mostù není prázdná
		// znièí tabulku bran mostù
		delete [] m_aBridgeGateTable;
		m_aBridgeGateTable = NULL;
	}

	// znièí tabulku hintù cest
	if ( m_aPathHintTable != NULL )
	{	// tabulka hintù cest není prázdná
		// znièí tabulku hintù cest
		delete [] m_aPathHintTable;
		m_aPathHintTable = NULL;
	}

	// znièí mapu grafu pro hledání cesty
	if ( m_pMap != NULL )
	{	// mapa grafu pro hledání cesty není prázdná
		// znièí mapu grafu pro hledání cesty
		delete [] m_pMap;
		m_pMap = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Inicializace a znièení grafù pro hledání cesty
//////////////////////////////////////////////////////////////////////

// zaène vytváøení "dwFindPathGraphCount" grafù pro hledání cesty
void CSFindPathGraph::PreCreateFindPathGraphs ( DWORD dwFindPathGraphCount ) 
{
	ASSERT ( m_dwFindPathGraphCount == DWORD_MAX );
	ASSERT ( dwFindPathGraphCount != DWORD_MAX );
	ASSERT ( m_dwMaxBridgeGateCount == 0 );
	ASSERT ( m_aBridgeGateDistance == NULL );

	// uschová si poèet grafù pro hledání cesty
	m_dwFindPathGraphCount = dwFindPathGraphCount;

	// vytvoøí grafy pro hledání cesty
	m_aFindPathGraph = ( dwFindPathGraphCount == 0 ) ? NULL : 
		new CSFindPathGraph[dwFindPathGraphCount];

	// vytvoøí pole vzdáleností MapCellù
	m_cMapCellDistance.Create ( g_cMap.GetMapSizeMapCell ().cx, 
		g_cMap.GetMapSizeMapCell ().cy, 2 * MAP_SQUARE_SIZE, INFINITE_DISTANCE );
}

// vytvoøí graf pro hledání cesty s indexem "dwIndex", s ID "dwID" a se jménem "szName" 
//		v archivu mapy "cMapArchive", vrací ukazatel na vytvoøenı graf
CSFindPathGraph *CSFindPathGraph::CreateFindPathGraph ( DWORD dwIndex, DWORD dwID, 
	const char *szName, CDataArchive cMapArchive ) 
{
	ASSERT ( m_dwFindPathGraphCount != DWORD_MAX );
	ASSERT ( dwIndex < m_dwFindPathGraphCount );
	ASSERT ( m_aBridgeGateDistance == NULL );

	// jméno adresáøe grafu pro hledání cesty
	CString strFindPathGraphDirectoryName = FIND_PATH_GRAPHS_DIRECTORY _T("\\");
	strFindPathGraphDirectoryName += szName;
	// otevøe archiv grafu pro hledání cesty
	CDataArchive cFindPathGraphArchive = cMapArchive.CreateArchive ( 
		strFindPathGraphDirectoryName );

	// vyplní ID grafu pro hledání cesty
	m_aFindPathGraph[dwIndex].Create ( dwID, cFindPathGraphArchive );

	// vrátí ukazatel na graf pro hledání cesty s indexem "dwIndex"
	return m_aFindPathGraph + dwIndex;
}

// ukonèí vytváøení grafù pro hledání cesty
void CSFindPathGraph::PostCreateFindPathGraphs () 
{
	ASSERT ( m_dwFindPathGraphCount != DWORD_MAX );
	ASSERT ( m_aBridgeGateDistance == NULL );

	// alokuje pole vzdáleností bran mostù
	if ( m_dwMaxBridgeGateCount > 0 )
	{
		m_aBridgeGateDistance = new struct SBridgeGateDistance[m_dwMaxBridgeGateCount];
	}
}

// znièí grafy pro hledání cesty
void CSFindPathGraph::DeleteFindPathGraphs () 
{
	// zjití, jsou-li grafy pro hledání cesty vytvoøeny
	if ( ( m_dwFindPathGraphCount != DWORD_MAX ) && ( m_dwFindPathGraphCount != 0 ) )
	{	// grafy pro hledání cesty jsou vytvoøeny
		ASSERT ( m_aFindPathGraph != NULL );

		// znièí grafy pro hledání cesty
		for ( DWORD dwIndex = m_dwFindPathGraphCount; dwIndex-- > 0; )
		{
			// znièí graf pro hledání cesty
			m_aFindPathGraph[dwIndex].Delete ();
		}

		// znièí vytvoøené grafy pro hledání cesty
		delete [] m_aFindPathGraph;

		// zneškodní ukazatel na grafy pro hledání cesty
		m_aFindPathGraph = NULL;
	}
	else
	{	// grafy pro hledání cesty nejsou vytvoøeny
		ASSERT ( m_aFindPathGraph == NULL );
	}

	// zneškodní poèet grafù pro hledání cesty
	m_dwFindPathGraphCount = DWORD_MAX;

	// znièí pole vzdáleností MapCellù
	m_cMapCellDistance.Delete ();

	// znièí pole vzdáleností bran mostù
	if ( m_aBridgeGateDistance != NULL )
	{	// pole vzdáleností bran mostù není prázdné
		// znièí pole vzdáleností bran mostù
		delete [] m_aBridgeGateDistance;
		m_aBridgeGateDistance = NULL;
	}
	// zneškodní nejvìtší poèet bran mostù grafu pro hledání cesty
	m_dwMaxBridgeGateCount = 0;
}

//////////////////////////////////////////////////////////////////////
// Ukládání grafù pro hledání cesty (obdoba CPersistentObject metod)
//////////////////////////////////////////////////////////////////////

/*
find path graphs stored

	CSFindPathGraph *pFindPathGraphs
*/

// ukládání dat grafù pro hledání cesty
void CSFindPathGraph::PersistentSaveFindPathGraphs ( CPersistentStorage &storage ) 
{
	ASSERT ( m_dwFindPathGraphCount != DWORD_MAX );
	ASSERT ( ( ( m_dwFindPathGraphCount == 0 ) && ( m_aFindPathGraph == NULL ) ) || 
		( ( m_dwFindPathGraphCount > 0 ) && ( m_aFindPathGraph != NULL ) ) );

	// uloí ukazatel na první graf pro hledání cesty
	storage << m_aFindPathGraph;
}

// nahrávání pouze uloenıch grafù pro hledání cesty
void CSFindPathGraph::PersistentLoadFindPathGraphs ( CPersistentStorage &storage ) 
{
	ASSERT ( m_dwFindPathGraphCount != DWORD_MAX );

	// naète ukazatel na první starı graf pro hledání cesty
	CSFindPathGraph *aFindPathGraph;
	storage >> (void *&)aFindPathGraph;
	LOAD_ASSERT ( ( ( m_dwFindPathGraphCount == 0 ) && ( aFindPathGraph == NULL ) ) || 
		( ( m_dwFindPathGraphCount > 0 ) && ( aFindPathGraph != NULL ) ) );


	// zaregistruje ukazatele na grafy pro hledání cesty
	for ( DWORD dwIndex = m_dwFindPathGraphCount; dwIndex-- > 0; )
	{
		// zaregistruje ukazatel na graf pro hledání cesty
		storage.RegisterPointer ( aFindPathGraph + dwIndex, m_aFindPathGraph + dwIndex );
	}
}

// pøeklad ukazatelù grafù pro hledání cesty
void CSFindPathGraph::PersistentTranslatePointersFindPathGraphs ( 
	CPersistentStorage &storage ) 
{
}

// inicializace nahranıch grafù pro hledání cesty
void CSFindPathGraph::PersistentInitFindPathGraphs () 
{
}

//////////////////////////////////////////////////////////////////////
// Operace s grafy pro hledání cest
//////////////////////////////////////////////////////////////////////

// najde novou cestu šíøky "dwWidth" z místa "pointStart" do místa "pointEnd", vyplní 
//		seznam kontrolních bodù cesty "pCheckPointPositionList"
BOOL CSFindPathGraph::FindPath ( CPointDW pointStart, CPointDW pointEnd, 
	DWORD dwWidth, CSCheckPointPositionList *pCheckPointPositionList ) 
{
	ASSERT ( dwWidth > 0 );
	ASSERT ( pCheckPointPositionList->IsEmpty () );

	// index brány mostu zaèátku cesty
	DWORD dwStartBridgeGateIndex;
	// index brány mostu konce cesty
	DWORD dwEndBridgeGateIndex;

	// zjistí index nejbliší brány mostu zaèátku cesty
	if ( ( dwStartBridgeGateIndex = GetNearestBridgeGateIndex ( pointStart, 
		(signed char)( dwWidth * 2 ), pointEnd ) ) == DWORD_MAX )
	{	// neexistuje brána mostu zaèátku cesty
		// cesta neexistuje
		return FALSE;
	}
	else if ( dwStartBridgeGateIndex != DWORD_MAX - 1 )
	{	// byla nalezena nejbliší brána mostu zaèátku cesty
		// zjistí index nejbliší brány mostu konce cesty
		if ( ( dwEndBridgeGateIndex = GetNearestBridgeGateIndex ( pointEnd, 
			(signed char)( dwWidth * 2 ), pointStart, TRUE ) ) == DWORD_MAX )
		{	// neexistuje brána mostu zaèátku cesty
			// cesta neexistuje
			return FALSE;
		}
		else if ( dwEndBridgeGateIndex != DWORD_MAX - 1 )
		{	// byla nalezena nejbliší brána mostu konce cesty
			// zjistí, jedná-li se o shodné brány mostu nebo o brány mostu jedné oblasti
			if ( ( dwStartBridgeGateIndex != dwEndBridgeGateIndex ) && 
				( ( ( m_aBridgeGateTable[dwStartBridgeGateIndex].pArea != m_aBridgeGateTable[dwEndBridgeGateIndex].pArea ) && 
				( m_aBridgeGateTable[dwStartBridgeGateIndex].pArea != m_aBridgeGateTable[dwEndBridgeGateIndex].pBridgeArea ) ) || 
				( m_aBridgeGateTable[dwStartBridgeGateIndex].pArea->dwWidth < dwWidth ) ) && 
				( ( ( m_aBridgeGateTable[dwStartBridgeGateIndex].pBridgeArea != m_aBridgeGateTable[dwEndBridgeGateIndex].pArea ) && 
				( m_aBridgeGateTable[dwStartBridgeGateIndex].pBridgeArea != m_aBridgeGateTable[dwEndBridgeGateIndex].pBridgeArea ) ) || 
				( m_aBridgeGateTable[dwStartBridgeGateIndex].pBridgeArea->dwWidth < dwWidth ) ) )
			{	// nejedná se o shodné brány mostu ani o brány mostu jedné oblasti
				// ******************
				CPointDW pointSecondBridgeGate, pointPreLastBridgeGate;

				// najde cestu mezi branami mostù
				switch ( FindBridgeGatePath ( dwStartBridgeGateIndex, dwEndBridgeGateIndex, 
					dwWidth, pCheckPointPositionList, pointSecondBridgeGate, 
					pointPreLastBridgeGate ) )
				{
				// cesta mezi branami neexistuje
				case 0 :
					// cesta neexistuje
					return FALSE;
				// pøímá cesta by mohla bıt kratší (jedná se o krátkou cestu mezi branami)
				case 1 :
				case 2 :
					// znièí seznam pozic kontrolních bodù cesty
					pCheckPointPositionList->RemoveAll ();
					break;
				// byla nalezena cesta mezi branami
				default :
					ASSERT ( !pCheckPointPositionList->IsEmpty () );

					// ******************
					// najde novı zaèátek cesty
					CPointDW pointNewStart ( m_aBridgeGateTable[dwStartBridgeGateIndex].dwX, 
						m_aBridgeGateTable[dwStartBridgeGateIndex].dwY );
					{
						DWORD dwNewStartIndex = 0;
						CPointDW point;
						POSITION pos = pCheckPointPositionList->GetHeadPosition ();
						for ( DWORD dwIndex = 0; ; dwIndex++ )
						{
							VERIFY ( pCheckPointPositionList->GetNext ( pos, point ) );

							if ( point == pointSecondBridgeGate )
							{	// jedná se o druhou bránu cesty
								break;
							}

							if ( point.GetDistanceSquare ( pointStart ) <= 
								( 2 * MAX_CHECK_POINT_DISTANCE ) * 
								( 2 * MAX_CHECK_POINT_DISTANCE ) )
							{	// jedná se o blízkı kontrolní bod cesty
								dwNewStartIndex = dwIndex + 1;
								pointNewStart = point;
							}
						}
						while ( dwNewStartIndex-- > 0 )
						{
							(void)pCheckPointPositionList->RemoveFirst ();
						}

						// najde pøedposlední bránu cesty
						POSITION posPreLast = pCheckPointPositionList->GetHeadPosition ();
						do
						{
							VERIFY ( pCheckPointPositionList->GetNext ( posPreLast, point ) );
						}
						while ( point != pointPreLastBridgeGate );

						POSITION posLast = posPreLast;
						// najde novı konec cesty
						while ( point.GetDistanceSquare ( pointEnd ) > 
							( 2 * MAX_CHECK_POINT_DISTANCE ) * 
							( 2 * MAX_CHECK_POINT_DISTANCE ) )
						{
							posLast = posPreLast;
							if ( !pCheckPointPositionList->GetNext ( posPreLast, point ) )
							{
								break;
							}
						}
						// znièí poslední kontrolní body cesty
						while ( !pCheckPointPositionList->IsEmpty ( posLast ) )
						{
							(void)pCheckPointPositionList->RemovePosition ( posLast );
						}
					}

					// pozice posledního kontrolního bodu v seznamu
					POSITION posLastCheckPointPosition = 
						pCheckPointPositionList->GetHeadPosition ();
					CPointDW pointLastCheckPointPosition;
					// najde pozici posledního kontrolního bodu cesty
					while ( pCheckPointPositionList->GetNext ( posLastCheckPointPosition, 
						pointLastCheckPointPosition ) );

					// pozice brány mostu konce cesty
//					CPointDW pointEndBridgeGate ( 
//						m_aBridgeGateTable[dwEndBridgeGateIndex].dwX, 
//						m_aBridgeGateTable[dwEndBridgeGateIndex].dwY );

					// pøidá na konec cesty kontrolní body konce cesty
					FindShortPath ( pointLastCheckPointPosition, pointEnd, 
						(signed char)( dwWidth * 2 ), posLastCheckPointPosition );

					// pozice brány mostu zaèátku cesty
//					CPointDW pointStartBridgeGate ( 
//						m_aBridgeGateTable[dwStartBridgeGateIndex].dwX, 
//						m_aBridgeGateTable[dwStartBridgeGateIndex].dwY );

					// doplní cestu o kontrolní body zaèátku cesty
					FindShortPath ( pointStart, pointNewStart, (signed char)( dwWidth * 2 ), 
						pCheckPointPositionList->GetHeadPosition () );

					// ******* MOZNA CASEM TEST, JESTLI TO PRECE JEN JESTE NEJDE ZKRATIT !!!

					// vrátí pøíznak nalezení cesty
					return TRUE;
				}
				// cesta mezi branami je krátká
			}
			// jedná se o shodné brány mostù nebo o brány mostù jedné oblasti
		}
		// od konce cesty je zaèátek cesty nejbliší
	}
	// od zaèátku cesty je konec cesty nejbliší

	ASSERT ( pCheckPointPositionList->IsEmpty () );

	// vytvoøí kontrolní body krátké cesty
	FindShortPath ( pointStart, pointEnd, (signed char)( dwWidth * 2 ), 
		pCheckPointPositionList->GetHeadPosition () );

	// vrátí pøíznak nalezení cesty
	return TRUE;
}

// *****************************, POSITION posCheckPointPosition, CSPath *pPath
// najde lokální cestu šíøky "dwWidth" z místa "pointStart" do místa "pointEnd" 
//		pro jednotku "pUnit", vrací pøidané a upravené kontrolní body na pozici 
//		"posCheckPointPosition"
enum CSFindPathGraph::EFindLocalPathState CSFindPathGraph::FindLocalPath ( CSUnit *pUnit, 
	CPointDW pointStart, CPointDW pointEnd, DWORD dwWidth, CSPath *pPath ) 
{
#ifdef _DEBUG
	{
		// pozice prvního kontrolního bodu v seznamu kontrolních bodù cesty
		POSITION posFirstCheckPoint = pPath->m_cCheckPointPositionList.GetHeadPosition ();
		// pozice prvního kontrolního bodu cesty
		CPointDW pointFirstCheckPoint;
		// zjistí pozici prvního kontrolního bodu cesty
		VERIFY ( CSCheckPointPositionList::GetNext ( posFirstCheckPoint, 
			pointFirstCheckPoint ) );
		// zkontroluje pozici prvního kontrolního bodu cesty
		ASSERT ( pointFirstCheckPoint == pointEnd );
	}
#endif //_DEBUG

	ASSERT ( m_cPositionPriorityQueue.IsEmpty () );

	// šíøka MapCellu
	signed char cMapCellWidth = (signed char)( dwWidth * 2 );
	// ****************************************************************
	// ****************************************************************
	// ****************************************************************
	// *****************DOCASNE, DOKUD NEBUDOU HINTY Z MAPEDITORU
	// ****************************************************************
	// ****************************************************************
	// ****************************************************************
	if ( GetMapCellAt ( pointStart ) < cMapCellWidth )
	{
		return EFLPS_PathBlocked;
	}

	// ********************************
	BOOL bOriginalEnd = TRUE;
	// zjistí, lze-li jednotku umístit na cílovou pozici
	if ( ( GetMapCellAt ( pointEnd ) < cMapCellWidth ) || 
		!CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointEnd, FALSE ) )
	{	// jednotku nelze umístit na cílovou pozici
		// fronta pozic
		static CSelfPooledQueue<CPointDW> cPositionQueue ( 50 );

		// aktualizuje vzdálenost zakázaného MapCellu
		m_cMapCellDistance.GetAt ( pointEnd ) = DWORD_MAX;

		CPointDW pointEndNear ( NO_MAP_POSITION, NO_MAP_POSITION );

		DWORD dwCounter = MAX_CHECK_POINT_DISTANCE * MAX_CHECK_POINT_DISTANCE * 3;

		cPositionQueue.Add ( pointEnd );

		CPointDW pointPosition;
		while ( cPositionQueue.RemoveFirst ( pointPosition ) )
		{
			if ( --dwCounter == 0 )
			{
				cPositionQueue.RemoveAll ();
				break;
			}

			// zpracuje sousední pozice na mapì
			for ( int nIndex = 8; nIndex-- > 0; )
			{
				// sousední pozice na mapì
				CPointDW pointSurroundingPosition = pointPosition + 
					m_aSurroundingPositionOffset[nIndex];

				// zjistí, jedná-li se o pozici na mapì
				if ( ( pointSurroundingPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
					( pointSurroundingPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
				{	// nejedná se o pozici na mapì
					// ukonèí zpracovávání sousední pozice
					continue;
				}

				// vzdálenost sousední pozice
				DWORD &rdwSurroundingPositionDistance = 
					m_cMapCellDistance.GetAt ( pointSurroundingPosition );

				// zpracuje sousední pozici
				if ( rdwSurroundingPositionDistance == INFINITE_DISTANCE )
				{	// jedná se o dosud nenavštívenou pozici
					// zjistí, jedná-li se o povolenı MapCell
					if ( ( GetMapCellAt ( pointSurroundingPosition ) >= cMapCellWidth ) && 
						CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointSurroundingPosition, 
						FALSE ) )
					{	// jedná se o povolenı MapCell
						// aktualizuje cílovou pozici
						pointEndNear = pointSurroundingPosition;
						// vyprázdní frontu
						cPositionQueue.RemoveAll ();
						break;
					}
					else
					{	// jedná se o zakázanı MapCell
						// aktualizuje vzdálenost zakázaného MapCellu
						rdwSurroundingPositionDistance = DWORD_MAX;
					}
					cPositionQueue.Add ( pointSurroundingPosition );
				}
				// jedná se o zakázanı MapCell
				ASSERT ( rdwSurroundingPositionDistance == DWORD_MAX );
			}
			// sousední pozice na mapì byla zpracována
		}
		// fronta byla zpracována

		if ( pointEndNear == CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ) )
		{	// nebyla nalezena cílová pozice
			// ukonèí umísování jednotky
			CSMapSquare::FindPathLoopFinishPlacingUnit ();

			// zruší oznaèení pozic na mapì
			m_cMapCellDistance.Clear ();

			return EFLPS_PathBlocked;
		}

		if ( FIND_LOCAL_PATH_TOLERACE * FIND_LOCAL_PATH_TOLERACE * 
			(double)pointEnd.GetDistanceSquare ( pointEndNear ) >= 
			(double)pointEnd.GetDistanceSquare ( pUnit->GetPosition () ) )
		{	// náhradní cíl je pøíliš blízko
			// ukonèí umísování jednotky
			CSMapSquare::FindPathLoopFinishPlacingUnit ();

			// zruší oznaèení pozic na mapì
			m_cMapCellDistance.Clear ();

			pPath->RemoveCheckPoint ();

			return EFLPS_Found;
		}

		pointEnd = pointEndNear;
		bOriginalEnd = FALSE;
	}

	ASSERT ( GetMapCellAt ( pointStart ) >= cMapCellWidth );
	ASSERT ( GetMapCellAt ( pointEnd ) >= cMapCellWidth );

// hledání cesty na mapì

	// hledání cesty na mapì
	{
		// oznaèí vıchozí pozici na mapì
		m_cMapCellDistance.GetAt ( pointStart ) = 0;
		// oznaèí koncovou pozici na mapì
		m_cMapCellDistance.GetAt ( pointEnd ) = DWORD_MAX - 1;

		ASSERT ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointStart, FALSE ) );

    // Pomocná promìnná pro zjišování vzdálenosti
    DWORD dwHelpValuation;

		// pøidá do prioritní fronty zaèátek cesty
		m_cPositionPriorityQueue.Add ( pointStart, FindLocalPathValuation ( 0, FindLocalPathDistance ( pointStart, pointEnd ) ) );

		// pozice na mapì
		CPointDW pointPosition;
		// vzdálenost pozice od zaèátku
		DWORD dwPositionDistance;

		// zpracuje prioritní frontu pozic na mapì
		while ( m_cPositionPriorityQueue.Remove ( pointPosition ) )
		{
      // zjistit vzdálenost bodu od zacatku
			dwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

			// zpracuje sousední pozice na mapì
			for ( int nIndex = 8; nIndex-- > 0; )
			{
				// sousední pozice na mapì
				CPointDW pointSurroundingPosition = pointPosition + 
					m_aSurroundingPositionOffset[nIndex];

				// zjistí, jedná-li se o pozici na mapì
				if ( ( pointSurroundingPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
					( pointSurroundingPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
				{	// nejedná se o pozici na mapì
					// ukonèí zpracovávání sousední pozice
					continue;
				}

				// vzdálenost sousední pozice
				DWORD &rdwSurroundingPositionDistance = 
					m_cMapCellDistance.GetAt ( pointSurroundingPosition );

				// zpracuje sousední pozici
				if ( rdwSurroundingPositionDistance < INFINITE_DISTANCE )
				{	// jedná se o ji navštívenou pozici
					// zjistí jedná-li se o kratší cestu

          // pokud ano, nevadi -> vyresime pri hledani cesty pruchodem zpatky po vysledkach vlny
/*					if ( ( dwPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) < 
						rdwSurroundingPositionDistance )
					{	// jedná se o kratší cestu
						// smae pozici z prioritní fronty
						VERIFY ( m_cPositionPriorityQueue.RemoveElement ( 
							pointSurroundingPosition ) );
						// aktualizuje vzdálenost MapCellu
						rdwSurroundingPositionDistance = dwPositionDistance + 
							( ( nIndex & 0x01 ) ? 10 : 14 );

            // Zjistí ohodnocení bodu
            FindLocalPathDistance ( pointSurroundingPosition, pointEnd, dwHelpDistance );
            dwHelpValuation = FindLocalPathValuation ( rdwSurroundingPositionDistance, dwHelpDistance );

            // Pokud je ohodnocení pøíliš velké, nevloí ho
            if ( dwHelpValuation > FIND_LOCAL_PATH_MAX_VALUATION )
            {
				      // ukonèí umísování jednotky
				      CSMapSquare::FindPathLoopFinishPlacingUnit ();

				      // znièí prioritní frontu pozic na mapì
				      m_cPositionPriorityQueue.RemoveAll ();

				      // zruší oznaèení pozic na mapì
				      m_cMapCellDistance.Clear ();

				      // vrátí pøíznak zablokované cesty
				      return EFLPS_PathBlocked;
            }

						// pøidá pozici do prioritní fronty
						m_cPositionPriorityQueue.Add ( dwHelpValuation, 
							pointSurroundingPosition );
					}*/
					// nejedná se o kratší cestu
				}
				else if ( rdwSurroundingPositionDistance == INFINITE_DISTANCE )
				{	// jedná se o dosud nenavštívenou pozici
					// zjistí, jedná-li se o povolenı MapCell
					if ( ( GetMapCellAt ( pointSurroundingPosition ) >= cMapCellWidth ) && 
						CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointSurroundingPosition, 
						FALSE ) )
					{	// jedná se o povolenı MapCell
						// aktualizuje vzdálenost MapCellu
						rdwSurroundingPositionDistance = dwPositionDistance + 
							( ( nIndex & 0x01 ) ? 10 : 14 );

            // Zjistí ohodnocení bodu
            dwHelpValuation = FindLocalPathValuation ( FindLocalPathDistance ( pointStart, pointSurroundingPosition ),
              FindLocalPathDistance ( pointSurroundingPosition, pointEnd ) );

            // Pokud je ohodnocení pøíliš velké, nevloí ho
            if ( dwHelpValuation > FIND_LOCAL_PATH_MAX_VALUATION )
            {
				      // ukonèí umísování jednotky
				      CSMapSquare::FindPathLoopFinishPlacingUnit ();

				      // znièí prioritní frontu pozic na mapì
				      m_cPositionPriorityQueue.Clear ();

				      // zruší oznaèení pozic na mapì
				      m_cMapCellDistance.Clear ();

				      // vrátí pøíznak zablokované cesty
				      return EFLPS_PathBlocked;
            }

						// pøidá pozici do prioritní fronty
						m_cPositionPriorityQueue.Add ( pointSurroundingPosition, dwHelpValuation );
					}
					else
					{	// jedná se o zakázanı MapCell
						// aktualizuje vzdálenost zakázaného MapCellu
						rdwSurroundingPositionDistance = DWORD_MAX;
					}
				}
				else if ( rdwSurroundingPositionDistance != DWORD_MAX )
				{	// jedná se o koncovou pozici
					ASSERT ( rdwSurroundingPositionDistance == DWORD_MAX - 1 );

					// aktualizuje vzdálenost koncové pozice
					rdwSurroundingPositionDistance = dwPositionDistance + 
						( ( nIndex & 0x01 ) ? 10 : 14 );

					// znièí frontu pozic na mapì
					m_cPositionPriorityQueue.Clear ();
					// ukonèí zpracovávání okolních MapCellù
					break;
				}
				// sousední pozice je zpracována
			}
			// sousední pozice jsou zpracovány
		}
		// prioritní fronta pozic na mapì byla zpracována

		// zjistí, byl-li dosaen cíl cesty
		if ( m_cMapCellDistance.GetAt ( pointEnd ) == ( DWORD_MAX - 1 ) )
		{	// cíl cesty nebyl dosaen
			// ukonèí umísování jednotky
			CSMapSquare::FindPathLoopFinishPlacingUnit ();

			// znièí prioritní frontu pozic na mapì
			m_cPositionPriorityQueue.Clear ();

			// zruší oznaèení pozic na mapì
			m_cMapCellDistance.Clear ();

			// vrátí pøíznak zablokované jednotky
			return EFLPS_UnitBlocked;
		}
		// cíl cesty byl dosaen

		ASSERT ( m_cMapCellDistance.GetAt ( pointEnd ) < INFINITE_DISTANCE );

		// znièí prioritní frontu pozic na mapì
		m_cPositionPriorityQueue.Clear ();
	}
	// cesta na mapì je nalezena

// pøevede cestu na kontrolní body

	// ************************************
	if ( !bOriginalEnd )
	{
		pPath->RemoveCheckPoint ();
		// pøidá kontrolní bod koncového bodu cesty
		CSCheckPointPositionList::Insert ( 
			pPath->m_cCheckPointPositionList.GetHeadPosition (), pointEnd );
	}

	POSITION posCheckPointPosition = pPath->m_cCheckPointPositionList.GetHeadPosition ();

	// poslední zpracovanı bod cesty
	CPointDW pointLastPosition = pointEnd;
	// vzdálenost posledního zpracovaného bodu cesty
	DWORD dwLastPositionDistance = m_cMapCellDistance.GetAt ( pointEnd );

	// pøevede cestu na kontrolní body
	while ( dwLastPositionDistance != 0 )
	{
		// další bod cesty
		CPointDW pointNextPosition;
		// vzdálenost dalšího bodu cesty
		DWORD dwNextPositionDistance;

    // pomocne promenne pro nalezeni minima (nejvetsiho spadu)
    dwNextPositionDistance = INFINITE_DISTANCE;
    DWORD dwTryPositionDistance;
    CPointDW pointTryPosition;

		// najde další bod cesty
		for ( int nIndex = 8; nIndex-- > 0; )
		{
			// zjistí sousední pozici na mapì
			pointTryPosition = pointLastPosition + m_aSurroundingPositionOffset[nIndex];

			// zjistí, jedná-li se o pozici na mapì
			if ( ( pointTryPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
				( pointTryPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
			{	// nejedná se o pozici na mapì
				// ukonèí zpracovávání sousední pozice
				continue;
			}

			// vzdálenost sousední pozice na mapì
			dwTryPositionDistance = m_cMapCellDistance.GetAt ( pointTryPosition );

			// zjistí, jedná-li se o prozkoumanou pozici na mapì
			if ( dwTryPositionDistance >= INFINITE_DISTANCE )
			{	// jedná se o neprozkoumanou pozici na mapì
				ASSERT ( ( dwTryPositionDistance == INFINITE_DISTANCE ) || 
					( dwTryPositionDistance == DWORD_MAX ) );
				// ukonèí zpracovávání sousední pozice
				continue;
			}

/*			ASSERT ( ( dwNextPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) >= 
				dwLastPositionDistance );

			// zjistí, jedná-li se o následující bod cesty
			if ( ( dwNextPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) == 
				dwLastPositionDistance )
			{	// jedná se o následující bod cesty
				// nechá zpracovat bod cesty
				break;
			}*/
      if ( ( dwTryPositionDistance < dwLastPositionDistance ) && ( dwTryPositionDistance < dwNextPositionDistance ) )
      {
        // Ohodnoceni, klesa a je zatim nejmensi nalezene
        dwNextPositionDistance = dwTryPositionDistance;
        pointNextPosition = pointTryPosition;
        continue;
      }
		}
		// byl nalezen další bod cesty
		ASSERT ( dwNextPositionDistance < INFINITE_DISTANCE );

	// zjistí, existuje-li úseèka mezi nelezenım dalším bodem a koncem cesty

		// vzdálenost koncù úseèky v jednotlivıch osách
		CSizeDW sizeDistance = pointEnd.GetRectSize ( pointNextPosition );

		// zjistí, je-li úseèka pøíliš dlouhá
		if ( ( sizeDistance.cx * sizeDistance.cx + sizeDistance.cy * sizeDistance.cy ) >= 
			MAX_CHECK_POINT_DISTANCE * MAX_CHECK_POINT_DISTANCE )
		{	// úseèka je pøíliš dlouhá
			// pøidá kontrolní bod koncového bodu cesty
			CSCheckPointPositionList::Insert ( posCheckPointPosition, pointLastPosition );
			// aktualizuje konec cesty
			pointEnd = pointLastPosition;
			// aktualizuje vzdálenost bodù úseèky v jednotlivıch osách
			sizeDistance = pointEnd.GetRectSize ( pointNextPosition );
		}

		// zjistí, ve kterém rozmìru je vzdálenost koncù úseèky vìtší
		if ( sizeDistance.cy < sizeDistance.cx )
		{	// vzdálenost koncù úseèky je vìtší v ose X
			// pozice bodu úseèky
			CPointDW pointPosition;

			// pozice bodu úseèky v ose X
			pointPosition.x = pointNextPosition.x;
			// pøírùstek v ose X
			DWORD dwXDifference = ( pointNextPosition.x < pointEnd.x ) ? 1 : (DWORD)-1;

			// pøesná pozice bodu úseèky v ose Y
			double dbYPosition = (double)pointNextPosition.y + 0.5;
			// pøesnı pøírùstek v ose Y
			double dbYDifference = ( (double)pointEnd.y - (double)pointNextPosition.y ) / 
				(double)sizeDistance.cx;

			ASSERT ( pointPosition.x != pointEnd.x );

			// "nulty" krok iterace, osa X je implicitne posunuta o "pul mapcellu",
			// na ose y pridame polovinu dbYDifference k prvni hranici mapcellu
			pointPosition.x += dwXDifference;
			dbYPosition += dbYDifference / 2;
			// pozice bodu úseèky v ose Y
			pointPosition.y = (DWORD)dbYPosition;

			// zkontroluje MapCelly bodù úseèky
			while ( pointPosition.x != pointEnd.x )
			{
				// zkontroluje první okraj MapCellu úseèky
				{
					// vzdálenost bodu úseèky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjistí, je-li MapCell bodu úseèky povolenı
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu úseèky je zakázánı nebo dosud nenavštívenı
						// zjistí, jedná-li se o nenavštívenı MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedná se o nenavštívenı MapCell
							// zjistí, jedná-li se o povolenı MapCell
							if ( ( GetMapCellAt ( pointPosition ) >= cMapCellWidth ) && 
								CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
								FALSE ) )
							{	// jedná se o povolenı MapCell
								// aktalizuje informaci o povoleném MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedná se o zakázanı MapCell
								// aktalizuje informaci o zakázaném MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell byl navštíven
						}
						// jedná se o navštívenı MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjistí, je-li MapCell zakázanı
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zakázanı
							// pøidá kontrolní bod koncového bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, 
									pointLastPosition );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukonèí kontrolu MapCellù bodù úseèky
							break;
						}
						// MapCell je povolenı
					}
					// MapCell bodu úseèky je povolenı
				}
				// první okraj MapCellu úseèky je povolenı

				// posune pozici na druhı okraj MapCellu
				dbYPosition += dbYDifference;
				// pozice bodu úseèky v ose Y
				pointPosition.y = (DWORD)dbYPosition;

				// zkontroluje druhı okraj MapCellu úseèky
				{
					// aktualizuje vzdálenost bodu úseèky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjistí, je-li MapCell bodu úseèky povolenı
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu úseèky je zakázánı nebo dosud nenavštívenı
						// zjistí, jedná-li se o nenavštívenı MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedná se o nenavštívenı MapCell
							// zjistí, jedná-li se o povolenı MapCell
							if ( ( GetMapCellAt ( pointPosition ) >= cMapCellWidth ) && 
								CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
								FALSE ) )
							{	// jedná se o povolenı MapCell
								// aktalizuje informaci o povoleném MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedná se o zakázanı MapCell
								// aktalizuje informaci o zakázaném MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell byl navštíven
						}
						// jedná se o navštívenı MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjistí, je-li MapCell zakázanı
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zakázanı
							// pøidá kontrolní bod koncového bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, 
								pointLastPosition );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukonèí kontrolu MapCellù bodù úseèky
							break;
						}
						// MapCell je povolenı
					}
					// MapCell bodu úseèky je povolenı
				}
				// druhı okraj MapCellu úseèky je povolenı

				// posune se na další bod úseèky
				pointPosition.x += dwXDifference;
			}
			// MapCelly bodù úseèky jsou zkontrolovány
		}
		else if ( sizeDistance.cy > sizeDistance.cx )
		{	// vzdálenost koncù úseèky je vìtší v ose Y
			// pozice bodu úseèky
			CPointDW pointPosition;

			// pozice bodu úseèky v ose Y
			pointPosition.y = pointNextPosition.y;
			// pøírùstek v ose Y
			DWORD dwYDifference = ( pointNextPosition.y < pointEnd.y ) ? 1 : (DWORD)-1;

			// pøesná pozice v ose X
			double dbXPosition = (double)pointNextPosition.x + 0.5;
			// pøesnı pøírùstek v ose X
			double dbXDifference = ( (double)pointEnd.x - (double)pointNextPosition.x ) / 
				(double)sizeDistance.cy;

			ASSERT ( pointPosition.y != pointEnd.y );

			// "nulty" krok iterace, osa Y je implicitne posunuta o "pul mapcellu",
			// na ose X pridame polovinu dbXDifference k prvni hranici mapcellu
			pointPosition.y += dwYDifference;
			dbXPosition += dbXDifference / 2;
			// pozice bodu úseèky v ose X
			pointPosition.x = (DWORD)dbXPosition;

			// zkontroluje MapCelly bodù úseèky
			while ( pointPosition.y != pointEnd.y )
			{
				// zkontroluje první okraj MapCellu úseèky
				{
					// vzdálenost bodu úseèky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjistí, je-li MapCell bodu úseèky povolenı
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu úseèky je zakázánı nebo dosud nenavštívenı
						// zjistí, jedná-li se o nenavštívenı MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedná se o nenavštívenı MapCell
							// zjistí, jedná-li se o povolenı MapCell
							if ( ( GetMapCellAt ( pointPosition ) >= cMapCellWidth ) && 
								CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
								FALSE ) )
							{	// jedná se o povolenı MapCell
								// aktalizuje informaci o povoleném MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedná se o zakázanı MapCell
								// aktalizuje informaci o zakázaném MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell je navštívenı
						}
						// jedná se o navštívenı MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjistí, je-li MapCell zakázanı
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zakázanı
							// pøidá kontrolní bod koncového bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, 
								pointLastPosition );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukonèí kontrolu MapCellù bodù úseèky
							break;
						}
						// MapCell je povolenı
					}
					// MapCell bodu úseèky je povolenı
				}
				// první okraj MapCellu úseèky je povolenı

				// posune pozici na druhı okraj MapCellu
				dbXPosition += dbXDifference;
				// pozice bodu úseèky v ose X
				pointPosition.x = (DWORD)dbXPosition;

				// zkontroluje druhı okraj MapCellu úseèky
				{
					// aktualizuje vzdálenost bodu úseèky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjistí, je-li MapCell bodu úseèky povolenı
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu úseèky je zakázánı nebo dosud nenavštívenı
						// zjistí, jedná-li se o nenavštívenı MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedná se o nenavštívenı MapCell
							// zjistí, jedná-li se o povolenı MapCell
							if ( ( GetMapCellAt ( pointPosition ) >= cMapCellWidth ) && 
								CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
								FALSE ) )
							{	// jedná se o povolenı MapCell
								// aktalizuje informaci o povoleném MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedná se o zakázanı MapCell
								// aktalizuje informaci o zakázaném MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell je navštívenı
						}
						// jedná se o navštívenı MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjistí, je-li MapCell zakázanı
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zakázanı
							// pøidá kontrolní bod koncového bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, 
								pointLastPosition );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukonèí kontrolu MapCellù bodù úseèky
							break;
						}
						// MapCell je povolenı
					}
					// MapCell bodu úseèky je povolenı
				}
				// druhı okraj MapCellu úseèky je povolenı

				// posune se na další bod úseèky
				pointPosition.y += dwYDifference;
			}
			// MapCelly bodù úseèky jsou zkontrolovány
		}
		else
		{	// vzdálenosti koncù úseèky jsou stejné v ose X i Y
			// pozice bodu úseèky
			CPointDW pointPosition = pointNextPosition;
			// pøírùstek v osách X a Y
			CPointDW pointDifference ( ( pointNextPosition.x < pointEnd.x ) ? 1 : (DWORD)-1, 
				( pointNextPosition.y < pointEnd.y ) ? 1 : (DWORD)-1 );

			ASSERT ( pointNextPosition != pointEnd );

			// posune se na další MapCell
			pointPosition += pointDifference;

			// zkontroluje MapCelly bodù úseèky
			while ( pointPosition != pointEnd )
			{
				// vzdálenost bodu úseèky
				DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

				// zjistí, je-li MapCell bodu úseèky povolenı
				if ( rdwPositionDistance >= INFINITE_DISTANCE )
				{	// MapCell bodu úseèky je zakázánı nebo dosud nenavštívenı
					// zjistí, jedná-li se o nenavštívenı MapCell
					if ( rdwPositionDistance == INFINITE_DISTANCE )
					{	// jedná se o nenavštívenı MapCell
						// zjistí, jedná-li se o povolenı MapCell
						if ( ( GetMapCellAt ( pointPosition ) >= cMapCellWidth ) && 
							CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
							FALSE ) )
						{	// jedná se o povolenı MapCell
							// aktalizuje informaci o povoleném MapCellu
							rdwPositionDistance = INFINITE_DISTANCE - 1;
						}
						else
						{	// jedná se o zakázanı MapCell
							// aktalizuje informaci o zakázaném MapCellu
							rdwPositionDistance = DWORD_MAX;
						}
						// MapCell je navštívenı
					}
					// jedná se o navštívenı MapCell
					ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
						( rdwPositionDistance == DWORD_MAX ) );

					// zjistí, je-li MapCell zakázanı
					if ( rdwPositionDistance == DWORD_MAX )
					{	// MapCell je zakázanı
						// pøidá kontrolní bod koncového bodu cesty
						CSCheckPointPositionList::Insert ( posCheckPointPosition, 
							pointLastPosition );
						// aktualizuje konec cesty
						pointEnd = pointLastPosition;
						// ukonèí kontrolu MapCellù bodù úseèky
						break;
					}
					// MapCell je povolenı
				}
				// MapCell bodu úseèky je povolenı

				// posune se na další MapCell
				pointPosition += pointDifference;
			}
			// MapCelly bodù úseèky jsou zkontrolovány
		}
		// úseèka je zpracována

		// aktualizuje poslední zpracovanı bod cesty
		pointLastPosition = pointNextPosition;
		// aktualizuje vzdálenost posledního zpracovaného bodu cesty
		dwLastPositionDistance = dwNextPositionDistance;
	}
	// cesta byla pøevedena na kontrolní body

	// ukonèí umísování jednotky
	CSMapSquare::FindPathLoopFinishPlacingUnit ();

	// zruší oznaèení pozic na mapì
	m_cMapCellDistance.Clear ();

	// vrátí pøíznak nalezení cesty
	return EFLPS_Found;
}

// *****************************, POSITION posCheckPointPosition, CSPath *pPath
// najde vzdušnou lokální cestu z místa "pointStart" do místa "pointEnd" pro jednotku 
//		"pUnit", vrací pøidané a upravené kontrolní body na pozici "posCheckPointPosition"
enum CSFindPathGraph::EFindLocalPathState CSFindPathGraph::FindAirLocalPath ( 
	CSUnit *pUnit, CPointDW pointStart, CPointDW pointEnd, CSPath *pPath ) 
{
#ifdef _DEBUG
	{
		// pozice prvního kontrolního bodu v seznamu kontrolních bodù cesty
		POSITION posFirstCheckPoint = pPath->m_cCheckPointPositionList.GetHeadPosition ();
		// pozice prvního kontrolního bodu cesty
		CPointDW pointFirstCheckPoint;
		// zjistí pozici prvního kontrolního bodu cesty
		VERIFY ( CSCheckPointPositionList::GetNext ( posFirstCheckPoint, 
			pointFirstCheckPoint ) );
		// zkontroluje pozici prvního kontrolního bodu cesty
		ASSERT ( pointFirstCheckPoint == pointEnd );
	}
#endif //_DEBUG

	ASSERT ( m_cPositionPriorityQueue.IsEmpty () );

// najde cílovou pozici

	// ********************************
	BOOL bOriginalEnd = TRUE;
	// zjistí, lze-li jednotku umístit na cílovou pozici
	if ( !CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointEnd, FALSE ) )
	{	// jednotku nelze umístit na cílovou pozici
		// fronta pozic
		static CSelfPooledQueue<CPointDW> cPositionQueue ( 50 );

		// aktualizuje vzdálenost zakázaného MapCellu
		m_cMapCellDistance.GetAt ( pointEnd ) = DWORD_MAX;

		CPointDW pointEndNear ( NO_MAP_POSITION, NO_MAP_POSITION );

		DWORD dwCounter = MAX_CHECK_POINT_DISTANCE * MAX_CHECK_POINT_DISTANCE * 3;

		cPositionQueue.Add ( pointEnd );

		CPointDW pointPosition;
		while ( cPositionQueue.RemoveFirst ( pointPosition ) )
		{
			if ( --dwCounter == 0 )
			{
				cPositionQueue.RemoveAll ();
				break;
			}

			// zpracuje sousední pozice na mapì
			for ( int nIndex = 8; nIndex-- > 0; )
			{
				// sousední pozice na mapì
				CPointDW pointSurroundingPosition = pointPosition + 
					m_aSurroundingPositionOffset[nIndex];

				// zjistí, jedná-li se o pozici na mapì
				if ( ( pointSurroundingPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
					( pointSurroundingPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
				{	// nejedná se o pozici na mapì
					// ukonèí zpracovávání sousední pozice
					continue;
				}

				// vzdálenost sousední pozice
				DWORD &rdwSurroundingPositionDistance = 
					m_cMapCellDistance.GetAt ( pointSurroundingPosition );

				// zpracuje sousední pozici
				if ( rdwSurroundingPositionDistance == INFINITE_DISTANCE )
				{	// jedná se o dosud nenavštívenou pozici
					// zjistí, jedná-li se o povolenı MapCell
					if ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, 
						pointSurroundingPosition, FALSE ) )
					{	// jedná se o povolenı MapCell
						// aktualizuje cílovou pozici
						pointEndNear = pointSurroundingPosition;
						// vyprázdní frontu
						cPositionQueue.RemoveAll ();
						break;
					}
					else
					{	// jedná se o zakázanı MapCell
						// aktualizuje vzdálenost zakázaného MapCellu
						rdwSurroundingPositionDistance = DWORD_MAX;
					}
					cPositionQueue.Add ( pointSurroundingPosition );
				}
				// jedná se o zakázanı MapCell
				ASSERT ( rdwSurroundingPositionDistance == DWORD_MAX );
			}
			// sousední pozice na mapì byla zpracována
		}
		// fronta byla zpracována

		if ( pointEndNear == CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ) )
		{	// nebyla nalezena cílová pozice
			// ukonèí umísování jednotky
			CSMapSquare::FindPathLoopFinishPlacingUnit ();

			// zruší oznaèení pozic na mapì
			m_cMapCellDistance.Clear ();

			return EFLPS_PathBlocked;
		}

		if ( FIND_LOCAL_PATH_TOLERACE * FIND_LOCAL_PATH_TOLERACE * 
			(double)pointEnd.GetDistanceSquare ( pointEndNear ) >= 
			(double)pointEnd.GetDistanceSquare ( pUnit->GetPosition () ) )
		{	// náhradní cíl je pøíliš blízko
			// ukonèí umísování jednotky
			CSMapSquare::FindPathLoopFinishPlacingUnit ();

			// zruší oznaèení pozic na mapì
			m_cMapCellDistance.Clear ();

			pPath->RemoveCheckPoint ();

			return EFLPS_Found;
		}

		pointEnd = pointEndNear;
		bOriginalEnd = FALSE;
	}

// hledání cesty na mapì

	// hledání cesty na mapì
	{
		// oznaèí vıchozí pozici na mapì
		m_cMapCellDistance.GetAt ( pointStart ) = 0;
		// oznaèí koncovou pozici na mapì
		m_cMapCellDistance.GetAt ( pointEnd ) = DWORD_MAX - 1;

		ASSERT ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointStart, FALSE ) );

    // Pomocná promìnná pro zjišování vzdálenosti
    DWORD dwHelpValuation;

		// pøidá do prioritní fronty zaèátek cesty
		m_cPositionPriorityQueue.Add ( pointStart, FindLocalPathValuation ( 0, FindLocalPathDistance ( pointStart, pointEnd ) ) );

		// pozice na mapì
		CPointDW pointPosition;
		// vzdálenost pozice od zaèátku
		DWORD dwPositionDistance;

		// zpracuje prioritní frontu pozic na mapì
		while ( m_cPositionPriorityQueue.Remove ( pointPosition ) )
		{
      // zjistit vzdálenost bodu od zacatku
			dwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

			// zpracuje sousední pozice na mapì
			for ( int nIndex = 8; nIndex-- > 0; )
			{
				// sousední pozice na mapì
				CPointDW pointSurroundingPosition = pointPosition + 
					m_aSurroundingPositionOffset[nIndex];

				// zjistí, jedná-li se o pozici na mapì
				if ( ( pointSurroundingPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
					( pointSurroundingPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
				{	// nejedná se o pozici na mapì
					// ukonèí zpracovávání sousední pozice
					continue;
				}

				// vzdálenost sousední pozice
				DWORD &rdwSurroundingPositionDistance = 
					m_cMapCellDistance.GetAt ( pointSurroundingPosition );

				// zpracuje sousední pozici
				if ( rdwSurroundingPositionDistance < INFINITE_DISTANCE )
				{	// jedná se o ji navštívenou pozici
          // neresime, vyresi to hledani cesty po zpatku po vysledkach vlny

/*					// zjistí jedná-li se o kratší cestu
					if ( ( dwPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) < 
						rdwSurroundingPositionDistance )
					{	// jedná se o kratší cestu
						// smae pozici z prioritní fronty
						VERIFY ( m_cPositionPriorityQueue.RemoveElement ( 
							pointSurroundingPosition ) );
						// aktualizuje vzdálenost MapCellu
						rdwSurroundingPositionDistance = dwPositionDistance + 
							( ( nIndex & 0x01 ) ? 10 : 14 );
            
            // Zjistí ohodnocení bodu
            FindLocalPathDistance ( pointSurroundingPosition, pointEnd, dwHelpDistance );
            dwHelpValuation = FindLocalPathValuation ( rdwSurroundingPositionDistance, dwHelpDistance );

            if ( dwHelpValuation > FIND_LOCAL_PATH_MAX_VALUATION )
			      {	// vzdálenost pozice od zaèátku pøesahuje horní limit
				      // ukonèí umísování jednotky
				      CSMapSquare::FindPathLoopFinishPlacingUnit ();

				      // znièí prioritní frontu pozic na mapì
				      m_cPositionPriorityQueue.RemoveAll ();

				      // zruší oznaèení pozic na mapì
				      m_cMapCellDistance.Clear ();

				      // vrátí pøíznak zablokované cesty
				      return EFLPS_PathBlocked;
            }

            // pøidá pozici do prioritní fronty
						m_cPositionPriorityQueue.Add ( dwHelpValuation, 
							pointSurroundingPosition );
					}*/
					// nejedná se o kratší cestu
				}
				else if ( rdwSurroundingPositionDistance == INFINITE_DISTANCE )
				{	// jedná se o dosud nenavštívenou pozici
					// zjistí, jedná-li se o povolenı MapCell
					if ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, 
						pointSurroundingPosition, FALSE ) )
					{	// jedná se o povolenı MapCell
						// aktualizuje vzdálenost MapCellu
						rdwSurroundingPositionDistance = dwPositionDistance + 
							( ( nIndex & 0x01 ) ? 10 : 14 );

            // Zjistí ohodnocení bodu
            dwHelpValuation = FindLocalPathValuation ( FindLocalPathDistance ( pointStart, pointSurroundingPosition),
              FindLocalPathDistance ( pointSurroundingPosition, pointEnd ) );

            if ( dwHelpValuation > FIND_LOCAL_PATH_MAX_VALUATION )
			      {	// vzdálenost pozice od zaèátku pøesahuje horní limit
				      // ukonèí umísování jednotky
				      CSMapSquare::FindPathLoopFinishPlacingUnit ();

				      // znièí prioritní frontu pozic na mapì
				      m_cPositionPriorityQueue.Clear ();

				      // zruší oznaèení pozic na mapì
				      m_cMapCellDistance.Clear ();

				      // vrátí pøíznak zablokované cesty
				      return EFLPS_PathBlocked;
            }

						// pøidá pozici do prioritní fronty
						m_cPositionPriorityQueue.Add ( pointSurroundingPosition, dwHelpValuation );
					}
					else
					{	// jedná se o zakázanı MapCell
						// aktualizuje vzdálenost zakázaného MapCellu
						rdwSurroundingPositionDistance = DWORD_MAX;
					}
				}
				else if ( rdwSurroundingPositionDistance != DWORD_MAX )
				{	// jedná se o koncovou pozici
					ASSERT ( rdwSurroundingPositionDistance == DWORD_MAX - 1 );

					// aktualizuje vzdálenost koncové pozice
					rdwSurroundingPositionDistance = dwPositionDistance + 
						( ( nIndex & 0x01 ) ? 10 : 14 );

					// znièí frontu pozic na mapì
					m_cPositionPriorityQueue.Clear ();
					// ukonèí zpracovávání okolních MapCellù
					break;
				}
				// sousední pozice je zpracována
			}
			// sousední pozice jsou zpracovány
		}
		// prioritní fronta pozic na mapì byla zpracována

		// zjistí, byl-li dosaen cíl cesty
		if ( m_cMapCellDistance.GetAt ( pointEnd ) == ( DWORD_MAX - 1 ) )
		{	// cíl cesty nebyl dosaen
			// ukonèí umísování jednotky
			CSMapSquare::FindPathLoopFinishPlacingUnit ();

			// znièí prioritní frontu pozic na mapì
			m_cPositionPriorityQueue.Clear ();

			// zruší oznaèení pozic na mapì
			m_cMapCellDistance.Clear ();

			// vrátí pøíznak zablokované jednotky
			return EFLPS_UnitBlocked;
		}
		// cíl cesty byl dosaen

		ASSERT ( m_cMapCellDistance.GetAt ( pointEnd ) < INFINITE_DISTANCE );

		// znièí prioritní frontu pozic na mapì
		m_cPositionPriorityQueue.Clear ();
	}
	// cesta na mapì je nalezena

// pøevede cestu na kontrolní body

	// ************************************
	if ( !bOriginalEnd )
	{
		pPath->RemoveCheckPoint ();
		// pøidá kontrolní bod koncového bodu cesty
		CSCheckPointPositionList::Insert ( 
			pPath->m_cCheckPointPositionList.GetHeadPosition (), pointEnd );
	}

	POSITION posCheckPointPosition = pPath->m_cCheckPointPositionList.GetHeadPosition ();

	// poslední zpracovanı bod cesty
	CPointDW pointLastPosition = pointEnd;
	// vzdálenost posledního zpracovaného bodu cesty
	DWORD dwLastPositionDistance = m_cMapCellDistance.GetAt ( pointEnd );

	// pøevede cestu na kontrolní body
	while ( dwLastPositionDistance != 0 )
	{
		// další bod cesty
		CPointDW pointNextPosition;
		// vzdálenost dalšího bodu cesty
		DWORD dwNextPositionDistance;

    // pomocne promenne pro nalezeni minima (nejvetsiho spadu)
    dwNextPositionDistance = INFINITE_DISTANCE;
    DWORD dwTryPositionDistance;
    CPointDW pointTryPosition;

		// najde další bod cesty
		for ( int nIndex = 8; nIndex-- > 0; )
		{
			// zjistí sousední pozici na mapì
			pointTryPosition = pointLastPosition + m_aSurroundingPositionOffset[nIndex];

			// zjistí, jedná-li se o pozici na mapì
			if ( ( pointTryPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
				( pointTryPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
			{	// nejedná se o pozici na mapì
				// ukonèí zpracovávání sousední pozice
				continue;
			}

			// vzdálenost sousední pozice na mapì
			dwTryPositionDistance = m_cMapCellDistance.GetAt ( pointTryPosition );

			// zjistí, jedná-li se o prozkoumanou pozici na mapì
			if ( dwTryPositionDistance >= INFINITE_DISTANCE )
			{	// jedná se o neprozkoumanou pozici na mapì
				ASSERT ( ( dwTryPositionDistance == INFINITE_DISTANCE ) || 
					( dwTryPositionDistance == DWORD_MAX ) );
				// ukonèí zpracovávání sousední pozice
				continue;
			}

/*			ASSERT ( ( ( dwNextPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) >= 
				dwLastPositionDistance ) || ( pointLastPosition == pointEnd ) );

			// zjistí, jedná-li se o následující bod cesty
			if ( ( dwNextPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) == 
				dwLastPositionDistance )
			{	// jedná se o následující bod cesty
				// nechá zpracovat bod cesty
				break;
			}*/

      if ( ( dwTryPositionDistance < dwLastPositionDistance ) && ( dwTryPositionDistance < dwNextPositionDistance ) )
      {
        dwNextPositionDistance = dwTryPositionDistance;
        pointNextPosition = pointTryPosition;
        continue;
      }
		}
		// byl nalezen další bod cesty
		ASSERT ( dwNextPositionDistance < INFINITE_DISTANCE );

	// zjistí, existuje-li úseèka mezi nelezenım dalším bodem a koncem cesty

		// vzdálenost koncù úseèky v jednotlivıch osách
		CSizeDW sizeDistance = pointEnd.GetRectSize ( pointNextPosition );

		// zjistí, je-li úseèka pøíliš dlouhá
		if ( ( sizeDistance.cx * sizeDistance.cx + sizeDistance.cy * sizeDistance.cy ) >= 
			MAX_CHECK_POINT_DISTANCE * MAX_CHECK_POINT_DISTANCE )
		{	// úseèka je pøíliš dlouhá
			// pøidá kontrolní bod koncového bodu cesty
			CSCheckPointPositionList::Insert ( posCheckPointPosition, pointLastPosition );
			// aktualizuje konec cesty
			pointEnd = pointLastPosition;
			// aktualizuje vzdálenost bodù úseèky v jednotlivıch osách
			sizeDistance = pointEnd.GetRectSize ( pointNextPosition );
		}

		// zjistí, ve kterém rozmìru je vzdálenost koncù úseèky vìtší
		if ( sizeDistance.cy < sizeDistance.cx )
		{	// vzdálenost koncù úseèky je vìtší v ose X
			// pozice bodu úseèky
			CPointDW pointPosition;

			// pozice bodu úseèky v ose X
			pointPosition.x = pointNextPosition.x;
			// pøírùstek v ose X
			DWORD dwXDifference = ( pointNextPosition.x < pointEnd.x ) ? 1 : (DWORD)-1;

			// pøesná pozice bodu úseèky v ose Y
			double dbYPosition = (double)pointNextPosition.y + 0.5;
			// pøesnı pøírùstek v ose Y
			double dbYDifference = ( (double)pointEnd.y - (double)pointNextPosition.y ) / 
				(double)sizeDistance.cx;

			ASSERT ( pointPosition.x != pointEnd.x );

			// "nulty" krok iterace, osa X je implicitne posunuta o "pul mapcellu",
			// na ose y pridame polovinu dbYDifference k prvni hranici mapcellu
			pointPosition.x += dwXDifference;
			dbYPosition += dbYDifference / 2;
			// pozice bodu úseèky v ose Y
			pointPosition.y = (DWORD)dbYPosition;

			// zkontroluje MapCelly bodù úseèky
			while ( pointPosition.x != pointEnd.x )
			{
				// zkontroluje první okraj MapCellu úseèky
				{
					// vzdálenost bodu úseèky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjistí, je-li MapCell bodu úseèky povolenı
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu úseèky je zakázánı nebo dosud nenavštívenı
						// zjistí, jedná-li se o nenavštívenı MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedná se o nenavštívenı MapCell
							// zjistí, jedná-li se o povolenı MapCell
							if ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
								FALSE ) )
							{	// jedná se o povolenı MapCell
								// aktalizuje informaci o povoleném MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedná se o zakázanı MapCell
								// aktalizuje informaci o zakázaném MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell byl navštíven
						}
						// jedná se o navštívenı MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjistí, je-li MapCell zakázanı
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zakázanı
							// pøidá kontrolní bod koncového bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, 
								pointLastPosition );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukonèí kontrolu MapCellù bodù úseèky
							break;
						}
						// MapCell je povolenı
					}
					// MapCell bodu úseèky je povolenı
				}
				// první okraj MapCellu úseèky je povolenı

				// posune pozici na druhı okraj MapCellu
				dbYPosition += dbYDifference;
				// pozice bodu úseèky v ose Y
				pointPosition.y = (DWORD)dbYPosition;

				// zkontroluje druhı okraj MapCellu úseèky
				{
					// aktualizuje vzdálenost bodu úseèky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjistí, je-li MapCell bodu úseèky povolenı
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu úseèky je zakázánı nebo dosud nenavštívenı
						// zjistí, jedná-li se o nenavštívenı MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedná se o nenavštívenı MapCell
							// zjistí, jedná-li se o povolenı MapCell
							if ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
								FALSE ) )
							{	// jedná se o povolenı MapCell
								// aktalizuje informaci o povoleném MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedná se o zakázanı MapCell
								// aktalizuje informaci o zakázaném MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell byl navštíven
						}
						// jedná se o navštívenı MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjistí, je-li MapCell zakázanı
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zakázanı
							// pøidá kontrolní bod koncového bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, 
								pointLastPosition );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukonèí kontrolu MapCellù bodù úseèky
							break;
						}
						// MapCell je povolenı
					}
					// MapCell bodu úseèky je povolenı
				}
				// druhı okraj MapCellu úseèky je povolenı

				// posune se na další bod úseèky
				pointPosition.x += dwXDifference;
			}
			// MapCelly bodù úseèky jsou zkontrolovány
		}
		else if ( sizeDistance.cy > sizeDistance.cx )
		{	// vzdálenost koncù úseèky je vìtší v ose Y
			// pozice bodu úseèky
			CPointDW pointPosition;

			// pozice bodu úseèky v ose Y
			pointPosition.y = pointNextPosition.y;
			// pøírùstek v ose Y
			DWORD dwYDifference = ( pointNextPosition.y < pointEnd.y ) ? 1 : (DWORD)-1;

			// pøesná pozice v ose X
			double dbXPosition = (double)pointNextPosition.x + 0.5;
			// pøesnı pøírùstek v ose X
			double dbXDifference = ( (double)pointEnd.x - (double)pointNextPosition.x ) / 
				(double)sizeDistance.cy;

			ASSERT ( pointPosition.y != pointEnd.y );

			// "nulty" krok iterace, osa Y je implicitne posunuta o "pul mapcellu",
			// na ose X pridame polovinu dbXDifference k prvni hranici mapcellu
			pointPosition.y += dwYDifference;
			dbXPosition += dbXDifference / 2;
			// pozice bodu úseèky v ose X
			pointPosition.x = (DWORD)dbXPosition;

			// zkontroluje MapCelly bodù úseèky
			while ( pointPosition.y != pointEnd.y )
			{
				// zkontroluje první okraj MapCellu úseèky
				{
					// vzdálenost bodu úseèky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjistí, je-li MapCell bodu úseèky povolenı
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu úseèky je zakázánı nebo dosud nenavštívenı
						// zjistí, jedná-li se o nenavštívenı MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedná se o nenavštívenı MapCell
							// zjistí, jedná-li se o povolenı MapCell
							if ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
								FALSE ) )
							{	// jedná se o povolenı MapCell
								// aktalizuje informaci o povoleném MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedná se o zakázanı MapCell
								// aktalizuje informaci o zakázaném MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell je navštívenı
						}
						// jedná se o navštívenı MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjistí, je-li MapCell zakázanı
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zakázanı
							// pøidá kontrolní bod koncového bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, 
								pointLastPosition );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukonèí kontrolu MapCellù bodù úseèky
							break;
						}
						// MapCell je povolenı
					}
					// MapCell bodu úseèky je povolenı
				}
				// první okraj MapCellu úseèky je povolenı

				// posune pozici na druhı okraj MapCellu
				dbXPosition += dbXDifference;
				// pozice bodu úseèky v ose X
				pointPosition.x = (DWORD)dbXPosition;

				// zkontroluje druhı okraj MapCellu úseèky
				{
					// aktualizuje vzdálenost bodu úseèky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjistí, je-li MapCell bodu úseèky povolenı
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu úseèky je zakázánı nebo dosud nenavštívenı
						// zjistí, jedná-li se o nenavštívenı MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedná se o nenavštívenı MapCell
							// zjistí, jedná-li se o povolenı MapCell
							if ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
								FALSE ) )
							{	// jedná se o povolenı MapCell
								// aktalizuje informaci o povoleném MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedná se o zakázanı MapCell
								// aktalizuje informaci o zakázaném MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell je navštívenı
						}
						// jedná se o navštívenı MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjistí, je-li MapCell zakázanı
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zakázanı
							// pøidá kontrolní bod koncového bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, 
								pointLastPosition );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukonèí kontrolu MapCellù bodù úseèky
							break;
						}
						// MapCell je povolenı
					}
					// MapCell bodu úseèky je povolenı
				}
				// druhı okraj MapCellu úseèky je povolenı

				// posune se na další bod úseèky
				pointPosition.y += dwYDifference;
			}
			// MapCelly bodù úseèky jsou zkontrolovány
		}
		else
		{	// vzdálenosti koncù úseèky jsou stejné v ose X i Y
			// pozice bodu úseèky
			CPointDW pointPosition = pointNextPosition;
			// pøírùstek v osách X a Y
			CPointDW pointDifference ( ( pointNextPosition.x < pointEnd.x ) ? 1 : (DWORD)-1, 
				( pointNextPosition.y < pointEnd.y ) ? 1 : (DWORD)-1 );

			ASSERT ( pointNextPosition != pointEnd );

			// posune se na další MapCell
			pointPosition += pointDifference;

			// zkontroluje MapCelly bodù úseèky
			while ( pointPosition != pointEnd )
			{
				// vzdálenost bodu úseèky
				DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

				// zjistí, je-li MapCell bodu úseèky povolenı
				if ( rdwPositionDistance >= INFINITE_DISTANCE )
				{	// MapCell bodu úseèky je zakázánı nebo dosud nenavštívenı
					// zjistí, jedná-li se o nenavštívenı MapCell
					if ( rdwPositionDistance == INFINITE_DISTANCE )
					{	// jedná se o nenavštívenı MapCell
						// zjistí, jedná-li se o povolenı MapCell
						if ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
							FALSE ) )
						{	// jedná se o povolenı MapCell
							// aktalizuje informaci o povoleném MapCellu
							rdwPositionDistance = INFINITE_DISTANCE - 1;
						}
						else
						{	// jedná se o zakázanı MapCell
							// aktalizuje informaci o zakázaném MapCellu
							rdwPositionDistance = DWORD_MAX;
						}
						// MapCell je navštívenı
					}
					// jedná se o navštívenı MapCell
					ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
						( rdwPositionDistance == DWORD_MAX ) );

					// zjistí, je-li MapCell zakázanı
					if ( rdwPositionDistance == DWORD_MAX )
					{	// MapCell je zakázanı
						// pøidá kontrolní bod koncového bodu cesty
						CSCheckPointPositionList::Insert ( posCheckPointPosition, 
							pointLastPosition );
						// aktualizuje konec cesty
						pointEnd = pointLastPosition;
						// ukonèí kontrolu MapCellù bodù úseèky
						break;
					}
					// MapCell je povolenı
				}
				// MapCell bodu úseèky je povolenı

				// posune se na další MapCell
				pointPosition += pointDifference;
			}
			// MapCelly bodù úseèky jsou zkontrolovány
		}
		// úseèka je zpracována

		// aktualizuje poslední zpracovanı bod cesty
		pointLastPosition = pointNextPosition;
		// aktualizuje vzdálenost posledního zpracovaného bodu cesty
		dwLastPositionDistance = dwNextPositionDistance;
	}
	// cesta byla pøevedena na kontrolní body

	// ukonèí umísování jednotky
	CSMapSquare::FindPathLoopFinishPlacingUnit ();

	// zruší oznaèení pozic na mapì
	m_cMapCellDistance.Clear ();

	// vrátí pøíznak nalezení cesty
	return EFLPS_Found;
}

//////////////////////////////////////////////////////////////////////
// Pomocné operace pro hledání cesty
//////////////////////////////////////////////////////////////////////

// vloí na pozici "posCheckPointPosition" seznam kontrolních bodù krátké cesty pro šíøku 
//		MapCellu "cMapCellWidth" z místa "pointStart" do místa "pointEnd" (cesta ji urèitì 
//		existuje)
void CSFindPathGraph::FindShortPath ( CPointDW pointStart, CPointDW pointEnd, 
	signed char cMapCellWidth, POSITION posCheckPointPosition ) 
{
	ASSERT ( ( cMapCellWidth > 0 ) && ( ( cMapCellWidth % 2 ) == 0 ) );

	ASSERT ( m_cPositionPriorityQueue.IsEmpty () );

	ASSERT ( GetMapCellAt ( pointStart ) >= cMapCellWidth );
	ASSERT ( GetMapCellAt ( pointEnd ) >= cMapCellWidth );

	// zjistí, shoduje-li se poèátek a konec cesty
	if ( pointStart == pointEnd )
	{	// poèátek a konec cesty se shodují
		// vloí prázdnou cestu
		return;
	}
	// zaèátek a konec cesty se neshodují

// hledání cesty na mapì

	// oznaèí vıchozí pozici na mapì
	m_cMapCellDistance.GetAt ( pointStart ) = 0;
	// oznaèí koncovou pozici na mapì
	m_cMapCellDistance.GetAt ( pointEnd ) = DWORD_MAX - 1;

  // Pomocná promìnná pro zjišování vzdálenosti
  DWORD dwHelpValuation;

  // pøidá do prioritní fronty zaèátek cesty
	m_cPositionPriorityQueue.Add ( pointStart, FindLocalPathValuation ( 0, FindLocalPathDistance ( pointStart, pointEnd ) ) );

	// pozice na mapì
	CPointDW pointPosition;
	// vzdálenost pozice od zaèátku
	DWORD dwPositionDistance;

	// zpracuje prioritní frontu pozic na mapì
	while ( m_cPositionPriorityQueue.Remove ( pointPosition ) )
	{
    // zjisti vzdalenost bodu od zacatku
    dwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

		// zpracuje sousední pozice na mapì
		for ( int nIndex = 8; nIndex-- > 0; )
		{
			// sousední pozice na mapì
			CPointDW pointSurroundingPosition = pointPosition + 
				m_aSurroundingPositionOffset[nIndex];

			// zjistí, jedná-li se o pozici na mapì
			if ( ( pointSurroundingPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
				( pointSurroundingPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
			{	// nejedná se o pozici na mapì
				// ukonèí zpracovávání sousední pozice
				continue;
			}

			// vzdálenost sousední pozice
			DWORD &rdwSurroundingPositionDistance = m_cMapCellDistance.GetAt ( 
				pointSurroundingPosition );

			// zpracuje sousední pozici
			if ( rdwSurroundingPositionDistance < INFINITE_DISTANCE )
			{	// jedná se o ji navštívenou pozici
        // vyresi se pri zpetnem pruchodu vysledkami vlny
/*				// zjistí jedná-li se o kratší cestu
				if ( ( dwPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) < 
					rdwSurroundingPositionDistance )
				{	// jedná se o kratší cestu
					// smae pozici z prioritní fronty
					VERIFY ( m_cPositionPriorityQueue.RemoveElement ( 
						pointSurroundingPosition ) );
					// aktualizuje vzdálenost MapCellu
					rdwSurroundingPositionDistance = dwPositionDistance + 
						( ( nIndex & 0x01 ) ? 10 : 14 );
					// pøidá pozici do prioritní fronty
					m_cPositionPriorityQueue.Add ( rdwSurroundingPositionDistance, 
						pointSurroundingPosition );
				}*/
				// nejedná se o kratší cestu
			}
			else if ( rdwSurroundingPositionDistance == INFINITE_DISTANCE )
			{	// jedná se o dosud nenavštívenou pozici
				// zjistí, jedná-li se o povolenı MapCell
				if ( GetMapCellAt ( pointSurroundingPosition ) >= cMapCellWidth )
				{	// jedná se o povolenı MapCell
					// aktualizuje vzdálenost MapCellu
					rdwSurroundingPositionDistance = dwPositionDistance + 
						( ( nIndex & 0x01 ) ? 10 : 14 );

          // Zjistí ohodnocení bodu
          dwHelpValuation = FindLocalPathValuation ( FindLocalPathDistance ( pointStart, pointSurroundingPosition),
            FindLocalPathDistance ( pointSurroundingPosition, pointEnd ) );

// *****************************************
// *****************************************
// *****************************************
// *******************ZIKI - docasne
// *****************************************
// *****************************************
// *****************************************
//          if ( dwHelpValuation > FIND_LOCAL_PATH_MAX_VALUATION )
//            continue;

					// pøidá pozici do prioritní fronty
					m_cPositionPriorityQueue.Add ( pointSurroundingPosition, dwHelpValuation );
				}
				else
				{	// jedná se o zakázanı MapCell
					// aktualizuje vzdálenost zakázaného MapCellu
					rdwSurroundingPositionDistance = DWORD_MAX;
				}
			}
			else if ( rdwSurroundingPositionDistance != DWORD_MAX )
			{	// jedná se o koncovou pozici
				ASSERT ( rdwSurroundingPositionDistance == DWORD_MAX - 1 );

				// aktualizuje vzdálenost koncové pozice
				rdwSurroundingPositionDistance = dwPositionDistance + 
					( ( nIndex & 0x01 ) ? 10 : 14 );

				// znièí frontu pozic na mapì
				m_cPositionPriorityQueue.Clear ();

				// ukonèí zpracovávání okolních MapCellù
				break;
			}
			// sousední pozice je zpracována
		}
		// sousední pozice jsou zpracovány
	}

	// prioritní fronta pozic na mapì byla zpracována
	ASSERT ( m_cMapCellDistance.GetAt ( pointEnd ) < INFINITE_DISTANCE );

	// znièí prioritní frontu pozic na mapì
	m_cPositionPriorityQueue.Clear ();

// pøevede cestu na kontrolní body

	// poslední zpracovanı bod cesty
	CPointDW pointLastPosition = pointEnd;
	// vzdálenost posledního zpracovaného bodu cesty
	DWORD dwLastPositionDistance = m_cMapCellDistance.GetAt ( pointEnd );

	// pøevede cestu na kontrolní body
	while ( dwLastPositionDistance != 0 )
	{
		// další bod cesty
		CPointDW pointNextPosition;
		// vzdálenost dalšího bodu cesty
		DWORD dwNextPositionDistance;

    // pomocne promenne pro nalezeni minima (nejvetsiho spadu)
    dwNextPositionDistance = INFINITE_DISTANCE;
    DWORD dwTryPositionDistance;
    CPointDW pointTryPosition;

    // najde další bod cesty
		for ( int nIndex = 8; nIndex-- > 0; )
		{
			// zjistí sousední pozici na mapì
			pointTryPosition = pointLastPosition + m_aSurroundingPositionOffset[nIndex];

			// zjistí, jedná-li se o pozici na mapì
			if ( ( pointTryPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
				( pointTryPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
			{	// nejedná se o pozici na mapì
				// ukonèí zpracovávání sousední pozice
				continue;
			}

			// vzdálenost sousední pozice na mapì
			dwTryPositionDistance = m_cMapCellDistance.GetAt ( pointTryPosition );

			// zjistí, jedná-li se o prozkoumanou pozici na mapì
			if ( dwTryPositionDistance >= INFINITE_DISTANCE )
			{	// jedná se o neprozkoumanou pozici na mapì
				ASSERT ( ( dwTryPositionDistance == INFINITE_DISTANCE ) || 
					( dwTryPositionDistance == DWORD_MAX ) );
				// ukonèí zpracovávání sousední pozice
				continue;
			}

/*			ASSERT ( ( dwNextPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) >= 
				dwLastPositionDistance );

			// zjistí, jedná-li se o následující bod cesty
			if ( ( dwNextPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) == 
				dwLastPositionDistance )
			{	// jedná se o následující bod cesty
				// nechá zpracovat bod cesty
				break;
			}*/

      if ( ( dwTryPositionDistance < dwLastPositionDistance ) && ( dwTryPositionDistance < dwNextPositionDistance ) )
      {
        dwNextPositionDistance = dwTryPositionDistance;
        pointNextPosition = pointTryPosition;
        continue;
      }
		}
		// byl nalezen další bod cesty
		ASSERT ( dwNextPositionDistance < INFINITE_DISTANCE );

	// zjistí, existuje-li úseèka mezi nelezenım dalším bodem a koncem cesty

		// vzdálenost koncù úseèky v jednotlivıch osách
		CSizeDW sizeDistance = pointEnd.GetRectSize ( pointNextPosition );

		// zjistí, je-li úseèka pøíliš dlouhá
		if ( ( sizeDistance.cx * sizeDistance.cx + sizeDistance.cy * sizeDistance.cy ) >= 
			MAX_CHECK_POINT_DISTANCE * MAX_CHECK_POINT_DISTANCE )
		{	// úseèka je pøíliš dlouhá
			// pøidá kontrolní bod koncového bodu cesty
			CSCheckPointPositionList::Insert ( posCheckPointPosition, pointEnd );
			// aktualizuje konec cesty
			pointEnd = pointLastPosition;
			// aktualizuje vzdálenost bodù úseèky v jednotlivıch osách
			sizeDistance = pointEnd.GetRectSize ( pointNextPosition );
		}

		// zjistí, ve kterém rozmìru je vzdálenost koncù úseèky vìtší
		if ( sizeDistance.cy < sizeDistance.cx )
		{	// vzdálenost koncù úseèky je vìtší v ose X
			// pozice bodu úseèky
			CPointDW pointPosition;

			// pozice bodu úseèky v ose X
			pointPosition.x = pointNextPosition.x;
			// pøírùstek v ose X
			DWORD dwXDifference = ( pointNextPosition.x < pointEnd.x ) ? 1 : (DWORD)-1;

			// pøesná pozice bodu úseèky v ose Y
			double dbYPosition = (double)pointNextPosition.y + 0.5;
			// pøesnı pøírùstek v ose Y
			double dbYDifference = ( (double)pointEnd.y - (double)pointNextPosition.y ) / 
				(double)sizeDistance.cx;

			ASSERT ( pointPosition.x != pointEnd.x );

			// "nulty" krok iterace, osa X je implicitne posunuta o "pul mapcellu",
			// na ose y pridame polovinu dbYDifference k prvni hranici mapcellu
			pointPosition.x += dwXDifference;
			dbYPosition += dbYDifference / 2;
			// pozice bodu úseèky v ose Y
			pointPosition.y = (DWORD)dbYPosition;

			// zkontroluje MapCelly bodù úseèky
			while ( pointPosition.x != pointEnd.x )
			{
				// zkontroluje první okraj MapCellu úseèky
				{
					// vzdálenost bodu úseèky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjistí, je-li MapCell bodu úseèky povolenı
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu úseèky je zakázánı nebo dosud nenavštívenı
						// zjistí, jedná-li se o nenavštívenı MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedná se o nenavštívenı MapCell
							// zjistí, jedná-li se o povolenı MapCell
							if ( GetMapCellAt ( pointPosition ) >= cMapCellWidth )
							{	// jedná se o povolenı MapCell
								// aktalizuje informaci o povoleném MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedná se o zakázanı MapCell
								// aktalizuje informaci o zakázaném MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell byl navštíven
						}
						// jedná se o navštívenı MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjistí, je-li MapCell zakázanı
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zakázanı
							// pøidá kontrolní bod koncového bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, pointEnd );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukonèí kontrolu MapCellù bodù úseèky
							break;
						}
						// MapCell je povolenı
					}
					// MapCell bodu úseèky je povolenı
				}
				// první okraj MapCellu úseèky je povolenı

				// posune pozici na druhı okraj MapCellu
				dbYPosition += dbYDifference;
				// pozice bodu úseèky v ose Y
				pointPosition.y = (DWORD)dbYPosition;

				// zkontroluje druhı okraj MapCellu úseèky
				{
					// aktualizuje vzdálenost bodu úseèky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjistí, je-li MapCell bodu úseèky povolenı
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu úseèky je zakázánı nebo dosud nenavštívenı
						// zjistí, jedná-li se o nenavštívenı MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedná se o nenavštívenı MapCell
							// zjistí, jedná-li se o povolenı MapCell
							if ( GetMapCellAt ( pointPosition ) >= cMapCellWidth )
							{	// jedná se o povolenı MapCell
								// aktalizuje informaci o povoleném MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedná se o zakázanı MapCell
								// aktalizuje informaci o zakázaném MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell byl navštíven
						}
						// jedná se o navštívenı MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjistí, je-li MapCell zakázanı
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zakázanı
							// pøidá kontrolní bod koncového bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, pointEnd );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukonèí kontrolu MapCellù bodù úseèky
							break;
						}
						// MapCell je povolenı
					}
					// MapCell bodu úseèky je povolenı
				}
				// druhı okraj MapCellu úseèky je povolenı

				// posune se na další bod úseèky
				pointPosition.x += dwXDifference;
			}
			// MapCelly bodù úseèky jsou zkontrolovány
		}
		else if ( sizeDistance.cy > sizeDistance.cx )
		{	// vzdálenost koncù úseèky je vìtší v ose Y
			// pozice bodu úseèky
			CPointDW pointPosition;

			// pozice bodu úseèky v ose Y
			pointPosition.y = pointNextPosition.y;
			// pøírùstek v ose Y
			DWORD dwYDifference = ( pointNextPosition.y < pointEnd.y ) ? 1 : (DWORD)-1;

			// pøesná pozice v ose X
			double dbXPosition = (double)pointNextPosition.x + 0.5;
			// pøesnı pøírùstek v ose X
			double dbXDifference = ( (double)pointEnd.x - (double)pointNextPosition.x ) / 
				(double)sizeDistance.cy;

			ASSERT ( pointPosition.y != pointEnd.y );

			// "nulty" krok iterace, osa Y je implicitne posunuta o "pul mapcellu",
			// na ose X pridame polovinu dbXDifference k prvni hranici mapcellu
			pointPosition.y += dwYDifference;
			dbXPosition += dbXDifference / 2;
			// pozice bodu úseèky v ose X
			pointPosition.x = (DWORD)dbXPosition;

			// zkontroluje MapCelly bodù úseèky
			while ( pointPosition.y != pointEnd.y )
			{
				// zkontroluje první okraj MapCellu úseèky
				{
					// vzdálenost bodu úseèky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjistí, je-li MapCell bodu úseèky povolenı
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu úseèky je zakázánı nebo dosud nenavštívenı
						// zjistí, jedná-li se o nenavštívenı MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedná se o nenavštívenı MapCell
							// zjistí, jedná-li se o povolenı MapCell
							if ( GetMapCellAt ( pointPosition ) >= cMapCellWidth )
							{	// jedná se o povolenı MapCell
								// aktalizuje informaci o povoleném MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedná se o zakázanı MapCell
								// aktalizuje informaci o zakázaném MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell je navštívenı
						}
						// jedná se o navštívenı MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjistí, je-li MapCell zakázanı
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zakázanı
							// pøidá kontrolní bod koncového bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, pointEnd );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukonèí kontrolu MapCellù bodù úseèky
							break;
						}
						// MapCell je povolenı
					}
					// MapCell bodu úseèky je povolenı
				}
				// první okraj MapCellu úseèky je povolenı

				// posune pozici na druhı okraj MapCellu
				dbXPosition += dbXDifference;
				// pozice bodu úseèky v ose X
				pointPosition.x = (DWORD)dbXPosition;

				// zkontroluje druhı okraj MapCellu úseèky
				{
					// aktualizuje vzdálenost bodu úseèky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjistí, je-li MapCell bodu úseèky povolenı
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu úseèky je zakázánı nebo dosud nenavštívenı
						// zjistí, jedná-li se o nenavštívenı MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedná se o nenavštívenı MapCell
							// zjistí, jedná-li se o povolenı MapCell
							if ( GetMapCellAt ( pointPosition ) >= cMapCellWidth )
							{	// jedná se o povolenı MapCell
								// aktalizuje informaci o povoleném MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedná se o zakázanı MapCell
								// aktalizuje informaci o zakázaném MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell je navštívenı
						}
						// jedná se o navštívenı MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjistí, je-li MapCell zakázanı
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zakázanı
							// pøidá kontrolní bod koncového bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, pointEnd );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukonèí kontrolu MapCellù bodù úseèky
							break;
						}
						// MapCell je povolenı
					}
					// MapCell bodu úseèky je povolenı
				}
				// druhı okraj MapCellu úseèky je povolenı

				// posune se na další bod úseèky
				pointPosition.y += dwYDifference;
			}
			// MapCelly bodù úseèky jsou zkontrolovány
		}
		else
		{	// vzdálenosti koncù úseèky jsou stejné v ose X i Y
			// pozice bodu úseèky
			CPointDW pointPosition = pointNextPosition;
			// pøírùstek v osách X a Y
			CPointDW pointDifference ( ( pointNextPosition.x < pointEnd.x ) ? 1 : (DWORD)-1, 
				( pointNextPosition.y < pointEnd.y ) ? 1 : (DWORD)-1 );

			ASSERT ( pointNextPosition != pointEnd );

			// posune se na další MapCell
			pointPosition += pointDifference;

			// zkontroluje MapCelly bodù úseèky
			while ( pointPosition != pointEnd )
			{
				// vzdálenost bodu úseèky
				DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

				// zjistí, je-li MapCell bodu úseèky povolenı
				if ( rdwPositionDistance >= INFINITE_DISTANCE )
				{	// MapCell bodu úseèky je zakázánı nebo dosud nenavštívenı
					// zjistí, jedná-li se o nenavštívenı MapCell
					if ( rdwPositionDistance == INFINITE_DISTANCE )
					{	// jedná se o nenavštívenı MapCell
						// zjistí, jedná-li se o povolenı MapCell
						if ( GetMapCellAt ( pointPosition ) >= cMapCellWidth )
						{	// jedná se o povolenı MapCell
							// aktalizuje informaci o povoleném MapCellu
							rdwPositionDistance = INFINITE_DISTANCE - 1;
						}
						else
						{	// jedná se o zakázanı MapCell
							// aktalizuje informaci o zakázaném MapCellu
							rdwPositionDistance = DWORD_MAX;
						}
						// MapCell je navštívenı
					}
					// jedná se o navštívenı MapCell
					ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
						( rdwPositionDistance == DWORD_MAX ) );

					// zjistí, je-li MapCell zakázanı
					if ( rdwPositionDistance == DWORD_MAX )
					{	// MapCell je zakázanı
						// pøidá kontrolní bod koncového bodu cesty
						CSCheckPointPositionList::Insert ( posCheckPointPosition, pointEnd );
						// aktualizuje konec cesty
						pointEnd = pointLastPosition;
						// ukonèí kontrolu MapCellù bodù úseèky
						break;
					}
					// MapCell je povolenı
				}
				// MapCell bodu úseèky je povolenı

				// posune se na další MapCell
				pointPosition += pointDifference;
			}
			// MapCelly bodù úseèky jsou zkontrolovány
		}
		// úseèka je zpracována

		// aktualizuje poslední zpracovanı bod cesty
		pointLastPosition = pointNextPosition;
		// aktualizuje vzdálenost posledního zpracovaného bodu cesty
		dwLastPositionDistance = dwNextPositionDistance;
	}
	// cesta byla pøevedena na kontrolní body

	// pøidá kontrolní bod koncového bodu cesty
	CSCheckPointPositionList::Insert ( posCheckPointPosition, pointEnd );

	// zruší oznaèení pozic na mapì
	m_cMapCellDistance.Clear ();
}

// *******************************************bFoundNearStart, CPointDW &
// vrátí index nejbliší brány mostu z pozice "pointStart" pro šíøku MapCellu 
//		"cMapCellWidth" (DWORD_MAX=neexistuje brána mostu, DWORD_MAX-1="pointEnd" je bliší)
DWORD CSFindPathGraph::GetNearestBridgeGateIndex ( CPointDW &pointStart, 
	signed char cMapCellWidth, CPointDW pointEnd, BOOL bFoundNearStart ) 
{
	ASSERT ( ( cMapCellWidth > 0 ) && ( ( cMapCellWidth % 2 ) == 0 ) );

	// fronta pozic na mapì
	static CSelfPooledQueue<CPointDW> cPositionQueue ( 200 );
	ASSERT ( cPositionQueue.IsEmpty () );

	// pozice na mapì
	CPointDW pointPosition = pointStart;
	// pozice nalezené brány mostu
	CPointDW pointBridgeGatePosition ( DWORD_MAX, DWORD_MAX );
	// hodnota MapCellu poèáteèní pozice
	signed char cStartMapCell = GetMapCellAt ( pointStart );

// **********************************************
//	// zjistí, zaèíná-li se na správné šíøce MapCellu
//	if ( cStartMapCell < cMapCellWidth )
//	{	// zaèíná se na špatné šíøce MapCellu
//		// vrátí pøíznak nenalezení cesty
//		return DWORD_MAX;
//	}
// **********************************************

// **********************************************
	// zjistí, zaèíná-li se na správné šíøce MapCellu
	if ( cStartMapCell < cMapCellWidth )
	{	// zaèíná se na špatné šíøce MapCellu
// **********************************************
		if ( bFoundNearStart )
		{
			// fronta pozic
			static CSelfPooledQueue<CPointDW> cPositionQueue ( 50 );

			// aktualizuje vzdálenost zakázaného MapCellu
			m_cMapCellDistance.GetAt ( pointStart ) = DWORD_MAX;

			CPointDW pointEndNear ( NO_MAP_POSITION, NO_MAP_POSITION );

			DWORD dwCounter = MAX_CHECK_POINT_DISTANCE * MAX_CHECK_POINT_DISTANCE * 3;

			cPositionQueue.Add ( pointStart );

			CPointDW pointPosition;
			while ( cPositionQueue.RemoveFirst ( pointPosition ) )
			{
				if ( --dwCounter == 0 )
				{
					cPositionQueue.RemoveAll ();
					break;
				}

				// zpracuje sousední pozice na mapì
				for ( int nIndex = 8; nIndex-- > 0; )
				{
					// sousední pozice na mapì
					CPointDW pointSurroundingPosition = pointPosition + 
						m_aSurroundingPositionOffset[nIndex];

					// zjistí, jedná-li se o pozici na mapì
					if ( ( pointSurroundingPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
						( pointSurroundingPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
					{	// nejedná se o pozici na mapì
						// ukonèí zpracovávání sousední pozice
						continue;
					}

					// vzdálenost sousední pozice
					DWORD &rdwSurroundingPositionDistance = 
						m_cMapCellDistance.GetAt ( pointSurroundingPosition );

					// zpracuje sousední pozici
					if ( rdwSurroundingPositionDistance == INFINITE_DISTANCE )
					{	// jedná se o dosud nenavštívenou pozici
						// zjistí, jedná-li se o povolenı MapCell
						if ( GetMapCellAt ( pointSurroundingPosition ) >= cMapCellWidth )
						{	// jedná se o povolenı MapCell
							// aktualizuje cílovou pozici
							pointEndNear = pointSurroundingPosition;
							// vyprázdní frontu
							cPositionQueue.RemoveAll ();
							break;
						}
						else
						{	// jedná se o zakázanı MapCell
							// aktualizuje vzdálenost zakázaného MapCellu
							rdwSurroundingPositionDistance = DWORD_MAX;
						}
						cPositionQueue.Add ( pointSurroundingPosition );
					}
					// jedná se o zakázanı MapCell
					ASSERT ( rdwSurroundingPositionDistance == DWORD_MAX );
				}
				// sousední pozice na mapì byla zpracována
			}
			// fronta byla zpracována

			if ( pointEndNear == CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ) )
			{	// nebyla nalezena cílová pozice
				// zruší oznaèení pozic na mapì
				m_cMapCellDistance.Clear ();

				return DWORD_MAX;
			}
			pointStart = pointEndNear;
			// aktualizuje hodnotu MapCellu poèáteèní pozice
			cStartMapCell = GetMapCellAt ( pointStart );
		}
		else
		{
			// zruší oznaèení pozic na mapì
			m_cMapCellDistance.Clear ();

			// vrátí pøíznak nenalezení cesty
			return DWORD_MAX;
		}
	}

	// oznaèí pozici na mapì
	m_cMapCellDistance.GetAt ( pointStart ) = 0;

	// zjistí, jedná-li se o bránu mostu
	if ( !IsBridgeGateMapCell ( cStartMapCell ) )
	{	// nejedná se o bránu mostu
		// pøidá pozici na mapì do fronty pozic
		cPositionQueue.Add ( pointStart );

		// najde nejbliší bránu mostu
		while ( cPositionQueue.RemoveFirst ( pointPosition ) )
		{
			ASSERT ( m_cMapCellDistance.GetAt ( pointPosition ) == 0 );
			ASSERT ( GetMapCellAt ( pointPosition ) >= cMapCellWidth );

			// projede okolní pozice na mapì
			for ( int nIndex = 8; nIndex-- > 0; )
			{
				// okolní pozice na mapì
				CPointDW pointSurroundingPosition = pointPosition + 
					m_aSurroundingPositionOffset[nIndex];

				// zjistí, jedná-li se o pozici na mapì
				if ( ( pointSurroundingPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
					( pointSurroundingPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
				{	// nejedná se o pozici na mapì
					// ukonèí zpracovávání okolní pozice
					continue;
				}

				// vzdálenost sousední pozice
				DWORD &rdwSurroundingPosition = m_cMapCellDistance.GetAt ( 
					pointSurroundingPosition );

				// zjistí, jedná-li se o prozkoumanou pozici na mapì
				if ( rdwSurroundingPosition == 0 )
				{	// jedná se o prozkoumanou pozici na mapì
					// ukonèí zpracovávání okolní pozice
					continue;
				}

				// hodnota MapCellu na okolní pozici na mapì
				signed char cSurroundingMapCell = GetMapCellAt ( pointSurroundingPosition );

				// zjistí, jedná-li se o pøípustnou pozici na mapì
				if ( cSurroundingMapCell >= cMapCellWidth )
				{	// jedná se o pøípustnou pozici
					// zjistí, jedná-li se o bránu mostu
					if ( IsBridgeGateMapCell ( cSurroundingMapCell ) )
					{	// jedná se o bránu mostu
						// uschová si pozici brány mostu
						pointBridgeGatePosition = pointSurroundingPosition;
						// znièí frontu pozic na mapì
						cPositionQueue.RemoveAll ();
						// ukonèí hledání nejbliší brány mostu
						break;
					}
					// nejedná se o bránu mostu

					// pøidá pozici na mapì do fronty
					cPositionQueue.Add ( pointSurroundingPosition );
					// oznaèí pozici na mapì
					rdwSurroundingPosition = 0;
				}
				// nejedná se o pøípustnou pozici na mapì
			}
			// zpracoval okolní pozice na mapì
		}
		// nenašel bránu mostu
	}
	else
	{	// jedná se o bránu mostu
		// aktualizuje pozici nalezené brány
		pointBridgeGatePosition = pointStart;
	}

	// pøíznak blízké koncové pozice
	BOOL bNearEndPosition = ( m_cMapCellDistance.GetValueAt ( pointEnd ) == 0 );

	// zruší oznaèení pozic na mapì
	m_cMapCellDistance.Clear ();

	// zjistí, je-li koncová pozice blízko poèáteèní pozici
	if ( bNearEndPosition )
	{	// koncová pozice je blízko poèáteèní pozici
		// vrátí pøíznak bliší koncové pozice
		return DWORD_MAX - 1;
	}

	// zjistí, byla-li nalezena brána mostu
	if ( pointBridgeGatePosition.x != DWORD_MAX )
	{	// byla nalezena brána mostu
		ASSERT ( pointBridgeGatePosition.x != DWORD_MAX );

		// najde index brány mostu
		for ( struct SBridgeGate *pBridgeGate = m_aBridgeGateTable; ; pBridgeGate++ )
		{
			// zjistí, jedná-li se o hledanou bránu mostu
			if ( ( pBridgeGate->dwX == pointBridgeGatePosition.x ) && 
				( pBridgeGate->dwY == pointBridgeGatePosition.y ) )
			{	// jedná se o hledanou bránu mostu
				// vrátí index brány mostu
				return ((DWORD)pBridgeGate - (DWORD)m_aBridgeGateTable) / sizeof ( SBridgeGate );
			}
		}
		// nebyl nalezen index brány
	}
	else
	{	// nebyla nalezena brána mostu
		ASSERT ( pointBridgeGatePosition.y == DWORD_MAX );
	}

	// vrátí pøíznak nenalezení brány mostu
	return DWORD_MAX;
}

// *****************************, CPointDW &pointSecondBridgeGate, CPointDW &pointPreLastBridgeGate
// najde nejkratší cestu od brány mostu "dwStartBridgeGateIndex" k bránì mostu 
//		"dwEndBridgeGateIndex" šíøky "dwWidth", vyplní seznam kontrolních bodù cesty 
//		"pCheckPointPositionList", vrací poèet bran mostù cesty
DWORD CSFindPathGraph::FindBridgeGatePath ( DWORD dwStartBridgeGateIndex, 
	DWORD dwEndBridgeGateIndex, DWORD dwWidth, 
	CSCheckPointPositionList *pCheckPointPositionList, CPointDW &pointSecondBridgeGate, 
	CPointDW &pointPreLastBridgeGate ) 
{
	ASSERT ( dwStartBridgeGateIndex != dwEndBridgeGateIndex );

	// vynuluje vzdálenosti bran mostù
	ClearBridgeGatesDistances ();

	// inicializuje vzdálenost vıchozí brány
	m_aBridgeGateDistance[dwStartBridgeGateIndex].dwDistance = 0;

	ASSERT ( m_cBridgeGateList.IsEmpty () );

	// pøidá do seznamu bran mostù index prvního mostu
	m_cBridgeGateList.Add ( dwStartBridgeGateIndex );

	// index nejbliší brány mostu
	DWORD dwBridgeGateIndex;

	// najde nejkratší cestu v grafu
	while ( m_cBridgeGateList.RemoveFirst ( dwBridgeGateIndex ) )
	{
		// zjistí ukazatel na bránu mostu
		struct SBridgeGate *pBridgeGate = m_aBridgeGateTable + dwBridgeGateIndex;

		// zjistí, jedná-li se o cílovou bránu
		if ( dwBridgeGateIndex == dwEndBridgeGateIndex )
		{	// jedná se o cílovou bránu
			// znièí seznam bran mostù
			m_cBridgeGateList.RemoveAll ();

			// poèet bran mostù nalezené cesty
			DWORD dwBridgeGateCount = 1;

			// *****************************
			// inicializuje pozici druhé brány mostu
			pointSecondBridgeGate = CPointDW ( m_aBridgeGateTable[dwEndBridgeGateIndex].dwX, 
				m_aBridgeGateTable[dwEndBridgeGateIndex].dwY );
			// inicializuje pozici pøedposlední brány mostu
			pointPreLastBridgeGate = CPointDW ( 
				m_aBridgeGateTable[dwStartBridgeGateIndex].dwX, 
				m_aBridgeGateTable[dwStartBridgeGateIndex].dwY );

			// vytvoøí seznam kontrolních bodù cesty (od konce)
			while ( dwEndBridgeGateIndex != dwStartBridgeGateIndex )
			{
				// aktualizuje poèet bran mostù
				dwBridgeGateCount++;

				// ukazatel na bránu mostu
				struct SBridgeGate *pEndBridgeGate = pBridgeGate;
				// aktualizuje index pøechozí brány mostu
				dwBridgeGateIndex = 
					m_aBridgeGateDistance[dwEndBridgeGateIndex].dwPreviousBridgeGateIndex;
				// aktualizuje ukazatel na pøedchozí bránu mostu
				pBridgeGate = m_aBridgeGateTable + dwBridgeGateIndex;

				// *****************************
				// aktualizuje pozici druhé brány mostu
				pointSecondBridgeGate = CPointDW ( pEndBridgeGate->dwX, pEndBridgeGate->dwY );
				// zjistí, jedná-li se o pøedposlední bránu mostu
				if ( dwBridgeGateCount == 3 )
				{	// jedná se o pøedposlední bránu mostu
					// aktualizuje pozici pøedposlední brány mostu
					pointPreLastBridgeGate = CPointDW ( pEndBridgeGate->dwX, 
						pEndBridgeGate->dwY );
				}

				// pøidá kontrolní bod brány mostu
				pCheckPointPositionList->Add ( CPointDW ( pEndBridgeGate->dwX, 
					pEndBridgeGate->dwY ) );

				// pøidá kontrolní body hintu cesty
				for ( struct SArea *pArea = pEndBridgeGate->pBridgeArea; ; )
				{
					// zjistí, je-li oblast brány mostu spoleènou oblastí bran mostù
					if ( ( ( pArea == pBridgeGate->pBridgeArea ) || 
						( pArea == pBridgeGate->pArea ) ) && ( pArea->dwWidth >= dwWidth ) )
					{	// oblast brány mostu je spoleènou oblastí bran mostù
						// index první brány mostu
						DWORD dwFirstIndex = DWORD_MAX;
						// index druhé brány mostu
						DWORD dwSecondIndex;

						// zjistí ukazatel na brány mostù oblasti
						struct SBridgeGate **pAreaBridgeGate = 
							(struct SBridgeGate **)( pArea + 1 );

						// zjistí indexy bran mostù v oblasti
						for ( ; ; pAreaBridgeGate++ )
						{
							// zjistí, jedná-li se o hledanou bránu mostu
							if ( ( *pAreaBridgeGate == pBridgeGate ) || 
								( *pAreaBridgeGate == pEndBridgeGate ) )
							{	// jedná se o hledanou bránu mostu
								// zjistí, jedná-li se o druhou nalezenou bránu mostu
								if ( dwFirstIndex != DWORD_MAX )
								{	// jedná se o druhou nalezenou bránu mostu
									// aktualizuje index druhé brány mostu
									dwSecondIndex = pAreaBridgeGate - 
										(struct SBridgeGate **)( pArea + 1 ); 
									// ukonèí hledání bran mostù
									break;
								}
								// jedná se o první nalezenou bránu mostu
								dwFirstIndex = pAreaBridgeGate - 
									(struct SBridgeGate **)( pArea + 1 );
							}
							// nejdená se o hledanou bránu mostu
						}
						// indexy bran mostù v oblasti jsou zjištìny
						ASSERT ( dwSecondIndex < pArea->dwBridgeGateCount );

						// cesta mezi branami mostù
						struct SBridgeGatePath *pBridgeGatePath = 
							(struct SBridgeGatePath *)( (struct SBridgeGate **)( pArea + 1 ) + 
							pArea->dwBridgeGateCount ) + dwFirstIndex * ( 2 * 
							pArea->dwBridgeGateCount - dwFirstIndex - 1 ) / 2 + dwSecondIndex - 
							dwFirstIndex - 1;

						// zjistí, jedná-li se o pouitou cestu mezi branami
						if ( ( m_aBridgeGateDistance[dwBridgeGateIndex].dwDistance + 
							pBridgeGatePath->dwBridgeGateDistance ) == 
							m_aBridgeGateDistance[dwEndBridgeGateIndex].dwDistance )
						{	// jedná se o pouitou cestu
							// zjistí ukazatel na hint cesty
							struct SPathHint *pPathHint = pBridgeGatePath->pPathHint;

							// zjistí, je-li pøedchozí brána mostu první
							if ( *pAreaBridgeGate == pBridgeGate )
							{	// pøedchozí brána mostu je druhá
								// zjistí ukazatel na kontrolní body hintu cesty
								struct SPathHintCheckPoint *pPathHintCheckPoint = 
									(struct SPathHintCheckPoint *)( pPathHint + 1 );

								// pøidá kontrolní body hintu cesty
								for ( DWORD dwIndex = pPathHint->dwCheckPointCount; 
									dwIndex-- > 0; pPathHintCheckPoint++ )
								{
									// pøidá další kontrolní bod hintu cesty
									pCheckPointPositionList->Add ( CPointDW ( 
										pPathHintCheckPoint->dwX, pPathHintCheckPoint->dwY ) );
								}
								// kontrolní body hintu cesty jsou pøidány
							}
							else
							{	// pøedchozí brána mostu je první
								// zjistí ukazatel na kontrolní body hintu cesty
								struct SPathHintCheckPoint *pPathHintCheckPoint = 
									(struct SPathHintCheckPoint *)( pPathHint + 1 ) + 
									pPathHint->dwCheckPointCount - 1;

								// pøidá kontrolní body hintu cesty
								for ( DWORD dwIndex = pPathHint->dwCheckPointCount; 
									dwIndex-- > 0; pPathHintCheckPoint-- )
								{
									// pøidá další kontrolní bod hintu cesty
									pCheckPointPositionList->Add ( CPointDW ( 
										pPathHintCheckPoint->dwX, pPathHintCheckPoint->dwY ) );
								}
								// kontrolní body hintu cesty jsou pøidány
							}
							// kontrolní body hintu cesty jsou pøidány
							break;
						}
						// nejedná se o pouitou cestu mezi branami
					}
					// oblast brány mostu není poadovaná spoleèná oblast bran mostù

					ASSERT ( pArea != pEndBridgeGate->pArea );
					// aktualizuje ukazatel na oblast brány mostu
					pArea = pEndBridgeGate->pArea;
					ASSERT ( ( ( pArea == pBridgeGate->pBridgeArea ) || 
						( pArea == pBridgeGate->pArea ) ) && ( pArea->dwWidth >= dwWidth ) );
				}
				// kontrolní body hintu cesty byly pøidány

				// posune se na pøedchozí bránu mostu
				dwEndBridgeGateIndex = dwBridgeGateIndex;
			}
			// seznam kontrolních bodù cesty je vytvoøen

			// vrátí poèet bran mostù cesty
			return dwBridgeGateCount;
		}
		// nejedná se o cílovou bránu

		// zjistí, je-li šíøka oblasti dostateèná
		if ( pBridgeGate->pBridgeArea->dwWidth >= dwWidth )
		{	// šíøka oblasti je dostateèná
			// aktualizuje brány mostù pøes první oblast
			UpdateBridgeGateAreaDistances ( dwBridgeGateIndex, pBridgeGate->pBridgeArea );
		}
		// zjistí, jedná-li se o stejné oblasti
		if ( pBridgeGate->pBridgeArea != pBridgeGate->pArea )
		{	// nejedná se o stejné oblasti
			// zjistí, je-li šíøka oblasti dostateèná
			if ( pBridgeGate->pArea->dwWidth >= dwWidth )
			{	// šíøka oblasti je dostateèná
				// aktualizuje brány mostù pøes druhou oblast
				UpdateBridgeGateAreaDistances ( dwBridgeGateIndex, pBridgeGate->pArea );
			}
		}
		// jedná se o stejné oblasti
	}
	// neexistuje cesta v grafu

	// vrátí cestu délky 0
	return 0;
}

// vynuluje vzdálenosti bran mostù (pomocná metoda metody "FindBridgeGatePath")
void CSFindPathGraph::ClearBridgeGatesDistances () 
{
	// ukazatel na vzdálenost brány mostu
	struct SBridgeGateDistance *pBridgeGateDistance = m_aBridgeGateDistance;

	// vynuluje vzdálenosti bran mostù
	for ( DWORD dwIndex = m_dwMaxBridgeGateCount; dwIndex-- > 0; pBridgeGateDistance++ )
	{
		// vynuluje vzdálenost brány mostu
		pBridgeGateDistance->dwDistance = INFINITE_DISTANCE;
		pBridgeGateDistance->dwPreviousBridgeGateIndex = DWORD_MAX;
	}
}

// aktualizuje vzdálenosti bran mostù oblasti "pArea" od brány mostu "dwBridgeGateIndex" 
//		(pomocná metoda metody "FindBridgeGatePath")
void CSFindPathGraph::UpdateBridgeGateAreaDistances ( DWORD dwBridgeGateIndex, 
	struct SArea *pArea ) 
{
	// vzdálenost brány mostu
	DWORD dwBridgeGateDistance = m_aBridgeGateDistance[dwBridgeGateIndex].dwDistance;
	ASSERT ( dwBridgeGateDistance < INFINITE_DISTANCE );
	// ukazatel na bránu mostu
	struct SBridgeGate *pBridgeGate = m_aBridgeGateTable + dwBridgeGateIndex;

	// zjistí ukazatel na brány mostù oblasti
	struct SBridgeGate **pAreaBridgeGate = (struct SBridgeGate **)( pArea + 1 );

	// zjistí index brány mostu v oblasti
	for ( ; ; pAreaBridgeGate++ )
	{
		// zjistí, jedná-li se o hledanou bránu mostu
		if ( *pAreaBridgeGate == pBridgeGate )
		{	// jedná se o hledanou bránu mostu
			// ukonèí hledání indexu brány mostu v oblasti
			break;
		}
	}

	// zjistí index nalezené brány mostu v branách mostù oblasti
	DWORD dwIndex = pAreaBridgeGate - (struct SBridgeGate **)( pArea + 1 );
	ASSERT ( dwIndex < pArea->dwBridgeGateCount );

	// aktualizuje ukazatel na první bránu mostu oblasti
	pAreaBridgeGate = (struct SBridgeGate **)( pArea + 1 );
	// ukazatel na cesty mezi branami mostù oblasti
	struct SBridgeGatePath *pBridgeGatePath = 
		(struct SBridgeGatePath *)( pAreaBridgeGate + pArea->dwBridgeGateCount );

	// zpracuje poèáteèní brány mostù oblasti
	for ( DWORD dwFirstIndex = 0; dwFirstIndex < pArea->dwBridgeGateCount; dwFirstIndex++ )
	{
		// zpracuje koncové brány mostù oblasti
		for ( DWORD dwSecondIndex = dwFirstIndex + 1; 
			dwSecondIndex < pArea->dwBridgeGateCount; dwSecondIndex++, pBridgeGatePath++ )
		{
			// zjistí, jedná-li se o pøechod s aktuální bránou mostu
			if ( ( dwFirstIndex == dwIndex ) || ( dwSecondIndex == dwIndex ) )
			{	// jedná se o pøechod s potøebnou branou mostu
				// ukazatel na jinou bránu mostu
				struct SBridgeGate *pOtherBridgeGate = pAreaBridgeGate[( dwFirstIndex == 
					dwIndex ) ? dwSecondIndex : dwFirstIndex];
				// index jiné brány mostu
				DWORD dwOtherBridgeGateIndex = pOtherBridgeGate - m_aBridgeGateTable;

				// zjistí, je-li pøechod zkrácením cesty
				if ( ( dwBridgeGateDistance + pBridgeGatePath->dwBridgeGateDistance ) < 
					m_aBridgeGateDistance[dwOtherBridgeGateIndex].dwDistance )
				{	// pøechod je zkrácením cesty
					// aktualizuje délku cesty do brány mostu
					DWORD dwOtherBridgeGateDistance = 
						m_aBridgeGateDistance[dwOtherBridgeGateIndex].dwDistance = 
						dwBridgeGateDistance + pBridgeGatePath->dwBridgeGateDistance;
					m_aBridgeGateDistance[dwOtherBridgeGateIndex].dwPreviousBridgeGateIndex = 
						dwBridgeGateIndex;
					// najde bránu mostu ze seznamu
					POSITION posBridgeGate = m_cBridgeGateList.Find ( dwOtherBridgeGateIndex );
					// zjistí, byla-li brána mostu v seznamu
					if ( posBridgeGate != NULL )
					{	// brána mostu je v seznamu
						// vyjme bránu mostu ze seznamu
						m_cBridgeGateList.RemovePosition ( posBridgeGate );
					}

				// pøidá bránu mostu do seznamu na správné místo

					// pozice brány mostu v seznamu bran mostù
					posBridgeGate = m_cBridgeGateList.GetHeadPosition ();
					// pozice pøedchozí brány mostu v seznamu bran mostù
					POSITION posLastBridgeGate = posBridgeGate;
					// index další brány mostu
					DWORD dwNextBridgeGateIndex;

					// projede seznam bran mostù
					while ( m_cBridgeGateList.GetNext ( posBridgeGate, 
						dwNextBridgeGateIndex ) )
					{
						// zjistí, má-li se pøidávaná brána mostu umístit
						if ( m_aBridgeGateDistance[dwNextBridgeGateIndex].dwDistance >= 
							dwOtherBridgeGateDistance )
						{	// pøidávaná brána mostu se má umístit
							// aktualizuje pozici umístìní pøidávané brány mostu
							posBridgeGate = posLastBridgeGate;
							// nechá umístit pøidávanou bránu mostu do seznamu bran mostù
							break;
						}
						// aktualizuje pozici poslední brány mostu
						posLastBridgeGate = posBridgeGate;
					}
					// umístí pøidávanou bránu mostu do seznamu bran mostù
					m_cBridgeGateList.Insert ( posLastBridgeGate, dwOtherBridgeGateIndex );
				}
				// pøechod není zkrácením cesty
			}
			// nejedná se o pøechod s aktuální bránou mostu
		}
		// zpracoval koncové brány mostù oblasti
	}
	// zpracoval poèáteèní brány mostù oblasti
}

// vrátí hodnotu MapCellu na pozici "pointMapCellPosition"
signed char CSFindPathGraph::GetMapCellAt ( CPointDW pointMapCellPosition ) 
{
	ASSERT ( ( pointMapCellPosition.x < g_cMap.GetMapSizeMapCell ().cx ) && 
		( pointMapCellPosition.y < g_cMap.GetMapSizeMapCell ().cy ) );

	// vrátí hodnotu MapCellu na pozici "pointMapCellPosition"
	return m_pMap[pointMapCellPosition.x + pointMapCellPosition.y * 
		g_cMap.GetMapSizeMapCell ().cx];
}
