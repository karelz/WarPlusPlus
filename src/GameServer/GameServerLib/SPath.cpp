/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da cesty na serveru hry
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
// Informace o cest�ch
//////////////////////////////////////////////////////////////////////

// z�mek front po�adavk� na hled�n� cesty
CMutex CSPath::m_mutexFindPathRequestQueueLock;
// fronty po�adavk� na hled�n� cesty
CSFindPathRequestQueue CSPath::m_aFindPathRequestQueue[CIVILIZATION_COUNT_MAX];
// ud�lost po�adavku na hled�n� cesty
CEvent CSPath::m_eventFindPathRequest;
// z�mek seznam� pozic kontroln�ch bod� cesty
//CMutex CSPath::m_mutexCheckPointPositionListLock;
// seznam smazan�ch cest
CSelfPooledList<CSPath *> CSPath::m_cDeletedPathList ( 20 );
// z�mek seznamu smazan�ch cest
CMutex CSPath::m_mutexDeletedPathListLock;

//////////////////////////////////////////////////////////////////////
// Informace o vl�knech v�po�tu serveru hry
//////////////////////////////////////////////////////////////////////

// ud�lost zapausov�n� FindPathLoop threadu
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

	// zamkne z�mek seznam� pozic kontroln�ch bod� cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Lock () );

	// zni�� seznam smazan�ch pozic kontroln�ch bod� cesty
	DeleteDeletedCheckPointPositionList ();

	// zni�� seznam pozic kontroln�ch bod� cesty
	m_cCheckPointPositionList.RemoveAll ();

	// odemkne z�mek seznam� pozic kontroln�ch bod� cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Unlock () );
}

//////////////////////////////////////////////////////////////////////
// Inicializace a zni�en� dat cest
//////////////////////////////////////////////////////////////////////

// vytvo�� data cest
void CSPath::CreatePathes ( CDataArchive cMapArchive ) 
{
}

// zni�� data cest
void CSPath::DeletePathes () 
{
	// po�adavek na hled�n� cesty
	SFindPathRequestInfo sFindPathRequestInfo;

	// zni�� fronty po�adavk� na hled�n� cesty
	for ( DWORD dwCivilizationIndex = CIVILIZATION_COUNT_MAX; 
		dwCivilizationIndex-- > 0; )
	{
		// zni�� frontu po�adavk� na hled�n� cesty
		m_aFindPathRequestQueue[dwCivilizationIndex].RemoveAll ();
	}

	// ukazatel na smazanou cestu
	CSPath *pDeletedPath;

	// zni�� smazan� cesty
	while ( m_cDeletedPathList.RemoveFirst ( pDeletedPath ) )
	{
		// zni�� smazanou cestu
		delete pDeletedPath;
	}
}

//////////////////////////////////////////////////////////////////////
// Inicializace a zni�en� dat objektu
//////////////////////////////////////////////////////////////////////

// vr�t� novou kopii cesty (lze volat jen z FindPathLoop threadu)
CSPath *CSPath::Duplicate () 
{
	ASSERT ( g_cMap.m_pFindPathLoopThread->m_nThreadID == GetCurrentThreadId () );

	ASSERT ( ( m_eState == EPS_Valid ) || ( m_eState == EPS_Found ) || 
		( m_eState == EPS_NotExists ) || ( m_eState == EPS_Blocked ) );

	// vytvo�� nov� objekt cesty
	CSPath *pPath = new CSPath;

	// zkop�ruje informace o cest�
	pPath->m_eState = m_eState;

	// zamkne z�mek seznam� pozic kontroln�ch bod� cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Lock () );

	// zni�� seznam smazan�ch pozic kontroln�ch bod� cesty
	DeleteDeletedCheckPointPositionList ();

	// pozice v seznamu pozic kontroln�ch bod� cesty
	POSITION posCheckPointPosition = m_cCheckPointPositionList.GetHeadPosition ();
	// pozice kontroln�ho bodu cesty
	CPointDW pointCheckPointPosition;

	// projede seznam kontroln�ch bod� cesty
	while ( m_cCheckPointPositionList.GetNext ( posCheckPointPosition, 
		pointCheckPointPosition ) )
	{
		// p�id� do seznamu smazan�ch kontroln�ch bod� nov� cesty dal�� kontroln� bod 
		//		kop�rovan� cesty
		pPath->m_cDeletedCheckPointPositionList.Add ( pointCheckPointPosition );
	}

#ifdef _DEBUG
	// po�et kontroln�ch bod� cesty
	DWORD dwCheckPointPositionCount = m_cCheckPointPositionList.GetSize ();
#endif //_DEBUG

	// odemkne z�mek seznam� pozic kontroln�ch bod� cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Unlock () );

// p�evr�t� seznam zkop�rovan�ch kontroln�ch bod�

	// zjist� pozici v seznamu pozic zkop�rovan�ch kontroln�ch bod� cesty
	posCheckPointPosition = pPath->m_cDeletedCheckPointPositionList.GetHeadPosition ();

	// p�esune zkop�rovan� kontroln� body do seznamu kontroln�ch bod� cesty
	while ( !pPath->m_cDeletedCheckPointPositionList.IsEmpty () )
	{
		ASSERT ( posCheckPointPosition == 
			pPath->m_cDeletedCheckPointPositionList.GetHeadPosition () );
		// p�esune kontroln� bod do seznamu kontroln�ch bod� cesty
		pPath->m_cDeletedCheckPointPositionList.Move ( posCheckPointPosition, 
			pPath->m_cCheckPointPositionList );
	}

	ASSERT ( pPath->m_cCheckPointPositionList.GetSize () == dwCheckPointPositionCount );

	// vr�t� vytvo�enou kopii cestu
	return pPath;
}

