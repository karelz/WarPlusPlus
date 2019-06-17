/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da mapy na serveru hry
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
// zkontroluje ���ku a v��ku MapSquaru
#if ( MAPSQUARE_WIDTH != MAPSQUARE_HEIGHT ) || ( MAPSQUARE_WIDTH != MAP_SQUARE_SIZE )
#error Incompatible MapSquare dimensions! (must be: MAPSQUARE_WIDTH = MAPSQUARE_HEIGHT = MAP_SQUARE_SIZE)
#endif

//////////////////////////////////////////////////////////////////////
// debuggovac� a trasovac� makra
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

// tolerance polom�ru viditelnosti jednotky
#define UNIT_VIEW_RADIUS_TOLERANCE(dwViewRadiusSquare) ( 2 * dwViewRadiusSquare + 9 )

// po�et TimeSlic� mezi maz�n�m mrtv�ch jednotek
#define IsDeletingDeadUnitsTimeSlice(dwTimeSlice)		( dwTimeSlice & 0x07 )

//////////////////////////////////////////////////////////////////////
// Glob�ln� prom�nn�
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
	// zne�kodn� data mapy
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

	// zak�e logov�n� u�ivatel�
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
// Inicializace a zni�en� dat objektu
//////////////////////////////////////////////////////////////////////

// inicializuje data mapy z archivu mapy "cMapArchive"
void CSMap::Create ( CDataArchive cMapArchive ) 
{
	ASSERT ( CheckEmptyData () );
	ASSERT ( !cMapArchive.GetArchivePath ().IsEmpty () );

	// vytvo�it statick� v�ci
	CSOneInstance::Create ();

	// aktualizuje jm�no archivu mapy
	m_strMapArchiveName = cMapArchive.GetArchivePath ();

	// pole archiv� knihoven typ� jednotek
	CDataArchivePtrArray cUnitTypeLibraryArchiveArray;

	try
	{
	// na�te data mapy ze souboru mapy

		// otev�e soubor mapy
		CArchiveFile cMapFile = cMapArchive.CreateFile ( MAP_FILE_NAME, CFile::modeRead | 
			CFile::shareDenyWrite );

		// po�et knihoven Mapex�
		DWORD dwMapexLibraryCount;
		// po�et knihoven jednotek
		DWORD dwUnitTypeLibraryCount;
		// po�et jednotek na map�
		DWORD dwUnitCount;
		// po�et typ� jednotek v knihovn�ch typ� jednotek
		DWORD dwUnitTypeCount = 0;
		// po�et ScriptSet�
		DWORD dwScriptSetCount;
		// po�et graf� pro hled�n� cest
		DWORD dwFindPathGraphCount;

	// na�te hlavi�ku verze souboru
		{
			SFileVersionHeader sFileVersionHeader;
			LOAD_ASSERT ( cMapFile.Read ( &sFileVersionHeader, sizeof ( sFileVersionHeader ) ) == sizeof ( sFileVersionHeader ) );

			// identifik�tor souboru mapy
			BYTE aMapFileID[16] = MAP_FILE_IDENTIFIER;
			ASSERT ( sizeof ( aMapFileID ) == sizeof ( sFileVersionHeader.m_aMapFileID ) );

			// zkontroluje identifik�tor souboru mapy
			for ( int nIndex = 16; nIndex-- > 0; )
			{
				LOAD_ASSERT ( aMapFileID[nIndex] == sFileVersionHeader.m_aMapFileID[nIndex] );
			}

			ASSERT ( CURRENT_MAP_FILE_VERSION >= COMPATIBLE_MAP_FILE_VERSION );
			// na�te ��sla verz� form�tu mapy
			m_dwMapFormatVersion = sFileVersionHeader.m_dwFormatVersion;
			m_dwMapCompatibleFormatVersion = sFileVersionHeader.m_dwCompatibleFormatVersion;
			LOAD_ASSERT ( m_dwMapFormatVersion >= m_dwMapCompatibleFormatVersion );

			// zjist�, jedn�-li se o spr�vnou verzi form�tu mapy
			if ( m_dwMapFormatVersion != CURRENT_MAP_FILE_VERSION )
			{	// nejedn� se o spr�vnou verzi form�tu mapy
				// zjist�, jedn�-li se o starou verzi form�tu mapy
				if ( m_dwMapFormatVersion < CURRENT_MAP_FILE_VERSION )
				{	// jedn� se o starou verzi form�tu mapy
					// zkontroluje kompatabilitu verze form�tu mapy
					LOAD_ASSERT ( m_dwMapFormatVersion >= COMPATIBLE_MAP_FILE_VERSION );
				}
				else
				{	// jedn� se o mlad�� verzi form�tu mapy
					// zkontroluje kompatabilitu verze form�tu mapy
					LOAD_ASSERT ( m_dwMapCompatibleFormatVersion <= CURRENT_MAP_FILE_VERSION );
				}
			}
		}

	// na�te hlavi�ku mapy
		{
			SMapHeader sMapHeader;
			LOAD_ASSERT ( cMapFile.Read ( &sMapHeader, sizeof ( sMapHeader ) ) == sizeof ( sMapHeader ) );

		// zpracuje hlavi�ku mapy

			// na�te velikost mapy
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

			// vytvo�� pole MapSquar�
			m_pMapSquares = new CSMapSquare[m_sizeMapMapSquare.cx * m_sizeMapMapSquare.cy];

			// zkontroluje d�lku jm�na mapy
			m_strMapName = sMapHeader.m_pName;
			LOAD_ASSERT ( m_strMapName.GetLength () < sizeof ( sMapHeader.m_pName ) );
			// zkontroluje d�lku popisu mapy
			m_strMapDescription = sMapHeader.m_pDescription;
			LOAD_ASSERT ( m_strMapDescription.GetLength () < sizeof ( sMapHeader.m_pDescription ) );

			// zkontroluje po�et knihoven Mapex�
			dwMapexLibraryCount = sMapHeader.m_dwMapexLibraryCount;
			LOAD_ASSERT ( dwMapexLibraryCount > 0 );

			// zkontroluje po�et civilizac�
			m_dwCivilizationCount = sMapHeader.m_dwCivilizationsCount;
			LOAD_ASSERT ( m_dwCivilizationCount > 0 );
			LOAD_ASSERT ( m_dwCivilizationCount <= CIVILIZATION_COUNT_MAX );

			// vytvo�� pole civilizac�
			m_pCivilizations = new CSCivilization[m_dwCivilizationCount];

			// zkontroluje po�et knihoven typ� jednotek
			dwUnitTypeLibraryCount = sMapHeader.m_dwUnitTypeLibraryCount;
			LOAD_ASSERT ( dwUnitTypeLibraryCount > 0 );

			// zjist� po�et jednotek na map�
			dwUnitCount = sMapHeader.m_dwUnitCount;

			// zjist� verzi mapy
			m_dwMapVersion = sMapHeader.m_dwMapVersion;

			// zjist� po�et ScriptSet�
			dwScriptSetCount = sMapHeader.m_dwScriptSetCount;
			LOAD_ASSERT ( dwScriptSetCount > 0 );

			// zjist� po�et graf� pro hled�n� cest
			dwFindPathGraphCount = sMapHeader.m_dwFindPathGraphCount;
		}

		// tabulka p�ekladu ID na jm�no ScriptSetu
		CMap<DWORD, DWORD, CString, CString &> cScriptSetTable;

	// nahraje hlavi�ky ScriptSet�
		{
			SScriptSetHeader sScriptSetHeader;

			for ( DWORD dwIndex = dwScriptSetCount; dwIndex-- > 0; )
			{
				// pokus� se na��st hlavi�ku ScriptSetu
				LOAD_ASSERT ( cMapFile.Read ( &sScriptSetHeader, sizeof ( sScriptSetHeader ) ) == sizeof ( sScriptSetHeader ) );

				// zkontroluje d�lku jm�na ScriptSetu
				int nScriptSetNameLength = strlen ( sScriptSetHeader.m_pFileName );
				LOAD_ASSERT ( ( nScriptSetNameLength > 0 ) && ( nScriptSetNameLength < 
					sizeof ( sScriptSetHeader.m_pFileName ) ) );

				// jm�no nalezen�ho ScriptSetu
				CString strFoundScriptSetName;
				// ID nalezen�ho ScriptSetu
				DWORD dwFoundScriptSetID;
				// pozice ScriptSetu v tabulce ScriptSet�
				POSITION posScriptSet = cScriptSetTable.GetStartPosition ();
				// jm�no ScriptSetu
				CString strScriptSetName ( sScriptSetHeader.m_pFileName );

				// zkontroluje unik�tnost ID a jm�na ScriptSetu
				while ( posScriptSet != NULL )
				{
					// zjist� informace o dal��m ScriptSetu v tabulce
					cScriptSetTable.GetNextAssoc ( posScriptSet, dwFoundScriptSetID, 
						strFoundScriptSetName );

					// zkontroluje unik�tnost ID a jm�na ScriptSetu
					LOAD_ASSERT ( ( strFoundScriptSetName.CompareNoCase ( 
						strScriptSetName ) != 0 ) && sScriptSetHeader.m_dwID != 
						dwFoundScriptSetID );
				}

				// p�id� z�znam o ScriptSetu do tabulky ScriptSet�
				cScriptSetTable.SetAt ( sScriptSetHeader.m_dwID, strScriptSetName );

				// nech� nahr�t ScriptSet
				g_cSFileManager.LoadScriptSet ( cMapArchive, strScriptSetName );
			}
		}

	// p�esko�� odkazy na knihovny Mapex�
		{
			// zjist� velikost odkaz� na knihovny Mapex�
			DWORD dwMapexLibraryNodeSize = dwMapexLibraryCount * 
				sizeof ( SMapexLibraryNode );

			// p�esko�� odkazy na knihovny Mapex�
			LOAD_ASSERT ( cMapFile.GetPosition () + dwMapexLibraryNodeSize <= 
				cMapFile.GetLength () );
			(void)cMapFile.Seek ( dwMapexLibraryNodeSize, CFile::current );
		}

		// tabulka p�ekladu ID na index civilizace
		CMap<DWORD, DWORD, DWORD, DWORD> cCivilizationTable;

	// na�te hlavi�ky civilizac�
		{
			// zjist� ukazatel na prvn� civilizaci z mapy
			CSCivilization *pCivilization = m_pCivilizations;

			// projede v�echny civilizace z mapy
			for ( DWORD dwCivilizationIndex = 0; dwCivilizationIndex < m_dwCivilizationCount; 
				dwCivilizationIndex++, pCivilization++ )
			{
				// na�te dal�� civilizaci
				pCivilization->Create ( cMapFile, (BYTE)dwCivilizationIndex, 
					cScriptSetTable );

				// zjist�, jedn�-li se o syst�movou civilizaci
				if ( dwCivilizationIndex == 0 )
				{	// jedn� se o syst�movou civilizaci
					// zkontroluje na�ten� syst�mov� civilizace
					LOAD_ASSERT ( pCivilization->GetCivilizationID () == 0 );
				}

				// zkontroluje unik�tnost jm�na a ID civilizace
				for ( DWORD dwIndex = 0; dwIndex < dwCivilizationIndex; dwIndex++ )
				{
					// zkontroluje unik�tnost jm�na civilizace
					LOAD_ASSERT ( pCivilization->GetCivilizationName () != m_pCivilizations[dwIndex].GetCivilizationName () );
					// zkontroluje unik�tnost ID civilizace
					LOAD_ASSERT ( pCivilization->GetCivilizationID () != m_pCivilizations[dwIndex].GetCivilizationID () );
				}

			// zaznamen� p�evod ID na index civilizace

				// nalezen� index civilizace
				DWORD dwFoundCivilizationIndex;
				// zkontroluje unik�tnost ID civilizace
				LOAD_ASSERT ( !cCivilizationTable.Lookup ( 
					pCivilization->GetCivilizationID (), dwFoundCivilizationIndex ) );
				// p�id� z�znam do tabulky p�ekladu ID na index civilizace
				cCivilizationTable.SetAt ( pCivilization->GetCivilizationID (), 
					dwCivilizationIndex );
			}
		}

	// na�te odkazy na knihovny typ� jednotek
		{
			SUnitTypeLibraryNode sUnitTypeLibraryNode;

			// inicializuje po�et typ� jednotek v knihovn�ch typ� jednotek
			dwUnitTypeCount = 0;

			for ( DWORD dwUnitTypeLibraryIndex = dwUnitTypeLibraryCount; 
				dwUnitTypeLibraryIndex-- > 0; )
			{
				// na�te odkaz na knihovnu typ� jednotek
				LOAD_ASSERT ( cMapFile.Read ( &sUnitTypeLibraryNode, sizeof ( sUnitTypeLibraryNode ) ) == sizeof ( sUnitTypeLibraryNode ) );

				// zkontroluje d�lku jm�na knihovny typ� jednotek
				int nUnitTypeLibraryFileNameLength = strlen ( sUnitTypeLibraryNode.m_pFileName );
				LOAD_ASSERT ( ( nUnitTypeLibraryFileNameLength > 0 ) && 
					( nUnitTypeLibraryFileNameLength < sizeof ( sUnitTypeLibraryNode.m_pFileName ) ) );

				// otev�e archiv knihovny typ� jednotek
				CDataArchive *pUnitTypeLibraryArchive = new CDataArchive;
				*pUnitTypeLibraryArchive = cMapArchive.CreateArchive ( 
					sUnitTypeLibraryNode.m_pFileName );

				// otev�e hlavi�kov� soubor knihovny
				CArchiveFile cUnitTypeLibraryHeaderFile = 
					pUnitTypeLibraryArchive->CreateFile ( _T("Library.Header"), 
					CFile::modeRead | CFile::shareDenyWrite );

				// p�e�te hlavi�ku verze knihovny typ� jednotek
				{
					SUnitTypeFileVersionHeader sUnitTypeLibraryVersionHeader;
					LOAD_ASSERT ( cUnitTypeLibraryHeaderFile.Read ( &sUnitTypeLibraryVersionHeader, sizeof ( sUnitTypeLibraryVersionHeader ) ) == sizeof ( sUnitTypeLibraryVersionHeader ) );

					// identifik�tor souboru knihovny typ� jednotek
					BYTE aUnitTypeLibraryFileID[16] = UNITTYPE_FILE_IDENTIFIER;
					ASSERT ( sizeof ( aUnitTypeLibraryFileID ) == 
						sizeof ( sUnitTypeLibraryVersionHeader.m_aFileID ) );

					// zkontroluje identifik�tor souboru knihovny typ� jednotek
					for ( int nIndex = 16; nIndex-- > 0; )
					{
						LOAD_ASSERT ( aUnitTypeLibraryFileID[nIndex] == 
							sUnitTypeLibraryVersionHeader.m_aFileID[nIndex] );
					}

					ASSERT ( CURRENT_MAP_FILE_VERSION >= COMPATIBLE_MAP_FILE_VERSION );
					// zkontroluje ��sla verz� form�tu knihovny typ� jednotek
					LOAD_ASSERT ( sUnitTypeLibraryVersionHeader.m_dwFormatVersion >= 
						sUnitTypeLibraryVersionHeader.m_dwCompatibleFormatVersion );

					// zjist�, jedn�-li se o spr�vnou verzi form�tu knihovny typ� jednotek
					if ( sUnitTypeLibraryVersionHeader.m_dwFormatVersion != CURRENT_MAP_FILE_VERSION )
					{	// nejedn� se o spr�vnou verzi form�tu knihovny typ� jednotek
						// zjist�, jedn�-li se o starou verzi form�tu knihovny typ� jednotek
						if ( sUnitTypeLibraryVersionHeader.m_dwFormatVersion < CURRENT_MAP_FILE_VERSION )
						{	// jedn� se o starou verzi form�tu knihovny typ� jednotek
							// zkontroluje kompatabilitu verze form�tu knihovny typ� jednotek
							LOAD_ASSERT ( sUnitTypeLibraryVersionHeader.m_dwFormatVersion >= 
								COMPATIBLE_MAP_FILE_VERSION );
						}
						else
						{	// jedn� se o mlad�� verzi form�tu knihovny typ� jednotek
							// zkontroluje kompatabilitu verze form�tu knihovny typ� jednotek
							LOAD_ASSERT ( sUnitTypeLibraryVersionHeader.m_dwCompatibleFormatVersion <= 
								CURRENT_MAP_FILE_VERSION );
						}
					}
				}

				// p�e�te hlavi�ku knihovny typ� jednotek
				SUnitTypeLibraryHeader sUnitTypeLibraryHeader;
				LOAD_ASSERT ( cUnitTypeLibraryHeaderFile.Read ( &sUnitTypeLibraryHeader, sizeof ( sUnitTypeLibraryHeader ) ) == sizeof ( sUnitTypeLibraryHeader ) );

				// aktualizuje po�et typ� jednotek v knihovn�ch typ� jednotek
				dwUnitTypeCount += sUnitTypeLibraryHeader.m_dwUnitTypeCount;

				// zav�e hlavi�kov� soubor knihovny
				cUnitTypeLibraryHeaderFile.Close ();

				// p�id� archiv knihovny typ� jednotek do seznamu archiv� knihoven typ� 
				//		jednotek
				cUnitTypeLibraryArchiveArray.Add ( pUnitTypeLibraryArchive );
			}

			// zkontroluje po�et typ� jednotek v knihovn�ch jednotek
			LOAD_ASSERT ( dwUnitTypeCount > 0 );
		}

	// vytvo�� typy jednotek civilizac�
		{
			// zjist� ukazatel na prvn� civilizaci z mapy
			CSCivilization *pCivilization = m_pCivilizations;

			// projede v�echny civilizace z mapy
			for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
				dwCivilizationIndex-- > 0; pCivilization++ )
			{
				// nech� vytvo�it typy jednotek civilizace
				pCivilization->CreateUnitTypes ( &cUnitTypeLibraryArchiveArray, dwUnitTypeCount );
			}

			// zjist� ukazatel na prvn� civilizaci z mapy
			pCivilization = m_pCivilizations;

			// projede v�echny civilizace z mapy
			for ( dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; 
				pCivilization++ )
			{
				// nech� dokon�it vytvo�en� typ� jednotek civilizace
				pCivilization->PostCreateUnitTypes ();
			}
		}

	// p�esko�� pole offset� MapSquar�
		{
			// zjist� velikost pole offset� MapSquar�
			DWORD dwMapSquareOffsetArraySize = m_sizeMapMapSquare.cx * 
				m_sizeMapMapSquare.cy * sizeof ( DWORD );

			// p�esko�� pole offset� MapSquar�
			LOAD_ASSERT ( cMapFile.GetPosition () + dwMapSquareOffsetArraySize <= 
				cMapFile.GetLength () );
			(void)cMapFile.Seek ( dwMapSquareOffsetArraySize, CFile::current );
		}
	
	// na�te informace o MapSquarech
		{
			// index MapSquaru
			CPointDW pointIndex;

			// na�te MapSquary
			for ( pointIndex.y = 0; pointIndex.y < m_sizeMapMapSquare.cy; pointIndex.y++ )
			{
				for ( pointIndex.x = 0; pointIndex.x < m_sizeMapMapSquare.cx; pointIndex.x++ )
				{
					// na�te dal�� MapSquare
					GetMapSquareFromIndex ( pointIndex )->Create ( cMapFile );
				}
			}
		}

		// vytvo�� memory pool pol� viditelnosti jednotky civilizacemi
		CSUnit::m_cCivilizationVisibilityArrayMemoryPool.Create ( 
			max ( m_dwCivilizationCount * sizeof ( WORD ), sizeof ( BYTE * ) ) );
		// vytvo�� memory pool pol� posledn�ch pozic, kde byla jednotka civilizacemi vid�na
		CSUnit::m_cCivilizationLastSeenPositionArrayMemoryPool.Create ( 
			max ( m_dwCivilizationCount * sizeof ( CSPosition ), sizeof ( BYTE * ) ) );

	// na�te hlavi�ky jednotek
		{
			SUnitHeader sUnitHeader;

#ifdef _DEBUG
			// uschov� ID MainLoop threadu
			DWORD dwMainLoopThreadID = m_dwMainLoopThreadID;
			// nasimuluje MainLoop thread
			m_dwMainLoopThreadID = GetCurrentThreadId ();
#endif //_DEBUG

			for ( DWORD dwUnitIndex = dwUnitCount; dwUnitIndex-- > 0; )
			{
				// na�te hlavi�ku jednotky
				LOAD_ASSERT ( cMapFile.Read ( &sUnitHeader, sizeof ( sUnitHeader ) ) == sizeof ( sUnitHeader ) );

				// zkontroluje um�st�n� jednotky na map�
				LOAD_ASSERT ( sUnitHeader.m_dwX < m_sizeMapMapCell.cx );
				LOAD_ASSERT ( sUnitHeader.m_dwY < m_sizeMapMapCell.cy );

				// zjist� index civilizace jednotky
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

				// vytvo�� novou jednotku typu "pUnitType"
				CSUnit *pUnit = new CSUnit;
				pUnit->Create ( pUnitType );

				// zv��� po�et odkaz� na jednotku
				pUnit->AddRef ();
				// p�id� vytvo�enou jednotku do jednotek mapy
				AddUnit ( pUnit );

				// nastav� pozici jednotky na map�
				LOAD_ASSERT ( sUnitHeader.m_dwZ != 0 );
				pUnit->SetVerticalPosition ( sUnitHeader.m_dwZ );
				CPointDW pointPosition ( sUnitHeader.m_dwX, sUnitHeader.m_dwY );
				LOAD_ASSERT ( CheckLoadedMapPosition ( pointPosition ) );
				pUnit->SetPosition ( pointPosition );
				// nastav� kontroln� body jednotky
				pUnit->SetFirstCheckPoint ( pointPosition, 0 );
				pUnit->SetSecondCheckPoint ( pointPosition, 0 );
				// nastav� nasm�rov�n� jednotky
				LOAD_ASSERT ( sUnitHeader.m_nDirection <= Dir_Random );
				pUnit->SetDirection ( (BYTE)sUnitHeader.m_nDirection );
			}

#ifdef _DEBUG
			// vr�t� uschovan� ID MainLoop threadu
			m_dwMainLoopThreadID = dwMainLoopThreadID;
#endif //_DEBUG
		}

	// na�te jm�na surovin
		{
			SResource sResource;

			// zjist� velikost ikony suroviny
			DWORD dwResourceIconSize = RESOURCE_ICON_WIDTH * 
				RESOURCE_ICON_HEIGHT * sizeof ( DWORD );

			// projede suroviny
			for ( DWORD dwResourceIndex = 0; dwResourceIndex < RESOURCE_COUNT; 
				dwResourceIndex++ )
			{
				// na�te surovinu
				LOAD_ASSERT ( cMapFile.Read ( &sResource, sizeof ( sResource ) ) == sizeof ( sResource ) );

				// zkontroluje d�lku jm�na suroviny
				m_aResourceName[dwResourceIndex] = sResource.m_szName;
				LOAD_ASSERT ( m_aResourceName[dwResourceIndex].GetLength () < sizeof ( sResource.m_szName ) );

				// p�esko�� ikonu suroviny
				LOAD_ASSERT ( cMapFile.GetPosition () + dwResourceIconSize <= 
					cMapFile.GetLength () );
				(void)cMapFile.Seek ( dwResourceIconSize, CFile::current );
			}
		}

	// na�te jm�na neviditelnost�
		{
			SInvisibility sInvisibility;

			// projede neviditelnosti
			for ( DWORD dwInvisibilityIndex = 0; 
				dwInvisibilityIndex < INVISIBILITY_COUNT; dwInvisibilityIndex++ )
			{
				// na�te neviditelnost
				LOAD_ASSERT ( cMapFile.Read ( &sInvisibility, sizeof ( sInvisibility ) ) == sizeof ( sInvisibility ) );

				// zkontroluje d�lku jm�na neviditelnosti
				m_aInvisibilityName[dwInvisibilityIndex] = sInvisibility.m_szName;
				LOAD_ASSERT ( m_aInvisibilityName[dwInvisibilityIndex].GetLength () < sizeof ( sInvisibility.m_szName ) );
			}
		}

	// na�te hlavi�ky graf� pro hled�n� cesty
		{
			SFindPathGraphHeader sFindPathGraphHeader;

			// vytvo�� pole graf� pro hled�n� cesty
			CSFindPathGraph::PreCreateFindPathGraphs ( dwFindPathGraphCount );

			// popis grafu pro hled�n� cesty
			typedef struct 
			{
				DWORD dwID;
				CSFindPathGraph *pFindPathGraph;
				CString strName;
			} SFindPathGraphDescription;

			// pole popis� graf� pro hled�n� cesty
			SFindPathGraphDescription *aFindPathGraphDescription = 
				new SFindPathGraphDescription[dwFindPathGraphCount];

			// projede hlavi�ky graf� pro hled�n� cesty
			for ( DWORD dwFindPathGraphIndex = dwFindPathGraphCount; 
				dwFindPathGraphIndex-- > 0; )
			{
				// na�te hlavi�ku grafu pro hled�n� cest
				LOAD_ASSERT ( cMapFile.Read ( &sFindPathGraphHeader, sizeof ( sFindPathGraphHeader ) ) == sizeof ( sFindPathGraphHeader ) );

				// zkontroluje unik�tnost ID a jm�na grafu pro hled�n� cesty
				for ( DWORD dwPreviousFindPathGraphIndex = dwFindPathGraphCount; 
					--dwPreviousFindPathGraphIndex > dwFindPathGraphIndex; )
				{
					// zkontroluje unik�tnost ID grafu pro hled�n� cesty
					LOAD_ASSERT ( sFindPathGraphHeader.m_dwID != 
						aFindPathGraphDescription[dwPreviousFindPathGraphIndex].dwID );
					// zkontroluje unik�tnost jm�na grafu pro hled�n� cesty
					LOAD_ASSERT ( aFindPathGraphDescription[dwPreviousFindPathGraphIndex].strName.CompareNoCase ( sFindPathGraphHeader.m_pName ) != 0 );
				}

				// inicializuje ID grafu pro hled�n� cesty
				aFindPathGraphDescription[dwFindPathGraphIndex].dwID = 
					sFindPathGraphHeader.m_dwID;
				LOAD_ASSERT ( sFindPathGraphHeader.m_dwID != 0 );

				// zkontroluje d�lku jm�na grafu pro hled�n� cesty
				int nFindPathGraphNameLength = strlen ( sFindPathGraphHeader.m_pName );
				LOAD_ASSERT ( ( nFindPathGraphNameLength > 0 ) && 
					( nFindPathGraphNameLength < sizeof ( sFindPathGraphHeader.m_pName ) ) );
				aFindPathGraphDescription[dwFindPathGraphIndex].strName = 
					sFindPathGraphHeader.m_pName;

				// vytvo�� graf pro hled�n� cesty
				aFindPathGraphDescription[dwFindPathGraphIndex].pFindPathGraph = 
					CSFindPathGraph::CreateFindPathGraph ( dwFindPathGraphIndex, 
					sFindPathGraphHeader.m_dwID, sFindPathGraphHeader.m_pName, cMapArchive );
			}

			// ukazatel na civilizaci
			CSCivilization *pCivilization = m_pCivilizations;

			// aktualizuje ukazatele civilizac� na grafy pro hled�n� cesty
			for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
				dwCivilizationIndex-- > 0; pCivilization++ )
			{
				// ukazatel na typ jednotky
				CSUnitType *pUnitType = pCivilization->m_pUnitTypes;

				// aktualizuje ukazatele typ� jednotek na grafy pro hled�n� cesty
				for ( DWORD dwUnitTypeIndex = pCivilization->m_dwUnitTypeCount; 
					dwUnitTypeIndex-- > 0; pUnitType++ )
				{
					// aktualizuje ukazatele na grafy pro hled�n� cesty
					for ( DWORD dwModeIndex = 8; dwModeIndex-- > 0; )
					{
						// ID grafu pro hled�n� cesty
						DWORD dwFindPathGraphID = 
							(DWORD)pUnitType->m_aFindPathGraph[dwModeIndex];

						// zjist�, jedn�-li se o graf pro vzdu�nou cestu
						if ( dwFindPathGraphID == 0 )
						{	// jedn� se o graf pro vzdu�nou cestu
							pUnitType->m_aFindPathGraph[dwModeIndex] = NULL;
						}
						else
						{	// nejedn� se o graf pro vzdu�nou cestu
							// najde odpov�daj�c� graf pro hled�n� cesty
							for ( DWORD dwIndex = dwFindPathGraphCount; dwIndex-- > 0; )
							{
								// zjist�, jedn�-li se o hledan� graf cesty
								if ( dwFindPathGraphID == 
									aFindPathGraphDescription[dwIndex].dwID )
								{
									// aktualizuje ukazatel na graf pro hled�n� cesty
									pUnitType->m_aFindPathGraph[dwModeIndex] = 
										aFindPathGraphDescription[dwIndex].pFindPathGraph;
									break;
								}
							}
							// zkontroluje spr�vnost ID grafu pro hled�n� cesty
							LOAD_ASSERT ( dwIndex != DWORD_MAX );
						}
						// ukazatel na graf pro hled�n� cesty je aktualizov�n
					}
					// ukazatele na grafy pro hled�n� cesty jsou aktualizov�ny
				}
				// typy jednotek jsou aktualizov�ny
			}
			// civilizace jsou aktualizov�ny

			// zni�� pole popis� graf� pro hled�n� cesty
			if ( aFindPathGraphDescription != NULL )
			{
				delete [] aFindPathGraphDescription;
			}
		}

	// ukon�� na��t�n� souboru mapy

		// zkontroluje konec souboru
		LOAD_ASSERT ( cMapFile.GetPosition () == cMapFile.GetLength () );
		cMapFile.Close ();

		// zni�� pole archiv� knihoven typ� jednotek
		for ( int nIndex = cUnitTypeLibraryArchiveArray.GetSize (); nIndex-- > 0; )
		{
			// z�sk� dal�� ukazatel na archiv knihovny typ� jednotek
			CDataArchive *pUnitTypeLibraryArchive = 
				(CDataArchive *)cUnitTypeLibraryArchiveArray.GetAt ( nIndex );
			// zav�e archiv knihovny typ� jednotek
			VERIFY ( pUnitTypeLibraryArchive->Delete () );
			// zni�� archiv knihovny typ� jednotek
			delete pUnitTypeLibraryArchive;
		}
		// zni�� prvky pole archiv� knihoven jednotek
		cUnitTypeLibraryArchiveArray.RemoveAll ();
	}
	catch ( CException * )
	{
    CSOneInstance::PreDelete ();

		// zni�� pole archiv� knihoven typ� jednotek
		for ( int nIndex = cUnitTypeLibraryArchiveArray.GetSize (); nIndex-- > 0; )
		{
			// z�sk� dal�� ukazatel na archiv knihovny typ� jednotek
			CDataArchive *pUnitTypeLibraryArchive = 
				(CDataArchive *)cUnitTypeLibraryArchiveArray.GetAt ( nIndex );
			// zav�e archiv knihovny typ� jednotek
			VERIFY ( pUnitTypeLibraryArchive->Delete () );
			// zni�� archiv knihovny typ� jednotek
			delete pUnitTypeLibraryArchive;
		}
		// zni�� prvky pole archiv� knihoven jednotek
		cUnitTypeLibraryArchiveArray.RemoveAll ();

		// zne�kodn� �aste�n� inicializovan� data mapy
		SetEmptyData ();

		// zni�it statick� v�ci
		CSOneInstance::Delete ();

		// pokra�uje ve zpracov�n� v�jimky
		throw;
	}

// ukon�� vytv��en� graf� pro hled�n� cesty

	// ukon�� vytv��en� graf� pro hled�n� cesty
	CSFindPathGraph::PostCreateFindPathGraphs ();

// vytvo�� data cest

	// vytvo�� data cest
	CSPath::CreatePathes ( cMapArchive );

// vytvo�� mapy jednotek

	// vytvo�� mapy jednotek
	CSMapSquare::CreateUnitMaps ();

