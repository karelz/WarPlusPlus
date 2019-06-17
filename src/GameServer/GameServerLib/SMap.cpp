/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída mapy na serveru hry
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SMap.h"

#include "SSkillType.h"
#include "SPath.h"
#include "SFindPathGraph.h"
#include "SPosition.h"
#include "SNotifications.h"

#include "GameServer\Civilization\Src\ClientCommunication\ZClientUnitInfoSender.h"
#include "Common\Map\Map.h"
#include "SFileManager.h"

#include "SUnit.inl"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// zkontroluje šíøku a výšku MapSquaru
#if ( MAPSQUARE_WIDTH != MAPSQUARE_HEIGHT ) || ( MAPSQUARE_WIDTH != MAP_SQUARE_SIZE )
#error Incompatible MapSquare dimensions! (must be: MAPSQUARE_WIDTH = MAPSQUARE_HEIGHT = MAP_SQUARE_SIZE)
#endif

//////////////////////////////////////////////////////////////////////
// debuggovací a trasovací makra
#ifdef _DEBUG
	extern BOOL g_bTraceServerLoops;
	extern BOOL g_bTraceServerClientCommunication;

	#define TRACE_LOOP if ( g_bTraceServerLoops ) TRACE_NEXT 
	#define TRACE_LOOP0(text) do { if ( g_bTraceServerLoops ) { TRACE0_NEXT(text); } } while ( 0 )
	#define TRACE_LOOP1(text,p1) do { if ( g_bTraceServerLoops ) { TRACE1_NEXT(text,p1); } } while ( 0 )
	#define TRACE_LOOP2(text,p1,p2) do { if ( g_bTraceServerLoops ) { TRACE2_NEXT(text,p1,p2); } } while ( 0 )
	#define TRACE_LOOP3(text,p1,p2,p3) do { if ( g_bTraceServerLoops ) { TRACE3_NEXT(text,p1,p2,p3); } } while ( 0 )

	#define TRACE_COMMUNICATION if ( g_bTraceServerClientCommunication ) TRACE_NEXT 
	#define TRACE_COMMUNICATION0(text) do { if ( g_bTraceServerClientCommunication ) { TRACE0_NEXT(text); } } while ( 0 )
	#define TRACE_COMMUNICATION1(text,p1) do { if ( g_bTraceServerClientCommunication ) { TRACE1_NEXT(text,p1); } } while ( 0 )
	#define TRACE_COMMUNICATION2(text,p1,p2) do { if ( g_bTraceServerClientCommunication ) { TRACE2_NEXT(text,p1,p2); } } while ( 0 )
	#define TRACE_COMMUNICATION3(text,p1,p2,p3) do { if ( g_bTraceServerClientCommunication ) { TRACE3_NEXT(text,p1,p2,p3); } } while ( 0 )
#else //!_DEBUG
	#define TRACE_LOOP TRACE
	#define TRACE_LOOP0 TRACE0
	#define TRACE_LOOP1 TRACE1
	#define TRACE_LOOP2 TRACE2
	#define TRACE_LOOP3 TRACE3

	#define TRACE_COMMUNICATION TRACE
	#define TRACE_COMMUNICATION0 TRACE0
	#define TRACE_COMMUNICATION1 TRACE1
	#define TRACE_COMMUNICATION2 TRACE2
	#define TRACE_COMMUNICATION3 TRACE3
#endif //!_DEBUG

IMPLEMENT_DYNAMIC ( CSMap, CPersistentObject )

//////////////////////////////////////////////////////////////////////
// Makra a konstanty
//////////////////////////////////////////////////////////////////////

// tolerance polomìru viditelnosti jednotky
#define UNIT_VIEW_RADIUS_TOLERANCE(dwViewRadiusSquare) ( 2 * dwViewRadiusSquare + 9 )

// poèet TimeSlicù mezi mazáním mrtvých jednotek
#define IsDeletingDeadUnitsTimeSlice(dwTimeSlice)		( dwTimeSlice & 0x07 )

//////////////////////////////////////////////////////////////////////
// Globální promìnné
//////////////////////////////////////////////////////////////////////

// Mapa hry
CSMap g_cMap;

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSMap::CSMap () : m_eventPauseGame ( FALSE, TRUE ), m_eventUnpauseGame ( TRUE, TRUE ), 
	m_cDeadUnitQueue ( 20 ), m_cDeadUnitClientStopInfoList ( 20 ), 
	m_cDeletedUnitList ( 50 ), m_cGetNearestUnitPlacePositionArray ( 4 ) 
{
	// zneškodní data mapy
	m_sizeMapMapSquare = CSizeDW ( 0, 0 );
	m_sizeMapMapCell = CSizeDW ( 0, 0 );
	m_pointCornerMapSquare = CPointDW ( 0, 0 );
	m_pointCornerMapCell = CPointDW ( 0, 0 );
	m_pMapSquares = NULL;
	m_strMapArchiveName.Empty ();
	m_dwMapFormatVersion = 0;
	m_dwMapCompatibleFormatVersion = DWORD_MAX;
	m_strMapName.Empty ();
	m_strMapDescription.Empty ();
	m_dwMapVersion = DWORD_MAX;

	m_dwCivilizationCount = 0;
	m_pCivilizations = NULL;

	m_pFirstUnitBlock = NULL;
	m_pLastUnitBlock = NULL;
	m_pFirstFreeUnit = NULL;

	m_dwTimeSliceLength = 0;
	m_dwTimeSlice = 0;

	m_pMainLoopThread = NULL;
	m_pClientInfoSenderLoopThread = NULL;
	m_pFindPathLoopThread = NULL;
	m_bInitialized = FALSE;

	m_pPreparingDPC = NULL;

	// zakáže logování uživatelù
	m_bUserLoginAllowed = FALSE;

	m_pFirstMapObserver = NULL;

#ifdef _DEBUG
	m_dwMainLoopThreadID = DWORD_MAX;
#endif //_DEBUG

	ASSERT ( CheckEmptyData () );
}

// destruktor
CSMap::~CSMap () 
{
	ASSERT ( CheckEmptyData () );
}

//////////////////////////////////////////////////////////////////////
// Inicializace a znièení dat objektu
//////////////////////////////////////////////////////////////////////

// inicializuje data mapy z archivu mapy "cMapArchive"
void CSMap::Create ( CDataArchive cMapArchive ) 
{
	ASSERT ( CheckEmptyData () );
	ASSERT ( !cMapArchive.GetArchivePath ().IsEmpty () );

	// vytvoøit statické vìci
	CSOneInstance::Create ();

	// aktualizuje jméno archivu mapy
	m_strMapArchiveName = cMapArchive.GetArchivePath ();

	// pole archivù knihoven typù jednotek
	CDataArchivePtrArray cUnitTypeLibraryArchiveArray;

	try
	{
	// naète data mapy ze souboru mapy

		// otevøe soubor mapy
		CArchiveFile cMapFile = cMapArchive.CreateFile ( MAP_FILE_NAME, CFile::modeRead | 
			CFile::shareDenyWrite );

		// poèet knihoven Mapexù
		DWORD dwMapexLibraryCount;
		// poèet knihoven jednotek
		DWORD dwUnitTypeLibraryCount;
		// poèet jednotek na mapì
		DWORD dwUnitCount;
		// poèet typù jednotek v knihovnách typù jednotek
		DWORD dwUnitTypeCount = 0;
		// poèet ScriptSetù
		DWORD dwScriptSetCount;
		// poèet grafù pro hledání cest
		DWORD dwFindPathGraphCount;

	// naète hlavièku verze souboru
		{
			SFileVersionHeader sFileVersionHeader;
			LOAD_ASSERT ( cMapFile.Read ( &sFileVersionHeader, sizeof ( sFileVersionHeader ) ) == sizeof ( sFileVersionHeader ) );

			// identifikátor souboru mapy
			BYTE aMapFileID[16] = MAP_FILE_IDENTIFIER;
			ASSERT ( sizeof ( aMapFileID ) == sizeof ( sFileVersionHeader.m_aMapFileID ) );

			// zkontroluje identifikátor souboru mapy
			for ( int nIndex = 16; nIndex-- > 0; )
			{
				LOAD_ASSERT ( aMapFileID[nIndex] == sFileVersionHeader.m_aMapFileID[nIndex] );
			}

			ASSERT ( CURRENT_MAP_FILE_VERSION >= COMPATIBLE_MAP_FILE_VERSION );
			// naète èísla verzí formátu mapy
			m_dwMapFormatVersion = sFileVersionHeader.m_dwFormatVersion;
			m_dwMapCompatibleFormatVersion = sFileVersionHeader.m_dwCompatibleFormatVersion;
			LOAD_ASSERT ( m_dwMapFormatVersion >= m_dwMapCompatibleFormatVersion );

			// zjistí, jedná-li se o správnou verzi formátu mapy
			if ( m_dwMapFormatVersion != CURRENT_MAP_FILE_VERSION )
			{	// nejedná se o správnou verzi formátu mapy
				// zjistí, jedná-li se o starou verzi formátu mapy
				if ( m_dwMapFormatVersion < CURRENT_MAP_FILE_VERSION )
				{	// jedná se o starou verzi formátu mapy
					// zkontroluje kompatabilitu verze formátu mapy
					LOAD_ASSERT ( m_dwMapFormatVersion >= COMPATIBLE_MAP_FILE_VERSION );
				}
				else
				{	// jedná se o mladší verzi formátu mapy
					// zkontroluje kompatabilitu verze formátu mapy
					LOAD_ASSERT ( m_dwMapCompatibleFormatVersion <= CURRENT_MAP_FILE_VERSION );
				}
			}
		}

	// naète hlavièku mapy
		{
			SMapHeader sMapHeader;
			LOAD_ASSERT ( cMapFile.Read ( &sMapHeader, sizeof ( sMapHeader ) ) == sizeof ( sMapHeader ) );

		// zpracuje hlavièku mapy

			// naète velikost mapy
			m_sizeMapMapCell.cx = sMapHeader.m_dwWidth;
			LOAD_ASSERT ( sMapHeader.m_dwWidth % MAP_SQUARE_SIZE == 0 );
			m_sizeMapMapSquare.cx = sMapHeader.m_dwWidth / MAP_SQUARE_SIZE;
			m_sizeMapMapCell.cy = sMapHeader.m_dwHeight;
			LOAD_ASSERT ( sMapHeader.m_dwHeight % MAP_SQUARE_SIZE == 0 );
			m_sizeMapMapSquare.cy = sMapHeader.m_dwHeight / MAP_SQUARE_SIZE;

			// zkontroluje velikost mapy
			LOAD_ASSERT ( m_sizeMapMapSquare.cx >= 2 );
			LOAD_ASSERT ( m_sizeMapMapSquare.cy >= 2 );
			LOAD_ASSERT ( m_sizeMapMapCell.cx < m_sizeMapMapCell.cx * m_sizeMapMapCell.cy );
			LOAD_ASSERT ( m_sizeMapMapCell.cy < m_sizeMapMapCell.cx * m_sizeMapMapCell.cy );
			LOAD_ASSERT ( m_sizeMapMapCell.cx * m_sizeMapMapCell.cy < m_sizeMapMapCell.cx * 
				m_sizeMapMapCell.cy * sizeof ( DWORD ) );

			// vytvoøí pole MapSquarù
			m_pMapSquares = new CSMapSquare[m_sizeMapMapSquare.cx * m_sizeMapMapSquare.cy];

			// zkontroluje délku jména mapy
			m_strMapName = sMapHeader.m_pName;
			LOAD_ASSERT ( m_strMapName.GetLength () < sizeof ( sMapHeader.m_pName ) );
			// zkontroluje délku popisu mapy
			m_strMapDescription = sMapHeader.m_pDescription;
			LOAD_ASSERT ( m_strMapDescription.GetLength () < sizeof ( sMapHeader.m_pDescription ) );

			// zkontroluje poèet knihoven Mapexù
			dwMapexLibraryCount = sMapHeader.m_dwMapexLibraryCount;
			LOAD_ASSERT ( dwMapexLibraryCount > 0 );

			// zkontroluje poèet civilizací
			m_dwCivilizationCount = sMapHeader.m_dwCivilizationsCount;
			LOAD_ASSERT ( m_dwCivilizationCount > 0 );
			LOAD_ASSERT ( m_dwCivilizationCount <= CIVILIZATION_COUNT_MAX );

			// vytvoøí pole civilizací
			m_pCivilizations = new CSCivilization[m_dwCivilizationCount];

			// zkontroluje poèet knihoven typù jednotek
			dwUnitTypeLibraryCount = sMapHeader.m_dwUnitTypeLibraryCount;
			LOAD_ASSERT ( dwUnitTypeLibraryCount > 0 );

			// zjistí poèet jednotek na mapì
			dwUnitCount = sMapHeader.m_dwUnitCount;

			// zjistí verzi mapy
			m_dwMapVersion = sMapHeader.m_dwMapVersion;

			// zjistí poèet ScriptSetù
			dwScriptSetCount = sMapHeader.m_dwScriptSetCount;
			LOAD_ASSERT ( dwScriptSetCount > 0 );

			// zjistí poèet grafù pro hledání cest
			dwFindPathGraphCount = sMapHeader.m_dwFindPathGraphCount;
		}

		// tabulka pøekladu ID na jméno ScriptSetu
		CMap<DWORD, DWORD, CString, CString &> cScriptSetTable;

	// nahraje hlavièky ScriptSetù
		{
			SScriptSetHeader sScriptSetHeader;

			for ( DWORD dwIndex = dwScriptSetCount; dwIndex-- > 0; )
			{
				// pokusí se naèíst hlavièku ScriptSetu
				LOAD_ASSERT ( cMapFile.Read ( &sScriptSetHeader, sizeof ( sScriptSetHeader ) ) == sizeof ( sScriptSetHeader ) );

				// zkontroluje délku jména ScriptSetu
				int nScriptSetNameLength = strlen ( sScriptSetHeader.m_pFileName );
				LOAD_ASSERT ( ( nScriptSetNameLength > 0 ) && ( nScriptSetNameLength < 
					sizeof ( sScriptSetHeader.m_pFileName ) ) );

				// jméno nalezeného ScriptSetu
				CString strFoundScriptSetName;
				// ID nalezeného ScriptSetu
				DWORD dwFoundScriptSetID;
				// pozice ScriptSetu v tabulce ScriptSetù
				POSITION posScriptSet = cScriptSetTable.GetStartPosition ();
				// jméno ScriptSetu
				CString strScriptSetName ( sScriptSetHeader.m_pFileName );

				// zkontroluje unikátnost ID a jména ScriptSetu
				while ( posScriptSet != NULL )
				{
					// zjistí informace o dalším ScriptSetu v tabulce
					cScriptSetTable.GetNextAssoc ( posScriptSet, dwFoundScriptSetID, 
						strFoundScriptSetName );

					// zkontroluje unikátnost ID a jména ScriptSetu
					LOAD_ASSERT ( ( strFoundScriptSetName.CompareNoCase ( 
						strScriptSetName ) != 0 ) && sScriptSetHeader.m_dwID != 
						dwFoundScriptSetID );
				}

				// pøidá záznam o ScriptSetu do tabulky ScriptSetù
				cScriptSetTable.SetAt ( sScriptSetHeader.m_dwID, strScriptSetName );

				// nechá nahrát ScriptSet
				g_cSFileManager.LoadScriptSet ( cMapArchive, strScriptSetName );
			}
		}

	// pøeskoèí odkazy na knihovny Mapexù
		{
			// zjistí velikost odkazù na knihovny Mapexù
			DWORD dwMapexLibraryNodeSize = dwMapexLibraryCount * 
				sizeof ( SMapexLibraryNode );

			// pøeskoèí odkazy na knihovny Mapexù
			LOAD_ASSERT ( cMapFile.GetPosition () + dwMapexLibraryNodeSize <= 
				cMapFile.GetLength () );
			(void)cMapFile.Seek ( dwMapexLibraryNodeSize, CFile::current );
		}

		// tabulka pøekladu ID na index civilizace
		CMap<DWORD, DWORD, DWORD, DWORD> cCivilizationTable;

	// naète hlavièky civilizací
		{
			// zjistí ukazatel na první civilizaci z mapy
			CSCivilization *pCivilization = m_pCivilizations;

			// projede všechny civilizace z mapy
			for ( DWORD dwCivilizationIndex = 0; dwCivilizationIndex < m_dwCivilizationCount; 
				dwCivilizationIndex++, pCivilization++ )
			{
				// naète další civilizaci
				pCivilization->Create ( cMapFile, (BYTE)dwCivilizationIndex, 
					cScriptSetTable );

				// zjistí, jedná-li se o systémovou civilizaci
				if ( dwCivilizationIndex == 0 )
				{	// jedná se o systémovou civilizaci
					// zkontroluje naètení systémové civilizace
					LOAD_ASSERT ( pCivilization->GetCivilizationID () == 0 );
				}

				// zkontroluje unikátnost jména a ID civilizace
				for ( DWORD dwIndex = 0; dwIndex < dwCivilizationIndex; dwIndex++ )
				{
					// zkontroluje unikátnost jména civilizace
					LOAD_ASSERT ( pCivilization->GetCivilizationName () != m_pCivilizations[dwIndex].GetCivilizationName () );
					// zkontroluje unikátnost ID civilizace
					LOAD_ASSERT ( pCivilization->GetCivilizationID () != m_pCivilizations[dwIndex].GetCivilizationID () );
				}

			// zaznamená pøevod ID na index civilizace

				// nalezený index civilizace
				DWORD dwFoundCivilizationIndex;
				// zkontroluje unikátnost ID civilizace
				LOAD_ASSERT ( !cCivilizationTable.Lookup ( 
					pCivilization->GetCivilizationID (), dwFoundCivilizationIndex ) );
				// pøidá záznam do tabulky pøekladu ID na index civilizace
				cCivilizationTable.SetAt ( pCivilization->GetCivilizationID (), 
					dwCivilizationIndex );
			}
		}

	// naète odkazy na knihovny typù jednotek
		{
			SUnitTypeLibraryNode sUnitTypeLibraryNode;

			// inicializuje poèet typù jednotek v knihovnách typù jednotek
			dwUnitTypeCount = 0;

			for ( DWORD dwUnitTypeLibraryIndex = dwUnitTypeLibraryCount; 
				dwUnitTypeLibraryIndex-- > 0; )
			{
				// naète odkaz na knihovnu typù jednotek
				LOAD_ASSERT ( cMapFile.Read ( &sUnitTypeLibraryNode, sizeof ( sUnitTypeLibraryNode ) ) == sizeof ( sUnitTypeLibraryNode ) );

				// zkontroluje délku jména knihovny typù jednotek
				int nUnitTypeLibraryFileNameLength = strlen ( sUnitTypeLibraryNode.m_pFileName );
				LOAD_ASSERT ( ( nUnitTypeLibraryFileNameLength > 0 ) && 
					( nUnitTypeLibraryFileNameLength < sizeof ( sUnitTypeLibraryNode.m_pFileName ) ) );

				// otevøe archiv knihovny typù jednotek
				CDataArchive *pUnitTypeLibraryArchive = new CDataArchive;
				*pUnitTypeLibraryArchive = cMapArchive.CreateArchive ( 
					sUnitTypeLibraryNode.m_pFileName );

				// otevøe hlavièkový soubor knihovny
				CArchiveFile cUnitTypeLibraryHeaderFile = 
					pUnitTypeLibraryArchive->CreateFile ( _T("Library.Header"), 
					CFile::modeRead | CFile::shareDenyWrite );

				// pøeète hlavièku verze knihovny typù jednotek
				{
					SUnitTypeFileVersionHeader sUnitTypeLibraryVersionHeader;
					LOAD_ASSERT ( cUnitTypeLibraryHeaderFile.Read ( &sUnitTypeLibraryVersionHeader, sizeof ( sUnitTypeLibraryVersionHeader ) ) == sizeof ( sUnitTypeLibraryVersionHeader ) );

					// identifikátor souboru knihovny typù jednotek
					BYTE aUnitTypeLibraryFileID[16] = UNITTYPE_FILE_IDENTIFIER;
					ASSERT ( sizeof ( aUnitTypeLibraryFileID ) == 
						sizeof ( sUnitTypeLibraryVersionHeader.m_aFileID ) );

					// zkontroluje identifikátor souboru knihovny typù jednotek
					for ( int nIndex = 16; nIndex-- > 0; )
					{
						LOAD_ASSERT ( aUnitTypeLibraryFileID[nIndex] == 
							sUnitTypeLibraryVersionHeader.m_aFileID[nIndex] );
					}

					ASSERT ( CURRENT_MAP_FILE_VERSION >= COMPATIBLE_MAP_FILE_VERSION );
					// zkontroluje èísla verzí formátu knihovny typù jednotek
					LOAD_ASSERT ( sUnitTypeLibraryVersionHeader.m_dwFormatVersion >= 
						sUnitTypeLibraryVersionHeader.m_dwCompatibleFormatVersion );

					// zjistí, jedná-li se o správnou verzi formátu knihovny typù jednotek
					if ( sUnitTypeLibraryVersionHeader.m_dwFormatVersion != CURRENT_MAP_FILE_VERSION )
					{	// nejedná se o správnou verzi formátu knihovny typù jednotek
						// zjistí, jedná-li se o starou verzi formátu knihovny typù jednotek
						if ( sUnitTypeLibraryVersionHeader.m_dwFormatVersion < CURRENT_MAP_FILE_VERSION )
						{	// jedná se o starou verzi formátu knihovny typù jednotek
							// zkontroluje kompatabilitu verze formátu knihovny typù jednotek
							LOAD_ASSERT ( sUnitTypeLibraryVersionHeader.m_dwFormatVersion >= 
								COMPATIBLE_MAP_FILE_VERSION );
						}
						else
						{	// jedná se o mladší verzi formátu knihovny typù jednotek
							// zkontroluje kompatabilitu verze formátu knihovny typù jednotek
							LOAD_ASSERT ( sUnitTypeLibraryVersionHeader.m_dwCompatibleFormatVersion <= 
								CURRENT_MAP_FILE_VERSION );
						}
					}
				}

				// pøeète hlavièku knihovny typù jednotek
				SUnitTypeLibraryHeader sUnitTypeLibraryHeader;
				LOAD_ASSERT ( cUnitTypeLibraryHeaderFile.Read ( &sUnitTypeLibraryHeader, sizeof ( sUnitTypeLibraryHeader ) ) == sizeof ( sUnitTypeLibraryHeader ) );

				// aktualizuje poèet typù jednotek v knihovnách typù jednotek
				dwUnitTypeCount += sUnitTypeLibraryHeader.m_dwUnitTypeCount;

				// zavøe hlavièkový soubor knihovny
				cUnitTypeLibraryHeaderFile.Close ();

				// pøidá archiv knihovny typù jednotek do seznamu archivù knihoven typù 
				//		jednotek
				cUnitTypeLibraryArchiveArray.Add ( pUnitTypeLibraryArchive );
			}

			// zkontroluje poèet typù jednotek v knihovnách jednotek
			LOAD_ASSERT ( dwUnitTypeCount > 0 );
		}

	// vytvoøí typy jednotek civilizací
		{
			// zjistí ukazatel na první civilizaci z mapy
			CSCivilization *pCivilization = m_pCivilizations;

			// projede všechny civilizace z mapy
			for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
				dwCivilizationIndex-- > 0; pCivilization++ )
			{
				// nechá vytvoøit typy jednotek civilizace
				pCivilization->CreateUnitTypes ( &cUnitTypeLibraryArchiveArray, dwUnitTypeCount );
			}

			// zjistí ukazatel na první civilizaci z mapy
			pCivilization = m_pCivilizations;

			// projede všechny civilizace z mapy
			for ( dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; 
				pCivilization++ )
			{
				// nechá dokonèit vytvoøení typù jednotek civilizace
				pCivilization->PostCreateUnitTypes ();
			}
		}

	// pøeskoèí pole offsetù MapSquarù
		{
			// zjistí velikost pole offsetù MapSquarù
			DWORD dwMapSquareOffsetArraySize = m_sizeMapMapSquare.cx * 
				m_sizeMapMapSquare.cy * sizeof ( DWORD );

			// pøeskoèí pole offsetù MapSquarù
			LOAD_ASSERT ( cMapFile.GetPosition () + dwMapSquareOffsetArraySize <= 
				cMapFile.GetLength () );
			(void)cMapFile.Seek ( dwMapSquareOffsetArraySize, CFile::current );
		}
	
	// naète informace o MapSquarech
		{
			// index MapSquaru
			CPointDW pointIndex;

			// naète MapSquary
			for ( pointIndex.y = 0; pointIndex.y < m_sizeMapMapSquare.cy; pointIndex.y++ )
			{
				for ( pointIndex.x = 0; pointIndex.x < m_sizeMapMapSquare.cx; pointIndex.x++ )
				{
					// naète další MapSquare
					GetMapSquareFromIndex ( pointIndex )->Create ( cMapFile );
				}
			}
		}

		// vytvoøí memory pool polí viditelnosti jednotky civilizacemi
		CSUnit::m_cCivilizationVisibilityArrayMemoryPool.Create ( 
			max ( m_dwCivilizationCount * sizeof ( WORD ), sizeof ( BYTE * ) ) );
		// vytvoøí memory pool polí posledních pozic, kde byla jednotka civilizacemi vidìna
		CSUnit::m_cCivilizationLastSeenPositionArrayMemoryPool.Create ( 
			max ( m_dwCivilizationCount * sizeof ( CSPosition ), sizeof ( BYTE * ) ) );

	// naète hlavièky jednotek
		{
			SUnitHeader sUnitHeader;

#ifdef _DEBUG
			// uschová ID MainLoop threadu
			DWORD dwMainLoopThreadID = m_dwMainLoopThreadID;
			// nasimuluje MainLoop thread
			m_dwMainLoopThreadID = GetCurrentThreadId ();
#endif //_DEBUG

			for ( DWORD dwUnitIndex = dwUnitCount; dwUnitIndex-- > 0; )
			{
				// naète hlavièku jednotky
				LOAD_ASSERT ( cMapFile.Read ( &sUnitHeader, sizeof ( sUnitHeader ) ) == sizeof ( sUnitHeader ) );

				// zkontroluje umístìní jednotky na mapì
				LOAD_ASSERT ( sUnitHeader.m_dwX < m_sizeMapMapCell.cx );
				LOAD_ASSERT ( sUnitHeader.m_dwY < m_sizeMapMapCell.cy );

				// zjistí index civilizace jednotky
				DWORD dwUnitCivilizationIndex;
				LOAD_ASSERT ( cCivilizationTable.Lookup ( sUnitHeader.m_dwCivilization, 
					dwUnitCivilizationIndex ) );
				ASSERT ( dwUnitCivilizationIndex < m_dwCivilizationCount );

				// najde typ jednotky civilizace
				CSUnitType *pUnitType = 
					m_pCivilizations[dwUnitCivilizationIndex].GetUnitTypeByID ( 
					sUnitHeader.m_dwUnitType );
				LOAD_ASSERT ( pUnitType != NULL );

				ASSERT ( pUnitType->GetCivilization ()->GetCivilizationIndex () == dwUnitCivilizationIndex );

				// vytvoøí novou jednotku typu "pUnitType"
				CSUnit *pUnit = new CSUnit;
				pUnit->Create ( pUnitType );

				// zvýší poèet odkazù na jednotku
				pUnit->AddRef ();
				// pøidá vytvoøenou jednotku do jednotek mapy
				AddUnit ( pUnit );

				// nastaví pozici jednotky na mapì
				LOAD_ASSERT ( sUnitHeader.m_dwZ != 0 );
				pUnit->SetVerticalPosition ( sUnitHeader.m_dwZ );
				CPointDW pointPosition ( sUnitHeader.m_dwX, sUnitHeader.m_dwY );
				LOAD_ASSERT ( CheckLoadedMapPosition ( pointPosition ) );
				pUnit->SetPosition ( pointPosition );
				// nastaví kontrolní body jednotky
				pUnit->SetFirstCheckPoint ( pointPosition, 0 );
				pUnit->SetSecondCheckPoint ( pointPosition, 0 );
				// nastaví nasmìrování jednotky
				LOAD_ASSERT ( sUnitHeader.m_nDirection <= Dir_Random );
				pUnit->SetDirection ( (BYTE)sUnitHeader.m_nDirection );
			}

#ifdef _DEBUG
			// vrátí uschované ID MainLoop threadu
			m_dwMainLoopThreadID = dwMainLoopThreadID;
#endif //_DEBUG
		}

	// naète jména surovin
		{
			SResource sResource;

			// zjistí velikost ikony suroviny
			DWORD dwResourceIconSize = RESOURCE_ICON_WIDTH * 
				RESOURCE_ICON_HEIGHT * sizeof ( DWORD );

			// projede suroviny
			for ( DWORD dwResourceIndex = 0; dwResourceIndex < RESOURCE_COUNT; 
				dwResourceIndex++ )
			{
				// naète surovinu
				LOAD_ASSERT ( cMapFile.Read ( &sResource, sizeof ( sResource ) ) == sizeof ( sResource ) );

				// zkontroluje délku jména suroviny
				m_aResourceName[dwResourceIndex] = sResource.m_szName;
				LOAD_ASSERT ( m_aResourceName[dwResourceIndex].GetLength () < sizeof ( sResource.m_szName ) );

				// pøeskoèí ikonu suroviny
				LOAD_ASSERT ( cMapFile.GetPosition () + dwResourceIconSize <= 
					cMapFile.GetLength () );
				(void)cMapFile.Seek ( dwResourceIconSize, CFile::current );
			}
		}

	// naète jména neviditelností
		{
			SInvisibility sInvisibility;

			// projede neviditelnosti
			for ( DWORD dwInvisibilityIndex = 0; 
				dwInvisibilityIndex < INVISIBILITY_COUNT; dwInvisibilityIndex++ )
			{
				// naète neviditelnost
				LOAD_ASSERT ( cMapFile.Read ( &sInvisibility, sizeof ( sInvisibility ) ) == sizeof ( sInvisibility ) );

				// zkontroluje délku jména neviditelnosti
				m_aInvisibilityName[dwInvisibilityIndex] = sInvisibility.m_szName;
				LOAD_ASSERT ( m_aInvisibilityName[dwInvisibilityIndex].GetLength () < sizeof ( sInvisibility.m_szName ) );
			}
		}

	// naète hlavièky grafù pro hledání cesty
		{
			SFindPathGraphHeader sFindPathGraphHeader;

			// vytvoøí pole grafù pro hledání cesty
			CSFindPathGraph::PreCreateFindPathGraphs ( dwFindPathGraphCount );

			// popis grafu pro hledání cesty
			typedef struct 
			{
				DWORD dwID;
				CSFindPathGraph *pFindPathGraph;
				CString strName;
			} SFindPathGraphDescription;

			// pole popisù grafù pro hledání cesty
			SFindPathGraphDescription *aFindPathGraphDescription = 
				new SFindPathGraphDescription[dwFindPathGraphCount];

			// projede hlavièky grafù pro hledání cesty
			for ( DWORD dwFindPathGraphIndex = dwFindPathGraphCount; 
				dwFindPathGraphIndex-- > 0; )
			{
				// naète hlavièku grafu pro hledání cest
				LOAD_ASSERT ( cMapFile.Read ( &sFindPathGraphHeader, sizeof ( sFindPathGraphHeader ) ) == sizeof ( sFindPathGraphHeader ) );

				// zkontroluje unikátnost ID a jména grafu pro hledání cesty
				for ( DWORD dwPreviousFindPathGraphIndex = dwFindPathGraphCount; 
					--dwPreviousFindPathGraphIndex > dwFindPathGraphIndex; )
				{
					// zkontroluje unikátnost ID grafu pro hledání cesty
					LOAD_ASSERT ( sFindPathGraphHeader.m_dwID != 
						aFindPathGraphDescription[dwPreviousFindPathGraphIndex].dwID );
					// zkontroluje unikátnost jména grafu pro hledání cesty
					LOAD_ASSERT ( aFindPathGraphDescription[dwPreviousFindPathGraphIndex].strName.CompareNoCase ( sFindPathGraphHeader.m_pName ) != 0 );
				}

				// inicializuje ID grafu pro hledání cesty
				aFindPathGraphDescription[dwFindPathGraphIndex].dwID = 
					sFindPathGraphHeader.m_dwID;
				LOAD_ASSERT ( sFindPathGraphHeader.m_dwID != 0 );

				// zkontroluje délku jména grafu pro hledání cesty
				int nFindPathGraphNameLength = strlen ( sFindPathGraphHeader.m_pName );
				LOAD_ASSERT ( ( nFindPathGraphNameLength > 0 ) && 
					( nFindPathGraphNameLength < sizeof ( sFindPathGraphHeader.m_pName ) ) );
				aFindPathGraphDescription[dwFindPathGraphIndex].strName = 
					sFindPathGraphHeader.m_pName;

				// vytvoøí graf pro hledání cesty
				aFindPathGraphDescription[dwFindPathGraphIndex].pFindPathGraph = 
					CSFindPathGraph::CreateFindPathGraph ( dwFindPathGraphIndex, 
					sFindPathGraphHeader.m_dwID, sFindPathGraphHeader.m_pName, cMapArchive );
			}

			// ukazatel na civilizaci
			CSCivilization *pCivilization = m_pCivilizations;

			// aktualizuje ukazatele civilizací na grafy pro hledání cesty
			for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
				dwCivilizationIndex-- > 0; pCivilization++ )
			{
				// ukazatel na typ jednotky
				CSUnitType *pUnitType = pCivilization->m_pUnitTypes;

				// aktualizuje ukazatele typù jednotek na grafy pro hledání cesty
				for ( DWORD dwUnitTypeIndex = pCivilization->m_dwUnitTypeCount; 
					dwUnitTypeIndex-- > 0; pUnitType++ )
				{
					// aktualizuje ukazatele na grafy pro hledání cesty
					for ( DWORD dwModeIndex = 8; dwModeIndex-- > 0; )
					{
						// ID grafu pro hledání cesty
						DWORD dwFindPathGraphID = 
							(DWORD)pUnitType->m_aFindPathGraph[dwModeIndex];

						// zjistí, jedná-li se o graf pro vzdušnou cestu
						if ( dwFindPathGraphID == 0 )
						{	// jedná se o graf pro vzdušnou cestu
							pUnitType->m_aFindPathGraph[dwModeIndex] = NULL;
						}
						else
						{	// nejedná se o graf pro vzdušnou cestu
							// najde odpovídající graf pro hledání cesty
							for ( DWORD dwIndex = dwFindPathGraphCount; dwIndex-- > 0; )
							{
								// zjistí, jedná-li se o hledaný graf cesty
								if ( dwFindPathGraphID == 
									aFindPathGraphDescription[dwIndex].dwID )
								{
									// aktualizuje ukazatel na graf pro hledání cesty
									pUnitType->m_aFindPathGraph[dwModeIndex] = 
										aFindPathGraphDescription[dwIndex].pFindPathGraph;
									break;
								}
							}
							// zkontroluje správnost ID grafu pro hledání cesty
							LOAD_ASSERT ( dwIndex != DWORD_MAX );
						}
						// ukazatel na graf pro hledání cesty je aktualizován
					}
					// ukazatele na grafy pro hledání cesty jsou aktualizovány
				}
				// typy jednotek jsou aktualizovány
			}
			// civilizace jsou aktualizovány

			// znièí pole popisù grafù pro hledání cesty
			if ( aFindPathGraphDescription != NULL )
			{
				delete [] aFindPathGraphDescription;
			}
		}

	// ukonèí naèítání souboru mapy

		// zkontroluje konec souboru
		LOAD_ASSERT ( cMapFile.GetPosition () == cMapFile.GetLength () );
		cMapFile.Close ();

		// znièí pole archivù knihoven typù jednotek
		for ( int nIndex = cUnitTypeLibraryArchiveArray.GetSize (); nIndex-- > 0; )
		{
			// získá další ukazatel na archiv knihovny typù jednotek
			CDataArchive *pUnitTypeLibraryArchive = 
				(CDataArchive *)cUnitTypeLibraryArchiveArray.GetAt ( nIndex );
			// zavøe archiv knihovny typù jednotek
			VERIFY ( pUnitTypeLibraryArchive->Delete () );
			// znièí archiv knihovny typù jednotek
			delete pUnitTypeLibraryArchive;
		}
		// znièí prvky pole archivù knihoven jednotek
		cUnitTypeLibraryArchiveArray.RemoveAll ();
	}
	catch ( CException * )
	{
    CSOneInstance::PreDelete ();

		// znièí pole archivù knihoven typù jednotek
		for ( int nIndex = cUnitTypeLibraryArchiveArray.GetSize (); nIndex-- > 0; )
		{
			// získá další ukazatel na archiv knihovny typù jednotek
			CDataArchive *pUnitTypeLibraryArchive = 
				(CDataArchive *)cUnitTypeLibraryArchiveArray.GetAt ( nIndex );
			// zavøe archiv knihovny typù jednotek
			VERIFY ( pUnitTypeLibraryArchive->Delete () );
			// znièí archiv knihovny typù jednotek
			delete pUnitTypeLibraryArchive;
		}
		// znièí prvky pole archivù knihoven jednotek
		cUnitTypeLibraryArchiveArray.RemoveAll ();

		// zneškodní èasteènì inicializovaná data mapy
		SetEmptyData ();

		// znièit statické vìci
		CSOneInstance::Delete ();

		// pokraèuje ve zpracování výjimky
		throw;
	}

