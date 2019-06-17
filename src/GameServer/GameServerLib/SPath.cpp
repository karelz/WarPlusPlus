/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída cesty na serveru hry
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SPath.h"

#include "SMap.h"
#include "SFindPathGraph.h"

#include "Common\Map\MFindPathGraphs.h"

#include <math.h>

IMPLEMENT_DYNAMIC ( CSPath, CPersistentObject )

//////////////////////////////////////////////////////////////////////
// Informace o cestách
//////////////////////////////////////////////////////////////////////

// zámek front požadavkù na hledání cesty
CMutex CSPath::m_mutexFindPathRequestQueueLock;
// fronty požadavkù na hledání cesty
CSFindPathRequestQueue CSPath::m_aFindPathRequestQueue[CIVILIZATION_COUNT_MAX];
// událost požadavku na hledání cesty
CEvent CSPath::m_eventFindPathRequest;
// zámek seznamù pozic kontrolních bodù cesty
//CMutex CSPath::m_mutexCheckPointPositionListLock;
// seznam smazaných cest
CSelfPooledList<CSPath *> CSPath::m_cDeletedPathList ( 20 );
// zámek seznamu smazaných cest
CMutex CSPath::m_mutexDeletedPathListLock;

//////////////////////////////////////////////////////////////////////
// Informace o vláknech výpoètu serveru hry
//////////////////////////////////////////////////////////////////////

// událost zapausování FindPathLoop threadu
CEvent CSPath::m_eventFindPathLoopPaused;

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSPath::CSPath () 
{
	m_eState = EPS_Uninitialized;
}

// destruktor (lze volat jen z FindPathLoop threadu)
CSPath::~CSPath () 
{
	ASSERT ( g_cMap.m_pFindPathLoopThread->m_nThreadID == GetCurrentThreadId () );

	// zamkne zámek seznamù pozic kontrolních bodù cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Lock () );

	// znièí seznam smazaných pozic kontrolních bodù cesty
	DeleteDeletedCheckPointPositionList ();

	// znièí seznam pozic kontrolních bodù cesty
	m_cCheckPointPositionList.RemoveAll ();

	// odemkne zámek seznamù pozic kontrolních bodù cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Unlock () );
}

//////////////////////////////////////////////////////////////////////
// Inicializace a znièení dat cest
//////////////////////////////////////////////////////////////////////

// vytvoøí data cest
void CSPath::CreatePathes ( CDataArchive cMapArchive ) 
{
}

// znièí data cest
void CSPath::DeletePathes () 
{
	// požadavek na hledání cesty
	SFindPathRequestInfo sFindPathRequestInfo;

	// znièí fronty požadavkù na hledání cesty
	for ( DWORD dwCivilizationIndex = CIVILIZATION_COUNT_MAX; 
		dwCivilizationIndex-- > 0; )
	{
		// znièí frontu požadavkù na hledání cesty
		m_aFindPathRequestQueue[dwCivilizationIndex].RemoveAll ();
	}

	// ukazatel na smazanou cestu
	CSPath *pDeletedPath;

	// znièí smazané cesty
	while ( m_cDeletedPathList.RemoveFirst ( pDeletedPath ) )
	{
		// znièí smazanou cestu
		delete pDeletedPath;
	}
}

//////////////////////////////////////////////////////////////////////
// Inicializace a znièení dat objektu
//////////////////////////////////////////////////////////////////////

// vrátí novou kopii cesty (lze volat jen z FindPathLoop threadu)
CSPath *CSPath::Duplicate () 
{
	ASSERT ( g_cMap.m_pFindPathLoopThread->m_nThreadID == GetCurrentThreadId () );

	ASSERT ( ( m_eState == EPS_Valid ) || ( m_eState == EPS_Found ) || 
		( m_eState == EPS_NotExists ) || ( m_eState == EPS_Blocked ) );

	// vytvoøí nový objekt cesty
	CSPath *pPath = new CSPath;

	// zkopíruje informace o cestì
	pPath->m_eState = m_eState;

	// zamkne zámek seznamù pozic kontrolních bodù cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Lock () );

	// znièí seznam smazaných pozic kontrolních bodù cesty
	DeleteDeletedCheckPointPositionList ();

	// pozice v seznamu pozic kontrolních bodù cesty
	POSITION posCheckPointPosition = m_cCheckPointPositionList.GetHeadPosition ();
	// pozice kontrolního bodu cesty
	CPointDW pointCheckPointPosition;

	// projede seznam kontrolních bodù cesty
	while ( m_cCheckPointPositionList.GetNext ( posCheckPointPosition, 
		pointCheckPointPosition ) )
	{
		// pøidá do seznamu smazaných kontrolních bodù nové cesty další kontrolní bod 
		//		kopírované cesty
		pPath->m_cDeletedCheckPointPositionList.Add ( pointCheckPointPosition );
	}

#ifdef _DEBUG
	// poèet kontrolních bodù cesty
	DWORD dwCheckPointPositionCount = m_cCheckPointPositionList.GetSize ();
#endif //_DEBUG

	// odemkne zámek seznamù pozic kontrolních bodù cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Unlock () );

// pøevrátí seznam zkopírovaných kontrolních bodù

	// zjistí pozici v seznamu pozic zkopírovaných kontrolních bodù cesty
	posCheckPointPosition = pPath->m_cDeletedCheckPointPositionList.GetHeadPosition ();

	// pøesune zkopírované kontrolní body do seznamu kontrolních bodù cesty
	while ( !pPath->m_cDeletedCheckPointPositionList.IsEmpty () )
	{
		ASSERT ( posCheckPointPosition == 
			pPath->m_cDeletedCheckPointPositionList.GetHeadPosition () );
		// pøesune kontrolní bod do seznamu kontrolních bodù cesty
		pPath->m_cDeletedCheckPointPositionList.Move ( posCheckPointPosition, 
			pPath->m_cCheckPointPositionList );
	}

	ASSERT ( pPath->m_cCheckPointPositionList.GetSize () == dwCheckPointPositionCount );

	// vrátí vytvoøenou kopii cestu
	return pPath;
}