// inicializace hry

	// inicializuje ��slo TimeSlicu
	m_dwTimeSlice = 1;

	try
	{
		// inicializuje index rohov�ho MapSquaru
		m_pointCornerMapSquare = CPointDW ( m_sizeMapMapSquare.cx - 1, 
			m_sizeMapMapSquare.cy - 1 );
		m_pointCornerMapCell = CPointDW ( m_sizeMapMapCell.cx - 1, 
			m_sizeMapMapCell.cy - 1 );

	// um�st�n� jednotek na mapu

#ifdef _DEBUG
		// uschov� ID MainLoop threadu
		DWORD dwMainLoopThreadID = m_dwMainLoopThreadID;
		// nasimuluje MainLoop thread
		m_dwMainLoopThreadID = GetCurrentThreadId ();
#endif //_DEBUG

		// vytvo�� pole p��znak� objeven� pozic na map� pro metodu "GetNearestUnitPlace"
		m_cGetNearestUnitPlacePositionArray.Create ( m_sizeMapMapCell.cx, 
			m_sizeMapMapCell.cy, 2 * MAP_SQUARE_SIZE, 0 );

		// projede v�echny bloky jednotek
		for ( SUnitBlock *pBlock = m_pFirstUnitBlock; pBlock != NULL; 
			pBlock = pBlock->pNext )
		{
			// ukazatel na jednotku v bloku
			CSUnit **pBlockUnit = pBlock->pUnits;
			// projede v�echny jednotky v bloku jednotek
			for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
			{
				// z�sk� ukazatel na jednotku
				CSUnit *pUnit = *pBlockUnit;

				// zjist�, je-li to platn� jednotka
				if ( pUnit == NULL )
				{	// nen� to platn� jednotka
					continue;
				}
				// je to platn� jednotka

				// um�st� jednotku na mapu
				pUnit->SetPosition ( GetNearestUnitPlace ( pUnit, pUnit->GetPosition () ) );
				LOAD_ASSERT ( IsMapPosition ( pUnit->GetPosition () ) );
				UnitPlaced ( pUnit );
				// nastav� kontroln� body jednotky
				pUnit->SetFirstCheckPoint ( pUnit->GetPosition (), 0 );
				pUnit->SetSecondCheckPoint ( pUnit->GetPosition (), 0 );
			}
		}

#ifdef _DEBUG
		// vr�t� uschovan� ID MainLoop threadu
		m_dwMainLoopThreadID = dwMainLoopThreadID;
#endif //_DEBUG

	// inicializace notifikac�

		// inicializuje notifikace
		InitializeNotifications ( TRUE );

	// roze�le civilizac�m zpr�vu o inicializovan� h�e

		// zjist� ukazatel na prvn� civilizaci z mapy
		CSCivilization *pCivilization = m_pCivilizations;

		// projede v�echny civilizace z mapy
		for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
			dwCivilizationIndex-- > 0; pCivilization++ )
		{
			// po�le civilizaci zpr�vu o inicializovan� h�e
			pCivilization->m_pZCivilization->OnGameInitialized ();
		}

	// inicializace hry

		// inicializuje hru (vytvo�� zapausovanou hru)
		InitGame ();
	}
	catch ( CException * )
	{
		// zne�kodn� �aste�n� inicializovan� data mapy
		SetEmptyData ();

		// pokra�uje ve zpracov�v�n� v�jimky
		throw;
	}

//	TRACE ( "Map created\n" );
}

// zni�� data mapy
void CSMap::Delete () 
{
	// p��znak inicializovan� mapy
	BOOL bInitialized = m_bInitialized;
	// zru�� p��znak inicializovan� mapy
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

// zni�� thready mapy

	// zjist�, je-li mapa inicializovan�
	if ( bInitialized )
	{	// mapa je inicializovan�
		// zjist�, je-li hra zapausovan�
		if ( !IsGamePaused () )
		{	// hra nen� zapausovan�
			// zapausuje hru
			PauseGame ();
		}

		// odpausuje hru s ukon�en�m hry
		UnpauseGame ( FALSE );
	}

// zni�� notifikace

	// zni�� notifikace
	DestructNotifications ();

// zni�� MapSquary

	if ( m_pMapSquares != NULL )
	{
		// zjist� ukazatel na prvn� MapSquare
		CSMapSquare *pMapSquare = m_pMapSquares;

		// projede v�echny MapSquary
		for ( DWORD dwMapSquareIndex = m_sizeMapMapSquare.cx * m_sizeMapMapSquare.cy; 
			dwMapSquareIndex-- > 0; pMapSquare++ )
		{
			// zni�� dal�� MapSquare
			pMapSquare->Delete ();
		}

		// zni�� pole MapSquar�
		delete [] m_pMapSquares;
		m_pMapSquares = NULL;
	}

// zru�� odkazy na jednotky na map�

//	TRACE ( "------------------------------ blocks\n" );

	// projede v�echny bloky jednotek
	for ( SUnitBlock *pBlock = m_pFirstUnitBlock; pBlock != NULL; )
	{
		// ukazatel na jednotku v bloku
		CSUnit **pBlockUnit = pBlock->pUnits;
		// projede v�echny jednotky v bloku jednotek
		for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
		{
			// ukazatel na jednotku
			CSUnit *pUnit = *pBlockUnit;
			// zjist�, je-li to platn� jednotka
			if ( pUnit != NULL )
			{	// je to platn� jednotka
	
//				TRACE ( "Unit ID=%d explicitly deleted (lives=%d, refcount=%d)\n", 
//					pUnit->GetID (), pUnit->GetLives (), pUnit->m_dwReferenceCounter );

				// sn�� jednotce po�et �ivot�
				pUnit->m_nLives = DEAD_UNIT_LIVES;
				// p�id� jednotku do seznamu smazan�ch jednotek
				m_cDeletedUnitList.Add ( pUnit );

				ASSERT ( ( pUnit->m_pUnitType != NULL ) && ( pUnit->m_pSkillData != NULL ) );
				// zni�� skilly jednotky
				pUnit->m_pUnitType->DestructSkills ( pUnit );

				// zru�� odkaz na jednotku
				pUnit->Release ();
				*pBlockUnit = NULL;
			}
		}

		// zjist� ukazatel na dal�� blok jednotek
		pBlock = pBlock->pNext;
	}

	// popis mrtv� jednotky
	struct SDeadUnit sDeadUnit;

//	TRACE ( "------------------------------ m_cDeadUnitQueue\n" );

	// zni�� frontu mrtv�ch jednotek
	while ( m_cDeadUnitQueue.RemoveFirst ( sDeadUnit ) )
	{
		ASSERT ( sDeadUnit.pUnit != NULL );
		// sn�� jednotce po�et �ivot�
		sDeadUnit.pUnit->m_nLives = DEAD_UNIT_LIVES;
		// p�id� jednotku do seznamu smazan�ch jednotek
		m_cDeletedUnitList.Add ( sDeadUnit.pUnit );

//		TRACE ( "Unit ID=%d m_cDeadUnitQueue.Remove (lives=%d, refcount=%d)\n", 
//			sDeadUnit.pUnit->GetID (), sDeadUnit.pUnit->GetLives (), 
//			sDeadUnit.pUnit->m_dwReferenceCounter );

		ASSERT ( ( sDeadUnit.pUnit->m_pUnitType != NULL ) && 
			( sDeadUnit.pUnit->m_pSkillData != NULL ) );
		// zni�� skilly jednotky
		sDeadUnit.pUnit->m_pUnitType->DestructSkills ( sDeadUnit.pUnit );

		// zru�� odkaz na jednotku
		sDeadUnit.pUnit->Release ();
	}

// zni�� seznam mrtv�ch jednotek pro rozes�l�n� stop infa klient�m

//	TRACE ( "------------------------------ m_cDeadUnitClientStopInfoList\n" );

	// mrtv� jednotka
	CSUnit *pUnit;
	// zpracuje seznam mrtv�ch jednotek pro rozes�l�n� stop infa klient�m
	while ( m_cDeadUnitClientStopInfoList.RemoveFirst ( pUnit ) )
	{
//		TRACE ( "Unit ID=%d m_cDeadUnitClientStopInfoList.Remove (lives=%d, refcount=%d)\n", 
//			pUnit->GetID (), pUnit->GetLives (), pUnit->m_dwReferenceCounter );
		// sn�� po�et odkaz� na jednotku
		pUnit->Release ();
	}

// zni�� odlo�en� vol�n� procedur civilizac�

	// projede v�echny civilizace
	for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; )
	{
		CSDeferredProcedureCall *pDPC;
		// projede v�echna odlo�en� vol�n� procedur civilizace
		while ( m_aDPCQueue[dwCivilizationIndex].RemoveFirst ( pDPC ) )
		{
			// zni�� dal�� odlo�en� vol�n� procedury
			delete pDPC;
		}
	}

// zni�� civilizace

	if ( m_pCivilizations != NULL )
	{
		// zjist� ukazatel na prvn� civilizaci
		CSCivilization *pCivilization = m_pCivilizations;

		// projede v�echny civilizace
		for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; 
			pCivilization++ )
		{
			// zni�� civilizaci na civilizaci
			if ( pCivilization->m_pZCivilization != NULL )
			{
				pCivilization->m_pZCivilization->PreDelete ();
			}
		}

		// zjist� ukazatel na prvn� civilizaci
		pCivilization = m_pCivilizations;

		// projede v�echny civilizace
		for ( dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; 
			pCivilization++ )
		{
			// zni�� civilizaci na civilizaci
			if ( pCivilization->m_pZCivilization != NULL )
			{
				pCivilization->m_pZCivilization->Delete ();
			}
		}

		// zjist� ukazatel na prvn� civilizaci
		pCivilization = m_pCivilizations;

		// projede v�echny civilizace
		for ( dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; 
			pCivilization++ )
		{
			// zni�� civilizaci na civilizaci
			if ( pCivilization->m_pZCivilization != NULL )
			{
				delete pCivilization->m_pZCivilization;
				pCivilization->m_pZCivilization = NULL;
			}
			// zni�� dal�� civilizaci
			pCivilization->Delete ();
		}

		// zni�� pole civilizac�
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

// zni�� jednotky na map�

	// projede v�echny bloky jednotek
	for ( pBlock = m_pFirstUnitBlock; pBlock != NULL; )
	{
		// uschov� si ukazatel na blok jednotek
		SUnitBlock *pLastBlock = pBlock;
		// zjist� ukazatel na dal�� blok jednotek
		pBlock = pBlock->pNext;

		// zni�� pole ukazatel� na jednotky bloku jednotek
		delete pLastBlock->pUnits;
		// zni�� uschovan� blok jednotek
		delete pLastBlock;
	}

	// zne�kodn� ukazatele na bloky jednotek
	m_pFirstUnitBlock = NULL;
	m_pLastUnitBlock = NULL;

	// projede v�echny z�znamy o voln�ch jednotk�ch
	for ( SFreeUnit *pFreeUnit = m_pFirstFreeUnit; pFreeUnit != NULL; )
	{
		// uschov� si ukazatel na z�znam o voln� jednotce
		SFreeUnit *pLastFreeUnit = pFreeUnit;
		// zjist� ukazatel na dal�� z�znam o voln� jednotce
		pFreeUnit = pFreeUnit->pNext;

		// zni�� uschovan� z�znam o voln� jednotce
		delete pLastFreeUnit;
	}

	// zne�kodn� ukazatel na prvn� volnou jednotku
	m_pFirstFreeUnit = NULL;

	// zni�� memory pool pol� viditelnosti jednotky civilizacemi
	CSUnit::m_cCivilizationVisibilityArrayMemoryPool.Delete ();
	// zni�� memory pool pol� posledn�ch pozic, kde byla jednotka civilizacemi vid�na
	CSUnit::m_cCivilizationLastSeenPositionArrayMemoryPool.Delete ();

// zni�� pomocn� data

	// zni�� pole p��znak� objeven� pozic na map� pro metodu "GetNearestUnitPlace"
	m_cGetNearestUnitPlacePositionArray.Delete ();

// zni�� data cest

#ifdef _DEBUG
		// uschov� ID FindPathLoop threadu
		DWORD dwFindPathLoopThreadID = m_pFindPathLoopThread->m_nThreadID;
		// nasimuluje FindPathLoop thread
		m_pFindPathLoopThread->m_nThreadID = GetCurrentThreadId ();
#endif //_DEBUG

	// zni�� data cest
	CSPath::DeletePathes ();

#ifdef _DEBUG
		// vr�t� uschovan� ID FindPathLoop threadu
		m_pFindPathLoopThread->m_nThreadID = dwFindPathLoopThreadID;
#endif //_DEBUG

// zni�� grafy pro hled�n� cesty

	// zni�� grafy pro hled�n� cesty
	CSFindPathGraph::DeleteFindPathGraphs ();

// vytvo�� mapy jednotek

	// vytvo�� mapy jednotek
	CSMapSquare::DeleteUnitMaps ();

// zni�� thready mapy

	// zjist�, je-li mapa inicializovan�
	if ( bInitialized )
	{	// mapa je inicializovan�
	// uschov� handly thread� mapy

		CWinThread *pMainLoopThread = m_pMainLoopThread;
		CWinThread *pClientInfoSenderLoopThread = m_pClientInfoSenderLoopThread;
		CWinThread *pFindPathLoopThread = m_pFindPathLoopThread;

		m_pMainLoopThread = NULL;
		m_pClientInfoSenderLoopThread = NULL;
		m_pFindPathLoopThread = NULL;

	// zne�kodn� data mapy

		SetEmptyData ();

	// po�k� na dojet� thread� mapy

		// tabulka pausovan�ch thread�
		HANDLE aPausingThreads[3] = { 
			pMainLoopThread->m_hThread, 
			pClientInfoSenderLoopThread->m_hThread, 
			pFindPathLoopThread->m_hThread, 
		};

		// po�k� na ukon�en� pausovan�ch thread�
		switch ( WaitForMultipleObjects ( 3, aPausingThreads, TRUE, INFINITE ) )
		{
		// thready byly ukon�eny
		case WAIT_OBJECT_0 :
		case ( WAIT_OBJECT_0 + 1 ) :
		case ( WAIT_OBJECT_0 + 2 ) :
			break;
		// thready se nepovedlo ukon�it
		default:
			TRACE0 ( _T("Error - can't end threads correctly (time out)\n") );
		}

	// zni�� handly thread� mapy

		delete pMainLoopThread;
		delete pClientInfoSenderLoopThread;
		delete pFindPathLoopThread;
	}
	else
	{	// mapa nen� inicializovan�
		// zne�kodn� data mapy
		SetEmptyData ();
	}

	// zni�it statick� v�ci
	CSOneInstance::Delete ();
}

// vypln� objekt ne�kodn�mi daty (zni�� ��ste�n� inicializavan� data)
void CSMap::SetEmptyData () 
{
	// aktualizuje p��znak pokra�ov�n� hry
	m_bContinueUnpausedGame = FALSE;

	// zru�� ud�lost zapausov�n� hry
	VERIFY ( m_eventPauseGame.ResetEvent () );
	// nastav� ud�lost odpausov�n� hry
	VERIFY ( m_eventUnpauseGame.SetEvent () );

	// zne�kodn� MapSquary
	if ( m_pMapSquares != NULL )
	{
		// zni�� pole MapSquar�
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

	// zne�kodn� jm�na surovin
	for ( DWORD dwResourceIndex = RESOURCE_COUNT; dwResourceIndex-- > 0; )
	{
		m_aResourceName[dwResourceIndex].Empty ();
	}

	// zne�kodn� jm�na neviditelnost�
	for ( DWORD dwInvisibilityIndex = INVISIBILITY_COUNT; dwInvisibilityIndex-- > 0; )
	{
		m_aInvisibilityName[dwInvisibilityIndex].Empty ();
	}

	// zne�kodn� civilizace
	if ( m_pCivilizations != NULL )
	{
		ASSERT ( m_dwCivilizationCount > 0 );
		// projede v�echny civilizace
		for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; )
		{
			// zne�kodn� dal�� civilziaci
			m_pCivilizations[dwCivilizationIndex].SetEmptyData ();
		}
		// zni�� pole civilizac�
		delete [] m_pCivilizations;
		m_pCivilizations = NULL;
	}
	m_dwCivilizationCount = 0;

	m_pFirstUnitBlock = NULL;
	m_pLastUnitBlock = NULL;
	m_pFirstFreeUnit = NULL;

	m_dwTimeSlice = 0;

	// zjist�, je-li platn� odkaz na MainLoop thread
	if ( m_pMainLoopThread != NULL )
	{	// zni�� odkaz na MainLoop thread
		delete m_pMainLoopThread;
		m_pMainLoopThread = NULL;
	}
	// zjist�, je-li platn� odkaz na ClientInfoSenderLoop thread
	if ( m_pClientInfoSenderLoopThread != NULL )
	{	// zni�� odkaz na ClientInfoSenderLoop thread
		delete m_pClientInfoSenderLoopThread;
		m_pClientInfoSenderLoopThread = NULL;
	}
	// zjist�, je-li platn� odkaz na FindPathLoop thread
	if ( m_pFindPathLoopThread != NULL )
	{	// zni�� odkaz na FindPathLoop thread
		delete m_pFindPathLoopThread;
		m_pFindPathLoopThread = NULL;
	}

	// projede v�echny civilizace
	for ( DWORD dwIndex = CIVILIZATION_COUNT_MAX; dwIndex-- > 0; )
	{
		CSDeferredProcedureCall *pDPC;
		// projede v�echna odlo�en� vol�n� procedur civilizace
		while ( m_aDPCQueue[dwIndex].RemoveFirst ( pDPC ) )
		{
			// zni�� dal�� odlo�en� vol�n� procedury
			delete pDPC;
		}
	}
	m_pPreparingDPC = NULL;

	// zni�� pole u�ivatel�
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zni�� popis u�ivatele
		delete m_aUserDescriptions.GetAt ( nIndex );
	}
	// zni�� prvky pole u�ivatel�
	m_aUserDescriptions.RemoveAll ();
}

//////////////////////////////////////////////////////////////////////
// Ukl�d�n� dat (CPersistentObject metody)
//////////////////////////////////////////////////////////////////////

/*
CSMap stored

	DWORD m_dwTimeSlice
	DWORD dwMapSizeX (m_sizeMapMapCell.cx)
	DWORD dwMapSizeY (m_sizeMapMapCell.cy)
	int nMapArchiveNameSize						// mus� b�t > 0
		char aMapArchiveName[nMapArchiveNameSize]
	DWORD m_dwMapFormatVersion
	DWORD m_dwMapCompatibleFormatVersion
	int nMapNameSize							// m��e b�t i 0 - pak je vynech�no aMapName
		char aMapName[nMapNameSize]
	int nMapDescriptionSize					// m��e b�t i 0 - pak je vynech�no aMapDescription
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
		int nUserPasswordSize				// m��e b�t i 0 - pak je vynech�no aUserPassword
		char aUserPassword[nUserPasswordSize]
		DWORD dwCivilizationIndex
	find path graphs stored
	pathes stored
	CStringTable stored
	CSOneInstance stored
*/

// ukl�d�n� dat
void CSMap::PersistentSave ( CPersistentStorage &storage ) 
{
//	BRACE_BLOCK(storage); TADY NE kvuli return FALSE; v PersistentLoad

	ASSERT ( CheckValidData () );

// ulo�� z�kladn� informace o h�e

	// ulo�� ��slo TimeSlicu
	storage << m_dwTimeSlice;

// ulo�� z�kladn� informace o map�

	// ulo�� velikost mapy
	storage << m_sizeMapMapCell.cx;
	storage << m_sizeMapMapCell.cy;

	// zjist� d�lku jm�na archivu mapy
	int nMapArchiveNameSize = m_strMapArchiveName.GetLength ();
	ASSERT ( nMapArchiveNameSize > 0 );
	// ulo�� d�lku jm�na archivu mapy
	storage << nMapArchiveNameSize;
	// ulo�� jm�no archivu mapy
	storage.Write ( m_strMapArchiveName, nMapArchiveNameSize );

	// ulo�� verzi form�tu mapy
	storage << m_dwMapFormatVersion;
	// ulo�� kompatabiln� verzi form�tu mapy
	storage << m_dwMapCompatibleFormatVersion;

	// zjist� d�lku jm�na mapy
	int nMapNameSize = m_strMapName.GetLength ();
	// ulo�� d�lku jm�na mapy
	storage << nMapNameSize;
	// zjist�, je-li jm�no mapy pr�zdn�
	if ( nMapNameSize > 0 )
	{	// jm�no mapy nen� pr�zdn�
		// ulo�� jm�no mapy
		storage.Write ( m_strMapName, nMapNameSize );
	}

	// zjist� d�lku popisu mapy
	int nMapDescriptionSize = m_strMapDescription.GetLength ();
	// ulo�� d�lku popisu mapy
	storage << nMapDescriptionSize;
	// zjist�, je-li popis mapy pr�zdn�
	if ( nMapDescriptionSize > 0 )
	{	// popis mapy nen� pr�zdn�
		// ulo�� popis mapy
		storage.Write ( m_strMapDescription, nMapDescriptionSize );
	}

	// ulo�� verzi mapy
	storage << m_dwMapVersion;

// ulo�� civilizace

	// ulo�� po�et civilizac�
	storage << m_dwCivilizationCount;

	// zjist� ukazatel na prvn� civilizaci
	CSCivilization *pCivilization = m_pCivilizations;

	// projede v�echny civilizace
	for ( DWORD dwCivilizationIndex = 0; dwCivilizationIndex < m_dwCivilizationCount; 
		dwCivilizationIndex++, pCivilization++ )
	{
		// ulo�� ukazatel na civilizaci
		storage << pCivilization;
		// ulo�� civilizaci
		pCivilization->PersistentSave ( storage );

	// ulo�� DPC frontu civilizace

		// ulo�� velikost DPC fronty civilizace
		storage << m_aDPCQueue[dwCivilizationIndex].GetSize ();

		// zjist� pozici prvn�ho prvku v DPC front� civilizace
		POSITION posDPC = m_aDPCQueue[dwCivilizationIndex].GetHeadPosition ();
		// ukazatel na DPC
		CSDeferredProcedureCall *pDPC;

		// projede DPC frontu civilizace
		while ( CSDeferredProcedureCallQueue::GetNext ( posDPC, pDPC ) )
		{
			// ulo�� ukazatel na DPC
			storage << pDPC;
			// ulo�� DPC ID
			storage << pDPC->GetID ();

			// ulo�� DPC
			pDPC->PersistentSave ( storage );
		}
	}

// pomocn� prom�nn�

	// po�et jednotek na map�
	DWORD dwUnitCount = 0;
	// ukazatel na blok jednotek
	SUnitBlock *pBlock;

// spo��t� po�et jednotek

	// projede v�echny bloky jednotek
	for ( pBlock = m_pFirstUnitBlock; pBlock != NULL; pBlock = pBlock->pNext )
	{
		// ukazatel na jednotku v bloku
		CSUnit **pBlockUnit = pBlock->pUnits;
		// projede v�echny jednotky v bloku jednotek
		for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
		{
			// zjist�, je-li to platn� jednotka
			if ( *pBlockUnit != NULL )
			{	// je to platn� jednotka
				// zv��� po�et jednotek na map�
				dwUnitCount++;
			}
		}
	}

// ulo�� jednotky na map�

	// ulo�� po�et jednotek
	storage << dwUnitCount;

	// projede v�echny bloky jednotek
	for ( pBlock = m_pFirstUnitBlock; pBlock != NULL; pBlock = pBlock->pNext )
	{
		// ukazatel na jednotku v bloku
		CSUnit **pBlockUnit = pBlock->pUnits;
		// projede v�echny jednotky v bloku jednotek
		for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
		{
			// zjist�, je-li to platn� jednotka
			if ( *pBlockUnit != NULL )
			{	// je to platn� jednotka
				// ulo�� ukazatel na jednotku
				storage << (*pBlockUnit);
				// ulo�� jednotku
				(*pBlockUnit)->PersistentSave ( storage );
			}
		}
	}

// ulo�� frontu mrtv�ch jednotek

	// po�et mrtv�ch jednotek
	DWORD dwDeadUnitCount = m_cDeadUnitQueue.GetSize ();

	// ulo�� po�et mrtv�ch jednotek
	storage << dwDeadUnitCount;

	// popis mrtv� jednotky
	struct SDeadUnit sDeadUnit;
	// pozice mrtv� jednotky ve front� mrtv�ch jednotek
	POSITION posDeadUnit = m_cDeadUnitQueue.GetHeadPosition ();

	// ulo�� frontu mrtv�ch jednotek
	while ( m_cDeadUnitQueue.GetNext ( posDeadUnit, sDeadUnit ) )
	{
		// ulo�� popis mrtv� jednotky
		storage << sDeadUnit.dwTimeSlice;
		storage << sDeadUnit.pUnit;
		// ulo�� mrtvou jednotku
		sDeadUnit.pUnit->PersistentSave ( storage );
	}

// ulo�� seznam smazan�ch jednotek

	// po�et smazan�ch jednotek
	DWORD dwDeletedUnitCount = m_cDeletedUnitList.GetSize ();

	// ulo�� po�et smazan�ch jednotek
	storage << dwDeletedUnitCount;

	// ukazatel na smazanou jednotku
	CSUnit *pDeletedUnit;
	// pozice smazan� jednotky v seznamu smazan�ch jednotek
	POSITION posDeletedUnit = m_cDeletedUnitList.GetHeadPosition ();

	// ulo�� seznam smazan�ch jednotek
	while ( m_cDeletedUnitList.GetNext ( posDeletedUnit, pDeletedUnit ) )
	{
		// ulo�� ukazatel na smazanou jednotku
		storage << pDeletedUnit;
		// ulo�� smazanou jednotku
		pDeletedUnit->PersistentSave ( storage );
	}

// ulo�� pole u�ivatel�

	// ulo�� po�et u�ivatel�
	storage << (DWORD)m_aUserDescriptions.GetSize ();

	// projede pole u�ivatel�
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjist� popis u�ivatele
		struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );

		// zjist� d�lku jm�na u�ivatele
		int nUserNameSize = pUserDescription->strName.GetLength ();
		ASSERT ( nUserNameSize > 0 );
		// ulo�� d�lku jm�na u�ivatele
		storage << nUserNameSize;
		// ulo�� jm�no u�ivatele
		storage.Write ( pUserDescription->strName, nUserNameSize );

		// zjist� d�lku hesla u�ivatele
		int nUserPasswordSize = pUserDescription->strPassword.GetLength ();
		// ulo�� d�lku hesla u�ivatele
		storage << nUserPasswordSize;
		// zjist�, je-li heslo u�ivatele pr�zdn�
		if ( nUserPasswordSize > 0 )
		{	// heslo u�ivatele nen� pr�zdn�
			// ulo�� heslo u�ivatele
			storage.Write ( pUserDescription->strPassword, nUserPasswordSize );
		}

		// ulo�� index civilizace
		ASSERT ( pUserDescription->dwCivilizationIndex > 0 );
		ASSERT ( pUserDescription->dwCivilizationIndex < m_dwCivilizationCount );
		storage << pUserDescription->dwCivilizationIndex;

		// ulo�� p��znak povolen�
		storage << pUserDescription->bEnabled;
	}

// ulo�� grafy pro hled�n� cesty

	// ulo�� grafy pro hled�n� cesty
	CSFindPathGraph::PersistentSaveFindPathGraphs ( storage );

// ulo�� cesty

	// ulo�� cesty
	CSPath::PersistentSavePathes ( storage );

// ulo�� �et�zce hry

	// ulo�� �et�zce hry
	g_StringTable.PersistentSave ( storage );

// ulo�� statick� data

	// ulo�� statick� data
	CSOneInstance::PersistentSave ( storage );
}