// ukonèí vytváøení grafù pro hledání cesty

	// ukonèí vytváøení grafù pro hledání cesty
	CSFindPathGraph::PostCreateFindPathGraphs ();

// vytvoøí data cest

	// vytvoøí data cest
	CSPath::CreatePathes ( cMapArchive );

// vytvoøí mapy jednotek

	// vytvoøí mapy jednotek
	CSMapSquare::CreateUnitMaps ();

// inicializace hry

	// inicializuje èíslo TimeSlicu
	m_dwTimeSlice = 1;

	try
	{
		// inicializuje index rohového MapSquaru
		m_pointCornerMapSquare = CPointDW ( m_sizeMapMapSquare.cx - 1, 
			m_sizeMapMapSquare.cy - 1 );
		m_pointCornerMapCell = CPointDW ( m_sizeMapMapCell.cx - 1, 
			m_sizeMapMapCell.cy - 1 );

	// umístìní jednotek na mapu

#ifdef _DEBUG
		// uschová ID MainLoop threadu
		DWORD dwMainLoopThreadID = m_dwMainLoopThreadID;
		// nasimuluje MainLoop thread
		m_dwMainLoopThreadID = GetCurrentThreadId ();
#endif //_DEBUG

		// vytvoøí pole pøíznakù objevení pozic na mapì pro metodu "GetNearestUnitPlace"
		m_cGetNearestUnitPlacePositionArray.Create ( m_sizeMapMapCell.cx, 
			m_sizeMapMapCell.cy, 2 * MAP_SQUARE_SIZE, 0 );

		// projede všechny bloky jednotek
		for ( SUnitBlock *pBlock = m_pFirstUnitBlock; pBlock != NULL; 
			pBlock = pBlock->pNext )
		{
			// ukazatel na jednotku v bloku
			CSUnit **pBlockUnit = pBlock->pUnits;
			// projede všechny jednotky v bloku jednotek
			for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
			{
				// získá ukazatel na jednotku
				CSUnit *pUnit = *pBlockUnit;

				// zjistí, je-li to platná jednotka
				if ( pUnit == NULL )
				{	// není to platná jednotka
					continue;
				}
				// je to platná jednotka

				// umístí jednotku na mapu
				pUnit->SetPosition ( GetNearestUnitPlace ( pUnit, pUnit->GetPosition () ) );
				LOAD_ASSERT ( IsMapPosition ( pUnit->GetPosition () ) );
				UnitPlaced ( pUnit );
				// nastaví kontrolní body jednotky
				pUnit->SetFirstCheckPoint ( pUnit->GetPosition (), 0 );
				pUnit->SetSecondCheckPoint ( pUnit->GetPosition (), 0 );
			}
		}

#ifdef _DEBUG
		// vrátí uschované ID MainLoop threadu
		m_dwMainLoopThreadID = dwMainLoopThreadID;
#endif //_DEBUG

	// inicializace notifikací

		// inicializuje notifikace
		InitializeNotifications ( TRUE );

	// rozešle civilizacím zprávu o inicializované høe

		// zjistí ukazatel na první civilizaci z mapy
		CSCivilization *pCivilization = m_pCivilizations;

		// projede všechny civilizace z mapy
		for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
			dwCivilizationIndex-- > 0; pCivilization++ )
		{
			// pošle civilizaci zprávu o inicializované høe
			pCivilization->m_pZCivilization->OnGameInitialized ();
		}

	// inicializace hry

		// inicializuje hru (vytvoøí zapausovanou hru)
		InitGame ();
	}
	catch ( CException * )
	{
		// zneškodní èasteènì inicializovaná data mapy
		SetEmptyData ();

		// pokraèuje ve zpracovávání výjimky
		throw;
	}

//	TRACE ( "Map created\n" );
}

// znièí data mapy
void CSMap::Delete () 
{
	// pøíznak inicializované mapy
	BOOL bInitialized = m_bInitialized;
	// zruší pøíznak inicializované mapy
	m_bInitialized = FALSE;

	// Call the predelete
	CSOneInstance::PreDelete ();

	// Disconnect all observers
	VERIFY ( m_cMapObserverListLock.Lock () );
	CSMapObserver *pObserver = m_pFirstMapObserver, *pNext;
	while ( pObserver != NULL )
	{
		pNext = pObserver;
		pObserver = pObserver->m_pNextObserver;
		pNext->m_pNextObserver = NULL;
	}
	m_pFirstMapObserver = NULL;
	VERIFY ( m_cMapObserverListLock.Unlock () );

// znièí thready mapy

	// zjistí, je-li mapa inicializovaná
	if ( bInitialized )
	{	// mapa je inicializovaná
		// zjistí, je-li hra zapausovaná
		if ( !IsGamePaused () )
		{	// hra není zapausovaná
			// zapausuje hru
			PauseGame ();
		}

		// odpausuje hru s ukonèením hry
		UnpauseGame ( FALSE );
	}

// znièí notifikace

	// znièí notifikace
	DestructNotifications ();

// znièí MapSquary

	if ( m_pMapSquares != NULL )
	{
		// zjistí ukazatel na první MapSquare
		CSMapSquare *pMapSquare = m_pMapSquares;

		// projede všechny MapSquary
		for ( DWORD dwMapSquareIndex = m_sizeMapMapSquare.cx * m_sizeMapMapSquare.cy; 
			dwMapSquareIndex-- > 0; pMapSquare++ )
		{
			// znièí další MapSquare
			pMapSquare->Delete ();
		}

		// znièí pole MapSquarù
		delete [] m_pMapSquares;
		m_pMapSquares = NULL;
	}

// zruší odkazy na jednotky na mapì

//	TRACE ( "------------------------------ blocks\n" );

	// projede všechny bloky jednotek
	for ( SUnitBlock *pBlock = m_pFirstUnitBlock; pBlock != NULL; )
	{
		// ukazatel na jednotku v bloku
		CSUnit **pBlockUnit = pBlock->pUnits;
		// projede všechny jednotky v bloku jednotek
		for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
		{
			// ukazatel na jednotku
			CSUnit *pUnit = *pBlockUnit;
			// zjistí, je-li to platná jednotka
			if ( pUnit != NULL )
			{	// je to platná jednotka
	
//				TRACE ( "Unit ID=%d explicitly deleted (lives=%d, refcount=%d)\n", 
//					pUnit->GetID (), pUnit->GetLives (), pUnit->m_dwReferenceCounter );

				// sníží jednotce poèet životù
				pUnit->m_nLives = DEAD_UNIT_LIVES;
				// pøidá jednotku do seznamu smazaných jednotek
				m_cDeletedUnitList.Add ( pUnit );

				ASSERT ( ( pUnit->m_pUnitType != NULL ) && ( pUnit->m_pSkillData != NULL ) );
				// znièí skilly jednotky
				pUnit->m_pUnitType->DestructSkills ( pUnit );

				// zruší odkaz na jednotku
				pUnit->Release ();
				*pBlockUnit = NULL;
			}
		}

		// zjistí ukazatel na další blok jednotek
		pBlock = pBlock->pNext;
	}

	// popis mrtvé jednotky
	struct SDeadUnit sDeadUnit;

//	TRACE ( "------------------------------ m_cDeadUnitQueue\n" );

	// znièí frontu mrtvých jednotek
	while ( m_cDeadUnitQueue.RemoveFirst ( sDeadUnit ) )
	{
		ASSERT ( sDeadUnit.pUnit != NULL );
		// sníží jednotce poèet životù
		sDeadUnit.pUnit->m_nLives = DEAD_UNIT_LIVES;
		// pøidá jednotku do seznamu smazaných jednotek
		m_cDeletedUnitList.Add ( sDeadUnit.pUnit );

//		TRACE ( "Unit ID=%d m_cDeadUnitQueue.Remove (lives=%d, refcount=%d)\n", 
//			sDeadUnit.pUnit->GetID (), sDeadUnit.pUnit->GetLives (), 
//			sDeadUnit.pUnit->m_dwReferenceCounter );

		ASSERT ( ( sDeadUnit.pUnit->m_pUnitType != NULL ) && 
			( sDeadUnit.pUnit->m_pSkillData != NULL ) );
		// znièí skilly jednotky
		sDeadUnit.pUnit->m_pUnitType->DestructSkills ( sDeadUnit.pUnit );

		// zruší odkaz na jednotku
		sDeadUnit.pUnit->Release ();
	}

// znièí seznam mrtvých jednotek pro rozesílání stop infa klientùm

//	TRACE ( "------------------------------ m_cDeadUnitClientStopInfoList\n" );

	// mrtvá jednotka
	CSUnit *pUnit;
	// zpracuje seznam mrtvých jednotek pro rozesílání stop infa klientùm
	while ( m_cDeadUnitClientStopInfoList.RemoveFirst ( pUnit ) )
	{
//		TRACE ( "Unit ID=%d m_cDeadUnitClientStopInfoList.Remove (lives=%d, refcount=%d)\n", 
//			pUnit->GetID (), pUnit->GetLives (), pUnit->m_dwReferenceCounter );
		// sníží poèet odkazù na jednotku
		pUnit->Release ();
	}

// znièí odložená volání procedur civilizací

	// projede všechny civilizace
	for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; )
	{
		CSDeferredProcedureCall *pDPC;
		// projede všechna odložená volání procedur civilizace
		while ( m_aDPCQueue[dwCivilizationIndex].RemoveFirst ( pDPC ) )
		{
			// znièí další odložené volání procedury
			delete pDPC;
		}
	}

// znièí civilizace

	if ( m_pCivilizations != NULL )
	{
		// zjistí ukazatel na první civilizaci
		CSCivilization *pCivilization = m_pCivilizations;

		// projede všechny civilizace
		for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; 
			pCivilization++ )
		{
			// znièí civilizaci na civilizaci
			if ( pCivilization->m_pZCivilization != NULL )
			{
				pCivilization->m_pZCivilization->PreDelete ();
			}
		}

		// zjistí ukazatel na první civilizaci
		pCivilization = m_pCivilizations;

		// projede všechny civilizace
		for ( dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; 
			pCivilization++ )
		{
			// znièí civilizaci na civilizaci
			if ( pCivilization->m_pZCivilization != NULL )
			{
				pCivilization->m_pZCivilization->Delete ();
			}
		}

		// zjistí ukazatel na první civilizaci
		pCivilization = m_pCivilizations;

		// projede všechny civilizace
		for ( dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; 
			pCivilization++ )
		{
			// znièí civilizaci na civilizaci
			if ( pCivilization->m_pZCivilization != NULL )
			{
				delete pCivilization->m_pZCivilization;
				pCivilization->m_pZCivilization = NULL;
			}
			// znièí další civilizaci
			pCivilization->Delete ();
		}

		// znièí pole civilizací
		delete [] m_pCivilizations;
		m_pCivilizations = NULL;
	}

#ifdef _DEBUG
	ASSERT ( m_cDeletedUnitList.IsEmpty () );
	TRACE ( "------------------------------ m_cDeletedUnitList\n" );
	while ( m_cDeletedUnitList.RemoveFirst ( pUnit ) )
	{
		TRACE ( "Unit ID=%d not deleted (lives=%d, refcount=%d)\n", pUnit->GetID (), 
			pUnit->GetLives (), pUnit->m_dwReferenceCounter );
	}
#endif //_DEBUG

// znièí jednotky na mapì

	// projede všechny bloky jednotek
	for ( pBlock = m_pFirstUnitBlock; pBlock != NULL; )
	{
		// uschová si ukazatel na blok jednotek
		SUnitBlock *pLastBlock = pBlock;
		// zjistí ukazatel na další blok jednotek
		pBlock = pBlock->pNext;

		// znièí pole ukazatelù na jednotky bloku jednotek
		delete pLastBlock->pUnits;
		// znièí uschovaný blok jednotek
		delete pLastBlock;
	}

	// zneškodní ukazatele na bloky jednotek
	m_pFirstUnitBlock = NULL;
	m_pLastUnitBlock = NULL;

	// projede všechny záznamy o volných jednotkách
	for ( SFreeUnit *pFreeUnit = m_pFirstFreeUnit; pFreeUnit != NULL; )
	{
		// uschová si ukazatel na záznam o volné jednotce
		SFreeUnit *pLastFreeUnit = pFreeUnit;
		// zjistí ukazatel na další záznam o volné jednotce
		pFreeUnit = pFreeUnit->pNext;

		// znièí uschovaný záznam o volné jednotce
		delete pLastFreeUnit;
	}

	// zneškodní ukazatel na první volnou jednotku
	m_pFirstFreeUnit = NULL;

	// znièí memory pool polí viditelnosti jednotky civilizacemi
	CSUnit::m_cCivilizationVisibilityArrayMemoryPool.Delete ();
	// znièí memory pool polí posledních pozic, kde byla jednotka civilizacemi vidìna
	CSUnit::m_cCivilizationLastSeenPositionArrayMemoryPool.Delete ();

// znièí pomocná data

	// znièí pole pøíznakù objevení pozic na mapì pro metodu "GetNearestUnitPlace"
	m_cGetNearestUnitPlacePositionArray.Delete ();

// znièí data cest

#ifdef _DEBUG
		// uschová ID FindPathLoop threadu
		DWORD dwFindPathLoopThreadID = m_pFindPathLoopThread->m_nThreadID;
		// nasimuluje FindPathLoop thread
		m_pFindPathLoopThread->m_nThreadID = GetCurrentThreadId ();
#endif //_DEBUG

	// znièí data cest
	CSPath::DeletePathes ();

#ifdef _DEBUG
		// vrátí uschované ID FindPathLoop threadu
		m_pFindPathLoopThread->m_nThreadID = dwFindPathLoopThreadID;
#endif //_DEBUG

// znièí grafy pro hledání cesty

	// znièí grafy pro hledání cesty
	CSFindPathGraph::DeleteFindPathGraphs ();

// vytvoøí mapy jednotek

	// vytvoøí mapy jednotek
	CSMapSquare::DeleteUnitMaps ();

// znièí thready mapy

	// zjistí, je-li mapa inicializovaná
	if ( bInitialized )
	{	// mapa je inicializovaná
	// uschová handly threadù mapy

		CWinThread *pMainLoopThread = m_pMainLoopThread;
		CWinThread *pClientInfoSenderLoopThread = m_pClientInfoSenderLoopThread;
		CWinThread *pFindPathLoopThread = m_pFindPathLoopThread;

		m_pMainLoopThread = NULL;
		m_pClientInfoSenderLoopThread = NULL;
		m_pFindPathLoopThread = NULL;

	// zneškodní data mapy

		SetEmptyData ();

	// poèká na dojetí threadù mapy

		// tabulka pausovaných threadù
		HANDLE aPausingThreads[3] = { 
			pMainLoopThread->m_hThread, 
			pClientInfoSenderLoopThread->m_hThread, 
			pFindPathLoopThread->m_hThread, 
		};

		// poèká na ukonèení pausovaných threadù
		switch ( WaitForMultipleObjects ( 3, aPausingThreads, TRUE, INFINITE ) )
		{
		// thready byly ukonèeny
		case WAIT_OBJECT_0 :
		case ( WAIT_OBJECT_0 + 1 ) :
		case ( WAIT_OBJECT_0 + 2 ) :
			break;
		// thready se nepovedlo ukonèit
		default:
			TRACE0 ( _T("Error - can't end threads correctly (time out)\n") );
		}

	// znièí handly threadù mapy

		delete pMainLoopThread;
		delete pClientInfoSenderLoopThread;
		delete pFindPathLoopThread;
	}
	else
	{	// mapa není inicializovaná
		// zneškodní data mapy
		SetEmptyData ();
	}

	// znièit statické vìci
	CSOneInstance::Delete ();
}

// vyplní objekt neškodnými daty (znièí èásteènì inicializavaná data)
void CSMap::SetEmptyData () 
{
	// aktualizuje pøíznak pokraèování hry
	m_bContinueUnpausedGame = FALSE;

	// zruší událost zapausování hry
	VERIFY ( m_eventPauseGame.ResetEvent () );
	// nastaví událost odpausování hry
	VERIFY ( m_eventUnpauseGame.SetEvent () );

	// zneškodní MapSquary
	if ( m_pMapSquares != NULL )
	{
		// znièí pole MapSquarù
		delete [] m_pMapSquares;
		m_pMapSquares = NULL;
	}
	m_sizeMapMapSquare = CSizeDW ( 0, 0 );
	m_sizeMapMapCell = CSizeDW ( 0, 0 );
	m_pointCornerMapSquare = CPointDW ( 0, 0 );
	m_pointCornerMapCell = CPointDW ( 0, 0 );

	m_strMapArchiveName.Empty ();
	m_dwMapFormatVersion = 0;
	m_dwMapCompatibleFormatVersion = DWORD_MAX;
	m_strMapName.Empty ();
	m_strMapDescription.Empty ();
	m_dwMapVersion = DWORD_MAX;

	// zneškodní jména surovin
	for ( DWORD dwResourceIndex = RESOURCE_COUNT; dwResourceIndex-- > 0; )
	{
		m_aResourceName[dwResourceIndex].Empty ();
	}

	// zneškodní jména neviditelností
	for ( DWORD dwInvisibilityIndex = INVISIBILITY_COUNT; dwInvisibilityIndex-- > 0; )
	{
		m_aInvisibilityName[dwInvisibilityIndex].Empty ();
	}

	// zneškodní civilizace
	if ( m_pCivilizations != NULL )
	{
		ASSERT ( m_dwCivilizationCount > 0 );
		// projede všechny civilizace
		for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; )
		{
			// zneškodní další civilziaci
			m_pCivilizations[dwCivilizationIndex].SetEmptyData ();
		}
		// znièí pole civilizací
		delete [] m_pCivilizations;
		m_pCivilizations = NULL;
	}
	m_dwCivilizationCount = 0;

	m_pFirstUnitBlock = NULL;
	m_pLastUnitBlock = NULL;
	m_pFirstFreeUnit = NULL;

	m_dwTimeSlice = 0;

	// zjistí, je-li platný odkaz na MainLoop thread
	if ( m_pMainLoopThread != NULL )
	{	// znièí odkaz na MainLoop thread
		delete m_pMainLoopThread;
		m_pMainLoopThread = NULL;
	}
	// zjistí, je-li platný odkaz na ClientInfoSenderLoop thread
	if ( m_pClientInfoSenderLoopThread != NULL )
	{	// znièí odkaz na ClientInfoSenderLoop thread
		delete m_pClientInfoSenderLoopThread;
		m_pClientInfoSenderLoopThread = NULL;
	}
	// zjistí, je-li platný odkaz na FindPathLoop thread
	if ( m_pFindPathLoopThread != NULL )
	{	// znièí odkaz na FindPathLoop thread
		delete m_pFindPathLoopThread;
		m_pFindPathLoopThread = NULL;
	}

	// projede všechny civilizace
	for ( DWORD dwIndex = CIVILIZATION_COUNT_MAX; dwIndex-- > 0; )
	{
		CSDeferredProcedureCall *pDPC;
		// projede všechna odložená volání procedur civilizace
		while ( m_aDPCQueue[dwIndex].RemoveFirst ( pDPC ) )
		{
			// znièí další odložené volání procedury
			delete pDPC;
		}
	}
	m_pPreparingDPC = NULL;

	// znièí pole uživatelù
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// znièí popis uživatele
		delete m_aUserDescriptions.GetAt ( nIndex );
	}
	// znièí prvky pole uživatelù
	m_aUserDescriptions.RemoveAll ();
}

//////////////////////////////////////////////////////////////////////
// Ukládání dat (CPersistentObject metody)
//////////////////////////////////////////////////////////////////////

/*
CSMap stored

	DWORD m_dwTimeSlice
	DWORD dwMapSizeX (m_sizeMapMapCell.cx)
	DWORD dwMapSizeY (m_sizeMapMapCell.cy)
	int nMapArchiveNameSize						// musí být > 0
		char aMapArchiveName[nMapArchiveNameSize]
	DWORD m_dwMapFormatVersion
	DWORD m_dwMapCompatibleFormatVersion
	int nMapNameSize							// mùže být i 0 - pak je vynecháno aMapName
		char aMapName[nMapNameSize]
	int nMapDescriptionSize					// mùže být i 0 - pak je vynecháno aMapDescription
		char aMapDescription[nMapDescriptionSize]
	DWORD m_dwMapVersion
	DWORD m_dwCivilizationCount
	[m_dwCivilizationCount]
		CSCivilization *
		CSCivilization stored
		DWORD dwDPCCount
		[dwDPCCount]
			CSDeferredProcedureCall *
			DWORD dwID
			CSDeferredProcedureCall stored
	DWORD dwUnitCount
	[dwUnitCount]
		CSUnit *
		CSUnit stored
	DWORD dwDeadUnitCount
	[dwDeadUnitCount]
		DWORD dwTimeSlice
		CSUnit *
		CSUnit stored
	DWORD dwDeletedUnitCount
	[dwDeletedUnitCount]
		CSUnit *
		CSUnit stored
	DWORD dwUserCount
	[dwUserCount]
		int nUserNameSize
		char aUserName[nUserNameSize]
		int nUserPasswordSize				// mùže být i 0 - pak je vynecháno aUserPassword
		char aUserPassword[nUserPasswordSize]
		DWORD dwCivilizationIndex
	find path graphs stored
	pathes stored
	CStringTable stored
	CSOneInstance stored
*/

// ukládání dat
void CSMap::PersistentSave ( CPersistentStorage &storage ) 
{
//	BRACE_BLOCK(storage); TADY NE kvuli return FALSE; v PersistentLoad

	ASSERT ( CheckValidData () );

// uloží základní informace o høe

	// uloží èíslo TimeSlicu
	storage << m_dwTimeSlice;

// uloží základní informace o mapì

	// uloží velikost mapy
	storage << m_sizeMapMapCell.cx;
	storage << m_sizeMapMapCell.cy;

	// zjistí délku jména archivu mapy
	int nMapArchiveNameSize = m_strMapArchiveName.GetLength ();
	ASSERT ( nMapArchiveNameSize > 0 );
	// uloží délku jména archivu mapy
	storage << nMapArchiveNameSize;
	// uloží jméno archivu mapy
	storage.Write ( m_strMapArchiveName, nMapArchiveNameSize );

	// uloží verzi formátu mapy
	storage << m_dwMapFormatVersion;
	// uloží kompatabilní verzi formátu mapy
	storage << m_dwMapCompatibleFormatVersion;

	// zjistí délku jména mapy
	int nMapNameSize = m_strMapName.GetLength ();
	// uloží délku jména mapy
	storage << nMapNameSize;
	// zjistí, je-li jméno mapy prázdné
	if ( nMapNameSize > 0 )
	{	// jméno mapy není prázdné
		// uloží jméno mapy
		storage.Write ( m_strMapName, nMapNameSize );
	}

	// zjistí délku popisu mapy
	int nMapDescriptionSize = m_strMapDescription.GetLength ();
	// uloží délku popisu mapy
	storage << nMapDescriptionSize;
	// zjistí, je-li popis mapy prázdný
	if ( nMapDescriptionSize > 0 )
	{	// popis mapy není prázdný
		// uloží popis mapy
		storage.Write ( m_strMapDescription, nMapDescriptionSize );
	}

	// uloží verzi mapy
	storage << m_dwMapVersion;

// uloží civilizace

	// uloží poèet civilizací
	storage << m_dwCivilizationCount;

	// zjistí ukazatel na první civilizaci
	CSCivilization *pCivilization = m_pCivilizations;

	// projede všechny civilizace
	for ( DWORD dwCivilizationIndex = 0; dwCivilizationIndex < m_dwCivilizationCount; 
		dwCivilizationIndex++, pCivilization++ )
	{
		// uloží ukazatel na civilizaci
		storage << pCivilization;
		// uloží civilizaci
		pCivilization->PersistentSave ( storage );

	// uloží DPC frontu civilizace

		// uloží velikost DPC fronty civilizace
		storage << m_aDPCQueue[dwCivilizationIndex].GetSize ();

		// zjistí pozici prvního prvku v DPC frontì civilizace
		POSITION posDPC = m_aDPCQueue[dwCivilizationIndex].GetHeadPosition ();
		// ukazatel na DPC
		CSDeferredProcedureCall *pDPC;

		// projede DPC frontu civilizace
		while ( CSDeferredProcedureCallQueue::GetNext ( posDPC, pDPC ) )
		{
			// uloží ukazatel na DPC
			storage << pDPC;
			// uloží DPC ID
			storage << pDPC->GetID ();

			// uloží DPC
			pDPC->PersistentSave ( storage );
		}
	}

// pomocné promìnné

	// poèet jednotek na mapì
	DWORD dwUnitCount = 0;
	// ukazatel na blok jednotek
	SUnitBlock *pBlock;

// spoèítá poèet jednotek

	// projede všechny bloky jednotek
	for ( pBlock = m_pFirstUnitBlock; pBlock != NULL; pBlock = pBlock->pNext )
	{
		// ukazatel na jednotku v bloku
		CSUnit **pBlockUnit = pBlock->pUnits;
		// projede všechny jednotky v bloku jednotek
		for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
		{
			// zjistí, je-li to platná jednotka
			if ( *pBlockUnit != NULL )
			{	// je to platná jednotka
				// zvýší poèet jednotek na mapì
				dwUnitCount++;
			}
		}
	}

// uloží jednotky na mapì

	// uloží poèet jednotek
	storage << dwUnitCount;

	// projede všechny bloky jednotek
	for ( pBlock = m_pFirstUnitBlock; pBlock != NULL; pBlock = pBlock->pNext )
	{
		// ukazatel na jednotku v bloku
		CSUnit **pBlockUnit = pBlock->pUnits;
		// projede všechny jednotky v bloku jednotek
		for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
		{
			// zjistí, je-li to platná jednotka
			if ( *pBlockUnit != NULL )
			{	// je to platná jednotka
				// uloží ukazatel na jednotku
				storage << (*pBlockUnit);
				// uloží jednotku
				(*pBlockUnit)->PersistentSave ( storage );
			}
		}
	}

// uloží frontu mrtvých jednotek

	// poèet mrtvých jednotek
	DWORD dwDeadUnitCount = m_cDeadUnitQueue.GetSize ();

	// uloží poèet mrtvých jednotek
	storage << dwDeadUnitCount;

	// popis mrtvé jednotky
	struct SDeadUnit sDeadUnit;
	// pozice mrtvé jednotky ve frontì mrtvých jednotek
	POSITION posDeadUnit = m_cDeadUnitQueue.GetHeadPosition ();

	// uloží frontu mrtvých jednotek
	while ( m_cDeadUnitQueue.GetNext ( posDeadUnit, sDeadUnit ) )
	{
		// uloží popis mrtvé jednotky
		storage << sDeadUnit.dwTimeSlice;
		storage << sDeadUnit.pUnit;
		// uloží mrtvou jednotku
		sDeadUnit.pUnit->PersistentSave ( storage );
	}

// uloží seznam smazaných jednotek

	// poèet smazaných jednotek
	DWORD dwDeletedUnitCount = m_cDeletedUnitList.GetSize ();

	// uloží poèet smazaných jednotek
	storage << dwDeletedUnitCount;

	// ukazatel na smazanou jednotku
	CSUnit *pDeletedUnit;
	// pozice smazané jednotky v seznamu smazaných jednotek
	POSITION posDeletedUnit = m_cDeletedUnitList.GetHeadPosition ();

	// uloží seznam smazaných jednotek
	while ( m_cDeletedUnitList.GetNext ( posDeletedUnit, pDeletedUnit ) )
	{
		// uloží ukazatel na smazanou jednotku
		storage << pDeletedUnit;
		// uloží smazanou jednotku
		pDeletedUnit->PersistentSave ( storage );
	}

// uloží pole uživatelù

	// uloží poèet uživatelù
	storage << (DWORD)m_aUserDescriptions.GetSize ();

	// projede pole uživatelù
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjistí popis uživatele
		struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );

		// zjistí délku jména uživatele
		int nUserNameSize = pUserDescription->strName.GetLength ();
		ASSERT ( nUserNameSize > 0 );
		// uloží délku jména uživatele
		storage << nUserNameSize;
		// uloží jméno uživatele
		storage.Write ( pUserDescription->strName, nUserNameSize );

		// zjistí délku hesla uživatele
		int nUserPasswordSize = pUserDescription->strPassword.GetLength ();
		// uloží délku hesla uživatele
		storage << nUserPasswordSize;
		// zjistí, je-li heslo uživatele prázdné
		if ( nUserPasswordSize > 0 )
		{	// heslo uživatele není prázdné
			// uloží heslo uživatele
			storage.Write ( pUserDescription->strPassword, nUserPasswordSize );
		}

		// uloží index civilizace
		ASSERT ( pUserDescription->dwCivilizationIndex > 0 );
		ASSERT ( pUserDescription->dwCivilizationIndex < m_dwCivilizationCount );
		storage << pUserDescription->dwCivilizationIndex;

		// uloží pøíznak povolení
		storage << pUserDescription->bEnabled;
	}

// uloží grafy pro hledání cesty

	// uloží grafy pro hledání cesty
	CSFindPathGraph::PersistentSaveFindPathGraphs ( storage );

// uloží cesty

	// uloží cesty
	CSPath::PersistentSavePathes ( storage );

// uloží øetìzce hry

	// uloží øetìzce hry
	g_StringTable.PersistentSave ( storage );

// uloží statická data

	// uloží statická data
	CSOneInstance::PersistentSave ( storage );
}