// znièí cestu civilizace "dwCivilizationIndex"
void CSPath::Destroy ( DWORD dwCivilizationIndex ) 
{
	// zamkne seznam smazaných cest
	VERIFY ( m_mutexDeletedPathListLock.Lock () );

	// pøidá cestu do seznamu smazaných cest
	m_cDeletedPathList.Add ( this );

	// odemkne seznam smazaných cest
	VERIFY ( m_mutexDeletedPathListLock.Unlock () );
}

// znièí cestu
void CSPath::Destroy () 
{
	// zamkne seznam smazaných cest
	VERIFY ( m_mutexDeletedPathListLock.Lock () );

	// pøidá cestu do seznamu smazaných cest
	m_cDeletedPathList.Add ( this );

	// odemkne seznam smazaných cest
	VERIFY ( m_mutexDeletedPathListLock.Unlock () );
}

//////////////////////////////////////////////////////////////////////
// Ukládání dat objektu (obdaba CPersistentObject metod)
//////////////////////////////////////////////////////////////////////

/*
CSPath stored

	enum EPathState m_eState
	DWORD dwCheckPointPositionCount		// mùže být i 0
	[dwCheckPointPositionCount]
		DWORD dwX
		DWORD dwY
*/

// ukládání dat cesty
void CSPath::PersistentSave ( CPersistentStorage &storage ) 
{
	ASSERT ( ( m_eState == EPS_Valid ) || ( m_eState == EPS_Found ) || 
		( m_eState == EPS_NotExists ) || ( m_eState == EPS_Blocked ) || 
		( m_eState == EPS_Searching ) );

	BRACE_BLOCK ( storage );

	// znièí seznam smazaných pozic kontrolních bodù cesty
	DeleteDeletedCheckPointPositionList ();

	// uloží stav cesty
	storage << (int)m_eState;

	// uloží velikost seznamu pozic kontrolních bodù cesty
	storage << m_cCheckPointPositionList.GetSize ();

	// pozice v seznamu pozic kontrolních bodù cesty
	POSITION posCheckPointPosition = m_cCheckPointPositionList.GetHeadPosition ();
	// pozice kontrolního bodu cesty
	CPointDW pointCheckPointPosition;

	// projede seznam kontrolních bodù cesty
	while ( m_cCheckPointPositionList.GetNext ( posCheckPointPosition, 
		pointCheckPointPosition ) )
	{
		// uloží souøadnice kontrolního bodu cesty
		storage << pointCheckPointPosition.x;
		storage << pointCheckPointPosition.y;
	}
	// uložil seznam kontrolních bodù cesty
}

// nahrávání pouze uložených dat cesty (vrací nahraný objekt cesty)
CSPath *CSPath::PersistentLoadPath ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	// vytvoøí nový objekt cesty
	CSPath *pPath = new CSPath;

	// nahraje uložená data cesty
	pPath->PersistentLoad ( storage );

	// vrátí vytvoøenou cestu
	return pPath;
}

// nahrání uložených dat cesty
void CSPath::PersistentLoad ( CPersistentStorage &storage ) 
{
	ASSERT ( m_eState == EPS_Uninitialized );
	ASSERT ( m_cCheckPointPositionList.IsEmpty () );
	ASSERT ( m_cDeletedCheckPointPositionList.IsEmpty () );

	// naète stav cesty
	storage >> (int &)m_eState;
	LOAD_ASSERT ( ( m_eState == EPS_Valid ) || ( m_eState == EPS_Found ) || 
		( m_eState == EPS_NotExists ) || ( m_eState == EPS_Blocked ) || 
		( m_eState == EPS_Searching ) );

	// naète velikost seznamu pozic kontrolních bodù cesty
	DWORD dwCheckPointPositionCount;
	storage >> dwCheckPointPositionCount;

	// naète seznam kontrolních bodù cesty
	for ( DWORD dwIndex = dwCheckPointPositionCount; dwIndex-- > 0; )
	{
		// pozice kontrolního bodu cesty
		CPointDW pointCheckPointPosition;

		// naète souøadnice kontrolního bodu cesty
		storage >> pointCheckPointPosition.x;
		storage >> pointCheckPointPosition.y;
		LOAD_ASSERT ( g_cMap.IsMapPosition ( pointCheckPointPosition ) );

		// pøidá kontrolní bod do seznamu smazaných kontrolních bodù
		m_cDeletedCheckPointPositionList.Add ( pointCheckPointPosition );
	}
	// naèetl seznam kontrolních bodù cesty do seznamu smazaných kontrolních bodù cesty

	ASSERT ( m_cDeletedCheckPointPositionList.GetSize () == dwCheckPointPositionCount );

// pøevrátí seznam naètených kontrolních bodù

	// pozice v seznamu pozic naètených kontrolních bodù cesty
	POSITION posCheckPointPosition = m_cDeletedCheckPointPositionList.GetHeadPosition ();

	// pøesune naètené kontrolní body do seznamu kontrolních bodù cesty
	while ( !m_cDeletedCheckPointPositionList.IsEmpty () )
	{
		ASSERT ( posCheckPointPosition == 
			m_cDeletedCheckPointPositionList.GetHeadPosition () );
		// pøesune kontrolní bod do seznamu kontrolních bodù cesty
		m_cDeletedCheckPointPositionList.Move ( posCheckPointPosition, 
			m_cCheckPointPositionList );
	}

	ASSERT ( m_cCheckPointPositionList.GetSize () == dwCheckPointPositionCount );
}

// pøeklad ukazatelù cesty
void CSPath::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
}

// inicializace nahraného objektu cesty
void CSPath::PersistentInit () 
{
}

//////////////////////////////////////////////////////////////////////
// Operace s cestou
//////////////////////////////////////////////////////////////////////