// nahr�v�n� pouze ulo�en�ch dat, preferuje archiv mapy "strMapArchiveName", je-li 
//		"strMapArchiveName" pr�zdn�, pou�ije ulo�en� jm�no archivu mapy (FALSE=�patn� 
//		verze archivu mapy, v "strMapArchiveName" vrac� ulo�en� jm�no archivu mapy
BOOL CSMap::PersistentLoad ( CPersistentStorage &storage, CString &strMapArchiveName ) 
{
	ASSERT ( CheckEmptyData () );

//	BRACE_BLOCK(storage); TADY NE kvuli return FALSE;

	try
	{
		// ulo�en� ukazatel
		void *pSavedPointer;

	// na�te z�kladn� informace o h�e

		// na�te ��slo TimeSlicu
		storage >> m_dwTimeSlice;

	// na�te z�kladn� informace o map�

		// na�te velikost mapy
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

		// vytvo�� pole MapSquar�
		m_pMapSquares = new CSMapSquare[m_sizeMapMapSquare.cx * m_sizeMapMapSquare.cy];

		// d�lka jm�na archivu mapy
		int nMapArchiveNameSize;
		// na�te d�lku jm�na archivu mapy
		storage >> nMapArchiveNameSize;
		LOAD_ASSERT ( nMapArchiveNameSize > 0 );
		// z�sk� ukazatel na na��tan� jm�no archivu mapy
		char *szMapArchiveName = m_strMapArchiveName.GetBufferSetLength ( nMapArchiveNameSize );
		// na�te jm�no archivu mapy
		storage.Read ( szMapArchiveName, nMapArchiveNameSize );
		// zkontroluje d�lku jm�na archivu mapy
		for ( int nCharacterIndex = nMapArchiveNameSize; nCharacterIndex-- > 0; )
		{
			// zkontroluje dal�� znak jm�na archivu mapy
			LOAD_ASSERT ( szMapArchiveName[nCharacterIndex] != 0 );
		}
		// ukon�� na��t�n� jm�na archivu mapy
		m_strMapArchiveName.ReleaseBuffer ( nMapArchiveNameSize );

		// na�te verzi form�tu mapy
		storage >> m_dwMapFormatVersion;
		// na�te kompatabiln� verzi form�tu mapy
		storage >> m_dwMapCompatibleFormatVersion;

		ASSERT ( m_strMapName.IsEmpty () );
		// d�lka jm�na mapy
		int nMapNameSize;
		// na�te d�lku jm�na mapy
		storage >> nMapNameSize;
		// zjist�, je-li jm�no mapy pr�zdn�
		if ( nMapNameSize > 0 )
		{	// jm�no mapy nen� pr�zdn�
			// z�sk� ukazatel na na��tan� jm�no mapy
			char *szMapName = m_strMapName.GetBufferSetLength ( nMapNameSize );
			// na�te jm�no mapy
			storage.Read ( szMapName, nMapNameSize );
			// zkontroluje d�lku jm�na mapy
			for ( int nCharacterIndex = nMapNameSize; nCharacterIndex-- > 0; )
			{
				// zkontroluje dal�� znak jm�na mapy
				LOAD_ASSERT ( szMapName[nCharacterIndex] != 0 );
			}
			// ukon�� na��t�n� jm�na mapy
			m_strMapName.ReleaseBuffer ( nMapNameSize );
		}

		ASSERT ( m_strMapDescription.IsEmpty () );
		// d�lka popisu mapy
		int nMapDescriptionSize;
		// na�te d�lku popisu mapy
		storage >> nMapDescriptionSize;
		// zjist�, je-li popis mapy pr�zdn�
		if ( nMapDescriptionSize > 0 )
		{	// popis mapy nen� pr�zdn�
			// z�sk� ukazatel na na��tan� popis mapy
			char *szMapDescription = m_strMapDescription.GetBufferSetLength ( nMapDescriptionSize );
			// na�te popis mapy
			storage.Read ( szMapDescription, nMapDescriptionSize );
			// zkontroluje d�lku popisu mapy
			for ( int nCharacterIndex = nMapDescriptionSize; nCharacterIndex-- > 0; )
			{
				// zkontroluje dal�� znak popisu mapy
				LOAD_ASSERT ( szMapDescription[nCharacterIndex] != 0 );
			}
			// ukon�� na��t�n� popisu mapy
			m_strMapDescription.ReleaseBuffer ( nMapDescriptionSize );
		}

		// na�te verzi mapy
		storage >> m_dwMapVersion;

		// na�te po�et civilizac�
		storage >> m_dwCivilizationCount;
		LOAD_ASSERT ( m_dwCivilizationCount > 0 );
		LOAD_ASSERT ( m_dwCivilizationCount <= CIVILIZATION_COUNT_MAX );

	// na�te informace z mapy

		// archiv mapy
		CDataArchive cMapArchive;

		// zjist�, je-li jm�no preferovan�ho archivu mapy platn�
		if ( strMapArchiveName.IsEmpty () )
		{	// jm�no preferovan�ho archivu mapy nen� platn�
			// pou�ije ulo�en� jm�no archivu mapy
			strMapArchiveName = m_strMapArchiveName;
		}

		// pokus� se otev��t archiv mapy
		if ( !g_cSFileManager.OpenMap ( cMapArchive, m_strMapArchiveName, 
			m_dwMapVersion ) )
		{	// nepoda�ilo se otev��t archiv mapy
			// vypln� jm�no archivu mapy
			strMapArchiveName = m_strMapArchiveName;
			// vr�t� p��znak �patn� verze archivu mapy
			return FALSE;
		}

	// na�te pot�ebn� kus mapy
		{
		// na�te data mapy ze souboru mapy

			// otev�e soubor mapy
			CArchiveFile cMapFile = cMapArchive.CreateFile ( MAP_FILE_NAME, 
				CFile::modeRead | CFile::shareDenyWrite );

			// po�et knihoven Mapex�
			DWORD dwMapexLibraryCount;
			// po�et knihoven jednotek
			DWORD dwUnitTypeLibraryCount;
			// po�et jednotek na map�
			DWORD dwUnitCount;
			// po�et ScriptSet�
			DWORD dwScriptSetCount;
			// po�et graf� pro hled�n� cest
			DWORD dwFindPathGraphCount;

		// na�te hlavi�ku verze souboru
			{
				SFileVersionHeader sFileVersionHeader;
				LOAD_ASSERT ( cMapFile.Read ( &sFileVersionHeader, sizeof ( sFileVersionHeader ) ) == sizeof ( sFileVersionHeader ) );

				// identifik�tor souboru mapy
				BYTE aMapFileID[16] = MAP_FILE_IDENTIFIER;
				ASSERT ( sizeof ( aMapFileID ) == sizeof ( sFileVersionHeader.m_aMapFileID ) );

				// zkontroluje identifik�tor souboru mapy
				for ( int nIndex = 16; nIndex-- > 0; )
				{
					LOAD_ASSERT ( aMapFileID[nIndex] == sFileVersionHeader.m_aMapFileID[nIndex] );
				}

				ASSERT ( CURRENT_MAP_FILE_VERSION >= COMPATIBLE_MAP_FILE_VERSION );
				// zkontroluje verze form�tu mapy
				LOAD_ASSERT ( sFileVersionHeader.m_dwFormatVersion == m_dwMapFormatVersion );
				LOAD_ASSERT ( sFileVersionHeader.m_dwCompatibleFormatVersion == m_dwMapCompatibleFormatVersion );
				LOAD_ASSERT ( m_dwMapFormatVersion >= m_dwMapCompatibleFormatVersion );

				// zjist�, jedn�-li se o spr�vnou verzi form�tu mapy
				if ( m_dwMapFormatVersion != CURRENT_MAP_FILE_VERSION )
				{	// nejedn� se o spr�vnou verzi form�tu mapy
					// zjist�, jedn�-li se o starou verzi form�tu mapy
					if ( m_dwMapFormatVersion < CURRENT_MAP_FILE_VERSION )
					{	// jedn� se o starou verzi form�tu mapy
						// zkontroluje kompatabilitu verze form�tu mapy
						LOAD_ASSERT ( m_dwMapFormatVersion >= COMPATIBLE_MAP_FILE_VERSION );
					}
					else
					{	// jedn� se o mlad�� verzi form�tu mapy
						// zkontroluje kompatabilitu verze form�tu mapy
						LOAD_ASSERT ( m_dwMapCompatibleFormatVersion <= CURRENT_MAP_FILE_VERSION );
					}
				}
			}

		// na�te hlavi�ku mapy
			{
				SMapHeader sMapHeader;
				LOAD_ASSERT ( cMapFile.Read ( &sMapHeader, sizeof ( sMapHeader ) ) == sizeof ( sMapHeader ) );

			// zpracuje hlavi�ku mapy

				// zkontroluje velikost mapy
				LOAD_ASSERT ( sMapHeader.m_dwWidth == m_sizeMapMapCell.cx );
				LOAD_ASSERT ( sMapHeader.m_dwHeight == m_sizeMapMapCell.cy );

				// zkontroluje jm�no mapy
				LOAD_ASSERT ( m_strMapName == sMapHeader.m_pName );
				LOAD_ASSERT ( m_strMapDescription == sMapHeader.m_pDescription );

				// zkontroluje po�et knihoven Mapex�
				dwMapexLibraryCount = sMapHeader.m_dwMapexLibraryCount;
				LOAD_ASSERT ( dwMapexLibraryCount > 0 );

				// zkontroluje po�et civilizac�
				LOAD_ASSERT ( m_dwCivilizationCount == sMapHeader.m_dwCivilizationsCount );

				// zkontroluje po�et knihoven typ� jednotek
				dwUnitTypeLibraryCount = sMapHeader.m_dwUnitTypeLibraryCount;
				LOAD_ASSERT ( dwUnitTypeLibraryCount > 0 );

				// zjist� po�et jednotek na map�
				dwUnitCount = sMapHeader.m_dwUnitCount;

				// zkontroluje verzi mapy
				LOAD_ASSERT ( m_dwMapVersion == sMapHeader.m_dwMapVersion );

				// zjist� po�et ScriptSet�
				dwScriptSetCount = sMapHeader.m_dwScriptSetCount;
				LOAD_ASSERT ( dwScriptSetCount > 0 );

				// zjist� po�et graf� pro hled�n� cest
				dwFindPathGraphCount = sMapHeader.m_dwFindPathGraphCount;
			}

		// nahraje hlavi�ky ScriptSet�
			{
				SScriptSetHeader sScriptSetHeader;

				// tabulka p�ekladu ID na jm�no ScriptSetu
				CMap<DWORD, DWORD, CString, CString &> cScriptSetTable;

				for ( DWORD dwIndex = dwScriptSetCount; dwIndex-- > 0; )
				{
					// pokus� se na��st hlavi�ku ScriptSetu
					LOAD_ASSERT ( cMapFile.Read ( &sScriptSetHeader, sizeof ( sScriptSetHeader ) ) == sizeof ( sScriptSetHeader ) );

					// zkontroluje d�lku jm�na ScriptSetu
					int nScriptSetNameLength = strlen ( sScriptSetHeader.m_pFileName );
					LOAD_ASSERT ( ( nScriptSetNameLength > 0 ) && ( nScriptSetNameLength < 
						sizeof ( sScriptSetHeader.m_pFileName ) ) );

					// jm�no nalezen�ho ScriptSetu
					CString strFoundScriptSetName;
					// ID nalezen�ho ScriptSetu
					DWORD dwFoundScriptSetID;
					// pozice ScriptSetu v tabulce ScriptSet�
					POSITION posScriptSet = cScriptSetTable.GetStartPosition ();
					// jm�no ScriptSetu
					CString strScriptSetName ( sScriptSetHeader.m_pFileName );

					// zkontroluje unik�tnost ID a jm�na ScriptSetu
					while ( posScriptSet != NULL )
					{
						// zjist� informace o dal��m ScriptSetu v tabulce
						cScriptSetTable.GetNextAssoc ( posScriptSet, dwFoundScriptSetID, 
							strFoundScriptSetName );

						// zkontroluje unik�tnost ID a jm�na ScriptSetu
						LOAD_ASSERT ( ( strFoundScriptSetName.CompareNoCase ( 
							strScriptSetName ) != 0 ) && sScriptSetHeader.m_dwID != 
							dwFoundScriptSetID );
					}

					// p�id� z�znam o ScriptSetu do tabulky ScriptSet�
					cScriptSetTable.SetAt ( sScriptSetHeader.m_dwID, strScriptSetName );

					// nech� nahr�t ScriptSet
					g_cSFileManager.LoadScriptSet ( cMapArchive, strScriptSetName );
				}
			}

		// p�esko�� odkazy na knihovny Mapex�, hlavi�ky civilizac�, odkazy na knihovny 
		//		typ� jednotek a pole offset� MapSquar�
			{
				// zjist� velikost p�eskakovan�ch dat
				DWORD dwSkippedDataSize = 
					dwMapexLibraryCount * sizeof ( SMapexLibraryNode ) + 
					m_dwCivilizationCount * sizeof ( SCivilizationHeader ) + 
					dwUnitTypeLibraryCount * sizeof ( SUnitTypeLibraryNode ) + 
					m_sizeMapMapSquare.cx * m_sizeMapMapSquare.cy * sizeof ( DWORD );

				// p�esko�� data
				LOAD_ASSERT ( cMapFile.GetPosition () + dwSkippedDataSize <= 
					cMapFile.GetLength () );
				(void)cMapFile.Seek ( dwSkippedDataSize, CFile::current );
			}

		// na�te informace o MapSquarech
			{
				// index MapSquaru
				CPointDW pointIndex;

				// na�te MapSquary
				for ( pointIndex.y = 0; pointIndex.y < m_sizeMapMapSquare.cy; pointIndex.y++ )
				{
					for ( pointIndex.x = 0; pointIndex.x < m_sizeMapMapSquare.cx; 
						pointIndex.x++ )
					{
						// na�te dal�� MapSquare
						GetMapSquareFromIndex ( pointIndex )->Create ( cMapFile );
					}
				}
			}

		// p�esko�� hlavi�ky jednotek
			{
				// zjist� velikost hlavi�ek jednotek
				DWORD dwUnitHeaderSize = dwUnitCount * sizeof ( SUnitHeader );

				// p�esko�� hlavi�ky jednotek
				LOAD_ASSERT ( cMapFile.GetPosition () + dwUnitHeaderSize <= 
					cMapFile.GetLength () );
				(void)cMapFile.Seek ( dwUnitHeaderSize, CFile::current );
			}

		// na�te jm�na surovin
			{
				SResource sResource;

				// zjist� velikost ikony suroviny
				DWORD dwResourceIconSize = RESOURCE_ICON_WIDTH * 
					RESOURCE_ICON_HEIGHT * sizeof ( DWORD );

				// projede suroviny
				for ( DWORD dwResourceIndex = 0; dwResourceIndex < RESOURCE_COUNT; 
					dwResourceIndex++ )
				{
					// na�te surovinu
					LOAD_ASSERT ( cMapFile.Read ( &sResource, sizeof ( sResource ) ) == sizeof ( sResource ) );

					// zkontroluje d�lku jm�na suroviny
					m_aResourceName[dwResourceIndex] = sResource.m_szName;
					LOAD_ASSERT ( m_aResourceName[dwResourceIndex].GetLength () < sizeof ( sResource.m_szName ) );

					// p�esko�� ikonu suroviny
					LOAD_ASSERT ( cMapFile.GetPosition () + dwResourceIconSize <= 
						cMapFile.GetLength () );
					(void)cMapFile.Seek ( dwResourceIconSize, CFile::current );
				}
			}

		// na�te jm�na neviditelnost�
			{
				SInvisibility sInvisibility;

				// projede neviditelnosti
				for ( DWORD dwInvisibilityIndex = 0; 
					dwInvisibilityIndex < INVISIBILITY_COUNT; dwInvisibilityIndex++ )
				{
					// na�te neviditelnost
					LOAD_ASSERT ( cMapFile.Read ( &sInvisibility, sizeof ( sInvisibility ) ) == sizeof ( sInvisibility ) );

					// zkontroluje d�lku jm�na neviditelnosti
					m_aInvisibilityName[dwInvisibilityIndex] = sInvisibility.m_szName;
					LOAD_ASSERT ( m_aInvisibilityName[dwInvisibilityIndex].GetLength () < sizeof ( sInvisibility.m_szName ) );
				}
			}

		// na�te hlavi�ky graf� pro hled�n� cesty
			{
				SFindPathGraphHeader sFindPathGraphHeader;

				// vytvo�� pole graf� pro hled�n� cesty
				CSFindPathGraph::PreCreateFindPathGraphs ( dwFindPathGraphCount );

				// na�te hlavi�ky graf� pro hled�n� cesty
				for ( ; dwFindPathGraphCount-- > 0; )
				{
					// na�te hlavi�ku grafu pro hled�n� cest
					LOAD_ASSERT ( cMapFile.Read ( &sFindPathGraphHeader, sizeof ( sFindPathGraphHeader ) ) == sizeof ( sFindPathGraphHeader ) );

					// zkontroluje d�lku jm�na grafu pro hled�n� cesty
					int nFindPathGraphNameLength = strlen ( sFindPathGraphHeader.m_pName );
					LOAD_ASSERT ( ( nFindPathGraphNameLength > 0 ) && 
						( nFindPathGraphNameLength < 
						sizeof ( sFindPathGraphHeader.m_pName ) ) );

					// vytvo�� graf pro hled�n� cesty
					(void)CSFindPathGraph::CreateFindPathGraph ( dwFindPathGraphCount, 
						sFindPathGraphHeader.m_dwID, sFindPathGraphHeader.m_pName, 
						cMapArchive );
				}
			}

		// ukon�� na��t�n� souboru mapy

			// zkontroluje konec souboru
			LOAD_ASSERT ( cMapFile.GetPosition () == cMapFile.GetLength () );
			cMapFile.Close ();

		// ukon�� vytv��en� graf� pro hled�n� cesty

			// ukon�� vytv��en� graf� pro hled�n� cesty
			CSFindPathGraph::PostCreateFindPathGraphs ();

		// vytvo�� data cest

			// vytvo�� data cest
			CSPath::CreatePathes ( cMapArchive );

		// vytvo�� mapy jednotek

			// vytvo�� mapy jednotek
			CSMapSquare::CreateUnitMaps ();
		}

	// na�te civilizace

		// vytvo�� pole civilizac�
		m_pCivilizations = new CSCivilization[m_dwCivilizationCount];

		// zjist� ukazatel na prvn� civilizaci
		CSCivilization *pCivilization = m_pCivilizations;

		// projede v�echny civilizace
		for ( DWORD dwCivilizationIndex = 0; dwCivilizationIndex < m_dwCivilizationCount; 
			dwCivilizationIndex++, pCivilization++ )
		{
			// na�te star� ukazatel na civilizaci
			storage >> pSavedPointer;
			LOAD_ASSERT ( pSavedPointer != NULL );
			// zaregistruje ukazatel na civilizaci
			storage.RegisterPointer ( pSavedPointer, pCivilization );

			// na�te civilizaci
			pCivilization->PersistentLoad ( storage );

			// inicializuje index civilizace
			pCivilization->SetCivilizationIndex ( dwCivilizationIndex );

			// zjist�, jedn�-li se o syst�movou civilizaci
			if ( dwCivilizationIndex == 0 )
			{	// jedn� se o syst�movou civilizaci
				// zkontroluje na�ten� syst�mov� civilizace
				LOAD_ASSERT ( pCivilization->GetCivilizationID () == 0 );
			}

			// zkontroluje unik�tnost jm�na a ID civilizace
			for ( DWORD dwIndex = 0; dwIndex < dwCivilizationIndex; dwIndex++ )
			{
				// zkontroluje unik�tnost jm�na civilizace
				LOAD_ASSERT ( pCivilization->GetCivilizationName () != m_pCivilizations[dwIndex].GetCivilizationName () );
				// zkontroluje unik�tnost ID civilizace
				LOAD_ASSERT ( pCivilization->GetCivilizationID () != m_pCivilizations[dwIndex].GetCivilizationID () );
			}

		// na�te DPC frontu civilizace

			// veliksot DPC fronty civilizace
			DWORD dwDPCCount;
			// na�te velikost DPC fronty civilizace
			storage >> dwDPCCount;

			ASSERT ( m_aDPCQueue[dwCivilizationIndex].IsEmpty () );

			// na�te DPC frontu civilizace
			for ( ; dwDPCCount-- > 0; )
			{
				// na�te star� ukazatel na DPC
				storage >> pSavedPointer;
				LOAD_ASSERT ( pSavedPointer != NULL );

				// DPC ID
				DWORD dwDPCID;
				// na�te DPC ID
				storage >> dwDPCID;

				// vytvo�� DPC z ID
				CSDeferredProcedureCall *pDPC = CSDeferredProcedureCall::CreateChildByID ( dwDPCID );
				LOAD_ASSERT ( pDPC != NULL );

				// zaregistruje ukazatel na DPC
				storage.RegisterPointer ( pSavedPointer, pDPC );

				// na�te DPC
				pDPC->PersistentLoad ( storage );

				// p�id� DPC do DPC fronty civilizace
				m_aDPCQueue[dwCivilizationIndex].Add ( pDPC );
			}
		}

		// vytvo�� memory pool pol� posledn�ch pozic, kde byla jednotka civilizacemi vid�na
		CSUnit::m_cCivilizationLastSeenPositionArrayMemoryPool.Create ( 
			max ( m_dwCivilizationCount * sizeof ( CSPosition ), sizeof ( BYTE * ) ) );

	// na�te jednotky na map�

		// po�et jednotek
		DWORD dwUnitCount;
		// na�te po�et jednotek
		storage >> dwUnitCount;

#ifdef _DEBUG
		// uschov� ID MainLoop threadu
		DWORD dwMainLoopThreadID = m_dwMainLoopThreadID;
		// nasimuluje MainLoopThread
		m_dwMainLoopThreadID = GetCurrentThreadId ();
#endif //_DEBUG

		// projede v�echny ulo�en� jednotky
		for ( ; dwUnitCount > 0; dwUnitCount-- )
		{
			// vytvo�� jednotku
			CSUnit *pUnit = new CSUnit;
			// na�te star� ukazatel na jednotku
			storage >> pSavedPointer;
			LOAD_ASSERT ( pSavedPointer != NULL );
			// zaregistruje ukazatel na jednotku
			storage.RegisterPointer ( pSavedPointer, pUnit );

			// na�te jednotku
			pUnit->PersistentLoad ( storage, m_dwCivilizationCount );

			// p�id� vytvo�enou jednotku do jednotek mapy
			AddUnit ( pUnit );
		}

#ifdef _DEBUG
		// vr�t� uschovan� ID MainLoop threadu
		m_dwMainLoopThreadID = dwMainLoopThreadID;
#endif //_DEBUG

	// na�te frontu mrtv�ch jednotek

		// po�et mrtv�ch jednotek
		DWORD dwDeadUnitCount;
		// na�te po�et mrtv�ch jednotek
		storage >> dwDeadUnitCount;

		// na�te frontu mrtv�ch jednotek
		for ( ; dwDeadUnitCount-- > 0; )
		{
			// popis mrtv� jednotky
			struct SDeadUnit sDeadUnit;

			// na�te popis mrtv� jednotky
			storage >> sDeadUnit.dwTimeSlice;
			// vytvo�� jednotku
			sDeadUnit.pUnit = new CSUnit;
			// na�te star� ukazatel na jednotku
			storage >> pSavedPointer;
			LOAD_ASSERT ( pSavedPointer != NULL );
			// zaregistruje ukazatel na jednotku
			storage.RegisterPointer ( pSavedPointer, sDeadUnit.pUnit );

			// na�te jednotku
			sDeadUnit.pUnit->PersistentLoad ( storage, m_dwCivilizationCount );

			// inicializuje jednotce ukazatel na z�mek
			sDeadUnit.pUnit->m_pLock = &m_cDeadDeletedUnitLock;

			// p�id� na�tenou jednotku do fronty mrtv�ch jednotek
			m_cDeadUnitQueue.Add ( sDeadUnit );
		}

	// na�te seznam smazan�ch jednotek

		// po�et smazan�ch jednotek
		DWORD dwDeletedUnitCount;

		// na�te po�et smazan�ch jednotek
		storage >> dwDeletedUnitCount;

		// na�te seznam smazan�ch jednotek
		for ( ; dwDeletedUnitCount-- > 0; )
		{
			// vytvo�� jednotku
			CSUnit *pUnit = new CSUnit;
			// na�te star� ukazatel na jednotku
			storage >> pSavedPointer;
			LOAD_ASSERT ( pSavedPointer != NULL );
			// zaregistruje ukazatel na jednotku
			storage.RegisterPointer ( pSavedPointer, pUnit );

			// na�te jednotku
			pUnit->PersistentLoad ( storage, m_dwCivilizationCount );

			// inicializuje jednotce ukazatel na z�mek
			pUnit->m_pLock = &m_cDeadDeletedUnitLock;

			// p�id� na�tenou jednotku do seznamu smazan�ch jednotek
			m_cDeletedUnitList.Add ( pUnit );
		}

	// na�te pole u�ivatel�

		// na�te po�et u�ivatel�
		DWORD dwUserCount;
		storage >> dwUserCount;

		// projede pole u�ivatel�
		for ( DWORD dwIndex = dwUserCount; dwIndex-- > 0; )
		{
			// vytvo�� nov� popis u�ivatele
			struct SUserDescription *pUserDescription = new SUserDescription;

			// d�lka jm�na u�ivatele
			int nUserNameSize;
			// na�te d�lku jm�na u�ivatele
			storage >> nUserNameSize;
			LOAD_ASSERT ( nUserNameSize > 0 );
			// z�sk� ukazatel na na��tan� jm�no u�ivatele
			char *szUserName = pUserDescription->strName.GetBufferSetLength ( nUserNameSize );
			// na�te jm�no u�ivatele
			storage.Read ( szUserName, nUserNameSize );
			// zkontroluje d�lku jm�na u�ivatele
			for ( int nCharacterIndex = nUserNameSize; nCharacterIndex-- > 0; )
			{
				// zkontroluje dal�� znak jm�na u�ivatele
				LOAD_ASSERT ( szUserName[nCharacterIndex] != 0 );
			}
			// ukon�� na��t�n� jm�na u�ivatele
			pUserDescription->strName.ReleaseBuffer ( nUserNameSize );

			// d�lka hesla u�ivatele
			int nUserPasswordSize;
			// na�te d�lku hesla u�ivatele
			storage >> nUserPasswordSize;
			// zjist�, je-li heslo u�ivatele pr�zdn�
			if ( nUserPasswordSize > 0 )
			{	// heslo u�ivatele nen� pr�zdn�
				// z�sk� ukazatel na na��tan� heslo u�ivatele
				char *szUserPassword = pUserDescription->strPassword.GetBufferSetLength ( nUserPasswordSize );
				// na�te heslo u�ivatele
				storage.Read ( szUserPassword, nUserPasswordSize );
				// zkontroluje d�lku hesla u�ivatele
				for ( int nCharacterIndex = nUserPasswordSize; nCharacterIndex-- > 0; )
				{
					// zkontroluje dal�� znak hesla u�ivatele
					LOAD_ASSERT ( szUserPassword[nCharacterIndex] != 0 );
				}
				// ukon�� na��t�n� hesla u�ivatele
				pUserDescription->strPassword.ReleaseBuffer ( nUserPasswordSize );
			}

			// na�te index civilizace
			storage >> pUserDescription->dwCivilizationIndex;
			LOAD_ASSERT ( pUserDescription->dwCivilizationIndex > 0 );
			LOAD_ASSERT ( pUserDescription->dwCivilizationIndex < m_dwCivilizationCount );

			// na�te p��znak povolen�
			storage >> pUserDescription->bEnabled;

			pUserDescription->bLoggedOn = FALSE;

			// p�id� u�ivatele do tabulky u�ivatel�
			m_aUserDescriptions.InsertAt ( 0, pUserDescription );
		}

	// na�te grafy pro hled�n� cesty

		// na�te grafy pro hled�n� cesty
		CSFindPathGraph::PersistentLoadFindPathGraphs ( storage );

	// na�te cesty

		// na�te cesty
		CSPath::PersistentLoadPathes ( storage, m_dwCivilizationCount );

	// na�te �et�zce hry

		// na�te �et�zce hry
		g_StringTable.PersistentLoad ( storage );

	// na�te statick� data

		// na�te statick� data
		CSOneInstance::PersistentLoad ( storage );
	}
	catch ( CException * )
	{
		// zne�kodn� �aste�n� inicializovan� data mapy
		SetEmptyData ();

		// pokra�uje ve zpracov�v�n� v�jimky
		throw;
	}

	// vr�t� p��znak �sp�n�ho na�ten� mapy
	return TRUE;
}

// p�eklad ukazatel�
void CSMap::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	try
	{
	// p�eklad ukazatel� civilizac�

		// zjist� ukazatel na prvn� civilizaci
		CSCivilization *pCivilization = m_pCivilizations;

		// projede v�echny civilizace
		for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; 
			pCivilization++ )
		{
			// p�elo�� ukazatele civilizace
			pCivilization->PersistentTranslatePointers ( storage );

		// p�elo�� ukazatele DPC fronty civilizace

			// zjist� pozici prvn�ho prvku v DPC front� civilizace
			POSITION posDPC = m_aDPCQueue[dwCivilizationIndex].GetHeadPosition ();
			// ukazatel na DPC
			CSDeferredProcedureCall *pDPC;

			// projede DPC frontu civilizace
			while ( CSDeferredProcedureCallQueue::GetNext ( posDPC, pDPC ) )
			{
				// p�elo�� ukazatele DPC
				pDPC->PersistentTranslatePointers ( storage );
			}
		}

	// p�eklad ukazatel� jednotek

		// projede v�echny bloky jednotek
		for ( SUnitBlock *pBlock = m_pFirstUnitBlock; pBlock != NULL; pBlock = pBlock->pNext )
		{
			// ukazatel na jednotku v bloku
			CSUnit **pBlockUnit = pBlock->pUnits;
			// projede v�echny jednotky v bloku jednotek
			for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
			{
				// zjist�, je-li to platn� jednotka
				if ( *pBlockUnit != NULL )
				{	// je to platn� jednotka
					// p�elo�� ukazatele jednotky
					(*pBlockUnit)->PersistentTranslatePointers ( storage );
				}
			}
		}

	// p�eklad ukazatel� mrtv�ch jednotek

		// popis mrtv� jednotky
		struct SDeadUnit sDeadUnit;
		// pozice mrtv� jednotky ve front� mrtv�ch jednotek
		POSITION posDeadUnit = m_cDeadUnitQueue.GetHeadPosition ();

		// projede frontu mrtv�ch jednotek
		while ( m_cDeadUnitQueue.GetNext ( posDeadUnit, sDeadUnit ) )
		{
			// p�elo�� ukazatele mrtv� jednotky
			sDeadUnit.pUnit->PersistentTranslatePointers ( storage );
		}

	// p�eklad ukazatel� smazan�ch jednotek

		// ukazatel na smazanou jednotku
		CSUnit *pDeletedUnit;
		// pozice smazan� jednotky v seznamu smazan�ch jednotek
		POSITION posDeletedUnit = m_cDeletedUnitList.GetHeadPosition ();

		// projede seznam smazan�ch jednotek
		while ( m_cDeletedUnitList.GetNext ( posDeletedUnit, pDeletedUnit ) )
		{
			// p�elo�� ukazatele smazan� jednotky
			pDeletedUnit->PersistentTranslatePointers ( storage );
		}

	// p�eklad ukazatel� graf� pro hled�n� cesty

		// p�eklad ukazatel� graf� pro hled�n� cesty
		CSFindPathGraph::PersistentTranslatePointersFindPathGraphs ( storage );

	// p�eklad ukazatel� cest

		// p�eklad ukazatel� cest
		CSPath::PersistentTranslatePointersPathes ( storage );

	// p�eklad ukazatel� �et�zc� hry

		// p�elo�� ukazatele �et�zc� hry
		g_StringTable.PersistentTranslatePointers ( storage );

	// p�eklad ukazatel� statick�ch dat

		// p�eklad ukazatel� statick�ch dat
		CSOneInstance::PersistentTranslatePointers ( storage );
	}
	catch ( CException * )
	{
		// zne�kodn� �aste�n� inicializovan� data mapy
		SetEmptyData ();

		// pokra�uje ve zpracov�v�n� v�jimky
		throw;
	}
}

// inicializace nahran�ho objektu
void CSMap::PersistentInit () 
{
	try
	{
		// inicializuje index rohov�ho MapSquaru
		m_pointCornerMapSquare = CPointDW ( m_sizeMapMapSquare.cx - 1, 
			m_sizeMapMapSquare.cy - 1 );
		m_pointCornerMapCell = CPointDW ( m_sizeMapMapCell.cx - 1, 
			m_sizeMapMapCell.cy - 1 );

	// inicializace civilizac�

		// zjist� ukazatel na prvn� civilizaci
		CSCivilization *pCivilization = m_pCivilizations;

		// projede v�echny civilizace
		for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; 
			pCivilization++ )
		{
			// inicializuje civilizaci
			pCivilization->PersistentInit ();

		// inicializuje DPC frontu civilizace

			// zjist� pozici prvn�ho prvku v DPC front� civilizace
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

	// inicializace jednotek a um�st�n� jednotek na mapu

		// vytvo�� pole p��znak� objeven� pozic na map� pro metodu "GetNearestUnitPlace"
		m_cGetNearestUnitPlacePositionArray.Create ( m_sizeMapMapCell.cx, 
			m_sizeMapMapCell.cy, 2, 0 );

		// vytvo�� memory pool pol� viditelnosti jednotky civilizacemi
		CSUnit::m_cCivilizationVisibilityArrayMemoryPool.Create ( 
			max ( m_dwCivilizationCount * sizeof ( WORD ), sizeof ( BYTE * ) ) );

		// projede v�echny bloky jednotek
		for ( SUnitBlock *pBlock = m_pFirstUnitBlock; pBlock != NULL; pBlock = pBlock->pNext )
		{
			// ukazatel na jednotku v bloku
			CSUnit **pBlockUnit = pBlock->pUnits;
			// projede v�echny jednotky v bloku jednotek
			for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
			{
				// zjist�, je-li to platn� jednotka
				if ( *pBlockUnit != NULL )
				{	// je to platn� jednotka
					// inicializuje jednotku
					(*pBlockUnit)->PersistentInit ();
				}
			}
		}

#ifdef _DEBUG
		// uschov� ID MainLoop threadu
		DWORD dwMainLoopThreadID = m_dwMainLoopThreadID;
		// nasimuluje MainLoopThread
		m_dwMainLoopThreadID = GetCurrentThreadId ();
#endif //_DEBUG

		// projede v�echny bloky jednotek
		for ( pBlock = m_pFirstUnitBlock; pBlock != NULL; pBlock = pBlock->pNext )
		{
			// ukazatel na jednotku v bloku
			CSUnit **pBlockUnit = pBlock->pUnits;
			// projede v�echny jednotky v bloku jednotek
			for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
			{
				// zjist�, je-li to platn� jednotka
				if ( *pBlockUnit != NULL )
				{	// je to platn� jednotka
					// inicializuje seznam jednotkou vid�n�ch jednotek
					(*pBlockUnit)->PersistentInitSeenUnitList ();
					// zpracuje p�id�n� jednotky na mapu
					LOAD_ASSERT ( CanPlaceUnitOnce ( *pBlockUnit, 
						(*pBlockUnit)->GetPosition (), TRUE ) );
					UnitPlaced ( *pBlockUnit );
				}
			}
		}

#ifdef _DEBUG
		// vr�t� uschovan� ID MainLoop threadu
		m_dwMainLoopThreadID = dwMainLoopThreadID;
#endif //_DEBUG

	// inicilazace mrtv�ch jednotek

		// popis mrtv� jednotky
		struct SDeadUnit sDeadUnit;
		// pozice mrtv� jednotky ve front� mrtv�ch jednotek
		POSITION posDeadUnit = m_cDeadUnitQueue.GetHeadPosition ();

		// projede frontu mrtv�ch jednotek
		while ( m_cDeadUnitQueue.GetNext ( posDeadUnit, sDeadUnit ) )
		{
			// inicializuje mrtvou jednotku
			sDeadUnit.pUnit->PersistentInit ( TRUE );
		}

	// inicializace smazan�ch jednotek

		// ukazatel na smazanou jednotku
		CSUnit *pDeletedUnit;
		// pozice smazan� jednotky v seznamu smazan�ch jednotek
		POSITION posDeletedUnit = m_cDeletedUnitList.GetHeadPosition ();

		// projede seznam smazan�ch jednotek
		while ( m_cDeletedUnitList.GetNext ( posDeletedUnit, pDeletedUnit ) )
		{
			// inicializuje smazanou jednotku
			pDeletedUnit->PersistentInit ( TRUE );
		}

	// inicializace graf� pro hled�n� cesty

		// inicializace graf� pro hled�n� cesty
		CSFindPathGraph::PersistentInitFindPathGraphs ();

	// inicializace cest

		// inicializace cest
		CSPath::PersistentInitPathes ();

	// inicializace �et�zc� hry

		// inicializuje �et�zce hry
		g_StringTable.PersistentInit ();

	// inicializace statick�ch dat

		// inicializace statick�ch dat
		CSOneInstance::PersistentInit ();

	// inicializace notifikac�

		// inicializuje notifikace
		InitializeNotifications ( FALSE );

	// inicializace hry

		// inicializuje hru (vytvo�� zapausovanou hru)
		InitGame ();
	}
	catch ( CException * )
	{
		// zne�kodn� �aste�n� inicializovan� data mapy
		SetEmptyData ();

		// pokra�uje ve zpracov�v�n� v�jimky
		throw;
	}
}

//////////////////////////////////////////////////////////////////////
// Operace s mapou
//////////////////////////////////////////////////////////////////////

// definice typu pro alokaci pole ukazatel� na jednotku
typedef CSUnit *TSUnitPointer;

// vytvo�� nov� blok jednotek (lze volat jen z MainLoop threadu)
// v�jimky: CMemoryException
void CSMap::CreateUnitBlock () 
{
	ASSERT ( IsMainLoopThread () );

	ASSERT ( ( m_pFirstUnitBlock == NULL ) || ( m_pFirstUnitBlock->pPrevious == NULL ) );
	ASSERT ( ( m_pLastUnitBlock == NULL ) || ( m_pLastUnitBlock->pNext == NULL ) );

	// vytvo�� nov� blok jednotek
	SUnitBlock *pBlock = new SUnitBlock;

	// vytvo�� nov� pole ukazatel� na jednotky
	CSUnit **pUnitReference = pBlock->pUnits = new TSUnitPointer[UNITS_IN_BLOCK];

	// inicializuje odkazy na jednotky v bloku a jednotky p�id� do seznamu voln�ch jednotek
	for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pUnitReference++ )
	{
		// vynuluje informaci o jednotce
		*pUnitReference = NULL;

		// vytvo�� informaci o voln� jednotce
		SFreeUnit *pFreeUnit = new SFreeUnit;
		// vypln� informace o voln� jednotce
		pFreeUnit->nIndex = nIndex;
		pFreeUnit->pBlock = pBlock;
		pFreeUnit->pNext = m_pFirstFreeUnit;

		// p�id� volnou jednotku na za��tek spoj�ku voln�ch jednotek
		m_pFirstFreeUnit = pFreeUnit;
	}

	// inicializuje blok jednotek
	pBlock->pNext = m_pFirstUnitBlock;
	pBlock->pPrevious = NULL;

// p�id� blok jednotek do spoj�ku blok� jednotek

	// zjist�, je-li spoj�k pr�zdn�
	if ( m_pFirstUnitBlock == NULL )
	{	// spoj�k blok� jednotek je pr�zdn�
		ASSERT ( m_pLastUnitBlock == NULL );
		// aktualizuje ukazatel na posledn� blok jednotek
		m_pLastUnitBlock = pBlock;
	}
	else
	{	// spoj�k blok� jednotek nen� pr�zdn�
		// p�id� blok jednotek p�ed prvn� blok jednotek
		m_pFirstUnitBlock->pPrevious = pBlock;
	}
	// aktualizuje ukazatel na prvn� blok jednotek
	m_pFirstUnitBlock = pBlock;