// nahrávání pouze uložených dat, preferuje archiv mapy "strMapArchiveName", je-li 
//		"strMapArchiveName" prázdný, použije uložené jméno archivu mapy (FALSE=špatná 
//		verze archivu mapy, v "strMapArchiveName" vrací uložené jméno archivu mapy
BOOL CSMap::PersistentLoad ( CPersistentStorage &storage, CString &strMapArchiveName ) 
{
	ASSERT ( CheckEmptyData () );

//	BRACE_BLOCK(storage); TADY NE kvuli return FALSE;

	try
	{
		// uložený ukazatel
		void *pSavedPointer;

	// naète základní informace o høe

		// naète èíslo TimeSlicu
		storage >> m_dwTimeSlice;

	// naète základní informace o mapì

		// naète velikost mapy
		storage >> m_sizeMapMapCell.cx;
		storage >> m_sizeMapMapCell.cy;
		LOAD_ASSERT ( m_sizeMapMapCell.cx % MAP_SQUARE_SIZE == 0 );
		LOAD_ASSERT ( m_sizeMapMapCell.cy % MAP_SQUARE_SIZE == 0 );
		m_sizeMapMapSquare.cx = m_sizeMapMapCell.cx / MAP_SQUARE_SIZE;
		m_sizeMapMapSquare.cy = m_sizeMapMapCell.cy / MAP_SQUARE_SIZE;
		LOAD_ASSERT ( m_sizeMapMapSquare.cx >= 2 );
		LOAD_ASSERT ( m_sizeMapMapSquare.cy >= 2 );
		LOAD_ASSERT ( m_sizeMapMapCell.cx < m_sizeMapMapCell.cx * m_sizeMapMapCell.cy );
		LOAD_ASSERT ( m_sizeMapMapCell.cy < m_sizeMapMapCell.cx * m_sizeMapMapCell.cy );
		LOAD_ASSERT ( m_sizeMapMapCell.cx * m_sizeMapMapCell.cy < m_sizeMapMapCell.cx * 
			m_sizeMapMapCell.cy * sizeof ( DWORD ) );

		// vytvoøí pole MapSquarù
		m_pMapSquares = new CSMapSquare[m_sizeMapMapSquare.cx * m_sizeMapMapSquare.cy];

		// délka jména archivu mapy
		int nMapArchiveNameSize;
		// naète délku jména archivu mapy
		storage >> nMapArchiveNameSize;
		LOAD_ASSERT ( nMapArchiveNameSize > 0 );
		// získá ukazatel na naèítané jméno archivu mapy
		char *szMapArchiveName = m_strMapArchiveName.GetBufferSetLength ( nMapArchiveNameSize );
		// naète jméno archivu mapy
		storage.Read ( szMapArchiveName, nMapArchiveNameSize );
		// zkontroluje délku jména archivu mapy
		for ( int nCharacterIndex = nMapArchiveNameSize; nCharacterIndex-- > 0; )
		{
			// zkontroluje další znak jména archivu mapy
			LOAD_ASSERT ( szMapArchiveName[nCharacterIndex] != 0 );
		}
		// ukonèí naèítání jména archivu mapy
		m_strMapArchiveName.ReleaseBuffer ( nMapArchiveNameSize );

		// naète verzi formátu mapy
		storage >> m_dwMapFormatVersion;
		// naète kompatabilní verzi formátu mapy
		storage >> m_dwMapCompatibleFormatVersion;

		ASSERT ( m_strMapName.IsEmpty () );
		// délka jména mapy
		int nMapNameSize;
		// naète délku jména mapy
		storage >> nMapNameSize;
		// zjistí, je-li jméno mapy prázdné
		if ( nMapNameSize > 0 )
		{	// jméno mapy není prázdné
			// získá ukazatel na naèítané jméno mapy
			char *szMapName = m_strMapName.GetBufferSetLength ( nMapNameSize );
			// naète jméno mapy
			storage.Read ( szMapName, nMapNameSize );
			// zkontroluje délku jména mapy
			for ( int nCharacterIndex = nMapNameSize; nCharacterIndex-- > 0; )
			{
				// zkontroluje další znak jména mapy
				LOAD_ASSERT ( szMapName[nCharacterIndex] != 0 );
			}
			// ukonèí naèítání jména mapy
			m_strMapName.ReleaseBuffer ( nMapNameSize );
		}

		ASSERT ( m_strMapDescription.IsEmpty () );
		// délka popisu mapy
		int nMapDescriptionSize;
		// naète délku popisu mapy
		storage >> nMapDescriptionSize;
		// zjistí, je-li popis mapy prázdný
		if ( nMapDescriptionSize > 0 )
		{	// popis mapy není prázdný
			// získá ukazatel na naèítaný popis mapy
			char *szMapDescription = m_strMapDescription.GetBufferSetLength ( nMapDescriptionSize );
			// naète popis mapy
			storage.Read ( szMapDescription, nMapDescriptionSize );
			// zkontroluje délku popisu mapy
			for ( int nCharacterIndex = nMapDescriptionSize; nCharacterIndex-- > 0; )
			{
				// zkontroluje další znak popisu mapy
				LOAD_ASSERT ( szMapDescription[nCharacterIndex] != 0 );
			}
			// ukonèí naèítání popisu mapy
			m_strMapDescription.ReleaseBuffer ( nMapDescriptionSize );
		}

		// naète verzi mapy
		storage >> m_dwMapVersion;

		// naète poèet civilizací
		storage >> m_dwCivilizationCount;
		LOAD_ASSERT ( m_dwCivilizationCount > 0 );
		LOAD_ASSERT ( m_dwCivilizationCount <= CIVILIZATION_COUNT_MAX );

	// naète informace z mapy

		// archiv mapy
		CDataArchive cMapArchive;

		// zjistí, je-li jméno preferovaného archivu mapy platné
		if ( strMapArchiveName.IsEmpty () )
		{	// jméno preferovaného archivu mapy není platné
			// použije uložené jméno archivu mapy
			strMapArchiveName = m_strMapArchiveName;
		}

		// pokusí se otevøít archiv mapy
		if ( !g_cSFileManager.OpenMap ( cMapArchive, m_strMapArchiveName, 
			m_dwMapVersion ) )
		{	// nepodaøilo se otevøít archiv mapy
			// vyplní jméno archivu mapy
			strMapArchiveName = m_strMapArchiveName;
			// vrátí pøíznak špatné verze archivu mapy
			return FALSE;
		}

	// naète potøebný kus mapy
		{
		// naète data mapy ze souboru mapy

			// otevøe soubor mapy
			CArchiveFile cMapFile = cMapArchive.CreateFile ( MAP_FILE_NAME, 
				CFile::modeRead | CFile::shareDenyWrite );

			// poèet knihoven Mapexù
			DWORD dwMapexLibraryCount;
			// poèet knihoven jednotek
			DWORD dwUnitTypeLibraryCount;
			// poèet jednotek na mapì
			DWORD dwUnitCount;
			// poèet ScriptSetù
			DWORD dwScriptSetCount;
			// poèet grafù pro hledání cest
			DWORD dwFindPathGraphCount;

		// naète hlavièku verze souboru
			{
				SFileVersionHeader sFileVersionHeader;
				LOAD_ASSERT ( cMapFile.Read ( &sFileVersionHeader, sizeof ( sFileVersionHeader ) ) == sizeof ( sFileVersionHeader ) );

				// identifikátor souboru mapy
				BYTE aMapFileID[16] = MAP_FILE_IDENTIFIER;
				ASSERT ( sizeof ( aMapFileID ) == sizeof ( sFileVersionHeader.m_aMapFileID ) );

				// zkontroluje identifikátor souboru mapy
				for ( int nIndex = 16; nIndex-- > 0; )
				{
					LOAD_ASSERT ( aMapFileID[nIndex] == sFileVersionHeader.m_aMapFileID[nIndex] );
				}

				ASSERT ( CURRENT_MAP_FILE_VERSION >= COMPATIBLE_MAP_FILE_VERSION );
				// zkontroluje verze formátu mapy
				LOAD_ASSERT ( sFileVersionHeader.m_dwFormatVersion == m_dwMapFormatVersion );
				LOAD_ASSERT ( sFileVersionHeader.m_dwCompatibleFormatVersion == m_dwMapCompatibleFormatVersion );
				LOAD_ASSERT ( m_dwMapFormatVersion >= m_dwMapCompatibleFormatVersion );

				// zjistí, jedná-li se o správnou verzi formátu mapy
				if ( m_dwMapFormatVersion != CURRENT_MAP_FILE_VERSION )
				{	// nejedná se o správnou verzi formátu mapy
					// zjistí, jedná-li se o starou verzi formátu mapy
					if ( m_dwMapFormatVersion < CURRENT_MAP_FILE_VERSION )
					{	// jedná se o starou verzi formátu mapy
						// zkontroluje kompatabilitu verze formátu mapy
						LOAD_ASSERT ( m_dwMapFormatVersion >= COMPATIBLE_MAP_FILE_VERSION );
					}
					else
					{	// jedná se o mladší verzi formátu mapy
						// zkontroluje kompatabilitu verze formátu mapy
						LOAD_ASSERT ( m_dwMapCompatibleFormatVersion <= CURRENT_MAP_FILE_VERSION );
					}
				}
			}

		// naète hlavièku mapy
			{
				SMapHeader sMapHeader;
				LOAD_ASSERT ( cMapFile.Read ( &sMapHeader, sizeof ( sMapHeader ) ) == sizeof ( sMapHeader ) );

			// zpracuje hlavièku mapy

				// zkontroluje velikost mapy
				LOAD_ASSERT ( sMapHeader.m_dwWidth == m_sizeMapMapCell.cx );
				LOAD_ASSERT ( sMapHeader.m_dwHeight == m_sizeMapMapCell.cy );

				// zkontroluje jméno mapy
				LOAD_ASSERT ( m_strMapName == sMapHeader.m_pName );
				LOAD_ASSERT ( m_strMapDescription == sMapHeader.m_pDescription );

				// zkontroluje poèet knihoven Mapexù
				dwMapexLibraryCount = sMapHeader.m_dwMapexLibraryCount;
				LOAD_ASSERT ( dwMapexLibraryCount > 0 );

				// zkontroluje poèet civilizací
				LOAD_ASSERT ( m_dwCivilizationCount == sMapHeader.m_dwCivilizationsCount );

				// zkontroluje poèet knihoven typù jednotek
				dwUnitTypeLibraryCount = sMapHeader.m_dwUnitTypeLibraryCount;
				LOAD_ASSERT ( dwUnitTypeLibraryCount > 0 );

				// zjistí poèet jednotek na mapì
				dwUnitCount = sMapHeader.m_dwUnitCount;

				// zkontroluje verzi mapy
				LOAD_ASSERT ( m_dwMapVersion == sMapHeader.m_dwMapVersion );

				// zjistí poèet ScriptSetù
				dwScriptSetCount = sMapHeader.m_dwScriptSetCount;
				LOAD_ASSERT ( dwScriptSetCount > 0 );

				// zjistí poèet grafù pro hledání cest
				dwFindPathGraphCount = sMapHeader.m_dwFindPathGraphCount;
			}

		// nahraje hlavièky ScriptSetù
			{
				SScriptSetHeader sScriptSetHeader;

				// tabulka pøekladu ID na jméno ScriptSetu
				CMap<DWORD, DWORD, CString, CString &> cScriptSetTable;

				for ( DWORD dwIndex = dwScriptSetCount; dwIndex-- > 0; )
				{
					// pokusí se naèíst hlavièku ScriptSetu
					LOAD_ASSERT ( cMapFile.Read ( &sScriptSetHeader, sizeof ( sScriptSetHeader ) ) == sizeof ( sScriptSetHeader ) );

					// zkontroluje délku jména ScriptSetu
					int nScriptSetNameLength = strlen ( sScriptSetHeader.m_pFileName );
					LOAD_ASSERT ( ( nScriptSetNameLength > 0 ) && ( nScriptSetNameLength < 
						sizeof ( sScriptSetHeader.m_pFileName ) ) );

					// jméno nalezeného ScriptSetu
					CString strFoundScriptSetName;
					// ID nalezeného ScriptSetu
					DWORD dwFoundScriptSetID;
					// pozice ScriptSetu v tabulce ScriptSetù
					POSITION posScriptSet = cScriptSetTable.GetStartPosition ();
					// jméno ScriptSetu
					CString strScriptSetName ( sScriptSetHeader.m_pFileName );

					// zkontroluje unikátnost ID a jména ScriptSetu
					while ( posScriptSet != NULL )
					{
						// zjistí informace o dalším ScriptSetu v tabulce
						cScriptSetTable.GetNextAssoc ( posScriptSet, dwFoundScriptSetID, 
							strFoundScriptSetName );

						// zkontroluje unikátnost ID a jména ScriptSetu
						LOAD_ASSERT ( ( strFoundScriptSetName.CompareNoCase ( 
							strScriptSetName ) != 0 ) && sScriptSetHeader.m_dwID != 
							dwFoundScriptSetID );
					}

					// pøidá záznam o ScriptSetu do tabulky ScriptSetù
					cScriptSetTable.SetAt ( sScriptSetHeader.m_dwID, strScriptSetName );

					// nechá nahrát ScriptSet
					g_cSFileManager.LoadScriptSet ( cMapArchive, strScriptSetName );
				}
			}

		// pøeskoèí odkazy na knihovny Mapexù, hlavièky civilizací, odkazy na knihovny 
		//		typù jednotek a pole offsetù MapSquarù
			{
				// zjistí velikost pøeskakovaných dat
				DWORD dwSkippedDataSize = 
					dwMapexLibraryCount * sizeof ( SMapexLibraryNode ) + 
					m_dwCivilizationCount * sizeof ( SCivilizationHeader ) + 
					dwUnitTypeLibraryCount * sizeof ( SUnitTypeLibraryNode ) + 
					m_sizeMapMapSquare.cx * m_sizeMapMapSquare.cy * sizeof ( DWORD );

				// pøeskoèí data
				LOAD_ASSERT ( cMapFile.GetPosition () + dwSkippedDataSize <= 
					cMapFile.GetLength () );
				(void)cMapFile.Seek ( dwSkippedDataSize, CFile::current );
			}

		// naète informace o MapSquarech
			{
				// index MapSquaru
				CPointDW pointIndex;

				// naète MapSquary
				for ( pointIndex.y = 0; pointIndex.y < m_sizeMapMapSquare.cy; pointIndex.y++ )
				{
					for ( pointIndex.x = 0; pointIndex.x < m_sizeMapMapSquare.cx; 
						pointIndex.x++ )
					{
						// naète další MapSquare
						GetMapSquareFromIndex ( pointIndex )->Create ( cMapFile );
					}
				}
			}

		// pøeskoèí hlavièky jednotek
			{
				// zjistí velikost hlavièek jednotek
				DWORD dwUnitHeaderSize = dwUnitCount * sizeof ( SUnitHeader );

				// pøeskoèí hlavièky jednotek
				LOAD_ASSERT ( cMapFile.GetPosition () + dwUnitHeaderSize <= 
					cMapFile.GetLength () );
				(void)cMapFile.Seek ( dwUnitHeaderSize, CFile::current );
			}

		// naète jména surovin
			{
				SResource sResource;

				// zjistí velikost ikony suroviny
				DWORD dwResourceIconSize = RESOURCE_ICON_WIDTH * 
					RESOURCE_ICON_HEIGHT * sizeof ( DWORD );

				// projede suroviny
				for ( DWORD dwResourceIndex = 0; dwResourceIndex < RESOURCE_COUNT; 
					dwResourceIndex++ )
				{
					// naète surovinu
					LOAD_ASSERT ( cMapFile.Read ( &sResource, sizeof ( sResource ) ) == sizeof ( sResource ) );

					// zkontroluje délku jména suroviny
					m_aResourceName[dwResourceIndex] = sResource.m_szName;
					LOAD_ASSERT ( m_aResourceName[dwResourceIndex].GetLength () < sizeof ( sResource.m_szName ) );

					// pøeskoèí ikonu suroviny
					LOAD_ASSERT ( cMapFile.GetPosition () + dwResourceIconSize <= 
						cMapFile.GetLength () );
					(void)cMapFile.Seek ( dwResourceIconSize, CFile::current );
				}
			}

		// naète jména neviditelností
			{
				SInvisibility sInvisibility;

				// projede neviditelnosti
				for ( DWORD dwInvisibilityIndex = 0; 
					dwInvisibilityIndex < INVISIBILITY_COUNT; dwInvisibilityIndex++ )
				{
					// naète neviditelnost
					LOAD_ASSERT ( cMapFile.Read ( &sInvisibility, sizeof ( sInvisibility ) ) == sizeof ( sInvisibility ) );

					// zkontroluje délku jména neviditelnosti
					m_aInvisibilityName[dwInvisibilityIndex] = sInvisibility.m_szName;
					LOAD_ASSERT ( m_aInvisibilityName[dwInvisibilityIndex].GetLength () < sizeof ( sInvisibility.m_szName ) );
				}
			}

		// naète hlavièky grafù pro hledání cesty
			{
				SFindPathGraphHeader sFindPathGraphHeader;

				// vytvoøí pole grafù pro hledání cesty
				CSFindPathGraph::PreCreateFindPathGraphs ( dwFindPathGraphCount );

				// naète hlavièky grafù pro hledání cesty
				for ( ; dwFindPathGraphCount-- > 0; )
				{
					// naète hlavièku grafu pro hledání cest
					LOAD_ASSERT ( cMapFile.Read ( &sFindPathGraphHeader, sizeof ( sFindPathGraphHeader ) ) == sizeof ( sFindPathGraphHeader ) );

					// zkontroluje délku jména grafu pro hledání cesty
					int nFindPathGraphNameLength = strlen ( sFindPathGraphHeader.m_pName );
					LOAD_ASSERT ( ( nFindPathGraphNameLength > 0 ) && 
						( nFindPathGraphNameLength < 
						sizeof ( sFindPathGraphHeader.m_pName ) ) );

					// vytvoøí graf pro hledání cesty
					(void)CSFindPathGraph::CreateFindPathGraph ( dwFindPathGraphCount, 
						sFindPathGraphHeader.m_dwID, sFindPathGraphHeader.m_pName, 
						cMapArchive );
				}
			}

		// ukonèí naèítání souboru mapy

			// zkontroluje konec souboru
			LOAD_ASSERT ( cMapFile.GetPosition () == cMapFile.GetLength () );
			cMapFile.Close ();

		// ukonèí vytváøení grafù pro hledání cesty

			// ukonèí vytváøení grafù pro hledání cesty
			CSFindPathGraph::PostCreateFindPathGraphs ();

		// vytvoøí data cest

			// vytvoøí data cest
			CSPath::CreatePathes ( cMapArchive );

		// vytvoøí mapy jednotek

			// vytvoøí mapy jednotek
			CSMapSquare::CreateUnitMaps ();
		}

	// naète civilizace

		// vytvoøí pole civilizací
		m_pCivilizations = new CSCivilization[m_dwCivilizationCount];

		// zjistí ukazatel na první civilizaci
		CSCivilization *pCivilization = m_pCivilizations;

		// projede všechny civilizace
		for ( DWORD dwCivilizationIndex = 0; dwCivilizationIndex < m_dwCivilizationCount; 
			dwCivilizationIndex++, pCivilization++ )
		{
			// naète starý ukazatel na civilizaci
			storage >> pSavedPointer;
			LOAD_ASSERT ( pSavedPointer != NULL );
			// zaregistruje ukazatel na civilizaci
			storage.RegisterPointer ( pSavedPointer, pCivilization );

			// naète civilizaci
			pCivilization->PersistentLoad ( storage );

			// inicializuje index civilizace
			pCivilization->SetCivilizationIndex ( dwCivilizationIndex );

			// zjistí, jedná-li se o systémovou civilizaci
			if ( dwCivilizationIndex == 0 )
			{	// jedná se o systémovou civilizaci
				// zkontroluje naètení systémové civilizace
				LOAD_ASSERT ( pCivilization->GetCivilizationID () == 0 );
			}

			// zkontroluje unikátnost jména a ID civilizace
			for ( DWORD dwIndex = 0; dwIndex < dwCivilizationIndex; dwIndex++ )
			{
				// zkontroluje unikátnost jména civilizace
				LOAD_ASSERT ( pCivilization->GetCivilizationName () != m_pCivilizations[dwIndex].GetCivilizationName () );
				// zkontroluje unikátnost ID civilizace
				LOAD_ASSERT ( pCivilization->GetCivilizationID () != m_pCivilizations[dwIndex].GetCivilizationID () );
			}

		// naète DPC frontu civilizace

			// veliksot DPC fronty civilizace
			DWORD dwDPCCount;
			// naète velikost DPC fronty civilizace
			storage >> dwDPCCount;

			ASSERT ( m_aDPCQueue[dwCivilizationIndex].IsEmpty () );

			// naète DPC frontu civilizace
			for ( ; dwDPCCount-- > 0; )
			{
				// naète starý ukazatel na DPC
				storage >> pSavedPointer;
				LOAD_ASSERT ( pSavedPointer != NULL );

				// DPC ID
				DWORD dwDPCID;
				// naète DPC ID
				storage >> dwDPCID;

				// vytvoøí DPC z ID
				CSDeferredProcedureCall *pDPC = CSDeferredProcedureCall::CreateChildByID ( dwDPCID );
				LOAD_ASSERT ( pDPC != NULL );

				// zaregistruje ukazatel na DPC
				storage.RegisterPointer ( pSavedPointer, pDPC );

				// naète DPC
				pDPC->PersistentLoad ( storage );

				// pøidá DPC do DPC fronty civilizace
				m_aDPCQueue[dwCivilizationIndex].Add ( pDPC );
			}
		}

		// vytvoøí memory pool polí posledních pozic, kde byla jednotka civilizacemi vidìna
		CSUnit::m_cCivilizationLastSeenPositionArrayMemoryPool.Create ( 
			max ( m_dwCivilizationCount * sizeof ( CSPosition ), sizeof ( BYTE * ) ) );

	// naète jednotky na mapì

		// poèet jednotek
		DWORD dwUnitCount;
		// naète poèet jednotek
		storage >> dwUnitCount;

#ifdef _DEBUG
		// uschová ID MainLoop threadu
		DWORD dwMainLoopThreadID = m_dwMainLoopThreadID;
		// nasimuluje MainLoopThread
		m_dwMainLoopThreadID = GetCurrentThreadId ();
#endif //_DEBUG

		// projede všechny uložené jednotky
		for ( ; dwUnitCount > 0; dwUnitCount-- )
		{
			// vytvoøí jednotku
			CSUnit *pUnit = new CSUnit;
			// naète starý ukazatel na jednotku
			storage >> pSavedPointer;
			LOAD_ASSERT ( pSavedPointer != NULL );
			// zaregistruje ukazatel na jednotku
			storage.RegisterPointer ( pSavedPointer, pUnit );

			// naète jednotku
			pUnit->PersistentLoad ( storage, m_dwCivilizationCount );

			// pøidá vytvoøenou jednotku do jednotek mapy
			AddUnit ( pUnit );
		}

#ifdef _DEBUG
		// vrátí uschované ID MainLoop threadu
		m_dwMainLoopThreadID = dwMainLoopThreadID;
#endif //_DEBUG

	// naète frontu mrtvých jednotek

		// poèet mrtvých jednotek
		DWORD dwDeadUnitCount;
		// naète poèet mrtvých jednotek
		storage >> dwDeadUnitCount;

		// naète frontu mrtvých jednotek
		for ( ; dwDeadUnitCount-- > 0; )
		{
			// popis mrtvé jednotky
			struct SDeadUnit sDeadUnit;

			// naète popis mrtvé jednotky
			storage >> sDeadUnit.dwTimeSlice;
			// vytvoøí jednotku
			sDeadUnit.pUnit = new CSUnit;
			// naète starý ukazatel na jednotku
			storage >> pSavedPointer;
			LOAD_ASSERT ( pSavedPointer != NULL );
			// zaregistruje ukazatel na jednotku
			storage.RegisterPointer ( pSavedPointer, sDeadUnit.pUnit );

			// naète jednotku
			sDeadUnit.pUnit->PersistentLoad ( storage, m_dwCivilizationCount );

			// inicializuje jednotce ukazatel na zámek
			sDeadUnit.pUnit->m_pLock = &m_cDeadDeletedUnitLock;

			// pøidá naètenou jednotku do fronty mrtvých jednotek
			m_cDeadUnitQueue.Add ( sDeadUnit );
		}

	// naète seznam smazaných jednotek

		// poèet smazaných jednotek
		DWORD dwDeletedUnitCount;

		// naète poèet smazaných jednotek
		storage >> dwDeletedUnitCount;

		// naète seznam smazaných jednotek
		for ( ; dwDeletedUnitCount-- > 0; )
		{
			// vytvoøí jednotku
			CSUnit *pUnit = new CSUnit;
			// naète starý ukazatel na jednotku
			storage >> pSavedPointer;
			LOAD_ASSERT ( pSavedPointer != NULL );
			// zaregistruje ukazatel na jednotku
			storage.RegisterPointer ( pSavedPointer, pUnit );

			// naète jednotku
			pUnit->PersistentLoad ( storage, m_dwCivilizationCount );

			// inicializuje jednotce ukazatel na zámek
			pUnit->m_pLock = &m_cDeadDeletedUnitLock;

			// pøidá naètenou jednotku do seznamu smazaných jednotek
			m_cDeletedUnitList.Add ( pUnit );
		}

	// naète pole uživatelù

		// naète poèet uživatelù
		DWORD dwUserCount;
		storage >> dwUserCount;

		// projede pole uživatelù
		for ( DWORD dwIndex = dwUserCount; dwIndex-- > 0; )
		{
			// vytvoøí nový popis uživatele
			struct SUserDescription *pUserDescription = new SUserDescription;

			// délka jména uživatele
			int nUserNameSize;
			// naète délku jména uživatele
			storage >> nUserNameSize;
			LOAD_ASSERT ( nUserNameSize > 0 );
			// získá ukazatel na naèítané jméno uživatele
			char *szUserName = pUserDescription->strName.GetBufferSetLength ( nUserNameSize );
			// naète jméno uživatele
			storage.Read ( szUserName, nUserNameSize );
			// zkontroluje délku jména uživatele
			for ( int nCharacterIndex = nUserNameSize; nCharacterIndex-- > 0; )
			{
				// zkontroluje další znak jména uživatele
				LOAD_ASSERT ( szUserName[nCharacterIndex] != 0 );
			}
			// ukonèí naèítání jména uživatele
			pUserDescription->strName.ReleaseBuffer ( nUserNameSize );

			// délka hesla uživatele
			int nUserPasswordSize;
			// naète délku hesla uživatele
			storage >> nUserPasswordSize;
			// zjistí, je-li heslo uživatele prázdné
			if ( nUserPasswordSize > 0 )
			{	// heslo uživatele není prázdné
				// získá ukazatel na naèítané heslo uživatele
				char *szUserPassword = pUserDescription->strPassword.GetBufferSetLength ( nUserPasswordSize );
				// naète heslo uživatele
				storage.Read ( szUserPassword, nUserPasswordSize );
				// zkontroluje délku hesla uživatele
				for ( int nCharacterIndex = nUserPasswordSize; nCharacterIndex-- > 0; )
				{
					// zkontroluje další znak hesla uživatele
					LOAD_ASSERT ( szUserPassword[nCharacterIndex] != 0 );
				}
				// ukonèí naèítání hesla uživatele
				pUserDescription->strPassword.ReleaseBuffer ( nUserPasswordSize );
			}

			// naète index civilizace
			storage >> pUserDescription->dwCivilizationIndex;
			LOAD_ASSERT ( pUserDescription->dwCivilizationIndex > 0 );
			LOAD_ASSERT ( pUserDescription->dwCivilizationIndex < m_dwCivilizationCount );

			// naète pøíznak povolení
			storage >> pUserDescription->bEnabled;

			pUserDescription->bLoggedOn = FALSE;

			// pøidá uživatele do tabulky uživatelù
			m_aUserDescriptions.InsertAt ( 0, pUserDescription );
		}

	// naète grafy pro hledání cesty

		// naète grafy pro hledání cesty
		CSFindPathGraph::PersistentLoadFindPathGraphs ( storage );

	// naète cesty

		// naète cesty
		CSPath::PersistentLoadPathes ( storage, m_dwCivilizationCount );

	// naète øetìzce hry

		// naète øetìzce hry
		g_StringTable.PersistentLoad ( storage );

	// naète statická data

		// naète statická data
		CSOneInstance::PersistentLoad ( storage );
	}
	catch ( CException * )
	{
		// zneškodní èasteènì inicializovaná data mapy
		SetEmptyData ();

		// pokraèuje ve zpracovávání výjimky
		throw;
	}

	// vrátí pøíznak úspìšného naètení mapy
	return TRUE;
}

// pøeklad ukazatelù
void CSMap::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	try
	{
	// pøeklad ukazatelù civilizací

		// zjistí ukazatel na první civilizaci
		CSCivilization *pCivilization = m_pCivilizations;

		// projede všechny civilizace
		for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; 
			pCivilization++ )
		{
			// pøeloží ukazatele civilizace
			pCivilization->PersistentTranslatePointers ( storage );

		// pøeloží ukazatele DPC fronty civilizace

			// zjistí pozici prvního prvku v DPC frontì civilizace
			POSITION posDPC = m_aDPCQueue[dwCivilizationIndex].GetHeadPosition ();
			// ukazatel na DPC
			CSDeferredProcedureCall *pDPC;

			// projede DPC frontu civilizace
			while ( CSDeferredProcedureCallQueue::GetNext ( posDPC, pDPC ) )
			{
				// pøeloží ukazatele DPC
				pDPC->PersistentTranslatePointers ( storage );
			}
		}

	// pøeklad ukazatelù jednotek

		// projede všechny bloky jednotek
		for ( SUnitBlock *pBlock = m_pFirstUnitBlock; pBlock != NULL; pBlock = pBlock->pNext )
		{
			// ukazatel na jednotku v bloku
			CSUnit **pBlockUnit = pBlock->pUnits;
			// projede všechny jednotky v bloku jednotek
			for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
			{
				// zjistí, je-li to platná jednotka
				if ( *pBlockUnit != NULL )
				{	// je to platná jednotka
					// pøeloží ukazatele jednotky
					(*pBlockUnit)->PersistentTranslatePointers ( storage );
				}
			}
		}

	// pøeklad ukazatelù mrtvých jednotek

		// popis mrtvé jednotky
		struct SDeadUnit sDeadUnit;
		// pozice mrtvé jednotky ve frontì mrtvých jednotek
		POSITION posDeadUnit = m_cDeadUnitQueue.GetHeadPosition ();

		// projede frontu mrtvých jednotek
		while ( m_cDeadUnitQueue.GetNext ( posDeadUnit, sDeadUnit ) )
		{
			// pøeloží ukazatele mrtvé jednotky
			sDeadUnit.pUnit->PersistentTranslatePointers ( storage );
		}

	// pøeklad ukazatelù smazaných jednotek

		// ukazatel na smazanou jednotku
		CSUnit *pDeletedUnit;
		// pozice smazané jednotky v seznamu smazaných jednotek
		POSITION posDeletedUnit = m_cDeletedUnitList.GetHeadPosition ();

		// projede seznam smazaných jednotek
		while ( m_cDeletedUnitList.GetNext ( posDeletedUnit, pDeletedUnit ) )
		{
			// pøeloží ukazatele smazané jednotky
			pDeletedUnit->PersistentTranslatePointers ( storage );
		}

	// pøeklad ukazatelù grafù pro hledání cesty

		// pøeklad ukazatelù grafù pro hledání cesty
		CSFindPathGraph::PersistentTranslatePointersFindPathGraphs ( storage );

	// pøeklad ukazatelù cest

		// pøeklad ukazatelù cest
		CSPath::PersistentTranslatePointersPathes ( storage );

	// pøeklad ukazatelù øetìzcù hry

		// pøeloží ukazatele øetìzcù hry
		g_StringTable.PersistentTranslatePointers ( storage );

	// pøeklad ukazatelù statických dat

		// pøeklad ukazatelù statických dat
		CSOneInstance::PersistentTranslatePointers ( storage );
	}
	catch ( CException * )
	{
		// zneškodní èasteènì inicializovaná data mapy
		SetEmptyData ();

		// pokraèuje ve zpracovávání výjimky
		throw;
	}
}

