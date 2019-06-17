/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da grafu pro hled�n� cesty
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
// Grafy pro hled�n� cesty
//////////////////////////////////////////////////////////////////////

// po�et graf� pro hled�n� cesty
DWORD CSFindPathGraph::m_dwFindPathGraphCount = DWORD_MAX;
// ukazatel na pole graf� pro hled�n� cesty
CSFindPathGraph *CSFindPathGraph::m_aFindPathGraph = NULL;
// pr�zdn� hint cesty
struct CSFindPathGraph::SPathHint CSFindPathGraph::m_sEmptyPathHint ( 0 );

//////////////////////////////////////////////////////////////////////
// Data pro hled�n� cesty
//////////////////////////////////////////////////////////////////////

// pole vzd�lenost� MapCell�
CArray2DOnDemand<DWORD> CSFindPathGraph::m_cMapCellDistance ( 10 );
// prioritn� fronta pozic na map�
CSmartPriorityQueue<CPointDW> CSFindPathGraph::m_cPositionPriorityQueue ( FIND_LOCAL_PATH_MAX_VALUATION + 1, FIND_LOCAL_PATH_MAX_VALUATION + 1);
// pole offset� pozic okoln�ch pozic na map�
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
// Data pro hled�n� cesty mezi branami
//////////////////////////////////////////////////////////////////////

// po�et bran most� nejv�t��ho grafu pro hled�n� cesty
DWORD CSFindPathGraph::m_dwMaxBridgeGateCount = 0;
// pole vzd�lenost� bran most�
struct CSFindPathGraph::SBridgeGateDistance *CSFindPathGraph::m_aBridgeGateDistance = 
	NULL;
// seznam bran most�
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
// Inicializace a zni�en� dat objektu
//////////////////////////////////////////////////////////////////////