// inicializuje ID privilegovan�ho threadu z�mku bloku jednotek
#ifdef _DEBUG
	// zjist�, je-li MainLoop thread spu�t�n
	if ( m_pMainLoopThread != NULL )
	{	// MainLoop thread je pu�t�n
		// inicializuje ID privilegovan�ho threadu z�mku bloku jednotek
		pBlock->cLock.SetPrivilegedThreadID ( m_dwMainLoopThreadID );
	}
#endif //_DEBUG
}

// p�id� jednotku "pUnit" do jednotek mapy a aktualizuje jej� ukazatel na z�mek
//		(lze volat jen z MainLoop threadu)
// v�jimky: CMemoryException
void CSMap::AddUnit ( CSUnit *pUnit ) 
{
	ASSERT ( IsMainLoopThread () );

	ASSERT ( pUnit != NULL );
	ASSERT ( pUnit->m_pLock == NULL );

	// zjist�, je-li n�jak� m�sto pro jednotku voln�
	if ( m_pFirstFreeUnit == NULL )
	{	// nen� voln� m�sto pro jednotku
		// nech� vytvo�it nov� blok jednotek
		CreateUnitBlock ();
	}

	ASSERT ( m_pFirstFreeUnit != NULL );
	ASSERT ( ( m_pFirstUnitBlock != NULL ) && ( m_pFirstUnitBlock->pPrevious == NULL ) );
	ASSERT ( ( m_pLastUnitBlock != NULL ) && ( m_pLastUnitBlock->pNext == NULL ) );

	ASSERT ( m_pFirstFreeUnit->pBlock->pUnits[m_pFirstFreeUnit->nIndex] == NULL );

	// vezme prvn� voln� m�sto pro jednotku
	SFreeUnit *pFreeUnit = m_pFirstFreeUnit;
	// um�st� jednotku na prvn� voln� m�sto
	pFreeUnit->pBlock->pUnits[pFreeUnit->nIndex] = pUnit;
	// aktualizuje ukazatel na z�mek jednotky
	pUnit->m_pLock = &pFreeUnit->pBlock->cLock;

	// aktualizuje prvn� volnou jednotku
	m_pFirstFreeUnit = pFreeUnit->pNext;
	// zni�� inforamci o voln� jednotce, kter� byla pr�v� obsazena
	delete pFreeUnit;
}

// sma�e jednotku "pUnit" z jednotek mapy (lze volat jen z MainLoop threadu)
// v�jimky: CMemoryException
void CSMap::DeleteUnit ( CSUnit *pUnit ) 
{
	ASSERT ( IsMainLoopThread () );

	ASSERT ( ( m_pFirstUnitBlock != NULL ) && ( m_pFirstUnitBlock->pPrevious == NULL ) );
	ASSERT ( ( m_pLastUnitBlock != NULL ) && ( m_pLastUnitBlock->pNext == NULL ) );

	// najde blok s mazanou jednotkou
	SUnitBlock *pBlock = m_pFirstUnitBlock;
	// prohled� spoj�k blok� jednotek
	while ( &pBlock->cLock != pUnit->m_pLock )
	{
		// prohled� dal�� blok jednotek
		pBlock = pBlock->pNext;
		ASSERT ( pBlock != NULL );
	}

	// najde mazanou jednotku v bloku
	CSUnit **pBlockUnit = pBlock->pUnits;
	// index jednotky v bloku
	int nIndex = 0;
	// prohled� pole jednotek v bloku
	while ( *pBlockUnit != pUnit )
	{
		// prohled� dal�� jednotku bloku
		pBlockUnit++;
		nIndex++;
		ASSERT ( nIndex < UNITS_IN_BLOCK );
	}

	// zamkne blok jednotek
	VERIFY ( pBlock->cLock.PrivilegedThreadWriterLock () );

	// vyma�e jednotku z bloku jednotek
	*pBlockUnit = NULL;

	// odemkne blok jednotek
	pBlock->cLock.PrivilegedThreadWriterUnlock ();

	// vytvo�� volnou jednotku
	SFreeUnit *pFreeUnit = new SFreeUnit;

	// inicializuje volnou jednotku
	pFreeUnit->pBlock = pBlock;
	pFreeUnit->nIndex = nIndex;
	pFreeUnit->pNext = m_pFirstFreeUnit;

	// p�id� volnou jednotku do spoj�ku voln�ch jednotek
	m_pFirstFreeUnit = pFreeUnit;
}

// vr�t� pole MapSquar�, na kter� jednotka vid� (mimo MainLoop thread mus� b�t jednotka 
//		zam�ena pro �ten�) (NULL=posledn� MapSquare) 
void CSMap::GetSeenMapSquares ( CSUnit *pUnit, CSMapSquare *(&aSeenMapSquares)[4] ) const 
{
	ASSERT ( pUnit != NULL );

	// zjist�, pozici jednotky na map�
	CPointDW pointPosition = pUnit->GetPosition ();

	// zjist�, je-li jednotka na map�
	if ( !IsMapPosition ( pointPosition ) )
	{	// jednotka nen� na map�
		// vynuluje pole vid�n�ch MapSquar�
		aSeenMapSquares[0] = NULL;
		return;
	}

	// index MapSquaru um�st�n� jednotky
	CPointDW pointIndex;
	// zjist� index MapSquaru um�st�n� jednotky a aktualizuje pozici jednotky na map� 
	//		na relativn� pozici v��i MapSquaru
	pointIndex.x = pointPosition.x / MAP_SQUARE_SIZE;
	pointPosition.x = pointPosition.x % MAP_SQUARE_SIZE;
	pointIndex.y = pointPosition.y / MAP_SQUARE_SIZE;
	pointPosition.y = pointPosition.y % MAP_SQUARE_SIZE;

	// p�id� MapSquare um�st�n� jednotky do vid�n�ch MapSquar�
	aSeenMapSquares[0] = GetMapSquareFromIndex ( pointIndex );
	// inicializuje index MapSquaru v poli MapSquar�
	int nIndex = 1;

	// index rohov� soused�c�ho MapSquaru
	CPointDW pointCornerIndex;
	// vzd�lenost od rohov� soused�c�ho MapSquaru
	CSizeDW sizeCornerDistance;

	// zjist�, je-li jednotka v horn� ��sti MapSquaru
	if ( pointPosition.y < ( MAP_SQUARE_SIZE / 2 ) )
	{	// jednotka je v horn� ��sti MapSquaru
		// zjist�, je-li MapSquare "nad" na map� a je-li vid�t
		if ( ( pointIndex.y != 0 ) && ( pointPosition.y < pUnit->GetViewRadius () ) )
		{	// MapSquare "nad" je na map� a je vid�t
			// p�id� MapSquare "nad" do vid�n�ch MapSquar�
			aSeenMapSquares[1] = GetMapSquareFromIndex ( CPointDW ( pointIndex.x, 
				pointCornerIndex.y = pointIndex.y - 1 ) );
			// aktualizuje index p��t�ho viditeln�ho MapSquaru
			nIndex = 2;
			// aktualizuje vzd�lenost od rohov� soused�c�ho MapSquaru
			sizeCornerDistance.cy = pointPosition.y;
		}
	}
	else
	{	// jednotka je v doln� ��sti MapSquaru
		// aktualizuje vzd�lenost od rohov� soused�c�ho MapSquaru
		sizeCornerDistance.cy = MAP_SQUARE_SIZE - pointPosition.y;
		// zjist�, je-li MapSquare "pod" na map� a je-li vid�t
		if ( ( pointIndex.y != m_pointCornerMapSquare.y ) && ( sizeCornerDistance.cy <= 
			pUnit->GetViewRadius () ) )
		{	// MapSquare "pod" je na map� a je vid�t
			// p�id� MapSquare "pod" do vid�n�ch MapSquar�
			aSeenMapSquares[1] = GetMapSquareFromIndex ( CPointDW ( pointIndex.x, 
				pointCornerIndex.y = pointIndex.y + 1 ) );
			// aktualizuje index p��t�ho viditeln�ho MapSquaru
			nIndex = 2;
		}
	}

	// zjist�, je-li jednotka v lev� ��sti MapSquaru
	if ( pointPosition.x < ( MAP_SQUARE_SIZE / 2 ) )
	{	// jednotka je v lev� ��sti MapSquaru
		// zjist�, je-li MapSquare "vlevo" na map� a je-li vid�t
		if ( ( pointIndex.x != 0 ) && ( pointPosition.x < pUnit->GetViewRadius () ) )
		{	// MapSquare "vlevo" je na map� a je vid�t
			// p�id� MapSquare "vlevo" do vid�n�ch MapSquar�
			aSeenMapSquares[nIndex++] = GetMapSquareFromIndex ( CPointDW ( 
				pointCornerIndex.x = pointIndex.x - 1, pointIndex.y ) );
			// aktualizuje vzd�lenost od rohov� soused�c�ho MapSquaru
			sizeCornerDistance.cx = pointPosition.x;
		}
	}
	else
	{	// jednotka je v prav� ��sti MapSquaru
		// aktualizuje vzd�lenost od rohov� soused�c�ho MapSquaru
		sizeCornerDistance.cx = MAP_SQUARE_SIZE - pointPosition.x;
		// zjist�, je-li MapSquare "vpravo" na map� a je-li vid�t
		if ( ( pointIndex.x != m_pointCornerMapSquare.x ) && ( sizeCornerDistance.cx <= 
			pUnit->GetViewRadius () ) )
		{	// MapSquare "vpravo" je na map� a je vid�t
			// p�id� MapSquare "vpravo" do vid�n�ch MapSquar�
			aSeenMapSquares[nIndex++] = GetMapSquareFromIndex ( CPointDW ( 
				pointCornerIndex.x = pointIndex.x + 1, pointIndex.y ) );
		}
	}

	// zjist�, m�-li p�id�vat rohov� soused�c� MapSquare
	if ( nIndex == 3 )
	{	// rohov� soused�c� MapSquare existuje a m��e b�t vid�t
		// zjist�, je-li rohov� soused�c� MapSquare vid�t
		if ( sizeCornerDistance.cx * sizeCornerDistance.cx + sizeCornerDistance.cy * 
			sizeCornerDistance.cy <= pUnit->GetViewRadiusSquare () )
		{	// rohov� soused�c� MapSquare je vid�t
			aSeenMapSquares[3] = GetMapSquareFromIndex ( pointCornerIndex );
		}
		else
		{	// rohov� MapSquare nen� vid�t
			// ukon�� pole MapSquar�
			aSeenMapSquares[3] = NULL;
		}
	}
	else
	{	// rohov� MapSquare nen� vid�t nebo neexistuje
		// ukon�� pole MapSquar�
		aSeenMapSquares[nIndex] = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Informace o h�e
//////////////////////////////////////////////////////////////////////

// zjist� jednotku podle ID jednotky (NULL=jednotka neexistuje)
CSUnit *CSMap::GetUnitByID ( DWORD dwID ) 
{
	// projede v�echny bloky jednotek (od konce)
	for ( SUnitBlock *pBlock = m_pLastUnitBlock; pBlock != NULL; 
		pBlock = pBlock->pPrevious )
	{
		// ukazatel na jednotku v bloku
		CSUnit **pBlockUnit = pBlock->pUnits;
		// zamkne blok jednotek pro �ten�
		VERIFY ( pBlock->cLock.ReaderLock () );
		// projede v�echny jednotky v bloku jednotek
		for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
		{
			// z�sk� ukazatel na jednotku
			CSUnit *pUnit = *pBlockUnit;

			// zjist�, je-li to platn� jednotka
			if ( pUnit == NULL )
			{	// nen� to platn� jednotka
				continue;
			}

			// zjist�, je-li to hledan� jednotka
			if ( pUnit->GetID () == dwID )
			{	// jedn� se o hledanou jednotku
				// odemkne zam�en� blok jednotek pro �ten�
				pBlock->cLock.ReaderUnlock ();
				// vr�t� ukazatel na hledanou jednotku
				return pUnit;
			}
		}
		// byly projety v�echny jednotky v bloku jednotek

		// odemkne zam�en� blok jednotek pro �ten�
		pBlock->cLock.ReaderUnlock ();
	}

	// vr�t� p��znak neexistence hledan� jednotky
	return NULL;
}

// pro civilizaci "dwCivilizationIndex" vypln� seznam "cUnitList" v�ech jednotek 
//		(vlastn�ch i nep��telsk�ch) v oblasti se st�edem "pointCenter" o polom�ru 
//		"dwRadius" (mus� b�t zam�eny MapSquary, "dwRadius" mus� b�t men�� ne� 
//		MAX_GET_UNITS_IN_AREA_RADIUS)
void CSMap::GetUnitsInAreaAll ( DWORD dwCivilizationIndex, CPointDW pointCenter, 
	DWORD dwRadius, CSUnitList &cUnitList ) 
{
	ASSERT ( cUnitList.IsEmpty () );
	ASSERT ( dwCivilizationIndex < m_dwCivilizationCount );
	ASSERT ( IsMapPosition ( pointCenter ) );
	ASSERT ( dwRadius <= MAX_GET_UNITS_IN_AREA_RADIUS );

	// index prvn�ho zpracov�van�ho MapSquaru
	CPointDW pointIndex = GetMapSquareIndex ( CPointDW ( ( pointCenter.x > dwRadius ) ? 
		( pointCenter.x - dwRadius ) : 0, ( pointCenter.y > dwRadius ) ? 
		( pointCenter.y - dwRadius ) : 0 ) );

	// index prav�ho doln�ho zpracov�van�ho MapSquaru
	CPointDW pointRightBottomIndex = GetMapSquareIndex ( CPointDW ( 
		min ( pointCenter.x + dwRadius, m_pointCornerMapCell.x ), 
		min ( pointCenter.y + dwRadius, m_pointCornerMapCell.y ) ) );

	// �tverec polom�ru oblasti
	DWORD dwRadiusSquare = dwRadius * dwRadius;

	// sou�adnice indexu prvn�ho zpracov�van�ho MapSquaru
	DWORD dwStartIndexX = pointIndex.x;
	
	// projede zpracov�van� MapSquary
	for ( ; pointIndex.y <= pointRightBottomIndex.y; pointIndex.y++ )
	{
		for ( pointIndex.x = dwStartIndexX ; pointIndex.x <= pointRightBottomIndex.x; 
			pointIndex.x++ )
		{
			// zpracov�van� MapSquare
			CSMapSquare *pMapSquare = GetMapSquareFromIndex ( pointIndex );

			// zjist� seznam jednotek v MapSquaru
			CSUnitList *pMapSquareUnitList = pMapSquare->GetUnitList ();

			// pozice jednotky v seznamu jednotek
			POSITION posUnit = pMapSquareUnitList->GetHeadPosition ();
			// ukazatel na jednotku
			CSUnit *pUnit;

			// projede seznam jednotek
			while ( CSUnitList::GetNext ( posUnit, pUnit ) )
			{
				ASSERT ( pUnit != NULL );
				// zjist�, je-li jednotka vid�na civilizac� "dwCivilizationIndex"
				if ( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 )
				{	// jednotka je viditeln� civilizac� "dwCivilizationIndex"
					// zjist�, je-li jednotka v oblasti
					if ( pointCenter.GetDistanceSquare ( pUnit->m_pointPosition ) <= 
						dwRadiusSquare )
					{	// jednotka je v oblasti
						// p�id� jednotku do v�sledn�ho seznamu jednotek
						cUnitList.Add ( pUnit );
					}
					// jednotka nen� v oblasti
				}
				// jednotka nen� civilizac� vid�na
			}
			// jednotky MapSquaru byly zpracov�ny
		}
	}
	// MapSquary byly zpracov�ny
}

// pro civilizaci "dwCivilizationIndex" vypln� seznam "cUnitList" nep��telsk�ch 
//		jednotek v oblasti se st�edem "pointCenter" o polom�ru "dwRadius" (mus� b�t 
//		zam�eny MapSquary, "dwRadius" mus� b�t men�� ne� MAX_GET_UNITS_IN_AREA_RADIUS)
void CSMap::GetUnitsInAreaEnemy ( DWORD dwCivilizationIndex, CPointDW pointCenter, 
	DWORD dwRadius, CSUnitList &cUnitList ) 
{
	ASSERT ( cUnitList.IsEmpty () );
	ASSERT ( dwCivilizationIndex < m_dwCivilizationCount );
	ASSERT ( IsMapPosition ( pointCenter ) );
	ASSERT ( dwRadius <= MAX_GET_UNITS_IN_AREA_RADIUS );

	// index prvn�ho zpracov�van�ho MapSquaru
	CPointDW pointIndex = GetMapSquareIndex ( CPointDW ( ( pointCenter.x > dwRadius ) ? 
		( pointCenter.x - dwRadius ) : 0, ( pointCenter.y > dwRadius ) ? 
		( pointCenter.y - dwRadius ) : 0 ) );

	// index prav�ho doln�ho zpracov�van�ho MapSquaru
	CPointDW pointRightBottomIndex = GetMapSquareIndex ( CPointDW ( 
		min ( pointCenter.x + dwRadius, m_pointCornerMapCell.x ), 
		min ( pointCenter.y + dwRadius, m_pointCornerMapCell.y ) ) );

	// �tverec polom�ru oblasti
	DWORD dwRadiusSquare = dwRadius * dwRadius;

	// sou�adnice indexu prvn�ho zpracov�van�ho MapSquaru
	DWORD dwStartIndexX = pointIndex.x;
	
	// projede zpracov�van� MapSquary
	for ( ; pointIndex.y <= pointRightBottomIndex.y; pointIndex.y++ )
	{
		for ( pointIndex.x = dwStartIndexX ; pointIndex.x <= pointRightBottomIndex.x; 
			pointIndex.x++ )
		{
			// zpracov�van� MapSquare
			CSMapSquare *pMapSquare = GetMapSquareFromIndex ( pointIndex );

			// zjist� seznam jednotek v MapSquaru
			CSUnitList *pMapSquareUnitList = pMapSquare->GetUnitList ();

			// pozice jednotky v seznamu jednotek
			POSITION posUnit = pMapSquareUnitList->GetHeadPosition ();
			// ukazatel na jednotku
			CSUnit *pUnit;

			// projede seznam jednotek
			while ( CSUnitList::GetNext ( posUnit, pUnit ) )
			{
				ASSERT ( pUnit != NULL );
				// zjist�, jedn�-li se o nep��telskou jednotku
				if ( pUnit->GetCivilizationIndex () != dwCivilizationIndex )
				{	// jedn� se o nep��telskou jednotku
					// zjist�, je-li jednotka vid�na civilizac� "dwCivilizationIndex"
					if ( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 )
					{	// jednotka je viditeln� civilizac� "dwCivilizationIndex"
						// zjist�, je-li jednotka v oblasti
						if ( pointCenter.GetDistanceSquare ( pUnit->m_pointPosition ) <= 
							dwRadiusSquare )
						{	// jednotka je v oblasti
							// p�id� jednotku do v�sledn�ho seznamu jednotek
							cUnitList.Add ( pUnit );
						}
						// jednotka nen� v oblasti
					}
					// jednotka nen� civilizac� vid�na
				}
				// nejedn� se o nep��telskou jednotku
			}
			// jednotky MapSquaru byly zpracov�ny
		}
	}
	// MapSquary byly zpracov�ny
}

// pro civilizaci "dwCivilizationIndex" vypln� seznam "cUnitList" vlastn�ch jednotek 
//		v oblasti se st�edem "pointCenter" o polom�ru "dwRadius" (mus� b�t zam�eny 
//		MapSquary, "dwRadius" mus� b�t men�� ne� MAX_GET_UNITS_IN_AREA_RADIUS)
void CSMap::GetUnitsInAreaOfMine ( DWORD dwCivilizationIndex, CPointDW pointCenter, 
	DWORD dwRadius, CSUnitList &cUnitList ) 
{
	ASSERT ( cUnitList.IsEmpty () );
	ASSERT ( dwCivilizationIndex < m_dwCivilizationCount );
	ASSERT ( IsMapPosition ( pointCenter ) );
	ASSERT ( dwRadius <= MAX_GET_UNITS_IN_AREA_RADIUS );

	// index prvn�ho zpracov�van�ho MapSquaru
	CPointDW pointIndex = GetMapSquareIndex ( CPointDW ( ( pointCenter.x > dwRadius ) ? 
		( pointCenter.x - dwRadius ) : 0, ( pointCenter.y > dwRadius ) ? 
		( pointCenter.y - dwRadius ) : 0 ) );

	// index prav�ho doln�ho zpracov�van�ho MapSquaru
	CPointDW pointRightBottomIndex = GetMapSquareIndex ( CPointDW ( 
		min ( pointCenter.x + dwRadius, m_pointCornerMapCell.x ), 
		min ( pointCenter.y + dwRadius, m_pointCornerMapCell.y ) ) );

	// �tverec polom�ru oblasti
	DWORD dwRadiusSquare = dwRadius * dwRadius;

	// sou�adnice indexu prvn�ho zpracov�van�ho MapSquaru
	DWORD dwStartIndexX = pointIndex.x;
	
	// projede zpracov�van� MapSquary
	for ( ; pointIndex.y <= pointRightBottomIndex.y; pointIndex.y++ )
	{
		for ( pointIndex.x = dwStartIndexX ; pointIndex.x <= pointRightBottomIndex.x; 
			pointIndex.x++ )
		{
			// zpracov�van� MapSquare
			CSMapSquare *pMapSquare = GetMapSquareFromIndex ( pointIndex );

			// zjist� seznam jednotek v MapSquaru
			CSUnitList *pMapSquareUnitList = pMapSquare->GetUnitList ();

			// pozice jednotky v seznamu jednotek
			POSITION posUnit = pMapSquareUnitList->GetHeadPosition ();
			// ukazatel na jednotku
			CSUnit *pUnit;

			// projede seznam jednotek
			while ( CSUnitList::GetNext ( posUnit, pUnit ) )
			{
				ASSERT ( pUnit != NULL );
				// zjist�, jedn�-li se o vlastn� jednotku
				if ( pUnit->GetCivilizationIndex () == dwCivilizationIndex )
				{	// jedn� se o vlastn� jednotku
					ASSERT ( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 );
					// zjist�, je-li jednotka v oblasti
					if ( pointCenter.GetDistanceSquare ( pUnit->m_pointPosition ) <= 
						dwRadiusSquare )
					{	// jednotka je v oblasti
						// p�id� jednotku do v�sledn�ho seznamu jednotek
						cUnitList.Add ( pUnit );
					}
					// jednotka nen� v oblasti
				}
				// nejedn� se o vlastn� jednotku
			}
			// jednotky MapSquaru byly zpracov�ny
		}
	}
	// MapSquary byly zpracov�ny
}

//////////////////////////////////////////////////////////////////////
// Operace s jednotkou mapy
//////////////////////////////////////////////////////////////////////

// pohne s jednotkou "pUnit" na pozici "pointPosition" - pouze pohyb jednotky po map�, 
//		nikoli mimo mapu, vol�no pro ka�d� pohyb jednotky (jednotka mus� b�t zam�ena 
//		pro z�pis, je-li nastaven p��znak "bMapSquaresLocked", nezamyk� MapSquary, lze 
//		volat jen z MainLoop threadu) (TRUE=jednotka byla p�em�st�na)
BOOL CSMap::MoveUnit ( CSUnit *pUnit, CPointDW pointPosition, BOOL bMapSquaresLocked ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );
	ASSERT ( pUnit != NULL );
	ASSERT ( IsMapPosition ( pUnit->GetPosition () ) );
	ASSERT ( IsMapPosition ( pointPosition ) );

	// graf pro hled�n� cesty pro jednotku
	CSFindPathGraph *pFindPathGraph = pUnit->GetFindPathGraph ();

	// zjist�, lze-li jednotka um�stit na mapu
	if ( ( ( pFindPathGraph != NULL ) && ( pFindPathGraph->GetMapCellAt ( 
		pointPosition ) < (signed char)( pUnit->GetMoveWidth () * 2 ) ) ) || 
		!CanPlaceUnitOnce ( pUnit, pointPosition, bMapSquaresLocked ) )
	{	// nelze um�stit jednotku na mapu
		// vr�t� p��znak nep�em�st�n� jednotky
		return FALSE;
	}

	// zjist� index MapSquaru nov� polohy jednotky
	CPointDW pointNewMapSquareIndex ( pointPosition.x / MAP_SQUARE_SIZE, 
		pointPosition.y / MAP_SQUARE_SIZE );
	// zjist� index MapSquaru star� polohy jednotky
	CPointDW pointOldMapSquareIndex = pUnit->GetPosition ();
	pointOldMapSquareIndex.x /= MAP_SQUARE_SIZE;
	pointOldMapSquareIndex.y /= MAP_SQUARE_SIZE;

	// nastav� novou pozici jednotky
	pUnit->SetPosition ( pointPosition );
	// nastav� p��znak pohybu jednotky
	pUnit->SetPositionChanged ();

	// zjist�, pohnula-li se jednotka z MapSquaru
	if ( pointOldMapSquareIndex != pointNewMapSquareIndex )
	{	// jednotka se pohnula z MapSquaru
	// p�epoj� jednotku mezi MapSquary a aktualizuje informace MapSquar�

		// zjist� star� MapSquare
		CSMapSquare *pOldMapSquare = GetMapSquareFromIndex ( pointOldMapSquareIndex );
		// zjist� nov� MapSquare
		CSMapSquare *pNewMapSquare = GetMapSquareFromIndex ( pointNewMapSquareIndex );

		// zjist�, jsou-li zam�eny MapSquary
		if ( !bMapSquaresLocked )
		{	// MapSquary nejsou zam�eny
			// zamkne MapSquary
			VERIFY ( m_mutexMapSquaresLock.Lock () );
		}

		// aktualizuje zm�nu pozice jednotky z/do sledovan�ch MapSquar�
		pUnit->m_cWatchedMapSquareChanged |= pNewMapSquare->m_cWatchingCivilizations ^ 
			pOldMapSquare->m_cWatchingCivilizations;

		// zjist�, seznam jednotek star�ho MapSquaru
		CSUnitList *pOldUnitList = pOldMapSquare->GetUnitList ();
		// zjist�, seznam jednotek nov�ho MapSquaru
		CSUnitList *pNewUnitList = pNewMapSquare->GetUnitList ();

		// najde jednotku ve star�m MapSquaru
		POSITION posUnit = pOldUnitList->Find ( pUnit );
		ASSERT ( posUnit != NULL );
		ASSERT ( !pOldUnitList->IsEmpty ( posUnit ) );

		// p�esune jednotku do nov�ho MapSquaru
		pOldUnitList->Move ( posUnit, *pNewUnitList );

		// aktualizuje nov� MapSquare po p�id�n� jednotky
		pNewMapSquare->UnitInserted ( pUnit );
		// aktualizuje star� MapSquare po odebr�n� jednotky
		pOldMapSquare->UnitDeleted ( pUnit );

		// zjist�, byly-li zam�eny MapSquary
		if ( !bMapSquaresLocked )
		{	// MapSquary nebyly zam�eny
			// odemkne MapSquary
			VERIFY ( m_mutexMapSquaresLock.Unlock () );

		}
	}

	// vr�t� p��znak p�em�st�n� jednotky
	return TRUE;
}

// odebere jednotku "pUnit" z mapy (jednotka mus� b�t zam�ena pro z�pis, je-li nastaven 
//		p��znak "bMapSquaresLocked", nezamyk� MapSquary, lze volat jen z MainLoop threadu)
void CSMap::RemoveUnit ( CSUnit *pUnit, BOOL bMapSquaresLocked ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );
	ASSERT ( pUnit != NULL );
	ASSERT ( IsMapPosition ( pUnit->GetPosition () ) );

	// zjist� MapSquare um�st�n� jednotky
	CSMapSquare *pMapSquare = GetMapSquareFromPosition ( pUnit->GetPosition () );

	// zjist�, jsou-li zam�eny MapSquary
	if ( !bMapSquaresLocked )
	{	// MapSquary nejsou zam�eny
		// zamkne MapSquary
		VERIFY ( m_mutexMapSquaresLock.Lock () );
	}

	// um�st� jednotku mimo mapu
	pUnit->SetPosition ( CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ) );
	pUnit->SetFirstCheckPoint ( CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ), 0 );
	pUnit->SetSecondCheckPoint ( CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ), 0 );
	// nastav� p��znak pohybu jednotky
	pUnit->SetPositionChanged ();
	pUnit->SetCheckPointInfoModified ();
	// aktualizuje zm�nu pozice jednotky z/do sledovan�ch MapSquar�
	pUnit->m_cWatchedMapSquareChanged |= pMapSquare->m_cWatchingCivilizations;

	// index civilizace jednotky
	DWORD dwCivilizationIndex = pUnit->GetCivilizationIndex ();
	// seznam jednotkou vid�n�ch jednotek
	CSUnitList *pUnitList = pUnit->GetSeenUnitList ();
	// ukazatel na jednotku na civilizaci
	CZUnit *pZUnit = pUnit->GetZUnit ();
	// jednotkou vid�n� jednotka
	CSUnit *pSeenUnit;
	while ( pUnitList->RemoveFirst ( pSeenUnit ) )
	{
		ASSERT ( pSeenUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 );
		// sn�� po�et jednotek sv� civilizace, kter� vid�nou jednotku vid�
		if ( --pSeenUnit->m_aCivilizationVisibility[dwCivilizationIndex] == 0 )
		{	// jednotka zmizela civilizaci z dohledu
			// nastav� p��znak zm�ny viditelnosti jednotky civilizac�
			pSeenUnit->m_cCivilizationVisibilityChanged.SetAt ( dwCivilizationIndex );
			// naposledy vid�n� pozice jednotky, kter� p�estala b�t vid�t
			CSPosition *pPosition = pSeenUnit->m_aCivilizationLastSeenPosition + 
				dwCivilizationIndex;
			// zamkne jednotku, kter� p�estala b�t vid�t na z�pis
			VERIFY ( pSeenUnit->PrivilegedThreadWriterLock () );
			// aktualizuje naposledy vid�nou pozici jednotky civilizac�
			*(CPointDW *)pPosition = pSeenUnit->m_pointPosition;
			pPosition->z = pSeenUnit->m_dwVerticalPosition;
			ASSERT ( pPosition->z != 0 );
			// odemkne jednotku, kter� p�estala b�t vid�t na z�pis
			pSeenUnit->PrivilegedThreadWriterUnlock ();
		}
		// zjist�, jedn�-li se o nep��telskou jednotku
		if ( pSeenUnit->GetCivilizationIndex () != 0 )
		{	// jedn� se o nep��telskou jednotku
			// po�le jednotce notifikaci o ukon�en� viditelnosti nep��telsk� jednotky
			SendNotification ( pZUnit, NOTIFICATION_ENEMY_UNIT_DISAPPEARED, pSeenUnit );
		}
		else
		{	// jedn� se o syst�movou jednotku
			// po�le jednotce notifikaci o ukon�en� viditelnosti syst�mov� jednotky
			SendNotification ( pZUnit, NOTIFICATION_SYSTEM_UNIT_DISAPPEARED, pSeenUnit );
		}
	}

	// zamkne seznam mrtv�ch jednotek pro rozes�l�n� stop infa klient�m
	VERIFY ( m_mutexDeadUnitClientStopInfoListLock.Lock () );

	// zv��� po�et odkaz� na jednotku
	pUnit->AddRef ();
	// p�id� do seznamu mrtv�ch jednotek pro rozes�l�n� stop infa klient�m mrtvou jednotku
	m_cDeadUnitClientStopInfoList.Add ( pUnit );

	// odemkne seznam mrtv�ch jednotek pro rozes�l�n� stop infa klient�m
	VERIFY ( m_mutexDeadUnitClientStopInfoListLock.Unlock () );

	// odebere jednotku z MapSquaru
	pMapSquare->DeleteUnit ( pUnit );

	// zjist�, byly-li zam�eny MapSquary
	if ( !bMapSquaresLocked )
	{	// MapSquary nebyly zam�eny
		// odemkne MapSquary
		VERIFY ( m_mutexMapSquaresLock.Unlock () );
	}
}

// um�st� jednotku "pUnit" na pozici "pointPosition" (jednotka mus� b�t zam�ena 
//		pro z�pis, je-li nastaven p��znak "bMapSquaresLocked", nezamyk� MapSquary, lze 
//		volat jen z MainLoop threadu) (TRUE=jednotka byla um�st�na)
BOOL CSMap::PlaceUnit ( CSUnit *pUnit, CPointDW pointPosition, BOOL bMapSquaresLocked ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );
	ASSERT ( pUnit != NULL );
	ASSERT ( !IsMapPosition ( pUnit->GetPosition () ) );
	ASSERT ( IsMapPosition ( pointPosition ) );

	// zjist� MapSquare um�st�n� jednotky
	CSMapSquare *pMapSquare = GetMapSquareFromPosition ( pointPosition );

	// zjist�, jsou-li zam�eny MapSquary
	if ( !bMapSquaresLocked )
	{	// MapSquary nejsou zam�eny
		// zamkne MapSquary
		VERIFY ( m_mutexMapSquaresLock.Lock () );
	}

	// graf pro hled�n� cesty pro jednotku
	CSFindPathGraph *pFindPathGraph = pUnit->GetFindPathGraph ();

	// zjist�, lze-li jednotka um�stit na mapu
	if ( ( ( pFindPathGraph != NULL ) && ( pFindPathGraph->GetMapCellAt ( 
		pointPosition ) < (signed char)( pUnit->GetMoveWidth () * 2 ) ) ) || 
		!CanPlaceUnitOnce ( pUnit, pointPosition, TRUE ) )
	{	// nelze um�stit jednotku na mapu
		// zjist�, byly-li zam�eny MapSquary
		if ( !bMapSquaresLocked )
		{	// MapSquary nebyly zam�eny
			// odemkne MapSquary
			VERIFY ( m_mutexMapSquaresLock.Unlock () );
		}
		// vr�t� p��znak neum�st�n� jednotky na mapu
		return FALSE;
	}

	// um�st� jednotku mimo mapu
	pUnit->SetPosition ( pointPosition );
	// nastav� p��znak pohybu jednotky
	pUnit->SetPositionChanged ();
	// aktualizuje zm�nu pozice jednotky z/do sledovan�ch MapSquar�
	pUnit->m_cWatchedMapSquareChanged |= pMapSquare->m_cWatchingCivilizations;

	// vlo�� jednotku do MapSquaru
	pMapSquare->InsertUnit ( pUnit );

	// zjist�, byly-li zam�eny MapSquary
	if ( !bMapSquaresLocked )
	{	// MapSquary nebyly zam�eny
		// odemkne MapSquary
		VERIFY ( m_mutexMapSquaresLock.Unlock () );
	}

	// vr�t� p��znak um�st�n� jednotky na mapu
	return TRUE;
}