// inicializace nahraného objektu
void CSMap::PersistentInit () 
{
	try
	{
		// inicializuje index rohového MapSquaru
		m_pointCornerMapSquare = CPointDW ( m_sizeMapMapSquare.cx - 1, 
			m_sizeMapMapSquare.cy - 1 );
		m_pointCornerMapCell = CPointDW ( m_sizeMapMapCell.cx - 1, 
			m_sizeMapMapCell.cy - 1 );

	// inicializace civilizací

		// zjistí ukazatel na první civilizaci
		CSCivilization *pCivilization = m_pCivilizations;

		// projede všechny civilizace
		for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; 
			pCivilization++ )
		{
			// inicializuje civilizaci
			pCivilization->PersistentInit ();

		// inicializuje DPC frontu civilizace

			// zjistí pozici prvního prvku v DPC frontì civilizace
			POSITION posDPC = m_aDPCQueue[dwCivilizationIndex].GetHeadPosition ();
			// ukazatel na DPC
			CSDeferredProcedureCall *pDPC;

			// projede DPC frontu civilizace
			while ( CSDeferredProcedureCallQueue::GetNext ( posDPC, pDPC ) )
			{
				// inicializuje DPC
				pDPC->PersistentInit ();
			}
		}

	// inicializace jednotek a umístìní jednotek na mapu

		// vytvoøí pole pøíznakù objevení pozic na mapì pro metodu "GetNearestUnitPlace"
		m_cGetNearestUnitPlacePositionArray.Create ( m_sizeMapMapCell.cx, 
			m_sizeMapMapCell.cy, 2, 0 );

		// vytvoøí memory pool polí viditelnosti jednotky civilizacemi
		CSUnit::m_cCivilizationVisibilityArrayMemoryPool.Create ( 
			max ( m_dwCivilizationCount * sizeof ( WORD ), sizeof ( BYTE * ) ) );

		// projede všechny bloky jednotek
		for ( SUnitBlock *pBlock = m_pFirstUnitBlock; pBlock != NULL; pBlock = pBlock->pNext )
		{
			// ukazatel na jednotku v bloku
			CSUnit **pBlockUnit = pBlock->pUnits;
			// projede všechny jednotky v bloku jednotek
			for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
			{
				// zjistí, je-li to platná jednotka
				if ( *pBlockUnit != NULL )
				{	// je to platná jednotka
					// inicializuje jednotku
					(*pBlockUnit)->PersistentInit ();
				}
			}
		}

#ifdef _DEBUG
		// uschová ID MainLoop threadu
		DWORD dwMainLoopThreadID = m_dwMainLoopThreadID;
		// nasimuluje MainLoopThread
		m_dwMainLoopThreadID = GetCurrentThreadId ();
#endif //_DEBUG

		// projede všechny bloky jednotek
		for ( pBlock = m_pFirstUnitBlock; pBlock != NULL; pBlock = pBlock->pNext )
		{
			// ukazatel na jednotku v bloku
			CSUnit **pBlockUnit = pBlock->pUnits;
			// projede všechny jednotky v bloku jednotek
			for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
			{
				// zjistí, je-li to platná jednotka
				if ( *pBlockUnit != NULL )
				{	// je to platná jednotka
					// inicializuje seznam jednotkou vidìných jednotek
					(*pBlockUnit)->PersistentInitSeenUnitList ();
					// zpracuje pøidání jednotky na mapu
					LOAD_ASSERT ( CanPlaceUnitOnce ( *pBlockUnit, 
						(*pBlockUnit)->GetPosition (), TRUE ) );
					UnitPlaced ( *pBlockUnit );
				}
			}
		}

#ifdef _DEBUG
		// vrátí uschované ID MainLoop threadu
		m_dwMainLoopThreadID = dwMainLoopThreadID;
#endif //_DEBUG

	// inicilazace mrtvých jednotek

		// popis mrtvé jednotky
		struct SDeadUnit sDeadUnit;
		// pozice mrtvé jednotky ve frontì mrtvých jednotek
		POSITION posDeadUnit = m_cDeadUnitQueue.GetHeadPosition ();

		// projede frontu mrtvých jednotek
		while ( m_cDeadUnitQueue.GetNext ( posDeadUnit, sDeadUnit ) )
		{
			// inicializuje mrtvou jednotku
			sDeadUnit.pUnit->PersistentInit ( TRUE );
		}

	// inicializace smazaných jednotek

		// ukazatel na smazanou jednotku
		CSUnit *pDeletedUnit;
		// pozice smazané jednotky v seznamu smazaných jednotek
		POSITION posDeletedUnit = m_cDeletedUnitList.GetHeadPosition ();

		// projede seznam smazaných jednotek
		while ( m_cDeletedUnitList.GetNext ( posDeletedUnit, pDeletedUnit ) )
		{
			// inicializuje smazanou jednotku
			pDeletedUnit->PersistentInit ( TRUE );
		}

	// inicializace grafù pro hledání cesty

		// inicializace grafù pro hledání cesty
		CSFindPathGraph::PersistentInitFindPathGraphs ();

	// inicializace cest

		// inicializace cest
		CSPath::PersistentInitPathes ();

	// inicializace øetìzcù hry

		// inicializuje øetìzce hry
		g_StringTable.PersistentInit ();

	// inicializace statických dat

		// inicializace statických dat
		CSOneInstance::PersistentInit ();

	// inicializace notifikací

		// inicializuje notifikace
		InitializeNotifications ( FALSE );

	// inicializace hry

		// inicializuje hru (vytvoøí zapausovanou hru)
		InitGame ();
	}
	catch ( CException * )
	{
		// zneškodní èasteènì inicializovaná data mapy
		SetEmptyData ();

		// pokraèuje ve zpracovávání výjimky
		throw;
	}
}

//////////////////////////////////////////////////////////////////////
// Operace s mapou
//////////////////////////////////////////////////////////////////////

// definice typu pro alokaci pole ukazatelù na jednotku
typedef CSUnit *TSUnitPointer;

// vytvoøí nový blok jednotek (lze volat jen z MainLoop threadu)
// výjimky: CMemoryException
void CSMap::CreateUnitBlock () 
{
	ASSERT ( IsMainLoopThread () );

	ASSERT ( ( m_pFirstUnitBlock == NULL ) || ( m_pFirstUnitBlock->pPrevious == NULL ) );
	ASSERT ( ( m_pLastUnitBlock == NULL ) || ( m_pLastUnitBlock->pNext == NULL ) );

	// vytvoøí nový blok jednotek
	SUnitBlock *pBlock = new SUnitBlock;

	// vytvoøí nové pole ukazatelù na jednotky
	CSUnit **pUnitReference = pBlock->pUnits = new TSUnitPointer[UNITS_IN_BLOCK];

	// inicializuje odkazy na jednotky v bloku a jednotky pøidá do seznamu volných jednotek
	for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pUnitReference++ )
	{
		// vynuluje informaci o jednotce
		*pUnitReference = NULL;

		// vytvoøí informaci o volné jednotce
		SFreeUnit *pFreeUnit = new SFreeUnit;
		// vyplní informace o volné jednotce
		pFreeUnit->nIndex = nIndex;
		pFreeUnit->pBlock = pBlock;
		pFreeUnit->pNext = m_pFirstFreeUnit;

		// pøidá volnou jednotku na zaèátek spojáku volných jednotek
		m_pFirstFreeUnit = pFreeUnit;
	}

	// inicializuje blok jednotek
	pBlock->pNext = m_pFirstUnitBlock;
	pBlock->pPrevious = NULL;

// pøidá blok jednotek do spojáku blokù jednotek

	// zjistí, je-li spoják prázdný
	if ( m_pFirstUnitBlock == NULL )
	{	// spoják blokù jednotek je prázdný
		ASSERT ( m_pLastUnitBlock == NULL );
		// aktualizuje ukazatel na poslední blok jednotek
		m_pLastUnitBlock = pBlock;
	}
	else
	{	// spoják blokù jednotek není prázdný
		// pøidá blok jednotek pøed první blok jednotek
		m_pFirstUnitBlock->pPrevious = pBlock;
	}
	// aktualizuje ukazatel na první blok jednotek
	m_pFirstUnitBlock = pBlock;

// inicializuje ID privilegovaného threadu zámku bloku jednotek
#ifdef _DEBUG
	// zjistí, je-li MainLoop thread spuštìn
	if ( m_pMainLoopThread != NULL )
	{	// MainLoop thread je puštìn
		// inicializuje ID privilegovaného threadu zámku bloku jednotek
		pBlock->cLock.SetPrivilegedThreadID ( m_dwMainLoopThreadID );
	}
#endif //_DEBUG
}

// pøidá jednotku "pUnit" do jednotek mapy a aktualizuje její ukazatel na zámek
//		(lze volat jen z MainLoop threadu)
// výjimky: CMemoryException
void CSMap::AddUnit ( CSUnit *pUnit ) 
{
	ASSERT ( IsMainLoopThread () );

	ASSERT ( pUnit != NULL );
	ASSERT ( pUnit->m_pLock == NULL );

	// zjistí, je-li nìjaké místo pro jednotku volné
	if ( m_pFirstFreeUnit == NULL )
	{	// není volné místo pro jednotku
		// nechá vytvoøit nový blok jednotek
		CreateUnitBlock ();
	}

	ASSERT ( m_pFirstFreeUnit != NULL );
	ASSERT ( ( m_pFirstUnitBlock != NULL ) && ( m_pFirstUnitBlock->pPrevious == NULL ) );
	ASSERT ( ( m_pLastUnitBlock != NULL ) && ( m_pLastUnitBlock->pNext == NULL ) );

	ASSERT ( m_pFirstFreeUnit->pBlock->pUnits[m_pFirstFreeUnit->nIndex] == NULL );

	// vezme první volné místo pro jednotku
	SFreeUnit *pFreeUnit = m_pFirstFreeUnit;
	// umístí jednotku na první volné místo
	pFreeUnit->pBlock->pUnits[pFreeUnit->nIndex] = pUnit;
	// aktualizuje ukazatel na zámek jednotky
	pUnit->m_pLock = &pFreeUnit->pBlock->cLock;

	// aktualizuje první volnou jednotku
	m_pFirstFreeUnit = pFreeUnit->pNext;
	// znièí inforamci o volné jednotce, která byla právì obsazena
	delete pFreeUnit;
}

// smaže jednotku "pUnit" z jednotek mapy (lze volat jen z MainLoop threadu)
// výjimky: CMemoryException
void CSMap::DeleteUnit ( CSUnit *pUnit ) 
{
	ASSERT ( IsMainLoopThread () );

	ASSERT ( ( m_pFirstUnitBlock != NULL ) && ( m_pFirstUnitBlock->pPrevious == NULL ) );
	ASSERT ( ( m_pLastUnitBlock != NULL ) && ( m_pLastUnitBlock->pNext == NULL ) );

	// najde blok s mazanou jednotkou
	SUnitBlock *pBlock = m_pFirstUnitBlock;
	// prohledá spoják blokù jednotek
	while ( &pBlock->cLock != pUnit->m_pLock )
	{
		// prohledá další blok jednotek
		pBlock = pBlock->pNext;
		ASSERT ( pBlock != NULL );
	}

	// najde mazanou jednotku v bloku
	CSUnit **pBlockUnit = pBlock->pUnits;
	// index jednotky v bloku
	int nIndex = 0;
	// prohledá pole jednotek v bloku
	while ( *pBlockUnit != pUnit )
	{
		// prohledá další jednotku bloku
		pBlockUnit++;
		nIndex++;
		ASSERT ( nIndex < UNITS_IN_BLOCK );
	}

	// zamkne blok jednotek
	VERIFY ( pBlock->cLock.PrivilegedThreadWriterLock () );

	// vymaže jednotku z bloku jednotek
	*pBlockUnit = NULL;

	// odemkne blok jednotek
	pBlock->cLock.PrivilegedThreadWriterUnlock ();

	// vytvoøí volnou jednotku
	SFreeUnit *pFreeUnit = new SFreeUnit;

	// inicializuje volnou jednotku
	pFreeUnit->pBlock = pBlock;
	pFreeUnit->nIndex = nIndex;
	pFreeUnit->pNext = m_pFirstFreeUnit;

	// pøidá volnou jednotku do spojáku volných jednotek
	m_pFirstFreeUnit = pFreeUnit;
}

// vrátí pole MapSquarù, na které jednotka vidí (mimo MainLoop thread musí být jednotka 
//		zamèena pro ètení) (NULL=poslední MapSquare) 
void CSMap::GetSeenMapSquares ( CSUnit *pUnit, CSMapSquare *(&aSeenMapSquares)[4] ) const 
{
	ASSERT ( pUnit != NULL );

	// zjistí, pozici jednotky na mapì
	CPointDW pointPosition = pUnit->GetPosition ();

	// zjistí, je-li jednotka na mapì
	if ( !IsMapPosition ( pointPosition ) )
	{	// jednotka není na mapì
		// vynuluje pole vidìných MapSquarù
		aSeenMapSquares[0] = NULL;
		return;
	}

	// index MapSquaru umístìní jednotky
	CPointDW pointIndex;
	// zjistí index MapSquaru umístìní jednotky a aktualizuje pozici jednotky na mapì 
	//		na relativní pozici vùèi MapSquaru
	pointIndex.x = pointPosition.x / MAP_SQUARE_SIZE;
	pointPosition.x = pointPosition.x % MAP_SQUARE_SIZE;
	pointIndex.y = pointPosition.y / MAP_SQUARE_SIZE;
	pointPosition.y = pointPosition.y % MAP_SQUARE_SIZE;

	// pøidá MapSquare umístìní jednotky do vidìných MapSquarù
	aSeenMapSquares[0] = GetMapSquareFromIndex ( pointIndex );
	// inicializuje index MapSquaru v poli MapSquarù
	int nIndex = 1;

	// index rohovì sousedícího MapSquaru
	CPointDW pointCornerIndex;
	// vzdálenost od rohovì sousedícího MapSquaru
	CSizeDW sizeCornerDistance;

	// zjistí, je-li jednotka v horní èásti MapSquaru
	if ( pointPosition.y < ( MAP_SQUARE_SIZE / 2 ) )
	{	// jednotka je v horní èásti MapSquaru
		// zjistí, je-li MapSquare "nad" na mapì a je-li vidìt
		if ( ( pointIndex.y != 0 ) && ( pointPosition.y < pUnit->GetViewRadius () ) )
		{	// MapSquare "nad" je na mapì a je vidìt
			// pøidá MapSquare "nad" do vidìných MapSquarù
			aSeenMapSquares[1] = GetMapSquareFromIndex ( CPointDW ( pointIndex.x, 
				pointCornerIndex.y = pointIndex.y - 1 ) );
			// aktualizuje index pøíštího viditelného MapSquaru
			nIndex = 2;
			// aktualizuje vzdálenost od rohovì sousedícího MapSquaru
			sizeCornerDistance.cy = pointPosition.y;
		}
	}
	else
	{	// jednotka je v dolní èásti MapSquaru
		// aktualizuje vzdálenost od rohovì sousedícího MapSquaru
		sizeCornerDistance.cy = MAP_SQUARE_SIZE - pointPosition.y;
		// zjistí, je-li MapSquare "pod" na mapì a je-li vidìt
		if ( ( pointIndex.y != m_pointCornerMapSquare.y ) && ( sizeCornerDistance.cy <= 
			pUnit->GetViewRadius () ) )
		{	// MapSquare "pod" je na mapì a je vidìt
			// pøidá MapSquare "pod" do vidìných MapSquarù
			aSeenMapSquares[1] = GetMapSquareFromIndex ( CPointDW ( pointIndex.x, 
				pointCornerIndex.y = pointIndex.y + 1 ) );
			// aktualizuje index pøíštího viditelného MapSquaru
			nIndex = 2;
		}
	}

	// zjistí, je-li jednotka v levé èásti MapSquaru
	if ( pointPosition.x < ( MAP_SQUARE_SIZE / 2 ) )
	{	// jednotka je v levé èásti MapSquaru
		// zjistí, je-li MapSquare "vlevo" na mapì a je-li vidìt
		if ( ( pointIndex.x != 0 ) && ( pointPosition.x < pUnit->GetViewRadius () ) )
		{	// MapSquare "vlevo" je na mapì a je vidìt
			// pøidá MapSquare "vlevo" do vidìných MapSquarù
			aSeenMapSquares[nIndex++] = GetMapSquareFromIndex ( CPointDW ( 
				pointCornerIndex.x = pointIndex.x - 1, pointIndex.y ) );
			// aktualizuje vzdálenost od rohovì sousedícího MapSquaru
			sizeCornerDistance.cx = pointPosition.x;
		}
	}
	else
	{	// jednotka je v pravé èásti MapSquaru
		// aktualizuje vzdálenost od rohovì sousedícího MapSquaru
		sizeCornerDistance.cx = MAP_SQUARE_SIZE - pointPosition.x;
		// zjistí, je-li MapSquare "vpravo" na mapì a je-li vidìt
		if ( ( pointIndex.x != m_pointCornerMapSquare.x ) && ( sizeCornerDistance.cx <= 
			pUnit->GetViewRadius () ) )
		{	// MapSquare "vpravo" je na mapì a je vidìt
			// pøidá MapSquare "vpravo" do vidìných MapSquarù
			aSeenMapSquares[nIndex++] = GetMapSquareFromIndex ( CPointDW ( 
				pointCornerIndex.x = pointIndex.x + 1, pointIndex.y ) );
		}
	}

	// zjistí, má-li pøidávat rohovì sousedící MapSquare
	if ( nIndex == 3 )
	{	// rohovì sousedící MapSquare existuje a mùže být vidìt
		// zjistí, je-li rohovì sousedící MapSquare vidìt
		if ( sizeCornerDistance.cx * sizeCornerDistance.cx + sizeCornerDistance.cy * 
			sizeCornerDistance.cy <= pUnit->GetViewRadiusSquare () )
		{	// rohovì sousedící MapSquare je vidìt
			aSeenMapSquares[3] = GetMapSquareFromIndex ( pointCornerIndex );
		}
		else
		{	// rohový MapSquare není vidìt
			// ukonèí pole MapSquarù
			aSeenMapSquares[3] = NULL;
		}
	}
	else
	{	// rohový MapSquare není vidìt nebo neexistuje
		// ukonèí pole MapSquarù
		aSeenMapSquares[nIndex] = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Informace o høe
//////////////////////////////////////////////////////////////////////

// zjistí jednotku podle ID jednotky (NULL=jednotka neexistuje)
CSUnit *CSMap::GetUnitByID ( DWORD dwID ) 
{
	// projede všechny bloky jednotek (od konce)
	for ( SUnitBlock *pBlock = m_pLastUnitBlock; pBlock != NULL; 
		pBlock = pBlock->pPrevious )
	{
		// ukazatel na jednotku v bloku
		CSUnit **pBlockUnit = pBlock->pUnits;
		// zamkne blok jednotek pro ètení
		VERIFY ( pBlock->cLock.ReaderLock () );
		// projede všechny jednotky v bloku jednotek
		for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
		{
			// získá ukazatel na jednotku
			CSUnit *pUnit = *pBlockUnit;

			// zjistí, je-li to platná jednotka
			if ( pUnit == NULL )
			{	// není to platná jednotka
				continue;
			}

			// zjistí, je-li to hledaná jednotka
			if ( pUnit->GetID () == dwID )
			{	// jedná se o hledanou jednotku
				// odemkne zamèený blok jednotek pro ètení
				pBlock->cLock.ReaderUnlock ();
				// vrátí ukazatel na hledanou jednotku
				return pUnit;
			}
		}
		// byly projety všechny jednotky v bloku jednotek

		// odemkne zamèený blok jednotek pro ètení
		pBlock->cLock.ReaderUnlock ();
	}

	// vrátí pøíznak neexistence hledané jednotky
	return NULL;
}

// pro civilizaci "dwCivilizationIndex" vyplní seznam "cUnitList" všech jednotek 
//		(vlastních i nepøátelských) v oblasti se støedem "pointCenter" o polomìru 
//		"dwRadius" (musí být zamèeny MapSquary, "dwRadius" musí být menší než 
//		MAX_GET_UNITS_IN_AREA_RADIUS)
void CSMap::GetUnitsInAreaAll ( DWORD dwCivilizationIndex, CPointDW pointCenter, 
	DWORD dwRadius, CSUnitList &cUnitList ) 
{
	ASSERT ( cUnitList.IsEmpty () );
	ASSERT ( dwCivilizationIndex < m_dwCivilizationCount );
	ASSERT ( IsMapPosition ( pointCenter ) );
	ASSERT ( dwRadius <= MAX_GET_UNITS_IN_AREA_RADIUS );

	// index prvního zpracovávaného MapSquaru
	CPointDW pointIndex = GetMapSquareIndex ( CPointDW ( ( pointCenter.x > dwRadius ) ? 
		( pointCenter.x - dwRadius ) : 0, ( pointCenter.y > dwRadius ) ? 
		( pointCenter.y - dwRadius ) : 0 ) );

	// index pravého dolního zpracovávaného MapSquaru
	CPointDW pointRightBottomIndex = GetMapSquareIndex ( CPointDW ( 
		min ( pointCenter.x + dwRadius, m_pointCornerMapCell.x ), 
		min ( pointCenter.y + dwRadius, m_pointCornerMapCell.y ) ) );

	// ètverec polomìru oblasti
	DWORD dwRadiusSquare = dwRadius * dwRadius;

	// souøadnice indexu prvního zpracovávaného MapSquaru
	DWORD dwStartIndexX = pointIndex.x;
	
	// projede zpracovávané MapSquary
	for ( ; pointIndex.y <= pointRightBottomIndex.y; pointIndex.y++ )
	{
		for ( pointIndex.x = dwStartIndexX ; pointIndex.x <= pointRightBottomIndex.x; 
			pointIndex.x++ )
		{
			// zpracovávaný MapSquare
			CSMapSquare *pMapSquare = GetMapSquareFromIndex ( pointIndex );

			// zjistí seznam jednotek v MapSquaru
			CSUnitList *pMapSquareUnitList = pMapSquare->GetUnitList ();

			// pozice jednotky v seznamu jednotek
			POSITION posUnit = pMapSquareUnitList->GetHeadPosition ();
			// ukazatel na jednotku
			CSUnit *pUnit;

			// projede seznam jednotek
			while ( CSUnitList::GetNext ( posUnit, pUnit ) )
			{
				ASSERT ( pUnit != NULL );
				// zjistí, je-li jednotka vidìna civilizací "dwCivilizationIndex"
				if ( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 )
				{	// jednotka je viditelná civilizací "dwCivilizationIndex"
					// zjistí, je-li jednotka v oblasti
					if ( pointCenter.GetDistanceSquare ( pUnit->m_pointPosition ) <= 
						dwRadiusSquare )
					{	// jednotka je v oblasti
						// pøidá jednotku do výsledného seznamu jednotek
						cUnitList.Add ( pUnit );
					}
					// jednotka není v oblasti
				}
				// jednotka není civilizací vidìna
			}
			// jednotky MapSquaru byly zpracovány
		}
	}
	// MapSquary byly zpracovány
}

// pro civilizaci "dwCivilizationIndex" vyplní seznam "cUnitList" nepøátelských 
//		jednotek v oblasti se støedem "pointCenter" o polomìru "dwRadius" (musí být 
//		zamèeny MapSquary, "dwRadius" musí být menší než MAX_GET_UNITS_IN_AREA_RADIUS)
void CSMap::GetUnitsInAreaEnemy ( DWORD dwCivilizationIndex, CPointDW pointCenter, 
	DWORD dwRadius, CSUnitList &cUnitList ) 
{
	ASSERT ( cUnitList.IsEmpty () );
	ASSERT ( dwCivilizationIndex < m_dwCivilizationCount );
	ASSERT ( IsMapPosition ( pointCenter ) );
	ASSERT ( dwRadius <= MAX_GET_UNITS_IN_AREA_RADIUS );

	// index prvního zpracovávaného MapSquaru
	CPointDW pointIndex = GetMapSquareIndex ( CPointDW ( ( pointCenter.x > dwRadius ) ? 
		( pointCenter.x - dwRadius ) : 0, ( pointCenter.y > dwRadius ) ? 
		( pointCenter.y - dwRadius ) : 0 ) );

	// index pravého dolního zpracovávaného MapSquaru
	CPointDW pointRightBottomIndex = GetMapSquareIndex ( CPointDW ( 
		min ( pointCenter.x + dwRadius, m_pointCornerMapCell.x ), 
		min ( pointCenter.y + dwRadius, m_pointCornerMapCell.y ) ) );

	// ètverec polomìru oblasti
	DWORD dwRadiusSquare = dwRadius * dwRadius;

	// souøadnice indexu prvního zpracovávaného MapSquaru
	DWORD dwStartIndexX = pointIndex.x;
	
	// projede zpracovávané MapSquary
	for ( ; pointIndex.y <= pointRightBottomIndex.y; pointIndex.y++ )
	{
		for ( pointIndex.x = dwStartIndexX ; pointIndex.x <= pointRightBottomIndex.x; 
			pointIndex.x++ )
		{
			// zpracovávaný MapSquare
			CSMapSquare *pMapSquare = GetMapSquareFromIndex ( pointIndex );

			// zjistí seznam jednotek v MapSquaru
			CSUnitList *pMapSquareUnitList = pMapSquare->GetUnitList ();

			// pozice jednotky v seznamu jednotek
			POSITION posUnit = pMapSquareUnitList->GetHeadPosition ();
			// ukazatel na jednotku
			CSUnit *pUnit;

			// projede seznam jednotek
			while ( CSUnitList::GetNext ( posUnit, pUnit ) )
			{
				ASSERT ( pUnit != NULL );
				// zjistí, jedná-li se o nepøátelskou jednotku
				if ( pUnit->GetCivilizationIndex () != dwCivilizationIndex )
				{	// jedná se o nepøátelskou jednotku
					// zjistí, je-li jednotka vidìna civilizací "dwCivilizationIndex"
					if ( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 )
					{	// jednotka je viditelná civilizací "dwCivilizationIndex"
						// zjistí, je-li jednotka v oblasti
						if ( pointCenter.GetDistanceSquare ( pUnit->m_pointPosition ) <= 
							dwRadiusSquare )
						{	// jednotka je v oblasti
							// pøidá jednotku do výsledného seznamu jednotek
							cUnitList.Add ( pUnit );
						}
						// jednotka není v oblasti
					}
					// jednotka není civilizací vidìna
				}
				// nejedná se o nepøátelskou jednotku
			}
			// jednotky MapSquaru byly zpracovány
		}
	}
	// MapSquary byly zpracovány
}

// pro civilizaci "dwCivilizationIndex" vyplní seznam "cUnitList" vlastních jednotek 
//		v oblasti se støedem "pointCenter" o polomìru "dwRadius" (musí být zamèeny 
//		MapSquary, "dwRadius" musí být menší než MAX_GET_UNITS_IN_AREA_RADIUS)
void CSMap::GetUnitsInAreaOfMine ( DWORD dwCivilizationIndex, CPointDW pointCenter, 
	DWORD dwRadius, CSUnitList &cUnitList ) 
{
	ASSERT ( cUnitList.IsEmpty () );
	ASSERT ( dwCivilizationIndex < m_dwCivilizationCount );
	ASSERT ( IsMapPosition ( pointCenter ) );
	ASSERT ( dwRadius <= MAX_GET_UNITS_IN_AREA_RADIUS );

	// index prvního zpracovávaného MapSquaru
	CPointDW pointIndex = GetMapSquareIndex ( CPointDW ( ( pointCenter.x > dwRadius ) ? 
		( pointCenter.x - dwRadius ) : 0, ( pointCenter.y > dwRadius ) ? 
		( pointCenter.y - dwRadius ) : 0 ) );

	// index pravého dolního zpracovávaného MapSquaru
	CPointDW pointRightBottomIndex = GetMapSquareIndex ( CPointDW ( 
		min ( pointCenter.x + dwRadius, m_pointCornerMapCell.x ), 
		min ( pointCenter.y + dwRadius, m_pointCornerMapCell.y ) ) );

	// ètverec polomìru oblasti
	DWORD dwRadiusSquare = dwRadius * dwRadius;

	// souøadnice indexu prvního zpracovávaného MapSquaru
	DWORD dwStartIndexX = pointIndex.x;
	
	// projede zpracovávané MapSquary
	for ( ; pointIndex.y <= pointRightBottomIndex.y; pointIndex.y++ )
	{
		for ( pointIndex.x = dwStartIndexX ; pointIndex.x <= pointRightBottomIndex.x; 
			pointIndex.x++ )
		{
			// zpracovávaný MapSquare
			CSMapSquare *pMapSquare = GetMapSquareFromIndex ( pointIndex );

			// zjistí seznam jednotek v MapSquaru
			CSUnitList *pMapSquareUnitList = pMapSquare->GetUnitList ();

			// pozice jednotky v seznamu jednotek
			POSITION posUnit = pMapSquareUnitList->GetHeadPosition ();
			// ukazatel na jednotku
			CSUnit *pUnit;

			// projede seznam jednotek
			while ( CSUnitList::GetNext ( posUnit, pUnit ) )
			{
				ASSERT ( pUnit != NULL );
				// zjistí, jedná-li se o vlastní jednotku
				if ( pUnit->GetCivilizationIndex () == dwCivilizationIndex )
				{	// jedná se o vlastní jednotku
					ASSERT ( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 );
					// zjistí, je-li jednotka v oblasti
					if ( pointCenter.GetDistanceSquare ( pUnit->m_pointPosition ) <= 
						dwRadiusSquare )
					{	// jednotka je v oblasti
						// pøidá jednotku do výsledného seznamu jednotek
						cUnitList.Add ( pUnit );
					}
					// jednotka není v oblasti
				}
				// nejedná se o vlastní jednotku
			}
			// jednotky MapSquaru byly zpracovány
		}
	}
	// MapSquary byly zpracovány
}

//////////////////////////////////////////////////////////////////////
// Operace s jednotkou mapy
//////////////////////////////////////////////////////////////////////

// pohne s jednotkou "pUnit" na pozici "pointPosition" - pouze pohyb jednotky po mapì, 
//		nikoli mimo mapu, voláno pro každý pohyb jednotky (jednotka musí být zamèena 
//		pro zápis, je-li nastaven pøíznak "bMapSquaresLocked", nezamyká MapSquary, lze 
//		volat jen z MainLoop threadu) (TRUE=jednotka byla pøemístìna)
BOOL CSMap::MoveUnit ( CSUnit *pUnit, CPointDW pointPosition, BOOL bMapSquaresLocked ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );
	ASSERT ( pUnit != NULL );
	ASSERT ( IsMapPosition ( pUnit->GetPosition () ) );
	ASSERT ( IsMapPosition ( pointPosition ) );

	// graf pro hledání cesty pro jednotku
	CSFindPathGraph *pFindPathGraph = pUnit->GetFindPathGraph ();

	// zjistí, lze-li jednotka umístit na mapu
	if ( ( ( pFindPathGraph != NULL ) && ( pFindPathGraph->GetMapCellAt ( 
		pointPosition ) < (signed char)( pUnit->GetMoveWidth () * 2 ) ) ) || 
		!CanPlaceUnitOnce ( pUnit, pointPosition, bMapSquaresLocked ) )
	{	// nelze umístit jednotku na mapu
		// vrátí pøíznak nepøemístìní jednotky
		return FALSE;
	}

	// zjistí index MapSquaru nové polohy jednotky
	CPointDW pointNewMapSquareIndex ( pointPosition.x / MAP_SQUARE_SIZE, 
		pointPosition.y / MAP_SQUARE_SIZE );
	// zjistí index MapSquaru staré polohy jednotky
	CPointDW pointOldMapSquareIndex = pUnit->GetPosition ();
	pointOldMapSquareIndex.x /= MAP_SQUARE_SIZE;
	pointOldMapSquareIndex.y /= MAP_SQUARE_SIZE;

	// nastaví novou pozici jednotky
	pUnit->SetPosition ( pointPosition );
	// nastaví pøíznak pohybu jednotky
	pUnit->SetPositionChanged ();

	// zjistí, pohnula-li se jednotka z MapSquaru
	if ( pointOldMapSquareIndex != pointNewMapSquareIndex )
	{	// jednotka se pohnula z MapSquaru
	// pøepojí jednotku mezi MapSquary a aktualizuje informace MapSquarù

		// zjistí starý MapSquare
		CSMapSquare *pOldMapSquare = GetMapSquareFromIndex ( pointOldMapSquareIndex );
		// zjistí nový MapSquare
		CSMapSquare *pNewMapSquare = GetMapSquareFromIndex ( pointNewMapSquareIndex );

		// zjistí, jsou-li zamèeny MapSquary
		if ( !bMapSquaresLocked )
		{	// MapSquary nejsou zamèeny
			// zamkne MapSquary
			VERIFY ( m_mutexMapSquaresLock.Lock () );
		}

		// aktualizuje zmìnu pozice jednotky z/do sledovaných MapSquarù
		pUnit->m_cWatchedMapSquareChanged |= pNewMapSquare->m_cWatchingCivilizations ^ 
			pOldMapSquare->m_cWatchingCivilizations;

		// zjistí, seznam jednotek starého MapSquaru
		CSUnitList *pOldUnitList = pOldMapSquare->GetUnitList ();
		// zjistí, seznam jednotek nového MapSquaru
		CSUnitList *pNewUnitList = pNewMapSquare->GetUnitList ();

		// najde jednotku ve starém MapSquaru
		POSITION posUnit = pOldUnitList->Find ( pUnit );
		ASSERT ( posUnit != NULL );
		ASSERT ( !pOldUnitList->IsEmpty ( posUnit ) );

		// pøesune jednotku do nového MapSquaru
		pOldUnitList->Move ( posUnit, *pNewUnitList );

		// aktualizuje nový MapSquare po pøidání jednotky
		pNewMapSquare->UnitInserted ( pUnit );
		// aktualizuje starý MapSquare po odebrání jednotky
		pOldMapSquare->UnitDeleted ( pUnit );

		// zjistí, byly-li zamèeny MapSquary
		if ( !bMapSquaresLocked )
		{	// MapSquary nebyly zamèeny
			// odemkne MapSquary
			VERIFY ( m_mutexMapSquaresLock.Unlock () );

		}
	}

	// vrátí pøíznak pøemístìní jednotky
	return TRUE;
}

// odebere jednotku "pUnit" z mapy (jednotka musí být zamèena pro zápis, je-li nastaven 
//		pøíznak "bMapSquaresLocked", nezamyká MapSquary, lze volat jen z MainLoop threadu)
void CSMap::RemoveUnit ( CSUnit *pUnit, BOOL bMapSquaresLocked ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );
	ASSERT ( pUnit != NULL );
	ASSERT ( IsMapPosition ( pUnit->GetPosition () ) );

	// zjistí MapSquare umístìní jednotky
	CSMapSquare *pMapSquare = GetMapSquareFromPosition ( pUnit->GetPosition () );

	// zjistí, jsou-li zamèeny MapSquary
	if ( !bMapSquaresLocked )
	{	// MapSquary nejsou zamèeny
		// zamkne MapSquary
		VERIFY ( m_mutexMapSquaresLock.Lock () );
	}

	// umístí jednotku mimo mapu
	pUnit->SetPosition ( CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ) );
	pUnit->SetFirstCheckPoint ( CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ), 0 );
	pUnit->SetSecondCheckPoint ( CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ), 0 );
	// nastaví pøíznak pohybu jednotky
	pUnit->SetPositionChanged ();
	pUnit->SetCheckPointInfoModified ();
	// aktualizuje zmìnu pozice jednotky z/do sledovaných MapSquarù
	pUnit->m_cWatchedMapSquareChanged |= pMapSquare->m_cWatchingCivilizations;

	// index civilizace jednotky
	DWORD dwCivilizationIndex = pUnit->GetCivilizationIndex ();
	// seznam jednotkou vidìných jednotek
	CSUnitList *pUnitList = pUnit->GetSeenUnitList ();
	// ukazatel na jednotku na civilizaci
	CZUnit *pZUnit = pUnit->GetZUnit ();
	// jednotkou vidìná jednotka
	CSUnit *pSeenUnit;
	while ( pUnitList->RemoveFirst ( pSeenUnit ) )
	{
		ASSERT ( pSeenUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 );
		// sníží poèet jednotek své civilizace, které vidìnou jednotku vidí
		if ( --pSeenUnit->m_aCivilizationVisibility[dwCivilizationIndex] == 0 )
		{	// jednotka zmizela civilizaci z dohledu
			// nastaví pøíznak zmìny viditelnosti jednotky civilizací
			pSeenUnit->m_cCivilizationVisibilityChanged.SetAt ( dwCivilizationIndex );
			// naposledy vidìná pozice jednotky, která pøestala být vidìt
			CSPosition *pPosition = pSeenUnit->m_aCivilizationLastSeenPosition + 
				dwCivilizationIndex;
			// zamkne jednotku, která pøestala být vidìt na zápis
			VERIFY ( pSeenUnit->PrivilegedThreadWriterLock () );
			// aktualizuje naposledy vidìnou pozici jednotky civilizací
			*(CPointDW *)pPosition = pSeenUnit->m_pointPosition;
			pPosition->z = pSeenUnit->m_dwVerticalPosition;
			ASSERT ( pPosition->z != 0 );
			// odemkne jednotku, která pøestala být vidìt na zápis
			pSeenUnit->PrivilegedThreadWriterUnlock ();
		}
		// zjistí, jedná-li se o nepøátelskou jednotku
		if ( pSeenUnit->GetCivilizationIndex () != 0 )
		{	// jedná se o nepøátelskou jednotku
			// pošle jednotce notifikaci o ukonèení viditelnosti nepøátelské jednotky
			SendNotification ( pZUnit, NOTIFICATION_ENEMY_UNIT_DISAPPEARED, pSeenUnit );
		}
		else
		{	// jedná se o systémovou jednotku
			// pošle jednotce notifikaci o ukonèení viditelnosti systémové jednotky
			SendNotification ( pZUnit, NOTIFICATION_SYSTEM_UNIT_DISAPPEARED, pSeenUnit );
		}
	}

	// zamkne seznam mrtvých jednotek pro rozesílání stop infa klientùm
	VERIFY ( m_mutexDeadUnitClientStopInfoListLock.Lock () );

	// zvýší poèet odkazù na jednotku
	pUnit->AddRef ();
	// pøidá do seznamu mrtvých jednotek pro rozesílání stop infa klientùm mrtvou jednotku
	m_cDeadUnitClientStopInfoList.Add ( pUnit );

	// odemkne seznam mrtvých jednotek pro rozesílání stop infa klientùm
	VERIFY ( m_mutexDeadUnitClientStopInfoListLock.Unlock () );

	// odebere jednotku z MapSquaru
	pMapSquare->DeleteUnit ( pUnit );

	// zjistí, byly-li zamèeny MapSquary
	if ( !bMapSquaresLocked )
	{	// MapSquary nebyly zamèeny
		// odemkne MapSquary
		VERIFY ( m_mutexMapSquaresLock.Unlock () );
	}
}