// zni�� cestu civilizace "dwCivilizationIndex"
void CSPath::Destroy ( DWORD dwCivilizationIndex ) 
{
	// zamkne seznam smazan�ch cest
	VERIFY ( m_mutexDeletedPathListLock.Lock () );

	// p�id� cestu do seznamu smazan�ch cest
	m_cDeletedPathList.Add ( this );

	// odemkne seznam smazan�ch cest
	VERIFY ( m_mutexDeletedPathListLock.Unlock () );
}

// zni�� cestu
void CSPath::Destroy () 
{
	// zamkne seznam smazan�ch cest
	VERIFY ( m_mutexDeletedPathListLock.Lock () );

	// p�id� cestu do seznamu smazan�ch cest
	m_cDeletedPathList.Add ( this );

	// odemkne seznam smazan�ch cest
	VERIFY ( m_mutexDeletedPathListLock.Unlock () );
}

//////////////////////////////////////////////////////////////////////
// Ukl�d�n� dat objektu (obdaba CPersistentObject metod)
//////////////////////////////////////////////////////////////////////

/*
CSPath stored

	enum EPathState m_eState
	DWORD dwCheckPointPositionCount		// m��e b�t i 0
	[dwCheckPointPositionCount]
		DWORD dwX
		DWORD dwY
*/

// ukl�d�n� dat cesty
void CSPath::PersistentSave ( CPersistentStorage &storage ) 
{
	ASSERT ( ( m_eState == EPS_Valid ) || ( m_eState == EPS_Found ) || 
		( m_eState == EPS_NotExists ) || ( m_eState == EPS_Blocked ) || 
		( m_eState == EPS_Searching ) );

	BRACE_BLOCK ( storage );

	// zni�� seznam smazan�ch pozic kontroln�ch bod� cesty
	DeleteDeletedCheckPointPositionList ();

	// ulo�� stav cesty
	storage << (int)m_eState;

	// ulo�� velikost seznamu pozic kontroln�ch bod� cesty
	storage << m_cCheckPointPositionList.GetSize ();

	// pozice v seznamu pozic kontroln�ch bod� cesty
	POSITION posCheckPointPosition = m_cCheckPointPositionList.GetHeadPosition ();
	// pozice kontroln�ho bodu cesty
	CPointDW pointCheckPointPosition;

	// projede seznam kontroln�ch bod� cesty
	while ( m_cCheckPointPositionList.GetNext ( posCheckPointPosition, 
		pointCheckPointPosition ) )
	{
		// ulo�� sou�adnice kontroln�ho bodu cesty
		storage << pointCheckPointPosition.x;
		storage << pointCheckPointPosition.y;
	}
	// ulo�il seznam kontroln�ch bod� cesty
}

// nahr�v�n� pouze ulo�en�ch dat cesty (vrac� nahran� objekt cesty)
CSPath *CSPath::PersistentLoadPath ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	// vytvo�� nov� objekt cesty
	CSPath *pPath = new CSPath;

	// nahraje ulo�en� data cesty
	pPath->PersistentLoad ( storage );

	// vr�t� vytvo�enou cestu
	return pPath;
}

// nahr�n� ulo�en�ch dat cesty
void CSPath::PersistentLoad ( CPersistentStorage &storage ) 
{
	ASSERT ( m_eState == EPS_Uninitialized );
	ASSERT ( m_cCheckPointPositionList.IsEmpty () );
	ASSERT ( m_cDeletedCheckPointPositionList.IsEmpty () );

	// na�te stav cesty
	storage >> (int &)m_eState;
	LOAD_ASSERT ( ( m_eState == EPS_Valid ) || ( m_eState == EPS_Found ) || 
		( m_eState == EPS_NotExists ) || ( m_eState == EPS_Blocked ) || 
		( m_eState == EPS_Searching ) );

	// na�te velikost seznamu pozic kontroln�ch bod� cesty
	DWORD dwCheckPointPositionCount;
	storage >> dwCheckPointPositionCount;

	// na�te seznam kontroln�ch bod� cesty
	for ( DWORD dwIndex = dwCheckPointPositionCount; dwIndex-- > 0; )
	{
		// pozice kontroln�ho bodu cesty
		CPointDW pointCheckPointPosition;

		// na�te sou�adnice kontroln�ho bodu cesty
		storage >> pointCheckPointPosition.x;
		storage >> pointCheckPointPosition.y;
		LOAD_ASSERT ( g_cMap.IsMapPosition ( pointCheckPointPosition ) );

		// p�id� kontroln� bod do seznamu smazan�ch kontroln�ch bod�
		m_cDeletedCheckPointPositionList.Add ( pointCheckPointPosition );
	}
	// na�etl seznam kontroln�ch bod� cesty do seznamu smazan�ch kontroln�ch bod� cesty

	ASSERT ( m_cDeletedCheckPointPositionList.GetSize () == dwCheckPointPositionCount );

// p�evr�t� seznam na�ten�ch kontroln�ch bod�

	// pozice v seznamu pozic na�ten�ch kontroln�ch bod� cesty
	POSITION posCheckPointPosition = m_cDeletedCheckPointPositionList.GetHeadPosition ();

	// p�esune na�ten� kontroln� body do seznamu kontroln�ch bod� cesty
	while ( !m_cDeletedCheckPointPositionList.IsEmpty () )
	{
		ASSERT ( posCheckPointPosition == 
			m_cDeletedCheckPointPositionList.GetHeadPosition () );
		// p�esune kontroln� bod do seznamu kontroln�ch bod� cesty
		m_cDeletedCheckPointPositionList.Move ( posCheckPointPosition, 
			m_cCheckPointPositionList );
	}

	ASSERT ( m_cCheckPointPositionList.GetSize () == dwCheckPointPositionCount );
}