// vytvo�� graf pro hled�n� cesty s ID "dwID" z archivu "cArchive"
void CSFindPathGraph::Create ( DWORD dwID, CDataArchive cArchive ) 
{
	ASSERT ( m_dwFindPathGraphID == 0 );

	ASSERT ( m_aAreaTable == NULL );
	ASSERT ( m_aBridgeGateTable == NULL );
	ASSERT ( m_aPathHintTable == NULL );

	ASSERT ( m_pMap == NULL );

	ASSERT ( m_aBridgeGateDistance == NULL );

	// uschov� si ID grafu pro hled�n� cesty
	m_dwFindPathGraphID = dwID;

// na�te hlavi�kov� soubor grafu pro hled�n� cesty

	// velikost tabulky oblast�
	DWORD dwAreaTableSize;
	// velikost tabulky bran most�
	DWORD dwBridgeGateTableSize;
	// velikost tabulky hint� cest
	DWORD dwPathHintTableSize;
	// po�et bran most�
	DWORD dwBridgeGateCount;

	// otev�e hlavi�kov� soubor grafu pro hled�n� cesty
	CArchiveFile cHeaderFile = cArchive.CreateFile ( FIND_PATH_GRAPH_HEADER_FILE_NAME, 
		CFile::modeRead | CFile::shareDenyWrite );

	// na�te hlavi�ku grafu pro hled�n� cesty
	{
		SMFindPathGraphHeader sFindPathGraphHeader;

		// na�te hlavi�ku grafu pro hled�n� cesty
		LOAD_ASSERT ( cHeaderFile.Read ( &sFindPathGraphHeader, sizeof ( sFindPathGraphHeader ) ) == sizeof ( sFindPathGraphHeader ) );

		// zjist� velikost tabulky oblast�
		dwAreaTableSize = sFindPathGraphHeader.m_dwAreaTableSize;
		LOAD_ASSERT ( ( dwAreaTableSize <= 4 * dwAreaTableSize ) && 
			( dwAreaTableSize <= 2 * dwAreaTableSize ) );
		// zjist� velikost tabulky bran most�
		dwBridgeGateTableSize = sFindPathGraphHeader.m_dwBridgeGateTableSize;
		LOAD_ASSERT ( dwBridgeGateTableSize % 4 == 0 );
		LOAD_ASSERT ( ( dwBridgeGateTableSize <= 4 * dwBridgeGateTableSize ) && 
			( dwBridgeGateTableSize <= 2 * dwBridgeGateTableSize ) );
		dwBridgeGateCount = dwBridgeGateTableSize / 4;
		// zjist� velikost tabulky hint� cest
		dwPathHintTableSize = sFindPathGraphHeader.m_dwPathHintTableSize;
		LOAD_ASSERT ( ( dwPathHintTableSize <= 4 * dwPathHintTableSize ) && 
			( dwPathHintTableSize <= 2 * dwPathHintTableSize ) );

		// zjist�, je-li po�et bran most� v�t��
		if ( dwBridgeGateCount > m_dwMaxBridgeGateCount )
		{	// po�et bran most� je v�t��
			// aktualizuje po�et bran most�
			m_dwMaxBridgeGateCount = dwBridgeGateCount;
		}
	}

	// na�te tabulku oblast�
	if ( dwAreaTableSize != 0 )
	{	// tabulka oblast� nen� pr�zdn�
		// alokuje tabulku oblast�
		m_aAreaTable = new DWORD[dwAreaTableSize];
		// na�te tabulku oblast�
		LOAD_ASSERT ( cHeaderFile.Read ( m_aAreaTable, dwAreaTableSize * sizeof ( DWORD ) ) == dwAreaTableSize * sizeof ( DWORD ) );
	}
	// tabulka oblast� je na�tena

	// alokuje a na�te tabulku bran most�
	if ( dwBridgeGateTableSize != 0 )
	{	// tabulka bran most� nen� pr�zdn�
		// alokuje tabulku bran most�
		m_aBridgeGateTable = new struct SBridgeGate[dwBridgeGateCount];
		ASSERT ( sizeof ( struct SBridgeGate ) == 4 * sizeof ( DWORD ) );
		// na�te tabulku bran most�
		LOAD_ASSERT ( cHeaderFile.Read ( m_aBridgeGateTable, dwBridgeGateTableSize * sizeof ( DWORD ) ) == dwBridgeGateTableSize * sizeof ( DWORD ) );
	}

	// alokuje a na�te tabulku hint� cest
	if ( dwPathHintTableSize != 0 )
	{	// tabulka hint� cest nen� pr�zdn�
		// alokuje tabulku hint� cest
		m_aPathHintTable = new DWORD[dwPathHintTableSize];
		// na�te tabulku hint� cest
		LOAD_ASSERT ( cHeaderFile.Read ( m_aPathHintTable, dwPathHintTableSize * sizeof ( DWORD ) ) == dwPathHintTableSize * sizeof ( DWORD ) );
	}

	// zkontroluje konec hlavi�kov�ho souboru
	LOAD_ASSERT ( cHeaderFile.GetPosition () == cHeaderFile.GetLength () );
	cHeaderFile.Close ();

// inicializuje tabulky

	// inicializuje tabulku oblast�
	for ( DWORD dwAreaIndex = 0; dwAreaIndex < dwAreaTableSize; )
	{
		// ukazatel na oblast
		struct SArea *pArea = (struct SArea *)( m_aAreaTable + dwAreaIndex );
		LOAD_ASSERT ( ( pArea->dwBridgeGateCount >= 1 ) && ( pArea->dwBridgeGateCount < 
			( pArea->dwBridgeGateCount * pArea->dwBridgeGateCount + 2 ) ) );
		// aktualizuje index dal�� oblasti
		dwAreaIndex += pArea->dwBridgeGateCount * pArea->dwBridgeGateCount + 2;
		LOAD_ASSERT ( dwAreaIndex <= dwAreaTableSize );

		// ukazatel na ukazatele na br�ny most� oblasti
		struct SBridgeGate **pBridgeGate = (struct SBridgeGate **)( pArea + 1 );
		// inicializuje ukazatele na br�ny most� oblasti
		for ( DWORD dwBridgeGateIndex = pArea->dwBridgeGateCount; dwBridgeGateIndex-- > 0; 
			pBridgeGate++ )
		{
			// inicializuje ukazatel na br�nu mostu
			LOAD_ASSERT ( ( *(DWORD *)pBridgeGate < dwBridgeGateCount ) );
			*pBridgeGate = m_aBridgeGateTable + *(DWORD *)pBridgeGate;
		}

		// ukazatel na cestu mezi branami most� oblasti
		struct SBridgeGatePath *pBridgeGatePath = (struct SBridgeGatePath *)pBridgeGate;
		// inicializuje cesty mezi branami most� oblasti
		for ( DWORD dwIndex = pArea->dwBridgeGateCount * ( pArea->dwBridgeGateCount - 1 ) / 
			2; dwIndex-- > 0; pBridgeGatePath++ )
		{
			LOAD_ASSERT ( pBridgeGatePath->dwBridgeGateDistance <= INFINITE_DISTANCE );
			// zjist�, je-li ukazatel na hint cesty platn�
			if ( (DWORD)pBridgeGatePath->pPathHint == INVALID_PATH_HINT )
			{	// ukazatel na hint cesty nen� platn�
				// aktualizuje ukazatel na pr�zdn� hunt cesty
				pBridgeGatePath->pPathHint = &m_sEmptyPathHint;
			}
			else
			{	// ukazatel na hint cesty je platn�
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
			// ukazatel na hint cesty je inicializov�n
		}
		// cesty mezi branami most� jsou inicializov�ny
		ASSERT ( (DWORD *)pBridgeGatePath == m_aAreaTable + dwAreaIndex );
	}
	// tabulka oblast� je inicializov�na
	ASSERT ( dwAreaIndex == dwAreaTableSize );

	// ukazatel na br�nu mostu
	struct SBridgeGate *pBridgeGate = m_aBridgeGateTable;
	// inicializuje tabulku bran most�
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
	// tabulka bran most� je inicializov�na
	ASSERT ( pBridgeGate == m_aBridgeGateTable + dwBridgeGateCount );

	// inicializuje tabulku hint� cest
	for ( DWORD dwPathHintIndex = 0; dwPathHintIndex < dwPathHintTableSize; )
	{
		// ukazatel na hint cesty
		struct SPathHint *pPathHint = (struct SPathHint *)( m_aPathHintTable + 
			dwPathHintIndex );
		LOAD_ASSERT ( pPathHint->dwCheckPointCount < 2 * pPathHint->dwCheckPointCount );
		// aktualizuje index dal��ho hintu cesty
		dwPathHintIndex += 2 * pPathHint->dwCheckPointCount + 1;
		LOAD_ASSERT ( dwPathHintIndex <= dwPathHintTableSize );

		// ukazatel na kontroln� bod hintu cesty
		struct SPathHintCheckPoint *pPathHintCheckPoint = 
			(struct SPathHintCheckPoint *)( pPathHint + 1 );
		// zkontroluje kontroln� body hintu cesty
		for ( DWORD dwIndex = pPathHint->dwCheckPointCount; dwIndex-- > 0; 
			pPathHintCheckPoint++ )
		{
			LOAD_ASSERT ( ( pPathHintCheckPoint->dwX < g_cMap.GetMapSizeMapCell ().cx ) && 
				( pPathHintCheckPoint->dwY < g_cMap.GetMapSizeMapCell ().cy ) );
		}
		// kontroln� body hintu cesty jsou v po��dku
	}
	// tabulka hint� cest je inicializov�na
	ASSERT ( dwPathHintIndex == dwPathHintTableSize );

// na�te mapu grafu pro hled�n� cesty

	// otev�e datov� soubor grafu pro hled�n� cesty
	CArchiveFile cDataFile = cArchive.CreateFile ( _T("beta.version.data"), 
		CFile::modeRead | CFile::shareDenyWrite );

	// zjist� velikost mapy
	DWORD dwMapSize = g_cMap.GetMapSizeMapCell ().cx * g_cMap.GetMapSizeMapCell ().cy;
	// alokuje mapu grafu pro hled�n� cesty
	m_pMap = new signed char[dwMapSize];
	// na�te mapu grafu pro hled�n� cesty
	LOAD_ASSERT ( cDataFile.Read ( m_pMap, dwMapSize ) == dwMapSize );

	// zkontroluje konec datov�ho souboru
	LOAD_ASSERT ( cDataFile.GetPosition () == cDataFile.GetLength () );
	cDataFile.Close ();

// zkontroluje ozna�en� bran most� na map�

	// ukazatel na br�nu mostu
	pBridgeGate = m_aBridgeGateTable;

	// projede br�ny most�
	for ( dwBridgeGateIndex = dwBridgeGateCount; dwBridgeGateIndex-- > 0; pBridgeGate++ )
	{
		// zkontroluje ozna�en� br�ny mostu na map�
		LOAD_ASSERT ( IsBridgeGateMapCell ( GetMapCellAt ( CPointDW ( pBridgeGate->dwX, 
			pBridgeGate->dwY ) ) ) );
	}
}

// zni�� graf pro hled�n� cesty
void CSFindPathGraph::Delete () 
{
	// zne�kodn� ID grafu pro hled�n� cesty
	m_dwFindPathGraphID = 0;

	// zni�� tabulku oblast�
	if ( m_aAreaTable != NULL )
	{	// tabulka oblast� nen� pr�zdn�
		// zni�� tabulku oblast�
		delete [] m_aAreaTable;
		m_aAreaTable = NULL;
	}

	// zni�� tabulku bran most�
	if ( m_aBridgeGateTable != NULL )
	{	// tabulka bran most� nen� pr�zdn�
		// zni�� tabulku bran most�
		delete [] m_aBridgeGateTable;
		m_aBridgeGateTable = NULL;
	}

	// zni�� tabulku hint� cest
	if ( m_aPathHintTable != NULL )
	{	// tabulka hint� cest nen� pr�zdn�
		// zni�� tabulku hint� cest
		delete [] m_aPathHintTable;
		m_aPathHintTable = NULL;
	}

	// zni�� mapu grafu pro hled�n� cesty
	if ( m_pMap != NULL )
	{	// mapa grafu pro hled�n� cesty nen� pr�zdn�
		// zni�� mapu grafu pro hled�n� cesty
		delete [] m_pMap;
		m_pMap = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Inicializace a zni�en� graf� pro hled�n� cesty
//////////////////////////////////////////////////////////////////////

// za�ne vytv��en� "dwFindPathGraphCount" graf� pro hled�n� cesty
void CSFindPathGraph::PreCreateFindPathGraphs ( DWORD dwFindPathGraphCount ) 
{
	ASSERT ( m_dwFindPathGraphCount == DWORD_MAX );
	ASSERT ( dwFindPathGraphCount != DWORD_MAX );
	ASSERT ( m_dwMaxBridgeGateCount == 0 );
	ASSERT ( m_aBridgeGateDistance == NULL );

	// uschov� si po�et graf� pro hled�n� cesty
	m_dwFindPathGraphCount = dwFindPathGraphCount;

	// vytvo�� grafy pro hled�n� cesty
	m_aFindPathGraph = ( dwFindPathGraphCount == 0 ) ? NULL : 
		new CSFindPathGraph[dwFindPathGraphCount];

	// vytvo�� pole vzd�lenost� MapCell�
	m_cMapCellDistance.Create ( g_cMap.GetMapSizeMapCell ().cx, 
		g_cMap.GetMapSizeMapCell ().cy, 2 * MAP_SQUARE_SIZE, INFINITE_DISTANCE );
}

// vytvo�� graf pro hled�n� cesty s indexem "dwIndex", s ID "dwID" a se jm�nem "szName" 
//		v archivu mapy "cMapArchive", vrac� ukazatel na vytvo�en� graf
CSFindPathGraph *CSFindPathGraph::CreateFindPathGraph ( DWORD dwIndex, DWORD dwID, 
	const char *szName, CDataArchive cMapArchive ) 
{
	ASSERT ( m_dwFindPathGraphCount != DWORD_MAX );
	ASSERT ( dwIndex < m_dwFindPathGraphCount );
	ASSERT ( m_aBridgeGateDistance == NULL );

	// jm�no adres��e grafu pro hled�n� cesty
	CString strFindPathGraphDirectoryName = FIND_PATH_GRAPHS_DIRECTORY _T("\\");
	strFindPathGraphDirectoryName += szName;
	// otev�e archiv grafu pro hled�n� cesty
	CDataArchive cFindPathGraphArchive = cMapArchive.CreateArchive ( 
		strFindPathGraphDirectoryName );

	// vypln� ID grafu pro hled�n� cesty
	m_aFindPathGraph[dwIndex].Create ( dwID, cFindPathGraphArchive );

	// vr�t� ukazatel na graf pro hled�n� cesty s indexem "dwIndex"
	return m_aFindPathGraph + dwIndex;
}

// ukon�� vytv��en� graf� pro hled�n� cesty
void CSFindPathGraph::PostCreateFindPathGraphs () 
{
	ASSERT ( m_dwFindPathGraphCount != DWORD_MAX );
	ASSERT ( m_aBridgeGateDistance == NULL );

	// alokuje pole vzd�lenost� bran most�
	if ( m_dwMaxBridgeGateCount > 0 )
	{
		m_aBridgeGateDistance = new struct SBridgeGateDistance[m_dwMaxBridgeGateCount];
	}
}

// zni�� grafy pro hled�n� cesty
void CSFindPathGraph::DeleteFindPathGraphs () 
{
	// zjit�, jsou-li grafy pro hled�n� cesty vytvo�eny
	if ( ( m_dwFindPathGraphCount != DWORD_MAX ) && ( m_dwFindPathGraphCount != 0 ) )
	{	// grafy pro hled�n� cesty jsou vytvo�eny
		ASSERT ( m_aFindPathGraph != NULL );

		// zni�� grafy pro hled�n� cesty
		for ( DWORD dwIndex = m_dwFindPathGraphCount; dwIndex-- > 0; )
		{
			// zni�� graf pro hled�n� cesty
			m_aFindPathGraph[dwIndex].Delete ();
		}

		// zni�� vytvo�en� grafy pro hled�n� cesty
		delete [] m_aFindPathGraph;

		// zne�kodn� ukazatel na grafy pro hled�n� cesty
		m_aFindPathGraph = NULL;
	}
	else
	{	// grafy pro hled�n� cesty nejsou vytvo�eny
		ASSERT ( m_aFindPathGraph == NULL );
	}

	// zne�kodn� po�et graf� pro hled�n� cesty
	m_dwFindPathGraphCount = DWORD_MAX;

	// zni�� pole vzd�lenost� MapCell�
	m_cMapCellDistance.Delete ();

	// zni�� pole vzd�lenost� bran most�
	if ( m_aBridgeGateDistance != NULL )
	{	// pole vzd�lenost� bran most� nen� pr�zdn�
		// zni�� pole vzd�lenost� bran most�
		delete [] m_aBridgeGateDistance;
		m_aBridgeGateDistance = NULL;
	}
	// zne�kodn� nejv�t�� po�et bran most� grafu pro hled�n� cesty
	m_dwMaxBridgeGateCount = 0;
}

//////////////////////////////////////////////////////////////////////
// Ukl�d�n� graf� pro hled�n� cesty (obdoba CPersistentObject metod)
//////////////////////////////////////////////////////////////////////

/*
find path graphs stored

	CSFindPathGraph *pFindPathGraphs
*/

// ukl�d�n� dat graf� pro hled�n� cesty
void CSFindPathGraph::PersistentSaveFindPathGraphs ( CPersistentStorage &storage ) 
{
	ASSERT ( m_dwFindPathGraphCount != DWORD_MAX );
	ASSERT ( ( ( m_dwFindPathGraphCount == 0 ) && ( m_aFindPathGraph == NULL ) ) || 
		( ( m_dwFindPathGraphCount > 0 ) && ( m_aFindPathGraph != NULL ) ) );

	// ulo�� ukazatel na prvn� graf pro hled�n� cesty
	storage << m_aFindPathGraph;
}

// nahr�v�n� pouze ulo�en�ch graf� pro hled�n� cesty
void CSFindPathGraph::PersistentLoadFindPathGraphs ( CPersistentStorage &storage ) 
{
	ASSERT ( m_dwFindPathGraphCount != DWORD_MAX );

	// na�te ukazatel na prvn� star� graf pro hled�n� cesty
	CSFindPathGraph *aFindPathGraph;
	storage >> (void *&)aFindPathGraph;
	LOAD_ASSERT ( ( ( m_dwFindPathGraphCount == 0 ) && ( aFindPathGraph == NULL ) ) || 
		( ( m_dwFindPathGraphCount > 0 ) && ( aFindPathGraph != NULL ) ) );


	// zaregistruje ukazatele na grafy pro hled�n� cesty
	for ( DWORD dwIndex = m_dwFindPathGraphCount; dwIndex-- > 0; )
	{
		// zaregistruje ukazatel na graf pro hled�n� cesty
		storage.RegisterPointer ( aFindPathGraph + dwIndex, m_aFindPathGraph + dwIndex );
	}
}

// p�eklad ukazatel� graf� pro hled�n� cesty
void CSFindPathGraph::PersistentTranslatePointersFindPathGraphs ( 
	CPersistentStorage &storage ) 
{
}

// inicializace nahran�ch graf� pro hled�n� cesty
void CSFindPathGraph::PersistentInitFindPathGraphs () 
{
}

//////////////////////////////////////////////////////////////////////
// Operace s grafy pro hled�n� cest
//////////////////////////////////////////////////////////////////////

// najde novou cestu ���ky "dwWidth" z m�sta "pointStart" do m�sta "pointEnd", vypln� 
//		seznam kontroln�ch bod� cesty "pCheckPointPositionList"
BOOL CSFindPathGraph::FindPath ( CPointDW pointStart, CPointDW pointEnd, 
	DWORD dwWidth, CSCheckPointPositionList *pCheckPointPositionList ) 
{
	ASSERT ( dwWidth > 0 );
	ASSERT ( pCheckPointPositionList->IsEmpty () );

	// index br�ny mostu za��tku cesty
	DWORD dwStartBridgeGateIndex;
	// index br�ny mostu konce cesty
	DWORD dwEndBridgeGateIndex;

	// zjist� index nejbli��� br�ny mostu za��tku cesty
	if ( ( dwStartBridgeGateIndex = GetNearestBridgeGateIndex ( pointStart, 
		(signed char)( dwWidth * 2 ), pointEnd ) ) == DWORD_MAX )
	{	// neexistuje br�na mostu za��tku cesty
		// cesta neexistuje
		return FALSE;
	}
	else if ( dwStartBridgeGateIndex != DWORD_MAX - 1 )
	{	// byla nalezena nejbli��� br�na mostu za��tku cesty
		// zjist� index nejbli��� br�ny mostu konce cesty
		if ( ( dwEndBridgeGateIndex = GetNearestBridgeGateIndex ( pointEnd, 
			(signed char)( dwWidth * 2 ), pointStart, TRUE ) ) == DWORD_MAX )
		{	// neexistuje br�na mostu za��tku cesty
			// cesta neexistuje
			return FALSE;
		}
		else if ( dwEndBridgeGateIndex != DWORD_MAX - 1 )
		{	// byla nalezena nejbli��� br�na mostu konce cesty
			// zjist�, jedn�-li se o shodn� br�ny mostu nebo o br�ny mostu jedn� oblasti
			if ( ( dwStartBridgeGateIndex != dwEndBridgeGateIndex ) && 
				( ( ( m_aBridgeGateTable[dwStartBridgeGateIndex].pArea != m_aBridgeGateTable[dwEndBridgeGateIndex].pArea ) && 
				( m_aBridgeGateTable[dwStartBridgeGateIndex].pArea != m_aBridgeGateTable[dwEndBridgeGateIndex].pBridgeArea ) ) || 
				( m_aBridgeGateTable[dwStartBridgeGateIndex].pArea->dwWidth < dwWidth ) ) && 
				( ( ( m_aBridgeGateTable[dwStartBridgeGateIndex].pBridgeArea != m_aBridgeGateTable[dwEndBridgeGateIndex].pArea ) && 
				( m_aBridgeGateTable[dwStartBridgeGateIndex].pBridgeArea != m_aBridgeGateTable[dwEndBridgeGateIndex].pBridgeArea ) ) || 
				( m_aBridgeGateTable[dwStartBridgeGateIndex].pBridgeArea->dwWidth < dwWidth ) ) )
			{	// nejedn� se o shodn� br�ny mostu ani o br�ny mostu jedn� oblasti
				// ******************
				CPointDW pointSecondBridgeGate, pointPreLastBridgeGate;

				// najde cestu mezi branami most�
				switch ( FindBridgeGatePath ( dwStartBridgeGateIndex, dwEndBridgeGateIndex, 
					dwWidth, pCheckPointPositionList, pointSecondBridgeGate, 
					pointPreLastBridgeGate ) )
				{
				// cesta mezi branami neexistuje
				case 0 :
					// cesta neexistuje
					return FALSE;
				// p��m� cesta by mohla b�t krat�� (jedn� se o kr�tkou cestu mezi branami)
				case 1 :
				case 2 :
					// zni�� seznam pozic kontroln�ch bod� cesty
					pCheckPointPositionList->RemoveAll ();
					break;
				// byla nalezena cesta mezi branami
				default :
					ASSERT ( !pCheckPointPositionList->IsEmpty () );

					// ******************
					// najde nov� za��tek cesty
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
							{	// jedn� se o druhou br�nu cesty
								break;
							}

							if ( point.GetDistanceSquare ( pointStart ) <= 
								( 2 * MAX_CHECK_POINT_DISTANCE ) * 
								( 2 * MAX_CHECK_POINT_DISTANCE ) )
							{	// jedn� se o bl�zk� kontroln� bod cesty
								dwNewStartIndex = dwIndex + 1;
								pointNewStart = point;
							}
						}
						while ( dwNewStartIndex-- > 0 )
						{
							(void)pCheckPointPositionList->RemoveFirst ();
						}

						// najde p�edposledn� br�nu cesty
						POSITION posPreLast = pCheckPointPositionList->GetHeadPosition ();
						do
						{
							VERIFY ( pCheckPointPositionList->GetNext ( posPreLast, point ) );
						}
						while ( point != pointPreLastBridgeGate );

						POSITION posLast = posPreLast;
						// najde nov� konec cesty
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
						// zni�� posledn� kontroln� body cesty
						while ( !pCheckPointPositionList->IsEmpty ( posLast ) )
						{
							(void)pCheckPointPositionList->RemovePosition ( posLast );
						}
					}

					// pozice posledn�ho kontroln�ho bodu v seznamu
					POSITION posLastCheckPointPosition = 
						pCheckPointPositionList->GetHeadPosition ();
					CPointDW pointLastCheckPointPosition;
					// najde pozici posledn�ho kontroln�ho bodu cesty
					while ( pCheckPointPositionList->GetNext ( posLastCheckPointPosition, 
						pointLastCheckPointPosition ) );

					// pozice br�ny mostu konce cesty
//					CPointDW pointEndBridgeGate ( 
//						m_aBridgeGateTable[dwEndBridgeGateIndex].dwX, 
//						m_aBridgeGateTable[dwEndBridgeGateIndex].dwY );

					// p�id� na konec cesty kontroln� body konce cesty
					FindShortPath ( pointLastCheckPointPosition, pointEnd, 
						(signed char)( dwWidth * 2 ), posLastCheckPointPosition );

					// pozice br�ny mostu za��tku cesty
//					CPointDW pointStartBridgeGate ( 
//						m_aBridgeGateTable[dwStartBridgeGateIndex].dwX, 
//						m_aBridgeGateTable[dwStartBridgeGateIndex].dwY );

					// dopln� cestu o kontroln� body za��tku cesty
					FindShortPath ( pointStart, pointNewStart, (signed char)( dwWidth * 2 ), 
						pCheckPointPositionList->GetHeadPosition () );

					// ******* MOZNA CASEM TEST, JESTLI TO PRECE JEN JESTE NEJDE ZKRATIT !!!

					// vr�t� p��znak nalezen� cesty
					return TRUE;
				}
				// cesta mezi branami je kr�tk�
			}
			// jedn� se o shodn� br�ny most� nebo o br�ny most� jedn� oblasti
		}
		// od konce cesty je za��tek cesty nejbli���
	}
	// od za��tku cesty je konec cesty nejbli���

	ASSERT ( pCheckPointPositionList->IsEmpty () );

	// vytvo�� kontroln� body kr�tk� cesty
	FindShortPath ( pointStart, pointEnd, (signed char)( dwWidth * 2 ), 
		pCheckPointPositionList->GetHeadPosition () );

	// vr�t� p��znak nalezen� cesty
	return TRUE;
}

// *****************************, POSITION posCheckPointPosition, CSPath *pPath
// najde lok�ln� cestu ���ky "dwWidth" z m�sta "pointStart" do m�sta "pointEnd" 
//		pro jednotku "pUnit", vrac� p�idan� a upraven� kontroln� body na pozici 
//		"posCheckPointPosition"
enum CSFindPathGraph::EFindLocalPathState CSFindPathGraph::FindLocalPath ( CSUnit *pUnit, 
	CPointDW pointStart, CPointDW pointEnd, DWORD dwWidth, CSPath *pPath ) 
{
#ifdef _DEBUG
	{
		// pozice prvn�ho kontroln�ho bodu v seznamu kontroln�ch bod� cesty
		POSITION posFirstCheckPoint = pPath->m_cCheckPointPositionList.GetHeadPosition ();
		// pozice prvn�ho kontroln�ho bodu cesty
		CPointDW pointFirstCheckPoint;
		// zjist� pozici prvn�ho kontroln�ho bodu cesty
		VERIFY ( CSCheckPointPositionList::GetNext ( posFirstCheckPoint, 
			pointFirstCheckPoint ) );
		// zkontroluje pozici prvn�ho kontroln�ho bodu cesty
		ASSERT ( pointFirstCheckPoint == pointEnd );
	}
#endif //_DEBUG

	ASSERT ( m_cPositionPriorityQueue.IsEmpty () );

	// ���ka MapCellu
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
	// zjist�, lze-li jednotku um�stit na c�lovou pozici
	if ( ( GetMapCellAt ( pointEnd ) < cMapCellWidth ) || 
		!CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointEnd, FALSE ) )
	{	// jednotku nelze um�stit na c�lovou pozici
		// fronta pozic
		static CSelfPooledQueue<CPointDW> cPositionQueue ( 50 );

		// aktualizuje vzd�lenost zak�zan�ho MapCellu
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

			// zpracuje sousedn� pozice na map�
			for ( int nIndex = 8; nIndex-- > 0; )
			{
				// sousedn� pozice na map�
				CPointDW pointSurroundingPosition = pointPosition + 
					m_aSurroundingPositionOffset[nIndex];

				// zjist�, jedn�-li se o pozici na map�
				if ( ( pointSurroundingPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
					( pointSurroundingPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
				{	// nejedn� se o pozici na map�
					// ukon�� zpracov�v�n� sousedn� pozice
					continue;
				}

				// vzd�lenost sousedn� pozice
				DWORD &rdwSurroundingPositionDistance = 
					m_cMapCellDistance.GetAt ( pointSurroundingPosition );

				// zpracuje sousedn� pozici
				if ( rdwSurroundingPositionDistance == INFINITE_DISTANCE )
				{	// jedn� se o dosud nenav�t�venou pozici
					// zjist�, jedn�-li se o povolen� MapCell
					if ( ( GetMapCellAt ( pointSurroundingPosition ) >= cMapCellWidth ) && 
						CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointSurroundingPosition, 
						FALSE ) )
					{	// jedn� se o povolen� MapCell
						// aktualizuje c�lovou pozici
						pointEndNear = pointSurroundingPosition;
						// vypr�zdn� frontu
						cPositionQueue.RemoveAll ();
						break;
					}
					else
					{	// jedn� se o zak�zan� MapCell
						// aktualizuje vzd�lenost zak�zan�ho MapCellu
						rdwSurroundingPositionDistance = DWORD_MAX;
					}
					cPositionQueue.Add ( pointSurroundingPosition );
				}
				// jedn� se o zak�zan� MapCell
				ASSERT ( rdwSurroundingPositionDistance == DWORD_MAX );
			}
			// sousedn� pozice na map� byla zpracov�na
		}
		// fronta byla zpracov�na

		if ( pointEndNear == CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ) )
		{	// nebyla nalezena c�lov� pozice
			// ukon�� um�s�ov�n� jednotky
			CSMapSquare::FindPathLoopFinishPlacingUnit ();

			// zru�� ozna�en� pozic na map�
			m_cMapCellDistance.Clear ();

			return EFLPS_PathBlocked;
		}

		if ( FIND_LOCAL_PATH_TOLERACE * FIND_LOCAL_PATH_TOLERACE * 
			(double)pointEnd.GetDistanceSquare ( pointEndNear ) >= 
			(double)pointEnd.GetDistanceSquare ( pUnit->GetPosition () ) )
		{	// n�hradn� c�l je p��li� bl�zko
			// ukon�� um�s�ov�n� jednotky
			CSMapSquare::FindPathLoopFinishPlacingUnit ();

			// zru�� ozna�en� pozic na map�
			m_cMapCellDistance.Clear ();

			pPath->RemoveCheckPoint ();

			return EFLPS_Found;
		}

		pointEnd = pointEndNear;
		bOriginalEnd = FALSE;
	}

	ASSERT ( GetMapCellAt ( pointStart ) >= cMapCellWidth );
	ASSERT ( GetMapCellAt ( pointEnd ) >= cMapCellWidth );

// hled�n� cesty na map�

	// hled�n� cesty na map�
	{
		// ozna�� v�choz� pozici na map�
		m_cMapCellDistance.GetAt ( pointStart ) = 0;
		// ozna�� koncovou pozici na map�
		m_cMapCellDistance.GetAt ( pointEnd ) = DWORD_MAX - 1;

		ASSERT ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointStart, FALSE ) );

    // Pomocn� prom�nn� pro zji��ov�n� vzd�lenosti
    DWORD dwHelpValuation;

		// p�id� do prioritn� fronty za��tek cesty
		m_cPositionPriorityQueue.Add ( pointStart, FindLocalPathValuation ( 0, FindLocalPathDistance ( pointStart, pointEnd ) ) );

		// pozice na map�
		CPointDW pointPosition;
		// vzd�lenost pozice od za��tku
		DWORD dwPositionDistance;

		// zpracuje prioritn� frontu pozic na map�
		while ( m_cPositionPriorityQueue.Remove ( pointPosition ) )
		{
      // zjistit vzd�lenost bodu od zacatku
			dwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

			// zpracuje sousedn� pozice na map�
			for ( int nIndex = 8; nIndex-- > 0; )
			{
				// sousedn� pozice na map�
				CPointDW pointSurroundingPosition = pointPosition + 
					m_aSurroundingPositionOffset[nIndex];

				// zjist�, jedn�-li se o pozici na map�
				if ( ( pointSurroundingPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
					( pointSurroundingPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
				{	// nejedn� se o pozici na map�
					// ukon�� zpracov�v�n� sousedn� pozice
					continue;
				}

				// vzd�lenost sousedn� pozice
				DWORD &rdwSurroundingPositionDistance = 
					m_cMapCellDistance.GetAt ( pointSurroundingPosition );

				// zpracuje sousedn� pozici
				if ( rdwSurroundingPositionDistance < INFINITE_DISTANCE )
				{	// jedn� se o ji� nav�t�venou pozici
					// zjist� jedn�-li se o krat�� cestu

          // pokud ano, nevadi -> vyresime pri hledani cesty pruchodem zpatky po vysledkach vlny
/*					if ( ( dwPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) < 
						rdwSurroundingPositionDistance )
					{	// jedn� se o krat�� cestu
						// sma�e pozici z prioritn� fronty
						VERIFY ( m_cPositionPriorityQueue.RemoveElement ( 
							pointSurroundingPosition ) );
						// aktualizuje vzd�lenost MapCellu
						rdwSurroundingPositionDistance = dwPositionDistance + 
							( ( nIndex & 0x01 ) ? 10 : 14 );

            // Zjist� ohodnocen� bodu
            FindLocalPathDistance ( pointSurroundingPosition, pointEnd, dwHelpDistance );
            dwHelpValuation = FindLocalPathValuation ( rdwSurroundingPositionDistance, dwHelpDistance );

            // Pokud je ohodnocen� p��li� velk�, nevlo�� ho
            if ( dwHelpValuation > FIND_LOCAL_PATH_MAX_VALUATION )
            {
				      // ukon�� um�s�ov�n� jednotky
				      CSMapSquare::FindPathLoopFinishPlacingUnit ();

				      // zni�� prioritn� frontu pozic na map�
				      m_cPositionPriorityQueue.RemoveAll ();

				      // zru�� ozna�en� pozic na map�
				      m_cMapCellDistance.Clear ();

				      // vr�t� p��znak zablokovan� cesty
				      return EFLPS_PathBlocked;
            }

						// p�id� pozici do prioritn� fronty
						m_cPositionPriorityQueue.Add ( dwHelpValuation, 
							pointSurroundingPosition );
					}*/
					// nejedn� se o krat�� cestu
				}
				else if ( rdwSurroundingPositionDistance == INFINITE_DISTANCE )
				{	// jedn� se o dosud nenav�t�venou pozici
					// zjist�, jedn�-li se o povolen� MapCell
					if ( ( GetMapCellAt ( pointSurroundingPosition ) >= cMapCellWidth ) && 
						CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointSurroundingPosition, 
						FALSE ) )
					{	// jedn� se o povolen� MapCell
						// aktualizuje vzd�lenost MapCellu
						rdwSurroundingPositionDistance = dwPositionDistance + 
							( ( nIndex & 0x01 ) ? 10 : 14 );

            // Zjist� ohodnocen� bodu
            dwHelpValuation = FindLocalPathValuation ( FindLocalPathDistance ( pointStart, pointSurroundingPosition ),
              FindLocalPathDistance ( pointSurroundingPosition, pointEnd ) );

            // Pokud je ohodnocen� p��li� velk�, nevlo�� ho
            if ( dwHelpValuation > FIND_LOCAL_PATH_MAX_VALUATION )
            {
				      // ukon�� um�s�ov�n� jednotky
				      CSMapSquare::FindPathLoopFinishPlacingUnit ();

				      // zni�� prioritn� frontu pozic na map�
				      m_cPositionPriorityQueue.Clear ();

				      // zru�� ozna�en� pozic na map�
				      m_cMapCellDistance.Clear ();

				      // vr�t� p��znak zablokovan� cesty
				      return EFLPS_PathBlocked;
            }

						// p�id� pozici do prioritn� fronty
						m_cPositionPriorityQueue.Add ( pointSurroundingPosition, dwHelpValuation );
					}
					else
					{	// jedn� se o zak�zan� MapCell
						// aktualizuje vzd�lenost zak�zan�ho MapCellu
						rdwSurroundingPositionDistance = DWORD_MAX;
					}
				}
				else if ( rdwSurroundingPositionDistance != DWORD_MAX )
				{	// jedn� se o koncovou pozici
					ASSERT ( rdwSurroundingPositionDistance == DWORD_MAX - 1 );

					// aktualizuje vzd�lenost koncov� pozice
					rdwSurroundingPositionDistance = dwPositionDistance + 
						( ( nIndex & 0x01 ) ? 10 : 14 );

					// zni�� frontu pozic na map�
					m_cPositionPriorityQueue.Clear ();
					// ukon�� zpracov�v�n� okoln�ch MapCell�
					break;
				}
				// sousedn� pozice je zpracov�na
			}
			// sousedn� pozice jsou zpracov�ny
		}
		// prioritn� fronta pozic na map� byla zpracov�na

		// zjist�, byl-li dosa�en c�l cesty
		if ( m_cMapCellDistance.GetAt ( pointEnd ) == ( DWORD_MAX - 1 ) )
		{	// c�l cesty nebyl dosa�en
			// ukon�� um�s�ov�n� jednotky
			CSMapSquare::FindPathLoopFinishPlacingUnit ();

			// zni�� prioritn� frontu pozic na map�
			m_cPositionPriorityQueue.Clear ();

			// zru�� ozna�en� pozic na map�
			m_cMapCellDistance.Clear ();

			// vr�t� p��znak zablokovan� jednotky
			return EFLPS_UnitBlocked;
		}
		// c�l cesty byl dosa�en

		ASSERT ( m_cMapCellDistance.GetAt ( pointEnd ) < INFINITE_DISTANCE );

		// zni�� prioritn� frontu pozic na map�
		m_cPositionPriorityQueue.Clear ();
	}
	// cesta na map� je nalezena

// p�evede cestu na kontroln� body

	// ************************************
	if ( !bOriginalEnd )
	{
		pPath->RemoveCheckPoint ();
		// p�id� kontroln� bod koncov�ho bodu cesty
		CSCheckPointPositionList::Insert ( 
			pPath->m_cCheckPointPositionList.GetHeadPosition (), pointEnd );
	}

	POSITION posCheckPointPosition = pPath->m_cCheckPointPositionList.GetHeadPosition ();

	// posledn� zpracovan� bod cesty
	CPointDW pointLastPosition = pointEnd;
	// vzd�lenost posledn�ho zpracovan�ho bodu cesty
	DWORD dwLastPositionDistance = m_cMapCellDistance.GetAt ( pointEnd );

	// p�evede cestu na kontroln� body
	while ( dwLastPositionDistance != 0 )
	{
		// dal�� bod cesty
		CPointDW pointNextPosition;
		// vzd�lenost dal��ho bodu cesty
		DWORD dwNextPositionDistance;

    // pomocne promenne pro nalezeni minima (nejvetsiho spadu)
    dwNextPositionDistance = INFINITE_DISTANCE;
    DWORD dwTryPositionDistance;
    CPointDW pointTryPosition;

		// najde dal�� bod cesty
		for ( int nIndex = 8; nIndex-- > 0; )
		{
			// zjist� sousedn� pozici na map�
			pointTryPosition = pointLastPosition + m_aSurroundingPositionOffset[nIndex];

			// zjist�, jedn�-li se o pozici na map�
			if ( ( pointTryPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
				( pointTryPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
			{	// nejedn� se o pozici na map�
				// ukon�� zpracov�v�n� sousedn� pozice
				continue;
			}

			// vzd�lenost sousedn� pozice na map�
			dwTryPositionDistance = m_cMapCellDistance.GetAt ( pointTryPosition );

			// zjist�, jedn�-li se o prozkoumanou pozici na map�
			if ( dwTryPositionDistance >= INFINITE_DISTANCE )
			{	// jedn� se o neprozkoumanou pozici na map�
				ASSERT ( ( dwTryPositionDistance == INFINITE_DISTANCE ) || 
					( dwTryPositionDistance == DWORD_MAX ) );
				// ukon�� zpracov�v�n� sousedn� pozice
				continue;
			}

/*			ASSERT ( ( dwNextPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) >= 
				dwLastPositionDistance );

			// zjist�, jedn�-li se o n�sleduj�c� bod cesty
			if ( ( dwNextPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) == 
				dwLastPositionDistance )
			{	// jedn� se o n�sleduj�c� bod cesty
				// nech� zpracovat bod cesty
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
		// byl nalezen dal�� bod cesty
		ASSERT ( dwNextPositionDistance < INFINITE_DISTANCE );

	// zjist�, existuje-li �se�ka mezi nelezen�m dal��m bodem a koncem cesty

		// vzd�lenost konc� �se�ky v jednotliv�ch os�ch
		CSizeDW sizeDistance = pointEnd.GetRectSize ( pointNextPosition );

		// zjist�, je-li �se�ka p��li� dlouh�
		if ( ( sizeDistance.cx * sizeDistance.cx + sizeDistance.cy * sizeDistance.cy ) >= 
			MAX_CHECK_POINT_DISTANCE * MAX_CHECK_POINT_DISTANCE )
		{	// �se�ka je p��li� dlouh�
			// p�id� kontroln� bod koncov�ho bodu cesty
			CSCheckPointPositionList::Insert ( posCheckPointPosition, pointLastPosition );
			// aktualizuje konec cesty
			pointEnd = pointLastPosition;
			// aktualizuje vzd�lenost bod� �se�ky v jednotliv�ch os�ch
			sizeDistance = pointEnd.GetRectSize ( pointNextPosition );
		}

		// zjist�, ve kter�m rozm�ru je vzd�lenost konc� �se�ky v�t��
		if ( sizeDistance.cy < sizeDistance.cx )
		{	// vzd�lenost konc� �se�ky je v�t�� v ose X
			// pozice bodu �se�ky
			CPointDW pointPosition;

			// pozice bodu �se�ky v ose X
			pointPosition.x = pointNextPosition.x;
			// p��r�stek v ose X
			DWORD dwXDifference = ( pointNextPosition.x < pointEnd.x ) ? 1 : (DWORD)-1;

			// p�esn� pozice bodu �se�ky v ose Y
			double dbYPosition = (double)pointNextPosition.y + 0.5;
			// p�esn� p��r�stek v ose Y
			double dbYDifference = ( (double)pointEnd.y - (double)pointNextPosition.y ) / 
				(double)sizeDistance.cx;

			ASSERT ( pointPosition.x != pointEnd.x );

			// "nulty" krok iterace, osa X je implicitne posunuta o "pul mapcellu",
			// na ose y pridame polovinu dbYDifference k prvni hranici mapcellu
			pointPosition.x += dwXDifference;
			dbYPosition += dbYDifference / 2;
			// pozice bodu �se�ky v ose Y
			pointPosition.y = (DWORD)dbYPosition;

			// zkontroluje MapCelly bod� �se�ky
			while ( pointPosition.x != pointEnd.x )
			{
				// zkontroluje prvn� okraj MapCellu �se�ky
				{
					// vzd�lenost bodu �se�ky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjist�, je-li MapCell bodu �se�ky povolen�
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
						// zjist�, jedn�-li se o nenav�t�ven� MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedn� se o nenav�t�ven� MapCell
							// zjist�, jedn�-li se o povolen� MapCell
							if ( ( GetMapCellAt ( pointPosition ) >= cMapCellWidth ) && 
								CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
								FALSE ) )
							{	// jedn� se o povolen� MapCell
								// aktalizuje informaci o povolen�m MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedn� se o zak�zan� MapCell
								// aktalizuje informaci o zak�zan�m MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell byl nav�t�ven
						}
						// jedn� se o nav�t�ven� MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjist�, je-li MapCell zak�zan�
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zak�zan�
							// p�id� kontroln� bod koncov�ho bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, 
									pointLastPosition );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukon�� kontrolu MapCell� bod� �se�ky
							break;
						}
						// MapCell je povolen�
					}
					// MapCell bodu �se�ky je povolen�
				}
				// prvn� okraj MapCellu �se�ky je povolen�

				// posune pozici na druh� okraj MapCellu
				dbYPosition += dbYDifference;
				// pozice bodu �se�ky v ose Y
				pointPosition.y = (DWORD)dbYPosition;

				// zkontroluje druh� okraj MapCellu �se�ky
				{
					// aktualizuje vzd�lenost bodu �se�ky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjist�, je-li MapCell bodu �se�ky povolen�
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
						// zjist�, jedn�-li se o nenav�t�ven� MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedn� se o nenav�t�ven� MapCell
							// zjist�, jedn�-li se o povolen� MapCell
							if ( ( GetMapCellAt ( pointPosition ) >= cMapCellWidth ) && 
								CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
								FALSE ) )
							{	// jedn� se o povolen� MapCell
								// aktalizuje informaci o povolen�m MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedn� se o zak�zan� MapCell
								// aktalizuje informaci o zak�zan�m MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell byl nav�t�ven
						}
						// jedn� se o nav�t�ven� MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjist�, je-li MapCell zak�zan�
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zak�zan�
							// p�id� kontroln� bod koncov�ho bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, 
								pointLastPosition );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukon�� kontrolu MapCell� bod� �se�ky
							break;
						}
						// MapCell je povolen�
					}
					// MapCell bodu �se�ky je povolen�
				}
				// druh� okraj MapCellu �se�ky je povolen�

				// posune se na dal�� bod �se�ky
				pointPosition.x += dwXDifference;
			}
			// MapCelly bod� �se�ky jsou zkontrolov�ny
		}
		else if ( sizeDistance.cy > sizeDistance.cx )
		{	// vzd�lenost konc� �se�ky je v�t�� v ose Y
			// pozice bodu �se�ky
			CPointDW pointPosition;

			// pozice bodu �se�ky v ose Y
			pointPosition.y = pointNextPosition.y;
			// p��r�stek v ose Y
			DWORD dwYDifference = ( pointNextPosition.y < pointEnd.y ) ? 1 : (DWORD)-1;

			// p�esn� pozice v ose X
			double dbXPosition = (double)pointNextPosition.x + 0.5;
			// p�esn� p��r�stek v ose X
			double dbXDifference = ( (double)pointEnd.x - (double)pointNextPosition.x ) / 
				(double)sizeDistance.cy;

			ASSERT ( pointPosition.y != pointEnd.y );

			// "nulty" krok iterace, osa Y je implicitne posunuta o "pul mapcellu",
			// na ose X pridame polovinu dbXDifference k prvni hranici mapcellu
			pointPosition.y += dwYDifference;
			dbXPosition += dbXDifference / 2;
			// pozice bodu �se�ky v ose X
			pointPosition.x = (DWORD)dbXPosition;

			// zkontroluje MapCelly bod� �se�ky
			while ( pointPosition.y != pointEnd.y )
			{
				// zkontroluje prvn� okraj MapCellu �se�ky
				{
					// vzd�lenost bodu �se�ky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjist�, je-li MapCell bodu �se�ky povolen�
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
						// zjist�, jedn�-li se o nenav�t�ven� MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedn� se o nenav�t�ven� MapCell
							// zjist�, jedn�-li se o povolen� MapCell
							if ( ( GetMapCellAt ( pointPosition ) >= cMapCellWidth ) && 
								CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
								FALSE ) )
							{	// jedn� se o povolen� MapCell
								// aktalizuje informaci o povolen�m MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedn� se o zak�zan� MapCell
								// aktalizuje informaci o zak�zan�m MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell je nav�t�ven�
						}
						// jedn� se o nav�t�ven� MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjist�, je-li MapCell zak�zan�
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zak�zan�
							// p�id� kontroln� bod koncov�ho bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, 
								pointLastPosition );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukon�� kontrolu MapCell� bod� �se�ky
							break;
						}
						// MapCell je povolen�
					}
					// MapCell bodu �se�ky je povolen�
				}
				// prvn� okraj MapCellu �se�ky je povolen�

				// posune pozici na druh� okraj MapCellu
				dbXPosition += dbXDifference;
				// pozice bodu �se�ky v ose X
				pointPosition.x = (DWORD)dbXPosition;

				// zkontroluje druh� okraj MapCellu �se�ky
				{
					// aktualizuje vzd�lenost bodu �se�ky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjist�, je-li MapCell bodu �se�ky povolen�
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
						// zjist�, jedn�-li se o nenav�t�ven� MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedn� se o nenav�t�ven� MapCell
							// zjist�, jedn�-li se o povolen� MapCell
							if ( ( GetMapCellAt ( pointPosition ) >= cMapCellWidth ) && 
								CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
								FALSE ) )
							{	// jedn� se o povolen� MapCell
								// aktalizuje informaci o povolen�m MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedn� se o zak�zan� MapCell
								// aktalizuje informaci o zak�zan�m MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell je nav�t�ven�
						}
						// jedn� se o nav�t�ven� MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjist�, je-li MapCell zak�zan�
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zak�zan�
							// p�id� kontroln� bod koncov�ho bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, 
								pointLastPosition );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukon�� kontrolu MapCell� bod� �se�ky
							break;
						}
						// MapCell je povolen�
					}
					// MapCell bodu �se�ky je povolen�
				}
				// druh� okraj MapCellu �se�ky je povolen�

				// posune se na dal�� bod �se�ky
				pointPosition.y += dwYDifference;
			}
			// MapCelly bod� �se�ky jsou zkontrolov�ny
		}
		else
		{	// vzd�lenosti konc� �se�ky jsou stejn� v ose X i Y
			// pozice bodu �se�ky
			CPointDW pointPosition = pointNextPosition;
			// p��r�stek v os�ch X a Y
			CPointDW pointDifference ( ( pointNextPosition.x < pointEnd.x ) ? 1 : (DWORD)-1, 
				( pointNextPosition.y < pointEnd.y ) ? 1 : (DWORD)-1 );

			ASSERT ( pointNextPosition != pointEnd );

			// posune se na dal�� MapCell
			pointPosition += pointDifference;

			// zkontroluje MapCelly bod� �se�ky
			while ( pointPosition != pointEnd )
			{
				// vzd�lenost bodu �se�ky
				DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

				// zjist�, je-li MapCell bodu �se�ky povolen�
				if ( rdwPositionDistance >= INFINITE_DISTANCE )
				{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
					// zjist�, jedn�-li se o nenav�t�ven� MapCell
					if ( rdwPositionDistance == INFINITE_DISTANCE )
					{	// jedn� se o nenav�t�ven� MapCell
						// zjist�, jedn�-li se o povolen� MapCell
						if ( ( GetMapCellAt ( pointPosition ) >= cMapCellWidth ) && 
							CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
							FALSE ) )
						{	// jedn� se o povolen� MapCell
							// aktalizuje informaci o povolen�m MapCellu
							rdwPositionDistance = INFINITE_DISTANCE - 1;
						}
						else
						{	// jedn� se o zak�zan� MapCell
							// aktalizuje informaci o zak�zan�m MapCellu
							rdwPositionDistance = DWORD_MAX;
						}
						// MapCell je nav�t�ven�
					}
					// jedn� se o nav�t�ven� MapCell
					ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
						( rdwPositionDistance == DWORD_MAX ) );

					// zjist�, je-li MapCell zak�zan�
					if ( rdwPositionDistance == DWORD_MAX )
					{	// MapCell je zak�zan�
						// p�id� kontroln� bod koncov�ho bodu cesty
						CSCheckPointPositionList::Insert ( posCheckPointPosition, 
							pointLastPosition );
						// aktualizuje konec cesty
						pointEnd = pointLastPosition;
						// ukon�� kontrolu MapCell� bod� �se�ky
						break;
					}
					// MapCell je povolen�
				}
				// MapCell bodu �se�ky je povolen�

				// posune se na dal�� MapCell
				pointPosition += pointDifference;
			}
			// MapCelly bod� �se�ky jsou zkontrolov�ny
		}
		// �se�ka je zpracov�na

		// aktualizuje posledn� zpracovan� bod cesty
		pointLastPosition = pointNextPosition;
		// aktualizuje vzd�lenost posledn�ho zpracovan�ho bodu cesty
		dwLastPositionDistance = dwNextPositionDistance;
	}
	// cesta byla p�evedena na kontroln� body

	// ukon�� um�s�ov�n� jednotky
	CSMapSquare::FindPathLoopFinishPlacingUnit ();

	// zru�� ozna�en� pozic na map�
	m_cMapCellDistance.Clear ();

	// vr�t� p��znak nalezen� cesty
	return EFLPS_Found;
}

// *****************************, POSITION posCheckPointPosition, CSPath *pPath
// najde vzdu�nou lok�ln� cestu z m�sta "pointStart" do m�sta "pointEnd" pro jednotku 
//		"pUnit", vrac� p�idan� a upraven� kontroln� body na pozici "posCheckPointPosition"
enum CSFindPathGraph::EFindLocalPathState CSFindPathGraph::FindAirLocalPath ( 
	CSUnit *pUnit, CPointDW pointStart, CPointDW pointEnd, CSPath *pPath ) 
{
#ifdef _DEBUG
	{
		// pozice prvn�ho kontroln�ho bodu v seznamu kontroln�ch bod� cesty
		POSITION posFirstCheckPoint = pPath->m_cCheckPointPositionList.GetHeadPosition ();
		// pozice prvn�ho kontroln�ho bodu cesty
		CPointDW pointFirstCheckPoint;
		// zjist� pozici prvn�ho kontroln�ho bodu cesty
		VERIFY ( CSCheckPointPositionList::GetNext ( posFirstCheckPoint, 
			pointFirstCheckPoint ) );
		// zkontroluje pozici prvn�ho kontroln�ho bodu cesty
		ASSERT ( pointFirstCheckPoint == pointEnd );
	}
#endif //_DEBUG

	ASSERT ( m_cPositionPriorityQueue.IsEmpty () );

// najde c�lovou pozici

	// ********************************
	BOOL bOriginalEnd = TRUE;
	// zjist�, lze-li jednotku um�stit na c�lovou pozici
	if ( !CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointEnd, FALSE ) )
	{	// jednotku nelze um�stit na c�lovou pozici
		// fronta pozic
		static CSelfPooledQueue<CPointDW> cPositionQueue ( 50 );

		// aktualizuje vzd�lenost zak�zan�ho MapCellu
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

			// zpracuje sousedn� pozice na map�
			for ( int nIndex = 8; nIndex-- > 0; )
			{
				// sousedn� pozice na map�
				CPointDW pointSurroundingPosition = pointPosition + 
					m_aSurroundingPositionOffset[nIndex];

				// zjist�, jedn�-li se o pozici na map�
				if ( ( pointSurroundingPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
					( pointSurroundingPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
				{	// nejedn� se o pozici na map�
					// ukon�� zpracov�v�n� sousedn� pozice
					continue;
				}

				// vzd�lenost sousedn� pozice
				DWORD &rdwSurroundingPositionDistance = 
					m_cMapCellDistance.GetAt ( pointSurroundingPosition );

				// zpracuje sousedn� pozici
				if ( rdwSurroundingPositionDistance == INFINITE_DISTANCE )
				{	// jedn� se o dosud nenav�t�venou pozici
					// zjist�, jedn�-li se o povolen� MapCell
					if ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, 
						pointSurroundingPosition, FALSE ) )
					{	// jedn� se o povolen� MapCell
						// aktualizuje c�lovou pozici
						pointEndNear = pointSurroundingPosition;
						// vypr�zdn� frontu
						cPositionQueue.RemoveAll ();
						break;
					}
					else
					{	// jedn� se o zak�zan� MapCell
						// aktualizuje vzd�lenost zak�zan�ho MapCellu
						rdwSurroundingPositionDistance = DWORD_MAX;
					}
					cPositionQueue.Add ( pointSurroundingPosition );
				}
				// jedn� se o zak�zan� MapCell
				ASSERT ( rdwSurroundingPositionDistance == DWORD_MAX );
			}
			// sousedn� pozice na map� byla zpracov�na
		}
		// fronta byla zpracov�na

		if ( pointEndNear == CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ) )
		{	// nebyla nalezena c�lov� pozice
			// ukon�� um�s�ov�n� jednotky
			CSMapSquare::FindPathLoopFinishPlacingUnit ();

			// zru�� ozna�en� pozic na map�
			m_cMapCellDistance.Clear ();

			return EFLPS_PathBlocked;
		}

		if ( FIND_LOCAL_PATH_TOLERACE * FIND_LOCAL_PATH_TOLERACE * 
			(double)pointEnd.GetDistanceSquare ( pointEndNear ) >= 
			(double)pointEnd.GetDistanceSquare ( pUnit->GetPosition () ) )
		{	// n�hradn� c�l je p��li� bl�zko
			// ukon�� um�s�ov�n� jednotky
			CSMapSquare::FindPathLoopFinishPlacingUnit ();

			// zru�� ozna�en� pozic na map�
			m_cMapCellDistance.Clear ();

			pPath->RemoveCheckPoint ();

			return EFLPS_Found;
		}

		pointEnd = pointEndNear;
		bOriginalEnd = FALSE;
	}

// hled�n� cesty na map�

	// hled�n� cesty na map�
	{
		// ozna�� v�choz� pozici na map�
		m_cMapCellDistance.GetAt ( pointStart ) = 0;
		// ozna�� koncovou pozici na map�
		m_cMapCellDistance.GetAt ( pointEnd ) = DWORD_MAX - 1;

		ASSERT ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointStart, FALSE ) );

    // Pomocn� prom�nn� pro zji��ov�n� vzd�lenosti
    DWORD dwHelpValuation;

		// p�id� do prioritn� fronty za��tek cesty
		m_cPositionPriorityQueue.Add ( pointStart, FindLocalPathValuation ( 0, FindLocalPathDistance ( pointStart, pointEnd ) ) );

		// pozice na map�
		CPointDW pointPosition;
		// vzd�lenost pozice od za��tku
		DWORD dwPositionDistance;

		// zpracuje prioritn� frontu pozic na map�
		while ( m_cPositionPriorityQueue.Remove ( pointPosition ) )
		{
      // zjistit vzd�lenost bodu od zacatku
			dwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

			// zpracuje sousedn� pozice na map�
			for ( int nIndex = 8; nIndex-- > 0; )
			{
				// sousedn� pozice na map�
				CPointDW pointSurroundingPosition = pointPosition + 
					m_aSurroundingPositionOffset[nIndex];

				// zjist�, jedn�-li se o pozici na map�
				if ( ( pointSurroundingPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
					( pointSurroundingPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
				{	// nejedn� se o pozici na map�
					// ukon�� zpracov�v�n� sousedn� pozice
					continue;
				}

				// vzd�lenost sousedn� pozice
				DWORD &rdwSurroundingPositionDistance = 
					m_cMapCellDistance.GetAt ( pointSurroundingPosition );

				// zpracuje sousedn� pozici
				if ( rdwSurroundingPositionDistance < INFINITE_DISTANCE )
				{	// jedn� se o ji� nav�t�venou pozici
          // neresime, vyresi to hledani cesty po zpatku po vysledkach vlny

/*					// zjist� jedn�-li se o krat�� cestu
					if ( ( dwPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) < 
						rdwSurroundingPositionDistance )
					{	// jedn� se o krat�� cestu
						// sma�e pozici z prioritn� fronty
						VERIFY ( m_cPositionPriorityQueue.RemoveElement ( 
							pointSurroundingPosition ) );
						// aktualizuje vzd�lenost MapCellu
						rdwSurroundingPositionDistance = dwPositionDistance + 
							( ( nIndex & 0x01 ) ? 10 : 14 );
            
            // Zjist� ohodnocen� bodu
            FindLocalPathDistance ( pointSurroundingPosition, pointEnd, dwHelpDistance );
            dwHelpValuation = FindLocalPathValuation ( rdwSurroundingPositionDistance, dwHelpDistance );

            if ( dwHelpValuation > FIND_LOCAL_PATH_MAX_VALUATION )
			      {	// vzd�lenost pozice od za��tku p�esahuje horn� limit
				      // ukon�� um�s�ov�n� jednotky
				      CSMapSquare::FindPathLoopFinishPlacingUnit ();

				      // zni�� prioritn� frontu pozic na map�
				      m_cPositionPriorityQueue.RemoveAll ();

				      // zru�� ozna�en� pozic na map�
				      m_cMapCellDistance.Clear ();

				      // vr�t� p��znak zablokovan� cesty
				      return EFLPS_PathBlocked;
            }

            // p�id� pozici do prioritn� fronty
						m_cPositionPriorityQueue.Add ( dwHelpValuation, 
							pointSurroundingPosition );
					}*/
					// nejedn� se o krat�� cestu
				}
				else if ( rdwSurroundingPositionDistance == INFINITE_DISTANCE )
				{	// jedn� se o dosud nenav�t�venou pozici
					// zjist�, jedn�-li se o povolen� MapCell
					if ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, 
						pointSurroundingPosition, FALSE ) )
					{	// jedn� se o povolen� MapCell
						// aktualizuje vzd�lenost MapCellu
						rdwSurroundingPositionDistance = dwPositionDistance + 
							( ( nIndex & 0x01 ) ? 10 : 14 );

            // Zjist� ohodnocen� bodu
            dwHelpValuation = FindLocalPathValuation ( FindLocalPathDistance ( pointStart, pointSurroundingPosition),
              FindLocalPathDistance ( pointSurroundingPosition, pointEnd ) );

            if ( dwHelpValuation > FIND_LOCAL_PATH_MAX_VALUATION )
			      {	// vzd�lenost pozice od za��tku p�esahuje horn� limit
				      // ukon�� um�s�ov�n� jednotky
				      CSMapSquare::FindPathLoopFinishPlacingUnit ();

				      // zni�� prioritn� frontu pozic na map�
				      m_cPositionPriorityQueue.Clear ();

				      // zru�� ozna�en� pozic na map�
				      m_cMapCellDistance.Clear ();

				      // vr�t� p��znak zablokovan� cesty
				      return EFLPS_PathBlocked;
            }

						// p�id� pozici do prioritn� fronty
						m_cPositionPriorityQueue.Add ( pointSurroundingPosition, dwHelpValuation );
					}
					else
					{	// jedn� se o zak�zan� MapCell
						// aktualizuje vzd�lenost zak�zan�ho MapCellu
						rdwSurroundingPositionDistance = DWORD_MAX;
					}
				}
				else if ( rdwSurroundingPositionDistance != DWORD_MAX )
				{	// jedn� se o koncovou pozici
					ASSERT ( rdwSurroundingPositionDistance == DWORD_MAX - 1 );

					// aktualizuje vzd�lenost koncov� pozice
					rdwSurroundingPositionDistance = dwPositionDistance + 
						( ( nIndex & 0x01 ) ? 10 : 14 );

					// zni�� frontu pozic na map�
					m_cPositionPriorityQueue.Clear ();
					// ukon�� zpracov�v�n� okoln�ch MapCell�
					break;
				}
				// sousedn� pozice je zpracov�na
			}
			// sousedn� pozice jsou zpracov�ny
		}
		// prioritn� fronta pozic na map� byla zpracov�na

		// zjist�, byl-li dosa�en c�l cesty
		if ( m_cMapCellDistance.GetAt ( pointEnd ) == ( DWORD_MAX - 1 ) )
		{	// c�l cesty nebyl dosa�en
			// ukon�� um�s�ov�n� jednotky
			CSMapSquare::FindPathLoopFinishPlacingUnit ();

			// zni�� prioritn� frontu pozic na map�
			m_cPositionPriorityQueue.Clear ();

			// zru�� ozna�en� pozic na map�
			m_cMapCellDistance.Clear ();

			// vr�t� p��znak zablokovan� jednotky
			return EFLPS_UnitBlocked;
		}
		// c�l cesty byl dosa�en

		ASSERT ( m_cMapCellDistance.GetAt ( pointEnd ) < INFINITE_DISTANCE );

		// zni�� prioritn� frontu pozic na map�
		m_cPositionPriorityQueue.Clear ();
	}
	// cesta na map� je nalezena