// umístí jednotku "pUnit" na pozici "pointPosition" (jednotka musí být zamèena 
//		pro zápis, je-li nastaven pøíznak "bMapSquaresLocked", nezamyká MapSquary, lze 
//		volat jen z MainLoop threadu) (TRUE=jednotka byla umístìna)
BOOL CSMap::PlaceUnit ( CSUnit *pUnit, CPointDW pointPosition, BOOL bMapSquaresLocked ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );
	ASSERT ( pUnit != NULL );
	ASSERT ( !IsMapPosition ( pUnit->GetPosition () ) );
	ASSERT ( IsMapPosition ( pointPosition ) );

	// zjistí MapSquare umístìní jednotky
	CSMapSquare *pMapSquare = GetMapSquareFromPosition ( pointPosition );

	// zjistí, jsou-li zamèeny MapSquary
	if ( !bMapSquaresLocked )
	{	// MapSquary nejsou zamèeny
		// zamkne MapSquary
		VERIFY ( m_mutexMapSquaresLock.Lock () );
	}

	// graf pro hledání cesty pro jednotku
	CSFindPathGraph *pFindPathGraph = pUnit->GetFindPathGraph ();

	// zjistí, lze-li jednotka umístit na mapu
	if ( ( ( pFindPathGraph != NULL ) && ( pFindPathGraph->GetMapCellAt ( 
		pointPosition ) < (signed char)( pUnit->GetMoveWidth () * 2 ) ) ) || 
		!CanPlaceUnitOnce ( pUnit, pointPosition, TRUE ) )
	{	// nelze umístit jednotku na mapu
		// zjistí, byly-li zamèeny MapSquary
		if ( !bMapSquaresLocked )
		{	// MapSquary nebyly zamèeny
			// odemkne MapSquary
			VERIFY ( m_mutexMapSquaresLock.Unlock () );
		}
		// vrátí pøíznak neumístìní jednotky na mapu
		return FALSE;
	}

	// umístí jednotku mimo mapu
	pUnit->SetPosition ( pointPosition );
	// nastaví pøíznak pohybu jednotky
	pUnit->SetPositionChanged ();
	// aktualizuje zmìnu pozice jednotky z/do sledovaných MapSquarù
	pUnit->m_cWatchedMapSquareChanged |= pMapSquare->m_cWatchingCivilizations;

	// vloží jednotku do MapSquaru
	pMapSquare->InsertUnit ( pUnit );

	// zjistí, byly-li zamèeny MapSquary
	if ( !bMapSquaresLocked )
	{	// MapSquary nebyly zamèeny
		// odemkne MapSquary
		VERIFY ( m_mutexMapSquaresLock.Unlock () );
	}

	// vrátí pøíznak umístìní jednotky na mapu
	return TRUE;
}

// zpracuje umístìní jednotky "pUnit" na mapu (MapSquary musí být zamèeny)
void CSMap::UnitPlaced ( CSUnit *pUnit ) 
{
	ASSERT ( pUnit != NULL );
	ASSERT ( IsMapPosition ( pUnit->GetPosition () ) );
	ASSERT ( CanPlaceUnitOnce ( pUnit, pUnit->GetPosition (), TRUE ) );

	// zjistí MapSquare umístìní jednotky
	CSMapSquare *pMapSquare = GetMapSquareFromPosition ( pUnit->GetPosition () );

	// nastaví pøíznak pohybu jednotky
	pUnit->SetPositionChanged ();
	// aktualizuje zmìnu pozice jednotky z/do sledovaných MapSquarù
	pUnit->m_cWatchedMapSquareChanged |= pMapSquare->m_cWatchingCivilizations;

	// vloží jednotku do MapSquaru
	pMapSquare->InsertUnit ( pUnit );
}

// vrátí nejbližší umístìní jednotky "pUnit" na pozici "pointPosition" (jednotka musí být 
//		zamèena pro ètení, MapSquary musí být zamèeny, lze volat jen z MainLoop threadu) 
//		(NO_MAP_POSITION - jednotku nelze umístit)
CPointDW CSMap::GetNearestUnitPlace ( CSUnit *pUnit, CPointDW pointPosition ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );

	ASSERT ( pUnit != NULL );
	ASSERT ( IsMapPosition ( pointPosition ) );

	// pozice umístìní jednotky
	CPointDW pointPlacedPosition ( NO_MAP_POSITION, NO_MAP_POSITION );

	// fronta pozic na mapì
	static CSelfPooledQueue<CPointDW> cPositionQueue ( 100 );
	ASSERT ( cPositionQueue.IsEmpty () );

	// graf pro hledání cesty pro jednotku
	CSFindPathGraph *pFindPathGraph = pUnit->GetFindPathGraph ();
	// ID MapCellu šíøky jednotky
	signed char cUnitWidthMapCellID = (signed char)( pUnit->GetMoveWidth () * 2 );

	// nejvìtší poèet zkoušených pozic
	DWORD dwMaxPositionCount = 2500;

	// zjistí, jedná-li se o prázdný graf pro hledání cesty
	if ( pFindPathGraph == NULL )
	{	// jedná se o prázdný graf pro hledání cesty
		// zjistí, jedná-li se o vzdušnou jednotku
		if ( pUnit->GetVerticalPosition () >= AIR_VERTICAL_POSITION )
		{	// jedná se o vzdušnou jednotku
			// vrátí požadovanou pozici jednotky
			return pointPosition;
		}
		// jedná se o pozemní jednotku

		// pøidá do fronty pozic na mapì poèáteèní pozici umístìní jednotky
		cPositionQueue.Add ( pointPosition );

		// pokusí se umístit jednotku
		while ( cPositionQueue.RemoveFirst ( pointPosition ) )
		{
			// zjistí jedná-li se o poslední zkoušenou pozici
			if ( dwMaxPositionCount-- == 0 )
			{	// jedná se o poslední zkoušenou pozici
				// ukonèí umísování jednotky
				break;
			}

			// zjistí, lze-li umístit jednotku na pozici
			if ( CSMapSquare::MainLoopCanPlaceUnit ( pUnit, pointPosition, TRUE ) )
			{	// jednotku lze umístit na pozici
				// uschová pozici umístìní jednotky
				pointPlacedPosition = pointPosition;
				// ukonèí umísování jednotky
				break;
			}

			// vyzkouší okolní pozice na mapì
			for ( DWORD dwSurroundingPositionIndex = 8; dwSurroundingPositionIndex-- > 0; )
			{
				// sousední pozice na mapì
				CPointDW pointSurroundingPosition = pointPosition + 
					CSMapSquare::m_aSurroundingPositionOffset[dwSurroundingPositionIndex];

				// zjistí, jedná-li se o pozici na mapì
				if ( ( pointSurroundingPosition.x < m_sizeMapMapCell.cx ) && 
					( pointSurroundingPosition.y < m_sizeMapMapCell.cy ) )
				{	// jedná se o pozici na mapì
					// pøíznak objevení pozice na mapì
					BYTE &nPosition = m_cGetNearestUnitPlacePositionArray.GetAt ( 
						pointSurroundingPosition );
					// zjistí, byla-li pozice již objevena
					if ( nPosition == 0 )
					{	// pozice dosud nebyla objevena
						// oznaèí pozici za objevenou
						nPosition = 1;
						// pøidá pozici do fronty pozic
						cPositionQueue.Add ( pointSurroundingPosition );
					}
					// pozice již byla objevena
				}
				// nejedná se o pozici na mapì
			}
			// jednotku nelze umístit na pozici
		}
		// pozemní jednotku s prázdným grafem pro hledání cesty se podaøilo umístit nebo ji 
		//		umístit nelze

		// ukonèí umísování jednotky
		CSMapSquare::MainLoopFinishPlacingUnit ();
	}
	else
	{	// jedná se o neprázdný graf pro hledání cesty
		// pøidá do fronty pozic na mapì poèáteèní pozici umístìní jednotky
		cPositionQueue.Add ( pointPosition );

		// zjistí, jedná-li se o vzdušnou jednotku
		if ( pUnit->GetVerticalPosition () >= AIR_VERTICAL_POSITION )
		{	// jedná se o vzdušnou jednotku
			// pokusí se umístit jednotku
			while ( cPositionQueue.RemoveFirst ( pointPosition ) )
			{
				// zjistí jedná-li se o poslední zkoušenou pozici
				if ( dwMaxPositionCount-- == 0 )
				{	// jedná se o poslední zkoušenou pozici
					// ukonèí umísování jednotky
					break;
				}

				// zjistí, lze-li umístit jednotku na pozici
				if ( pFindPathGraph->GetMapCellAt ( pointPosition ) >= cUnitWidthMapCellID )
				{	// jednotku lze umístit na pozici
					// uschová pozici umístìní jednotky
					pointPlacedPosition = pointPosition;
					// ukonèí umísování jednotky
					break;
				}

				// vyzkouší okolní pozice na mapì
				for ( DWORD dwSurroundingPositionIndex = 8; dwSurroundingPositionIndex-- > 0; )
				{
					// sousední pozice na mapì
					CPointDW pointSurroundingPosition = pointPosition + 
						CSMapSquare::m_aSurroundingPositionOffset[dwSurroundingPositionIndex];

					// zjistí, jedná-li se o pozici na mapì
					if ( ( pointSurroundingPosition.x < m_sizeMapMapCell.cx ) && 
						( pointSurroundingPosition.y < m_sizeMapMapCell.cy ) )
					{	// jedná se o pozici na mapì
						// pøíznak objevení pozice na mapì
						BYTE &nPosition = m_cGetNearestUnitPlacePositionArray.GetAt ( 
							pointSurroundingPosition );
						// zjistí, byla-li pozice již objevena
						if ( nPosition == 0 )
						{	// pozice dosud nebyla objevena
							// oznaèí pozici za objevenou
							nPosition = 1;
							// pøidá pozici do fronty pozic
							cPositionQueue.Add ( pointSurroundingPosition );
						}
						// pozice již byla objevena
					}
					// nejedná se o pozici na mapì
				}
				// jednotku nelze umístit na pozici
			}
			// vzdušnou jednotku s neprázdným grafem pro hledání cesty se podaøilo umístit 
			//		nebo ji umístit nelze
		}
		else
		{	// jedná se o pozemní jednotku
			// pokusí se umístit jednotku
			while ( cPositionQueue.RemoveFirst ( pointPosition ) )
			{
				// zjistí jedná-li se o poslední zkoušenou pozici
				if ( dwMaxPositionCount-- == 0 )
				{	// jedná se o poslední zkoušenou pozici
					// ukonèí umísování jednotky
					break;
				}

				// zjistí, lze-li umístit jednotku na pozici
				if ( ( pFindPathGraph->GetMapCellAt ( pointPosition ) >= 
					cUnitWidthMapCellID ) && CSMapSquare::MainLoopCanPlaceUnit ( pUnit, 
					pointPosition, TRUE ) )
				{	// jednotku lze umístit na pozici
					// uschová pozici umístìní jednotky
					pointPlacedPosition = pointPosition;
					// ukonèí umísování jednotky
					break;
				}

				// vyzkouší okolní pozice na mapì
				for ( DWORD dwSurroundingPositionIndex = 8; dwSurroundingPositionIndex-- > 0; )

				{
					// sousední pozice na mapì
					CPointDW pointSurroundingPosition = pointPosition + 
						CSMapSquare::m_aSurroundingPositionOffset[dwSurroundingPositionIndex];

					// zjistí, jedná-li se o pozici na mapì
					if ( ( pointSurroundingPosition.x < m_sizeMapMapCell.cx ) && 
						( pointSurroundingPosition.y < m_sizeMapMapCell.cy ) )
					{	// jedná se o pozici na mapì
						// pøíznak objevení pozice na mapì
						BYTE &nPosition = m_cGetNearestUnitPlacePositionArray.GetAt ( 
							pointSurroundingPosition );
						// zjistí, byla-li pozice již objevena
						if ( nPosition == 0 )
						{	// pozice dosud nebyla objevena
							// oznaèí pozici za objevenou
							nPosition = 1;
							// pøidá pozici do fronty pozic
							cPositionQueue.Add ( pointSurroundingPosition );
						}
						// pozice již byla objevena
					}
					// nejedná se o pozici na mapì
				}
				// jednotku nelze umístit na pozici
			}
			// pozemní jednotku s neprázdným grafem pro hledání cesty se podaøilo umístit 
			//		nebo ji umístit nelze

			// ukonèí umísování jednotky
			CSMapSquare::MainLoopFinishPlacingUnit ();
		}
		// jednotku s neprázdným grafem pro hledání cesty se podaøilo umístit nebo ji 
		//		umístit nelze
	}
	// jednotku s prázdným i neprázdným grafem pro hledání cesty se podaøilo umístit nebo 
	//		ji umístit nelze

	// znièí frontu pozic na mapì
	cPositionQueue.RemoveAll ();

	// smaže pole pøíznakù objevení pozice na mapì
	m_cGetNearestUnitPlacePositionArray.Clear ();

	// vrátí pozici umístìní jednotky
	return pointPlacedPosition;
}

//////////////////////////////////////////////////////////////////////
// Operace øízení hry (TimeSlicy a pausování hry)
//////////////////////////////////////////////////////////////////////

// zapausuje hru (poèká na zapausování hry)
void CSMap::PauseGame () 
{
	ASSERT ( CheckValidData () );

	// zkontroluje, není-li hra již zapausována
	ASSERT ( !m_eventPauseGame.Lock ( 0 ) );
	ASSERT ( m_eventUnpauseGame.Lock ( 0 ) );
	ASSERT ( !m_eventMainLoopPaused.Lock ( 0 ) );
	ASSERT ( !m_eventClientInfoSenderLoopPaused.Lock ( 0 ) );
	ASSERT ( !CSPath::m_eventFindPathLoopPaused.Lock ( 0 ) );

	// zamkne zámek pøíznaku povolení logování uživatelù
	VERIFY ( m_cUserLoginAllowedLock.WriterLock () );

	// zakáže logování uživatelù
	m_bUserLoginAllowed = FALSE;

	// odemkne zámek pøíznaku povolení logování uživatelù
	m_cUserLoginAllowedLock.WriterUnlock ();

	// zruší událost odpausování hry
	VERIFY ( m_eventUnpauseGame.ResetEvent () );
	// nastaví událost zapausování hry
	VERIFY ( m_eventPauseGame.SetEvent () );

	// poèet událostí zapausování threadù a civilizací
	DWORD dwPausingEventsCount = 3 + m_dwCivilizationCount;
	// vytvoøí tabulku událostí zapausování threadù a civilizací
	HANDLE *aPausingEvents = new HANDLE[dwPausingEventsCount];

	// inicializuje události zapausovaných threadù
	aPausingEvents[0] = (HANDLE)m_eventMainLoopPaused;
	aPausingEvents[1] = (HANDLE)m_eventClientInfoSenderLoopPaused;
	aPausingEvents[2] = (HANDLE)CSPath::m_eventFindPathLoopPaused;

	// nechá zapausovat jednotlivé civilizace
	for ( DWORD dwCivilizationIndex = 0; dwCivilizationIndex < m_dwCivilizationCount; 
		dwCivilizationIndex++ )
	{
		// nechá zapausovat civilizaci a pøidá událost zapausování civilizace do tabulky
		aPausingEvents[dwCivilizationIndex + 3] = 
			(HANDLE)*m_pCivilizations[dwCivilizationIndex].m_pZCivilization->Pause ();
	}

	// poèká na zapausování pausovaných threadù a civilizací
	DWORD dwPausingResult = WaitForMultipleObjects ( dwPausingEventsCount, 
		aPausingEvents, TRUE, INFINITE );

	// znièí tabulku událostí zapausovaných threadù a civilizací
	delete aPausingEvents;

	// zkontroluje výsledek zapausování threadù a civilizací
	ASSERT ( ( dwPausingResult >= WAIT_OBJECT_0 ) && ( dwPausingResult < WAIT_OBJECT_0 + 
		dwPausingEventsCount ) );
	// thready i civilizace byly zapausovány
	ASSERT ( !m_eventMainLoopPaused.Lock ( 0 ) );
	ASSERT ( !m_eventClientInfoSenderLoopPaused.Lock ( 0 ) );
	ASSERT ( !CSPath::m_eventFindPathLoopPaused.Lock ( 0 ) );

	// hra je zapausována - ukonèí zapausování hry
	return;
}

// odpausuje hru s pøíznakem pokraèování hry "bContinueGame"
void CSMap::UnpauseGame ( BOOL bContinueGame ) 
{
	ASSERT ( CheckValidData () );

	// zkontroluje zapausování hry
	ASSERT ( m_eventPauseGame.Lock ( 0 ) );
	ASSERT ( !m_eventUnpauseGame.Lock ( 0 ) );

	// aktualizuje pøíznak pokraèování hry
	m_bContinueUnpausedGame = bContinueGame;

	// nechá odpausovat jednotlivé civilizace
	for ( DWORD dwCivilizationIndex = 0; dwCivilizationIndex < m_dwCivilizationCount; 
		dwCivilizationIndex++ )
	{
		// nechá odpausovat další civilizaci
		m_pCivilizations[dwCivilizationIndex].m_pZCivilization->Unpause ( 
			bContinueGame != FALSE );
	}

	// zjistí, má-li hra pokraèovat
	if ( bContinueGame )
	{	// hra má pokraèovat
		// zamkne zámek pøíznaku povolení logování uživatelù
		VERIFY ( m_cUserLoginAllowedLock.WriterLock () );

		// povolí logování uživatelù
		m_bUserLoginAllowed = TRUE;

		// odemkne zámek pøíznaku povolení logování uživatelù
		m_cUserLoginAllowedLock.WriterUnlock ();
	}

	// zruší událost zapausování hry
	VERIFY ( m_eventPauseGame.ResetEvent () );
	// nastaví událost odpausování hry
	VERIFY ( m_eventUnpauseGame.SetEvent () );
}

// initializuje hru (vytvoøí zapausovanou hru)
void CSMap::InitGame () 
{
	// zkontroluje thready mapy
	ASSERT ( m_pMainLoopThread == NULL );
	ASSERT ( m_pClientInfoSenderLoopThread == NULL );
	ASSERT ( m_pFindPathLoopThread == NULL );

// inicializuje interní data mapy

	// inicializuje událost požadavku sledování MapSquaru
	VERIFY ( m_eventWatchingMapSquareRequest.ResetEvent () );

// inicializuje hru

	// inicializuje události MainLoop threadu
	VERIFY ( m_eventClientInfoSenderLoopTimeSliceFinished.ResetEvent () );

	// inicializuje události zapausování threadù
	VERIFY ( CSPath::m_eventFindPathLoopPaused.ResetEvent () );
	VERIFY ( m_eventClientInfoSenderLoopPaused.ResetEvent () );
	VERIFY ( m_eventMainLoopPaused.ResetEvent () );

	// inicializuje zapausování hry
	VERIFY ( m_eventUnpauseGame.ResetEvent () );
	VERIFY ( m_eventPauseGame.SetEvent () );

#ifdef _DEBUG
	// zneškodní ID MainLoop threadu
	m_dwMainLoopThreadID = DWORD_MAX;
#endif //_DEBUG

	// vytvoøí thready mapy
	m_pMainLoopThread = AfxBeginThread ( RunMainLoop, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL );
	LOAD_ASSERT ( m_pMainLoopThread != NULL );
	m_pClientInfoSenderLoopThread = AfxBeginThread ( RunClientInfoSenderLoop, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL );
	LOAD_ASSERT ( m_pClientInfoSenderLoopThread != NULL );
	m_pFindPathLoopThread = AfxBeginThread ( CSPath::FindPathLoop, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL );
	LOAD_ASSERT ( m_pFindPathLoopThread != NULL );

	// nastaví autodelete na FALSE
	m_pMainLoopThread->m_bAutoDelete = FALSE;
	m_pClientInfoSenderLoopThread->m_bAutoDelete = FALSE;
	m_pFindPathLoopThread->m_bAutoDelete = FALSE;

	// pustí vytvoøené thready mapy
	m_pMainLoopThread->ResumeThread ();
	m_pClientInfoSenderLoopThread->ResumeThread ();
	m_pFindPathLoopThread->ResumeThread ();

	// tabulka pausovaných threadù
	HANDLE aPausingThreads[3] = { 
		(HANDLE)m_eventMainLoopPaused, 
		(HANDLE)m_eventClientInfoSenderLoopPaused, 
		(HANDLE)CSPath::m_eventFindPathLoopPaused, 
	};

	// poèká na zapausování pausovaných threadù
	switch ( WaitForMultipleObjects ( 3, aPausingThreads, TRUE, INFINITE ) )
	{
	// thready byly zapausovány
	case WAIT_OBJECT_0 :
	case ( WAIT_OBJECT_0 + 1 ) :
	case ( WAIT_OBJECT_0 + 2 ) :
		ASSERT ( !m_eventMainLoopPaused.Lock ( 0 ) );
		ASSERT ( !m_eventClientInfoSenderLoopPaused.Lock ( 0 ) );
		ASSERT ( !CSPath::m_eventFindPathLoopPaused.Lock ( 0 ) );
		// hra je zapausována - ukonèí zapausování hry
		break;
	// thready se nepovedlo zapausovat
	default:
		ASSERT ( FALSE );
		return;
	}

	// nastaví pøíznak inicializavané mapy
	m_bInitialized = TRUE;
}

// spustí MainLoop mapy "pMap"
UINT CSMap::RunMainLoop ( LPVOID pMap ) 
{
	ASSERT ( pMap != NULL );

	// spustí MainLoop mapy "pMap"
	((CSMap *)pMap)->MainLoop ();

	// vrátí pøíznak úspìchu
	return 0;
}

// spustí ClientInfoSenderLoop mapy "pMap"
UINT CSMap::RunClientInfoSenderLoop ( LPVOID pMap ) 
{
	ASSERT ( pMap != NULL );

	// spustí ClientInfoSenderLoop mapy "pMap"
	((CSMap *)pMap)->ClientInfoSenderLoop ();

	// vrátí pøíznak úspìchu
	return 0;
}