// p�eklad ukazatel� cesty
void CSPath::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
}

// inicializace nahran�ho objektu cesty
void CSPath::PersistentInit () 
{
}

//////////////////////////////////////////////////////////////////////
// Operace s cestou
//////////////////////////////////////////////////////////////////////

// nech� naj�t novou cestu ���ky "dwWidth" z m�sta "pointPosition" do m�sta 
//		"pointDestination" v grafu "pFindPathGraph" (NULL=vzdu�n� cesta) pro civilizaci 
//		"dwCivilizationIndex" 
CSPath *CSPath::FindPath ( CPointDW pointPosition, CPointDW pointDestination, 
	CSFindPathGraph *pFindPathGraph, DWORD dwWidth, DWORD dwCivilizationIndex ) 
{
	ASSERT ( g_cMap.IsMapPosition ( pointPosition ) );
	ASSERT ( g_cMap.IsMapPosition ( pointDestination ) );
	ASSERT ( dwCivilizationIndex < g_cMap.GetCivilizationCount () );

	// vytvo�� nov� objekt cesty
	CSPath *pPath = new CSPath;

	// nastav� stav cesty na hled�n� cesty
	pPath->m_eState = EPS_Searching;

	// vytvo�� po�adavek na hled�n� cesty
	SFindPathRequestInfo sFindPathRequestInfo;
	sFindPathRequestInfo.pointPosition = pointPosition;
	sFindPathRequestInfo.pointDestination = pointDestination;
	sFindPathRequestInfo.pFindPathGraph = pFindPathGraph;
	sFindPathRequestInfo.dwWidth = dwWidth;
	sFindPathRequestInfo.pUnit = NULL;
	sFindPathRequestInfo.pPath = pPath;

	// zamkne fronty po�adavk� na hled�n� cesty
	VERIFY ( m_mutexFindPathRequestQueueLock.Lock () );

	// p�id� po�adavek na hled�n� cesty do fronty civilizace
	m_aFindPathRequestQueue[dwCivilizationIndex].Add ( sFindPathRequestInfo );

	// odemkne fronty po�adavk� na hled�n� cesty
	VERIFY ( m_mutexFindPathRequestQueueLock.Unlock () );

	// nastav� ud�lost po�adavku na hled�n� cesty
	VERIFY ( m_eventFindPathRequest.SetEvent () );

	// vr�t� vytvo�enou cestu
	return pPath;
}