// p�evede cestu na kontroln� body

	// ************************************
	if ( !bOriginalEnd )
	{
		pPath->RemoveCheckPoint ();
		// p�id� kontroln� bod koncov�ho bodu cesty
		CSCheckPointPositionList::Insert ( 
			pPath->m_cCheckPointPositionList.GetHeadPosition (), pointEnd );
	}

	POSITION posCheckPointPosition = pPath->m_cCheckPointPositionList.GetHeadPosition ();

	// posledn� zpracovan� bod cesty
	CPointDW pointLastPosition = pointEnd;
	// vzd�lenost posledn�ho zpracovan�ho bodu cesty
	DWORD dwLastPositionDistance = m_cMapCellDistance.GetAt ( pointEnd );

	// p�evede cestu na kontroln� body
	while ( dwLastPositionDistance != 0 )
	{
		// dal�� bod cesty
		CPointDW pointNextPosition;
		// vzd�lenost dal��ho bodu cesty
		DWORD dwNextPositionDistance;

    // pomocne promenne pro nalezeni minima (nejvetsiho spadu)
    dwNextPositionDistance = INFINITE_DISTANCE;
    DWORD dwTryPositionDistance;
    CPointDW pointTryPosition;

		// najde dal�� bod cesty
		for ( int nIndex = 8; nIndex-- > 0; )
		{
			// zjist� sousedn� pozici na map�
			pointTryPosition = pointLastPosition + m_aSurroundingPositionOffset[nIndex];

			// zjist�, jedn�-li se o pozici na map�
			if ( ( pointTryPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
				( pointTryPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
			{	// nejedn� se o pozici na map�
				// ukon�� zpracov�v�n� sousedn� pozice
				continue;
			}

			// vzd�lenost sousedn� pozice na map�
			dwTryPositionDistance = m_cMapCellDistance.GetAt ( pointTryPosition );

			// zjist�, jedn�-li se o prozkoumanou pozici na map�
			if ( dwTryPositionDistance >= INFINITE_DISTANCE )
			{	// jedn� se o neprozkoumanou pozici na map�
				ASSERT ( ( dwTryPositionDistance == INFINITE_DISTANCE ) || 
					( dwTryPositionDistance == DWORD_MAX ) );
				// ukon�� zpracov�v�n� sousedn� pozice
				continue;
			}

/*			ASSERT ( ( ( dwNextPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) >= 
				dwLastPositionDistance ) || ( pointLastPosition == pointEnd ) );

			// zjist�, jedn�-li se o n�sleduj�c� bod cesty
			if ( ( dwNextPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) == 
				dwLastPositionDistance )
			{	// jedn� se o n�sleduj�c� bod cesty
				// nech� zpracovat bod cesty
				break;
			}*/

      if ( ( dwTryPositionDistance < dwLastPositionDistance ) && ( dwTryPositionDistance < dwNextPositionDistance ) )
      {
        dwNextPositionDistance = dwTryPositionDistance;
        pointNextPosition = pointTryPosition;
        continue;
      }
		}
		// byl nalezen dal�� bod cesty
		ASSERT ( dwNextPositionDistance < INFINITE_DISTANCE );

	// zjist�, existuje-li �se�ka mezi nelezen�m dal��m bodem a koncem cesty

		// vzd�lenost konc� �se�ky v jednotliv�ch os�ch
		CSizeDW sizeDistance = pointEnd.GetRectSize ( pointNextPosition );

		// zjist�, je-li �se�ka p��li� dlouh�
		if ( ( sizeDistance.cx * sizeDistance.cx + sizeDistance.cy * sizeDistance.cy ) >= 
			MAX_CHECK_POINT_DISTANCE * MAX_CHECK_POINT_DISTANCE )
		{	// �se�ka je p��li� dlouh�
			// p�id� kontroln� bod koncov�ho bodu cesty
			CSCheckPointPositionList::Insert ( posCheckPointPosition, pointLastPosition );
			// aktualizuje konec cesty
			pointEnd = pointLastPosition;
			// aktualizuje vzd�lenost bod� �se�ky v jednotliv�ch os�ch
			sizeDistance = pointEnd.GetRectSize ( pointNextPosition );
		}

		// zjist�, ve kter�m rozm�ru je vzd�lenost konc� �se�ky v�t��
		if ( sizeDistance.cy < sizeDistance.cx )
		{	// vzd�lenost konc� �se�ky je v�t�� v ose X
			// pozice bodu �se�ky
			CPointDW pointPosition;

			// pozice bodu �se�ky v ose X
			pointPosition.x = pointNextPosition.x;
			// p��r�stek v ose X
			DWORD dwXDifference = ( pointNextPosition.x < pointEnd.x ) ? 1 : (DWORD)-1;

			// p�esn� pozice bodu �se�ky v ose Y
			double dbYPosition = (double)pointNextPosition.y + 0.5;
			// p�esn� p��r�stek v ose Y
			double dbYDifference = ( (double)pointEnd.y - (double)pointNextPosition.y ) / 
				(double)sizeDistance.cx;

			ASSERT ( pointPosition.x != pointEnd.x );

			// "nulty" krok iterace, osa X je implicitne posunuta o "pul mapcellu",
			// na ose y pridame polovinu dbYDifference k prvni hranici mapcellu
			pointPosition.x += dwXDifference;
			dbYPosition += dbYDifference / 2;
			// pozice bodu �se�ky v ose Y
			pointPosition.y = (DWORD)dbYPosition;

			// zkontroluje MapCelly bod� �se�ky
			while ( pointPosition.x != pointEnd.x )
			{
				// zkontroluje prvn� okraj MapCellu �se�ky
				{
					// vzd�lenost bodu �se�ky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjist�, je-li MapCell bodu �se�ky povolen�
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
						// zjist�, jedn�-li se o nenav�t�ven� MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedn� se o nenav�t�ven� MapCell
							// zjist�, jedn�-li se o povolen� MapCell
							if ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
								FALSE ) )
							{	// jedn� se o povolen� MapCell
								// aktalizuje informaci o povolen�m MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedn� se o zak�zan� MapCell
								// aktalizuje informaci o zak�zan�m MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell byl nav�t�ven
						}
						// jedn� se o nav�t�ven� MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjist�, je-li MapCell zak�zan�
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zak�zan�
							// p�id� kontroln� bod koncov�ho bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, 
								pointLastPosition );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukon�� kontrolu MapCell� bod� �se�ky
							break;
						}
						// MapCell je povolen�
					}
					// MapCell bodu �se�ky je povolen�
				}
				// prvn� okraj MapCellu �se�ky je povolen�

				// posune pozici na druh� okraj MapCellu
				dbYPosition += dbYDifference;
				// pozice bodu �se�ky v ose Y
				pointPosition.y = (DWORD)dbYPosition;

				// zkontroluje druh� okraj MapCellu �se�ky
				{
					// aktualizuje vzd�lenost bodu �se�ky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjist�, je-li MapCell bodu �se�ky povolen�
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
						// zjist�, jedn�-li se o nenav�t�ven� MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedn� se o nenav�t�ven� MapCell
							// zjist�, jedn�-li se o povolen� MapCell
							if ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
								FALSE ) )
							{	// jedn� se o povolen� MapCell
								// aktalizuje informaci o povolen�m MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedn� se o zak�zan� MapCell
								// aktalizuje informaci o zak�zan�m MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell byl nav�t�ven
						}
						// jedn� se o nav�t�ven� MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjist�, je-li MapCell zak�zan�
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zak�zan�
							// p�id� kontroln� bod koncov�ho bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, 
								pointLastPosition );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukon�� kontrolu MapCell� bod� �se�ky
							break;
						}
						// MapCell je povolen�
					}
					// MapCell bodu �se�ky je povolen�
				}
				// druh� okraj MapCellu �se�ky je povolen�

				// posune se na dal�� bod �se�ky
				pointPosition.x += dwXDifference;
			}
			// MapCelly bod� �se�ky jsou zkontrolov�ny
		}
		else if ( sizeDistance.cy > sizeDistance.cx )
		{	// vzd�lenost konc� �se�ky je v�t�� v ose Y
			// pozice bodu �se�ky
			CPointDW pointPosition;

			// pozice bodu �se�ky v ose Y
			pointPosition.y = pointNextPosition.y;
			// p��r�stek v ose Y
			DWORD dwYDifference = ( pointNextPosition.y < pointEnd.y ) ? 1 : (DWORD)-1;

			// p�esn� pozice v ose X
			double dbXPosition = (double)pointNextPosition.x + 0.5;
			// p�esn� p��r�stek v ose X
			double dbXDifference = ( (double)pointEnd.x - (double)pointNextPosition.x ) / 
				(double)sizeDistance.cy;

			ASSERT ( pointPosition.y != pointEnd.y );

			// "nulty" krok iterace, osa Y je implicitne posunuta o "pul mapcellu",
			// na ose X pridame polovinu dbXDifference k prvni hranici mapcellu
			pointPosition.y += dwYDifference;
			dbXPosition += dbXDifference / 2;
			// pozice bodu �se�ky v ose X
			pointPosition.x = (DWORD)dbXPosition;

			// zkontroluje MapCelly bod� �se�ky
			while ( pointPosition.y != pointEnd.y )
			{
				// zkontroluje prvn� okraj MapCellu �se�ky
				{
					// vzd�lenost bodu �se�ky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjist�, je-li MapCell bodu �se�ky povolen�
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
						// zjist�, jedn�-li se o nenav�t�ven� MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedn� se o nenav�t�ven� MapCell
							// zjist�, jedn�-li se o povolen� MapCell
							if ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
								FALSE ) )
							{	// jedn� se o povolen� MapCell
								// aktalizuje informaci o povolen�m MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedn� se o zak�zan� MapCell
								// aktalizuje informaci o zak�zan�m MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell je nav�t�ven�
						}
						// jedn� se o nav�t�ven� MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjist�, je-li MapCell zak�zan�
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zak�zan�
							// p�id� kontroln� bod koncov�ho bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, 
								pointLastPosition );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukon�� kontrolu MapCell� bod� �se�ky
							break;
						}
						// MapCell je povolen�
					}
					// MapCell bodu �se�ky je povolen�
				}
				// prvn� okraj MapCellu �se�ky je povolen�

				// posune pozici na druh� okraj MapCellu
				dbXPosition += dbXDifference;
				// pozice bodu �se�ky v ose X
				pointPosition.x = (DWORD)dbXPosition;

				// zkontroluje druh� okraj MapCellu �se�ky
				{
					// aktualizuje vzd�lenost bodu �se�ky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjist�, je-li MapCell bodu �se�ky povolen�
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
						// zjist�, jedn�-li se o nenav�t�ven� MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedn� se o nenav�t�ven� MapCell
							// zjist�, jedn�-li se o povolen� MapCell
							if ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
								FALSE ) )
							{	// jedn� se o povolen� MapCell
								// aktalizuje informaci o povolen�m MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedn� se o zak�zan� MapCell
								// aktalizuje informaci o zak�zan�m MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell je nav�t�ven�
						}
						// jedn� se o nav�t�ven� MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjist�, je-li MapCell zak�zan�
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zak�zan�
							// p�id� kontroln� bod koncov�ho bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, 
								pointLastPosition );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukon�� kontrolu MapCell� bod� �se�ky
							break;
						}
						// MapCell je povolen�
					}
					// MapCell bodu �se�ky je povolen�
				}
				// druh� okraj MapCellu �se�ky je povolen�

				// posune se na dal�� bod �se�ky
				pointPosition.y += dwYDifference;
			}
			// MapCelly bod� �se�ky jsou zkontrolov�ny
		}
		else
		{	// vzd�lenosti konc� �se�ky jsou stejn� v ose X i Y
			// pozice bodu �se�ky
			CPointDW pointPosition = pointNextPosition;
			// p��r�stek v os�ch X a Y
			CPointDW pointDifference ( ( pointNextPosition.x < pointEnd.x ) ? 1 : (DWORD)-1, 
				( pointNextPosition.y < pointEnd.y ) ? 1 : (DWORD)-1 );

			ASSERT ( pointNextPosition != pointEnd );

			// posune se na dal�� MapCell
			pointPosition += pointDifference;

			// zkontroluje MapCelly bod� �se�ky
			while ( pointPosition != pointEnd )
			{
				// vzd�lenost bodu �se�ky
				DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

				// zjist�, je-li MapCell bodu �se�ky povolen�
				if ( rdwPositionDistance >= INFINITE_DISTANCE )
				{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
					// zjist�, jedn�-li se o nenav�t�ven� MapCell
					if ( rdwPositionDistance == INFINITE_DISTANCE )
					{	// jedn� se o nenav�t�ven� MapCell
						// zjist�, jedn�-li se o povolen� MapCell
						if ( CSMapSquare::FindPathLoopCanPlaceUnit ( pUnit, pointPosition, 
							FALSE ) )
						{	// jedn� se o povolen� MapCell
							// aktalizuje informaci o povolen�m MapCellu
							rdwPositionDistance = INFINITE_DISTANCE - 1;
						}
						else
						{	// jedn� se o zak�zan� MapCell
							// aktalizuje informaci o zak�zan�m MapCellu
							rdwPositionDistance = DWORD_MAX;
						}
						// MapCell je nav�t�ven�
					}
					// jedn� se o nav�t�ven� MapCell
					ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
						( rdwPositionDistance == DWORD_MAX ) );

					// zjist�, je-li MapCell zak�zan�
					if ( rdwPositionDistance == DWORD_MAX )
					{	// MapCell je zak�zan�
						// p�id� kontroln� bod koncov�ho bodu cesty
						CSCheckPointPositionList::Insert ( posCheckPointPosition, 
							pointLastPosition );
						// aktualizuje konec cesty
						pointEnd = pointLastPosition;
						// ukon�� kontrolu MapCell� bod� �se�ky
						break;
					}
					// MapCell je povolen�
				}
				// MapCell bodu �se�ky je povolen�

				// posune se na dal�� MapCell
				pointPosition += pointDifference;
			}
			// MapCelly bod� �se�ky jsou zkontrolov�ny
		}
		// �se�ka je zpracov�na

		// aktualizuje posledn� zpracovan� bod cesty
		pointLastPosition = pointNextPosition;
		// aktualizuje vzd�lenost posledn�ho zpracovan�ho bodu cesty
		dwLastPositionDistance = dwNextPositionDistance;
	}
	// cesta byla p�evedena na kontroln� body

	// ukon�� um�s�ov�n� jednotky
	CSMapSquare::FindPathLoopFinishPlacingUnit ();

	// zru�� ozna�en� pozic na map�
	m_cMapCellDistance.Clear ();

	// vr�t� p��znak nalezen� cesty
	return EFLPS_Found;
}