// poèká na zaèátek nového TimeSlicu (s timeoutem "dwTimeout")
BOOL CSMap::WaitForNewTimeSlice ( DWORD dwTimeout ) 
{
	// smyèka èekání na zprávu s timeoutem
	for ( ; ; )
	{
		// poèká na timer s timoutem "dwTimeout"
		switch ( MsgWaitForMultipleObjects ( 0, NULL, FALSE, dwTimeout, QS_TIMER ) )
		{
		// zpráva timeru
		case WAIT_OBJECT_0 :
			// zpráva
			MSG sMessage;

			// pokusí se pøeèíst z fronty zpráv zprávu timeru
			if ( PeekMessage ( &sMessage, NULL, WM_TIMER, WM_TIMER, PM_REMOVE ) )
			{	// byla pøeètena zpráva timeru z fronty zpráv
				ASSERT ( sMessage.message == WM_TIMER );
				// vrátí pøíznak pøeètení zprávy timeru
				return TRUE;
			}
			// nebyla získána žádná zpráva timeru

			// pokraèuje v èekání na zprávu timeru
			break;
		// timeout
		case WAIT_TIMEOUT :
			// vrátí pøíznak timeoutu
			return FALSE;
		// nemožný pøípad
		default :
			ASSERT ( FALSE );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Operace pro klienta civilizace
//////////////////////////////////////////////////////////////////////

// zaregistruje klienta civilizace "pClient" (mùže trvat dlouho - èeká na dojetí 
//		rozesílání informací o jednotkách klientùm)
void CSMap::RegisterClient ( CZClientUnitInfoSender *pClient ) 
{
	ASSERT ( pClient != NULL );

	// zamkne seznamy zaregistrovaných klientù civilizací pro zápis
	VERIFY ( m_cRegisteredClientListLock.WriterLock () );

	ASSERT ( m_aRegisteredClientList[pClient->GetCivilizationIndex ()].FindClient ( pClient ) == NULL );
	// pøipraví zaregistraci klienta civilizace
	struct SRegisteredClientInfo sClientInfo;
	sClientInfo.pClient = pClient;
	// vytvoøí pole sledovaných MapSquarù
	sClientInfo.pWatchedMapSquareArray = new CSWatchedMapSquareArray;
	// zaregistruje klienta civilizace
	m_aRegisteredClientList[pClient->GetCivilizationIndex ()].Add ( sClientInfo );

	TRACE_COMMUNICATION1 ( _T("#	RegisterClient ( Client=%x )\n"), pClient );

	// odemkne seznamy zaregistrovaných klientù civilizací pro zápis
	m_cRegisteredClientListLock.WriterUnlock ();
}

// odregistruje klienta civilizace "pClient" (mùže trvat dlouho - èeká na dojetí 
//		rozesílání informací o jednotkách klientùm, bìhem volání této metody NESMÍ být 
//		volány metody s klientem "pClient", ale server mùže rozeslat ještì nìkteré 
//		informace - klient je musí stornovat) (odregistrováním klienta civilizace 
//		ztrácí server všechny odkazy na klienta)
void CSMap::UnregisterClient ( CZClientUnitInfoSender *pClient ) 
{
	ASSERT ( pClient != NULL );

// zruší požadavky klienta civilizace na sledování MapSquarù

	// zamkne frontu požadavkù na sledování MapSquarù
	VERIFY ( m_mutexWatchingMapSquareRequestQueueLock.Lock () );

	// smaže z fronty požadavkù na sledování MapSquarù všechny požadavky klienta "pClient"
	m_cWatchingMapSquareRequestQueue.RemoveAll ( pClient );

	// odemkne frontu požadavkù na sledování MapSquarù
	VERIFY ( m_mutexWatchingMapSquareRequestQueueLock.Unlock () );

// zruší požadavky klienta civilizace na sledování jednotek

	// zamkne frontu požadavkù na sledování jednotek
	VERIFY ( m_mutexWatchingUnitRequestQueueLock.Lock () );

	// zjistí pozici prvního požadavku na sledování jednotky
	POSITION posRequest = m_cWatchingUnitRequestQueue.GetHeadPosition ();

	// informace o požadavku na sledování jednotky
	struct SWatchingUnitRequestInfo sRequestInfo;

	// schová si pozici posledního požadavku
	POSITION posLastRequest = posRequest;

	// projede frontu požadavkù na sledování jednotek
	while ( CSWatchingUnitRequestQueue::GetNext ( posRequest, sRequestInfo ) )
	{
		// zjistí, jedná-li to požadavek klienta "pClient"
		if ( sRequestInfo.pClient == pClient )
		{	// jedná se o požadavek klienta "pClient"
			// smaže požadavek z fronty požadavkù na sledování jednotek
			(void)m_cWatchingUnitRequestQueue.RemovePosition ( posRequest );
			// aktualizuje pozici dalšího požadavku
			posRequest = posLastRequest;

			// zjistí, jedná-li se o požadavek zaèátku sledování jednotky
			if ( sRequestInfo.bStartWatchingUnit )
			{	// jedná se o požadavek zaèátku sledování jednotky
				// sníží poèet odkazù na jednotku
				sRequestInfo.pUnit->Release ();
			}
		}
		else
		{	// nejedná se o požadavek klienta "pClient"
			// aktualizuje pozici poslední jednotky
			posLastRequest = posRequest;
		}
	}
	// fronta požadavkù na sledování jednotek byla projeta

	// odemkne frontu požadavkù na sledování jednotek
	VERIFY ( m_mutexWatchingUnitRequestQueueLock.Unlock () );

// zruší jednotky sledované klientem civilizace

	// zamkne seznam sledovaných jednotek
	VERIFY ( m_mutexWatchedUnitListLock.Lock () );

	// získá pozici první sledované jednotky
	POSITION posUnit = m_cWatchedUnitList.GetHeadPosition ();

	// informace o sledované jednotce
	struct SWatchedUnitInfo sUnitInfo;

	// schová si pozici poslední jednotky
	POSITION posLastUnit = posUnit;

	// projede seznam sledovaných jednotek
	while ( m_cWatchedUnitList.GetNext ( posUnit, sUnitInfo ) )
	{
		// zjistí, je-li další sledovaná jednotka sledována odregistrovávaným klientem
		if ( sUnitInfo.pClient == pClient )
		{	// jednotka je sledována odregistrovávaným klientem
			// aktualizuje pozici další jednotky
			posUnit = posLastUnit;
			// smaže jednotku ze seznamu sledovaných jednotek
			VERIFY ( m_cWatchedUnitList.RemovePosition ( posUnit ) == sUnitInfo );

			// sníží poèet klientù sledujících jednotku
			ASSERT ( sUnitInfo.pUnit->m_dwWatchingCivilizationClientCount > 0 );
			sUnitInfo.pUnit->m_dwWatchingCivilizationClientCount--;

			// sníží poèet odkazù na jednotku
			sUnitInfo.pUnit->Release ();
		}
		else
		{	// jednotka není sledována odregistrovávaným klientem
			// aktualizuje pozici poslední jednotky
			posLastUnit = posUnit;
		}
	}

	// odemkne seznam sledovaných jednotek
	VERIFY ( m_mutexWatchedUnitListLock.Unlock () );

// odregistruje klienta civilizace

	// zamkne seznamy zaregistrovaných klientù civilizací pro zápis
	VERIFY ( m_cRegisteredClientListLock.WriterLock () );

	// zjistí index civilizace klientù
	DWORD dwCivilizationIndex = pClient->GetCivilizationIndex ();

	// zjistí umístìní klienta v seznamu klientù civilizace
	POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].FindClient ( pClient );
	ASSERT ( posClient != NULL );

	// odregistruje klienta civilizace
	struct SRegisteredClientInfo sClientInfo = 
		m_aRegisteredClientList[dwCivilizationIndex].RemovePosition ( posClient );
	ASSERT ( ( sClientInfo.pClient == pClient ) && ( sClientInfo.pWatchedMapSquareArray != NULL ) );

	TRACE_COMMUNICATION1 ( _T("#	UnregisterClient ( Client=%x )\n"), pClient );

// ukonèí sledování všech sledovaných MapSquarù

	// zjistí pozici prvního klienta civilizace
	POSITION posRemainingClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();

	// informace o zbývajícím klientovi civilizace
	struct SRegisteredClientInfo sRemainingClientInfo;

	// projede zbývající klienty civilizace
	while ( CSRegisteredClientList::GetNext ( posRemainingClient, sRemainingClientInfo ) )
	{
		ASSERT ( ( sRemainingClientInfo.pClient != NULL ) && ( sRemainingClientInfo.pClient != pClient ) && 
			( sRemainingClientInfo.pWatchedMapSquareArray != NULL ) );
		// projede MapSquary sledované odregistrovaným klientem civilizace
		for ( int nWatchedMapSquareIndex = sClientInfo.pWatchedMapSquareArray->GetCount (); 
			nWatchedMapSquareIndex-- > 0; )
		{
			// zjistí další MapSquare sledovaný oderegistrovaným klientem civilizace
			CSMapSquare *pWatchedMapSquare = 
				sClientInfo.pWatchedMapSquareArray->GetAt ( nWatchedMapSquareIndex );
			ASSERT ( pWatchedMapSquare->m_cWatchingCivilizations.GetAt ( dwCivilizationIndex ) );
			// zjistí, je-li MapSquare sledován i zbývajícím klientem civilizace
			if ( sRemainingClientInfo.pWatchedMapSquareArray->Find ( pWatchedMapSquare ) )
			{	// MapSquare je sledován i zbývajícím klientem civilizace
				// smaže sledovaný MapSquare z MapSquarù sledovaných odregistrovaným 
				//		klientem civilizace
				sClientInfo.pWatchedMapSquareArray->RemoveAt ( nWatchedMapSquareIndex );
			}
		}
	}
	// v seznamu sledovaných MapSquarù ponechal pouze MapSquary sledované výhradnì 
	//		odregistrovávaným klientem

	// projede MapSquary sledované výhradnì odregistrovaným klientem civilizace
	for ( int nWatchedMapSquareIndex = sClientInfo.pWatchedMapSquareArray->GetCount (); 
		nWatchedMapSquareIndex-- > 0; )
	{
		// zjistí další MapSquare sledovaný odregistrovaným klientem civilizace
		CSMapSquare *pWatchedMapSquare = 
			sClientInfo.pWatchedMapSquareArray->GetAt ( nWatchedMapSquareIndex );
		// zruší pøíznak civilizace sledující MapSquare
		// *****ASSERT ( pWatchedMapSquare->m_cWatchingCivilizations.GetAt ( dwCivilizationIndex ) );
		pWatchedMapSquare->m_cWatchingCivilizations.ClearAt ( dwCivilizationIndex );
	}

	// smaže pole sledovaných MapSquarù odregistrovaným klientem civilizace
	sClientInfo.pWatchedMapSquareArray->RemoveAll ();
	// znièí pole sledovaných MapSquarù odregistrovaným klientem civilizace
	delete sClientInfo.pWatchedMapSquareArray;

	// odemkne seznamy zaregistrovaných klientù civilizací pro zápis
	m_cRegisteredClientListLock.WriterUnlock ();
}

// zaène sledovat jednotku "dwID" klientem "pClient" (FALSE=jednotka již neexistuje)
BOOL CSMap::StartWatchingUnit ( CZClientUnitInfoSender *pClient, DWORD dwID ) 
{
	ASSERT ( pClient != NULL );

	// zjistí jednotku podle ID
	CSUnit *pUnit = GetUnitByID ( dwID );

	// zjistí, existuje-li jednotka
	if ( pUnit == NULL )
	{	// jednotka již neexistuje
		// vrátí pøíznak neexistence jednotky
		return FALSE;
	}

	// zvýší poèet odkazù na jednotku
	pUnit->AddRef ();

	// zamkne frontu požadavkù na sledování jednotek
	VERIFY ( m_mutexWatchingUnitRequestQueueLock.Lock () );

	// informace o požadavku na sledování jednotky
	struct SWatchingUnitRequestInfo sRequestInfo;
	// vyplní požadavek na sledování jednotky
	sRequestInfo.bStartWatchingUnit = TRUE;
	sRequestInfo.pClient = pClient;
	sRequestInfo.pUnit = pUnit;

	TRACE_COMMUNICATION2 ( _T("#	StartWatchingUnit ( Client=%x, UnitID=%d )\n"), pClient, dwID );

	// pøidá požadavek na sledování jednotky
	m_cWatchingUnitRequestQueue.Add ( sRequestInfo );

	// odemkne frontu požadavkù na sledování jednotek
	VERIFY ( m_mutexWatchingUnitRequestQueueLock.Unlock () );

	// vrátí pøíznak existující jednotky
	return TRUE;
}

// ukonèí sledování jednotky "dwID" klientem "pClient"
void CSMap::StopWatchingUnit ( CZClientUnitInfoSender *pClient, DWORD dwID ) 
{
	ASSERT ( pClient != NULL );

	// zamkne frontu požadavkù na sledování jednotek
	VERIFY ( m_mutexWatchingUnitRequestQueueLock.Lock () );

	// informace o požadavku na sledování jednotky
	struct SWatchingUnitRequestInfo sRequestInfo;
	// vyplní požadavek na ukonèení sledování jednotky
	sRequestInfo.bStartWatchingUnit = FALSE;
	sRequestInfo.pClient = pClient;
	sRequestInfo.dwID = dwID;

	TRACE_COMMUNICATION2 ( _T("#	StopWatchingUnit ( Client=%x, UnitID=%d )\n"), pClient, dwID );

	// pøidá požadavek na ukonèení sledování jednotky
	m_cWatchingUnitRequestQueue.Add ( sRequestInfo );

	// odemkne frontu požadavkù na sledování jednotek
	VERIFY ( m_mutexWatchingUnitRequestQueueLock.Unlock () );
}

// zaène sledovat MapSquare "pointIndex" klientem "pClient"
void CSMap::StartWatchingMapSquare ( CZClientUnitInfoSender *pClient, CPointDW pointIndex ) 
{
	ASSERT ( pClient != NULL );

	// zjistí požadovaný MapSquare na sledování
	CSMapSquare *pMapSquare = GetMapSquareFromIndex ( pointIndex );

	// zamkne frontu požadavkù na sledování MapSquarù
	VERIFY ( m_mutexWatchingMapSquareRequestQueueLock.Lock () );

	// informace o požadavku na sledování MapSquaru
	struct SWatchingMapSquareRequestInfo sRequestInfo;
	// vyplní požadavek na sledování MapSquaru
	sRequestInfo.bStartWatchingMapSquare = TRUE;
	sRequestInfo.pClient = pClient;
	sRequestInfo.pMapSquare = pMapSquare;

	TRACE_COMMUNICATION3 ( _T("#	StartWatchingMapSquare ( Client=%x, MapSquare=(%d, %d) )\n"), pClient, pointIndex.x, pointIndex.y );

	// pøidá požadavek na sledování MapSquaru
	m_cWatchingMapSquareRequestQueue.Add ( sRequestInfo );

	// nastaví událost požadavku na sledování MapSquaru
	VERIFY ( m_eventWatchingMapSquareRequest.SetEvent () );

	// odemkne frontu požadavkù na sledování MapSquarù
	VERIFY ( m_mutexWatchingMapSquareRequestQueueLock.Unlock () );
}

// ukonèí sledování MapSquaru "pointIndex" klientem "pClient"
void CSMap::StopWatchingMapSquare ( CZClientUnitInfoSender *pClient, CPointDW pointIndex ) 
{
	ASSERT ( pClient != NULL );

	// zjistí požadovaný MapSquare na sledování
	CSMapSquare *pMapSquare = GetMapSquareFromIndex ( pointIndex );

	// zamkne frontu požadavkù na sledování MapSquarù
	VERIFY ( m_mutexWatchingMapSquareRequestQueueLock.Lock () );

	// informace o požadavku na sledování MapSquaru
	struct SWatchingMapSquareRequestInfo sRequestInfo;
	// vyplní požadavek na ukonèení sledování MapSquaru
	sRequestInfo.bStartWatchingMapSquare = FALSE;
	sRequestInfo.pClient = pClient;
	sRequestInfo.pMapSquare = pMapSquare;

	TRACE_COMMUNICATION3 ( _T("#	StopWatchingMapSquare ( Client=%x, MapSquare=(%d, %d) )\n"), pClient, pointIndex.x, pointIndex.y );

	// pøidá požadavek na ukonèení sledování MapSquaru
	m_cWatchingMapSquareRequestQueue.Add ( sRequestInfo );

	// nastaví událost požadavku na sledování MapSquaru
	VERIFY ( m_eventWatchingMapSquareRequest.SetEvent () );

	// odemkne frontu požadavkù na sledování MapSquarù
	VERIFY ( m_mutexWatchingMapSquareRequestQueueLock.Unlock () );
}

//////////////////////////////////////////////////////////////////////
// Operace s odloženým voláním procedury (DPC)
//////////////////////////////////////////////////////////////////////

// zaregistruje civilizaci "dwCivilizationIndex" odložené volání procedury "pDPC"
void CSMap::RegisterDPC ( CSDeferredProcedureCall *pDPC, DWORD dwCivilizationIndex ) 
{
	ASSERT ( dwCivilizationIndex < m_dwCivilizationCount );
	ASSERT ( pDPC != NULL );

	// zamkne DPC fronty civilizací
	VERIFY ( m_mutexDPCQueueLock.Lock () );

	// pøidá DPC do DPC fronty civilizace
	m_aDPCQueue[dwCivilizationIndex].Add ( pDPC );

	// odemkne DPC fronty civilizací
	VERIFY ( m_mutexDPCQueueLock.Unlock () );
}

// odregistruje civilizaci "dwCivilizationIndex" odložené volání procedury "pDPC" 
//		(FALSE=DPC nebylo zaregistrované)
BOOL CSMap::UnregisterDPC ( CSDeferredProcedureCall *pDPC, DWORD dwCivilizationIndex ) 
{
	ASSERT ( dwCivilizationIndex < m_dwCivilizationCount );
	ASSERT ( pDPC != NULL );

	// pøíznak úspìšného odregistrování DPC
	BOOL bUnregistered;

	// zamkne DPC fronty civilizací
	VERIFY ( m_mutexDPCQueueLock.Lock () );

	// pokusí se smazat DPC z DPC fronty civilizace
	if ( !m_aDPCQueue[dwCivilizationIndex].RemoveElement ( pDPC ) )
	{	// DPC nebylo nalezeno v DPC frontì civilizace
		// zjistí, je-li DPC pøipravováno k provedení
		if ( m_pPreparingDPC == pDPC )
		{	// DPC je pøipravováno k provedení
			// odregistruje pøipravované DPC
			m_pPreparingDPC = NULL;
			// nastaví návratový pøíznak odregistrování DPC
			bUnregistered = TRUE;
		}
		else
		{	// DPC není pøipravováno k provedení
			// smaže návratový pøíznak odregistrování DPC
			bUnregistered = FALSE;
		}
	}
	else
	{	// DPC bylo úspìšnì smazáno
		// nastaví návratový pøíznak odregistrování DPC
		bUnregistered = TRUE;
	}

	// odemkne DPC fronty civilizací
	VERIFY ( m_mutexDPCQueueLock.Unlock () );

	// vrátí pøíznak odregistrování DPC
	return bUnregistered;
}

// odregistruje odložené volání procedury "pDPC" (FALSE=DPC nebylo zaregistrované)
BOOL CSMap::UnregisterDPC ( CSDeferredProcedureCall *pDPC ) 
{
	ASSERT ( pDPC != NULL );

	// zamkne DPC fronty civilizací
	VERIFY ( m_mutexDPCQueueLock.Lock () );

	// projede DPC fronty všech civilizací
	for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; )
	{
		// pokusí se smazat DPC z DPC fronty civilizace
		if ( m_aDPCQueue[dwCivilizationIndex].RemoveElement ( pDPC ) )
		{	// DPC bylo z DPC fronty civilizace smazáno
			// odemkne DPC fronty civilizací
			VERIFY ( m_mutexDPCQueueLock.Unlock () );

			// vrátí pøíznak úspìšného odregistrování DPC
			return TRUE;
		}
	}
	// DPC nebylo nalezeno v DPC frontách civilizací

	// pøíznak úspìšného odregistrování DPC
	BOOL bUnregistered;

	// zjistí, je-li DPC pøipravováno k provedení
	if ( m_pPreparingDPC == pDPC )
	{	// DPC je pøipravováno k provedení
		// odregistruje pøipravované DPC
		m_pPreparingDPC = NULL;
		// nastaví návratový pøíznak odregistrování DPC
		bUnregistered = TRUE;
	}
	else
	{	// DPC není pøipravováno k provedení
		// smaže návratový pøíznak odregistrování DPC
		bUnregistered = FALSE;
	}

	// odemkne DPC fronty civilizací
	VERIFY ( m_mutexDPCQueueLock.Unlock () );

	// vrátí pøíznak odregistrování DPC
	return bUnregistered;
}

//////////////////////////////////////////////////////////////////////
// Vlákna výpoètu serveru hry
//////////////////////////////////////////////////////////////////////

// hlavní smyèka mapy
void CSMap::MainLoop () 
{
	ASSERT ( !m_eventMainLoopPaused.Lock ( 0 ) );

#ifdef _DEBUG
	// inicializuje ID MainLoop threadu
	ASSERT ( m_dwMainLoopThreadID == DWORD_MAX );
	m_dwMainLoopThreadID = GetCurrentThreadId ();

	// inicializuje ID privilegovaného threadu zámkù blokù jednotek
	{
		// projede všechny bloky jednotek
		for ( SUnitBlock *pBlock = m_pFirstUnitBlock; pBlock != NULL; 
			pBlock = pBlock->pNext )
		{
			// inicializuje ID privilegovaného threadu zámku bloku jednotek
			pBlock->cLock.SetPrivilegedThreadID ( m_dwMainLoopThreadID );
		}
	}

	// inicializuje ID privilegovaného threadu zámku mrtvých a smazaných jednotek
	m_cDeadDeletedUnitLock.SetPrivilegedThreadID ( m_dwMainLoopThreadID );
#endif //_DEBUG

	ASSERT ( CheckValidData () );

// promìnné hlavního cyklu mapy

	// pøíznak TimeSlicu s poèítáním viditelnosti
	BOOL bVisibilityTimeSlice;

	// nastaví èasovaè TimeSlicù
	DWORD dwTimeSliceLength = m_dwTimeSliceLength;
	ASSERT ( dwTimeSliceLength > 0 );
	UINT nTimer = SetTimer ( NULL, 0, dwTimeSliceLength, NULL );

	// hlavní cyklus TimeSlicù
	for ( ; ; m_dwTimeSlice++ )
	{
	// pomocné promìnné cyklu

		// ukazatel na aktuální blok jednotek
		SUnitBlock *pBlock;

	// zpracuje zapausování hry

		// zjistí, je-li hra zapausovaná
		if ( m_eventPauseGame.Lock ( 0 ) )
		{	// hra je zapausovaná
			// nastaví událost zapausování MainLoop threadu
			VERIFY ( m_eventMainLoopPaused.SetEvent () );

			// ukonèí èasovaè TimeSlicù
			VERIFY ( KillTimer ( NULL, nTimer ) );

			// uspí thread a zjistí, má-li se pokraèovat ve høe
			if ( !CanContinueGame () )
			{	// hra se má ukonèit
				#ifdef _DEBUG
					// aktualizuje ID MainLoop threadu
					m_dwMainLoopThreadID = DWORD_MAX;
				#endif //_DEBUG
				// ukonèí hru
				return;
			}
			// hra má pokraèovat

			// nastaví èasovaè TimeSlicù
			dwTimeSliceLength = m_dwTimeSliceLength;
			ASSERT ( dwTimeSliceLength > 0 );
			nTimer = SetTimer ( NULL, 0, dwTimeSliceLength, NULL );
		}

		TRACE_LOOP1 ( _T("* TimeSlice: %d\n"), m_dwTimeSlice );

	// inicializace promìnných cyklu

		// nastaví pøíznak TimeSlicu s poèítáním viditelnosti
		bVisibilityTimeSlice = IsVisibilityTimeSlice ( m_dwTimeSlice );

	// provádìní TimeSlicu (cyklus pøes všechny jednotky a jejich skilly)

		// pøíznak zamèení MapSquarù MainLoopem
		BOOL bMapSquaresLocked = TRUE;
		// zamkne MapSquary
		VERIFY ( m_mutexMapSquaresLock.Lock () );

		// projede všechny bloky jednotek
		for ( pBlock = m_pFirstUnitBlock; pBlock != NULL; pBlock = pBlock->pNext )
		{
			// ukazatel na jednotku v bloku
			CSUnit **pBlockUnit = pBlock->pUnits;
			// zamkne blok jednotek pro zápis
			VERIFY ( pBlock->cLock.PrivilegedThreadWriterLock () );
			// projede všechny jednotky v bloku jednotek
			for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
			{
				// získá ukazatel na jednotku
				CSUnit *pUnit = *pBlockUnit;

				// zjistí, je-li to platná jednotka
				if ( pUnit == NULL )
				{	// není to platná jednotka
					continue;
				}

				// poèet skill jednotky
				DWORD dwSkillCount = pUnit->GetUnitType ()->GetSkillCount ();
				// získá ukazatel na popis typu skilly
				CSUnitType::SSkillTypeDescription *pSkillTypeDescription = 
					pUnit->GetUnitType ()->m_aSkillTypeDescription;
				// maska módu jednotky
				BYTE nModeMask = pUnit->GetModeMask ();

				ASSERT ( pUnit->m_nLives > 0 );

				// provede TimeSlice na skillách jednotky
				for ( DWORD dwSkillIndex = 0; dwSkillIndex < dwSkillCount; dwSkillIndex++, 
					pSkillTypeDescription++ )
				{
					// zjistí, je-li skilla zapnuta (enabled)
					if ( !pSkillTypeDescription->bEnabled )
					{	// skilla není zapnuta
						continue;
					}
					// zjistí, je-li skilla v tomto módu povolena
					if ( !( pSkillTypeDescription->nAllowedModesMask & nModeMask ) )
					{	// skilla není v tomto módu povolena
						continue;
					}
					// zjistí, je-li skilla aktivní
					if ( !pUnit->IsSkillActive ( dwSkillIndex ) )
					{	// skilla není aktivní
						continue;
					}
					// provede TimeSlice na skille jednotky
					pSkillTypeDescription->pSkillType->DoTimeSlice ( pUnit );
				}
				// byl proveden TimeSlice na skillách jednotky
			}
			// byl zpracován celý blok jednotek

			// odemkne zamèený blok jednotek pro zápis
			pBlock->cLock.PrivilegedThreadWriterUnlock ();
		}

	// smaže mrtvé jednotky

		// zjistí, jedná-li se o TimeSlice mazání mrtvých jednotek
		if ( IsDeletingDeadUnitsTimeSlice ( m_dwTimeSlice ) )
		{
		// smaže mrtvé jednotky

			// ukazatel na popis mrtvé jednotky
			struct SDeadUnit sDeadUnit;

			// smaže mrtvé jednotky
			while ( m_cDeadUnitQueue.Get ( sDeadUnit ) )
			{
				// znièí skilly jednotky
				sDeadUnit.pUnit->m_pUnitType->DestructSkills ( sDeadUnit.pUnit );

				// zjistí, má-li být mrtvá jednotka již smazána
				if ( sDeadUnit.dwTimeSlice <= m_dwTimeSlice )
				{	// jednotka má být již smazána
					// odstraní jednotku z fronty mrtvých jednotek
					VERIFY ( m_cDeadUnitQueue.RemoveFirst ( sDeadUnit ) );

					// zamkne seznam smazaných jednotek
					VERIFY ( m_mutexDeletedUnitListLock.Lock () );
					// pøidá jednotku do seznamu smazaných jednotek
					m_cDeletedUnitList.Add ( sDeadUnit.pUnit );
					// odemkne seznam smazaných jednotek
					VERIFY ( m_mutexDeletedUnitListLock.Unlock () );

					// uvolní odkaz na jednotku
					sDeadUnit.pUnit->Release ();
				}
				else
				{
					break;
				}
				// jednotka ještì nemá být smazána
			}

		// ukonèí skilly smazaných jednotek

			// zamkne seznam smazaných jednotek
			VERIFY ( m_mutexDeletedUnitListLock.Lock () );

			// ukazatel na smazanou jednotku
			CSUnit *pDeletedUnit;
			// pozice smazané jednotky v seznamu smazaných jednotek
			POSITION posDeletedUnit = m_cDeletedUnitList.GetHeadPosition ();

			// projede seznam smazaných jednotek
			while ( m_cDeletedUnitList.GetNext ( posDeletedUnit, pDeletedUnit ) )
			{
				// znièí skilly smazané jednotky
				pDeletedUnit->m_pUnitType->DestructSkills ( pDeletedUnit );
			}

			// odemkne seznam smazaných jednotek
			VERIFY ( m_mutexDeletedUnitListLock.Unlock () );
		}

	// øešení viditelnosti

		// zjistí, má-li se poèítat viditelnost
		if ( bVisibilityTimeSlice )
		{	// má se poèítat viditelnost
			// projede všechny bloky jednotek
			for ( pBlock = m_pFirstUnitBlock; pBlock != NULL; pBlock = pBlock->pNext )
			{
				// ukazatel na jednotku v bloku
				CSUnit **pBlockUnit = pBlock->pUnits;
				// projede všechny jednotky v bloku jednotek
				for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
				{
					// získá ukazatel na jednotku
					CSUnit *pUnit = *pBlockUnit;

					// zjistí, je-li to platná jednotka
					if ( pUnit == NULL )
					{	// není to platná jednotka
						continue;
					}
					// je to platná jednotka

					// vidìné MapSquary
					CSMapSquare *aSeenMapSquares[4];

					// zjistí vidìné MapSquary
					GetSeenMapSquares ( pUnit, aSeenMapSquares );

					// zjistí index civilizace jednotky
					DWORD dwUnitCivilizationIndex = pUnit->GetCivilizationIndex ();

					// zjistí pozici jednotky
					CPointDW pointUnitPosition = pUnit->GetPosition ();
					// zjistí druhou mocninu polomìru viditelnosti jednotky
					DWORD dwUnitViewRadiusSquare = pUnit->GetViewRadiusSquare ();
					// zjistí odkaz na seznam vidìných jednotek
					CSUnitList *pSeenUnitList = pUnit->GetSeenUnitList ();
					// zjistí bitovou masku pøíznakù detekce neviditelnosti ostatních 
					//		jednotek
					DWORD dwUnitInvisibilityDetection = pUnit->GetInvisibilityDetection ();

					// seznam naposledy vidìných jednotek
					CSUnitList cLastSeenUnitList;
					// pøesune seznam jednotek do seznamu naposledy vidìných jednotek
					pSeenUnitList->MoveList ( cLastSeenUnitList );

					// projede vidìné MapSquary
					for ( int nIndex = 0; ( nIndex < 4 ) && ( aSeenMapSquares[nIndex] != NULL ); nIndex++ )
					{
						// zjistí, je-li v MapSquaru cizí jednotka
						if ( aSeenMapSquares[nIndex]->IsExclusive ( dwUnitCivilizationIndex ) )
						{	// v MapSquaru není cizí jednotka
							continue;
						}
						// v MapSquaru je cizí jednotka

						// zjistí ukazatel na seznam jednotek vidìného MapSquaru
						CSUnitList *pSeenMapSquareUnitList = aSeenMapSquares[nIndex]->GetUnitList ();

						// zjistí pozici první jednotky seznamu jednotek ve vidìném MapSquaru
						POSITION posSeenMapSquareUnit = pSeenMapSquareUnitList->GetHeadPosition ();
						// jednotka z vidìného MapSquaru
						CSUnit *pSeenMapSquareUnit;

						// projede jednotky v seznamu jednotek vidìného MapSquaru
						while ( CSUnitList::GetNext ( posSeenMapSquareUnit, pSeenMapSquareUnit ) )
						{
							// zjistí index civilizace vidìné jednotky
							DWORD dwSeenUnitCivilizationIndex = 
								pSeenMapSquareUnit->GetCivilizationIndex ();

							// zjistí, je-li jednotka ze stejné civilizace
							if ( dwSeenUnitCivilizationIndex == dwUnitCivilizationIndex )
							{	// jednotky jsou ze stejné civilizace
								continue;
							}

							// zjistí, je-li jednotka skuteènì vidìt
							if ( ( pointUnitPosition.GetDistanceSquare ( pSeenMapSquareUnit->GetPosition () ) < 
								dwUnitViewRadiusSquare ) && pSeenMapSquareUnit->IsVisible ( dwUnitInvisibilityDetection ) )
							{	// jednotka je skuteènì vidìt
								// najde jednotku v seznamu naposledy vidìných jednotek
								POSITION posSeenUnit = cLastSeenUnitList.Find ( pSeenMapSquareUnit );
								// zjistí, byla-li jednotka již vidìna
								if ( posSeenUnit != NULL )
								{	// jednotka již byla vidìna
									ASSERT ( !cLastSeenUnitList.IsEmpty ( posSeenUnit ) );
									// pøesune jednotku do vidìných jednotek
									cLastSeenUnitList.Move ( posSeenUnit, *pSeenUnitList );
								}
								else
								{	// jednotka dosud nebyla vidìt
									// pøidá jednotku do seznamu vidìných jednotek
									pSeenUnitList->Add ( pSeenMapSquareUnit );
									// zjistí, jedná-li se o nepøátelskou jednotku
									if ( dwSeenUnitCivilizationIndex != 0 )
									{	// jedná se o nepøátelskou jednotku
										// zvýší poèet jednotek své civilizace, které vidìnou jednotku vidí
										if ( pSeenMapSquareUnit->m_aCivilizationVisibility[dwUnitCivilizationIndex]++ == 0 )
										{	// jednotka se objevila v dohledu civilizace civilizaci
											// nastaví pøíznak zmìny viditelnosti jednotky civilizací
											pSeenMapSquareUnit->m_cCivilizationVisibilityChanged.SetAt ( dwUnitCivilizationIndex );
											// pošle hlavnímu veliteli notifikaci o zviditelnìní nepøátelské jednotky civilizací
											SendNotification ( m_pCivilizations[dwUnitCivilizationIndex].GetZCivilization ()->GetGeneralCommander (), 
												NOTIFICATION_ENEMY_UNIT_SEEN_BY_CIVILIZATION, 
												pSeenMapSquareUnit );
										}
										// pošle jednotce notifikaci o zviditelnìní nepøátelské jednotky
										SendNotification ( pUnit->GetZUnit (), 
											NOTIFICATION_ENEMY_UNIT_SEEN, pSeenMapSquareUnit );
									}
									else
									{	// jedná se o systémovou jednotku
										// zvýší poèet jednotek své civilizace, které vidìnou jednotku vidí
										if ( pSeenMapSquareUnit->m_aCivilizationVisibility[dwUnitCivilizationIndex]++ == 0 )
										{	// jednotka se objevila v dohledu civilizace civilizaci
											// nastaví pøíznak zmìny viditelnosti jednotky civilizací
											pSeenMapSquareUnit->m_cCivilizationVisibilityChanged.SetAt ( dwUnitCivilizationIndex );
											// pošle hlavnímu veliteli notifikaci o zviditelnìní systémové jednotky civilizací
											SendNotification ( 
												m_pCivilizations[dwUnitCivilizationIndex].GetZCivilization ()->GetGeneralCommander (), 
												NOTIFICATION_SYSTEM_UNIT_SEEN_BY_CIVILIZATION, 
												pSeenMapSquareUnit );
										}
										// pošle jednotce notifikaci o zviditelnìní systémové jednotky
										SendNotification ( pUnit->GetZUnit (), 
											NOTIFICATION_SYSTEM_UNIT_SEEN, pSeenMapSquareUnit );
									}
								}
							}
						}
					}
					// byly projety vidìné MapSquary

					// jednotka, která pøestala být vidìt
					CSUnit *pLastSeenUnit;
					// projede jednotky, které pøestaly být vidìt
					while ( cLastSeenUnitList.RemoveFirst ( pLastSeenUnit ) )
					{
						ASSERT ( pLastSeenUnit->m_aCivilizationVisibility[dwUnitCivilizationIndex] > 0 );
						// sníží poèet jednotek své civilizace, které vidìnou jednotku vidí
						if ( --pLastSeenUnit->m_aCivilizationVisibility[dwUnitCivilizationIndex] == 0 )
						{	// jednotka zmizela civilizaci z dohledu
							// nastaví pøíznak zmìny viditelnosti jednotky civilizací
							pLastSeenUnit->m_cCivilizationVisibilityChanged.SetAt ( dwUnitCivilizationIndex );
							// naposledy vidìná pozice jednotky, která pøestala být vidìt
							CSPosition *pPosition = 
								pLastSeenUnit->m_aCivilizationLastSeenPosition + 
								dwUnitCivilizationIndex;
							// zamkne jednotku, která pøestala být vidìt na zápis
							VERIFY ( pLastSeenUnit->PrivilegedThreadWriterLock () );
							// zjistí, je-li jednotka, která pøestala být vidìt, v okolí jednotky
							if ( pointUnitPosition.GetDistanceSquare ( pLastSeenUnit->GetPosition () ) <= 
								UNIT_VIEW_RADIUS_TOLERANCE ( dwUnitViewRadiusSquare ) )
							{	// jednotka, která pøestala být vidìt, je v okolí jednotky
								// aktualizuje naposledy vidìnou pozici jednotky civilizací
								*(CPointDW *)pPosition = pLastSeenUnit->m_pointPosition;
								pPosition->z = pLastSeenUnit->m_dwVerticalPosition;
								ASSERT ( pPosition->z != 0 );
							}
							else
							{	// jednotka, která pøestala být vidìt, není v okolí jednotky
								// aktualizuje naposledy vidìnou pozici jednotky civilizací
								pPosition->x = pPosition->y = NO_MAP_POSITION;
								pPosition->z = 0;
							}
							// odemkne jednotku, která pøestala být vidìt na zápis
							pLastSeenUnit->PrivilegedThreadWriterUnlock ();
						}
						// zjistí, jedná-li se o nepøátelskou jednotku
						if ( pLastSeenUnit->GetCivilizationIndex () != 0 )
						{	// jedná se o nepøátelskou jednotku
							// pošle jednotce notifikaci o ukonèení viditelnosti nepøátelské jednotky
							SendNotification ( pUnit->GetZUnit (), 
								NOTIFICATION_ENEMY_UNIT_DISAPPEARED, pLastSeenUnit );
						}
						else
						{	// jedná se o systémovou jednotku
							// pošle jednotce notifikaci o ukonèení viditelnosti systémové jednotky
							SendNotification ( pUnit->GetZUnit (), 
								NOTIFICATION_SYSTEM_UNIT_DISAPPEARED, pLastSeenUnit );
						}
					}
					// rozeslal notifikace o jednotkách, které pøestaly být vidìt
				}
				// byly projety všechny jednotky v bloku jednotek
			}
			// byly projety všechny bloky jednotek
		}
		// viditelnost byla spoèítána

	// upozorní ClientInfoSenderLoop na ukonèení TimeSlicu

		// pustí ClientInfoSenderLoop èekající na konec TimeSlicu
		VERIFY ( m_eventClientInfoSenderLoopTimeSliceFinished.SetEvent () );

	// vyøizování odložených volání procedur (DPC)

		// provádí DPC dokud èasovaè nenastaví událost nového TimeSlicu
		do
		{
			// pøíznak volání DPC
			BOOL bDPCCalled = FALSE;

			// projede DPC fronty civilizací
			for ( m_dwRunningDPCCivilizationIndex = m_dwCivilizationCount; 
				m_dwRunningDPCCivilizationIndex-- > 0; )
			{
				// zjistí, je-li DPC fronta civilizace prázdná
				if ( !m_aDPCQueue[m_dwRunningDPCCivilizationIndex].IsEmpty () )
				{	// DPC fronta civilizace není prázdná
					// zamkne DPC fronty civilizací
					VERIFY ( m_mutexDPCQueueLock.Lock () );

					// ukazatel na DPC
					CSDeferredProcedureCall *pDPC;

					// odebere první DPC z DPC fronty civilizace
					VERIFY ( m_aDPCQueue[m_dwRunningDPCCivilizationIndex].RemoveFirst ( 
						pDPC ) );

					// zjistí asociovanou jednotku DPC
					CSUnit *pUnit = pDPC->GetUnit ();

					// zjistí, má-li DPC asociovánu jednotku
					if ( pUnit != NULL )
					{	// DPC má asociovánu jednotku
					// pøipraví DPC na provedení

						// zjistí pøíznak zamykání jednotky pro zápis
						BOOL bWriterLock = pDPC->NeedWriterLock ();

						// aktualizuje pøipravované DPC
						m_pPreparingDPC = pDPC;

						// odemkne DPC fronty civilizací
						VERIFY ( m_mutexDPCQueueLock.Unlock () );

						// zjistí, má-li se asociovaná jednotka DPC zamykat pro zápis
						if ( bWriterLock )
						{	// asociovaná jednotka DPC se má zamykat pro zápis
							// zamkne asociovanou jednotku DPC pro zápis
							VERIFY ( pUnit->PrivilegedThreadWriterLock () );
						}
						else
						{	// asociovaná jednotka DPC se má zamykat pro ètení
							// zamkne asociovanou jednotku DPC pro ètení
							VERIFY ( pUnit->PrivilegedThreadReaderLock () );
						}

					// provede DPC

						// zamkne DPC fronty civilizací
						VERIFY ( m_mutexDPCQueueLock.Lock () );

						// zjistí pøíznak odregistrování DPC
						BOOL bDPCUnregistered = ( m_pPreparingDPC == NULL );

						// aktualizuje pøipravované DPC
						m_pPreparingDPC = NULL;

						// odemkne DPC fronty civilizací
						VERIFY ( m_mutexDPCQueueLock.Unlock () );

						// zjistí, bylo-li DPC odregistrováno
						if ( !bDPCUnregistered )
						{	// DPC nebylo odregistrováno
							// provede DPC
							pDPC->Run ();
							// znièí DPC
							delete pDPC;
						}

					// uklidí po provedeném DPC

						// zjistí, byla-li asociovaná jednotka DPC zamèena pro zápis
						if ( bWriterLock )
						{	// asociovaná jednotka DPC byla zamèena pro zápis
							// odemkne asociovanou jednotku DPC pro zápis
							pUnit->PrivilegedThreadWriterUnlock ();
						}
						else
						{	// asociovaná jednotka DPC byla zamèena pro ètení
							// odemkne asociovanou jednotku DPC pro ètení
							pUnit->PrivilegedThreadReaderUnlock ();
						}
					}
					else
					{	// DPC nemá asociovánu jednotku
					// provede DPC

						// odemkne DPC fronty civilizací
						VERIFY ( m_mutexDPCQueueLock.Unlock () );

						// provede DPC
						pDPC->Run ();
						// znièí DPC
						delete pDPC;
					}

					// nastaví pøíznak volání DPC
					bDPCCalled = TRUE;
				}
			}

			// zjistí, bylo-li voláno nìkteré DPC
			if ( !bDPCCalled )
			{	// nebylo voláno žádné DPC
				// odemkne MapSquary
				VERIFY ( m_mutexMapSquaresLock.Unlock () );
				// aktualizuje pøíznak zamèení MapSquarù MainLoopem
				bMapSquaresLocked = FALSE;

				// poèká na nové pøíchozí DPC
				if ( WaitForNewTimeSlice ( m_dwTimeSliceLength / DPC_INCOME_TEST_DELAY_RATE ) )
				{	// nastala událost nového TimeSlicu
					// ukonèí vyøizování odložených volání procedur (DPC)
					break;
				}
				// zamkne MapSquary
				VERIFY ( m_mutexMapSquaresLock.Lock () );
				// aktualizuje pøíznak zamèení MapSquarù MainLoopem
				bMapSquaresLocked = TRUE;

				// nechá zkontrolovat novì pøíchozí DPC
				continue;
			}
		} while ( !WaitForNewTimeSlice ( 0 ) );

		// zjistí, jsou-li MapSquary zamèeny MainLoopem
		if ( bMapSquaresLocked )
		{	// MapSquary jsou zamèeny MainLoopem
			// odemkne MapSquary
			VERIFY ( m_mutexMapSquaresLock.Unlock () );
			// aktualizuje pøíznak zamèení MapSquarù MainLoopem
			bMapSquaresLocked = FALSE;
		}

	// aktualizuje délku TimeSlicu

		// zjistí, zmìnila-li se délka TimeSlicu
		if ( dwTimeSliceLength != m_dwTimeSliceLength )
		{	// zmìnila se délka TimeSlicu
			// ukonèí èasovaè TimeSlicù
			VERIFY ( KillTimer ( NULL, nTimer ) );

			// nastaví èasovaè TimeSlicù
			dwTimeSliceLength = m_dwTimeSliceLength;
			ASSERT ( dwTimeSliceLength > 0 );
			nTimer = SetTimer ( NULL, 0, dwTimeSliceLength, NULL );
		}

	// zpracuje ukonèení TimeSlicu

		VERIFY ( m_cMapObserverListLock.Lock ( ) );

		CSMapObserver *pObserver = m_pFirstMapObserver;
		while ( pObserver != NULL )
		{
			pObserver->OnTimeslice ( m_dwTimeSlice );
			pObserver = pObserver->m_pNextObserver;
		}

		VERIFY ( m_cMapObserverListLock.Unlock ( ) );
	}
	// skonèil hlavní cyklus TimeSlicù

#ifdef _DEBUG
	// zneškodní ID MainLoop threadu
	m_dwMainLoopThreadID = DWORD_MAX;
#endif //_DEBUG
}

// smyèka rozesílání informací klientùm civilizací
void CSMap::ClientInfoSenderLoop () 
{
	ASSERT ( !m_eventClientInfoSenderLoopPaused.Lock ( 0 ) );

// promìnné hlavního cyklu rozesílání informací o jednotkách

	// brief info jednotky
	SUnitBriefInfo sBriefInfo;
	// start info jednotky
	SUnitStartInfo sStartInfo;
	// full info jednotky
	SUnitFullInfo sFullInfo;
	// enemy full info jednotky
	SUnitEnemyFullInfo sEnemyFullInfo;
	// start full info jednotky
	SUnitStartFullInfo sStartFullInfo;
	// start enemy full info jednotky
	SUnitStartEnemyFullInfo sStartEnemyFullInfo;
	// check point info jednotky
	SUnitCheckPointInfo sCheckPointInfo;

	// ukazatel na klienta, kterému se posílá MiniMapa
	CZClientUnitInfoSender *pMiniMapClient = NULL;
	// index klienta, kterému se posílá MiniMapa
	DWORD dwMiniMapClientIndex = 0;
	// index civilizace klienta, kterému se posílá MiniMapa
	DWORD dwMiniMapClientCivilizationIndex = 0;

// inicializace rozesílání MiniMapy

	// inicializace rozesílání MiniMapy
	CZClientUnitInfoSender::InitMiniMapServer ();

// hlavní cyklus rozesílání informací klientùm civilizací
	for ( ; ; )
	{
		// tabulka událostí, na které se èeká
		HANDLE aNewLoopEvents[3] = { 
			(HANDLE)m_eventClientInfoSenderLoopTimeSliceFinished, 
			(HANDLE)m_eventWatchingMapSquareRequest, 
			(HANDLE)m_eventPauseGame,
		};

		// èíslo posledního TimeSlicu pøed èekáním na ukonèení TimeSlicu
		DWORD dwLastTimeSlice = m_dwTimeSlice;
		// pøíznak ukonèení TimeSlicu
		BOOL bTimeSliceFinished = FALSE;

		// cyklus èekání na konec TimeSlicu
		do
		{
			// poèká na konec TimeSlicu nebo na požadavek sledování MapSquaru
			switch ( WaitForMultipleObjects ( 3, aNewLoopEvents, FALSE, INFINITE ) )
			{
			// TimeSlice byl ukonèen
			case WAIT_OBJECT_0 :
				// nastaví pøíznak ukonèení TimeSlicu
				bTimeSliceFinished = TRUE;
				break;
			// pøišel požadavek na sledování MapSquaru
			case ( WAIT_OBJECT_0 + 1 ) :
				{
				// vyøídí požadavky na sledování MapSquarù

					// zamkne MapSquary
					VERIFY ( m_mutexMapSquaresLock.Lock () );

					// zamkne seznamy zaregistrovaných klientù civilizací pro ètení
					VERIFY ( m_cRegisteredClientListLock.ReaderLock () );

					// zamkne frontu požadavkù na sledování MapSquarù
					VERIFY ( m_mutexWatchingMapSquareRequestQueueLock.Lock () );

					// informace o požadavku na sledování MapSquaru
					struct SWatchingMapSquareRequestInfo sRequestInfo;

					// vyjme první požadavek na sledování MapSquaru z fronty
					while ( m_cWatchingMapSquareRequestQueue.RemoveFirst ( sRequestInfo ) )
					{
						// zjistí klienta požadavku na sledování MapSquaru
						CZClientUnitInfoSender *pClient = sRequestInfo.pClient;
						// zjistí MapSquare požadavku na sledování MapSquaru
						CSMapSquare *pMapSquare = sRequestInfo.pMapSquare;
						// zjistí index civilizace klienta
						DWORD dwCivilizationIndex = pClient->GetCivilizationIndex ();

						// pøíznak, zmìnila-li se sledovanost MapSquaru
						BOOL bWatchedMapSquareChanged;

						// najde klienta civilizace
						POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].FindClient ( 
							pClient );
						ASSERT ( posClient != NULL );

						// získá informace o klientovi civilizace
						struct SRegisteredClientInfo sClientInfo;
						VERIFY ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) );

						// zjistí, jedná-li se o zaèátek sledování MapSquaru
						if ( sRequestInfo.bStartWatchingMapSquare )
						{	// jedná se o požadavek na zaèátek sledování MapSquaru
							// zjistí, je-li MapSquare klientem již sledován
							bWatchedMapSquareChanged = !sClientInfo.pWatchedMapSquareArray->Find ( pMapSquare );

							TRACE_COMMUNICATION3 ( _T("#	Starting watch MapSquare ( Client=%x, MapSquare=(%d, %d) )\n"), pClient, pMapSquare->GetIndex ().x, pMapSquare->GetIndex ().y );

							// pøidá MapSquare do pole klientem sledovaných MapSquarù
							sClientInfo.pWatchedMapSquareArray->Add ( pMapSquare );

							// aktualizuje pøíznak sledování MapSquaru civilizací
							pMapSquare->m_cWatchingCivilizations.SetAt ( dwCivilizationIndex );
						}
						else
						{	// jedná se o požadavek na ukonèení sledování MapSquaru
							ASSERT ( sClientInfo.pWatchedMapSquareArray->Find ( pMapSquare ) );

							TRACE_COMMUNICATION3 ( _T("#	Stopping watch MapSquare ( Client=%x, MapSquare=(%d, %d) )\n"), pClient, pMapSquare->GetIndex ().x, pMapSquare->GetIndex ().y );

							// odebere MapSquare z pole klientem sledovaných MapSquarù
							sClientInfo.pWatchedMapSquareArray->RemoveElement ( pMapSquare );

							// zjistí, je-li MapSquare klientem stále sledován
							bWatchedMapSquareChanged = !sClientInfo.pWatchedMapSquareArray->Find ( pMapSquare );

							// zjistí, je-li MapSquare klientem stále sledován
							if ( bWatchedMapSquareChanged )
							{	// MapSquare již není klientem sledován
							// aktualizuje pøíznak sledování MapSquaru civilizací

								// zjistí pozici prvního klienta civilizace
								posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();

								// pøíznak sledování MapSquaru civilizací
								BOOL bMapSquareWatched = FALSE;

								// projede klienty civilizace
								while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
								{
									ASSERT ( ( sClientInfo.pClient != NULL ) && 
										( sClientInfo.pWatchedMapSquareArray != NULL ) );

									// zjistí, je-li MapSquare sledován klientem civilizace
									if ( sClientInfo.pWatchedMapSquareArray->Find ( pMapSquare ) )
									{	// MapSquare je sledován klientem civilizace
										// nastaví pøíznak sledování MapSquaru civilizací
										bMapSquareWatched = TRUE;
										break;
									}
								}
								// projel klienty civilizace

								// zjistí, je-li MapSquare sledován civilizací
								if ( !bMapSquareWatched )
								{	// MapSquare již není sledován civilizací
									ASSERT ( pMapSquare->m_cWatchingCivilizations.GetAt ( dwCivilizationIndex ) );
									// smaže pøíznak sledování MapSquaru civilizací
									pMapSquare->m_cWatchingCivilizations.ClearAt ( dwCivilizationIndex );
								}
							}
							// byl aktualizován pøíznak sledování MapSquaru civilizací
						}
						// požadavek na sledování MapSquaru byl zpracován

						// zjistí, zmìnila-li se sledovanost MapSquaru
						if ( bWatchedMapSquareChanged )
						{	// sledovanost MapSquaru se zmìnila
						// nastaví všem jednotkám MapSquaru pøíznak zmìny pozice jednotky 
						//		z/do sledovaných MapSquarù

							// zjistí seznam jednotek MapSquaru
							CSUnitList *pUnitList = pMapSquare->GetUnitList ();

							// ukazatel na jednotku MapSquaru
							CSUnit *pUnit;

							// získá pozici první jednotky MapSquaru
							POSITION posUnit = pUnitList->GetHeadPosition ();
							// projede jednotky MapSquaru
							while ( CSUnitList::GetNext ( posUnit, pUnit ) )
							{
								// nastaví jednotce pøíznak zmìny pozice jednotky z/do 
								//		sledovaných MapSquarù
								pUnit->m_cWatchedMapSquareChanged.SetAt ( dwCivilizationIndex );
							}
							// všechny jednotky MapSquaru byly projety
						}
						// byly zmìnìny pøípadné zmìny pozice jednotky z/do sledovaných 
						//		MapSquarù
					}
					// všechny požadavky na sledování MapSquarù byly zpracovány

					// smaže událost požadavku na sledování MapSquaru
					VERIFY ( m_eventWatchingMapSquareRequest.ResetEvent () );

					// odemkne frontu požadavkù na sledování MapSquarù
					VERIFY ( m_mutexWatchingMapSquareRequestQueueLock.Unlock () );

					// odemkne seznamy zaregistrovaných klientù civilizací pro ètení
					m_cRegisteredClientListLock.ReaderUnlock ();

					// odemkne MapSquary
					VERIFY ( m_mutexMapSquaresLock.Unlock () );
				}
				break;
			// hra byla zapausována
			case ( WAIT_OBJECT_0 + 2 ) :
				// nastaví událost zapausování ClientInfoSenderLoop threadu
				VERIFY ( m_eventClientInfoSenderLoopPaused.SetEvent () );

				// uspí thread a zjistí, má-li se pokraèovat ve høe
				if ( !CanContinueGame () )
				{	// hra se má ukonèit
					// ukonèí rozesílání minimapy
					CZClientUnitInfoSender::DoneMiniMapServer ();

					// ukonèí hru
					return;
				}
				// hra má pokraèovat
				break;
			// nemožný pøípad
			default:
				ASSERT ( FALSE );
			}

			// zjistí zmìnilo-li se èíslo TimeSlicu
			if ( dwLastTimeSlice != m_dwTimeSlice )
			{	// zmìnilo se èíslo TimeSlicu
				// nastaví pøíznak ukonèení TimeSlicu
				bTimeSliceFinished = TRUE;
			}
		} while ( !bTimeSliceFinished );

		TRACE_LOOP1 ( _T("* Sending client info ( TimeSlice = %d )\n"), m_dwTimeSlice );

	// zaène rozesílat informace o jednotkách

		// zamkne seznamy zaregistrovaných klientù civilizací pro ètení
		VERIFY ( m_cRegisteredClientListLock.ReaderLock () );

		// upozorní klienty civilizací na zaèátek rozesílání informací o jednotkách
		{
			// projede civilizace
			for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; )
			{
				// informace o zaregistrovaném klientovi
				struct SRegisteredClientInfo sClientInfo;

				// zjistí pozici prvního klienta civilizace
				POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
				// projede klienty civilizace
				while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
				{
					TRACE_COMMUNICATION1 ( _T("#		StartSendingInfo ( Client=%x )\n"), sClientInfo.pClient );
					// upozorní klienta na zaèátek rozesílání informací o jednotkách
					sClientInfo.pClient->StartSendingInfo ();
					TRACE_COMMUNICATION1 ( _T("#	SendResourceInfo ( Client=%x )\n"), sClientInfo.pClient );
					// pošle klientovi informace o surovinách
					sClientInfo.pClient->SendResourceInfo ( 
						m_pCivilizations[sClientInfo.pClient->GetCivilizationIndex ()].GetResources () );
				}
			}
		}

	// vyøídí požadavky na sledování jednotek

		// zamkne frontu požadavkù na sledování jednotek
		VERIFY ( m_mutexWatchingUnitRequestQueueLock.Lock () );

		// zjistí, je-li fronta požadavkù na sledování jednotek prázdná
		if ( !m_cWatchingUnitRequestQueue.IsEmpty () )
		{	// fronta požadavkù na sledování jednotek není prázdná
			// informace o požadavku na sledování jednotky
			struct SWatchingUnitRequestInfo sRequestInfo;

			// zamkne seznam sledovaných jednotek
			VERIFY ( m_mutexWatchedUnitListLock.Lock () );

			// vyjme první požadavek na sledování jednotky z fronty
			while ( m_cWatchingUnitRequestQueue.RemoveFirst ( sRequestInfo ) )
			{
				// zjistí klienta požadavku na sledování jednotky
				CZClientUnitInfoSender *pClient = sRequestInfo.pClient;

				// zjistí, jedná-li se o zaèátek sledování jednotky
				if ( sRequestInfo.bStartWatchingUnit )
				{	// jedná se o požadavek na zaèátek sledování jednotky
					// zjistí jednotku požadavku na sledování jednotky
					CSUnit *pUnit = sRequestInfo.pUnit;

					// pøipraví informace o sledované jednotce
					struct SWatchedUnitInfo sUnitInfo;
					sUnitInfo.pUnit = pUnit;
					sUnitInfo.pClient = pClient;

				// nechá jednotku sledovat

					// zjistí, je-li jednotka již sledována
					BOOL bUnitAlreadyWatched = ( m_cWatchedUnitList.Find ( sUnitInfo ) != NULL );
					// pøidá jednotku do seznamu sledovaných jednotek
					m_cWatchedUnitList.Add ( sUnitInfo );

					// zvýší poèet klientù sledujících jednotku
					pUnit->m_dwWatchingCivilizationClientCount++;

					// zjistí, byla-li jednotka již sledována
					if ( !bUnitAlreadyWatched )
					{	// jednotka dosud nebyla sledována
					// odešle start (enemy) full info jednotky

						// zamkne jednotku pro ètení
						VERIFY ( pUnit->ReaderLock () );

						// zjistí, jedná-li se o jednotku civilizace klienta
						if ( pClient->GetCivilizationIndex () == pUnit->GetCivilizationIndex () )
						{	// jedná se o jednotku civilizace klienta
							TRACE_COMMUNICATION2 ( _T("# StartFullInfo ( Client=%x, UnitID=%x )\n"), pClient, pUnit->GetID () );
							// pošle start full info jednotky
							pUnit->SendStartFullInfo ( &sStartFullInfo, pClient );
						}
						else
						{	// nejedná se o jednotku civilizace klienta
							TRACE_COMMUNICATION2 ( _T("# StartEnemyFullInfo ( Client=%x, UnitID=%x )\n"), pClient, pUnit->GetID () );
							// pošle start enemy full info jednotky
							pUnit->SendStartEnemyFullInfo ( &sStartEnemyFullInfo, pClient );
						}

						// odemkne jednotku pro ètení
						pUnit->ReaderUnlock ();
					}
					// bylo odesláno pøípadné start (enemy) full info jednotky
				}
				else
				{	// jedná se o požadavek na ukonèení sledování jednotky
					// zjistí ID jednotky požadavku na sledování jednotky
					DWORD dwID = sRequestInfo.dwID;

					// zjistí umístìní sledované jednotky "dwID" v seznamu sledovaných jednotek
					POSITION posUnit = m_cWatchedUnitList.FindUnitByID ( dwID, pClient );
					ASSERT ( posUnit != NULL );

					// smaže jednotku "dwID" ze seznamu sledovaných jednotek
					struct SWatchedUnitInfo sUnitInfo = 
						m_cWatchedUnitList.RemovePosition ( posUnit );
					ASSERT ( ( sUnitInfo.pUnit->GetID () == dwID ) && 
						( sUnitInfo.pClient == pClient ) );

					// zjistí mazanou jednotku
					CSUnit *pUnit = sUnitInfo.pUnit;

					// sníží poèet klientù sledujících jednotku
					ASSERT ( pUnit->m_dwWatchingCivilizationClientCount > 0 );
					pUnit->m_dwWatchingCivilizationClientCount--;

					// zjistí civilizaci jednotky
					DWORD dwUnitCivilizationIndex = pUnit->GetCivilizationIndex ();

					// sníží poèet odkazù na jednotku
					pUnit->Release ();

					// zjistí, je-li jednotka stále sledována klientem "pClient"
					if ( m_cWatchedUnitList.Find ( sUnitInfo ) == NULL )
					{	// jednotka již není sledována klientem "pClient"
						// zjistí, jedná-li se o jednotku ze stejné civilziace
						if ( pClient->GetCivilizationIndex () == dwUnitCivilizationIndex )
						{	// jedná se o jednotku ze stejné civilizace
							TRACE_COMMUNICATION2 ( _T("# StopFullInfo ( Client=%x, UnitID=%x )\n"), pClient, dwID );
							// odešle klientovi stop full info jednotky
							pClient->StopFullInfo ( dwID );
						}
						else
						{	// jedná se o jednotku z cizí civilizace
							TRACE_COMMUNICATION2 ( _T("# StopEnemyFullInfo ( Client=%x, UnitID=%x )\n"), pClient, dwID );
							// odešle klientovi stop enemy full info jednotky
							pClient->StopEnemyFullInfo ( dwID );
						}
					}
					// odeslal pøípadné stop (enemy) full info
				}
				// požadavek na sledování jednotky byl zpracován
			}
			// všechny požadavky na sledování jednotek byly zpracovány

			// odemkne seznam sledovaných jednotek
			VERIFY ( m_mutexWatchedUnitListLock.Unlock () );
		}

		// odemkne frontu požadavkù na sledování jednotek
		VERIFY ( m_mutexWatchingUnitRequestQueueLock.Unlock () );

	// najde klienta, kterému má posílat MiniMapu

		do
		{
			// informace o zaregistrovaném klientovi
			struct SRegisteredClientInfo *pRegisteredClientInfo;

			// najde klienta, kterému se posílala MiniMapa
			POSITION posMiniMapClient = 
				m_aRegisteredClientList[dwMiniMapClientCivilizationIndex].FindClient ( 
				pMiniMapClient );

			// pokusí se najít v civilizaci dalšího klienta, kterému bude posílat MiniMapu
			if ( posMiniMapClient == NULL )
			{	// nepodaøilo se najít klienta
				// zjistí klienta s indexem index "dwMiniMapClientIndex"
				posMiniMapClient = 
					m_aRegisteredClientList[dwMiniMapClientCivilizationIndex].GetHeadPosition ();
				for ( DWORD dwIndex = 0; dwIndex <= dwMiniMapClientIndex; dwIndex++ )
				{
					// získá dalšího klienta
					if ( !CSRegisteredClientList::GetNext ( posMiniMapClient, 
						pRegisteredClientInfo ) )
					{	// nepodaøilo se získat dalšího klienta
						break;
					}
					// zjistí ukazatel na klienta civilizace
					pMiniMapClient = pRegisteredClientInfo->pClient;
				}
				// zjistí, podaøilo-li se najít klienta s indexem "dwMiniMapClientIndex"
				if ( dwIndex == dwMiniMapClientIndex + 1 )
				{	// podaøilo se najít klienta s indexem "dwMiniMapClientIndex"
					// ukonèí hledání klienta civilizace
					break;
				}
				// nepodaøilo se najít klienta ani podle jeho indexu
			}
			else
			{	// podaøilo se najít klienta civilizace
				// vezme nalezeného klienta civilizace
				VERIFY ( CSRegisteredClientList::GetNext ( posMiniMapClient ) );

				// vezme dalšího klienta civilizace
				if ( CSRegisteredClientList::GetNext ( posMiniMapClient, pRegisteredClientInfo ) )
				{	// podaøilo se vzít dalšího klienta civilizace
					// zjistí ukazatel na klienta civilizace
					pMiniMapClient = pRegisteredClientInfo->pClient;
					// zjistí index klienta civilizace
					posMiniMapClient = 
						m_aRegisteredClientList[dwMiniMapClientCivilizationIndex].GetHeadPosition ();
					for ( dwMiniMapClientIndex = 0; ; dwMiniMapClientIndex++ )
					{
						// získá dalšího klienta
						VERIFY ( CSRegisteredClientList::GetNext ( posMiniMapClient, 
							pRegisteredClientInfo ) );
						// zjistí, jedná-li se o hledaného klienta
						if ( pRegisteredClientInfo->pClient == pMiniMapClient )
						{	// našel hledaného klienta civilizace
							// ukonèí zjišování klienta civilizace
							break;
						}
					}
					ASSERT ( dwMiniMapClientIndex > 0 );
					// ukonèí hledání klienta civilizace
					break;
				}
				// jednalo se o posledního klienta civilizace
			}
			// vyhledá klienta z další civilizace

			// projede seznamy klientù civilizací
			for ( DWORD dwCivilizationIndex = dwMiniMapClientCivilizationIndex + 1; ; 
				dwCivilizationIndex++ )
			{
				// aktualizuje index civilizace
				dwCivilizationIndex = dwCivilizationIndex % CIVILIZATION_COUNT_MAX;

				// zjistí, jedná-li se o poèáteèní civilizaci
				if ( dwCivilizationIndex == dwMiniMapClientCivilizationIndex )
				{	// jedná se o poèáteèní civilizaci
					// zneškodní ukazatel na klienta, kterému se bude posílat MiniMapa
					pMiniMapClient = NULL;
					dwMiniMapClientIndex = 0;
					dwMiniMapClientCivilizationIndex = 0;
					// ukonèí prohledávání civilizací
					break;
				}

				// zjistí, je-li seznam klientù civilizace prázdný
				if ( !m_aRegisteredClientList[dwCivilizationIndex].IsEmpty () )
				{	// seznam klientù civilizace není prázdný
					// aktualizuje ukazatel na klienta, kterému se bude posílat MiniMapa
					posMiniMapClient = 
						m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
					VERIFY ( CSRegisteredClientList::GetNext ( posMiniMapClient, 
						pRegisteredClientInfo ) );
					// zjistí ukazatel na klienta civilizace
					pMiniMapClient = pRegisteredClientInfo->pClient;
					dwMiniMapClientIndex = 0;
					dwMiniMapClientCivilizationIndex = dwCivilizationIndex;
					// ukonèí prohledávání civilizací
					break;
				}
			}
		} while ( 0 );

	// informuje klienty o mrtvých jednotkách

		// zamkne seznam mrtvých jednotek pro rozesílání stop infa klientùm
		VERIFY ( m_mutexDeadUnitClientStopInfoListLock.Lock () );

		// mrtvá jednotka
		CSUnit *pUnit;
		// zpracuje seznam mrtvých jednotek pro rozesílání stop infa klientùm
		while ( m_cDeadUnitClientStopInfoList.RemoveFirst ( pUnit ) )
		{
			// zjistí ID mrtvé jednotky
			DWORD dwUnitID = pUnit->GetID ();

			// projede civilizace
			for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
				dwCivilizationIndex-- > 0; )
			{
				// informace o zaregistrovaném klientovi
				struct SRegisteredClientInfo sClientInfo;

				// zjistí, jedná-li se o civilizaci sledující jednotku
				if ( pUnit->m_cWatchedMapSquareChanged.GetAt ( dwCivilizationIndex ) )
				{	// jedná se o civilizaci sledující jednotku
					// zjistí pozici prvního klienta civilizace
					POSITION posClient = 
						m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
					// projede klienty civilizace
					while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
					{
						TRACE_COMMUNICATION2 ( _T("# StopInfo ( Client=%x, UnitID=%x )\n"), 
							sClientInfo.pClient, dwUnitID );
						// pošle klientovi civilizace stop info jednotky
						sClientInfo.pClient->StopInfo ( dwUnitID );
					}
				}
			}

			// sníží poèet odkazù na jednotku
			pUnit->Release ();
		}

		// odemkne seznam mrtvých jednotek pro rozesílání stop infa klientùm
		VERIFY ( m_mutexDeadUnitClientStopInfoListLock.Unlock () );

	// informuje klienta o zaèátku posílání MiniMapy

		// zjistí, má-li se posílat MiniMapa
		if ( pMiniMapClient != NULL )
		{	// má se posílat MiniMapa
			// upozorní klienta na zaèátek posílání MiniMapy
			pMiniMapClient->StartRenderingUnits ();
		}

	// rozešle klientùm civilizací informace o jednotkách

		// projede všechny bloky jednotek (od konce)
		for ( SUnitBlock *pBlock = m_pLastUnitBlock; pBlock != NULL; 
			pBlock = pBlock->pPrevious )
		{
			// ukazatel na jednotku v bloku
			CSUnit **pBlockUnit = pBlock->pUnits;
			// zamkne blok jednotek pro ètení
			VERIFY ( pBlock->cLock.ReaderLock () );
			// projede všechny jednotky v bloku jednotek
			for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
			{
				// získá ukazatel na jednotku
				CSUnit *pUnit = *pBlockUnit;

				// zjistí, je-li to platná jednotka
				if ( pUnit == NULL )
				{	// není to platná jednotka
					continue;
				}

				// zjistí, má-li se posílat MiniMapa
				if ( pMiniMapClient != NULL )
				{	// má se posílat MiniMapa
					// zjistí, je-li jednotka vidìna civilizací klienta
					if ( pUnit->m_aCivilizationVisibility[dwMiniMapClientCivilizationIndex] > 0 )
					{	// jednotka je vidìna civilizací klienta
						// nakreslí jednotku do MiniMapy klienta civilizace
						pMiniMapClient->RenderUnit ( pUnit->GetPosition ().x, 
							pUnit->GetPosition ().y, pUnit->m_dwCivilizationIndex, 
							pUnit->m_pUnitType->m_aMoveWidth[pUnit->GetMode ()] );
					}
				}

				// získá odkaz na pøíznaky zmìny pozice jednotky pøes hranici sledovaných 
				//		MapSquarù
				CSCivilizationBitArray &cWatchedMapSquareChanged = 
					pUnit->m_cWatchedMapSquareChanged;

				// zjistí, zmìnila-li jednotka pozici pøes hranici sledovaných MapSquarù
				if ( cWatchedMapSquareChanged.IsSet () )
				{	// jednotka zmìnila pozici pøes hranici sledovaných MapSquarù
					// projede civilizace
					for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
						dwCivilizationIndex-- > 0; )
					{
						// zjistí, zmìnila-li jednotka pozici pøes hranici civilizací 
						//		sledovaných MapSquarù
						if ( cWatchedMapSquareChanged.GetAt ( dwCivilizationIndex ) )
						{	// jednotka zmìnila pozici pøes hranici civilizací sledovaných 
							//		MapSquarù
							// vynuluje pøíznak zmìny pozice pøes hranice civilizací 
							//		sledovaných MapSquarù
							cWatchedMapSquareChanged.ClearAt ( dwCivilizationIndex );

							// informace o zaregistrovaném klientovi
							struct SRegisteredClientInfo sClientInfo;

							// získá MapSquare jednotky
							CSMapSquare *pUnitMapSquare = pUnit->GetMapSquare ();
							// zjistí, je-li jednotka vidìt klientem civilizace
							if ( ( pUnitMapSquare != NULL ) && 
								pUnitMapSquare->m_cWatchingCivilizations.GetAt ( dwCivilizationIndex ) && 
								( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 ) )
							{	// jednotka je v civilizací sledovaném MapSquaru a je vidìt
								// pøipraví start info jednotky
								pUnit->GetStartInfo ( &sStartInfo );
								// zjistí pozici prvního klienta civilizace
								POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
								// projede klienty civilizace
								while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
								{
									TRACE_COMMUNICATION2 ( _T("# StartInfo ( Client=%x, UnitID=%x )\n"), sClientInfo.pClient, sStartInfo.dwID );
									// pošle klientovi civilizace start info jednotky
									sClientInfo.pClient->StartInfo ( &sStartInfo );
								}
							}
							else
							{	// jednotka není v civilizací sledovaném MapSquaru
								// pøipraví stop info jednotky
								DWORD dwUnitID = pUnit->GetID ();
								// zjistí pozici prvního klienta civilizace
								POSITION posClient= m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
								// projede klienty civilizace
								while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
								{
									TRACE_COMMUNICATION2 ( _T("# StopInfo ( Client=%x, UnitID=%x )\n"), sClientInfo.pClient, dwUnitID );
									// pošle klientovi civilizace stop info jednotky
									sClientInfo.pClient->StopInfo ( dwUnitID );
								}
							}
							// klientovi bylo posláno start/stop info o jednotce
						}
						// pøípadná zmìna pozici pøes hranici civilizací sledovaných MapSquarù 
						//		byla obsloužena
					}
					// všechny civilizace byly projety
				}
				// pøípadná zmìna pozice jednotky pøes hranici sledovaných MapSquarù byla 
				//		obsloužena

				// získá odkaz na pøíznaky zmìny viditelnosti jednotky civilizacemi
				CSCivilizationBitArray &cCivilizationVisibilityChanged = 
					pUnit->m_cCivilizationVisibilityChanged;

				// zjistí, zmìnila-li se viditelnost jednotky civilizacemi
				if ( cCivilizationVisibilityChanged.IsSet () )
				{	// zmìnila se viditelnost jednotky civilizacemi
					// zjistí, je-li jednotka sledována
					if ( pUnit->m_dwWatchingCivilizationClientCount > 0 )
					{	// jednotka je sledována
						// zamkne seznam sledovaných jednotek
						VERIFY ( m_mutexWatchedUnitListLock.Lock () );

						// projede civilizace
						for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
							dwCivilizationIndex-- > 0; )
						{
							// zjistí, zmìnila-li se viditelnost jednotky civilizací
							if ( cCivilizationVisibilityChanged.GetAt ( dwCivilizationIndex ) )
							{	// zmìnila se viditelnost jednotky civilizací
								// vynuluje pøíznak zmìny viditelnosti jednotky civilizací
								cCivilizationVisibilityChanged.ClearAt ( dwCivilizationIndex );

								// informace o sledované jednotce
								struct SWatchedUnitInfo sUnitInfo;

								// zjistí ID jednotky
								DWORD dwUnitID = pUnit->GetID ();
								// zjistí index civilizace jednotky
								DWORD dwUnitCivilizationIndex = pUnit->GetCivilizationIndex ();
								// získá pozici první sledované jednotky
								POSITION posUnit = m_cWatchedUnitList.GetHeadPosition ();

								// najde všechny klienty sledující jednotku
								while ( m_cWatchedUnitList.FindUnit ( pUnit, posUnit ) )
								{
									// získá informace o sledované jednotce
									VERIFY ( m_cWatchedUnitList.GetNext ( posUnit, sUnitInfo ) );
									ASSERT ( sUnitInfo.pUnit == pUnit );

									// zjistí, jedná-li se o klienta zpracovávané civilizace
									if ( sUnitInfo.pClient->GetCivilizationIndex () != dwCivilizationIndex )
									{	// nejedná se o klienta zpracovávané civilizace
										// pokraèuje v projíždìní klientù sledujících jednotku
										continue;
									}

									// zjistí, je-li jednotka vidìt civilizací
									if ( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 )
									{	// jednotka je civilizací vidìt
										// zjistí, jedná-li se o jednotku vlastní civilizace
										if ( dwUnitCivilizationIndex == dwCivilizationIndex )
										{	// jedná se o jednotku vlastní civilizace
											TRACE_COMMUNICATION2 ( _T("# StartFullInfo ( Client=%x, UnitID=%x )\n"), sUnitInfo.pClient, sStartFullInfo.dwID );
											// nechá poslat start full info jednotky
											pUnit->SendStartFullInfo ( &sStartFullInfo, sUnitInfo.pClient );
										}
										else
										{	// jedná se o jednotku cizí civilizace
											TRACE_COMMUNICATION2 ( _T("# StartEnemyFullInfo ( Client=%x, UnitID=%x )\n"), sUnitInfo.pClient, sStartEnemyFullInfo.dwID );
											// nechá poslat start enemy full info jednotky
											pUnit->SendStartEnemyFullInfo ( &sStartEnemyFullInfo, sUnitInfo.pClient );
										}
									}
									else
									{	// jednotka není civilizací vidìt
										// zjistí, jedná-li se o jednotku vlastní civilizace
										if ( dwUnitCivilizationIndex == dwCivilizationIndex )
										{	// jedná se o jednotku vlastní civilizace
											TRACE_COMMUNICATION2 ( _T("# StopFullInfo ( Client=%x, UnitID=%x )\n"), sUnitInfo.pClient, dwUnitID );
											// pošle stop full info jednotky
											sUnitInfo.pClient->StopFullInfo ( dwUnitID );
										}
										else
										{	// jedná se o jednotku cizí civilizace
											TRACE_COMMUNICATION2 ( _T("# StopEnemyFullInfo ( Client=%x, UnitID=%x )\n"), sUnitInfo.pClient, dwUnitID );
											// pošle stop enemy full info jednotky
											sUnitInfo.pClient->StopEnemyFullInfo ( dwUnitID );
										}
									}
									// klientovi bylo rozesláno info jednotky
								}
								// všem klientùm sledujícím jednotku bylo rozesláno info jednotky

								// získá MapSquare jednotky
								CSMapSquare *pUnitMapSquare = pUnit->GetMapSquare ();
								// zjistí, je-li jednotka v klientem sledovaném MapSquaru
								if ( ( pUnitMapSquare != NULL ) && 
									pUnitMapSquare->m_cWatchingCivilizations.GetAt ( dwCivilizationIndex ) )
								{	// jednotka je v klientem sledovaném MapSquaru
									// informace o zaregistrovaném klientovi
									struct SRegisteredClientInfo sClientInfo;

									// zjistí, je-li jednotka civilizací vidìt
									if ( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 )
									{	// jednotka je civilizací vidìt
										// pøipraví start info jednotky
										pUnit->GetStartInfo ( &sStartInfo );
										// zjistí pozici prvního klienta civilizace
										POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
										// projede klienty civilizace
										while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
										{
											TRACE_COMMUNICATION2 ( _T("# StartInfo ( Client=%x, UnitID=%x )\n"), sClientInfo.pClient, sStartInfo.dwID );
											// pošle klientovi civilizace start info jednotky
											sClientInfo.pClient->StartInfo ( &sStartInfo );
										}
									}
									else
									{	// jednotka civilizací není vidìt
										// zjistí pozici prvního klienta civilizace
										POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
										// projede klienty civilizace
										while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
										{
											TRACE_COMMUNICATION2 ( _T("# StopInfo ( Client=%x, UnitID=%x )\n"), sClientInfo.pClient, dwUnitID );
											// pošle klientovi civilizace start info jednotky
											sClientInfo.pClient->StopInfo ( dwUnitID );
										}
									}
									// klientovi bylo posláno start/stop info o jednotce
								}
								// pøípadná zmìna viditelnosti jednotky civilizací byla obsloužena
							}
							// viditelnost jednotky civilizací se nezmìnila
						}
						// všechny civilizace byly projety

						// odemkne seznam sledovaných jednotek
						VERIFY ( m_mutexWatchedUnitListLock.Unlock () );
					}
					else
					{	// jednotka není sledována
						// projede civilizace
						for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
							dwCivilizationIndex-- > 0; )
						{
							// zjistí, zmìnila-li se viditelnost jednotky civilizací
							if ( cCivilizationVisibilityChanged.GetAt ( dwCivilizationIndex ) )
							{	// zmìnila se viditelnost jednotky civilizací
								// vynuluje pøíznak zmìny viditelnosti jednotky civilizací
								cCivilizationVisibilityChanged.ClearAt ( dwCivilizationIndex );

								// získá MapSquare jednotky
								CSMapSquare *pUnitMapSquare = pUnit->GetMapSquare ();
								// zjistí, je-li jednotka v klientem sledovaném MapSquaru
								if ( ( pUnitMapSquare != NULL ) && 
									pUnitMapSquare->m_cWatchingCivilizations.GetAt ( dwCivilizationIndex ) )
								{	// jednotka je v klientem sledovaném MapSquaru
									// informace o zaregistrovaném klientovi
									struct SRegisteredClientInfo sClientInfo;

									// zjistí, je-li jednotka civilizací vidìt
									if ( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 )
									{	// jednotka je civilizací vidìt
										// pøipraví start info jednotky
										pUnit->GetStartInfo ( &sStartInfo );
										// zjistí pozici prvního klienta civilizace
										POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
										// projede klienty civilizace
										while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
										{
											TRACE_COMMUNICATION2 ( _T("# StartInfo ( Client=%x, UnitID=%x )\n"), sClientInfo.pClient, sStartInfo.dwID );
											// pošle klientovi civilizace start info jednotky
											sClientInfo.pClient->StartInfo ( &sStartInfo );
										}
									}
									else
									{	// jednotka civilizací není vidìt
										// pøipraví stop info jednotky
										DWORD dwUnitID = pUnit->GetID ();
										// zjistí pozici prvního klienta civilizace
										POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
										// projede klienty civilizace
										while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
										{
											TRACE_COMMUNICATION2 ( _T("# StopInfo ( Client=%x, UnitID=%x )\n"), sClientInfo.pClient, dwUnitID );
											// pošle klientovi civilizace start info jednotky
											sClientInfo.pClient->StopInfo ( dwUnitID );
										}
									}
									// klientovi bylo posláno start/stop info o jednotce
								}
								// pøípadná zmìna viditelnosti jednotky civilizací byla obsloužena
							}
							// viditelnost jednotky civilizací se nezmìnila
						}
						// všechny civilizace byly projety
					}
					// sledovaná a nesledovaná jednotka je obsloužena
				}
				// pøípadná zmìna viditelnosti jednotky civilizacemi byla obsloužena

				// zjistí, jedná-li se o sledovanou jednotku
				if ( pUnit->m_dwWatchingCivilizationClientCount > 0 )
				{	// jedná se o sledovanou jednotku
					// zjistí pøíznak zmìny full infa
					BOOL bFullInfoModified = pUnit->IsFullInfoModified ();
					// zjistí, zmìnilo-li se full info jednotky
					if ( bFullInfoModified )
					{	// full info jednotky se zmìnilo
						// smaže pøíznak zmìny full infa jednotky
						pUnit->ClearFullInfoModified ();
					}

					// zamkne seznam sledovaných jednotek
					VERIFY ( m_mutexWatchedUnitListLock.Lock () );

					// informace o sledované jednotce
					struct SWatchedUnitInfo sUnitInfo;

					// získá pozici první sledované jednotky
					POSITION posUnit = m_cWatchedUnitList.GetHeadPosition ();

					// najde všechny klienty sledující jednotku
					while ( m_cWatchedUnitList.FindUnit ( pUnit, posUnit ) )
					{
						// získá informace o sledované jednotce
						VERIFY ( m_cWatchedUnitList.GetNext ( posUnit, sUnitInfo ) );
						ASSERT ( sUnitInfo.pUnit == pUnit );

						// zjistí index civilizace klienta civilizace
						DWORD dwClientCivilizationIndex = sUnitInfo.pClient->GetCivilizationIndex ();

						// zjistí, je-li jednotka vidìna civilizací
						if ( pUnit->m_aCivilizationVisibility[dwClientCivilizationIndex] > 0 )
						{	// jednotka je vidìna civilizací klienta
							// zjistí, jedná-li se o jednotku vlastní civilizace
							if ( pUnit->GetCivilizationIndex () == dwClientCivilizationIndex )
							{	// jedná se o jednotku vlastní civilizace
								TRACE_COMMUNICATION2 ( _T("# FullInfo ( Client=%x, UnitID=%x )\n"), sUnitInfo.pClient, sFullInfo.dwID );
								// nechá poslat full info jednotky
								pUnit->SendFullInfo ( &sFullInfo, sUnitInfo.pClient );
							}
							else
							{	// jedná se o jednotku cizí civilizace
								TRACE_COMMUNICATION2 ( _T("# EnemyFullInfo ( Client=%x, UnitID=%x )\n"), sUnitInfo.pClient, sEnemyFullInfo.dwID );
								// nechá poslat enemy full info jednotky
								pUnit->SendEnemyFullInfo ( &sEnemyFullInfo, sUnitInfo.pClient );
							}
						}
						// info o pøípadnì vidìné jednotce bylo rozesláno
					}
					// všem klientùm sledujícím jednotku bylo rozesláno full info jednotky

					// odemkne seznam sledovaných jednotek
					VERIFY ( m_mutexWatchedUnitListLock.Unlock () );
				}
				else
				{	// jednotka není sledována
					// smaže pøíznak zmìny full infa jednotky
					pUnit->ClearFullInfoModified ();
				}
				// zmìna full infa jednotky byla obsloužena

				// zjistí, zmìnily-li bìžné informace jednotky pro klienta
				if ( pUnit->IsCommonClientInfoModified () )
				{	// bìžné informace jednotky pro klienta se zmìnily
					// získá MapSquare jednotky
					CSMapSquare *pUnitMapSquare = pUnit->GetMapSquare ();
					// ukazatel na bitové pole civilizací sledujích MapSquare, ve kterém 
					//		se jednotka nachází
					CSCivilizationBitArray *pWatchingCivilizations;
					// pøíznak, je-li MapSquare sledován nìkterou civilizací
					BOOL bWatchingMapSquare = ( ( pUnitMapSquare != NULL ) && 
						(pWatchingCivilizations = &pUnitMapSquare->m_cWatchingCivilizations)->IsSet () );

					// zjistí, zmìnilo-li se brief info jednotky
					if ( pUnit->IsBriefInfoModified () )
					{	// zmìnilo se brief info jednotky
						// smaže pøíznak zmìny brief infa jednotky
						pUnit->ClearBriefInfoModified ();
						// zjistí, je-li MapSquare sledován nìkterou civilizací
						if ( bWatchingMapSquare )
						{	// MapSquare je sledován nìkterou civilizací
							// pøipraví brief info jednotky
							pUnit->GetBriefInfo ( &sBriefInfo );
							// projede civilizace
							for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
								dwCivilizationIndex-- > 0; )
							{
								// zjistí, je-li MapSquare sledován civilizací
								if ( pWatchingCivilizations->GetAt ( dwCivilizationIndex ) )
								{	// jednotka je v klientem sledovaném MapSquaru
									// zjistí, je-li jednotka civilizací vidìt
									if ( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 )
									{	// jednotka je civilizací vidìt
										// informace o zaregistrovaném klientovi
										struct SRegisteredClientInfo sClientInfo;

										// zjistí pozici prvního klienta civilizace
										POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
										// projede klienty civilizace
										while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
										{
											TRACE_COMMUNICATION2 ( _T("# BriefInfo ( Client=%x, UnitID=%x )\n"), sClientInfo.pClient, sBriefInfo.dwID );
											// pošle klientovi civilizace brief info jednotky
											sClientInfo.pClient->BriefInfo ( &sBriefInfo );
										}
										// všichni klienti civilizace byli projeti
									}
									// pøípadné informace o viditelné jednotce byly odeslány
								}
								// MapSquare není civilizací sledován
							}
							// všechny civilizace byly projety
						}
						// pøípadné sledování MapSquaru civilizací bylo obslouženo
					}
					// pøípadná zmìna brief infa byla obsloužena

					// zjistí, zmìnilo-li se check point info jednotky
					if ( pUnit->IsCheckPointInfoModified () )
					{	// zmìnilo se check point info jednotky
						// smaže pøíznak zmìny check point infa jednotky
						pUnit->ClearCheckPointInfoModified ();
						// zjistí, je-li MapSquare sledován nìkterou civilizací
						if ( bWatchingMapSquare )
						{	// MapSquare je sledován nìkterou civilizací
							// pøipraví check point info jednotky
							pUnit->GetCheckPointInfo ( &sCheckPointInfo );
							// projede civilizace
							for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
								dwCivilizationIndex-- > 0; )
							{
								// zjistí, je-li MapSquare sledován civilizací
								if ( pWatchingCivilizations->GetAt ( dwCivilizationIndex ) )
								{	// jednotka je v klientem sledovaném MapSquaru
									// zjistí, je-li jednotka civilizací vidìt
									if ( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 )
									{	// jednotka je civilizací vidìt
										// informace o zaregistrovaném klientovi
										struct SRegisteredClientInfo sClientInfo;

										// zjistí pozici prvního klienta civilizace
										POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
										// projede klienty civilizace
										while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
										{
											TRACE_COMMUNICATION2 ( _T("# CheckPointInfo ( Client=%x, UnitID=%x )\n"), sClientInfo.pClient, sCheckPointInfo.dwID );
											// pošle klientovi civilizace check point info jednotky
											sClientInfo.pClient->CheckPointInfo ( &sCheckPointInfo );
										}
										// všichni klienti civilizace byli projeti
									}
									// pøípadné informace o viditelné jednotce byly odeslány
								}
								// MapSquare není civilizací sledován
							}
							// všechny civilizace byly projety
						}
						// pøípadné sledování MapSquaru civilizací bylo obslouženo
					}
					// pøípadná zmìna check point infa byla obsloužena
				}
				// klientovi byly odeslány všechny informace o jednotce
			}
			// byly projety všechny jednotky v bloku jednotek

			// odemkne zamèený blok jednotek pro ètení
			pBlock->cLock.ReaderUnlock ();
		}
		// byly projety všechny bloky jednotek

	// informuje klienta o ukonèení posílání MiniMapy

		// zjistí, má-li se posílat MiniMapa
		if ( pMiniMapClient != NULL )
		{	// má se posílat MiniMapa
			// upozorní klienta na zaèátek posílání MiniMapy
			pMiniMapClient->StopRenderingUnits ();
		}

	// ukonèí rozesílání informací o jednotkách

		// upozorní klienty civilizací na konec rozesílání informací o jednotkách
		{
			// projede civilizace
			for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; )
			{
				// informace o zaregistrovaném klientovi
				struct SRegisteredClientInfo sClientInfo;

				// zjistí pozici prvního klienta civilizace
				POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
				// projede klienty civilizace
				while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
				{
					TRACE_COMMUNICATION1 ( _T("#		StopSendingInfo ( Client=%x )\n"), sClientInfo.pClient );
					// upozorní klienta na konec rozesílání informací o jednotkách
					sClientInfo.pClient->StopSendingInfo ();
				}
			}
		}

		// odemkne seznamy zaregistrovaných klientù civilizací pro ètení
		m_cRegisteredClientListLock.ReaderUnlock ();
	}
	// skonèil hlavní cyklus rozesílání informací klientùm civilizací
}