// nech� naj�t lok�ln� cestu k prvn�mu kontroln�mu bodu cesty (je-li jednotka bl�zko 
//		kontroln�ho bodu cesty, p�esko�� ho) (TRUE=kontroln� bod cesty byl p�esko�en)
BOOL CSPath::FindLocalPath ( CSUnit *pUnit ) 
{
	ASSERT ( pUnit->GetVerticalPosition () < AIR_VERTICAL_POSITION );

	// pozice jednotky
	CPointDW pointPosition = pUnit->GetPosition ();
	// pozice c�lov�ho kontroln�ho bodu cesty
	CPointDW pointDestination;

	// pozice c�lov�ho kontroln�ho bodu v seznamu kontroln�ch bod� cesty
	POSITION posDestinationCheckPoint = m_cCheckPointPositionList.GetHeadPosition ();

	// zjist� pozici c�lov�ho kontroln�ho bodu cesty
	VERIFY ( m_cCheckPointPositionList.GetNext ( posDestinationCheckPoint, 
		pointDestination ) );
	// ********************************
//	ASSERT ( pointPosition != pointDestination );
	if ( pointPosition == pointDestination )
	{
		TRACE ( "Warning FindLocalPath on current position (unit ID=%d, position=%d,%d\n", pUnit->GetID (), pointPosition.x, pointPosition.y );
		return TRUE;
	}

	// tolerovan� vz�lenost
	DWORD dwToleranceDistance = 2 * pUnit->GetMoveWidth () + 3;

	// zjist�, je-li c�lov� kontroln� bod cesty v tolerovan� vzd�lenosti
	if ( ( pointDestination.GetDistanceSquare ( pointPosition ) <= dwToleranceDistance * 
		dwToleranceDistance ) )
	{	// c�lov� kontroln� bod je v tolerovan� vzd�lenosti
		// p�esune prvn� kontroln� bod do smazan�ch kontroln�ch bod�
		m_cCheckPointPositionList.Move ( m_cCheckPointPositionList.GetHeadPosition (), 
			m_cDeletedCheckPointPositionList );

		// pozice kontroln�ho bodu v seznamu kontroln�ch bod� cesty
		posDestinationCheckPoint = m_cCheckPointPositionList.GetHeadPosition ();

		// zjist�, je-li seznam kontroln�ch bod� cesty pr�zdn�
		if ( !CSCheckPointPositionList::IsEmpty ( posDestinationCheckPoint ) )
		{	// seznam kontroln�ch bod� cesty nen� pr�zdn�
			// pozice kontroln�ho bodu cesty
			CPointDW pointCheckPointPosition;

			// zjist� pozici kontroln�ho bodu cesty
			VERIFY ( m_cCheckPointPositionList.GetNext ( posDestinationCheckPoint, 
				pointCheckPointPosition ) );
			// zjist�, jedn�-li se o v�choz� pozici jednotky
			if ( pointPosition == pointCheckPointPosition )
			{	// jedn� se o v�choz� pozici jednotky
				// p�esune prvn� kontroln� bod do smazan�ch kontroln�ch bod�
				m_cCheckPointPositionList.Move ( m_cCheckPointPositionList.GetHeadPosition (), 
					m_cDeletedCheckPointPositionList );
#ifdef _DEBUG
				// pozice kontroln�ho bodu v seznamu kontroln�ch bod� cesty
				posDestinationCheckPoint = m_cCheckPointPositionList.GetHeadPosition ();
				// zjist�, je-li seznam kontroln�ch bod� cesty pr�zdn�
				if ( !CSCheckPointPositionList::IsEmpty ( posDestinationCheckPoint ) )
				{	// seznam kontroln�ch bod� cesty nen� pr�zdn�
					// zjist� pozici kontroln�ho bodu cesty
					VERIFY ( m_cCheckPointPositionList.GetNext ( posDestinationCheckPoint, 
						pointCheckPointPosition ) );
					ASSERT ( pointPosition != pointCheckPointPosition );
				}
#endif //_DEBUG
			}
			// nejedn� se o v�choz� pozici jednotky
		}
		// seznam kontroln�ch bod� cesty je pr�zdn�

		// vr�t� p��znak p�esko�en� kontroln�ho bodu cesty
		return TRUE;
	}
	// c�lov� kontroln� bod nen� v tolerovan� vzd�lenosti

	// index civilizace jednotky
	DWORD dwCivilizationIndex = pUnit->GetCivilizationIndex ();

	// nastav� stav cesty na hled�n� cesty
	m_eState = EPS_Searching;

	// vytvo�� po�adavek na hled�n� cesty
	SFindPathRequestInfo sFindPathRequestInfo;
	sFindPathRequestInfo.pointPosition = pointPosition;
	sFindPathRequestInfo.pointDestination = pointDestination;
	sFindPathRequestInfo.pFindPathGraph = pUnit->GetFindPathGraph ();
	sFindPathRequestInfo.dwWidth = pUnit->GetMoveWidth ();
	sFindPathRequestInfo.pUnit = pUnit;
	sFindPathRequestInfo.pPath = this;

	// zamkne fronty po�adavk� na hled�n� cesty
	VERIFY ( m_mutexFindPathRequestQueueLock.Lock () );

	// p�id� po�adavek na hled�n� cesty do fronty civilizace
	m_aFindPathRequestQueue[dwCivilizationIndex].Add ( sFindPathRequestInfo );

	// odemkne fronty po�adavk� na hled�n� cesty
	VERIFY ( m_mutexFindPathRequestQueueLock.Unlock () );

	// nastav� ud�lost po�adavku na hled�n� cesty
	VERIFY ( m_eventFindPathRequest.SetEvent () );

	// vr�t� p��znak zad�n� po�adavku na hled�n� lokln� cesty
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// Pomocn� operace s cestou
//////////////////////////////////////////////////////////////////////

// najde novou cestu ���ky "dwWidth" z m�sta "pointPosition" do m�sta "pointDestination" 
//		v grafu "pFindPathGraph" (NULL=vzdu�n� cesta) (lze volat jen z FindPathLoop threadu)
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
		// nastav� p��znak cesty na nalezenou cestu
		m_eState = EPS_Found;
		return;
	}

	// zamkne z�mek seznam� pozic kontroln�ch bod� cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Lock () );

	// zni�� seznam smazan�ch pozic kontroln�ch bod� cesty
	DeleteDeletedCheckPointPositionList ();

	// zjist� jedn�-li se o vzdu�nou cestu
	if ( pFindPathGraph == NULL )
	{	// jedn� se o vzdu�nou cestu
		// d�lka vzdu�n� cesty
		double dbDistance = sqrt ( pointPosition.GetDistanceSquare ( pointDestination ) );

		// po�et kontroln�ch bod� uvnit� vzdu�n� cesty
		int nCheckPointCount = (int)( dbDistance / (double)MAX_CHECK_POINT_DISTANCE );

		// zjist�, m�-li vzdu�n� cesta vnit�n� kontroln� body
		if ( nCheckPointCount > 0 )
		{
			// po�et �sek� mezi kontroln�mi body
			nCheckPointCount++;

			// p�esn� vzd�lenost kontroln�ch bod� v ose X
			double dbXCheckPointDistance = ( (double)pointDestination.x - 
				(double)pointPosition.x ) / (double)nCheckPointCount;
			// p�esn� vzd�lenost kontroln�ch bod� v ose Y
			double dbYCheckPointDistance = ( (double)pointDestination.y - 
				(double)pointPosition.y ) / (double)nCheckPointCount;

			// p�esn� pozice kontroln�ho bodu v ose X
			double dbXCheckPointPosition = (double)pointDestination.x + 0.5;
			// p�esn� pozice kontroln�ho bodu v ose Y
			double dbYCheckPointPosition = (double)pointDestination.y + 0.5;

			// vytvo�� kontroln� body cesty
			do
			{
				// p�id� do seznamu kontroln�ch bod� dal�� kontroln� bod cesty
				m_cCheckPointPositionList.Add ( CPointDW ( (DWORD)dbXCheckPointPosition, 
					(DWORD)dbYCheckPointPosition ) );
				// aktualizuje pozici dal��ho kontroln�ho bodu cesty
				dbXCheckPointPosition -= dbXCheckPointDistance;
				dbYCheckPointPosition -= dbYCheckPointDistance;
			}
			while ( nCheckPointCount-- > 0 );
			// byly vytvo�eny kontroln� body cesty
		}
		else
		{	// vzdu�n� cesta nem� vnit�n� kontroln� body
			// p�id� do seznamu kontroln�ch bod� kontroln� bod konce cesty
			m_cCheckPointPositionList.Add ( pointDestination );
		}
		// byly vytvo�eny kontroln� body cesty

		// nastav� p��znak nalezen� cesty
		m_eState = EPS_Found;
	}
	else
	{	// nejedn� se o vzdu�nou cestu
		// najde cestu pomoc� grafu
		if ( pFindPathGraph->FindPath ( pointPosition, pointDestination, dwWidth, 
			&m_cCheckPointPositionList ) )
		{	// na�el cestu pomoc� grafu
			// nastav� p��znak nalezen� cesty
			m_eState = EPS_Found;
		}
		else
		{	// nena�el cestu pomoc� grafu
			// nastav� p��znak neexistuj�c� cesty
			m_eState = EPS_NotExists;
		}
		// byla nalezena grafov� cesta
	}
	// byla nalezena vzdu�n� nebo grafov� cesta

	// odemkne z�mek seznam� pozic kontroln�ch bod� cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Unlock () );
}