// zpracuje um�st�n� jednotky "pUnit" na mapu (MapSquary mus� b�t zam�eny)
void CSMap::UnitPlaced ( CSUnit *pUnit ) 
{
	ASSERT ( pUnit != NULL );
	ASSERT ( IsMapPosition ( pUnit->GetPosition () ) );
	ASSERT ( CanPlaceUnitOnce ( pUnit, pUnit->GetPosition (), TRUE ) );

	// zjist� MapSquare um�st�n� jednotky
	CSMapSquare *pMapSquare = GetMapSquareFromPosition ( pUnit->GetPosition () );

	// nastav� p��znak pohybu jednotky
	pUnit->SetPositionChanged ();
	// aktualizuje zm�nu pozice jednotky z/do sledovan�ch MapSquar�
	pUnit->m_cWatchedMapSquareChanged |= pMapSquare->m_cWatchingCivilizations;

	// vlo�� jednotku do MapSquaru
	pMapSquare->InsertUnit ( pUnit );
}

// vr�t� nejbli��� um�st�n� jednotky "pUnit" na pozici "pointPosition" (jednotka mus� b�t 
//		zam�ena pro �ten�, MapSquary mus� b�t zam�eny, lze volat jen z MainLoop threadu) 
//		(NO_MAP_POSITION - jednotku nelze um�stit)
CPointDW CSMap::GetNearestUnitPlace ( CSUnit *pUnit, CPointDW pointPosition ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );

	ASSERT ( pUnit != NULL );
	ASSERT ( IsMapPosition ( pointPosition ) );

	// pozice um�st�n� jednotky
	CPointDW pointPlacedPosition ( NO_MAP_POSITION, NO_MAP_POSITION );

	// fronta pozic na map�
	static CSelfPooledQueue<CPointDW> cPositionQueue ( 100 );
	ASSERT ( cPositionQueue.IsEmpty () );

	// graf pro hled�n� cesty pro jednotku
	CSFindPathGraph *pFindPathGraph = pUnit->GetFindPathGraph ();
	// ID MapCellu ���ky jednotky
	signed char cUnitWidthMapCellID = (signed char)( pUnit->GetMoveWidth () * 2 );

	// nejv�t�� po�et zkou�en�ch pozic
	DWORD dwMaxPositionCount = 2500;

	// zjist�, jedn�-li se o pr�zdn� graf pro hled�n� cesty
	if ( pFindPathGraph == NULL )
	{	// jedn� se o pr�zdn� graf pro hled�n� cesty
		// zjist�, jedn�-li se o vzdu�nou jednotku
		if ( pUnit->GetVerticalPosition () >= AIR_VERTICAL_POSITION )
		{	// jedn� se o vzdu�nou jednotku
			// vr�t� po�adovanou pozici jednotky
			return pointPosition;
		}
		// jedn� se o pozemn� jednotku

		// p�id� do fronty pozic na map� po��te�n� pozici um�st�n� jednotky
		cPositionQueue.Add ( pointPosition );

		// pokus� se um�stit jednotku
		while ( cPositionQueue.RemoveFirst ( pointPosition ) )
		{
			// zjist� jedn�-li se o posledn� zkou�enou pozici
			if ( dwMaxPositionCount-- == 0 )
			{	// jedn� se o posledn� zkou�enou pozici
				// ukon�� um�s�ov�n� jednotky
				break;
			}

			// zjist�, lze-li um�stit jednotku na pozici
			if ( CSMapSquare::MainLoopCanPlaceUnit ( pUnit, pointPosition, TRUE ) )
			{	// jednotku lze um�stit na pozici
				// uschov� pozici um�st�n� jednotky
				pointPlacedPosition = pointPosition;
				// ukon�� um�s�ov�n� jednotky
				break;
			}

			// vyzkou�� okoln� pozice na map�
			for ( DWORD dwSurroundingPositionIndex = 8; dwSurroundingPositionIndex-- > 0; )
			{
				// sousedn� pozice na map�
				CPointDW pointSurroundingPosition = pointPosition + 
					CSMapSquare::m_aSurroundingPositionOffset[dwSurroundingPositionIndex];

				// zjist�, jedn�-li se o pozici na map�
				if ( ( pointSurroundingPosition.x < m_sizeMapMapCell.cx ) && 
					( pointSurroundingPosition.y < m_sizeMapMapCell.cy ) )
				{	// jedn� se o pozici na map�
					// p��znak objeven� pozice na map�
					BYTE &nPosition = m_cGetNearestUnitPlacePositionArray.GetAt ( 
						pointSurroundingPosition );
					// zjist�, byla-li pozice ji� objevena
					if ( nPosition == 0 )
					{	// pozice dosud nebyla objevena
						// ozna�� pozici za objevenou
						nPosition = 1;
						// p�id� pozici do fronty pozic
						cPositionQueue.Add ( pointSurroundingPosition );
					}
					// pozice ji� byla objevena
				}
				// nejedn� se o pozici na map�
			}
			// jednotku nelze um�stit na pozici
		}
		// pozemn� jednotku s pr�zdn�m grafem pro hled�n� cesty se poda�ilo um�stit nebo ji 
		//		um�stit nelze

		// ukon�� um�s�ov�n� jednotky
		CSMapSquare::MainLoopFinishPlacingUnit ();
	}
	else
	{	// jedn� se o nepr�zdn� graf pro hled�n� cesty
		// p�id� do fronty pozic na map� po��te�n� pozici um�st�n� jednotky
		cPositionQueue.Add ( pointPosition );

		// zjist�, jedn�-li se o vzdu�nou jednotku
		if ( pUnit->GetVerticalPosition () >= AIR_VERTICAL_POSITION )
		{	// jedn� se o vzdu�nou jednotku
			// pokus� se um�stit jednotku
			while ( cPositionQueue.RemoveFirst ( pointPosition ) )
			{
				// zjist� jedn�-li se o posledn� zkou�enou pozici
				if ( dwMaxPositionCount-- == 0 )
				{	// jedn� se o posledn� zkou�enou pozici
					// ukon�� um�s�ov�n� jednotky
					break;
				}

				// zjist�, lze-li um�stit jednotku na pozici
				if ( pFindPathGraph->GetMapCellAt ( pointPosition ) >= cUnitWidthMapCellID )
				{	// jednotku lze um�stit na pozici
					// uschov� pozici um�st�n� jednotky
					pointPlacedPosition = pointPosition;
					// ukon�� um�s�ov�n� jednotky
					break;
				}

				// vyzkou�� okoln� pozice na map�
				for ( DWORD dwSurroundingPositionIndex = 8; dwSurroundingPositionIndex-- > 0; )
				{
					// sousedn� pozice na map�
					CPointDW pointSurroundingPosition = pointPosition + 
						CSMapSquare::m_aSurroundingPositionOffset[dwSurroundingPositionIndex];

					// zjist�, jedn�-li se o pozici na map�
					if ( ( pointSurroundingPosition.x < m_sizeMapMapCell.cx ) && 
						( pointSurroundingPosition.y < m_sizeMapMapCell.cy ) )
					{	// jedn� se o pozici na map�
						// p��znak objeven� pozice na map�
						BYTE &nPosition = m_cGetNearestUnitPlacePositionArray.GetAt ( 
							pointSurroundingPosition );
						// zjist�, byla-li pozice ji� objevena
						if ( nPosition == 0 )
						{	// pozice dosud nebyla objevena
							// ozna�� pozici za objevenou
							nPosition = 1;
							// p�id� pozici do fronty pozic
							cPositionQueue.Add ( pointSurroundingPosition );
						}
						// pozice ji� byla objevena
					}
					// nejedn� se o pozici na map�
				}
				// jednotku nelze um�stit na pozici
			}
			// vzdu�nou jednotku s nepr�zdn�m grafem pro hled�n� cesty se poda�ilo um�stit 
			//		nebo ji um�stit nelze
		}
		else
		{	// jedn� se o pozemn� jednotku
			// pokus� se um�stit jednotku
			while ( cPositionQueue.RemoveFirst ( pointPosition ) )
			{
				// zjist� jedn�-li se o posledn� zkou�enou pozici
				if ( dwMaxPositionCount-- == 0 )
				{	// jedn� se o posledn� zkou�enou pozici
					// ukon�� um�s�ov�n� jednotky
					break;
				}

				// zjist�, lze-li um�stit jednotku na pozici
				if ( ( pFindPathGraph->GetMapCellAt ( pointPosition ) >= 
					cUnitWidthMapCellID ) && CSMapSquare::MainLoopCanPlaceUnit ( pUnit, 
					pointPosition, TRUE ) )
				{	// jednotku lze um�stit na pozici
					// uschov� pozici um�st�n� jednotky
					pointPlacedPosition = pointPosition;
					// ukon�� um�s�ov�n� jednotky
					break;
				}

				// vyzkou�� okoln� pozice na map�
				for ( DWORD dwSurroundingPositionIndex = 8; dwSurroundingPositionIndex-- > 0; )

				{
					// sousedn� pozice na map�
					CPointDW pointSurroundingPosition = pointPosition + 
						CSMapSquare::m_aSurroundingPositionOffset[dwSurroundingPositionIndex];

					// zjist�, jedn�-li se o pozici na map�
					if ( ( pointSurroundingPosition.x < m_sizeMapMapCell.cx ) && 
						( pointSurroundingPosition.y < m_sizeMapMapCell.cy ) )
					{	// jedn� se o pozici na map�
						// p��znak objeven� pozice na map�
						BYTE &nPosition = m_cGetNearestUnitPlacePositionArray.GetAt ( 
							pointSurroundingPosition );
						// zjist�, byla-li pozice ji� objevena
						if ( nPosition == 0 )
						{	// pozice dosud nebyla objevena
							// ozna�� pozici za objevenou
							nPosition = 1;
							// p�id� pozici do fronty pozic
							cPositionQueue.Add ( pointSurroundingPosition );
						}
						// pozice ji� byla objevena
					}
					// nejedn� se o pozici na map�
				}
				// jednotku nelze um�stit na pozici
			}
			// pozemn� jednotku s nepr�zdn�m grafem pro hled�n� cesty se poda�ilo um�stit 
			//		nebo ji um�stit nelze

			// ukon�� um�s�ov�n� jednotky
			CSMapSquare::MainLoopFinishPlacingUnit ();
		}
		// jednotku s nepr�zdn�m grafem pro hled�n� cesty se poda�ilo um�stit nebo ji 
		//		um�stit nelze
	}
	// jednotku s pr�zdn�m i nepr�zdn�m grafem pro hled�n� cesty se poda�ilo um�stit nebo 
	//		ji um�stit nelze

	// zni�� frontu pozic na map�
	cPositionQueue.RemoveAll ();

	// sma�e pole p��znak� objeven� pozice na map�
	m_cGetNearestUnitPlacePositionArray.Clear ();

	// vr�t� pozici um�st�n� jednotky
	return pointPlacedPosition;
}

//////////////////////////////////////////////////////////////////////
// Operace ��zen� hry (TimeSlicy a pausov�n� hry)
//////////////////////////////////////////////////////////////////////

// zapausuje hru (po�k� na zapausov�n� hry)
void CSMap::PauseGame () 
{
	ASSERT ( CheckValidData () );

	// zkontroluje, nen�-li hra ji� zapausov�na
	ASSERT ( !m_eventPauseGame.Lock ( 0 ) );
	ASSERT ( m_eventUnpauseGame.Lock ( 0 ) );
	ASSERT ( !m_eventMainLoopPaused.Lock ( 0 ) );
	ASSERT ( !m_eventClientInfoSenderLoopPaused.Lock ( 0 ) );
	ASSERT ( !CSPath::m_eventFindPathLoopPaused.Lock ( 0 ) );

	// zamkne z�mek p��znaku povolen� logov�n� u�ivatel�
	VERIFY ( m_cUserLoginAllowedLock.WriterLock () );

	// zak�e logov�n� u�ivatel�
	m_bUserLoginAllowed = FALSE;

	// odemkne z�mek p��znaku povolen� logov�n� u�ivatel�
	m_cUserLoginAllowedLock.WriterUnlock ();

	// zru�� ud�lost odpausov�n� hry
	VERIFY ( m_eventUnpauseGame.ResetEvent () );
	// nastav� ud�lost zapausov�n� hry
	VERIFY ( m_eventPauseGame.SetEvent () );

	// po�et ud�lost� zapausov�n� thread� a civilizac�
	DWORD dwPausingEventsCount = 3 + m_dwCivilizationCount;
	// vytvo�� tabulku ud�lost� zapausov�n� thread� a civilizac�
	HANDLE *aPausingEvents = new HANDLE[dwPausingEventsCount];

	// inicializuje ud�losti zapausovan�ch thread�
	aPausingEvents[0] = (HANDLE)m_eventMainLoopPaused;
	aPausingEvents[1] = (HANDLE)m_eventClientInfoSenderLoopPaused;
	aPausingEvents[2] = (HANDLE)CSPath::m_eventFindPathLoopPaused;

	// nech� zapausovat jednotliv� civilizace
	for ( DWORD dwCivilizationIndex = 0; dwCivilizationIndex < m_dwCivilizationCount; 
		dwCivilizationIndex++ )
	{
		// nech� zapausovat civilizaci a p�id� ud�lost zapausov�n� civilizace do tabulky
		aPausingEvents[dwCivilizationIndex + 3] = 
			(HANDLE)*m_pCivilizations[dwCivilizationIndex].m_pZCivilization->Pause ();
	}

	// po�k� na zapausov�n� pausovan�ch thread� a civilizac�
	DWORD dwPausingResult = WaitForMultipleObjects ( dwPausingEventsCount, 
		aPausingEvents, TRUE, INFINITE );

	// zni�� tabulku ud�lost� zapausovan�ch thread� a civilizac�
	delete aPausingEvents;

	// zkontroluje v�sledek zapausov�n� thread� a civilizac�
	ASSERT ( ( dwPausingResult >= WAIT_OBJECT_0 ) && ( dwPausingResult < WAIT_OBJECT_0 + 
		dwPausingEventsCount ) );
	// thready i civilizace byly zapausov�ny
	ASSERT ( !m_eventMainLoopPaused.Lock ( 0 ) );
	ASSERT ( !m_eventClientInfoSenderLoopPaused.Lock ( 0 ) );
	ASSERT ( !CSPath::m_eventFindPathLoopPaused.Lock ( 0 ) );

	// hra je zapausov�na - ukon�� zapausov�n� hry
	return;
}

// odpausuje hru s p��znakem pokra�ov�n� hry "bContinueGame"
void CSMap::UnpauseGame ( BOOL bContinueGame ) 
{
	ASSERT ( CheckValidData () );

	// zkontroluje zapausov�n� hry
	ASSERT ( m_eventPauseGame.Lock ( 0 ) );
	ASSERT ( !m_eventUnpauseGame.Lock ( 0 ) );

	// aktualizuje p��znak pokra�ov�n� hry
	m_bContinueUnpausedGame = bContinueGame;

	// nech� odpausovat jednotliv� civilizace
	for ( DWORD dwCivilizationIndex = 0; dwCivilizationIndex < m_dwCivilizationCount; 
		dwCivilizationIndex++ )
	{
		// nech� odpausovat dal�� civilizaci
		m_pCivilizations[dwCivilizationIndex].m_pZCivilization->Unpause ( 
			bContinueGame != FALSE );
	}

	// zjist�, m�-li hra pokra�ovat
	if ( bContinueGame )
	{	// hra m� pokra�ovat
		// zamkne z�mek p��znaku povolen� logov�n� u�ivatel�
		VERIFY ( m_cUserLoginAllowedLock.WriterLock () );

		// povol� logov�n� u�ivatel�
		m_bUserLoginAllowed = TRUE;

		// odemkne z�mek p��znaku povolen� logov�n� u�ivatel�
		m_cUserLoginAllowedLock.WriterUnlock ();
	}

	// zru�� ud�lost zapausov�n� hry
	VERIFY ( m_eventPauseGame.ResetEvent () );
	// nastav� ud�lost odpausov�n� hry
	VERIFY ( m_eventUnpauseGame.SetEvent () );
}

// initializuje hru (vytvo�� zapausovanou hru)
void CSMap::InitGame () 
{
	// zkontroluje thready mapy
	ASSERT ( m_pMainLoopThread == NULL );
	ASSERT ( m_pClientInfoSenderLoopThread == NULL );
	ASSERT ( m_pFindPathLoopThread == NULL );

// inicializuje intern� data mapy

	// inicializuje ud�lost po�adavku sledov�n� MapSquaru
	VERIFY ( m_eventWatchingMapSquareRequest.ResetEvent () );

// inicializuje hru

	// inicializuje ud�losti MainLoop threadu
	VERIFY ( m_eventClientInfoSenderLoopTimeSliceFinished.ResetEvent () );

	// inicializuje ud�losti zapausov�n� thread�
	VERIFY ( CSPath::m_eventFindPathLoopPaused.ResetEvent () );
	VERIFY ( m_eventClientInfoSenderLoopPaused.ResetEvent () );
	VERIFY ( m_eventMainLoopPaused.ResetEvent () );

	// inicializuje zapausov�n� hry
	VERIFY ( m_eventUnpauseGame.ResetEvent () );
	VERIFY ( m_eventPauseGame.SetEvent () );

#ifdef _DEBUG
	// zne�kodn� ID MainLoop threadu
	m_dwMainLoopThreadID = DWORD_MAX;
#endif //_DEBUG

	// vytvo�� thready mapy
	m_pMainLoopThread = AfxBeginThread ( RunMainLoop, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL );
	LOAD_ASSERT ( m_pMainLoopThread != NULL );
	m_pClientInfoSenderLoopThread = AfxBeginThread ( RunClientInfoSenderLoop, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL );
	LOAD_ASSERT ( m_pClientInfoSenderLoopThread != NULL );
	m_pFindPathLoopThread = AfxBeginThread ( CSPath::FindPathLoop, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL );
	LOAD_ASSERT ( m_pFindPathLoopThread != NULL );

	// nastav� autodelete na FALSE
	m_pMainLoopThread->m_bAutoDelete = FALSE;
	m_pClientInfoSenderLoopThread->m_bAutoDelete = FALSE;
	m_pFindPathLoopThread->m_bAutoDelete = FALSE;

	// pust� vytvo�en� thready mapy
	m_pMainLoopThread->ResumeThread ();
	m_pClientInfoSenderLoopThread->ResumeThread ();
	m_pFindPathLoopThread->ResumeThread ();

	// tabulka pausovan�ch thread�
	HANDLE aPausingThreads[3] = { 
		(HANDLE)m_eventMainLoopPaused, 
		(HANDLE)m_eventClientInfoSenderLoopPaused, 
		(HANDLE)CSPath::m_eventFindPathLoopPaused, 
	};

	// po�k� na zapausov�n� pausovan�ch thread�
	switch ( WaitForMultipleObjects ( 3, aPausingThreads, TRUE, INFINITE ) )
	{
	// thready byly zapausov�ny
	case WAIT_OBJECT_0 :
	case ( WAIT_OBJECT_0 + 1 ) :
	case ( WAIT_OBJECT_0 + 2 ) :
		ASSERT ( !m_eventMainLoopPaused.Lock ( 0 ) );
		ASSERT ( !m_eventClientInfoSenderLoopPaused.Lock ( 0 ) );
		ASSERT ( !CSPath::m_eventFindPathLoopPaused.Lock ( 0 ) );
		// hra je zapausov�na - ukon�� zapausov�n� hry
		break;
	// thready se nepovedlo zapausovat
	default:
		ASSERT ( FALSE );
		return;
	}

	// nastav� p��znak inicializavan� mapy
	m_bInitialized = TRUE;
}

// spust� MainLoop mapy "pMap"
UINT CSMap::RunMainLoop ( LPVOID pMap ) 
{
	ASSERT ( pMap != NULL );

	// spust� MainLoop mapy "pMap"
	((CSMap *)pMap)->MainLoop ();

	// vr�t� p��znak �sp�chu
	return 0;
}

// spust� ClientInfoSenderLoop mapy "pMap"
UINT CSMap::RunClientInfoSenderLoop ( LPVOID pMap ) 
{
	ASSERT ( pMap != NULL );

	// spust� ClientInfoSenderLoop mapy "pMap"
	((CSMap *)pMap)->ClientInfoSenderLoop ();

	// vr�t� p��znak �sp�chu
	return 0;
}