//////////////////////////////////////////////////////////////////////
// Operace s uživateli
//////////////////////////////////////////////////////////////////////

// zjistí poèet uživatelù (lze volat jen z aplikace serveru)
DWORD CSMap::GetUserCount () 
{
	// vrátí poèet uživatelù
	return m_aUserDescriptions.GetSize ();
}

// vyplní informace o uživateli èíslo "dwUserIndex" a vrátí identifikaci uživatele 
//		"dwUserID", která je platná do smazání uživatele (lze volat jen z aplikace 
//		serveru)
void CSMap::GetUser ( DWORD dwUserIndex, CString &strName, CString &strPassword, 
	DWORD &dwCivilizationIndex, DWORD &dwUserID ) 
{
	ASSERT ( dwUserIndex < (DWORD)m_aUserDescriptions.GetSize () );

	// zjistí popis uživatele
	struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( 
		(int)dwUserIndex );

	// vyplní ID uživatele
	dwUserID = (DWORD)pUserDescription;

	// vyplní informace o uživateli
	strName = pUserDescription->strName;
	strPassword = pUserDescription->strPassword;
	dwCivilizationIndex = pUserDescription->dwCivilizationIndex;
}

// vyplní informace o uživateli s identifikací "dwUserID" (lze volat jen z aplikace 
//		serveru)
void CSMap::GetUser ( DWORD dwUserID, CString &strName, CString &strPassword, 
	DWORD &dwCivilizationIndex ) 
{
	// najde uživatele s identifikací "dwUserID"
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjistí, neshoduje-li se identifikace uživatele
		if ( (DWORD)m_aUserDescriptions.GetAt ( nIndex ) == dwUserID )
		{	// identifikace uživatele se shoduje
			// zjistí popis uživatele
			struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );

			// vyplní informace o uživateli
			strName = pUserDescription->strName;
			strPassword = pUserDescription->strPassword;
			dwCivilizationIndex = pUserDescription->dwCivilizationIndex;

			// ukonèí získání informací o uživateli
			return;
		}
	}
	// uživatel nebyl nalezen
	ASSERT ( FALSE );
}