// najde lok�ln� cestu ���ky "dwWidth" z m�sta "pointPosition" do m�sta 
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

	// zamkne z�mek seznam� pozic kontroln�ch bod� cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Lock () );

	// zni�� seznam smazan�ch pozic kontroln�ch bod� cesty
	DeleteDeletedCheckPointPositionList ();

	// nech� naj�t lok�ln� cestu
	switch ( pFindPathGraph->FindLocalPath ( pUnit, pointPosition, pointDestination, 
		dwWidth, this ) )
	{
	// cesta byla nalezena
	case CSFindPathGraph::EFLPS_Found :
		// nastav� p��znak nalezen� cesty
		m_eState = EPS_Found;
		break;
	// jednotka je zablokov�na
	case CSFindPathGraph::EFLPS_UnitBlocked :
		// nastav� p��znak neexistuj�c� cesty
		m_eState = EPS_NotExists;
		break;
	// cesta je zablokov�na
	case CSFindPathGraph::EFLPS_PathBlocked :
		// nastav� p��znak zablokovan� cesty
		m_eState = EPS_Blocked;
		break;
	// nemo�n� p��pad
	default:
		ASSERT ( FALSE );
	}

	// odemkne z�mek seznam� pozic kontroln�ch bod� cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Unlock () );
}

// najde vzdu�nou lok�ln� cestu z m�sta "pointPosition" do m�sta "pointDestination" 
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

	// zamkne z�mek seznam� pozic kontroln�ch bod� cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Lock () );

	// zni�� seznam smazan�ch pozic kontroln�ch bod� cesty
	DeleteDeletedCheckPointPositionList ();

	// **************************
	// nech� naj�t lok�ln� cestu
	switch ( CSFindPathGraph::FindAirLocalPath ( pUnit, pointPosition, pointDestination, 
		this ) )
	{
	// cesta byla nalezena
	case CSFindPathGraph::EFLPS_Found :
		// nastav� p��znak nalezen� cesty
		m_eState = EPS_Found;
		break;
	// jednotka je zablokov�na
	case CSFindPathGraph::EFLPS_UnitBlocked :
		// nastav� p��znak neexistuj�c� cesty
		m_eState = EPS_NotExists;
		break;
	// cesta je zablokov�na
	case CSFindPathGraph::EFLPS_PathBlocked :
		// nastav� p��znak zablokovan� cesty
		m_eState = EPS_Blocked;
		break;
	// nemo�n� p��pad
	default:
		ASSERT ( FALSE );
	}

	// odemkne z�mek seznam� pozic kontroln�ch bod� cesty
//	VERIFY ( m_mutexCheckPointPositionListLock.Unlock () );
}

//////////////////////////////////////////////////////////////////////
// Operace s kontroln�mi body cesty pro MoveSkillu
//////////////////////////////////////////////////////////////////////

// vr�t� pozici prvn�ho kontroln�ho bodu cesty (NULL=konec cesty) (tuto metodu lze 
//		volat v�hradn� z MoveSkilly na cestu, kterou MoveSkilla vlastn� jako jedin�)
CPointDW *CSPath::GetCheckPointPosition () 
{
	// zjist�, jedn�-li se o konec cesty
	if ( m_cCheckPointPositionList.IsEmpty () )
	{	// jedn� se o konec cesty
		// vr�t� p��znak konce cesty
		return NULL;
	}

	// pozice prvn�ho kontroln�ho bodu
	CPointDW *pCheckPointPosition;

	// pozice prvn�ho kontroln�ho bodu v seznamu kontroln�ch bod�
	POSITION posCheckPointPosition = m_cCheckPointPositionList.GetHeadPosition ();

	// zjist� hodnotu pozice prvn�ho kontroln�ho bodu
	VERIFY ( m_cCheckPointPositionList.GetNext ( posCheckPointPosition, 
		pCheckPointPosition ) );

	// vr�t� ukazatel na pozici prvn�ho kontroln�ho bodu
	return pCheckPointPosition;
}

// vr�t� pozici druh�ho kontroln�ho bodu cesty (p��padn� prvn�ho, neexistuje-li druh�)
//		(tuto metodu lze volat v�hradn� z MoveSkilly na cestu, kterou MoveSkilla vlastn� 
//		jako jedin�)
CPointDW *CSPath::GetNextCheckPointPosition () 
{
	ASSERT ( !m_cCheckPointPositionList.IsEmpty () );

	// pozice druh�ho kontroln�ho bodu
	CPointDW *pCheckPointPosition;

	// zjist� prvn� kontroln� bod
	POSITION posCheckPointPosition = m_cCheckPointPositionList.GetHeadPosition ();
	// zjist� hodnotu pozice prvn�ho kontroln�ho bodu
	VERIFY ( m_cCheckPointPositionList.GetNext ( posCheckPointPosition, 
		pCheckPointPosition ) );

	// zjist�, jedn�-li se o posledn� kontroln� bod
	if ( !CSCheckPointPositionList::IsEmpty ( posCheckPointPosition ) )
	{	// nejedn� se o posledn� kontroln� bod
		// zjist� hodnotu pozice druh�ho kontroln�ho bodu
		VERIFY ( m_cCheckPointPositionList.GetNext ( posCheckPointPosition, 
			pCheckPointPosition ) );
	}

	// vr�t� ukazatel na pozici druh�ho kontroln�ho bodu
	return pCheckPointPosition;
}