// po�k� na za��tek nov�ho TimeSlicu (s timeoutem "dwTimeout")
BOOL CSMap::WaitForNewTimeSlice ( DWORD dwTimeout ) 
{
	// smy�ka �ek�n� na zpr�vu s timeoutem
	for ( ; ; )
	{
		// po�k� na timer s timoutem "dwTimeout"
		switch ( MsgWaitForMultipleObjects ( 0, NULL, FALSE, dwTimeout, QS_TIMER ) )
		{
		// zpr�va timeru
		case WAIT_OBJECT_0 :
			// zpr�va
			MSG sMessage;

			// pokus� se p�e��st z fronty zpr�v zpr�vu timeru
			if ( PeekMessage ( &sMessage, NULL, WM_TIMER, WM_TIMER, PM_REMOVE ) )
			{	// byla p�e�tena zpr�va timeru z fronty zpr�v
				ASSERT ( sMessage.message == WM_TIMER );
				// vr�t� p��znak p�e�ten� zpr�vy timeru
				return TRUE;
			}
			// nebyla z�sk�na ��dn� zpr�va timeru

			// pokra�uje v �ek�n� na zpr�vu timeru
			break;
		// timeout
		case WAIT_TIMEOUT :
			// vr�t� p��znak timeoutu
			return FALSE;
		// nemo�n� p��pad
		default :
			ASSERT ( FALSE );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Operace pro klienta civilizace
//////////////////////////////////////////////////////////////////////

// zaregistruje klienta civilizace "pClient" (m��e trvat dlouho - �ek� na dojet� 
//		rozes�l�n� informac� o jednotk�ch klient�m)
void CSMap::RegisterClient ( CZClientUnitInfoSender *pClient ) 
{
	ASSERT ( pClient != NULL );

	// zamkne seznamy zaregistrovan�ch klient� civilizac� pro z�pis
	VERIFY ( m_cRegisteredClientListLock.WriterLock () );

	ASSERT ( m_aRegisteredClientList[pClient->GetCivilizationIndex ()].FindClient ( pClient ) == NULL );
	// p�iprav� zaregistraci klienta civilizace
	struct SRegisteredClientInfo sClientInfo;
	sClientInfo.pClient = pClient;
	// vytvo�� pole sledovan�ch MapSquar�
	sClientInfo.pWatchedMapSquareArray = new CSWatchedMapSquareArray;
	// zaregistruje klienta civilizace
	m_aRegisteredClientList[pClient->GetCivilizationIndex ()].Add ( sClientInfo );

	TRACE_COMMUNICATION1 ( _T("#	RegisterClient ( Client=%x )\n"), pClient );

	// odemkne seznamy zaregistrovan�ch klient� civilizac� pro z�pis
	m_cRegisteredClientListLock.WriterUnlock ();
}

// odregistruje klienta civilizace "pClient" (m��e trvat dlouho - �ek� na dojet� 
//		rozes�l�n� informac� o jednotk�ch klient�m, b�hem vol�n� t�to metody NESM� b�t 
//		vol�ny metody s klientem "pClient", ale server m��e rozeslat je�t� n�kter� 
//		informace - klient je mus� stornovat) (odregistrov�n�m klienta civilizace 
//		ztr�c� server v�echny odkazy na klienta)
void CSMap::UnregisterClient ( CZClientUnitInfoSender *pClient ) 
{
	ASSERT ( pClient != NULL );

// zru�� po�adavky klienta civilizace na sledov�n� MapSquar�

	// zamkne frontu po�adavk� na sledov�n� MapSquar�
	VERIFY ( m_mutexWatchingMapSquareRequestQueueLock.Lock () );

	// sma�e z fronty po�adavk� na sledov�n� MapSquar� v�echny po�adavky klienta "pClient"
	m_cWatchingMapSquareRequestQueue.RemoveAll ( pClient );

	// odemkne frontu po�adavk� na sledov�n� MapSquar�
	VERIFY ( m_mutexWatchingMapSquareRequestQueueLock.Unlock () );

// zru�� po�adavky klienta civilizace na sledov�n� jednotek

	// zamkne frontu po�adavk� na sledov�n� jednotek
	VERIFY ( m_mutexWatchingUnitRequestQueueLock.Lock () );

	// zjist� pozici prvn�ho po�adavku na sledov�n� jednotky
	POSITION posRequest = m_cWatchingUnitRequestQueue.GetHeadPosition ();

	// informace o po�adavku na sledov�n� jednotky
	struct SWatchingUnitRequestInfo sRequestInfo;

	// schov� si pozici posledn�ho po�adavku
	POSITION posLastRequest = posRequest;

	// projede frontu po�adavk� na sledov�n� jednotek
	while ( CSWatchingUnitRequestQueue::GetNext ( posRequest, sRequestInfo ) )
	{
		// zjist�, jedn�-li to po�adavek klienta "pClient"
		if ( sRequestInfo.pClient == pClient )
		{	// jedn� se o po�adavek klienta "pClient"
			// sma�e po�adavek z fronty po�adavk� na sledov�n� jednotek
			(void)m_cWatchingUnitRequestQueue.RemovePosition ( posRequest );
			// aktualizuje pozici dal��ho po�adavku
			posRequest = posLastRequest;

			// zjist�, jedn�-li se o po�adavek za��tku sledov�n� jednotky
			if ( sRequestInfo.bStartWatchingUnit )
			{	// jedn� se o po�adavek za��tku sledov�n� jednotky
				// sn�� po�et odkaz� na jednotku
				sRequestInfo.pUnit->Release ();
			}
		}
		else
		{	// nejedn� se o po�adavek klienta "pClient"
			// aktualizuje pozici posledn� jednotky
			posLastRequest = posRequest;
		}
	}
	// fronta po�adavk� na sledov�n� jednotek byla projeta

	// odemkne frontu po�adavk� na sledov�n� jednotek
	VERIFY ( m_mutexWatchingUnitRequestQueueLock.Unlock () );

// zru�� jednotky sledovan� klientem civilizace

	// zamkne seznam sledovan�ch jednotek
	VERIFY ( m_mutexWatchedUnitListLock.Lock () );

	// z�sk� pozici prvn� sledovan� jednotky
	POSITION posUnit = m_cWatchedUnitList.GetHeadPosition ();

	// informace o sledovan� jednotce
	struct SWatchedUnitInfo sUnitInfo;

	// schov� si pozici posledn� jednotky
	POSITION posLastUnit = posUnit;

	// projede seznam sledovan�ch jednotek
	while ( m_cWatchedUnitList.GetNext ( posUnit, sUnitInfo ) )
	{
		// zjist�, je-li dal�� sledovan� jednotka sledov�na odregistrov�van�m klientem
		if ( sUnitInfo.pClient == pClient )
		{	// jednotka je sledov�na odregistrov�van�m klientem
			// aktualizuje pozici dal�� jednotky
			posUnit = posLastUnit;
			// sma�e jednotku ze seznamu sledovan�ch jednotek
			VERIFY ( m_cWatchedUnitList.RemovePosition ( posUnit ) == sUnitInfo );

			// sn�� po�et klient� sleduj�c�ch jednotku
			ASSERT ( sUnitInfo.pUnit->m_dwWatchingCivilizationClientCount > 0 );
			sUnitInfo.pUnit->m_dwWatchingCivilizationClientCount--;

			// sn�� po�et odkaz� na jednotku
			sUnitInfo.pUnit->Release ();
		}
		else
		{	// jednotka nen� sledov�na odregistrov�van�m klientem
			// aktualizuje pozici posledn� jednotky
			posLastUnit = posUnit;
		}
	}

	// odemkne seznam sledovan�ch jednotek
	VERIFY ( m_mutexWatchedUnitListLock.Unlock () );

// odregistruje klienta civilizace

	// zamkne seznamy zaregistrovan�ch klient� civilizac� pro z�pis
	VERIFY ( m_cRegisteredClientListLock.WriterLock () );

	// zjist� index civilizace klient�
	DWORD dwCivilizationIndex = pClient->GetCivilizationIndex ();

	// zjist� um�st�n� klienta v seznamu klient� civilizace
	POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].FindClient ( pClient );
	ASSERT ( posClient != NULL );

	// odregistruje klienta civilizace
	struct SRegisteredClientInfo sClientInfo = 
		m_aRegisteredClientList[dwCivilizationIndex].RemovePosition ( posClient );
	ASSERT ( ( sClientInfo.pClient == pClient ) && ( sClientInfo.pWatchedMapSquareArray != NULL ) );

	TRACE_COMMUNICATION1 ( _T("#	UnregisterClient ( Client=%x )\n"), pClient );

// ukon�� sledov�n� v�ech sledovan�ch MapSquar�

	// zjist� pozici prvn�ho klienta civilizace
	POSITION posRemainingClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();

	// informace o zb�vaj�c�m klientovi civilizace
	struct SRegisteredClientInfo sRemainingClientInfo;

	// projede zb�vaj�c� klienty civilizace
	while ( CSRegisteredClientList::GetNext ( posRemainingClient, sRemainingClientInfo ) )
	{
		ASSERT ( ( sRemainingClientInfo.pClient != NULL ) && ( sRemainingClientInfo.pClient != pClient ) && 
			( sRemainingClientInfo.pWatchedMapSquareArray != NULL ) );
		// projede MapSquary sledovan� odregistrovan�m klientem civilizace
		for ( int nWatchedMapSquareIndex = sClientInfo.pWatchedMapSquareArray->GetCount (); 
			nWatchedMapSquareIndex-- > 0; )
		{
			// zjist� dal�� MapSquare sledovan� oderegistrovan�m klientem civilizace
			CSMapSquare *pWatchedMapSquare = 
				sClientInfo.pWatchedMapSquareArray->GetAt ( nWatchedMapSquareIndex );
			ASSERT ( pWatchedMapSquare->m_cWatchingCivilizations.GetAt ( dwCivilizationIndex ) );
			// zjist�, je-li MapSquare sledov�n i zb�vaj�c�m klientem civilizace
			if ( sRemainingClientInfo.pWatchedMapSquareArray->Find ( pWatchedMapSquare ) )
			{	// MapSquare je sledov�n i zb�vaj�c�m klientem civilizace
				// sma�e sledovan� MapSquare z MapSquar� sledovan�ch odregistrovan�m 
				//		klientem civilizace
				sClientInfo.pWatchedMapSquareArray->RemoveAt ( nWatchedMapSquareIndex );
			}
		}
	}
	// v seznamu sledovan�ch MapSquar� ponechal pouze MapSquary sledovan� v�hradn� 
	//		odregistrov�van�m klientem

	// projede MapSquary sledovan� v�hradn� odregistrovan�m klientem civilizace
	for ( int nWatchedMapSquareIndex = sClientInfo.pWatchedMapSquareArray->GetCount (); 
		nWatchedMapSquareIndex-- > 0; )
	{
		// zjist� dal�� MapSquare sledovan� odregistrovan�m klientem civilizace
		CSMapSquare *pWatchedMapSquare = 
			sClientInfo.pWatchedMapSquareArray->GetAt ( nWatchedMapSquareIndex );
		// zru�� p��znak civilizace sleduj�c� MapSquare
		// *****ASSERT ( pWatchedMapSquare->m_cWatchingCivilizations.GetAt ( dwCivilizationIndex ) );
		pWatchedMapSquare->m_cWatchingCivilizations.ClearAt ( dwCivilizationIndex );
	}

	// sma�e pole sledovan�ch MapSquar� odregistrovan�m klientem civilizace
	sClientInfo.pWatchedMapSquareArray->RemoveAll ();
	// zni�� pole sledovan�ch MapSquar� odregistrovan�m klientem civilizace
	delete sClientInfo.pWatchedMapSquareArray;

	// odemkne seznamy zaregistrovan�ch klient� civilizac� pro z�pis
	m_cRegisteredClientListLock.WriterUnlock ();
}

// za�ne sledovat jednotku "dwID" klientem "pClient" (FALSE=jednotka ji� neexistuje)
BOOL CSMap::StartWatchingUnit ( CZClientUnitInfoSender *pClient, DWORD dwID ) 
{
	ASSERT ( pClient != NULL );

	// zjist� jednotku podle ID
	CSUnit *pUnit = GetUnitByID ( dwID );

	// zjist�, existuje-li jednotka
	if ( pUnit == NULL )
	{	// jednotka ji� neexistuje
		// vr�t� p��znak neexistence jednotky
		return FALSE;
	}

	// zv��� po�et odkaz� na jednotku
	pUnit->AddRef ();

	// zamkne frontu po�adavk� na sledov�n� jednotek
	VERIFY ( m_mutexWatchingUnitRequestQueueLock.Lock () );

	// informace o po�adavku na sledov�n� jednotky
	struct SWatchingUnitRequestInfo sRequestInfo;
	// vypln� po�adavek na sledov�n� jednotky
	sRequestInfo.bStartWatchingUnit = TRUE;
	sRequestInfo.pClient = pClient;
	sRequestInfo.pUnit = pUnit;

	TRACE_COMMUNICATION2 ( _T("#	StartWatchingUnit ( Client=%x, UnitID=%d )\n"), pClient, dwID );

	// p�id� po�adavek na sledov�n� jednotky
	m_cWatchingUnitRequestQueue.Add ( sRequestInfo );

	// odemkne frontu po�adavk� na sledov�n� jednotek
	VERIFY ( m_mutexWatchingUnitRequestQueueLock.Unlock () );

	// vr�t� p��znak existuj�c� jednotky
	return TRUE;
}

// ukon�� sledov�n� jednotky "dwID" klientem "pClient"
void CSMap::StopWatchingUnit ( CZClientUnitInfoSender *pClient, DWORD dwID ) 
{
	ASSERT ( pClient != NULL );

	// zamkne frontu po�adavk� na sledov�n� jednotek
	VERIFY ( m_mutexWatchingUnitRequestQueueLock.Lock () );

	// informace o po�adavku na sledov�n� jednotky
	struct SWatchingUnitRequestInfo sRequestInfo;
	// vypln� po�adavek na ukon�en� sledov�n� jednotky
	sRequestInfo.bStartWatchingUnit = FALSE;
	sRequestInfo.pClient = pClient;
	sRequestInfo.dwID = dwID;

	TRACE_COMMUNICATION2 ( _T("#	StopWatchingUnit ( Client=%x, UnitID=%d )\n"), pClient, dwID );

	// p�id� po�adavek na ukon�en� sledov�n� jednotky
	m_cWatchingUnitRequestQueue.Add ( sRequestInfo );

	// odemkne frontu po�adavk� na sledov�n� jednotek
	VERIFY ( m_mutexWatchingUnitRequestQueueLock.Unlock () );
}

// za�ne sledovat MapSquare "pointIndex" klientem "pClient"
void CSMap::StartWatchingMapSquare ( CZClientUnitInfoSender *pClient, CPointDW pointIndex ) 
{
	ASSERT ( pClient != NULL );

	// zjist� po�adovan� MapSquare na sledov�n�
	CSMapSquare *pMapSquare = GetMapSquareFromIndex ( pointIndex );

	// zamkne frontu po�adavk� na sledov�n� MapSquar�
	VERIFY ( m_mutexWatchingMapSquareRequestQueueLock.Lock () );

	// informace o po�adavku na sledov�n� MapSquaru
	struct SWatchingMapSquareRequestInfo sRequestInfo;
	// vypln� po�adavek na sledov�n� MapSquaru
	sRequestInfo.bStartWatchingMapSquare = TRUE;
	sRequestInfo.pClient = pClient;
	sRequestInfo.pMapSquare = pMapSquare;

	TRACE_COMMUNICATION3 ( _T("#	StartWatchingMapSquare ( Client=%x, MapSquare=(%d, %d) )\n"), pClient, pointIndex.x, pointIndex.y );

	// p�id� po�adavek na sledov�n� MapSquaru
	m_cWatchingMapSquareRequestQueue.Add ( sRequestInfo );

	// nastav� ud�lost po�adavku na sledov�n� MapSquaru
	VERIFY ( m_eventWatchingMapSquareRequest.SetEvent () );

	// odemkne frontu po�adavk� na sledov�n� MapSquar�
	VERIFY ( m_mutexWatchingMapSquareRequestQueueLock.Unlock () );
}

// ukon�� sledov�n� MapSquaru "pointIndex" klientem "pClient"
void CSMap::StopWatchingMapSquare ( CZClientUnitInfoSender *pClient, CPointDW pointIndex ) 
{
	ASSERT ( pClient != NULL );

	// zjist� po�adovan� MapSquare na sledov�n�
	CSMapSquare *pMapSquare = GetMapSquareFromIndex ( pointIndex );

	// zamkne frontu po�adavk� na sledov�n� MapSquar�
	VERIFY ( m_mutexWatchingMapSquareRequestQueueLock.Lock () );

	// informace o po�adavku na sledov�n� MapSquaru
	struct SWatchingMapSquareRequestInfo sRequestInfo;
	// vypln� po�adavek na ukon�en� sledov�n� MapSquaru
	sRequestInfo.bStartWatchingMapSquare = FALSE;
	sRequestInfo.pClient = pClient;
	sRequestInfo.pMapSquare = pMapSquare;

	TRACE_COMMUNICATION3 ( _T("#	StopWatchingMapSquare ( Client=%x, MapSquare=(%d, %d) )\n"), pClient, pointIndex.x, pointIndex.y );

	// p�id� po�adavek na ukon�en� sledov�n� MapSquaru
	m_cWatchingMapSquareRequestQueue.Add ( sRequestInfo );

	// nastav� ud�lost po�adavku na sledov�n� MapSquaru
	VERIFY ( m_eventWatchingMapSquareRequest.SetEvent () );

	// odemkne frontu po�adavk� na sledov�n� MapSquar�
	VERIFY ( m_mutexWatchingMapSquareRequestQueueLock.Unlock () );
}

//////////////////////////////////////////////////////////////////////
// Operace s odlo�en�m vol�n�m procedury (DPC)
//////////////////////////////////////////////////////////////////////

// zaregistruje civilizaci "dwCivilizationIndex" odlo�en� vol�n� procedury "pDPC"
void CSMap::RegisterDPC ( CSDeferredProcedureCall *pDPC, DWORD dwCivilizationIndex ) 
{
	ASSERT ( dwCivilizationIndex < m_dwCivilizationCount );
	ASSERT ( pDPC != NULL );

	// zamkne DPC fronty civilizac�
	VERIFY ( m_mutexDPCQueueLock.Lock () );

	// p�id� DPC do DPC fronty civilizace
	m_aDPCQueue[dwCivilizationIndex].Add ( pDPC );

	// odemkne DPC fronty civilizac�
	VERIFY ( m_mutexDPCQueueLock.Unlock () );
}

// odregistruje civilizaci "dwCivilizationIndex" odlo�en� vol�n� procedury "pDPC" 
//		(FALSE=DPC nebylo zaregistrovan�)
BOOL CSMap::UnregisterDPC ( CSDeferredProcedureCall *pDPC, DWORD dwCivilizationIndex ) 
{
	ASSERT ( dwCivilizationIndex < m_dwCivilizationCount );
	ASSERT ( pDPC != NULL );

	// p��znak �sp�n�ho odregistrov�n� DPC
	BOOL bUnregistered;

	// zamkne DPC fronty civilizac�
	VERIFY ( m_mutexDPCQueueLock.Lock () );

	// pokus� se smazat DPC z DPC fronty civilizace
	if ( !m_aDPCQueue[dwCivilizationIndex].RemoveElement ( pDPC ) )
	{	// DPC nebylo nalezeno v DPC front� civilizace
		// zjist�, je-li DPC p�ipravov�no k proveden�
		if ( m_pPreparingDPC == pDPC )
		{	// DPC je p�ipravov�no k proveden�
			// odregistruje p�ipravovan� DPC
			m_pPreparingDPC = NULL;
			// nastav� n�vratov� p��znak odregistrov�n� DPC
			bUnregistered = TRUE;
		}
		else
		{	// DPC nen� p�ipravov�no k proveden�
			// sma�e n�vratov� p��znak odregistrov�n� DPC
			bUnregistered = FALSE;
		}
	}
	else
	{	// DPC bylo �sp�n� smaz�no
		// nastav� n�vratov� p��znak odregistrov�n� DPC
		bUnregistered = TRUE;
	}

	// odemkne DPC fronty civilizac�
	VERIFY ( m_mutexDPCQueueLock.Unlock () );

	// vr�t� p��znak odregistrov�n� DPC
	return bUnregistered;
}

// odregistruje odlo�en� vol�n� procedury "pDPC" (FALSE=DPC nebylo zaregistrovan�)
BOOL CSMap::UnregisterDPC ( CSDeferredProcedureCall *pDPC ) 
{
	ASSERT ( pDPC != NULL );

	// zamkne DPC fronty civilizac�
	VERIFY ( m_mutexDPCQueueLock.Lock () );

	// projede DPC fronty v�ech civilizac�
	for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; )
	{
		// pokus� se smazat DPC z DPC fronty civilizace
		if ( m_aDPCQueue[dwCivilizationIndex].RemoveElement ( pDPC ) )
		{	// DPC bylo z DPC fronty civilizace smaz�no
			// odemkne DPC fronty civilizac�
			VERIFY ( m_mutexDPCQueueLock.Unlock () );

			// vr�t� p��znak �sp�n�ho odregistrov�n� DPC
			return TRUE;
		}
	}
	// DPC nebylo nalezeno v DPC front�ch civilizac�

	// p��znak �sp�n�ho odregistrov�n� DPC
	BOOL bUnregistered;

	// zjist�, je-li DPC p�ipravov�no k proveden�
	if ( m_pPreparingDPC == pDPC )
	{	// DPC je p�ipravov�no k proveden�
		// odregistruje p�ipravovan� DPC
		m_pPreparingDPC = NULL;
		// nastav� n�vratov� p��znak odregistrov�n� DPC
		bUnregistered = TRUE;
	}
	else
	{	// DPC nen� p�ipravov�no k proveden�
		// sma�e n�vratov� p��znak odregistrov�n� DPC
		bUnregistered = FALSE;
	}

	// odemkne DPC fronty civilizac�
	VERIFY ( m_mutexDPCQueueLock.Unlock () );

	// vr�t� p��znak odregistrov�n� DPC
	return bUnregistered;
}

//////////////////////////////////////////////////////////////////////
// Vl�kna v�po�tu serveru hry
//////////////////////////////////////////////////////////////////////

// hlavn� smy�ka mapy
void CSMap::MainLoop () 
{
	ASSERT ( !m_eventMainLoopPaused.Lock ( 0 ) );

#ifdef _DEBUG
	// inicializuje ID MainLoop threadu
	ASSERT ( m_dwMainLoopThreadID == DWORD_MAX );
	m_dwMainLoopThreadID = GetCurrentThreadId ();

	// inicializuje ID privilegovan�ho threadu z�mk� blok� jednotek
	{
		// projede v�echny bloky jednotek
		for ( SUnitBlock *pBlock = m_pFirstUnitBlock; pBlock != NULL; 
			pBlock = pBlock->pNext )
		{
			// inicializuje ID privilegovan�ho threadu z�mku bloku jednotek
			pBlock->cLock.SetPrivilegedThreadID ( m_dwMainLoopThreadID );
		}
	}

	// inicializuje ID privilegovan�ho threadu z�mku mrtv�ch a smazan�ch jednotek
	m_cDeadDeletedUnitLock.SetPrivilegedThreadID ( m_dwMainLoopThreadID );
#endif //_DEBUG

	ASSERT ( CheckValidData () );

// prom�nn� hlavn�ho cyklu mapy

	// p��znak TimeSlicu s po��t�n�m viditelnosti
	BOOL bVisibilityTimeSlice;

	// nastav� �asova� TimeSlic�
	DWORD dwTimeSliceLength = m_dwTimeSliceLength;
	ASSERT ( dwTimeSliceLength > 0 );
	UINT nTimer = SetTimer ( NULL, 0, dwTimeSliceLength, NULL );

	// hlavn� cyklus TimeSlic�
	for ( ; ; m_dwTimeSlice++ )
	{
	// pomocn� prom�nn� cyklu

		// ukazatel na aktu�ln� blok jednotek
		SUnitBlock *pBlock;

	// zpracuje zapausov�n� hry

		// zjist�, je-li hra zapausovan�
		if ( m_eventPauseGame.Lock ( 0 ) )
		{	// hra je zapausovan�
			// nastav� ud�lost zapausov�n� MainLoop threadu
			VERIFY ( m_eventMainLoopPaused.SetEvent () );

			// ukon�� �asova� TimeSlic�
			VERIFY ( KillTimer ( NULL, nTimer ) );

			// usp� thread a zjist�, m�-li se pokra�ovat ve h�e
			if ( !CanContinueGame () )
			{	// hra se m� ukon�it
				#ifdef _DEBUG
					// aktualizuje ID MainLoop threadu
					m_dwMainLoopThreadID = DWORD_MAX;
				#endif //_DEBUG
				// ukon�� hru
				return;
			}
			// hra m� pokra�ovat

			// nastav� �asova� TimeSlic�
			dwTimeSliceLength = m_dwTimeSliceLength;
			ASSERT ( dwTimeSliceLength > 0 );
			nTimer = SetTimer ( NULL, 0, dwTimeSliceLength, NULL );
		}

		TRACE_LOOP1 ( _T("* TimeSlice: %d\n"), m_dwTimeSlice );

	// inicializace prom�nn�ch cyklu

		// nastav� p��znak TimeSlicu s po��t�n�m viditelnosti
		bVisibilityTimeSlice = IsVisibilityTimeSlice ( m_dwTimeSlice );

	// prov�d�n� TimeSlicu (cyklus p�es v�echny jednotky a jejich skilly)

		// p��znak zam�en� MapSquar� MainLoopem
		BOOL bMapSquaresLocked = TRUE;
		// zamkne MapSquary
		VERIFY ( m_mutexMapSquaresLock.Lock () );

		// projede v�echny bloky jednotek
		for ( pBlock = m_pFirstUnitBlock; pBlock != NULL; pBlock = pBlock->pNext )
		{
			// ukazatel na jednotku v bloku
			CSUnit **pBlockUnit = pBlock->pUnits;
			// zamkne blok jednotek pro z�pis
			VERIFY ( pBlock->cLock.PrivilegedThreadWriterLock () );
			// projede v�echny jednotky v bloku jednotek
			for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
			{
				// z�sk� ukazatel na jednotku
				CSUnit *pUnit = *pBlockUnit;

				// zjist�, je-li to platn� jednotka
				if ( pUnit == NULL )
				{	// nen� to platn� jednotka
					continue;
				}

				// po�et skill jednotky
				DWORD dwSkillCount = pUnit->GetUnitType ()->GetSkillCount ();
				// z�sk� ukazatel na popis typu skilly
				CSUnitType::SSkillTypeDescription *pSkillTypeDescription = 
					pUnit->GetUnitType ()->m_aSkillTypeDescription;
				// maska m�du jednotky
				BYTE nModeMask = pUnit->GetModeMask ();

				ASSERT ( pUnit->m_nLives > 0 );

				// provede TimeSlice na skill�ch jednotky
				for ( DWORD dwSkillIndex = 0; dwSkillIndex < dwSkillCount; dwSkillIndex++, 
					pSkillTypeDescription++ )
				{
					// zjist�, je-li skilla zapnuta (enabled)
					if ( !pSkillTypeDescription->bEnabled )
					{	// skilla nen� zapnuta
						continue;
					}
					// zjist�, je-li skilla v tomto m�du povolena
					if ( !( pSkillTypeDescription->nAllowedModesMask & nModeMask ) )
					{	// skilla nen� v tomto m�du povolena
						continue;
					}
					// zjist�, je-li skilla aktivn�
					if ( !pUnit->IsSkillActive ( dwSkillIndex ) )
					{	// skilla nen� aktivn�
						continue;
					}
					// provede TimeSlice na skille jednotky
					pSkillTypeDescription->pSkillType->DoTimeSlice ( pUnit );
				}
				// byl proveden TimeSlice na skill�ch jednotky
			}
			// byl zpracov�n cel� blok jednotek

			// odemkne zam�en� blok jednotek pro z�pis
			pBlock->cLock.PrivilegedThreadWriterUnlock ();
		}

	// sma�e mrtv� jednotky

		// zjist�, jedn�-li se o TimeSlice maz�n� mrtv�ch jednotek
		if ( IsDeletingDeadUnitsTimeSlice ( m_dwTimeSlice ) )
		{
		// sma�e mrtv� jednotky

			// ukazatel na popis mrtv� jednotky
			struct SDeadUnit sDeadUnit;

			// sma�e mrtv� jednotky
			while ( m_cDeadUnitQueue.Get ( sDeadUnit ) )
			{
				// zni�� skilly jednotky
				sDeadUnit.pUnit->m_pUnitType->DestructSkills ( sDeadUnit.pUnit );

				// zjist�, m�-li b�t mrtv� jednotka ji� smaz�na
				if ( sDeadUnit.dwTimeSlice <= m_dwTimeSlice )
				{	// jednotka m� b�t ji� smaz�na
					// odstran� jednotku z fronty mrtv�ch jednotek
					VERIFY ( m_cDeadUnitQueue.RemoveFirst ( sDeadUnit ) );

					// zamkne seznam smazan�ch jednotek
					VERIFY ( m_mutexDeletedUnitListLock.Lock () );
					// p�id� jednotku do seznamu smazan�ch jednotek
					m_cDeletedUnitList.Add ( sDeadUnit.pUnit );
					// odemkne seznam smazan�ch jednotek
					VERIFY ( m_mutexDeletedUnitListLock.Unlock () );

					// uvoln� odkaz na jednotku
					sDeadUnit.pUnit->Release ();
				}
				else
				{
					break;
				}
				// jednotka je�t� nem� b�t smaz�na
			}

		// ukon�� skilly smazan�ch jednotek

			// zamkne seznam smazan�ch jednotek
			VERIFY ( m_mutexDeletedUnitListLock.Lock () );

			// ukazatel na smazanou jednotku
			CSUnit *pDeletedUnit;
			// pozice smazan� jednotky v seznamu smazan�ch jednotek
			POSITION posDeletedUnit = m_cDeletedUnitList.GetHeadPosition ();

			// projede seznam smazan�ch jednotek
			while ( m_cDeletedUnitList.GetNext ( posDeletedUnit, pDeletedUnit ) )
			{
				// zni�� skilly smazan� jednotky
				pDeletedUnit->m_pUnitType->DestructSkills ( pDeletedUnit );
			}

			// odemkne seznam smazan�ch jednotek
			VERIFY ( m_mutexDeletedUnitListLock.Unlock () );
		}

	// �e�en� viditelnosti

		// zjist�, m�-li se po��tat viditelnost
		if ( bVisibilityTimeSlice )
		{	// m� se po��tat viditelnost
			// projede v�echny bloky jednotek
			for ( pBlock = m_pFirstUnitBlock; pBlock != NULL; pBlock = pBlock->pNext )
			{
				// ukazatel na jednotku v bloku
				CSUnit **pBlockUnit = pBlock->pUnits;
				// projede v�echny jednotky v bloku jednotek
				for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
				{
					// z�sk� ukazatel na jednotku
					CSUnit *pUnit = *pBlockUnit;

					// zjist�, je-li to platn� jednotka
					if ( pUnit == NULL )
					{	// nen� to platn� jednotka
						continue;
					}
					// je to platn� jednotka

					// vid�n� MapSquary
					CSMapSquare *aSeenMapSquares[4];

					// zjist� vid�n� MapSquary
					GetSeenMapSquares ( pUnit, aSeenMapSquares );

					// zjist� index civilizace jednotky
					DWORD dwUnitCivilizationIndex = pUnit->GetCivilizationIndex ();

					// zjist� pozici jednotky
					CPointDW pointUnitPosition = pUnit->GetPosition ();
					// zjist� druhou mocninu polom�ru viditelnosti jednotky
					DWORD dwUnitViewRadiusSquare = pUnit->GetViewRadiusSquare ();
					// zjist� odkaz na seznam vid�n�ch jednotek
					CSUnitList *pSeenUnitList = pUnit->GetSeenUnitList ();
					// zjist� bitovou masku p��znak� detekce neviditelnosti ostatn�ch 
					//		jednotek
					DWORD dwUnitInvisibilityDetection = pUnit->GetInvisibilityDetection ();

					// seznam naposledy vid�n�ch jednotek
					CSUnitList cLastSeenUnitList;
					// p�esune seznam jednotek do seznamu naposledy vid�n�ch jednotek
					pSeenUnitList->MoveList ( cLastSeenUnitList );

					// projede vid�n� MapSquary
					for ( int nIndex = 0; ( nIndex < 4 ) && ( aSeenMapSquares[nIndex] != NULL ); nIndex++ )
					{
						// zjist�, je-li v MapSquaru ciz� jednotka
						if ( aSeenMapSquares[nIndex]->IsExclusive ( dwUnitCivilizationIndex ) )
						{	// v MapSquaru nen� ciz� jednotka
							continue;
						}
						// v MapSquaru je ciz� jednotka

						// zjist� ukazatel na seznam jednotek vid�n�ho MapSquaru
						CSUnitList *pSeenMapSquareUnitList = aSeenMapSquares[nIndex]->GetUnitList ();

						// zjist� pozici prvn� jednotky seznamu jednotek ve vid�n�m MapSquaru
						POSITION posSeenMapSquareUnit = pSeenMapSquareUnitList->GetHeadPosition ();
						// jednotka z vid�n�ho MapSquaru
						CSUnit *pSeenMapSquareUnit;

						// projede jednotky v seznamu jednotek vid�n�ho MapSquaru
						while ( CSUnitList::GetNext ( posSeenMapSquareUnit, pSeenMapSquareUnit ) )
						{
							// zjist� index civilizace vid�n� jednotky
							DWORD dwSeenUnitCivilizationIndex = 
								pSeenMapSquareUnit->GetCivilizationIndex ();

							// zjist�, je-li jednotka ze stejn� civilizace
							if ( dwSeenUnitCivilizationIndex == dwUnitCivilizationIndex )
							{	// jednotky jsou ze stejn� civilizace
								continue;
							}

							// zjist�, je-li jednotka skute�n� vid�t
							if ( ( pointUnitPosition.GetDistanceSquare ( pSeenMapSquareUnit->GetPosition () ) < 
								dwUnitViewRadiusSquare ) && pSeenMapSquareUnit->IsVisible ( dwUnitInvisibilityDetection ) )
							{	// jednotka je skute�n� vid�t
								// najde jednotku v seznamu naposledy vid�n�ch jednotek
								POSITION posSeenUnit = cLastSeenUnitList.Find ( pSeenMapSquareUnit );
								// zjist�, byla-li jednotka ji� vid�na
								if ( posSeenUnit != NULL )
								{	// jednotka ji� byla vid�na
									ASSERT ( !cLastSeenUnitList.IsEmpty ( posSeenUnit ) );
									// p�esune jednotku do vid�n�ch jednotek
									cLastSeenUnitList.Move ( posSeenUnit, *pSeenUnitList );
								}
								else
								{	// jednotka dosud nebyla vid�t
									// p�id� jednotku do seznamu vid�n�ch jednotek
									pSeenUnitList->Add ( pSeenMapSquareUnit );
									// zjist�, jedn�-li se o nep��telskou jednotku
									if ( dwSeenUnitCivilizationIndex != 0 )
									{	// jedn� se o nep��telskou jednotku
										// zv��� po�et jednotek sv� civilizace, kter� vid�nou jednotku vid�
										if ( pSeenMapSquareUnit->m_aCivilizationVisibility[dwUnitCivilizationIndex]++ == 0 )
										{	// jednotka se objevila v dohledu civilizace civilizaci
											// nastav� p��znak zm�ny viditelnosti jednotky civilizac�
											pSeenMapSquareUnit->m_cCivilizationVisibilityChanged.SetAt ( dwUnitCivilizationIndex );
											// po�le hlavn�mu veliteli notifikaci o zviditeln�n� nep��telsk� jednotky civilizac�
											SendNotification ( m_pCivilizations[dwUnitCivilizationIndex].GetZCivilization ()->GetGeneralCommander (), 
												NOTIFICATION_ENEMY_UNIT_SEEN_BY_CIVILIZATION, 
												pSeenMapSquareUnit );
										}
										// po�le jednotce notifikaci o zviditeln�n� nep��telsk� jednotky
										SendNotification ( pUnit->GetZUnit (), 
											NOTIFICATION_ENEMY_UNIT_SEEN, pSeenMapSquareUnit );
									}
									else
									{	// jedn� se o syst�movou jednotku
										// zv��� po�et jednotek sv� civilizace, kter� vid�nou jednotku vid�
										if ( pSeenMapSquareUnit->m_aCivilizationVisibility[dwUnitCivilizationIndex]++ == 0 )
										{	// jednotka se objevila v dohledu civilizace civilizaci
											// nastav� p��znak zm�ny viditelnosti jednotky civilizac�
											pSeenMapSquareUnit->m_cCivilizationVisibilityChanged.SetAt ( dwUnitCivilizationIndex );
											// po�le hlavn�mu veliteli notifikaci o zviditeln�n� syst�mov� jednotky civilizac�
											SendNotification ( 
												m_pCivilizations[dwUnitCivilizationIndex].GetZCivilization ()->GetGeneralCommander (), 
												NOTIFICATION_SYSTEM_UNIT_SEEN_BY_CIVILIZATION, 
												pSeenMapSquareUnit );
										}
										// po�le jednotce notifikaci o zviditeln�n� syst�mov� jednotky
										SendNotification ( pUnit->GetZUnit (), 
											NOTIFICATION_SYSTEM_UNIT_SEEN, pSeenMapSquareUnit );
									}
								}
							}
						}
					}
					// byly projety vid�n� MapSquary

					// jednotka, kter� p�estala b�t vid�t
					CSUnit *pLastSeenUnit;
					// projede jednotky, kter� p�estaly b�t vid�t
					while ( cLastSeenUnitList.RemoveFirst ( pLastSeenUnit ) )
					{
						ASSERT ( pLastSeenUnit->m_aCivilizationVisibility[dwUnitCivilizationIndex] > 0 );
						// sn�� po�et jednotek sv� civilizace, kter� vid�nou jednotku vid�
						if ( --pLastSeenUnit->m_aCivilizationVisibility[dwUnitCivilizationIndex] == 0 )
						{	// jednotka zmizela civilizaci z dohledu
							// nastav� p��znak zm�ny viditelnosti jednotky civilizac�
							pLastSeenUnit->m_cCivilizationVisibilityChanged.SetAt ( dwUnitCivilizationIndex );
							// naposledy vid�n� pozice jednotky, kter� p�estala b�t vid�t
							CSPosition *pPosition = 
								pLastSeenUnit->m_aCivilizationLastSeenPosition + 
								dwUnitCivilizationIndex;
							// zamkne jednotku, kter� p�estala b�t vid�t na z�pis
							VERIFY ( pLastSeenUnit->PrivilegedThreadWriterLock () );
							// zjist�, je-li jednotka, kter� p�estala b�t vid�t, v okol� jednotky
							if ( pointUnitPosition.GetDistanceSquare ( pLastSeenUnit->GetPosition () ) <= 
								UNIT_VIEW_RADIUS_TOLERANCE ( dwUnitViewRadiusSquare ) )
							{	// jednotka, kter� p�estala b�t vid�t, je v okol� jednotky
								// aktualizuje naposledy vid�nou pozici jednotky civilizac�
								*(CPointDW *)pPosition = pLastSeenUnit->m_pointPosition;
								pPosition->z = pLastSeenUnit->m_dwVerticalPosition;
								ASSERT ( pPosition->z != 0 );
							}
							else
							{	// jednotka, kter� p�estala b�t vid�t, nen� v okol� jednotky
								// aktualizuje naposledy vid�nou pozici jednotky civilizac�
								pPosition->x = pPosition->y = NO_MAP_POSITION;
								pPosition->z = 0;
							}
							// odemkne jednotku, kter� p�estala b�t vid�t na z�pis
							pLastSeenUnit->PrivilegedThreadWriterUnlock ();
						}
						// zjist�, jedn�-li se o nep��telskou jednotku
						if ( pLastSeenUnit->GetCivilizationIndex () != 0 )
						{	// jedn� se o nep��telskou jednotku
							// po�le jednotce notifikaci o ukon�en� viditelnosti nep��telsk� jednotky
							SendNotification ( pUnit->GetZUnit (), 
								NOTIFICATION_ENEMY_UNIT_DISAPPEARED, pLastSeenUnit );
						}
						else
						{	// jedn� se o syst�movou jednotku
							// po�le jednotce notifikaci o ukon�en� viditelnosti syst�mov� jednotky
							SendNotification ( pUnit->GetZUnit (), 
								NOTIFICATION_SYSTEM_UNIT_DISAPPEARED, pLastSeenUnit );
						}
					}
					// rozeslal notifikace o jednotk�ch, kter� p�estaly b�t vid�t
				}
				// byly projety v�echny jednotky v bloku jednotek
			}
			// byly projety v�echny bloky jednotek
		}
		// viditelnost byla spo��t�na

	// upozorn� ClientInfoSenderLoop na ukon�en� TimeSlicu

		// pust� ClientInfoSenderLoop �ekaj�c� na konec TimeSlicu
		VERIFY ( m_eventClientInfoSenderLoopTimeSliceFinished.SetEvent () );

	// vy�izov�n� odlo�en�ch vol�n� procedur (DPC)

		// prov�d� DPC dokud �asova� nenastav� ud�lost nov�ho TimeSlicu
		do
		{
			// p��znak vol�n� DPC
			BOOL bDPCCalled = FALSE;

			// projede DPC fronty civilizac�
			for ( m_dwRunningDPCCivilizationIndex = m_dwCivilizationCount; 
				m_dwRunningDPCCivilizationIndex-- > 0; )
			{
				// zjist�, je-li DPC fronta civilizace pr�zdn�
				if ( !m_aDPCQueue[m_dwRunningDPCCivilizationIndex].IsEmpty () )
				{	// DPC fronta civilizace nen� pr�zdn�
					// zamkne DPC fronty civilizac�
					VERIFY ( m_mutexDPCQueueLock.Lock () );

					// ukazatel na DPC
					CSDeferredProcedureCall *pDPC;

					// odebere prvn� DPC z DPC fronty civilizace
					VERIFY ( m_aDPCQueue[m_dwRunningDPCCivilizationIndex].RemoveFirst ( 
						pDPC ) );

					// zjist� asociovanou jednotku DPC
					CSUnit *pUnit = pDPC->GetUnit ();

					// zjist�, m�-li DPC asociov�nu jednotku
					if ( pUnit != NULL )
					{	// DPC m� asociov�nu jednotku
					// p�iprav� DPC na proveden�

						// zjist� p��znak zamyk�n� jednotky pro z�pis
						BOOL bWriterLock = pDPC->NeedWriterLock ();

						// aktualizuje p�ipravovan� DPC
						m_pPreparingDPC = pDPC;

						// odemkne DPC fronty civilizac�
						VERIFY ( m_mutexDPCQueueLock.Unlock () );

						// zjist�, m�-li se asociovan� jednotka DPC zamykat pro z�pis
						if ( bWriterLock )
						{	// asociovan� jednotka DPC se m� zamykat pro z�pis
							// zamkne asociovanou jednotku DPC pro z�pis
							VERIFY ( pUnit->PrivilegedThreadWriterLock () );
						}
						else
						{	// asociovan� jednotka DPC se m� zamykat pro �ten�
							// zamkne asociovanou jednotku DPC pro �ten�
							VERIFY ( pUnit->PrivilegedThreadReaderLock () );
						}

					// provede DPC

						// zamkne DPC fronty civilizac�
						VERIFY ( m_mutexDPCQueueLock.Lock () );

						// zjist� p��znak odregistrov�n� DPC
						BOOL bDPCUnregistered = ( m_pPreparingDPC == NULL );

						// aktualizuje p�ipravovan� DPC
						m_pPreparingDPC = NULL;

						// odemkne DPC fronty civilizac�
						VERIFY ( m_mutexDPCQueueLock.Unlock () );

						// zjist�, bylo-li DPC odregistrov�no
						if ( !bDPCUnregistered )
						{	// DPC nebylo odregistrov�no
							// provede DPC
							pDPC->Run ();
							// zni�� DPC
							delete pDPC;
						}

					// uklid� po proveden�m DPC

						// zjist�, byla-li asociovan� jednotka DPC zam�ena pro z�pis
						if ( bWriterLock )
						{	// asociovan� jednotka DPC byla zam�ena pro z�pis
							// odemkne asociovanou jednotku DPC pro z�pis
							pUnit->PrivilegedThreadWriterUnlock ();
						}
						else
						{	// asociovan� jednotka DPC byla zam�ena pro �ten�
							// odemkne asociovanou jednotku DPC pro �ten�
							pUnit->PrivilegedThreadReaderUnlock ();
						}
					}
					else
					{	// DPC nem� asociov�nu jednotku
					// provede DPC

						// odemkne DPC fronty civilizac�
						VERIFY ( m_mutexDPCQueueLock.Unlock () );

						// provede DPC
						pDPC->Run ();
						// zni�� DPC
						delete pDPC;
					}

					// nastav� p��znak vol�n� DPC
					bDPCCalled = TRUE;
				}
			}

			// zjist�, bylo-li vol�no n�kter� DPC
			if ( !bDPCCalled )
			{	// nebylo vol�no ��dn� DPC
				// odemkne MapSquary
				VERIFY ( m_mutexMapSquaresLock.Unlock () );
				// aktualizuje p��znak zam�en� MapSquar� MainLoopem
				bMapSquaresLocked = FALSE;

				// po�k� na nov� p��choz� DPC
				if ( WaitForNewTimeSlice ( m_dwTimeSliceLength / DPC_INCOME_TEST_DELAY_RATE ) )
				{	// nastala ud�lost nov�ho TimeSlicu
					// ukon�� vy�izov�n� odlo�en�ch vol�n� procedur (DPC)
					break;
				}
				// zamkne MapSquary
				VERIFY ( m_mutexMapSquaresLock.Lock () );
				// aktualizuje p��znak zam�en� MapSquar� MainLoopem
				bMapSquaresLocked = TRUE;

				// nech� zkontrolovat nov� p��choz� DPC
				continue;
			}
		} while ( !WaitForNewTimeSlice ( 0 ) );

		// zjist�, jsou-li MapSquary zam�eny MainLoopem
		if ( bMapSquaresLocked )
		{	// MapSquary jsou zam�eny MainLoopem
			// odemkne MapSquary
			VERIFY ( m_mutexMapSquaresLock.Unlock () );
			// aktualizuje p��znak zam�en� MapSquar� MainLoopem
			bMapSquaresLocked = FALSE;
		}

	// aktualizuje d�lku TimeSlicu

		// zjist�, zm�nila-li se d�lka TimeSlicu
		if ( dwTimeSliceLength != m_dwTimeSliceLength )
		{	// zm�nila se d�lka TimeSlicu
			// ukon�� �asova� TimeSlic�
			VERIFY ( KillTimer ( NULL, nTimer ) );

			// nastav� �asova� TimeSlic�
			dwTimeSliceLength = m_dwTimeSliceLength;
			ASSERT ( dwTimeSliceLength > 0 );
			nTimer = SetTimer ( NULL, 0, dwTimeSliceLength, NULL );
		}

	// zpracuje ukon�en� TimeSlicu

		VERIFY ( m_cMapObserverListLock.Lock ( ) );

		CSMapObserver *pObserver = m_pFirstMapObserver;
		while ( pObserver != NULL )
		{
			pObserver->OnTimeslice ( m_dwTimeSlice );
			pObserver = pObserver->m_pNextObserver;
		}

		VERIFY ( m_cMapObserverListLock.Unlock ( ) );
	}
	// skon�il hlavn� cyklus TimeSlic�

#ifdef _DEBUG
	// zne�kodn� ID MainLoop threadu
	m_dwMainLoopThreadID = DWORD_MAX;
#endif //_DEBUG
}

// smy�ka rozes�l�n� informac� klient�m civilizac�
void CSMap::ClientInfoSenderLoop () 
{
	ASSERT ( !m_eventClientInfoSenderLoopPaused.Lock ( 0 ) );

// prom�nn� hlavn�ho cyklu rozes�l�n� informac� o jednotk�ch

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

	// ukazatel na klienta, kter�mu se pos�l� MiniMapa
	CZClientUnitInfoSender *pMiniMapClient = NULL;
	// index klienta, kter�mu se pos�l� MiniMapa
	DWORD dwMiniMapClientIndex = 0;
	// index civilizace klienta, kter�mu se pos�l� MiniMapa
	DWORD dwMiniMapClientCivilizationIndex = 0;

// inicializace rozes�l�n� MiniMapy

	// inicializace rozes�l�n� MiniMapy
	CZClientUnitInfoSender::InitMiniMapServer ();

// hlavn� cyklus rozes�l�n� informac� klient�m civilizac�
	for ( ; ; )
	{
		// tabulka ud�lost�, na kter� se �ek�
		HANDLE aNewLoopEvents[3] = { 
			(HANDLE)m_eventClientInfoSenderLoopTimeSliceFinished, 
			(HANDLE)m_eventWatchingMapSquareRequest, 
			(HANDLE)m_eventPauseGame,
		};

		// ��slo posledn�ho TimeSlicu p�ed �ek�n�m na ukon�en� TimeSlicu
		DWORD dwLastTimeSlice = m_dwTimeSlice;
		// p��znak ukon�en� TimeSlicu
		BOOL bTimeSliceFinished = FALSE;

		// cyklus �ek�n� na konec TimeSlicu
		do
		{
			// po�k� na konec TimeSlicu nebo na po�adavek sledov�n� MapSquaru
			switch ( WaitForMultipleObjects ( 3, aNewLoopEvents, FALSE, INFINITE ) )
			{
			// TimeSlice byl ukon�en
			case WAIT_OBJECT_0 :
				// nastav� p��znak ukon�en� TimeSlicu
				bTimeSliceFinished = TRUE;
				break;
			// p�i�el po�adavek na sledov�n� MapSquaru
			case ( WAIT_OBJECT_0 + 1 ) :
				{
				// vy��d� po�adavky na sledov�n� MapSquar�

					// zamkne MapSquary
					VERIFY ( m_mutexMapSquaresLock.Lock () );

					// zamkne seznamy zaregistrovan�ch klient� civilizac� pro �ten�
					VERIFY ( m_cRegisteredClientListLock.ReaderLock () );

					// zamkne frontu po�adavk� na sledov�n� MapSquar�
					VERIFY ( m_mutexWatchingMapSquareRequestQueueLock.Lock () );

					// informace o po�adavku na sledov�n� MapSquaru
					struct SWatchingMapSquareRequestInfo sRequestInfo;

					// vyjme prvn� po�adavek na sledov�n� MapSquaru z fronty
					while ( m_cWatchingMapSquareRequestQueue.RemoveFirst ( sRequestInfo ) )
					{
						// zjist� klienta po�adavku na sledov�n� MapSquaru
						CZClientUnitInfoSender *pClient = sRequestInfo.pClient;
						// zjist� MapSquare po�adavku na sledov�n� MapSquaru
						CSMapSquare *pMapSquare = sRequestInfo.pMapSquare;
						// zjist� index civilizace klienta
						DWORD dwCivilizationIndex = pClient->GetCivilizationIndex ();

						// p��znak, zm�nila-li se sledovanost MapSquaru
						BOOL bWatchedMapSquareChanged;

						// najde klienta civilizace
						POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].FindClient ( 
							pClient );
						ASSERT ( posClient != NULL );

						// z�sk� informace o klientovi civilizace
						struct SRegisteredClientInfo sClientInfo;
						VERIFY ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) );

						// zjist�, jedn�-li se o za��tek sledov�n� MapSquaru
						if ( sRequestInfo.bStartWatchingMapSquare )
						{	// jedn� se o po�adavek na za��tek sledov�n� MapSquaru
							// zjist�, je-li MapSquare klientem ji� sledov�n
							bWatchedMapSquareChanged = !sClientInfo.pWatchedMapSquareArray->Find ( pMapSquare );

							TRACE_COMMUNICATION3 ( _T("#	Starting watch MapSquare ( Client=%x, MapSquare=(%d, %d) )\n"), pClient, pMapSquare->GetIndex ().x, pMapSquare->GetIndex ().y );

							// p�id� MapSquare do pole klientem sledovan�ch MapSquar�
							sClientInfo.pWatchedMapSquareArray->Add ( pMapSquare );

							// aktualizuje p��znak sledov�n� MapSquaru civilizac�
							pMapSquare->m_cWatchingCivilizations.SetAt ( dwCivilizationIndex );
						}
						else
						{	// jedn� se o po�adavek na ukon�en� sledov�n� MapSquaru
							ASSERT ( sClientInfo.pWatchedMapSquareArray->Find ( pMapSquare ) );

							TRACE_COMMUNICATION3 ( _T("#	Stopping watch MapSquare ( Client=%x, MapSquare=(%d, %d) )\n"), pClient, pMapSquare->GetIndex ().x, pMapSquare->GetIndex ().y );

							// odebere MapSquare z pole klientem sledovan�ch MapSquar�
							sClientInfo.pWatchedMapSquareArray->RemoveElement ( pMapSquare );

							// zjist�, je-li MapSquare klientem st�le sledov�n
							bWatchedMapSquareChanged = !sClientInfo.pWatchedMapSquareArray->Find ( pMapSquare );

							// zjist�, je-li MapSquare klientem st�le sledov�n
							if ( bWatchedMapSquareChanged )
							{	// MapSquare ji� nen� klientem sledov�n
							// aktualizuje p��znak sledov�n� MapSquaru civilizac�

								// zjist� pozici prvn�ho klienta civilizace
								posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();

								// p��znak sledov�n� MapSquaru civilizac�
								BOOL bMapSquareWatched = FALSE;

								// projede klienty civilizace
								while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
								{
									ASSERT ( ( sClientInfo.pClient != NULL ) && 
										( sClientInfo.pWatchedMapSquareArray != NULL ) );

									// zjist�, je-li MapSquare sledov�n klientem civilizace
									if ( sClientInfo.pWatchedMapSquareArray->Find ( pMapSquare ) )
									{	// MapSquare je sledov�n klientem civilizace
										// nastav� p��znak sledov�n� MapSquaru civilizac�
										bMapSquareWatched = TRUE;
										break;
									}
								}
								// projel klienty civilizace

								// zjist�, je-li MapSquare sledov�n civilizac�
								if ( !bMapSquareWatched )
								{	// MapSquare ji� nen� sledov�n civilizac�
									ASSERT ( pMapSquare->m_cWatchingCivilizations.GetAt ( dwCivilizationIndex ) );
									// sma�e p��znak sledov�n� MapSquaru civilizac�
									pMapSquare->m_cWatchingCivilizations.ClearAt ( dwCivilizationIndex );
								}
							}
							// byl aktualizov�n p��znak sledov�n� MapSquaru civilizac�
						}
						// po�adavek na sledov�n� MapSquaru byl zpracov�n

						// zjist�, zm�nila-li se sledovanost MapSquaru
						if ( bWatchedMapSquareChanged )
						{	// sledovanost MapSquaru se zm�nila
						// nastav� v�em jednotk�m MapSquaru p��znak zm�ny pozice jednotky 
						//		z/do sledovan�ch MapSquar�

							// zjist� seznam jednotek MapSquaru
							CSUnitList *pUnitList = pMapSquare->GetUnitList ();

							// ukazatel na jednotku MapSquaru
							CSUnit *pUnit;

							// z�sk� pozici prvn� jednotky MapSquaru
							POSITION posUnit = pUnitList->GetHeadPosition ();
							// projede jednotky MapSquaru
							while ( CSUnitList::GetNext ( posUnit, pUnit ) )
							{
								// nastav� jednotce p��znak zm�ny pozice jednotky z/do 
								//		sledovan�ch MapSquar�
								pUnit->m_cWatchedMapSquareChanged.SetAt ( dwCivilizationIndex );
							}
							// v�echny jednotky MapSquaru byly projety
						}
						// byly zm�n�ny p��padn� zm�ny pozice jednotky z/do sledovan�ch 
						//		MapSquar�
					}
					// v�echny po�adavky na sledov�n� MapSquar� byly zpracov�ny

					// sma�e ud�lost po�adavku na sledov�n� MapSquaru
					VERIFY ( m_eventWatchingMapSquareRequest.ResetEvent () );

					// odemkne frontu po�adavk� na sledov�n� MapSquar�
					VERIFY ( m_mutexWatchingMapSquareRequestQueueLock.Unlock () );

					// odemkne seznamy zaregistrovan�ch klient� civilizac� pro �ten�
					m_cRegisteredClientListLock.ReaderUnlock ();

					// odemkne MapSquary
					VERIFY ( m_mutexMapSquaresLock.Unlock () );
				}
				break;
			// hra byla zapausov�na
			case ( WAIT_OBJECT_0 + 2 ) :
				// nastav� ud�lost zapausov�n� ClientInfoSenderLoop threadu
				VERIFY ( m_eventClientInfoSenderLoopPaused.SetEvent () );

				// usp� thread a zjist�, m�-li se pokra�ovat ve h�e
				if ( !CanContinueGame () )
				{	// hra se m� ukon�it
					// ukon�� rozes�l�n� minimapy
					CZClientUnitInfoSender::DoneMiniMapServer ();

					// ukon�� hru
					return;
				}
				// hra m� pokra�ovat
				break;
			// nemo�n� p��pad
			default:
				ASSERT ( FALSE );
			}

			// zjist� zm�nilo-li se ��slo TimeSlicu
			if ( dwLastTimeSlice != m_dwTimeSlice )
			{	// zm�nilo se ��slo TimeSlicu
				// nastav� p��znak ukon�en� TimeSlicu
				bTimeSliceFinished = TRUE;
			}
		} while ( !bTimeSliceFinished );

		TRACE_LOOP1 ( _T("* Sending client info ( TimeSlice = %d )\n"), m_dwTimeSlice );

	// za�ne rozes�lat informace o jednotk�ch

		// zamkne seznamy zaregistrovan�ch klient� civilizac� pro �ten�
		VERIFY ( m_cRegisteredClientListLock.ReaderLock () );

		// upozorn� klienty civilizac� na za��tek rozes�l�n� informac� o jednotk�ch
		{
			// projede civilizace
			for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; )
			{
				// informace o zaregistrovan�m klientovi
				struct SRegisteredClientInfo sClientInfo;

				// zjist� pozici prvn�ho klienta civilizace
				POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
				// projede klienty civilizace
				while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
				{
					TRACE_COMMUNICATION1 ( _T("#		StartSendingInfo ( Client=%x )\n"), sClientInfo.pClient );
					// upozorn� klienta na za��tek rozes�l�n� informac� o jednotk�ch
					sClientInfo.pClient->StartSendingInfo ();
					TRACE_COMMUNICATION1 ( _T("#	SendResourceInfo ( Client=%x )\n"), sClientInfo.pClient );
					// po�le klientovi informace o surovin�ch
					sClientInfo.pClient->SendResourceInfo ( 
						m_pCivilizations[sClientInfo.pClient->GetCivilizationIndex ()].GetResources () );
				}
			}
		}

	// vy��d� po�adavky na sledov�n� jednotek

		// zamkne frontu po�adavk� na sledov�n� jednotek
		VERIFY ( m_mutexWatchingUnitRequestQueueLock.Lock () );

		// zjist�, je-li fronta po�adavk� na sledov�n� jednotek pr�zdn�
		if ( !m_cWatchingUnitRequestQueue.IsEmpty () )
		{	// fronta po�adavk� na sledov�n� jednotek nen� pr�zdn�
			// informace o po�adavku na sledov�n� jednotky
			struct SWatchingUnitRequestInfo sRequestInfo;

			// zamkne seznam sledovan�ch jednotek
			VERIFY ( m_mutexWatchedUnitListLock.Lock () );

			// vyjme prvn� po�adavek na sledov�n� jednotky z fronty
			while ( m_cWatchingUnitRequestQueue.RemoveFirst ( sRequestInfo ) )
			{
				// zjist� klienta po�adavku na sledov�n� jednotky
				CZClientUnitInfoSender *pClient = sRequestInfo.pClient;

				// zjist�, jedn�-li se o za��tek sledov�n� jednotky
				if ( sRequestInfo.bStartWatchingUnit )
				{	// jedn� se o po�adavek na za��tek sledov�n� jednotky
					// zjist� jednotku po�adavku na sledov�n� jednotky
					CSUnit *pUnit = sRequestInfo.pUnit;

					// p�iprav� informace o sledovan� jednotce
					struct SWatchedUnitInfo sUnitInfo;
					sUnitInfo.pUnit = pUnit;
					sUnitInfo.pClient = pClient;

				// nech� jednotku sledovat

					// zjist�, je-li jednotka ji� sledov�na
					BOOL bUnitAlreadyWatched = ( m_cWatchedUnitList.Find ( sUnitInfo ) != NULL );
					// p�id� jednotku do seznamu sledovan�ch jednotek
					m_cWatchedUnitList.Add ( sUnitInfo );

					// zv��� po�et klient� sleduj�c�ch jednotku
					pUnit->m_dwWatchingCivilizationClientCount++;

					// zjist�, byla-li jednotka ji� sledov�na
					if ( !bUnitAlreadyWatched )
					{	// jednotka dosud nebyla sledov�na
					// ode�le start (enemy) full info jednotky

						// zamkne jednotku pro �ten�
						VERIFY ( pUnit->ReaderLock () );

						// zjist�, jedn�-li se o jednotku civilizace klienta
						if ( pClient->GetCivilizationIndex () == pUnit->GetCivilizationIndex () )
						{	// jedn� se o jednotku civilizace klienta
							TRACE_COMMUNICATION2 ( _T("# StartFullInfo ( Client=%x, UnitID=%x )\n"), pClient, pUnit->GetID () );
							// po�le start full info jednotky
							pUnit->SendStartFullInfo ( &sStartFullInfo, pClient );
						}
						else
						{	// nejedn� se o jednotku civilizace klienta
							TRACE_COMMUNICATION2 ( _T("# StartEnemyFullInfo ( Client=%x, UnitID=%x )\n"), pClient, pUnit->GetID () );
							// po�le start enemy full info jednotky
							pUnit->SendStartEnemyFullInfo ( &sStartEnemyFullInfo, pClient );
						}

						// odemkne jednotku pro �ten�
						pUnit->ReaderUnlock ();
					}
					// bylo odesl�no p��padn� start (enemy) full info jednotky
				}
				else
				{	// jedn� se o po�adavek na ukon�en� sledov�n� jednotky
					// zjist� ID jednotky po�adavku na sledov�n� jednotky
					DWORD dwID = sRequestInfo.dwID;

					// zjist� um�st�n� sledovan� jednotky "dwID" v seznamu sledovan�ch jednotek
					POSITION posUnit = m_cWatchedUnitList.FindUnitByID ( dwID, pClient );
					ASSERT ( posUnit != NULL );

					// sma�e jednotku "dwID" ze seznamu sledovan�ch jednotek
					struct SWatchedUnitInfo sUnitInfo = 
						m_cWatchedUnitList.RemovePosition ( posUnit );
					ASSERT ( ( sUnitInfo.pUnit->GetID () == dwID ) && 
						( sUnitInfo.pClient == pClient ) );

					// zjist� mazanou jednotku
					CSUnit *pUnit = sUnitInfo.pUnit;

					// sn�� po�et klient� sleduj�c�ch jednotku
					ASSERT ( pUnit->m_dwWatchingCivilizationClientCount > 0 );
					pUnit->m_dwWatchingCivilizationClientCount--;

					// zjist� civilizaci jednotky
					DWORD dwUnitCivilizationIndex = pUnit->GetCivilizationIndex ();

					// sn�� po�et odkaz� na jednotku
					pUnit->Release ();

					// zjist�, je-li jednotka st�le sledov�na klientem "pClient"
					if ( m_cWatchedUnitList.Find ( sUnitInfo ) == NULL )
					{	// jednotka ji� nen� sledov�na klientem "pClient"
						// zjist�, jedn�-li se o jednotku ze stejn� civilziace
						if ( pClient->GetCivilizationIndex () == dwUnitCivilizationIndex )
						{	// jedn� se o jednotku ze stejn� civilizace
							TRACE_COMMUNICATION2 ( _T("# StopFullInfo ( Client=%x, UnitID=%x )\n"), pClient, dwID );
							// ode�le klientovi stop full info jednotky
							pClient->StopFullInfo ( dwID );
						}
						else
						{	// jedn� se o jednotku z ciz� civilizace
							TRACE_COMMUNICATION2 ( _T("# StopEnemyFullInfo ( Client=%x, UnitID=%x )\n"), pClient, dwID );
							// ode�le klientovi stop enemy full info jednotky
							pClient->StopEnemyFullInfo ( dwID );
						}
					}
					// odeslal p��padn� stop (enemy) full info
				}
				// po�adavek na sledov�n� jednotky byl zpracov�n
			}
			// v�echny po�adavky na sledov�n� jednotek byly zpracov�ny

			// odemkne seznam sledovan�ch jednotek
			VERIFY ( m_mutexWatchedUnitListLock.Unlock () );
		}

		// odemkne frontu po�adavk� na sledov�n� jednotek
		VERIFY ( m_mutexWatchingUnitRequestQueueLock.Unlock () );

	// najde klienta, kter�mu m� pos�lat MiniMapu

		do
		{
			// informace o zaregistrovan�m klientovi
			struct SRegisteredClientInfo *pRegisteredClientInfo;

			// najde klienta, kter�mu se pos�lala MiniMapa
			POSITION posMiniMapClient = 
				m_aRegisteredClientList[dwMiniMapClientCivilizationIndex].FindClient ( 
				pMiniMapClient );

			// pokus� se naj�t v civilizaci dal��ho klienta, kter�mu bude pos�lat MiniMapu
			if ( posMiniMapClient == NULL )
			{	// nepoda�ilo se naj�t klienta
				// zjist� klienta s indexem index "dwMiniMapClientIndex"
				posMiniMapClient = 
					m_aRegisteredClientList[dwMiniMapClientCivilizationIndex].GetHeadPosition ();
				for ( DWORD dwIndex = 0; dwIndex <= dwMiniMapClientIndex; dwIndex++ )
				{
					// z�sk� dal��ho klienta
					if ( !CSRegisteredClientList::GetNext ( posMiniMapClient, 
						pRegisteredClientInfo ) )
					{	// nepoda�ilo se z�skat dal��ho klienta
						break;
					}
					// zjist� ukazatel na klienta civilizace
					pMiniMapClient = pRegisteredClientInfo->pClient;
				}
				// zjist�, poda�ilo-li se naj�t klienta s indexem "dwMiniMapClientIndex"
				if ( dwIndex == dwMiniMapClientIndex + 1 )
				{	// poda�ilo se naj�t klienta s indexem "dwMiniMapClientIndex"
					// ukon�� hled�n� klienta civilizace
					break;
				}
				// nepoda�ilo se naj�t klienta ani podle jeho indexu
			}
			else
			{	// poda�ilo se naj�t klienta civilizace
				// vezme nalezen�ho klienta civilizace
				VERIFY ( CSRegisteredClientList::GetNext ( posMiniMapClient ) );

				// vezme dal��ho klienta civilizace
				if ( CSRegisteredClientList::GetNext ( posMiniMapClient, pRegisteredClientInfo ) )
				{	// poda�ilo se vz�t dal��ho klienta civilizace
					// zjist� ukazatel na klienta civilizace
					pMiniMapClient = pRegisteredClientInfo->pClient;
					// zjist� index klienta civilizace
					posMiniMapClient = 
						m_aRegisteredClientList[dwMiniMapClientCivilizationIndex].GetHeadPosition ();
					for ( dwMiniMapClientIndex = 0; ; dwMiniMapClientIndex++ )
					{
						// z�sk� dal��ho klienta
						VERIFY ( CSRegisteredClientList::GetNext ( posMiniMapClient, 
							pRegisteredClientInfo ) );
						// zjist�, jedn�-li se o hledan�ho klienta
						if ( pRegisteredClientInfo->pClient == pMiniMapClient )
						{	// na�el hledan�ho klienta civilizace
							// ukon�� zji��ov�n� klienta civilizace
							break;
						}
					}
					ASSERT ( dwMiniMapClientIndex > 0 );
					// ukon�� hled�n� klienta civilizace
					break;
				}
				// jednalo se o posledn�ho klienta civilizace
			}
			// vyhled� klienta z dal�� civilizace

			// projede seznamy klient� civilizac�
			for ( DWORD dwCivilizationIndex = dwMiniMapClientCivilizationIndex + 1; ; 
				dwCivilizationIndex++ )
			{
				// aktualizuje index civilizace
				dwCivilizationIndex = dwCivilizationIndex % CIVILIZATION_COUNT_MAX;

				// zjist�, jedn�-li se o po��te�n� civilizaci
				if ( dwCivilizationIndex == dwMiniMapClientCivilizationIndex )
				{	// jedn� se o po��te�n� civilizaci
					// zne�kodn� ukazatel na klienta, kter�mu se bude pos�lat MiniMapa
					pMiniMapClient = NULL;
					dwMiniMapClientIndex = 0;
					dwMiniMapClientCivilizationIndex = 0;
					// ukon�� prohled�v�n� civilizac�
					break;
				}

				// zjist�, je-li seznam klient� civilizace pr�zdn�
				if ( !m_aRegisteredClientList[dwCivilizationIndex].IsEmpty () )
				{	// seznam klient� civilizace nen� pr�zdn�
					// aktualizuje ukazatel na klienta, kter�mu se bude pos�lat MiniMapa
					posMiniMapClient = 
						m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
					VERIFY ( CSRegisteredClientList::GetNext ( posMiniMapClient, 
						pRegisteredClientInfo ) );
					// zjist� ukazatel na klienta civilizace
					pMiniMapClient = pRegisteredClientInfo->pClient;
					dwMiniMapClientIndex = 0;
					dwMiniMapClientCivilizationIndex = dwCivilizationIndex;
					// ukon�� prohled�v�n� civilizac�
					break;
				}
			}
		} while ( 0 );

	// informuje klienty o mrtv�ch jednotk�ch

		// zamkne seznam mrtv�ch jednotek pro rozes�l�n� stop infa klient�m
		VERIFY ( m_mutexDeadUnitClientStopInfoListLock.Lock () );

		// mrtv� jednotka
		CSUnit *pUnit;
		// zpracuje seznam mrtv�ch jednotek pro rozes�l�n� stop infa klient�m
		while ( m_cDeadUnitClientStopInfoList.RemoveFirst ( pUnit ) )
		{
			// zjist� ID mrtv� jednotky
			DWORD dwUnitID = pUnit->GetID ();

			// projede civilizace
			for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
				dwCivilizationIndex-- > 0; )
			{
				// informace o zaregistrovan�m klientovi
				struct SRegisteredClientInfo sClientInfo;

				// zjist�, jedn�-li se o civilizaci sleduj�c� jednotku
				if ( pUnit->m_cWatchedMapSquareChanged.GetAt ( dwCivilizationIndex ) )
				{	// jedn� se o civilizaci sleduj�c� jednotku
					// zjist� pozici prvn�ho klienta civilizace
					POSITION posClient = 
						m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
					// projede klienty civilizace
					while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
					{
						TRACE_COMMUNICATION2 ( _T("# StopInfo ( Client=%x, UnitID=%x )\n"), 
							sClientInfo.pClient, dwUnitID );
						// po�le klientovi civilizace stop info jednotky
						sClientInfo.pClient->StopInfo ( dwUnitID );
					}
				}
			}

			// sn�� po�et odkaz� na jednotku
			pUnit->Release ();
		}

		// odemkne seznam mrtv�ch jednotek pro rozes�l�n� stop infa klient�m
		VERIFY ( m_mutexDeadUnitClientStopInfoListLock.Unlock () );

	// informuje klienta o za��tku pos�l�n� MiniMapy

		// zjist�, m�-li se pos�lat MiniMapa
		if ( pMiniMapClient != NULL )
		{	// m� se pos�lat MiniMapa
			// upozorn� klienta na za��tek pos�l�n� MiniMapy
			pMiniMapClient->StartRenderingUnits ();
		}

	// roze�le klient�m civilizac� informace o jednotk�ch

		// projede v�echny bloky jednotek (od konce)
		for ( SUnitBlock *pBlock = m_pLastUnitBlock; pBlock != NULL; 
			pBlock = pBlock->pPrevious )
		{
			// ukazatel na jednotku v bloku
			CSUnit **pBlockUnit = pBlock->pUnits;
			// zamkne blok jednotek pro �ten�
			VERIFY ( pBlock->cLock.ReaderLock () );
			// projede v�echny jednotky v bloku jednotek
			for ( int nIndex = UNITS_IN_BLOCK; nIndex-- > 0; pBlockUnit++ )
			{
				// z�sk� ukazatel na jednotku
				CSUnit *pUnit = *pBlockUnit;

				// zjist�, je-li to platn� jednotka
				if ( pUnit == NULL )
				{	// nen� to platn� jednotka
					continue;
				}

				// zjist�, m�-li se pos�lat MiniMapa
				if ( pMiniMapClient != NULL )
				{	// m� se pos�lat MiniMapa
					// zjist�, je-li jednotka vid�na civilizac� klienta
					if ( pUnit->m_aCivilizationVisibility[dwMiniMapClientCivilizationIndex] > 0 )
					{	// jednotka je vid�na civilizac� klienta
						// nakresl� jednotku do MiniMapy klienta civilizace
						pMiniMapClient->RenderUnit ( pUnit->GetPosition ().x, 
							pUnit->GetPosition ().y, pUnit->m_dwCivilizationIndex, 
							pUnit->m_pUnitType->m_aMoveWidth[pUnit->GetMode ()] );
					}
				}

				// z�sk� odkaz na p��znaky zm�ny pozice jednotky p�es hranici sledovan�ch 
				//		MapSquar�
				CSCivilizationBitArray &cWatchedMapSquareChanged = 
					pUnit->m_cWatchedMapSquareChanged;

				// zjist�, zm�nila-li jednotka pozici p�es hranici sledovan�ch MapSquar�
				if ( cWatchedMapSquareChanged.IsSet () )
				{	// jednotka zm�nila pozici p�es hranici sledovan�ch MapSquar�
					// projede civilizace
					for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
						dwCivilizationIndex-- > 0; )
					{
						// zjist�, zm�nila-li jednotka pozici p�es hranici civilizac� 
						//		sledovan�ch MapSquar�
						if ( cWatchedMapSquareChanged.GetAt ( dwCivilizationIndex ) )
						{	// jednotka zm�nila pozici p�es hranici civilizac� sledovan�ch 
							//		MapSquar�
							// vynuluje p��znak zm�ny pozice p�es hranice civilizac� 
							//		sledovan�ch MapSquar�
							cWatchedMapSquareChanged.ClearAt ( dwCivilizationIndex );

							// informace o zaregistrovan�m klientovi
							struct SRegisteredClientInfo sClientInfo;

							// z�sk� MapSquare jednotky
							CSMapSquare *pUnitMapSquare = pUnit->GetMapSquare ();
							// zjist�, je-li jednotka vid�t klientem civilizace
							if ( ( pUnitMapSquare != NULL ) && 
								pUnitMapSquare->m_cWatchingCivilizations.GetAt ( dwCivilizationIndex ) && 
								( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 ) )
							{	// jednotka je v civilizac� sledovan�m MapSquaru a je vid�t
								// p�iprav� start info jednotky
								pUnit->GetStartInfo ( &sStartInfo );
								// zjist� pozici prvn�ho klienta civilizace
								POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
								// projede klienty civilizace
								while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
								{
									TRACE_COMMUNICATION2 ( _T("# StartInfo ( Client=%x, UnitID=%x )\n"), sClientInfo.pClient, sStartInfo.dwID );
									// po�le klientovi civilizace start info jednotky
									sClientInfo.pClient->StartInfo ( &sStartInfo );
								}
							}
							else
							{	// jednotka nen� v civilizac� sledovan�m MapSquaru
								// p�iprav� stop info jednotky
								DWORD dwUnitID = pUnit->GetID ();
								// zjist� pozici prvn�ho klienta civilizace
								POSITION posClient= m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
								// projede klienty civilizace
								while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
								{
									TRACE_COMMUNICATION2 ( _T("# StopInfo ( Client=%x, UnitID=%x )\n"), sClientInfo.pClient, dwUnitID );
									// po�le klientovi civilizace stop info jednotky
									sClientInfo.pClient->StopInfo ( dwUnitID );
								}
							}
							// klientovi bylo posl�no start/stop info o jednotce
						}
						// p��padn� zm�na pozici p�es hranici civilizac� sledovan�ch MapSquar� 
						//		byla obslou�ena
					}
					// v�echny civilizace byly projety
				}
				// p��padn� zm�na pozice jednotky p�es hranici sledovan�ch MapSquar� byla 
				//		obslou�ena

				// z�sk� odkaz na p��znaky zm�ny viditelnosti jednotky civilizacemi
				CSCivilizationBitArray &cCivilizationVisibilityChanged = 
					pUnit->m_cCivilizationVisibilityChanged;

				// zjist�, zm�nila-li se viditelnost jednotky civilizacemi
				if ( cCivilizationVisibilityChanged.IsSet () )
				{	// zm�nila se viditelnost jednotky civilizacemi
					// zjist�, je-li jednotka sledov�na
					if ( pUnit->m_dwWatchingCivilizationClientCount > 0 )
					{	// jednotka je sledov�na
						// zamkne seznam sledovan�ch jednotek
						VERIFY ( m_mutexWatchedUnitListLock.Lock () );

						// projede civilizace
						for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
							dwCivilizationIndex-- > 0; )
						{
							// zjist�, zm�nila-li se viditelnost jednotky civilizac�
							if ( cCivilizationVisibilityChanged.GetAt ( dwCivilizationIndex ) )
							{	// zm�nila se viditelnost jednotky civilizac�
								// vynuluje p��znak zm�ny viditelnosti jednotky civilizac�
								cCivilizationVisibilityChanged.ClearAt ( dwCivilizationIndex );

								// informace o sledovan� jednotce
								struct SWatchedUnitInfo sUnitInfo;

								// zjist� ID jednotky
								DWORD dwUnitID = pUnit->GetID ();
								// zjist� index civilizace jednotky
								DWORD dwUnitCivilizationIndex = pUnit->GetCivilizationIndex ();
								// z�sk� pozici prvn� sledovan� jednotky
								POSITION posUnit = m_cWatchedUnitList.GetHeadPosition ();

								// najde v�echny klienty sleduj�c� jednotku
								while ( m_cWatchedUnitList.FindUnit ( pUnit, posUnit ) )
								{
									// z�sk� informace o sledovan� jednotce
									VERIFY ( m_cWatchedUnitList.GetNext ( posUnit, sUnitInfo ) );
									ASSERT ( sUnitInfo.pUnit == pUnit );

									// zjist�, jedn�-li se o klienta zpracov�van� civilizace
									if ( sUnitInfo.pClient->GetCivilizationIndex () != dwCivilizationIndex )
									{	// nejedn� se o klienta zpracov�van� civilizace
										// pokra�uje v proj�d�n� klient� sleduj�c�ch jednotku
										continue;
									}

									// zjist�, je-li jednotka vid�t civilizac�
									if ( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 )
									{	// jednotka je civilizac� vid�t
										// zjist�, jedn�-li se o jednotku vlastn� civilizace
										if ( dwUnitCivilizationIndex == dwCivilizationIndex )
										{	// jedn� se o jednotku vlastn� civilizace
											TRACE_COMMUNICATION2 ( _T("# StartFullInfo ( Client=%x, UnitID=%x )\n"), sUnitInfo.pClient, sStartFullInfo.dwID );
											// nech� poslat start full info jednotky
											pUnit->SendStartFullInfo ( &sStartFullInfo, sUnitInfo.pClient );
										}
										else
										{	// jedn� se o jednotku ciz� civilizace
											TRACE_COMMUNICATION2 ( _T("# StartEnemyFullInfo ( Client=%x, UnitID=%x )\n"), sUnitInfo.pClient, sStartEnemyFullInfo.dwID );
											// nech� poslat start enemy full info jednotky
											pUnit->SendStartEnemyFullInfo ( &sStartEnemyFullInfo, sUnitInfo.pClient );
										}
									}
									else
									{	// jednotka nen� civilizac� vid�t
										// zjist�, jedn�-li se o jednotku vlastn� civilizace
										if ( dwUnitCivilizationIndex == dwCivilizationIndex )
										{	// jedn� se o jednotku vlastn� civilizace
											TRACE_COMMUNICATION2 ( _T("# StopFullInfo ( Client=%x, UnitID=%x )\n"), sUnitInfo.pClient, dwUnitID );
											// po�le stop full info jednotky
											sUnitInfo.pClient->StopFullInfo ( dwUnitID );
										}
										else
										{	// jedn� se o jednotku ciz� civilizace
											TRACE_COMMUNICATION2 ( _T("# StopEnemyFullInfo ( Client=%x, UnitID=%x )\n"), sUnitInfo.pClient, dwUnitID );
											// po�le stop enemy full info jednotky
											sUnitInfo.pClient->StopEnemyFullInfo ( dwUnitID );
										}
									}
									// klientovi bylo rozesl�no info jednotky
								}
								// v�em klient�m sleduj�c�m jednotku bylo rozesl�no info jednotky

								// z�sk� MapSquare jednotky
								CSMapSquare *pUnitMapSquare = pUnit->GetMapSquare ();
								// zjist�, je-li jednotka v klientem sledovan�m MapSquaru
								if ( ( pUnitMapSquare != NULL ) && 
									pUnitMapSquare->m_cWatchingCivilizations.GetAt ( dwCivilizationIndex ) )
								{	// jednotka je v klientem sledovan�m MapSquaru
									// informace o zaregistrovan�m klientovi
									struct SRegisteredClientInfo sClientInfo;

									// zjist�, je-li jednotka civilizac� vid�t
									if ( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 )
									{	// jednotka je civilizac� vid�t
										// p�iprav� start info jednotky
										pUnit->GetStartInfo ( &sStartInfo );
										// zjist� pozici prvn�ho klienta civilizace
										POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
										// projede klienty civilizace
										while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
										{
											TRACE_COMMUNICATION2 ( _T("# StartInfo ( Client=%x, UnitID=%x )\n"), sClientInfo.pClient, sStartInfo.dwID );
											// po�le klientovi civilizace start info jednotky
											sClientInfo.pClient->StartInfo ( &sStartInfo );
										}
									}
									else
									{	// jednotka civilizac� nen� vid�t
										// zjist� pozici prvn�ho klienta civilizace
										POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
										// projede klienty civilizace
										while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
										{
											TRACE_COMMUNICATION2 ( _T("# StopInfo ( Client=%x, UnitID=%x )\n"), sClientInfo.pClient, dwUnitID );
											// po�le klientovi civilizace start info jednotky
											sClientInfo.pClient->StopInfo ( dwUnitID );
										}
									}
									// klientovi bylo posl�no start/stop info o jednotce
								}
								// p��padn� zm�na viditelnosti jednotky civilizac� byla obslou�ena
							}
							// viditelnost jednotky civilizac� se nezm�nila
						}
						// v�echny civilizace byly projety

						// odemkne seznam sledovan�ch jednotek
						VERIFY ( m_mutexWatchedUnitListLock.Unlock () );
					}
					else
					{	// jednotka nen� sledov�na
						// projede civilizace
						for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
							dwCivilizationIndex-- > 0; )
						{
							// zjist�, zm�nila-li se viditelnost jednotky civilizac�
							if ( cCivilizationVisibilityChanged.GetAt ( dwCivilizationIndex ) )
							{	// zm�nila se viditelnost jednotky civilizac�
								// vynuluje p��znak zm�ny viditelnosti jednotky civilizac�
								cCivilizationVisibilityChanged.ClearAt ( dwCivilizationIndex );

								// z�sk� MapSquare jednotky
								CSMapSquare *pUnitMapSquare = pUnit->GetMapSquare ();
								// zjist�, je-li jednotka v klientem sledovan�m MapSquaru
								if ( ( pUnitMapSquare != NULL ) && 
									pUnitMapSquare->m_cWatchingCivilizations.GetAt ( dwCivilizationIndex ) )
								{	// jednotka je v klientem sledovan�m MapSquaru
									// informace o zaregistrovan�m klientovi
									struct SRegisteredClientInfo sClientInfo;

									// zjist�, je-li jednotka civilizac� vid�t
									if ( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 )
									{	// jednotka je civilizac� vid�t
										// p�iprav� start info jednotky
										pUnit->GetStartInfo ( &sStartInfo );
										// zjist� pozici prvn�ho klienta civilizace
										POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
										// projede klienty civilizace
										while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
										{
											TRACE_COMMUNICATION2 ( _T("# StartInfo ( Client=%x, UnitID=%x )\n"), sClientInfo.pClient, sStartInfo.dwID );
											// po�le klientovi civilizace start info jednotky
											sClientInfo.pClient->StartInfo ( &sStartInfo );
										}
									}
									else
									{	// jednotka civilizac� nen� vid�t
										// p�iprav� stop info jednotky
										DWORD dwUnitID = pUnit->GetID ();
										// zjist� pozici prvn�ho klienta civilizace
										POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
										// projede klienty civilizace
										while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
										{
											TRACE_COMMUNICATION2 ( _T("# StopInfo ( Client=%x, UnitID=%x )\n"), sClientInfo.pClient, dwUnitID );
											// po�le klientovi civilizace start info jednotky
											sClientInfo.pClient->StopInfo ( dwUnitID );
										}
									}
									// klientovi bylo posl�no start/stop info o jednotce
								}
								// p��padn� zm�na viditelnosti jednotky civilizac� byla obslou�ena
							}
							// viditelnost jednotky civilizac� se nezm�nila
						}
						// v�echny civilizace byly projety
					}
					// sledovan� a nesledovan� jednotka je obslou�ena
				}
				// p��padn� zm�na viditelnosti jednotky civilizacemi byla obslou�ena

				// zjist�, jedn�-li se o sledovanou jednotku
				if ( pUnit->m_dwWatchingCivilizationClientCount > 0 )
				{	// jedn� se o sledovanou jednotku
					// zjist� p��znak zm�ny full infa
					BOOL bFullInfoModified = pUnit->IsFullInfoModified ();
					// zjist�, zm�nilo-li se full info jednotky
					if ( bFullInfoModified )
					{	// full info jednotky se zm�nilo
						// sma�e p��znak zm�ny full infa jednotky
						pUnit->ClearFullInfoModified ();
					}

					// zamkne seznam sledovan�ch jednotek
					VERIFY ( m_mutexWatchedUnitListLock.Lock () );

					// informace o sledovan� jednotce
					struct SWatchedUnitInfo sUnitInfo;

					// z�sk� pozici prvn� sledovan� jednotky
					POSITION posUnit = m_cWatchedUnitList.GetHeadPosition ();

					// najde v�echny klienty sleduj�c� jednotku
					while ( m_cWatchedUnitList.FindUnit ( pUnit, posUnit ) )
					{
						// z�sk� informace o sledovan� jednotce
						VERIFY ( m_cWatchedUnitList.GetNext ( posUnit, sUnitInfo ) );
						ASSERT ( sUnitInfo.pUnit == pUnit );

						// zjist� index civilizace klienta civilizace
						DWORD dwClientCivilizationIndex = sUnitInfo.pClient->GetCivilizationIndex ();

						// zjist�, je-li jednotka vid�na civilizac�
						if ( pUnit->m_aCivilizationVisibility[dwClientCivilizationIndex] > 0 )
						{	// jednotka je vid�na civilizac� klienta
							// zjist�, jedn�-li se o jednotku vlastn� civilizace
							if ( pUnit->GetCivilizationIndex () == dwClientCivilizationIndex )
							{	// jedn� se o jednotku vlastn� civilizace
								TRACE_COMMUNICATION2 ( _T("# FullInfo ( Client=%x, UnitID=%x )\n"), sUnitInfo.pClient, sFullInfo.dwID );
								// nech� poslat full info jednotky
								pUnit->SendFullInfo ( &sFullInfo, sUnitInfo.pClient );
							}
							else
							{	// jedn� se o jednotku ciz� civilizace
								TRACE_COMMUNICATION2 ( _T("# EnemyFullInfo ( Client=%x, UnitID=%x )\n"), sUnitInfo.pClient, sEnemyFullInfo.dwID );
								// nech� poslat enemy full info jednotky
								pUnit->SendEnemyFullInfo ( &sEnemyFullInfo, sUnitInfo.pClient );
							}
						}
						// info o p��padn� vid�n� jednotce bylo rozesl�no
					}
					// v�em klient�m sleduj�c�m jednotku bylo rozesl�no full info jednotky

					// odemkne seznam sledovan�ch jednotek
					VERIFY ( m_mutexWatchedUnitListLock.Unlock () );
				}
				else
				{	// jednotka nen� sledov�na
					// sma�e p��znak zm�ny full infa jednotky
					pUnit->ClearFullInfoModified ();
				}
				// zm�na full infa jednotky byla obslou�ena

				// zjist�, zm�nily-li b�n� informace jednotky pro klienta
				if ( pUnit->IsCommonClientInfoModified () )
				{	// b�n� informace jednotky pro klienta se zm�nily
					// z�sk� MapSquare jednotky
					CSMapSquare *pUnitMapSquare = pUnit->GetMapSquare ();
					// ukazatel na bitov� pole civilizac� sleduj�ch MapSquare, ve kter�m 
					//		se jednotka nach�z�
					CSCivilizationBitArray *pWatchingCivilizations;
					// p��znak, je-li MapSquare sledov�n n�kterou civilizac�
					BOOL bWatchingMapSquare = ( ( pUnitMapSquare != NULL ) && 
						(pWatchingCivilizations = &pUnitMapSquare->m_cWatchingCivilizations)->IsSet () );

					// zjist�, zm�nilo-li se brief info jednotky
					if ( pUnit->IsBriefInfoModified () )
					{	// zm�nilo se brief info jednotky
						// sma�e p��znak zm�ny brief infa jednotky
						pUnit->ClearBriefInfoModified ();
						// zjist�, je-li MapSquare sledov�n n�kterou civilizac�
						if ( bWatchingMapSquare )
						{	// MapSquare je sledov�n n�kterou civilizac�
							// p�iprav� brief info jednotky
							pUnit->GetBriefInfo ( &sBriefInfo );
							// projede civilizace
							for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
								dwCivilizationIndex-- > 0; )
							{
								// zjist�, je-li MapSquare sledov�n civilizac�
								if ( pWatchingCivilizations->GetAt ( dwCivilizationIndex ) )
								{	// jednotka je v klientem sledovan�m MapSquaru
									// zjist�, je-li jednotka civilizac� vid�t
									if ( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 )
									{	// jednotka je civilizac� vid�t
										// informace o zaregistrovan�m klientovi
										struct SRegisteredClientInfo sClientInfo;

										// zjist� pozici prvn�ho klienta civilizace
										POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
										// projede klienty civilizace
										while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
										{
											TRACE_COMMUNICATION2 ( _T("# BriefInfo ( Client=%x, UnitID=%x )\n"), sClientInfo.pClient, sBriefInfo.dwID );
											// po�le klientovi civilizace brief info jednotky
											sClientInfo.pClient->BriefInfo ( &sBriefInfo );
										}
										// v�ichni klienti civilizace byli projeti
									}
									// p��padn� informace o viditeln� jednotce byly odesl�ny
								}
								// MapSquare nen� civilizac� sledov�n
							}
							// v�echny civilizace byly projety
						}
						// p��padn� sledov�n� MapSquaru civilizac� bylo obslou�eno
					}
					// p��padn� zm�na brief infa byla obslou�ena

					// zjist�, zm�nilo-li se check point info jednotky
					if ( pUnit->IsCheckPointInfoModified () )
					{	// zm�nilo se check point info jednotky
						// sma�e p��znak zm�ny check point infa jednotky
						pUnit->ClearCheckPointInfoModified ();
						// zjist�, je-li MapSquare sledov�n n�kterou civilizac�
						if ( bWatchingMapSquare )
						{	// MapSquare je sledov�n n�kterou civilizac�
							// p�iprav� check point info jednotky
							pUnit->GetCheckPointInfo ( &sCheckPointInfo );
							// projede civilizace
							for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; 
								dwCivilizationIndex-- > 0; )
							{
								// zjist�, je-li MapSquare sledov�n civilizac�
								if ( pWatchingCivilizations->GetAt ( dwCivilizationIndex ) )
								{	// jednotka je v klientem sledovan�m MapSquaru
									// zjist�, je-li jednotka civilizac� vid�t
									if ( pUnit->m_aCivilizationVisibility[dwCivilizationIndex] > 0 )
									{	// jednotka je civilizac� vid�t
										// informace o zaregistrovan�m klientovi
										struct SRegisteredClientInfo sClientInfo;

										// zjist� pozici prvn�ho klienta civilizace
										POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
										// projede klienty civilizace
										while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
										{
											TRACE_COMMUNICATION2 ( _T("# CheckPointInfo ( Client=%x, UnitID=%x )\n"), sClientInfo.pClient, sCheckPointInfo.dwID );
											// po�le klientovi civilizace check point info jednotky
											sClientInfo.pClient->CheckPointInfo ( &sCheckPointInfo );
										}
										// v�ichni klienti civilizace byli projeti
									}
									// p��padn� informace o viditeln� jednotce byly odesl�ny
								}
								// MapSquare nen� civilizac� sledov�n
							}
							// v�echny civilizace byly projety
						}
						// p��padn� sledov�n� MapSquaru civilizac� bylo obslou�eno
					}
					// p��padn� zm�na check point infa byla obslou�ena
				}
				// klientovi byly odesl�ny v�echny informace o jednotce
			}
			// byly projety v�echny jednotky v bloku jednotek

			// odemkne zam�en� blok jednotek pro �ten�
			pBlock->cLock.ReaderUnlock ();
		}
		// byly projety v�echny bloky jednotek

	// informuje klienta o ukon�en� pos�l�n� MiniMapy

		// zjist�, m�-li se pos�lat MiniMapa
		if ( pMiniMapClient != NULL )
		{	// m� se pos�lat MiniMapa
			// upozorn� klienta na za��tek pos�l�n� MiniMapy
			pMiniMapClient->StopRenderingUnits ();
		}

	// ukon�� rozes�l�n� informac� o jednotk�ch

		// upozorn� klienty civilizac� na konec rozes�l�n� informac� o jednotk�ch
		{
			// projede civilizace
			for ( DWORD dwCivilizationIndex = m_dwCivilizationCount; dwCivilizationIndex-- > 0; )
			{
				// informace o zaregistrovan�m klientovi
				struct SRegisteredClientInfo sClientInfo;

				// zjist� pozici prvn�ho klienta civilizace
				POSITION posClient = m_aRegisteredClientList[dwCivilizationIndex].GetHeadPosition ();
				// projede klienty civilizace
				while ( CSRegisteredClientList::GetNext ( posClient, sClientInfo ) )
				{
					TRACE_COMMUNICATION1 ( _T("#		StopSendingInfo ( Client=%x )\n"), sClientInfo.pClient );
					// upozorn� klienta na konec rozes�l�n� informac� o jednotk�ch
					sClientInfo.pClient->StopSendingInfo ();
				}
			}
		}

		// odemkne seznamy zaregistrovan�ch klient� civilizac� pro �ten�
		m_cRegisteredClientListLock.ReaderUnlock ();
	}
	// skon�il hlavn� cyklus rozes�l�n� informac� klient�m civilizac�
}