// vrátí TRUE pokud je uživatel povolen
BOOL CSMap::IsUserEnabled ( DWORD dwUserID )
{
	// najde uživatele s identifikací "dwUserID"
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjistí, neshoduje-li se identifikace uživatele
		if ( (DWORD)m_aUserDescriptions.GetAt ( nIndex ) == dwUserID )
		{	// identifikace uživatele se shoduje
			// zjistí popis uživatele
			struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );

			// zjistí zda je uživatel povolen
			return ( pUserDescription->bEnabled );
		}
	}
	// uživatel nebyl nalezen
	ASSERT ( FALSE );

	return FALSE;
}

// vrátí TRUE pokud je uživatel nalogován
BOOL CSMap::IsUserLoggedOn ( DWORD dwUserID )
{
	// najde uživatele s identifikací "dwUserID"
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjistí, neshoduje-li se identifikace uživatele
		if ( (DWORD)m_aUserDescriptions.GetAt ( nIndex ) == dwUserID )
		{	// identifikace uživatele se shoduje
			// zjistí popis uživatele
			struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );

			// zjistí zda je uživatel nalogován
			return ( pUserDescription->bLoggedOn );
		}
	}
	// uživatel nebyl nalezen
	ASSERT ( FALSE );

	return FALSE;
}

// povolí uživatele
void CSMap::EnableUser ( DWORD dwUserID )
{
	// najde uživatele s identifikací "dwUserID"
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjistí, neshoduje-li se identifikace uživatele
		if ( (DWORD)m_aUserDescriptions.GetAt ( nIndex ) == dwUserID )
		{	// identifikace uživatele se shoduje
			// zjistí popis uživatele
			struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );

			// povolí uživatele
			pUserDescription->bEnabled = TRUE;

			return;
		}
	}
	// uživatel nebyl nalezen
	ASSERT ( FALSE );
}

// zakázání uživatele
void CSMap::DisableUser ( DWORD dwUserID )
{
	// najde uživatele s identifikací "dwUserID"
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjistí, neshoduje-li se identifikace uživatele
		if ( (DWORD)m_aUserDescriptions.GetAt ( nIndex ) == dwUserID )
		{	// identifikace uživatele se shoduje
			// zjistí popis uživatele
			struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );

			// zakáže uživatele
			pUserDescription->bEnabled = FALSE;

			// odpojí uživatele pokud je pøipojen
			CSCivilization *pCiv = GetCivilization( pUserDescription->dwCivilizationIndex );
			pCiv->m_pZCivilization->DisconnectUser( pUserDescription->strName );

			return;
		}
	}
	// uživatel nebyl nalezen
	ASSERT ( FALSE );
}

// zmìní informace o uživateli s identifikací "dwUserID" (FALSE=duplicitní jméno) (lze 
//		volat jen z aplikace serveru)
BOOL CSMap::EditUser ( DWORD dwUserID, LPCTSTR lpcszName, LPCTSTR lpcszPassword, 
	DWORD dwCivilizationIndex ) 
{
	ASSERT ( strlen ( lpcszName ) > 0 );
	ASSERT ( dwCivilizationIndex < m_dwCivilizationCount );

	// zjistí popis uživatele
	struct SUserDescription *pUserDescription = (SUserDescription *)dwUserID;

	// zjistí, bylo-li zmìnìno jméno
	if ( pUserDescription->strName != lpcszName )
	{	// bylo zmìnìno jméno uživatele
		// zjistí, je-li jméno uživatele jednoznaèné
		for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
		{
			// zjistí, neshoduje-li se jméno uživatele
			if ( m_aUserDescriptions.GetAt ( nIndex )->strName == lpcszName )
			{	// jméno uživatele se shoduje
				// vrátí pøíznak duplicitního uživatele
				return FALSE;
			}
		}
	}

	// zamkne pole popisù uživatelù
	VERIFY ( m_mutexUserDescriptionsWriterLock.Lock () );

	// aktualizuje informace o uživateli
	pUserDescription->strName = lpcszName;
	pUserDescription->strPassword = lpcszPassword;
	pUserDescription->dwCivilizationIndex = dwCivilizationIndex;

	// odemkne pole popisù uživatelù
	VERIFY ( m_mutexUserDescriptionsWriterLock.Unlock () );

	// vrátí pøíznak úspìchu
	return TRUE;
}

// pøidá civilizaci "dwCivilizationIndex" uživatele se jménem "lpcszName" a heslem 
//		"lpcszPassword", pøidá-li uživatele, vyplní jeho identifikaci "dwUserID", která 
//		je platná do smazání uživatele (FALSE=duplicitní uživatel) (lze volat jen 
//		z aplikace serveru)
BOOL CSMap::AddUser ( LPCTSTR lpcszName, LPCTSTR lpcszPassword, 
	DWORD dwCivilizationIndex, DWORD &dwUserID ) 
{
	ASSERT ( strlen ( lpcszName ) > 0 );
	ASSERT ( dwCivilizationIndex < m_dwCivilizationCount );

	// zjistí, je-li jméno uživatele jednoznaèné
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjistí, neshoduje-li se jméno uživatele
		if ( m_aUserDescriptions.GetAt ( nIndex )->strName == lpcszName )
		{	// jméno uživatele se shoduje
			// vrátí pøíznak duplicitního uživatele
			return FALSE;
		}
	}

	// vytvoøí nový popis uživatele
	struct SUserDescription *pUserDescription = new SUserDescription;

	// vyplní identifikaci uživatele
	dwUserID = (DWORD)pUserDescription;

	// inicializuje popis uživatele
	pUserDescription->strName = lpcszName;
	pUserDescription->strPassword = lpcszPassword;
	pUserDescription->dwCivilizationIndex = dwCivilizationIndex;
	pUserDescription->bEnabled = TRUE;
	pUserDescription->bLoggedOn = FALSE;

	// zamkne pole popisù uživatelù
	VERIFY ( m_mutexUserDescriptionsWriterLock.Lock () );

	// pøidá popis uživatele do tabulky uživatelù
	m_aUserDescriptions.Add ( pUserDescription );

	// odemkne pole popisù uživatelù
	VERIFY ( m_mutexUserDescriptionsWriterLock.Unlock () );

	// vrátí pøíznak úspìchu
	return TRUE;
}

// smaže uživatele s identifikací "dwUserID" (identifikace je dále neplatná) (lze 
//		volat jen z aplikace serveru)
void CSMap::DeleteUser ( DWORD dwUserID ) 
{
	// najde uživatele s identifikací "dwUserID"
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjistí, neshoduje-li se identifikace uživatele
		if ( (DWORD)m_aUserDescriptions.GetAt ( nIndex ) == dwUserID )
		{	// identifikace uživatele se shoduje
			// zamkne pole popisù uživatelù
			VERIFY ( m_mutexUserDescriptionsWriterLock.Lock () );

			// smaže uživatele z pole popisù uživatelù
			m_aUserDescriptions.RemoveAt ( nIndex );
			// znièí popis uživatele
			delete (SUserDescription *)dwUserID;

			// odemkne pole popisù uživatelù
			VERIFY ( m_mutexUserDescriptionsWriterLock.Unlock () );

			// ukonèí mazání uživatele
			return;
		}
	}
	// uživatel nebyl nalezen
	ASSERT ( FALSE );
}

// najde uživatele "lpcszName" s heslem "lpcszPassword", vyplní civilizaci uživatele 
//		"pZCivilization" a ID uživatele "dwUserID" (je-li uživatel nalezen), vrací: UE_OK, 
//		UE_BadName, UE_BadPassword, UE_UserDisabled
CSMap::EUserError CSMap::FindUser ( LPCTSTR lpcszName, LPCTSTR lpcszPassword, 
		CZCivilization *&pZCivilization, DWORD &dwUserID ) 
{
	// zamkne pole popisù uživatelù
	VERIFY ( m_mutexUserDescriptionsWriterLock.Lock () );

	// návratový chybový kód uživatele
	EUserError eError = UE_BadName;

	// zneškodní ukazatel na civilizaci civilizace
	pZCivilization = NULL;

	// projede pole uživatelù
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjistí popis uživatele
		struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );

		// zjistí, neshoduje-li se jméno uživatele
		if ( pUserDescription->strName == lpcszName )
		{	// jméno uživatele se shoduje
			// zkontroluje zda uživatel není zakázán
			if ( !pUserDescription->bEnabled )
			{	// uživatel je zakázán
				eError = UE_UserDisabled;
			}
			else
			{	// uživatel není zakázán
				// zjistí, shoduje-li se heslo
				eError = ( pUserDescription->strPassword == lpcszPassword ) ? UE_OK : 
					UE_BadPassword;
			}
			// vyplní civilizaci uživatele na civilizaci
			ASSERT ( pUserDescription->dwCivilizationIndex < m_dwCivilizationCount );
			pZCivilization = m_pCivilizations[pUserDescription->dwCivilizationIndex].m_pZCivilization;
			ASSERT ( pZCivilization != NULL );
			// vyplní ID uživatele
			dwUserID = (DWORD)pUserDescription;
			// ukonèí prohledávání pole uživatelù
			break;
		}
	}

	// odemkne pole popisù uživatelù
	VERIFY ( m_mutexUserDescriptionsWriterLock.Unlock () );

	// vrátí pøíznak chybného jména uživatele
	return eError;
}

// zaène logování uživatele (FALSE=hra nebìží, uživatel se nemùže nalogovat)
BOOL CSMap::StartUserLogin () 
{
	// zamkne zámek pøíznaku povolení logování uživatelù
	VERIFY ( m_cUserLoginAllowedLock.ReaderLock () );

	// zjistí mùže-li se uživatel nalogovat
	if ( !m_bUserLoginAllowed )
	{	// uživatel se nemùže nalogovat
		// odemkne zámek pøíznaku povolení logování uživatelù
		m_cUserLoginAllowedLock.ReaderUnlock ();
		// vrátí pøíznak neúspìšného zaèátku logování
		return FALSE;
	}
	else
	{	// uživatel se mùže nalogovat
		// vrátí pøíznak zaèátku logování uživatele
		return TRUE;
	}
}

// ukonèí logování uživatele
void CSMap::FinishUserLogin () 
{
	// odemkne zámek pøíznaku povolení logování uživatelù
	m_cUserLoginAllowedLock.ReaderUnlock ();
}

// uživatel je nalogován (voláno po nalogování, pøed voláním FinishUserLogin)
void CSMap::UserLoggedOn ( LPCTSTR lpcszName ) 
{
	DWORD dwUserID = 0;

	// Nastavit pøíznak o nalogování
	{
		// najde uživatele s jméne "lpcszName"
		for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
		{
			struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );
			// zjistí, neshoduje-li se identifikace uživatele
			if ( pUserDescription->strName.Compare(lpcszName) == 0 )
			{
				// nastaví pøíznak
				pUserDescription->bLoggedOn = TRUE;

				dwUserID = (DWORD)pUserDescription;

				break;
			}
		}
	}

	if ( dwUserID == 0 ) return;

	// Zavolá notifikace
	{
		VERIFY ( m_cMapObserverListLock.Lock ( ) );

		CSMapObserver *pObserver = m_pFirstMapObserver;

		while ( pObserver != NULL )
		{
			pObserver->OnUserLoggedOn ( dwUserID );
			pObserver = pObserver->m_pNextObserver;
		}

		VERIFY ( m_cMapObserverListLock.Unlock ( ) );
	}
}

// uživatel je odlogován (voláno po odlogování)
void CSMap::UserLoggedOff ( LPCTSTR lpcszName ) 
{
	DWORD dwUserID;

	// Nastavit pøíznak o nalogování
	{
		// najde uživatele s jméne "lpcszName"
		for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
		{
			struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );
			// zjistí, neshoduje-li se identifikace uživatele
			if ( pUserDescription->strName.Compare(lpcszName) == 0 )
			{
				// nastaví pøíznak
				pUserDescription->bLoggedOn = FALSE;

				dwUserID = (DWORD)pUserDescription;

				break;
			}
		}
	}

	if ( dwUserID == 0 ) return;

	// Zavolá notifikace
	{
		VERIFY ( m_cMapObserverListLock.Lock ( ) );

		CSMapObserver *pObserver = m_pFirstMapObserver;

		while ( pObserver != NULL )
		{
			pObserver->OnUserLoggedOff ( dwUserID );
			pObserver = pObserver->m_pNextObserver;
		}

		VERIFY ( m_cMapObserverListLock.Unlock ( ) );
	}
}

// Pøidá observer do spojáku observerù
void CSMap::RegisterMapObserver ( CSMap::CSMapObserver *pMapObserver )
{
	ASSERT ( pMapObserver != NULL );
	ASSERT ( pMapObserver->m_pNextObserver == NULL );

	VERIFY ( m_cMapObserverListLock.Lock ( ) );

	// Pøidá observer do spojáku
	pMapObserver->m_pNextObserver = m_pFirstMapObserver;
	m_pFirstMapObserver = pMapObserver;

	VERIFY ( m_cMapObserverListLock.Unlock ( ) );
}

// Odebere observer ze spojáku observerù
void CSMap::UnregisterMapObserver ( CSMap::CSMapObserver *pMapObserver )
{
	ASSERT ( pMapObserver != NULL );

	VERIFY ( m_cMapObserverListLock.Lock ( ) );

	// Najde ho ve spojáku
	CSMapObserver *pObserver, **ppPointer;
	ppPointer = &m_pFirstMapObserver;
	pObserver = *ppPointer;
	while ( pObserver != NULL )
	{
		if ( pObserver == pMapObserver )
		{	// Nalezen
			// Odebrat ze spojáku
			*ppPointer = pObserver->m_pNextObserver;
			pObserver->m_pNextObserver = NULL;

			break;
		}

		ppPointer = &pObserver->m_pNextObserver;
		pObserver = pObserver->m_pNextObserver;
	}

	VERIFY ( m_cMapObserverListLock.Unlock ( ) );
}

//////////////////////////////////////////////////////////////////////
// Debuggovací informace
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// zkontroluje správná data objektu (TRUE=OK)
BOOL CSMap::CheckValidData () 
{
	ASSERT ( ( m_sizeMapMapSquare.cx > 0 ) && ( m_sizeMapMapSquare.cy > 0 ) );
	ASSERT ( ( m_sizeMapMapCell.cx > 0 ) && ( m_sizeMapMapCell.cy > 0 ) );
	ASSERT ( ( m_pointCornerMapSquare.x > 0 ) && ( m_pointCornerMapSquare.y > 0 ) );
	ASSERT ( ( m_pointCornerMapCell.x > 0 ) && ( m_pointCornerMapCell.y > 0 ) );
	ASSERT ( m_pMapSquares != NULL );
	ASSERT ( !m_strMapArchiveName.IsEmpty () );

	ASSERT ( m_dwCivilizationCount > 0 );
	ASSERT ( m_pCivilizations != NULL );

	ASSERT ( m_dwTimeSlice > 0 );

	ASSERT ( m_pMainLoopThread != NULL );
	ASSERT ( m_pClientInfoSenderLoopThread != NULL );
	ASSERT ( m_pFindPathLoopThread != NULL );

	// zkontroluje, nebìží-li provádìní DPC
	ASSERT ( m_pPreparingDPC == NULL );

	ASSERT ( m_dwMainLoopThreadID != DWORD_MAX );

	return TRUE;
}

// zkontroluje neškodná data objektu (TRUE=OK)
BOOL CSMap::CheckEmptyData () 
{
	ASSERT ( !m_bInitialized );

	ASSERT ( m_sizeMapMapSquare == CSizeDW ( 0, 0 ) );
	ASSERT ( m_sizeMapMapCell == CSizeDW ( 0, 0 ) );
	ASSERT ( m_pointCornerMapSquare == CPointDW ( 0, 0 ) );
	ASSERT ( m_pointCornerMapCell == CPointDW ( 0, 0 ) );
	ASSERT ( m_pMapSquares == NULL );
	ASSERT ( m_strMapArchiveName.IsEmpty () );
	ASSERT ( m_dwMapFormatVersion == 0 );
	ASSERT ( m_dwMapCompatibleFormatVersion == DWORD_MAX );
	ASSERT ( m_strMapName.IsEmpty () );
	ASSERT ( m_strMapDescription.IsEmpty () );
	ASSERT ( m_dwMapVersion == DWORD_MAX );

	ASSERT ( m_dwCivilizationCount == 0 );
	ASSERT ( m_pCivilizations == NULL );

	ASSERT ( m_pFirstUnitBlock == NULL );
	ASSERT ( m_pLastUnitBlock == NULL );
	ASSERT ( m_pFirstFreeUnit == NULL );

	ASSERT ( m_dwTimeSlice == 0 );

	ASSERT ( m_pMainLoopThread == NULL );
	ASSERT ( m_pClientInfoSenderLoopThread == NULL );
	ASSERT ( m_pFindPathLoopThread == NULL );

	for ( DWORD dwIndex = CIVILIZATION_COUNT_MAX; dwIndex-- > 0; )
	{
		ASSERT ( m_aRegisteredClientList[dwIndex].IsEmpty () );
		ASSERT ( m_aDPCQueue[dwIndex].IsEmpty () );
	}
	ASSERT ( m_cWatchedUnitList.IsEmpty () );
	ASSERT ( m_cWatchingUnitRequestQueue.IsEmpty () );
	ASSERT ( m_cWatchingMapSquareRequestQueue.IsEmpty () );
	ASSERT ( m_pPreparingDPC == NULL );

	ASSERT ( m_aUserDescriptions.GetSize () == 0 );

	ASSERT ( m_dwMainLoopThreadID == DWORD_MAX );

	return TRUE;
}

#endif //_DEBUG