// odstran� prvn� kontroln� bod cesty (tuto metodu lze volat v�hradn� z MoveSkilly 
//		na cestu, kterou MoveSkilla vlastn� jako jedin�)
void CSPath::RemoveCheckPoint () 
{
	ASSERT ( !m_cCheckPointPositionList.IsEmpty () );

	// p�esune prvn� kontroln� bod do smazan�ch kontroln�ch bod�
	m_cCheckPointPositionList.Move ( m_cCheckPointPositionList.GetHeadPosition (), 
		m_cDeletedCheckPointPositionList );
}

//////////////////////////////////////////////////////////////////////
// Operace s kontroln�mi body cesty
//////////////////////////////////////////////////////////////////////

// sma�e seznam pozic kontroln�ch bod� cesty (z�mek seznam� pozic kontroln�ch bod� 
//		cesty mus� b�t zam�en pro z�pis)
void CSPath::DeleteDeletedCheckPointPositionList () 
{
	// zni�� seznam smazan�ch pozic kontroln�ch bod� cesty
	m_cDeletedCheckPointPositionList.RemoveAll ();
}

//////////////////////////////////////////////////////////////////////
// Vl�kna v�po�tu serveru hry
//////////////////////////////////////////////////////////////////////

// smy�ka hled�n� cesty
UINT CSPath::FindPathLoop ( LPVOID pMap ) 
{
	ASSERT ( !m_eventFindPathLoopPaused.Lock ( 0 ) );

	// zjist� po�et civilizac�
	DWORD dwCivilizationCount = ((CSMap *)pMap)->GetCivilizationCount ();

	// p��znak pr�zdn� fronty po�adavk� na hled�n� cesty
	BOOL bFindPathRequestQueueEmpty = FALSE;

	// tabulka ud�lost�, na kter� se �ek�
	HANDLE aNewLoopEvents[2] = { 
		(HANDLE)m_eventFindPathRequest, 
		(HANDLE)((CSMap *)pMap)->m_eventPauseGame, 
	};

	// cyklus hled�n� cest
	for ( ; ; )
	{
	// vy�izov�n� po�adavk� na hled�n� cesty

		// p��znak hled�n� cesty v tomto pr�chodu cyklem
		BOOL bPathFound = FALSE;

		// zru�� ud�lost po�adavku na hled�n� cesty
		VERIFY ( m_eventFindPathRequest.ResetEvent () );

		// projede fronty po�adavk� na hled�n� cesty civilizac�
		for ( DWORD dwCivilizationIndex = dwCivilizationCount; dwCivilizationIndex-- > 0; )
		{
		// zpracuje zapausov�n� hry

			// zjist�, je-li hra zapausovan�
			if ( !bFindPathRequestQueueEmpty && ((CSMap *)pMap)->IsGamePaused () )
			{	// hra je zapausovan�
				// nastav� ud�lost zapausov�n� FindPathLoop threadu
				VERIFY ( m_eventFindPathLoopPaused.SetEvent () );

				// usp� thread a zjist�, m�-li se pokra�ovat ve h�e
				if ( !((CSMap *)pMap)->CanContinueGame () )
				{	// hra se m� ukon�it
					// ukon�� hru
					return 0;
				}
				// hra m� pokra�ovat
			}

		// vy�izov�n� po�adavk� na hled�n� cesty

			// zjist�, je-li fronta po�adavk� na hled�n� cesty civilizace pr�zdn�
			if ( !( bFindPathRequestQueueEmpty = 
				m_aFindPathRequestQueue[dwCivilizationIndex].IsEmpty () ) )
			{	// fronta po�adavk� na hled�n� cesty civilizace nen� pr�zdn�
				// zamkne fronty po�adavk� na hled�n� cesty civilizac�
				VERIFY ( m_mutexFindPathRequestQueueLock.Lock () );

				// po�adavek na hled�n� cesty
				SFindPathRequestInfo sFindPathRequestInfo = 
					m_aFindPathRequestQueue[dwCivilizationIndex].RemoveFirst ();

				// odemkne fronty po�adavk� na hled�n� cesty civilizac�
				VERIFY ( m_mutexFindPathRequestQueueLock.Unlock () );

				// nastav� p��znak hled�n� cesty v tomto pr�chodu cyklem
				bPathFound = TRUE;

				ASSERT ( g_cMap.IsMapPosition ( sFindPathRequestInfo.pointPosition ) );
				ASSERT ( g_cMap.IsMapPosition ( sFindPathRequestInfo.pointDestination ) );
				ASSERT ( sFindPathRequestInfo.pPath != NULL );
				ASSERT ( sFindPathRequestInfo.pPath->m_eState == EPS_Searching );

				// zjist�, jedn�-li se o hled�n� lok�ln� cesty
				if ( sFindPathRequestInfo.pUnit == NULL )
				{	// nejden� se o hled�n� lok�ln� cesty
					ASSERT ( sFindPathRequestInfo.pPath->m_cCheckPointPositionList.IsEmpty () );
					ASSERT ( sFindPathRequestInfo.pPath->m_cDeletedCheckPointPositionList.IsEmpty () );

					// nech� naj�t cestu
					sFindPathRequestInfo.pPath->FindPath ( sFindPathRequestInfo.pointPosition, 
						sFindPathRequestInfo.pointDestination, 
						sFindPathRequestInfo.pFindPathGraph, sFindPathRequestInfo.dwWidth );
				}
				else
				{	// jedn� se o hled�n� lok�ln� cesty
					ASSERT ( !sFindPathRequestInfo.pPath->m_cCheckPointPositionList.IsEmpty () );

					// zjist�, jedn�-li se o hled�n� vzdu�n� lok�ln� cesty
					if ( sFindPathRequestInfo.pFindPathGraph == NULL )
					{	// jedn� se o hled�n� vzdu�n� lok�ln� cesty
						// nech� naj�t vzdu�nou lok�ln� cestu
						sFindPathRequestInfo.pPath->FindAirLocalPath ( 
							sFindPathRequestInfo.pointPosition, 
							sFindPathRequestInfo.pointDestination, 
							sFindPathRequestInfo.pUnit );
					}
					else
					{	// jedn� se o hled�n� lok�ln� cesty
						// nech� naj�t lok�ln� cestu
						sFindPathRequestInfo.pPath->FindLocalPath ( 
							sFindPathRequestInfo.pointPosition, 
							sFindPathRequestInfo.pointDestination, 
							sFindPathRequestInfo.pFindPathGraph, sFindPathRequestInfo.dwWidth, 
							sFindPathRequestInfo.pUnit );
					}
				}
				// po�adavek na hled�n� cesty byl zpracov�n
			}
			// byl zpracov�n p��padn� po�adavek na hled�n�
		}
		// obslou�il v�echny fronty po�adavk� na hled�n� cesty

	// zni�� smazan� cesty

		// zamkne seznam smazan�ch cest
		VERIFY ( m_mutexDeletedPathListLock.Lock () );

		// ukazatel na smazanou cestu
		CSPath *pDeletedPath;

		// zni�� smazan� cesty
		while ( m_cDeletedPathList.RemoveFirst ( pDeletedPath ) )
		{
			// zamkne fronty po�adavk� na hled�n� cesty civilizac�
			VERIFY ( m_mutexFindPathRequestQueueLock.Lock () );

#ifdef _DEBUG
			// p��znak smaz�n� po�adavku na hled�n� cesty
			BOOL bFindPathRequestDeleted = FALSE;
#endif //_DEBUG

			// sma�e po�adavek na hled�n� cesty
			for ( DWORD dwCivilizationIndex = dwCivilizationCount; 
				dwCivilizationIndex-- > 0; )
			{
				// pokus� se smazat po�adavek na hled�n� cesty z fronty po�adavk� civilizace
				if ( m_aFindPathRequestQueue[dwCivilizationIndex].RemoveFindPathRequest ( 
					pDeletedPath ) )
				{	// po�adavek na hled�n� cesty z fronty po�adavk� civilizace byl smaz�n
#ifdef _DEBUG
					// nastav� p��znak smaz�n� po�adavku na hled�n� cesty
					bFindPathRequestDeleted = TRUE;
					ASSERT ( pDeletedPath->m_eState == EPS_Searching );
#endif //_DEBUG
					// ukon�� maz�n� pa�adavku na hled�n� cesty
					break;
				}
			}

			// odemkne fronty po�adavk� na hled�n� cesty civilizac�
			VERIFY ( m_mutexFindPathRequestQueueLock.Unlock () );

			ASSERT ( bFindPathRequestDeleted || ( pDeletedPath->m_eState == EPS_Valid ) || 
				( pDeletedPath->m_eState == EPS_Found ) || ( pDeletedPath->m_eState == 
				EPS_NotExists ) || ( pDeletedPath->m_eState == EPS_Blocked ) );

			// zni�� smazanou cestu
			delete pDeletedPath;
		}

		// odemkne seznam smazan�ch cest
		VERIFY ( m_mutexDeletedPathListLock.Unlock () );

	// �ek�n� na po�adavek na hled�n� cesty

		// zjist�, byla-li v tomto pr�chodu cyklem hled�na cesta
		if ( !bPathFound )
		{	// v tomto pr�chodu cyklem nebyla hled�na cesta
			// po�k� na po�adavek na hled�n� cesty nebo zapausov�n� hry
			switch ( WaitForMultipleObjects ( 2, aNewLoopEvents, FALSE, INFINITE ) )
			{
			// p�ibyl po�adavek na hled�n� cesty
			case WAIT_OBJECT_0 :
				break;
			// hra byla zapausov�na
			case ( WAIT_OBJECT_0 + 1 ) :
				// zru��me p��znak pr�zdn� fronty po�adavk� na hled�n� cesty
				bFindPathRequestQueueEmpty = FALSE;
				break;
			// nemo�n� p��pad
			default:
				ASSERT ( FALSE );
			}
		}
		// cesta nebyla v tomto pr�chodu cyklem hled�na
	}
	// skon�il cyklus hled�n� cest

	// nemo�n� v�tev programu
	ASSERT ( FALSE );
}