//////////////////////////////////////////////////////////////////////
// Operace s u�ivateli
//////////////////////////////////////////////////////////////////////

// zjist� po�et u�ivatel� (lze volat jen z aplikace serveru)
DWORD CSMap::GetUserCount () 
{
	// vr�t� po�et u�ivatel�
	return m_aUserDescriptions.GetSize ();
}

// vypln� informace o u�ivateli ��slo "dwUserIndex" a vr�t� identifikaci u�ivatele 
//		"dwUserID", kter� je platn� do smaz�n� u�ivatele (lze volat jen z aplikace 
//		serveru)
void CSMap::GetUser ( DWORD dwUserIndex, CString &strName, CString &strPassword, 
	DWORD &dwCivilizationIndex, DWORD &dwUserID ) 
{
	ASSERT ( dwUserIndex < (DWORD)m_aUserDescriptions.GetSize () );

	// zjist� popis u�ivatele
	struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( 
		(int)dwUserIndex );

	// vypln� ID u�ivatele
	dwUserID = (DWORD)pUserDescription;

	// vypln� informace o u�ivateli
	strName = pUserDescription->strName;
	strPassword = pUserDescription->strPassword;
	dwCivilizationIndex = pUserDescription->dwCivilizationIndex;
}

// vypln� informace o u�ivateli s identifikac� "dwUserID" (lze volat jen z aplikace 
//		serveru)
void CSMap::GetUser ( DWORD dwUserID, CString &strName, CString &strPassword, 
	DWORD &dwCivilizationIndex ) 
{
	// najde u�ivatele s identifikac� "dwUserID"
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjist�, neshoduje-li se identifikace u�ivatele
		if ( (DWORD)m_aUserDescriptions.GetAt ( nIndex ) == dwUserID )
		{	// identifikace u�ivatele se shoduje
			// zjist� popis u�ivatele
			struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );

			// vypln� informace o u�ivateli
			strName = pUserDescription->strName;
			strPassword = pUserDescription->strPassword;
			dwCivilizationIndex = pUserDescription->dwCivilizationIndex;

			// ukon�� z�sk�n� informac� o u�ivateli
			return;
		}
	}
	// u�ivatel nebyl nalezen
	ASSERT ( FALSE );
}