//////////////////////////////////////////////////////////////////////
// Pomocn� operace pro hled�n� cesty
//////////////////////////////////////////////////////////////////////

// vlo�� na pozici "posCheckPointPosition" seznam kontroln�ch bod� kr�tk� cesty pro ���ku 
//		MapCellu "cMapCellWidth" z m�sta "pointStart" do m�sta "pointEnd" (cesta ji� ur�it� 
//		existuje)
void CSFindPathGraph::FindShortPath ( CPointDW pointStart, CPointDW pointEnd, 
	signed char cMapCellWidth, POSITION posCheckPointPosition ) 
{
	ASSERT ( ( cMapCellWidth > 0 ) && ( ( cMapCellWidth % 2 ) == 0 ) );

	ASSERT ( m_cPositionPriorityQueue.IsEmpty () );

	ASSERT ( GetMapCellAt ( pointStart ) >= cMapCellWidth );
	ASSERT ( GetMapCellAt ( pointEnd ) >= cMapCellWidth );

	// zjist�, shoduje-li se po��tek a konec cesty
	if ( pointStart == pointEnd )
	{	// po��tek a konec cesty se shoduj�
		// vlo�� pr�zdnou cestu
		return;
	}
	// za��tek a konec cesty se neshoduj�

// hled�n� cesty na map�

	// ozna�� v�choz� pozici na map�
	m_cMapCellDistance.GetAt ( pointStart ) = 0;
	// ozna�� koncovou pozici na map�
	m_cMapCellDistance.GetAt ( pointEnd ) = DWORD_MAX - 1;

  // Pomocn� prom�nn� pro zji��ov�n� vzd�lenosti
  DWORD dwHelpValuation;

  // p�id� do prioritn� fronty za��tek cesty
	m_cPositionPriorityQueue.Add ( pointStart, FindLocalPathValuation ( 0, FindLocalPathDistance ( pointStart, pointEnd ) ) );

	// pozice na map�
	CPointDW pointPosition;
	// vzd�lenost pozice od za��tku
	DWORD dwPositionDistance;

	// zpracuje prioritn� frontu pozic na map�
	while ( m_cPositionPriorityQueue.Remove ( pointPosition ) )
	{
    // zjisti vzdalenost bodu od zacatku
    dwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

		// zpracuje sousedn� pozice na map�
		for ( int nIndex = 8; nIndex-- > 0; )
		{
			// sousedn� pozice na map�
			CPointDW pointSurroundingPosition = pointPosition + 
				m_aSurroundingPositionOffset[nIndex];

			// zjist�, jedn�-li se o pozici na map�
			if ( ( pointSurroundingPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
				( pointSurroundingPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
			{	// nejedn� se o pozici na map�
				// ukon�� zpracov�v�n� sousedn� pozice
				continue;
			}

			// vzd�lenost sousedn� pozice
			DWORD &rdwSurroundingPositionDistance = m_cMapCellDistance.GetAt ( 
				pointSurroundingPosition );

			// zpracuje sousedn� pozici
			if ( rdwSurroundingPositionDistance < INFINITE_DISTANCE )
			{	// jedn� se o ji� nav�t�venou pozici
        // vyresi se pri zpetnem pruchodu vysledkami vlny
/*				// zjist� jedn�-li se o krat�� cestu
				if ( ( dwPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) < 
					rdwSurroundingPositionDistance )
				{	// jedn� se o krat�� cestu
					// sma�e pozici z prioritn� fronty
					VERIFY ( m_cPositionPriorityQueue.RemoveElement ( 
						pointSurroundingPosition ) );
					// aktualizuje vzd�lenost MapCellu
					rdwSurroundingPositionDistance = dwPositionDistance + 
						( ( nIndex & 0x01 ) ? 10 : 14 );
					// p�id� pozici do prioritn� fronty
					m_cPositionPriorityQueue.Add ( rdwSurroundingPositionDistance, 
						pointSurroundingPosition );
				}*/
				// nejedn� se o krat�� cestu
			}
			else if ( rdwSurroundingPositionDistance == INFINITE_DISTANCE )
			{	// jedn� se o dosud nenav�t�venou pozici
				// zjist�, jedn�-li se o povolen� MapCell
				if ( GetMapCellAt ( pointSurroundingPosition ) >= cMapCellWidth )
				{	// jedn� se o povolen� MapCell
					// aktualizuje vzd�lenost MapCellu
					rdwSurroundingPositionDistance = dwPositionDistance + 
						( ( nIndex & 0x01 ) ? 10 : 14 );

          // Zjist� ohodnocen� bodu
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

					// p�id� pozici do prioritn� fronty
					m_cPositionPriorityQueue.Add ( pointSurroundingPosition, dwHelpValuation );
				}
				else
				{	// jedn� se o zak�zan� MapCell
					// aktualizuje vzd�lenost zak�zan�ho MapCellu
					rdwSurroundingPositionDistance = DWORD_MAX;
				}
			}
			else if ( rdwSurroundingPositionDistance != DWORD_MAX )
			{	// jedn� se o koncovou pozici
				ASSERT ( rdwSurroundingPositionDistance == DWORD_MAX - 1 );

				// aktualizuje vzd�lenost koncov� pozice
				rdwSurroundingPositionDistance = dwPositionDistance + 
					( ( nIndex & 0x01 ) ? 10 : 14 );

				// zni�� frontu pozic na map�
				m_cPositionPriorityQueue.Clear ();

				// ukon�� zpracov�v�n� okoln�ch MapCell�
				break;
			}
			// sousedn� pozice je zpracov�na
		}
		// sousedn� pozice jsou zpracov�ny
	}

	// prioritn� fronta pozic na map� byla zpracov�na
	ASSERT ( m_cMapCellDistance.GetAt ( pointEnd ) < INFINITE_DISTANCE );

	// zni�� prioritn� frontu pozic na map�
	m_cPositionPriorityQueue.Clear ();

// p�evede cestu na kontroln� body

	// posledn� zpracovan� bod cesty
	CPointDW pointLastPosition = pointEnd;
	// vzd�lenost posledn�ho zpracovan�ho bodu cesty
	DWORD dwLastPositionDistance = m_cMapCellDistance.GetAt ( pointEnd );

	// p�evede cestu na kontroln� body
	while ( dwLastPositionDistance != 0 )
	{
		// dal�� bod cesty
		CPointDW pointNextPosition;
		// vzd�lenost dal��ho bodu cesty
		DWORD dwNextPositionDistance;

    // pomocne promenne pro nalezeni minima (nejvetsiho spadu)
    dwNextPositionDistance = INFINITE_DISTANCE;
    DWORD dwTryPositionDistance;
    CPointDW pointTryPosition;

    // najde dal�� bod cesty
		for ( int nIndex = 8; nIndex-- > 0; )
		{
			// zjist� sousedn� pozici na map�
			pointTryPosition = pointLastPosition + m_aSurroundingPositionOffset[nIndex];

			// zjist�, jedn�-li se o pozici na map�
			if ( ( pointTryPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
				( pointTryPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
			{	// nejedn� se o pozici na map�
				// ukon�� zpracov�v�n� sousedn� pozice
				continue;
			}

			// vzd�lenost sousedn� pozice na map�
			dwTryPositionDistance = m_cMapCellDistance.GetAt ( pointTryPosition );

			// zjist�, jedn�-li se o prozkoumanou pozici na map�
			if ( dwTryPositionDistance >= INFINITE_DISTANCE )
			{	// jedn� se o neprozkoumanou pozici na map�
				ASSERT ( ( dwTryPositionDistance == INFINITE_DISTANCE ) || 
					( dwTryPositionDistance == DWORD_MAX ) );
				// ukon�� zpracov�v�n� sousedn� pozice
				continue;
			}

/*			ASSERT ( ( dwNextPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) >= 
				dwLastPositionDistance );

			// zjist�, jedn�-li se o n�sleduj�c� bod cesty
			if ( ( dwNextPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) == 
				dwLastPositionDistance )
			{	// jedn� se o n�sleduj�c� bod cesty
				// nech� zpracovat bod cesty
				break;
			}*/

      if ( ( dwTryPositionDistance < dwLastPositionDistance ) && ( dwTryPositionDistance < dwNextPositionDistance ) )
      {
        dwNextPositionDistance = dwTryPositionDistance;
        pointNextPosition = pointTryPosition;
        continue;
      }
		}
		// byl nalezen dal�� bod cesty
		ASSERT ( dwNextPositionDistance < INFINITE_DISTANCE );

	// zjist�, existuje-li �se�ka mezi nelezen�m dal��m bodem a koncem cesty

		// vzd�lenost konc� �se�ky v jednotliv�ch os�ch
		CSizeDW sizeDistance = pointEnd.GetRectSize ( pointNextPosition );

		// zjist�, je-li �se�ka p��li� dlouh�
		if ( ( sizeDistance.cx * sizeDistance.cx + sizeDistance.cy * sizeDistance.cy ) >= 
			MAX_CHECK_POINT_DISTANCE * MAX_CHECK_POINT_DISTANCE )
		{	// �se�ka je p��li� dlouh�
			// p�id� kontroln� bod koncov�ho bodu cesty
			CSCheckPointPositionList::Insert ( posCheckPointPosition, pointEnd );
			// aktualizuje konec cesty
			pointEnd = pointLastPosition;
			// aktualizuje vzd�lenost bod� �se�ky v jednotliv�ch os�ch
			sizeDistance = pointEnd.GetRectSize ( pointNextPosition );
		}

		// zjist�, ve kter�m rozm�ru je vzd�lenost konc� �se�ky v�t��
		if ( sizeDistance.cy < sizeDistance.cx )
		{	// vzd�lenost konc� �se�ky je v�t�� v ose X
			// pozice bodu �se�ky
			CPointDW pointPosition;

			// pozice bodu �se�ky v ose X
			pointPosition.x = pointNextPosition.x;
			// p��r�stek v ose X
			DWORD dwXDifference = ( pointNextPosition.x < pointEnd.x ) ? 1 : (DWORD)-1;

			// p�esn� pozice bodu �se�ky v ose Y
			double dbYPosition = (double)pointNextPosition.y + 0.5;
			// p�esn� p��r�stek v ose Y
			double dbYDifference = ( (double)pointEnd.y - (double)pointNextPosition.y ) / 
				(double)sizeDistance.cx;

			ASSERT ( pointPosition.x != pointEnd.x );

			// "nulty" krok iterace, osa X je implicitne posunuta o "pul mapcellu",
			// na ose y pridame polovinu dbYDifference k prvni hranici mapcellu
			pointPosition.x += dwXDifference;
			dbYPosition += dbYDifference / 2;
			// pozice bodu �se�ky v ose Y
			pointPosition.y = (DWORD)dbYPosition;

			// zkontroluje MapCelly bod� �se�ky
			while ( pointPosition.x != pointEnd.x )
			{
				// zkontroluje prvn� okraj MapCellu �se�ky
				{
					// vzd�lenost bodu �se�ky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjist�, je-li MapCell bodu �se�ky povolen�
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
						// zjist�, jedn�-li se o nenav�t�ven� MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedn� se o nenav�t�ven� MapCell
							// zjist�, jedn�-li se o povolen� MapCell
							if ( GetMapCellAt ( pointPosition ) >= cMapCellWidth )
							{	// jedn� se o povolen� MapCell
								// aktalizuje informaci o povolen�m MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedn� se o zak�zan� MapCell
								// aktalizuje informaci o zak�zan�m MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell byl nav�t�ven
						}
						// jedn� se o nav�t�ven� MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjist�, je-li MapCell zak�zan�
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zak�zan�
							// p�id� kontroln� bod koncov�ho bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, pointEnd );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukon�� kontrolu MapCell� bod� �se�ky
							break;
						}
						// MapCell je povolen�
					}
					// MapCell bodu �se�ky je povolen�
				}
				// prvn� okraj MapCellu �se�ky je povolen�

				// posune pozici na druh� okraj MapCellu
				dbYPosition += dbYDifference;
				// pozice bodu �se�ky v ose Y
				pointPosition.y = (DWORD)dbYPosition;

				// zkontroluje druh� okraj MapCellu �se�ky
				{
					// aktualizuje vzd�lenost bodu �se�ky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjist�, je-li MapCell bodu �se�ky povolen�
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
						// zjist�, jedn�-li se o nenav�t�ven� MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedn� se o nenav�t�ven� MapCell
							// zjist�, jedn�-li se o povolen� MapCell
							if ( GetMapCellAt ( pointPosition ) >= cMapCellWidth )
							{	// jedn� se o povolen� MapCell
								// aktalizuje informaci o povolen�m MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedn� se o zak�zan� MapCell
								// aktalizuje informaci o zak�zan�m MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell byl nav�t�ven
						}
						// jedn� se o nav�t�ven� MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjist�, je-li MapCell zak�zan�
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zak�zan�
							// p�id� kontroln� bod koncov�ho bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, pointEnd );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukon�� kontrolu MapCell� bod� �se�ky
							break;
						}
						// MapCell je povolen�
					}
					// MapCell bodu �se�ky je povolen�
				}
				// druh� okraj MapCellu �se�ky je povolen�

				// posune se na dal�� bod �se�ky
				pointPosition.x += dwXDifference;
			}
			// MapCelly bod� �se�ky jsou zkontrolov�ny
		}
		else if ( sizeDistance.cy > sizeDistance.cx )
		{	// vzd�lenost konc� �se�ky je v�t�� v ose Y
			// pozice bodu �se�ky
			CPointDW pointPosition;

			// pozice bodu �se�ky v ose Y
			pointPosition.y = pointNextPosition.y;
			// p��r�stek v ose Y
			DWORD dwYDifference = ( pointNextPosition.y < pointEnd.y ) ? 1 : (DWORD)-1;

			// p�esn� pozice v ose X
			double dbXPosition = (double)pointNextPosition.x + 0.5;
			// p�esn� p��r�stek v ose X
			double dbXDifference = ( (double)pointEnd.x - (double)pointNextPosition.x ) / 
				(double)sizeDistance.cy;

			ASSERT ( pointPosition.y != pointEnd.y );

			// "nulty" krok iterace, osa Y je implicitne posunuta o "pul mapcellu",
			// na ose X pridame polovinu dbXDifference k prvni hranici mapcellu
			pointPosition.y += dwYDifference;
			dbXPosition += dbXDifference / 2;
			// pozice bodu �se�ky v ose X
			pointPosition.x = (DWORD)dbXPosition;

			// zkontroluje MapCelly bod� �se�ky
			while ( pointPosition.y != pointEnd.y )
			{
				// zkontroluje prvn� okraj MapCellu �se�ky
				{
					// vzd�lenost bodu �se�ky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjist�, je-li MapCell bodu �se�ky povolen�
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
						// zjist�, jedn�-li se o nenav�t�ven� MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedn� se o nenav�t�ven� MapCell
							// zjist�, jedn�-li se o povolen� MapCell
							if ( GetMapCellAt ( pointPosition ) >= cMapCellWidth )
							{	// jedn� se o povolen� MapCell
								// aktalizuje informaci o povolen�m MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedn� se o zak�zan� MapCell
								// aktalizuje informaci o zak�zan�m MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell je nav�t�ven�
						}
						// jedn� se o nav�t�ven� MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjist�, je-li MapCell zak�zan�
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zak�zan�
							// p�id� kontroln� bod koncov�ho bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, pointEnd );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukon�� kontrolu MapCell� bod� �se�ky
							break;
						}
						// MapCell je povolen�
					}
					// MapCell bodu �se�ky je povolen�
				}
				// prvn� okraj MapCellu �se�ky je povolen�

				// posune pozici na druh� okraj MapCellu
				dbXPosition += dbXDifference;
				// pozice bodu �se�ky v ose X
				pointPosition.x = (DWORD)dbXPosition;

				// zkontroluje druh� okraj MapCellu �se�ky
				{
					// aktualizuje vzd�lenost bodu �se�ky
					DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

					// zjist�, je-li MapCell bodu �se�ky povolen�
					if ( rdwPositionDistance >= INFINITE_DISTANCE )
					{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
						// zjist�, jedn�-li se o nenav�t�ven� MapCell
						if ( rdwPositionDistance == INFINITE_DISTANCE )
						{	// jedn� se o nenav�t�ven� MapCell
							// zjist�, jedn�-li se o povolen� MapCell
							if ( GetMapCellAt ( pointPosition ) >= cMapCellWidth )
							{	// jedn� se o povolen� MapCell
								// aktalizuje informaci o povolen�m MapCellu
								rdwPositionDistance = INFINITE_DISTANCE - 1;
							}
							else
							{	// jedn� se o zak�zan� MapCell
								// aktalizuje informaci o zak�zan�m MapCellu
								rdwPositionDistance = DWORD_MAX;
							}
							// MapCell je nav�t�ven�
						}
						// jedn� se o nav�t�ven� MapCell
						ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
							( rdwPositionDistance == DWORD_MAX ) );

						// zjist�, je-li MapCell zak�zan�
						if ( rdwPositionDistance == DWORD_MAX )
						{	// MapCell je zak�zan�
							// p�id� kontroln� bod koncov�ho bodu cesty
							CSCheckPointPositionList::Insert ( posCheckPointPosition, pointEnd );
							// aktualizuje konec cesty
							pointEnd = pointLastPosition;
							// ukon�� kontrolu MapCell� bod� �se�ky
							break;
						}
						// MapCell je povolen�
					}
					// MapCell bodu �se�ky je povolen�
				}
				// druh� okraj MapCellu �se�ky je povolen�

				// posune se na dal�� bod �se�ky
				pointPosition.y += dwYDifference;
			}
			// MapCelly bod� �se�ky jsou zkontrolov�ny
		}
		else
		{	// vzd�lenosti konc� �se�ky jsou stejn� v ose X i Y
			// pozice bodu �se�ky
			CPointDW pointPosition = pointNextPosition;
			// p��r�stek v os�ch X a Y
			CPointDW pointDifference ( ( pointNextPosition.x < pointEnd.x ) ? 1 : (DWORD)-1, 
				( pointNextPosition.y < pointEnd.y ) ? 1 : (DWORD)-1 );

			ASSERT ( pointNextPosition != pointEnd );

			// posune se na dal�� MapCell
			pointPosition += pointDifference;

			// zkontroluje MapCelly bod� �se�ky
			while ( pointPosition != pointEnd )
			{
				// vzd�lenost bodu �se�ky
				DWORD &rdwPositionDistance = m_cMapCellDistance.GetAt ( pointPosition );

				// zjist�, je-li MapCell bodu �se�ky povolen�
				if ( rdwPositionDistance >= INFINITE_DISTANCE )
				{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
					// zjist�, jedn�-li se o nenav�t�ven� MapCell
					if ( rdwPositionDistance == INFINITE_DISTANCE )
					{	// jedn� se o nenav�t�ven� MapCell
						// zjist�, jedn�-li se o povolen� MapCell
						if ( GetMapCellAt ( pointPosition ) >= cMapCellWidth )
						{	// jedn� se o povolen� MapCell
							// aktalizuje informaci o povolen�m MapCellu
							rdwPositionDistance = INFINITE_DISTANCE - 1;
						}
						else
						{	// jedn� se o zak�zan� MapCell
							// aktalizuje informaci o zak�zan�m MapCellu
							rdwPositionDistance = DWORD_MAX;
						}
						// MapCell je nav�t�ven�
					}
					// jedn� se o nav�t�ven� MapCell
					ASSERT ( ( rdwPositionDistance < INFINITE_DISTANCE ) || 
						( rdwPositionDistance == DWORD_MAX ) );

					// zjist�, je-li MapCell zak�zan�
					if ( rdwPositionDistance == DWORD_MAX )
					{	// MapCell je zak�zan�
						// p�id� kontroln� bod koncov�ho bodu cesty
						CSCheckPointPositionList::Insert ( posCheckPointPosition, pointEnd );
						// aktualizuje konec cesty
						pointEnd = pointLastPosition;
						// ukon�� kontrolu MapCell� bod� �se�ky
						break;
					}
					// MapCell je povolen�
				}
				// MapCell bodu �se�ky je povolen�

				// posune se na dal�� MapCell
				pointPosition += pointDifference;
			}
			// MapCelly bod� �se�ky jsou zkontrolov�ny
		}
		// �se�ka je zpracov�na

		// aktualizuje posledn� zpracovan� bod cesty
		pointLastPosition = pointNextPosition;
		// aktualizuje vzd�lenost posledn�ho zpracovan�ho bodu cesty
		dwLastPositionDistance = dwNextPositionDistance;
	}
	// cesta byla p�evedena na kontroln� body

	// p�id� kontroln� bod koncov�ho bodu cesty
	CSCheckPointPositionList::Insert ( posCheckPointPosition, pointEnd );

	// zru�� ozna�en� pozic na map�
	m_cMapCellDistance.Clear ();
}