//////////////////////////////////////////////////////////////////////
// Ukl�d�n� spole�n�ch dat cest (obdaba CPersistentObject metod)
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

// ukl�d�n� dat cest
void CSPath::PersistentSavePathes ( CPersistentStorage &storage ) 
{
	// ulo�� fronty po�adavk� na hled�n� cesty
	for ( DWORD dwCivilizationIndex = g_cMap.GetCivilizationCount (); 
		dwCivilizationIndex-- > 0; )
	{
		// zjist� velikost fronty po�adavk� na hled�n� cesty
		DWORD dwFindPathRequestCount = 
			m_aFindPathRequestQueue[dwCivilizationIndex].GetSize ();

		// ulo�� velikost fronty po�adavk� na hled�n� cesty
		storage << dwFindPathRequestCount;

		// po�adavek na hled�n� cesty
		SFindPathRequestInfo sFindPathRequestInfo;

		// ulo�� v�echny po�adavky na hled�n� cesty
		while ( m_aFindPathRequestQueue[dwCivilizationIndex].RemoveFirst ( 
			sFindPathRequestInfo ) )
		{
			ASSERT ( g_cMap.IsMapPosition ( sFindPathRequestInfo.pointPosition ) );
			ASSERT ( g_cMap.IsMapPosition ( sFindPathRequestInfo.pointDestination ) );
			ASSERT ( sFindPathRequestInfo.pPath != NULL );
			ASSERT ( ( sFindPathRequestInfo.pUnit == NULL ) || 
				( sFindPathRequestInfo.pFindPathGraph != NULL ) );

			// ulo�� po�adavek na hled�n� cesty
			storage << sFindPathRequestInfo.pointPosition.x;
			storage << sFindPathRequestInfo.pointPosition.y;
			storage << sFindPathRequestInfo.pointDestination.x;
			storage << sFindPathRequestInfo.pointDestination.y;
			storage << sFindPathRequestInfo.dwWidth;
			storage << sFindPathRequestInfo.pFindPathGraph;
			storage << sFindPathRequestInfo.pUnit;
			storage << sFindPathRequestInfo.pPath;
		}
		// v�echny po�adavky na hled�n� cesty byly zpracov�ny
	}
	// fronty po�adavk� na hled�n� cesty byly zpracov�ny
}