// nechá najít novou cestu šíøky "dwWidth" z místa "pointPosition" do místa 
//		"pointDestination" v grafu "pFindPathGraph" (NULL=vzdušná cesta) pro civilizaci 
//		"dwCivilizationIndex" 
CSPath *CSPath::FindPath ( CPointDW pointPosition, CPointDW pointDestination, 
	CSFindPathGraph *pFindPathGraph, DWORD dwWidth, DWORD dwCivilizationIndex ) 
{
	ASSERT ( g_cMap.IsMapPosition ( pointPosition ) );
	ASSERT ( g_cMap.IsMapPosition ( pointDestination ) );
	ASSERT ( dwCivilizationIndex < g_cMap.GetCivilizationCount () );

	// vytvoøí nový objekt cesty
	CSPath *pPath = new CSPath;

	// nastaví stav cesty na hledání cesty
	pPath->m_eState = EPS_Searching;

	// vytvoøí požadavek na hledání cesty
	SFindPathRequestInfo sFindPathRequestInfo;
	sFindPathRequestInfo.pointPosition = pointPosition;
	sFindPathRequestInfo.pointDestination = pointDestination;
	sFindPathRequestInfo.pFindPathGraph = pFindPathGraph;
	sFindPathRequestInfo.dwWidth = dwWidth;
	sFindPathRequestInfo.pUnit = NULL;
	sFindPathRequestInfo.pPath = pPath;

	// zamkne fronty požadavkù na hledání cesty
	VERIFY ( m_mutexFindPathRequestQueueLock.Lock () );

	// pøidá požadavek na hledání cesty do fronty civilizace
	m_aFindPathRequestQueue[dwCivilizationIndex].Add ( sFindPathRequestInfo );

	// odemkne fronty požadavkù na hledání cesty
	VERIFY ( m_mutexFindPathRequestQueueLock.Unlock () );

	// nastaví událost požadavku na hledání cesty
	VERIFY ( m_eventFindPathRequest.SetEvent () );

	// vrátí vytvoøenou cestu
	return pPath;
}