// vr�t� TRUE pokud je u�ivatel povolen
BOOL CSMap::IsUserEnabled ( DWORD dwUserID )
{
	// najde u�ivatele s identifikac� "dwUserID"
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjist�, neshoduje-li se identifikace u�ivatele
		if ( (DWORD)m_aUserDescriptions.GetAt ( nIndex ) == dwUserID )
		{	// identifikace u�ivatele se shoduje
			// zjist� popis u�ivatele
			struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );

			// zjist� zda je u�ivatel povolen
			return ( pUserDescription->bEnabled );
		}
	}
	// u�ivatel nebyl nalezen
	ASSERT ( FALSE );

	return FALSE;
}

// vr�t� TRUE pokud je u�ivatel nalogov�n
BOOL CSMap::IsUserLoggedOn ( DWORD dwUserID )
{
	// najde u�ivatele s identifikac� "dwUserID"
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjist�, neshoduje-li se identifikace u�ivatele
		if ( (DWORD)m_aUserDescriptions.GetAt ( nIndex ) == dwUserID )
		{	// identifikace u�ivatele se shoduje
			// zjist� popis u�ivatele
			struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );

			// zjist� zda je u�ivatel nalogov�n
			return ( pUserDescription->bLoggedOn );
		}
	}
	// u�ivatel nebyl nalezen
	ASSERT ( FALSE );

	return FALSE;
}

// povol� u�ivatele
void CSMap::EnableUser ( DWORD dwUserID )
{
	// najde u�ivatele s identifikac� "dwUserID"
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjist�, neshoduje-li se identifikace u�ivatele
		if ( (DWORD)m_aUserDescriptions.GetAt ( nIndex ) == dwUserID )
		{	// identifikace u�ivatele se shoduje
			// zjist� popis u�ivatele
			struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );

			// povol� u�ivatele
			pUserDescription->bEnabled = TRUE;

			return;
		}
	}
	// u�ivatel nebyl nalezen
	ASSERT ( FALSE );
}

// zak�z�n� u�ivatele
void CSMap::DisableUser ( DWORD dwUserID )
{
	// najde u�ivatele s identifikac� "dwUserID"
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjist�, neshoduje-li se identifikace u�ivatele
		if ( (DWORD)m_aUserDescriptions.GetAt ( nIndex ) == dwUserID )
		{	// identifikace u�ivatele se shoduje
			// zjist� popis u�ivatele
			struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );

			// zak�e u�ivatele
			pUserDescription->bEnabled = FALSE;

			// odpoj� u�ivatele pokud je p�ipojen
			CSCivilization *pCiv = GetCivilization( pUserDescription->dwCivilizationIndex );
			pCiv->m_pZCivilization->DisconnectUser( pUserDescription->strName );

			return;
		}
	}
	// u�ivatel nebyl nalezen
	ASSERT ( FALSE );
}

// zm�n� informace o u�ivateli s identifikac� "dwUserID" (FALSE=duplicitn� jm�no) (lze 
//		volat jen z aplikace serveru)
BOOL CSMap::EditUser ( DWORD dwUserID, LPCTSTR lpcszName, LPCTSTR lpcszPassword, 
	DWORD dwCivilizationIndex ) 
{
	ASSERT ( strlen ( lpcszName ) > 0 );
	ASSERT ( dwCivilizationIndex < m_dwCivilizationCount );

	// zjist� popis u�ivatele
	struct SUserDescription *pUserDescription = (SUserDescription *)dwUserID;

	// zjist�, bylo-li zm�n�no jm�no
	if ( pUserDescription->strName != lpcszName )
	{	// bylo zm�n�no jm�no u�ivatele
		// zjist�, je-li jm�no u�ivatele jednozna�n�
		for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
		{
			// zjist�, neshoduje-li se jm�no u�ivatele
			if ( m_aUserDescriptions.GetAt ( nIndex )->strName == lpcszName )
			{	// jm�no u�ivatele se shoduje
				// vr�t� p��znak duplicitn�ho u�ivatele
				return FALSE;
			}
		}
	}

	// zamkne pole popis� u�ivatel�
	VERIFY ( m_mutexUserDescriptionsWriterLock.Lock () );

	// aktualizuje informace o u�ivateli
	pUserDescription->strName = lpcszName;
	pUserDescription->strPassword = lpcszPassword;
	pUserDescription->dwCivilizationIndex = dwCivilizationIndex;

	// odemkne pole popis� u�ivatel�
	VERIFY ( m_mutexUserDescriptionsWriterLock.Unlock () );

	// vr�t� p��znak �sp�chu
	return TRUE;
}

// p�id� civilizaci "dwCivilizationIndex" u�ivatele se jm�nem "lpcszName" a heslem 
//		"lpcszPassword", p�id�-li u�ivatele, vypln� jeho identifikaci "dwUserID", kter� 
//		je platn� do smaz�n� u�ivatele (FALSE=duplicitn� u�ivatel) (lze volat jen 
//		z aplikace serveru)
BOOL CSMap::AddUser ( LPCTSTR lpcszName, LPCTSTR lpcszPassword, 
	DWORD dwCivilizationIndex, DWORD &dwUserID ) 
{
	ASSERT ( strlen ( lpcszName ) > 0 );
	ASSERT ( dwCivilizationIndex < m_dwCivilizationCount );

	// zjist�, je-li jm�no u�ivatele jednozna�n�
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjist�, neshoduje-li se jm�no u�ivatele
		if ( m_aUserDescriptions.GetAt ( nIndex )->strName == lpcszName )
		{	// jm�no u�ivatele se shoduje
			// vr�t� p��znak duplicitn�ho u�ivatele
			return FALSE;
		}
	}

	// vytvo�� nov� popis u�ivatele
	struct SUserDescription *pUserDescription = new SUserDescription;

	// vypln� identifikaci u�ivatele
	dwUserID = (DWORD)pUserDescription;

	// inicializuje popis u�ivatele
	pUserDescription->strName = lpcszName;
	pUserDescription->strPassword = lpcszPassword;
	pUserDescription->dwCivilizationIndex = dwCivilizationIndex;
	pUserDescription->bEnabled = TRUE;
	pUserDescription->bLoggedOn = FALSE;

	// zamkne pole popis� u�ivatel�
	VERIFY ( m_mutexUserDescriptionsWriterLock.Lock () );

	// p�id� popis u�ivatele do tabulky u�ivatel�
	m_aUserDescriptions.Add ( pUserDescription );

	// odemkne pole popis� u�ivatel�
	VERIFY ( m_mutexUserDescriptionsWriterLock.Unlock () );

	// vr�t� p��znak �sp�chu
	return TRUE;
}

// sma�e u�ivatele s identifikac� "dwUserID" (identifikace je d�le neplatn�) (lze 
//		volat jen z aplikace serveru)
void CSMap::DeleteUser ( DWORD dwUserID ) 
{
	// najde u�ivatele s identifikac� "dwUserID"
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjist�, neshoduje-li se identifikace u�ivatele
		if ( (DWORD)m_aUserDescriptions.GetAt ( nIndex ) == dwUserID )
		{	// identifikace u�ivatele se shoduje
			// zamkne pole popis� u�ivatel�
			VERIFY ( m_mutexUserDescriptionsWriterLock.Lock () );

			// sma�e u�ivatele z pole popis� u�ivatel�
			m_aUserDescriptions.RemoveAt ( nIndex );
			// zni�� popis u�ivatele
			delete (SUserDescription *)dwUserID;

			// odemkne pole popis� u�ivatel�
			VERIFY ( m_mutexUserDescriptionsWriterLock.Unlock () );

			// ukon�� maz�n� u�ivatele
			return;
		}
	}
	// u�ivatel nebyl nalezen
	ASSERT ( FALSE );
}

// najde u�ivatele "lpcszName" s heslem "lpcszPassword", vypln� civilizaci u�ivatele 
//		"pZCivilization" a ID u�ivatele "dwUserID" (je-li u�ivatel nalezen), vrac�: UE_OK, 
//		UE_BadName, UE_BadPassword, UE_UserDisabled
CSMap::EUserError CSMap::FindUser ( LPCTSTR lpcszName, LPCTSTR lpcszPassword, 
		CZCivilization *&pZCivilization, DWORD &dwUserID ) 
{
	// zamkne pole popis� u�ivatel�
	VERIFY ( m_mutexUserDescriptionsWriterLock.Lock () );

	// n�vratov� chybov� k�d u�ivatele
	EUserError eError = UE_BadName;

	// zne�kodn� ukazatel na civilizaci civilizace
	pZCivilization = NULL;

	// projede pole u�ivatel�
	for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
	{
		// zjist� popis u�ivatele
		struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );

		// zjist�, neshoduje-li se jm�no u�ivatele
		if ( pUserDescription->strName == lpcszName )
		{	// jm�no u�ivatele se shoduje
			// zkontroluje zda u�ivatel nen� zak�z�n
			if ( !pUserDescription->bEnabled )
			{	// u�ivatel je zak�z�n
				eError = UE_UserDisabled;
			}
			else
			{	// u�ivatel nen� zak�z�n
				// zjist�, shoduje-li se heslo
				eError = ( pUserDescription->strPassword == lpcszPassword ) ? UE_OK : 
					UE_BadPassword;
			}
			// vypln� civilizaci u�ivatele na civilizaci
			ASSERT ( pUserDescription->dwCivilizationIndex < m_dwCivilizationCount );
			pZCivilization = m_pCivilizations[pUserDescription->dwCivilizationIndex].m_pZCivilization;
			ASSERT ( pZCivilization != NULL );
			// vypln� ID u�ivatele
			dwUserID = (DWORD)pUserDescription;
			// ukon�� prohled�v�n� pole u�ivatel�
			break;
		}
	}

	// odemkne pole popis� u�ivatel�
	VERIFY ( m_mutexUserDescriptionsWriterLock.Unlock () );

	// vr�t� p��znak chybn�ho jm�na u�ivatele
	return eError;
}

// za�ne logov�n� u�ivatele (FALSE=hra neb��, u�ivatel se nem��e nalogovat)
BOOL CSMap::StartUserLogin () 
{
	// zamkne z�mek p��znaku povolen� logov�n� u�ivatel�
	VERIFY ( m_cUserLoginAllowedLock.ReaderLock () );

	// zjist� m��e-li se u�ivatel nalogovat
	if ( !m_bUserLoginAllowed )
	{	// u�ivatel se nem��e nalogovat
		// odemkne z�mek p��znaku povolen� logov�n� u�ivatel�
		m_cUserLoginAllowedLock.ReaderUnlock ();
		// vr�t� p��znak ne�sp�n�ho za��tku logov�n�
		return FALSE;
	}
	else
	{	// u�ivatel se m��e nalogovat
		// vr�t� p��znak za��tku logov�n� u�ivatele
		return TRUE;
	}
}

// ukon�� logov�n� u�ivatele
void CSMap::FinishUserLogin () 
{
	// odemkne z�mek p��znaku povolen� logov�n� u�ivatel�
	m_cUserLoginAllowedLock.ReaderUnlock ();
}

// u�ivatel je nalogov�n (vol�no po nalogov�n�, p�ed vol�n�m FinishUserLogin)
void CSMap::UserLoggedOn ( LPCTSTR lpcszName ) 
{
	DWORD dwUserID = 0;

	// Nastavit p��znak o nalogov�n�
	{
		// najde u�ivatele s jm�ne "lpcszName"
		for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
		{
			struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );
			// zjist�, neshoduje-li se identifikace u�ivatele
			if ( pUserDescription->strName.Compare(lpcszName) == 0 )
			{
				// nastav� p��znak
				pUserDescription->bLoggedOn = TRUE;

				dwUserID = (DWORD)pUserDescription;

				break;
			}
		}
	}

	if ( dwUserID == 0 ) return;

	// Zavol� notifikace
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

// u�ivatel je odlogov�n (vol�no po odlogov�n�)
void CSMap::UserLoggedOff ( LPCTSTR lpcszName ) 
{
	DWORD dwUserID;

	// Nastavit p��znak o nalogov�n�
	{
		// najde u�ivatele s jm�ne "lpcszName"
		for ( int nIndex = m_aUserDescriptions.GetSize (); nIndex-- > 0; )
		{
			struct SUserDescription *pUserDescription = m_aUserDescriptions.GetAt ( nIndex );
			// zjist�, neshoduje-li se identifikace u�ivatele
			if ( pUserDescription->strName.Compare(lpcszName) == 0 )
			{
				// nastav� p��znak
				pUserDescription->bLoggedOn = FALSE;

				dwUserID = (DWORD)pUserDescription;

				break;
			}
		}
	}

	if ( dwUserID == 0 ) return;

	// Zavol� notifikace
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

// P�id� observer do spoj�ku observer�
void CSMap::RegisterMapObserver ( CSMap::CSMapObserver *pMapObserver )
{
	ASSERT ( pMapObserver != NULL );
	ASSERT ( pMapObserver->m_pNextObserver == NULL );

	VERIFY ( m_cMapObserverListLock.Lock ( ) );

	// P�id� observer do spoj�ku
	pMapObserver->m_pNextObserver = m_pFirstMapObserver;
	m_pFirstMapObserver = pMapObserver;

	VERIFY ( m_cMapObserverListLock.Unlock ( ) );
}

// Odebere observer ze spoj�ku observer�
void CSMap::UnregisterMapObserver ( CSMap::CSMapObserver *pMapObserver )
{
	ASSERT ( pMapObserver != NULL );

	VERIFY ( m_cMapObserverListLock.Lock ( ) );

	// Najde ho ve spoj�ku
	CSMapObserver *pObserver, **ppPointer;
	ppPointer = &m_pFirstMapObserver;
	pObserver = *ppPointer;
	while ( pObserver != NULL )
	{
		if ( pObserver == pMapObserver )
		{	// Nalezen
			// Odebrat ze spoj�ku
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
// Debuggovac� informace
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// zkontroluje spr�vn� data objektu (TRUE=OK)
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

	// zkontroluje, neb��-li prov�d�n� DPC
	ASSERT ( m_pPreparingDPC == NULL );

	ASSERT ( m_dwMainLoopThreadID != DWORD_MAX );

	return TRUE;
}

// zkontroluje ne�kodn� data objektu (TRUE=OK)
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