// nahr�v�n� pouze ulo�en�ch dat cest s "dwCivilizationCount" civilizacemi
void CSPath::PersistentLoadPathes ( CPersistentStorage &storage, 
	DWORD dwCivilizationCount ) 
{
	// ulo�� fronty po�adavk� na hled�n� cesty
	for ( DWORD dwCivilizationIndex = dwCivilizationCount; dwCivilizationIndex-- > 0; )
	{
		// na�te velikost fronty po�adavk� na hled�n� cesty
		DWORD dwFindPathRequestCount;
		storage >> dwFindPathRequestCount;

		// po�adavek na hled�n� cesty
		SFindPathRequestInfo sFindPathRequestInfo;

		// na�te v�echny po�adavky na hled�n� cesty
		for ( DWORD dwFindPathRequestIndex = dwFindPathRequestCount; 
			dwFindPathRequestIndex-- > 0; )
		{
			// na�te po�adavek na hled�n� cesty
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
		// v�echny po�adavky na hled�n� cesty byly zpracov�ny
	}
	// fronty po�adavk� na hled�n� cesty byly zpracov�ny
}

// p�eklad ukazatel� cest
void CSPath::PersistentTranslatePointersPathes ( CPersistentStorage &storage ) 
{
	// projede fronty po�adavk� na hled�n� cesty
	for ( DWORD dwCivilizationIndex = CIVILIZATION_COUNT_MAX; dwCivilizationIndex-- > 0; )
	{
		// zjist� pozici prvn�ho prvku fronty po�adavk� na hled�n� cesty
		POSITION posFindPathRequest = 
			m_aFindPathRequestQueue[dwCivilizationIndex].GetHeadPosition ();

		// po�adavek na hled�n� cesty
		SFindPathRequestInfo *pFindPathRequestInfo;

		// na�te v�echny po�adavky na hled�n� cesty
		while ( CSFindPathRequestQueue::GetNext ( posFindPathRequest, 
			pFindPathRequestInfo ) )
		{
			// p�elo�� ukazatel na graf pro hled�n� cesty
			pFindPathRequestInfo->pFindPathGraph = 
				(CSFindPathGraph *)storage.TranslatePointer ( 
				pFindPathRequestInfo->pFindPathGraph );
			// p�elo�� ukazatel na jednotku
			pFindPathRequestInfo->pUnit = (CSUnit *)storage.TranslatePointer ( 
				pFindPathRequestInfo->pUnit );
			LOAD_ASSERT ( pFindPathRequestInfo->pUnit->GetVerticalPosition () < 
				AIR_VERTICAL_POSITION );
			// p�elo�� ukazatel na cestu
			pFindPathRequestInfo->pPath = (CSPath *)storage.TranslatePointer ( 
				pFindPathRequestInfo->pPath );
		}
		// v�echny po�adavky na hled�n� cesty byly zpracov�ny
	}
	// fronty po�adavk� na hled�n� cesty byly zpracov�ny
}

// inicializace nahran�ch objekt� cest
void CSPath::PersistentInitPathes () 
{
	// projede fronty po�adavk� na hled�n� cesty
	for ( DWORD dwCivilizationIndex = CIVILIZATION_COUNT_MAX; dwCivilizationIndex-- > 0; )
	{
		// zjist� pozici prvn�ho prvku fronty po�adavk� na hled�n� cesty
		POSITION posFindPathRequest = 
			m_aFindPathRequestQueue[dwCivilizationIndex].GetHeadPosition ();

		// po�adavek na hled�n� cesty
		SFindPathRequestInfo *pFindPathRequestInfo;

		// na�te v�echny po�adavky na hled�n� cesty
		while ( CSFindPathRequestQueue::GetNext ( posFindPathRequest, 
			pFindPathRequestInfo ) )
		{
			// zkontroluje po�adavek na hled�n� cesty
			LOAD_ASSERT ( g_cMap.CheckLoadedMapPosition ( 
				pFindPathRequestInfo->pointPosition ) );
			LOAD_ASSERT ( g_cMap.CheckLoadedMapPosition ( 
				pFindPathRequestInfo->pointDestination ) );
		}
		// v�echny po�adavky na hled�n� cesty byly zpracov�ny
	}
	// fronty po�adavk� na hled�n� cesty byly zpracov�ny
}