// nechá najít lokální cestu k prvnímu kontrolnímu bodu cesty (je-li jednotka blízko 
//		kontrolního bodu cesty, pøeskoèí ho) (TRUE=kontrolní bod cesty byl pøeskoèen)
BOOL CSPath::FindLocalPath ( CSUnit *pUnit ) 
{
	ASSERT ( pUnit->GetVerticalPosition () < AIR_VERTICAL_POSITION );

	// pozice jednotky
	CPointDW pointPosition = pUnit->GetPosition ();
	// pozice cílového kontrolního bodu cesty
	CPointDW pointDestination;

	// pozice cílového kontrolního bodu v seznamu kontrolních bodù cesty
	POSITION posDestinationCheckPoint = m_cCheckPointPositionList.GetHeadPosition ();

	// zjistí pozici cílového kontrolního bodu cesty
	VERIFY ( m_cCheckPointPositionList.GetNext ( posDestinationCheckPoint, 
		pointDestination ) );
	// ********************************
//	ASSERT ( pointPosition != pointDestination );
	if ( pointPosition == pointDestination )
	{
		TRACE ( "Warning FindLocalPath on current position (unit ID=%d, position=%d,%d\n", pUnit->GetID (), pointPosition.x, pointPosition.y );
		return TRUE;
	}

	// tolerovaná vzálenost
	DWORD dwToleranceDistance = 2 * pUnit->GetMoveWidth () + 3;

	// zjistí, je-li cílový kontrolní bod cesty v tolerované vzdálenosti
	if ( ( pointDestination.GetDistanceSquare ( pointPosition ) <= dwToleranceDistance * 
		dwToleranceDistance ) )
	{	// cílový kontrolní bod je v tolerované vzdálenosti
		// pøesune první kontrolní bod do smazaných kontrolních bodù
		m_cCheckPointPositionList.Move ( m_cCheckPointPositionList.GetHeadPosition (), 
			m_cDeletedCheckPointPositionList );

		// pozice kontrolního bodu v seznamu kontrolních bodù cesty
		posDestinationCheckPoint = m_cCheckPointPositionList.GetHeadPosition ();

		// zjistí, je-li seznam kontrolních bodù cesty prázdný
		if ( !CSCheckPointPositionList::IsEmpty ( posDestinationCheckPoint ) )
		{	// seznam kontrolních bodù cesty není prázdný
			// pozice kontrolního bodu cesty
			CPointDW pointCheckPointPosition;

			// zjistí pozici kontrolního bodu cesty
			VERIFY ( m_cCheckPointPositionList.GetNext ( posDestinationCheckPoint, 
				pointCheckPointPosition ) );
			// zjistí, jedná-li se o výchozí pozici jednotky
			if ( pointPosition == pointCheckPointPosition )
			{	// jedná se o výchozí pozici jednotky
				// pøesune první kontrolní bod do smazaných kontrolních bodù
				m_cCheckPointPositionList.Move ( m_cCheckPointPositionList.GetHeadPosition (), 
					m_cDeletedCheckPointPositionList );
#ifdef _DEBUG
				// pozice kontrolního bodu v seznamu kontrolních bodù cesty
				posDestinationCheckPoint = m_cCheckPointPositionList.GetHeadPosition ();
				// zjistí, je-li seznam kontrolních bodù cesty prázdný
				if ( !CSCheckPointPositionList::IsEmpty ( posDestinationCheckPoint ) )
				{	// seznam kontrolních bodù cesty není prázdný
					// zjistí pozici kontrolního bodu cesty
					VERIFY ( m_cCheckPointPositionList.GetNext ( posDestinationCheckPoint, 
						pointCheckPointPosition ) );
					ASSERT ( pointPosition != pointCheckPointPosition );
				}
#endif //_DEBUG
			}
			// nejedná se o výchozí pozici jednotky
		}
		// seznam kontrolních bodù cesty je prázdný

		// vrátí pøíznak pøeskoèení kontrolního bodu cesty
		return TRUE;
	}
	// cílový kontrolní bod není v tolerované vzdálenosti

	// index civilizace jednotky
	DWORD dwCivilizationIndex = pUnit->GetCivilizationIndex ();

	// nastaví stav cesty na hledání cesty
	m_eState = EPS_Searching;

	// vytvoøí požadavek na hledání cesty
	SFindPathRequestInfo sFindPathRequestInfo;
	sFindPathRequestInfo.pointPosition = pointPosition;
	sFindPathRequestInfo.pointDestination = pointDestination;
	sFindPathRequestInfo.pFindPathGraph = pUnit->GetFindPathGraph ();
	sFindPathRequestInfo.dwWidth = pUnit->GetMoveWidth ();
	sFindPathRequestInfo.pUnit = pUnit;
	sFindPathRequestInfo.pPath = this;

	// zamkne fronty požadavkù na hledání cesty
	VERIFY ( m_mutexFindPathRequestQueueLock.Lock () );

	// pøidá požadavek na hledání cesty do fronty civilizace
	m_aFindPathRequestQueue[dwCivilizationIndex].Add ( sFindPathRequestInfo );

	// odemkne fronty požadavkù na hledání cesty
	VERIFY ( m_mutexFindPathRequestQueueLock.Unlock () );

	// nastaví událost požadavku na hledání cesty
	VERIFY ( m_eventFindPathRequest.SetEvent () );

	// vrátí pøíznak zadání požadavku na hledání loklní cesty
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// Pomocné operace s cestou
//////////////////////////////////////////////////////////////////////

// najde novou cestu šíøky "dwWidth" z místa "pointPosition" do místa "pointDestination" 
//		v grafu "pFindPathGraph" (NULL=vzdušná cesta) (lze volat jen z FindPathLoop threadu)
void CSPath::FindPath ( CPointDW pointPosition, CPointDW pointDestination, 
	CSFindPathGraph *pFindPathGraph, DWORD dwWidth ) 
{
	ASSERT ( g_cMap.m_pFindPathLoopThread->m_nThreadID == GetCurrentThreadId () );

	ASSERT ( g_cMap.IsMapPosition ( pointPosition ) );
	ASSERT ( g_cMap.IsMapPosition ( pointDestination ) );

	ASSERT ( m_eState == EPS_Searching );
	ASSERT ( m_cCheckPointPositionList.IsEmpty () );
	ASSERT ( m_cDeletedCheckPointPositionList.IsEmpty () );

	if ( pointPosition == pointDestination )
	{
		// nastaví pøíznak cesty na nalezenou cestu
		m_eState = EPS_Found;
		return;
	}

	// zamkne zámek seznamù pozic kontrolních bodù cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Lock () );

	// znièí seznam smazaných pozic kontrolních bodù cesty
	DeleteDeletedCheckPointPositionList ();

	// zjistí jedná-li se o vzdušnou cestu
	if ( pFindPathGraph == NULL )
	{	// jedná se o vzdušnou cestu
		// délka vzdušné cesty
		double dbDistance = sqrt ( pointPosition.GetDistanceSquare ( pointDestination ) );

		// poèet kontrolních bodù uvnitø vzdušné cesty
		int nCheckPointCount = (int)( dbDistance / (double)MAX_CHECK_POINT_DISTANCE );

		// zjistí, má-li vzdušná cesta vnitøní kontrolní body
		if ( nCheckPointCount > 0 )
		{
			// poèet úsekù mezi kontrolními body
			nCheckPointCount++;

			// pøesná vzdálenost kontrolních bodù v ose X
			double dbXCheckPointDistance = ( (double)pointDestination.x - 
				(double)pointPosition.x ) / (double)nCheckPointCount;
			// pøesná vzdálenost kontrolních bodù v ose Y
			double dbYCheckPointDistance = ( (double)pointDestination.y - 
				(double)pointPosition.y ) / (double)nCheckPointCount;

			// pøesná pozice kontrolního bodu v ose X
			double dbXCheckPointPosition = (double)pointDestination.x + 0.5;
			// pøesná pozice kontrolního bodu v ose Y
			double dbYCheckPointPosition = (double)pointDestination.y + 0.5;

			// vytvoøí kontrolní body cesty
			do
			{
				// pøidá do seznamu kontrolních bodù další kontrolní bod cesty
				m_cCheckPointPositionList.Add ( CPointDW ( (DWORD)dbXCheckPointPosition, 
					(DWORD)dbYCheckPointPosition ) );
				// aktualizuje pozici dalšího kontrolního bodu cesty
				dbXCheckPointPosition -= dbXCheckPointDistance;
				dbYCheckPointPosition -= dbYCheckPointDistance;
			}
			while ( nCheckPointCount-- > 0 );
			// byly vytvoøeny kontrolní body cesty
		}
		else
		{	// vzdušná cesta nemá vnitøní kontrolní body
			// pøidá do seznamu kontrolních bodù kontrolní bod konce cesty
			m_cCheckPointPositionList.Add ( pointDestination );
		}
		// byly vytvoøeny kontrolní body cesty

		// nastaví pøíznak nalezené cesty
		m_eState = EPS_Found;
	}
	else
	{	// nejedná se o vzdušnou cestu
		// najde cestu pomocí grafu
		if ( pFindPathGraph->FindPath ( pointPosition, pointDestination, dwWidth, 
			&m_cCheckPointPositionList ) )
		{	// našel cestu pomocí grafu
			// nastaví pøíznak nalezené cesty
			m_eState = EPS_Found;
		}
		else
		{	// nenašel cestu pomocí grafu
			// nastaví pøíznak neexistující cesty
			m_eState = EPS_NotExists;
		}
		// byla nalezena grafová cesta
	}
	// byla nalezena vzdušná nebo grafová cesta

	// odemkne zámek seznamù pozic kontrolních bodù cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Unlock () );
}

// najde lokální cestu šíøky "dwWidth" z místa "pointPosition" do místa 
//		"pointDestination" v grafu "pFindPathGraph" pro jednotku "pUnit" (lze volat jen 
//		z FindPathLoop threadu)
void CSPath::FindLocalPath ( CPointDW pointPosition, CPointDW pointDestination, 
	CSFindPathGraph *pFindPathGraph, DWORD dwWidth, CSUnit *pUnit ) 
{
	ASSERT ( g_cMap.m_pFindPathLoopThread->m_nThreadID == GetCurrentThreadId () );

	ASSERT ( g_cMap.IsMapPosition ( pointPosition ) );
	ASSERT ( g_cMap.IsMapPosition ( pointDestination ) );
	ASSERT ( pointPosition != pointDestination );
	ASSERT ( pFindPathGraph != NULL );
	ASSERT ( pUnit != NULL );
	ASSERT ( !m_cCheckPointPositionList.IsEmpty () );

	ASSERT ( m_eState == EPS_Searching );

	// zamkne zámek seznamù pozic kontrolních bodù cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Lock () );

	// znièí seznam smazaných pozic kontrolních bodù cesty
	DeleteDeletedCheckPointPositionList ();

	// nechá najít lokální cestu
	switch ( pFindPathGraph->FindLocalPath ( pUnit, pointPosition, pointDestination, 
		dwWidth, this ) )
	{
	// cesta byla nalezena
	case CSFindPathGraph::EFLPS_Found :
		// nastaví pøíznak nalezené cesty
		m_eState = EPS_Found;
		break;
	// jednotka je zablokována
	case CSFindPathGraph::EFLPS_UnitBlocked :
		// nastaví pøíznak neexistující cesty
		m_eState = EPS_NotExists;
		break;
	// cesta je zablokována
	case CSFindPathGraph::EFLPS_PathBlocked :
		// nastaví pøíznak zablokované cesty
		m_eState = EPS_Blocked;
		break;
	// nemožný pøípad
	default:
		ASSERT ( FALSE );
	}

	// odemkne zámek seznamù pozic kontrolních bodù cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Unlock () );
}

// najde vzdušnou lokální cestu z místa "pointPosition" do místa "pointDestination" 
//		pro jednotku "pUnit" (lze volat jen z FindPathLoop threadu)
void CSPath::FindAirLocalPath ( CPointDW pointPosition, CPointDW pointDestination, 
	CSUnit *pUnit ) 
{
	ASSERT ( g_cMap.m_pFindPathLoopThread->m_nThreadID == GetCurrentThreadId () );

	ASSERT ( g_cMap.IsMapPosition ( pointPosition ) );
	ASSERT ( g_cMap.IsMapPosition ( pointDestination ) );
	ASSERT ( pointPosition != pointDestination );
	ASSERT ( pUnit != NULL );
	ASSERT ( !m_cCheckPointPositionList.IsEmpty () );

	ASSERT ( m_eState == EPS_Searching );

	// zamkne zámek seznamù pozic kontrolních bodù cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Lock () );

	// znièí seznam smazaných pozic kontrolních bodù cesty
	DeleteDeletedCheckPointPositionList ();

	// **************************
	// nechá najít lokální cestu
	switch ( CSFindPathGraph::FindAirLocalPath ( pUnit, pointPosition, pointDestination, 
		this ) )
	{
	// cesta byla nalezena
	case CSFindPathGraph::EFLPS_Found :
		// nastaví pøíznak nalezené cesty
		m_eState = EPS_Found;
		break;
	// jednotka je zablokována
	case CSFindPathGraph::EFLPS_UnitBlocked :
		// nastaví pøíznak neexistující cesty
		m_eState = EPS_NotExists;
		break;
	// cesta je zablokována
	case CSFindPathGraph::EFLPS_PathBlocked :
		// nastaví pøíznak zablokované cesty
		m_eState = EPS_Blocked;
		break;
	// nemožný pøípad
	default:
		ASSERT ( FALSE );
	}

	// odemkne zámek seznamù pozic kontrolních bodù cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Unlock () );
}

//////////////////////////////////////////////////////////////////////
// Operace s kontrolními body cesty pro MoveSkillu
//////////////////////////////////////////////////////////////////////

// vrátí pozici prvního kontrolního bodu cesty (NULL=konec cesty) (tuto metodu lze 
//		volat výhradnì z MoveSkilly na cestu, kterou MoveSkilla vlastní jako jediná)
CPointDW *CSPath::GetCheckPointPosition () 
{
	// zjistí, jedná-li se o konec cesty
	if ( m_cCheckPointPositionList.IsEmpty () )
	{	// jedná se o konec cesty
		// vrátí pøíznak konce cesty
		return NULL;
	}

	// pozice prvního kontrolního bodu
	CPointDW *pCheckPointPosition;

	// pozice prvního kontrolního bodu v seznamu kontrolních bodù
	POSITION posCheckPointPosition = m_cCheckPointPositionList.GetHeadPosition ();

	// zjistí hodnotu pozice prvního kontrolního bodu
	VERIFY ( m_cCheckPointPositionList.GetNext ( posCheckPointPosition, 
		pCheckPointPosition ) );

	// vrátí ukazatel na pozici prvního kontrolního bodu
	return pCheckPointPosition;
}

// vrátí pozici druhého kontrolního bodu cesty (pøípadnì prvního, neexistuje-li druhý)
//		(tuto metodu lze volat výhradnì z MoveSkilly na cestu, kterou MoveSkilla vlastní 
//		jako jediná)
CPointDW *CSPath::GetNextCheckPointPosition () 
{
	ASSERT ( !m_cCheckPointPositionList.IsEmpty () );

	// pozice druhého kontrolního bodu
	CPointDW *pCheckPointPosition;

	// zjistí první kontrolní bod
	POSITION posCheckPointPosition = m_cCheckPointPositionList.GetHeadPosition ();
	// zjistí hodnotu pozice prvního kontrolního bodu
	VERIFY ( m_cCheckPointPositionList.GetNext ( posCheckPointPosition, 
		pCheckPointPosition ) );

	// zjistí, jedná-li se o poslední kontrolní bod
	if ( !CSCheckPointPositionList::IsEmpty ( posCheckPointPosition ) )
	{	// nejedná se o poslední kontrolní bod
		// zjistí hodnotu pozice druhého kontrolního bodu
		VERIFY ( m_cCheckPointPositionList.GetNext ( posCheckPointPosition, 
			pCheckPointPosition ) );
	}

	// vrátí ukazatel na pozici druhého kontrolního bodu
	return pCheckPointPosition;
}

// odstraní první kontrolní bod cesty (tuto metodu lze volat výhradnì z MoveSkilly 
//		na cestu, kterou MoveSkilla vlastní jako jediná)
void CSPath::RemoveCheckPoint () 
{
	ASSERT ( !m_cCheckPointPositionList.IsEmpty () );

	// pøesune první kontrolní bod do smazaných kontrolních bodù
	m_cCheckPointPositionList.Move ( m_cCheckPointPositionList.GetHeadPosition (), 
		m_cDeletedCheckPointPositionList );
}

//////////////////////////////////////////////////////////////////////
// Operace s kontrolními body cesty
//////////////////////////////////////////////////////////////////////

// smaže seznam pozic kontrolních bodù cesty (zámek seznamù pozic kontrolních bodù 
//		cesty musí být zamèen pro zápis)
void CSPath::DeleteDeletedCheckPointPositionList () 
{
	// znièí seznam smazaných pozic kontrolních bodù cesty
	m_cDeletedCheckPointPositionList.RemoveAll ();
}

//////////////////////////////////////////////////////////////////////
// Vlákna výpoètu serveru hry
//////////////////////////////////////////////////////////////////////

// smyèka hledání cesty
UINT CSPath::FindPathLoop ( LPVOID pMap ) 
{
	ASSERT ( !m_eventFindPathLoopPaused.Lock ( 0 ) );

	// zjistí poèet civilizací
	DWORD dwCivilizationCount = ((CSMap *)pMap)->GetCivilizationCount ();

	// pøíznak prázdné fronty požadavkù na hledání cesty
	BOOL bFindPathRequestQueueEmpty = FALSE;

	// tabulka událostí, na které se èeká
	HANDLE aNewLoopEvents[2] = { 
		(HANDLE)m_eventFindPathRequest, 
		(HANDLE)((CSMap *)pMap)->m_eventPauseGame, 
	};

	// cyklus hledání cest
	for ( ; ; )
	{
	// vyøizování požadavkù na hledání cesty

		// pøíznak hledání cesty v tomto prùchodu cyklem
		BOOL bPathFound = FALSE;

		// zruší událost požadavku na hledání cesty
		VERIFY ( m_eventFindPathRequest.ResetEvent () );

		// projede fronty požadavkù na hledání cesty civilizací
		for ( DWORD dwCivilizationIndex = dwCivilizationCount; dwCivilizationIndex-- > 0; )
		{
		// zpracuje zapausování hry

			// zjistí, je-li hra zapausovaná
			if ( !bFindPathRequestQueueEmpty && ((CSMap *)pMap)->IsGamePaused () )
			{	// hra je zapausovaná
				// nastaví událost zapausování FindPathLoop threadu
				VERIFY ( m_eventFindPathLoopPaused.SetEvent () );

				// uspí thread a zjistí, má-li se pokraèovat ve høe
				if ( !((CSMap *)pMap)->CanContinueGame () )
				{	// hra se má ukonèit
					// ukonèí hru
					return 0;
				}
				// hra má pokraèovat
			}

		// vyøizování požadavkù na hledání cesty

			// zjistí, je-li fronta požadavkù na hledání cesty civilizace prázdná
			if ( !( bFindPathRequestQueueEmpty = 
				m_aFindPathRequestQueue[dwCivilizationIndex].IsEmpty () ) )
			{	// fronta požadavkù na hledání cesty civilizace není prázdná
				// zamkne fronty požadavkù na hledání cesty civilizací
				VERIFY ( m_mutexFindPathRequestQueueLock.Lock () );

				// požadavek na hledání cesty
				SFindPathRequestInfo sFindPathRequestInfo = 
					m_aFindPathRequestQueue[dwCivilizationIndex].RemoveFirst ();

				// odemkne fronty požadavkù na hledání cesty civilizací
				VERIFY ( m_mutexFindPathRequestQueueLock.Unlock () );

				// nastaví pøíznak hledání cesty v tomto prùchodu cyklem
				bPathFound = TRUE;

				ASSERT ( g_cMap.IsMapPosition ( sFindPathRequestInfo.pointPosition ) );
				ASSERT ( g_cMap.IsMapPosition ( sFindPathRequestInfo.pointDestination ) );
				ASSERT ( sFindPathRequestInfo.pPath != NULL );
				ASSERT ( sFindPathRequestInfo.pPath->m_eState == EPS_Searching );

				// zjistí, jedná-li se o hledání lokální cesty
				if ( sFindPathRequestInfo.pUnit == NULL )
				{	// nejdená se o hledání lokální cesty
					ASSERT ( sFindPathRequestInfo.pPath->m_cCheckPointPositionList.IsEmpty () );
					ASSERT ( sFindPathRequestInfo.pPath->m_cDeletedCheckPointPositionList.IsEmpty () );

					// nechá najít cestu
					sFindPathRequestInfo.pPath->FindPath ( sFindPathRequestInfo.pointPosition, 
						sFindPathRequestInfo.pointDestination, 
						sFindPathRequestInfo.pFindPathGraph, sFindPathRequestInfo.dwWidth );
				}
				else
				{	// jedná se o hledání lokální cesty
					ASSERT ( !sFindPathRequestInfo.pPath->m_cCheckPointPositionList.IsEmpty () );

					// zjistí, jedná-li se o hledání vzdušné lokální cesty
					if ( sFindPathRequestInfo.pFindPathGraph == NULL )
					{	// jedná se o hledání vzdušné lokální cesty
						// nechá najít vzdušnou lokální cestu
						sFindPathRequestInfo.pPath->FindAirLocalPath ( 
							sFindPathRequestInfo.pointPosition, 
							sFindPathRequestInfo.pointDestination, 
							sFindPathRequestInfo.pUnit );
					}
					else
					{	// jedná se o hledání lokální cesty
						// nechá najít lokální cestu
						sFindPathRequestInfo.pPath->FindLocalPath ( 
							sFindPathRequestInfo.pointPosition, 
							sFindPathRequestInfo.pointDestination, 
							sFindPathRequestInfo.pFindPathGraph, sFindPathRequestInfo.dwWidth, 
							sFindPathRequestInfo.pUnit );
					}
				}
				// požadavek na hledání cesty byl zpracován
			}
			// byl zpracován pøípadný požadavek na hledání
		}
		// obsloužil všechny fronty požadavkù na hledání cesty

	// znièí smazané cesty

		// zamkne seznam smazaných cest
		VERIFY ( m_mutexDeletedPathListLock.Lock () );

		// ukazatel na smazanou cestu
		CSPath *pDeletedPath;

		// znièí smazané cesty
		while ( m_cDeletedPathList.RemoveFirst ( pDeletedPath ) )
		{
			// zamkne fronty požadavkù na hledání cesty civilizací
			VERIFY ( m_mutexFindPathRequestQueueLock.Lock () );

#ifdef _DEBUG
			// pøíznak smazání požadavku na hledání cesty
			BOOL bFindPathRequestDeleted = FALSE;
#endif //_DEBUG

			// smaže požadavek na hledání cesty
			for ( DWORD dwCivilizationIndex = dwCivilizationCount; 
				dwCivilizationIndex-- > 0; )
			{
				// pokusí se smazat požadavek na hledání cesty z fronty požadavkù civilizace
				if ( m_aFindPathRequestQueue[dwCivilizationIndex].RemoveFindPathRequest ( 
					pDeletedPath ) )
				{	// požadavek na hledání cesty z fronty požadavkù civilizace byl smazán
#ifdef _DEBUG
					// nastaví pøíznak smazání požadavku na hledání cesty
					bFindPathRequestDeleted = TRUE;
					ASSERT ( pDeletedPath->m_eState == EPS_Searching );
#endif //_DEBUG
					// ukonèí mazání pažadavku na hledání cesty
					break;
				}
			}

			// odemkne fronty požadavkù na hledání cesty civilizací
			VERIFY ( m_mutexFindPathRequestQueueLock.Unlock () );

			ASSERT ( bFindPathRequestDeleted || ( pDeletedPath->m_eState == EPS_Valid ) || 
				( pDeletedPath->m_eState == EPS_Found ) || ( pDeletedPath->m_eState == 
				EPS_NotExists ) || ( pDeletedPath->m_eState == EPS_Blocked ) );

			// znièí smazanou cestu
			delete pDeletedPath;
		}

		// odemkne seznam smazaných cest
		VERIFY ( m_mutexDeletedPathListLock.Unlock () );

	// èekání na požadavek na hledání cesty

		// zjistí, byla-li v tomto prùchodu cyklem hledána cesta
		if ( !bPathFound )
		{	// v tomto prùchodu cyklem nebyla hledána cesta
			// poèká na požadavek na hledání cesty nebo zapausování hry
			switch ( WaitForMultipleObjects ( 2, aNewLoopEvents, FALSE, INFINITE ) )
			{
			// pøibyl požadavek na hledání cesty
			case WAIT_OBJECT_0 :
				break;
			// hra byla zapausována
			case ( WAIT_OBJECT_0 + 1 ) :
				// zrušíme pøíznak prázdné fronty požadavkù na hledání cesty
				bFindPathRequestQueueEmpty = FALSE;
				break;
			// nemožný pøípad
			default:
				ASSERT ( FALSE );
			}
		}
		// cesta nebyla v tomto prùchodu cyklem hledána
	}
	// skonèil cyklus hledání cest

	// nemožná vìtev programu
	ASSERT ( FALSE );
}

//////////////////////////////////////////////////////////////////////
// Ukládání spoleèných dat cest (obdaba CPersistentObject metod)
//////////////////////////////////////////////////////////////////////

/*
pathes stored

	[g_cMap.m_dwCivilizationCount]
		dwFindPathRequestCount
		[dwFindPathRequestCount]
			DWORD pointPosition.x
			DWORD pointPosition.y
			DWORD pointDestination.x
			DWORD pointDestination.y
			DWORD dwWidth
			CSFindPathGraph *pFindPathGraph
			CSUnit *pUnit
			CSPath *pPath
*/

// ukládání dat cest
void CSPath::PersistentSavePathes ( CPersistentStorage &storage ) 
{
	// uloží fronty požadavkù na hledání cesty
	for ( DWORD dwCivilizationIndex = g_cMap.GetCivilizationCount (); 
		dwCivilizationIndex-- > 0; )
	{
		// zjistí velikost fronty požadavkù na hledání cesty
		DWORD dwFindPathRequestCount = 
			m_aFindPathRequestQueue[dwCivilizationIndex].GetSize ();

		// uloží velikost fronty požadavkù na hledání cesty
		storage << dwFindPathRequestCount;

		// požadavek na hledání cesty
		SFindPathRequestInfo sFindPathRequestInfo;

		// uloží všechny požadavky na hledání cesty
		while ( m_aFindPathRequestQueue[dwCivilizationIndex].RemoveFirst ( 
			sFindPathRequestInfo ) )
		{
			ASSERT ( g_cMap.IsMapPosition ( sFindPathRequestInfo.pointPosition ) );
			ASSERT ( g_cMap.IsMapPosition ( sFindPathRequestInfo.pointDestination ) );
			ASSERT ( sFindPathRequestInfo.pPath != NULL );
			ASSERT ( ( sFindPathRequestInfo.pUnit == NULL ) || 
				( sFindPathRequestInfo.pFindPathGraph != NULL ) );

			// uloží požadavek na hledání cesty
			storage << sFindPathRequestInfo.pointPosition.x;
			storage << sFindPathRequestInfo.pointPosition.y;
			storage << sFindPathRequestInfo.pointDestination.x;
			storage << sFindPathRequestInfo.pointDestination.y;
			storage << sFindPathRequestInfo.dwWidth;
			storage << sFindPathRequestInfo.pFindPathGraph;
			storage << sFindPathRequestInfo.pUnit;
			storage << sFindPathRequestInfo.pPath;
		}
		// všechny požadavky na hledání cesty byly zpracovány
	}
	// fronty požadavkù na hledání cesty byly zpracovány
}

// nahrávání pouze uložených dat cest s "dwCivilizationCount" civilizacemi
void CSPath::PersistentLoadPathes ( CPersistentStorage &storage, 
	DWORD dwCivilizationCount ) 
{
	// uloží fronty požadavkù na hledání cesty
	for ( DWORD dwCivilizationIndex = dwCivilizationCount; dwCivilizationIndex-- > 0; )
	{
		// naète velikost fronty požadavkù na hledání cesty
		DWORD dwFindPathRequestCount;
		storage >> dwFindPathRequestCount;

		// požadavek na hledání cesty
		SFindPathRequestInfo sFindPathRequestInfo;

		// naète všechny požadavky na hledání cesty
		for ( DWORD dwFindPathRequestIndex = dwFindPathRequestCount; 
			dwFindPathRequestIndex-- > 0; )
		{
			// naète požadavek na hledání cesty
			storage >> sFindPathRequestInfo.pointPosition.x;
			storage >> sFindPathRequestInfo.pointPosition.y;
			storage >> sFindPathRequestInfo.pointDestination.x;
			storage >> sFindPathRequestInfo.pointDestination.y;
			storage >> sFindPathRequestInfo.dwWidth;
			storage >> (void *&)sFindPathRequestInfo.pFindPathGraph;
			storage >> (void *&)sFindPathRequestInfo.pUnit;
			storage >> (void *&)sFindPathRequestInfo.pPath;
			LOAD_ASSERT ( sFindPathRequestInfo.pPath != NULL );
			LOAD_ASSERT ( sFindPathRequestInfo.pUnit == NULL );
		}
		// všechny požadavky na hledání cesty byly zpracovány
	}
	// fronty požadavkù na hledání cesty byly zpracovány
}

// pøeklad ukazatelù cest
void CSPath::PersistentTranslatePointersPathes ( CPersistentStorage &storage ) 
{
	// projede fronty požadavkù na hledání cesty
	for ( DWORD dwCivilizationIndex = CIVILIZATION_COUNT_MAX; dwCivilizationIndex-- > 0; )
	{
		// zjistí pozici prvního prvku fronty požadavkù na hledání cesty
		POSITION posFindPathRequest = 
			m_aFindPathRequestQueue[dwCivilizationIndex].GetHeadPosition ();

		// požadavek na hledání cesty
		SFindPathRequestInfo *pFindPathRequestInfo;

		// naète všechny požadavky na hledání cesty
		while ( CSFindPathRequestQueue::GetNext ( posFindPathRequest, 
			pFindPathRequestInfo ) )
		{
			// pøeloží ukazatel na graf pro hledání cesty
			pFindPathRequestInfo->pFindPathGraph = 
				(CSFindPathGraph *)storage.TranslatePointer ( 
				pFindPathRequestInfo->pFindPathGraph );
			// pøeloží ukazatel na jednotku
			pFindPathRequestInfo->pUnit = (CSUnit *)storage.TranslatePointer ( 
				pFindPathRequestInfo->pUnit );
			LOAD_ASSERT ( pFindPathRequestInfo->pUnit->GetVerticalPosition () < 
				AIR_VERTICAL_POSITION );
			// pøeloží ukazatel na cestu
			pFindPathRequestInfo->pPath = (CSPath *)storage.TranslatePointer ( 
				pFindPathRequestInfo->pPath );
		}
		// všechny požadavky na hledání cesty byly zpracovány
	}
	// fronty požadavkù na hledání cesty byly zpracovány
}

// inicializace nahraných objektù cest
void CSPath::PersistentInitPathes () 
{
	// projede fronty požadavkù na hledání cesty
	for ( DWORD dwCivilizationIndex = CIVILIZATION_COUNT_MAX; dwCivilizationIndex-- > 0; )
	{
		// zjistí pozici prvního prvku fronty požadavkù na hledání cesty
		POSITION posFindPathRequest = 
			m_aFindPathRequestQueue[dwCivilizationIndex].GetHeadPosition ();

		// požadavek na hledání cesty
		SFindPathRequestInfo *pFindPathRequestInfo;

		// naète všechny požadavky na hledání cesty
		while ( CSFindPathRequestQueue::GetNext ( posFindPathRequest, 
			pFindPathRequestInfo ) )
		{
			// zkontroluje požadavek na hledání cesty
			LOAD_ASSERT ( g_cMap.CheckLoadedMapPosition ( 
				pFindPathRequestInfo->pointPosition ) );
			LOAD_ASSERT ( g_cMap.CheckLoadedMapPosition ( 
				pFindPathRequestInfo->pointDestination ) );
		}
		// všechny požadavky na hledání cesty byly zpracovány
	}
	// fronty požadavkù na hledání cesty byly zpracovány
}