// *******************************************bFoundNearStart, CPointDW &
// vr�t� index nejbli��� br�ny mostu z pozice "pointStart" pro ���ku MapCellu 
//		"cMapCellWidth" (DWORD_MAX=neexistuje br�na mostu, DWORD_MAX-1="pointEnd" je bli���)
DWORD CSFindPathGraph::GetNearestBridgeGateIndex ( CPointDW &pointStart, 
	signed char cMapCellWidth, CPointDW pointEnd, BOOL bFoundNearStart ) 
{
	ASSERT ( ( cMapCellWidth > 0 ) && ( ( cMapCellWidth % 2 ) == 0 ) );

	// fronta pozic na map�
	static CSelfPooledQueue<CPointDW> cPositionQueue ( 200 );
	ASSERT ( cPositionQueue.IsEmpty () );

	// pozice na map�
	CPointDW pointPosition = pointStart;
	// pozice nalezen� br�ny mostu
	CPointDW pointBridgeGatePosition ( DWORD_MAX, DWORD_MAX );
	// hodnota MapCellu po��te�n� pozice
	signed char cStartMapCell = GetMapCellAt ( pointStart );

// **********************************************
//	// zjist�, za��n�-li se na spr�vn� ���ce MapCellu
//	if ( cStartMapCell < cMapCellWidth )
//	{	// za��n� se na �patn� ���ce MapCellu
//		// vr�t� p��znak nenalezen� cesty
//		return DWORD_MAX;
//	}
// **********************************************

// **********************************************
	// zjist�, za��n�-li se na spr�vn� ���ce MapCellu
	if ( cStartMapCell < cMapCellWidth )
	{	// za��n� se na �patn� ���ce MapCellu
// **********************************************
		if ( bFoundNearStart )
		{
			// fronta pozic
			static CSelfPooledQueue<CPointDW> cPositionQueue ( 50 );

			// aktualizuje vzd�lenost zak�zan�ho MapCellu
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

				// zpracuje sousedn� pozice na map�
				for ( int nIndex = 8; nIndex-- > 0; )
				{
					// sousedn� pozice na map�
					CPointDW pointSurroundingPosition = pointPosition + 
						m_aSurroundingPositionOffset[nIndex];

					// zjist�, jedn�-li se o pozici na map�
					if ( ( pointSurroundingPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
						( pointSurroundingPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
					{	// nejedn� se o pozici na map�
						// ukon�� zpracov�v�n� sousedn� pozice
						continue;
					}

					// vzd�lenost sousedn� pozice
					DWORD &rdwSurroundingPositionDistance = 
						m_cMapCellDistance.GetAt ( pointSurroundingPosition );

					// zpracuje sousedn� pozici
					if ( rdwSurroundingPositionDistance == INFINITE_DISTANCE )
					{	// jedn� se o dosud nenav�t�venou pozici
						// zjist�, jedn�-li se o povolen� MapCell
						if ( GetMapCellAt ( pointSurroundingPosition ) >= cMapCellWidth )
						{	// jedn� se o povolen� MapCell
							// aktualizuje c�lovou pozici
							pointEndNear = pointSurroundingPosition;
							// vypr�zdn� frontu
							cPositionQueue.RemoveAll ();
							break;
						}
						else
						{	// jedn� se o zak�zan� MapCell
							// aktualizuje vzd�lenost zak�zan�ho MapCellu
							rdwSurroundingPositionDistance = DWORD_MAX;
						}
						cPositionQueue.Add ( pointSurroundingPosition );
					}
					// jedn� se o zak�zan� MapCell
					ASSERT ( rdwSurroundingPositionDistance == DWORD_MAX );
				}
				// sousedn� pozice na map� byla zpracov�na
			}
			// fronta byla zpracov�na

			if ( pointEndNear == CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ) )
			{	// nebyla nalezena c�lov� pozice
				// zru�� ozna�en� pozic na map�
				m_cMapCellDistance.Clear ();

				return DWORD_MAX;
			}
			pointStart = pointEndNear;
			// aktualizuje hodnotu MapCellu po��te�n� pozice
			cStartMapCell = GetMapCellAt ( pointStart );
		}
		else
		{
			// zru�� ozna�en� pozic na map�
			m_cMapCellDistance.Clear ();

			// vr�t� p��znak nenalezen� cesty
			return DWORD_MAX;
		}
	}

	// ozna�� pozici na map�
	m_cMapCellDistance.GetAt ( pointStart ) = 0;

	// zjist�, jedn�-li se o br�nu mostu
	if ( !IsBridgeGateMapCell ( cStartMapCell ) )
	{	// nejedn� se o br�nu mostu
		// p�id� pozici na map� do fronty pozic
		cPositionQueue.Add ( pointStart );

		// najde nejbli��� br�nu mostu
		while ( cPositionQueue.RemoveFirst ( pointPosition ) )
		{
			ASSERT ( m_cMapCellDistance.GetAt ( pointPosition ) == 0 );
			ASSERT ( GetMapCellAt ( pointPosition ) >= cMapCellWidth );

			// projede okoln� pozice na map�
			for ( int nIndex = 8; nIndex-- > 0; )
			{
				// okoln� pozice na map�
				CPointDW pointSurroundingPosition = pointPosition + 
					m_aSurroundingPositionOffset[nIndex];

				// zjist�, jedn�-li se o pozici na map�
				if ( ( pointSurroundingPosition.x >= g_cMap.GetMapSizeMapCell ().cx ) || 
					( pointSurroundingPosition.y >= g_cMap.GetMapSizeMapCell ().cy ) )
				{	// nejedn� se o pozici na map�
					// ukon�� zpracov�v�n� okoln� pozice
					continue;
				}

				// vzd�lenost sousedn� pozice
				DWORD &rdwSurroundingPosition = m_cMapCellDistance.GetAt ( 
					pointSurroundingPosition );

				// zjist�, jedn�-li se o prozkoumanou pozici na map�
				if ( rdwSurroundingPosition == 0 )
				{	// jedn� se o prozkoumanou pozici na map�
					// ukon�� zpracov�v�n� okoln� pozice
					continue;
				}

				// hodnota MapCellu na okoln� pozici na map�
				signed char cSurroundingMapCell = GetMapCellAt ( pointSurroundingPosition );

				// zjist�, jedn�-li se o p��pustnou pozici na map�
				if ( cSurroundingMapCell >= cMapCellWidth )
				{	// jedn� se o p��pustnou pozici
					// zjist�, jedn�-li se o br�nu mostu
					if ( IsBridgeGateMapCell ( cSurroundingMapCell ) )
					{	// jedn� se o br�nu mostu
						// uschov� si pozici br�ny mostu
						pointBridgeGatePosition = pointSurroundingPosition;
						// zni�� frontu pozic na map�
						cPositionQueue.RemoveAll ();
						// ukon�� hled�n� nejbli��� br�ny mostu
						break;
					}
					// nejedn� se o br�nu mostu

					// p�id� pozici na map� do fronty
					cPositionQueue.Add ( pointSurroundingPosition );
					// ozna�� pozici na map�
					rdwSurroundingPosition = 0;
				}
				// nejedn� se o p��pustnou pozici na map�
			}
			// zpracoval okoln� pozice na map�
		}
		// nena�el br�nu mostu
	}
	else
	{	// jedn� se o br�nu mostu
		// aktualizuje pozici nalezen� br�ny
		pointBridgeGatePosition = pointStart;
	}

	// p��znak bl�zk� koncov� pozice
	BOOL bNearEndPosition = ( m_cMapCellDistance.GetValueAt ( pointEnd ) == 0 );

	// zru�� ozna�en� pozic na map�
	m_cMapCellDistance.Clear ();

	// zjist�, je-li koncov� pozice bl�zko po��te�n� pozici
	if ( bNearEndPosition )
	{	// koncov� pozice je bl�zko po��te�n� pozici
		// vr�t� p��znak bli��� koncov� pozice
		return DWORD_MAX - 1;
	}

	// zjist�, byla-li nalezena br�na mostu
	if ( pointBridgeGatePosition.x != DWORD_MAX )
	{	// byla nalezena br�na mostu
		ASSERT ( pointBridgeGatePosition.x != DWORD_MAX );

		// najde index br�ny mostu
		for ( struct SBridgeGate *pBridgeGate = m_aBridgeGateTable; ; pBridgeGate++ )
		{
			// zjist�, jedn�-li se o hledanou br�nu mostu
			if ( ( pBridgeGate->dwX == pointBridgeGatePosition.x ) && 
				( pBridgeGate->dwY == pointBridgeGatePosition.y ) )
			{	// jedn� se o hledanou br�nu mostu
				// vr�t� index br�ny mostu
				return ((DWORD)pBridgeGate - (DWORD)m_aBridgeGateTable) / sizeof ( SBridgeGate );
			}
		}
		// nebyl nalezen index br�ny
	}
	else
	{	// nebyla nalezena br�na mostu
		ASSERT ( pointBridgeGatePosition.y == DWORD_MAX );
	}

	// vr�t� p��znak nenalezen� br�ny mostu
	return DWORD_MAX;
}

// *****************************, CPointDW &pointSecondBridgeGate, CPointDW &pointPreLastBridgeGate
// najde nejkrat�� cestu od br�ny mostu "dwStartBridgeGateIndex" k br�n� mostu 
//		"dwEndBridgeGateIndex" ���ky "dwWidth", vypln� seznam kontroln�ch bod� cesty 
//		"pCheckPointPositionList", vrac� po�et bran most� cesty
DWORD CSFindPathGraph::FindBridgeGatePath ( DWORD dwStartBridgeGateIndex, 
	DWORD dwEndBridgeGateIndex, DWORD dwWidth, 
	CSCheckPointPositionList *pCheckPointPositionList, CPointDW &pointSecondBridgeGate, 
	CPointDW &pointPreLastBridgeGate ) 
{
	ASSERT ( dwStartBridgeGateIndex != dwEndBridgeGateIndex );

	// vynuluje vzd�lenosti bran most�
	ClearBridgeGatesDistances ();

	// inicializuje vzd�lenost v�choz� br�ny
	m_aBridgeGateDistance[dwStartBridgeGateIndex].dwDistance = 0;

	ASSERT ( m_cBridgeGateList.IsEmpty () );

	// p�id� do seznamu bran most� index prvn�ho mostu
	m_cBridgeGateList.Add ( dwStartBridgeGateIndex );

	// index nejbli��� br�ny mostu
	DWORD dwBridgeGateIndex;

	// najde nejkrat�� cestu v grafu
	while ( m_cBridgeGateList.RemoveFirst ( dwBridgeGateIndex ) )
	{
		// zjist� ukazatel na br�nu mostu
		struct SBridgeGate *pBridgeGate = m_aBridgeGateTable + dwBridgeGateIndex;

		// zjist�, jedn�-li se o c�lovou br�nu
		if ( dwBridgeGateIndex == dwEndBridgeGateIndex )
		{	// jedn� se o c�lovou br�nu
			// zni�� seznam bran most�
			m_cBridgeGateList.RemoveAll ();

			// po�et bran most� nalezen� cesty
			DWORD dwBridgeGateCount = 1;

			// *****************************
			// inicializuje pozici druh� br�ny mostu
			pointSecondBridgeGate = CPointDW ( m_aBridgeGateTable[dwEndBridgeGateIndex].dwX, 
				m_aBridgeGateTable[dwEndBridgeGateIndex].dwY );
			// inicializuje pozici p�edposledn� br�ny mostu
			pointPreLastBridgeGate = CPointDW ( 
				m_aBridgeGateTable[dwStartBridgeGateIndex].dwX, 
				m_aBridgeGateTable[dwStartBridgeGateIndex].dwY );

			// vytvo�� seznam kontroln�ch bod� cesty (od konce)
			while ( dwEndBridgeGateIndex != dwStartBridgeGateIndex )
			{
				// aktualizuje po�et bran most�
				dwBridgeGateCount++;

				// ukazatel na br�nu mostu
				struct SBridgeGate *pEndBridgeGate = pBridgeGate;
				// aktualizuje index p�echoz� br�ny mostu
				dwBridgeGateIndex = 
					m_aBridgeGateDistance[dwEndBridgeGateIndex].dwPreviousBridgeGateIndex;
				// aktualizuje ukazatel na p�edchoz� br�nu mostu
				pBridgeGate = m_aBridgeGateTable + dwBridgeGateIndex;

				// *****************************
				// aktualizuje pozici druh� br�ny mostu
				pointSecondBridgeGate = CPointDW ( pEndBridgeGate->dwX, pEndBridgeGate->dwY );
				// zjist�, jedn�-li se o p�edposledn� br�nu mostu
				if ( dwBridgeGateCount == 3 )
				{	// jedn� se o p�edposledn� br�nu mostu
					// aktualizuje pozici p�edposledn� br�ny mostu
					pointPreLastBridgeGate = CPointDW ( pEndBridgeGate->dwX, 
						pEndBridgeGate->dwY );
				}

				// p�id� kontroln� bod br�ny mostu
				pCheckPointPositionList->Add ( CPointDW ( pEndBridgeGate->dwX, 
					pEndBridgeGate->dwY ) );

				// p�id� kontroln� body hintu cesty
				for ( struct SArea *pArea = pEndBridgeGate->pBridgeArea; ; )
				{
					// zjist�, je-li oblast br�ny mostu spole�nou oblast� bran most�
					if ( ( ( pArea == pBridgeGate->pBridgeArea ) || 
						( pArea == pBridgeGate->pArea ) ) && ( pArea->dwWidth >= dwWidth ) )
					{	// oblast br�ny mostu je spole�nou oblast� bran most�
						// index prvn� br�ny mostu
						DWORD dwFirstIndex = DWORD_MAX;
						// index druh� br�ny mostu
						DWORD dwSecondIndex;

						// zjist� ukazatel na br�ny most� oblasti
						struct SBridgeGate **pAreaBridgeGate = 
							(struct SBridgeGate **)( pArea + 1 );

						// zjist� indexy bran most� v oblasti
						for ( ; ; pAreaBridgeGate++ )
						{
							// zjist�, jedn�-li se o hledanou br�nu mostu
							if ( ( *pAreaBridgeGate == pBridgeGate ) || 
								( *pAreaBridgeGate == pEndBridgeGate ) )
							{	// jedn� se o hledanou br�nu mostu
								// zjist�, jedn�-li se o druhou nalezenou br�nu mostu
								if ( dwFirstIndex != DWORD_MAX )
								{	// jedn� se o druhou nalezenou br�nu mostu
									// aktualizuje index druh� br�ny mostu
									dwSecondIndex = pAreaBridgeGate - 
										(struct SBridgeGate **)( pArea + 1 ); 
									// ukon�� hled�n� bran most�
									break;
								}
								// jedn� se o prvn� nalezenou br�nu mostu
								dwFirstIndex = pAreaBridgeGate - 
									(struct SBridgeGate **)( pArea + 1 );
							}
							// nejden� se o hledanou br�nu mostu
						}
						// indexy bran most� v oblasti jsou zji�t�ny
						ASSERT ( dwSecondIndex < pArea->dwBridgeGateCount );

						// cesta mezi branami most�
						struct SBridgeGatePath *pBridgeGatePath = 
							(struct SBridgeGatePath *)( (struct SBridgeGate **)( pArea + 1 ) + 
							pArea->dwBridgeGateCount ) + dwFirstIndex * ( 2 * 
							pArea->dwBridgeGateCount - dwFirstIndex - 1 ) / 2 + dwSecondIndex - 
							dwFirstIndex - 1;

						// zjist�, jedn�-li se o pou�itou cestu mezi branami
						if ( ( m_aBridgeGateDistance[dwBridgeGateIndex].dwDistance + 
							pBridgeGatePath->dwBridgeGateDistance ) == 
							m_aBridgeGateDistance[dwEndBridgeGateIndex].dwDistance )
						{	// jedn� se o pou�itou cestu
							// zjist� ukazatel na hint cesty
							struct SPathHint *pPathHint = pBridgeGatePath->pPathHint;

							// zjist�, je-li p�edchoz� br�na mostu prvn�
							if ( *pAreaBridgeGate == pBridgeGate )
							{	// p�edchoz� br�na mostu je druh�
								// zjist� ukazatel na kontroln� body hintu cesty
								struct SPathHintCheckPoint *pPathHintCheckPoint = 
									(struct SPathHintCheckPoint *)( pPathHint + 1 );

								// p�id� kontroln� body hintu cesty
								for ( DWORD dwIndex = pPathHint->dwCheckPointCount; 
									dwIndex-- > 0; pPathHintCheckPoint++ )
								{
									// p�id� dal�� kontroln� bod hintu cesty
									pCheckPointPositionList->Add ( CPointDW ( 
										pPathHintCheckPoint->dwX, pPathHintCheckPoint->dwY ) );
								}
								// kontroln� body hintu cesty jsou p�id�ny
							}
							else
							{	// p�edchoz� br�na mostu je prvn�
								// zjist� ukazatel na kontroln� body hintu cesty
								struct SPathHintCheckPoint *pPathHintCheckPoint = 
									(struct SPathHintCheckPoint *)( pPathHint + 1 ) + 
									pPathHint->dwCheckPointCount - 1;

								// p�id� kontroln� body hintu cesty
								for ( DWORD dwIndex = pPathHint->dwCheckPointCount; 
									dwIndex-- > 0; pPathHintCheckPoint-- )
								{
									// p�id� dal�� kontroln� bod hintu cesty
									pCheckPointPositionList->Add ( CPointDW ( 
										pPathHintCheckPoint->dwX, pPathHintCheckPoint->dwY ) );
								}
								// kontroln� body hintu cesty jsou p�id�ny
							}
							// kontroln� body hintu cesty jsou p�id�ny
							break;
						}
						// nejedn� se o pou�itou cestu mezi branami
					}
					// oblast br�ny mostu nen� po�adovan� spole�n� oblast bran most�

					ASSERT ( pArea != pEndBridgeGate->pArea );
					// aktualizuje ukazatel na oblast br�ny mostu
					pArea = pEndBridgeGate->pArea;
					ASSERT ( ( ( pArea == pBridgeGate->pBridgeArea ) || 
						( pArea == pBridgeGate->pArea ) ) && ( pArea->dwWidth >= dwWidth ) );
				}
				// kontroln� body hintu cesty byly p�id�ny

				// posune se na p�edchoz� br�nu mostu
				dwEndBridgeGateIndex = dwBridgeGateIndex;
			}
			// seznam kontroln�ch bod� cesty je vytvo�en

			// vr�t� po�et bran most� cesty
			return dwBridgeGateCount;
		}
		// nejedn� se o c�lovou br�nu

		// zjist�, je-li ���ka oblasti dostate�n�
		if ( pBridgeGate->pBridgeArea->dwWidth >= dwWidth )
		{	// ���ka oblasti je dostate�n�
			// aktualizuje br�ny most� p�es prvn� oblast
			UpdateBridgeGateAreaDistances ( dwBridgeGateIndex, pBridgeGate->pBridgeArea );
		}
		// zjist�, jedn�-li se o stejn� oblasti
		if ( pBridgeGate->pBridgeArea != pBridgeGate->pArea )
		{	// nejedn� se o stejn� oblasti
			// zjist�, je-li ���ka oblasti dostate�n�
			if ( pBridgeGate->pArea->dwWidth >= dwWidth )
			{	// ���ka oblasti je dostate�n�
				// aktualizuje br�ny most� p�es druhou oblast
				UpdateBridgeGateAreaDistances ( dwBridgeGateIndex, pBridgeGate->pArea );
			}
		}
		// jedn� se o stejn� oblasti
	}
	// neexistuje cesta v grafu

	// vr�t� cestu d�lky 0
	return 0;
}

// vynuluje vzd�lenosti bran most� (pomocn� metoda metody "FindBridgeGatePath")
void CSFindPathGraph::ClearBridgeGatesDistances () 
{
	// ukazatel na vzd�lenost br�ny mostu
	struct SBridgeGateDistance *pBridgeGateDistance = m_aBridgeGateDistance;

	// vynuluje vzd�lenosti bran most�
	for ( DWORD dwIndex = m_dwMaxBridgeGateCount; dwIndex-- > 0; pBridgeGateDistance++ )
	{
		// vynuluje vzd�lenost br�ny mostu
		pBridgeGateDistance->dwDistance = INFINITE_DISTANCE;
		pBridgeGateDistance->dwPreviousBridgeGateIndex = DWORD_MAX;
	}
}

// aktualizuje vzd�lenosti bran most� oblasti "pArea" od br�ny mostu "dwBridgeGateIndex" 
//		(pomocn� metoda metody "FindBridgeGatePath")
void CSFindPathGraph::UpdateBridgeGateAreaDistances ( DWORD dwBridgeGateIndex, 
	struct SArea *pArea ) 
{
	// vzd�lenost br�ny mostu
	DWORD dwBridgeGateDistance = m_aBridgeGateDistance[dwBridgeGateIndex].dwDistance;
	ASSERT ( dwBridgeGateDistance < INFINITE_DISTANCE );
	// ukazatel na br�nu mostu
	struct SBridgeGate *pBridgeGate = m_aBridgeGateTable + dwBridgeGateIndex;

	// zjist� ukazatel na br�ny most� oblasti
	struct SBridgeGate **pAreaBridgeGate = (struct SBridgeGate **)( pArea + 1 );

	// zjist� index br�ny mostu v oblasti
	for ( ; ; pAreaBridgeGate++ )
	{
		// zjist�, jedn�-li se o hledanou br�nu mostu
		if ( *pAreaBridgeGate == pBridgeGate )
		{	// jedn� se o hledanou br�nu mostu
			// ukon�� hled�n� indexu br�ny mostu v oblasti
			break;
		}
	}

	// zjist� index nalezen� br�ny mostu v bran�ch most� oblasti
	DWORD dwIndex = pAreaBridgeGate - (struct SBridgeGate **)( pArea + 1 );
	ASSERT ( dwIndex < pArea->dwBridgeGateCount );

	// aktualizuje ukazatel na prvn� br�nu mostu oblasti
	pAreaBridgeGate = (struct SBridgeGate **)( pArea + 1 );
	// ukazatel na cesty mezi branami most� oblasti
	struct SBridgeGatePath *pBridgeGatePath = 
		(struct SBridgeGatePath *)( pAreaBridgeGate + pArea->dwBridgeGateCount );

	// zpracuje po��te�n� br�ny most� oblasti
	for ( DWORD dwFirstIndex = 0; dwFirstIndex < pArea->dwBridgeGateCount; dwFirstIndex++ )
	{
		// zpracuje koncov� br�ny most� oblasti
		for ( DWORD dwSecondIndex = dwFirstIndex + 1; 
			dwSecondIndex < pArea->dwBridgeGateCount; dwSecondIndex++, pBridgeGatePath++ )
		{
			// zjist�, jedn�-li se o p�echod s aktu�ln� br�nou mostu
			if ( ( dwFirstIndex == dwIndex ) || ( dwSecondIndex == dwIndex ) )
			{	// jedn� se o p�echod s pot�ebnou branou mostu
				// ukazatel na jinou br�nu mostu
				struct SBridgeGate *pOtherBridgeGate = pAreaBridgeGate[( dwFirstIndex == 
					dwIndex ) ? dwSecondIndex : dwFirstIndex];
				// index jin� br�ny mostu
				DWORD dwOtherBridgeGateIndex = pOtherBridgeGate - m_aBridgeGateTable;

				// zjist�, je-li p�echod zkr�cen�m cesty
				if ( ( dwBridgeGateDistance + pBridgeGatePath->dwBridgeGateDistance ) < 
					m_aBridgeGateDistance[dwOtherBridgeGateIndex].dwDistance )
				{	// p�echod je zkr�cen�m cesty
					// aktualizuje d�lku cesty do br�ny mostu
					DWORD dwOtherBridgeGateDistance = 
						m_aBridgeGateDistance[dwOtherBridgeGateIndex].dwDistance = 
						dwBridgeGateDistance + pBridgeGatePath->dwBridgeGateDistance;
					m_aBridgeGateDistance[dwOtherBridgeGateIndex].dwPreviousBridgeGateIndex = 
						dwBridgeGateIndex;
					// najde br�nu mostu ze seznamu
					POSITION posBridgeGate = m_cBridgeGateList.Find ( dwOtherBridgeGateIndex );
					// zjist�, byla-li br�na mostu v seznamu
					if ( posBridgeGate != NULL )
					{	// br�na mostu je v seznamu
						// vyjme br�nu mostu ze seznamu
						m_cBridgeGateList.RemovePosition ( posBridgeGate );
					}

				// p�id� br�nu mostu do seznamu na spr�vn� m�sto

					// pozice br�ny mostu v seznamu bran most�
					posBridgeGate = m_cBridgeGateList.GetHeadPosition ();
					// pozice p�edchoz� br�ny mostu v seznamu bran most�
					POSITION posLastBridgeGate = posBridgeGate;
					// index dal�� br�ny mostu
					DWORD dwNextBridgeGateIndex;

					// projede seznam bran most�
					while ( m_cBridgeGateList.GetNext ( posBridgeGate, 
						dwNextBridgeGateIndex ) )
					{
						// zjist�, m�-li se p�id�van� br�na mostu um�stit
						if ( m_aBridgeGateDistance[dwNextBridgeGateIndex].dwDistance >= 
							dwOtherBridgeGateDistance )
						{	// p�id�van� br�na mostu se m� um�stit
							// aktualizuje pozici um�st�n� p�id�van� br�ny mostu
							posBridgeGate = posLastBridgeGate;
							// nech� um�stit p�id�vanou br�nu mostu do seznamu bran most�
							break;
						}
						// aktualizuje pozici posledn� br�ny mostu
						posLastBridgeGate = posBridgeGate;
					}
					// um�st� p�id�vanou br�nu mostu do seznamu bran most�
					m_cBridgeGateList.Insert ( posLastBridgeGate, dwOtherBridgeGateIndex );
				}
				// p�echod nen� zkr�cen�m cesty
			}
			// nejedn� se o p�echod s aktu�ln� br�nou mostu
		}
		// zpracoval koncov� br�ny most� oblasti
	}
	// zpracoval po��te�n� br�ny most� oblasti
}

// vr�t� hodnotu MapCellu na pozici "pointMapCellPosition"
signed char CSFindPathGraph::GetMapCellAt ( CPointDW pointMapCellPosition ) 
{
	ASSERT ( ( pointMapCellPosition.x < g_cMap.GetMapSizeMapCell ().cx ) && 
		( pointMapCellPosition.y < g_cMap.GetMapSizeMapCell ().cy ) );

	// vr�t� hodnotu MapCellu na pozici "pointMapCellPosition"
	return m_pMap[pointMapCellPosition.x + pointMapCellPosition.y * 
		g_cMap.GetMapSizeMapCell ().cx];
}
