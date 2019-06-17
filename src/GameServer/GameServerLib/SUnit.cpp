/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da jednotky na serveru hry. Obsahuje v�echny 
 *          informace o jednotce, kter� pot�ebuje GameServer. 
 *          Ka�d� jednotka na map� (voj��ek, budova, atp.) 
 *          je reprezentov�na svoji vlastn� instanc� t�to t��dy.
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SUnit.h"

#include <stdlib.h>
#include <string.h>

#include "SMap.h"
#include "SUnitType.h"
#include "SSkillType.h"
#include "SCivilization.h"

#include "Common\ServerClient\UnitInfoStructures.h"
#include "GameServer\Civilization\Src\ZUnit.h"
#include "GameServer\Civilization\Src\ClientCommunication\ZClientUnitInfoSender.h"
#include "Common\Map\Map.h"

#include "SUnit.inl"
#include "SUnitType.inl"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// po�et TimeSlic� zpo�d�n� smaz�n� jednotky (minim�ln� 1)
#define UNIT_DELETE_DELAY		20

IMPLEMENT_DYNAMIC ( CSUnit, CPersistentObject )

//////////////////////////////////////////////////////////////////////
// Statick� data t��dy
//////////////////////////////////////////////////////////////////////

// dal�� generovan� ID jednotky
DWORD CSUnit::m_dwNextGeneratedID = 0;

// memory pool pol� po�tu jednotek civilizac� vid�c�ch jenotku
CMemoryPool CSUnit::m_cCivilizationVisibilityArrayMemoryPool ( 100 );
// memory pool pol� posledn�ch pozic, kde byla jednotka civilizacemi vid�na
CMemoryPool CSUnit::m_cCivilizationLastSeenPositionArrayMemoryPool ( 100 );

// tabulka sm�r� kladn� poloroviny osy X
BYTE CSUnit::m_aDirectionTableXPositive[5] = 
	{ Dir_South, Dir_SouthEast, Dir_East, Dir_NorthEast, Dir_North };
// tabulka sm�r� z�porn� poloroviny osy X
BYTE CSUnit::m_aDirectionTableXNegative[5] = 
	{ Dir_North, Dir_NorthWest, Dir_West, Dir_SouthWest, Dir_South };

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSUnit::CSUnit () 
{
	// zne�kodn� data jednotky
	m_nLives = 0;
	m_nLivesMax = 0;
	m_dwViewRadius = 0;
	m_dwViewRadiusSquare = 0;
	m_pointPosition = CPointDW ( DWORD_MAX, DWORD_MAX );
	m_dwVerticalPosition = 0;
	m_strUnitName.Empty ();
	m_nDirection = 0xff;
	m_nMode = 8;
	m_dwInvisibilityState = 0;
	m_dwInvisibilityDetection = 0;
	m_dwAppearanceID = DWORD_MAX;

	m_cFirstCheckPoint.Set ( CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ), 0 );
	m_cSecondCheckPoint.Set ( CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ), 0 );

	m_pZUnit = NULL;
	m_dwID = DWORD_MAX;
	m_pLock = NULL;
	m_pUnitType = NULL;
	m_dwCivilizationIndex = CIVILIZATION_COUNT_MAX;

	m_dwModificationFlags = 0;

	m_pSkillData = NULL;

	m_aCivilizationVisibility = NULL;
	m_aCivilizationLastSeenPosition = NULL;
	m_cCivilizationVisibilityChanged.FillArray ( FALSE );
	m_cWatchedMapSquareChanged.FillArray ( FALSE );
	m_dwWatchingCivilizationClientCount = 0;

	m_dwReferenceCounter = 0;

	m_pNextUnit = NULL;
}

// destruktor
CSUnit::~CSUnit () 
{
	// zkontroluje ne�kodn� data jednotky
	ASSERT ( CheckEmptyData () );
}

//////////////////////////////////////////////////////////////////////
// Inicializace a zni�en� dat objektu
//////////////////////////////////////////////////////////////////////

// p�edvytvo�� jednotku typu "pUnitType"
void CSUnit::PreCreate ( CSUnitType *pUnitType ) 
{
	// zkontroluje ne�kodn� data jednotky
	ASSERT ( CheckEmptyData () );

	// zkontroluje inicializa�n� data jednotky
	ASSERT ( pUnitType != NULL );

	// inicializuje hern� informace o jednotce
	m_nLives = m_nLivesMax = pUnitType->GetLivesMax ();
	SetViewRadius ( pUnitType->GetViewRadius () );
	m_pointPosition = CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION );
	m_dwVerticalPosition = pUnitType->GetVerticalPosition ();
	ASSERT ( m_dwVerticalPosition != 0 );
	m_nDirection = Dir_Random;
	m_nMode = 0;
	m_dwInvisibilityState = pUnitType->m_aInvisibilityState[0];
	m_dwInvisibilityDetection = pUnitType->m_aInvisibilityDetection[0];
	m_dwAppearanceID = pUnitType->m_aAppearanceID[0];

	// inicializuje serverov� informace o jednotce
	GenerateID ();
	m_pUnitType = pUnitType;
	m_dwCivilizationIndex = pUnitType->GetCivilization ()->GetCivilizationIndex ();

//	TRACE ( "Creating unit ID=%d (@0x%x)\n", m_dwID, this );

	m_dwModificationFlags = MF_Clear;

	// inicializuje bitov� pole p��znak� active/deactive skill jednotky na 0
	m_cActiveSkills.Create ( pUnitType->GetSkillCount (), FALSE );
	// alokuje data skill
	m_pSkillData = new BYTE[pUnitType->GetSkillDataSize ()];

	// zamkne memory pooly pol� civilizac� jednotky
	VERIFY ( m_mutexCivilizationArrayMemoryPoolLock.Lock () );

	// vytvo�� pole po�tu jednotek jednotliv�ch civilizac�, kter� jednotku vid�
	m_aCivilizationVisibility = (WORD *)m_cCivilizationVisibilityArrayMemoryPool.Allocate ();
	// vytvo�� pole posledn�ch pozic, kde byla jednotka civilizacemi vid�na
	m_aCivilizationLastSeenPosition = 
		(CSPosition *)m_cCivilizationLastSeenPositionArrayMemoryPool.Allocate ();

	// odemkne memory pooly pol� civilizac� jednotky
	VERIFY ( m_mutexCivilizationArrayMemoryPoolLock.Unlock () );

	// inicializuje pole po�tu jednotek jednotliv�ch civilizac�, kter� jednotku vid�
	memset ( m_aCivilizationVisibility, 0, g_cMap.GetCivilizationCount () * 
		sizeof ( WORD ) );
	m_aCivilizationVisibility[m_dwCivilizationIndex] = 1;
	// inicializuje pole posledn�ch pozic, kde byla jednotka civilizacemi vid�na
	for ( DWORD dwCivilizationIndex = g_cMap.GetCivilizationCount (); 
		dwCivilizationIndex-- > 0; )
	{
		m_aCivilizationLastSeenPosition[dwCivilizationIndex].x = NO_MAP_POSITION;
		m_aCivilizationLastSeenPosition[dwCivilizationIndex].y = NO_MAP_POSITION;
		m_aCivilizationLastSeenPosition[dwCivilizationIndex].z = 0;
	}
	// inicializuje po�et klient� civilizac� sleduj�c�ch jednotku
	m_dwWatchingCivilizationClientCount = 0;

	// inicializuje skilly jednotky
	pUnitType->InitializeSkills ( this );

	// zne�kodn� ukazatel na jednotku na civilizaci
	m_pZUnit = NULL;
}

// vytvo�� jednotku typu "pUnitType" (typ jednotky mus� b�t zam�en pro z�pis)
void CSUnit::Create ( CSUnitType *pUnitType ) 
{
	// p�edvytvo�� jednotku
	PreCreate ( pUnitType );

	// vytvo�� jednotku na civilizaci
	m_pZUnit = pUnitType->GetCivilization ()->m_pZCivilization->CreateUnit ( this );
	LOAD_ASSERT ( m_pZUnit != NULL );

	// p�id� jednotku do seznamu jednotek typu jednotky
	pUnitType->AddUnit ( this );
}

// zni�� data jednotky
void CSUnit::Delete () 
{
	ASSERT ( m_nLives <= 0 );
	ASSERT ( m_dwReferenceCounter == 0 );

	// zjist�, je-li platn� ukazatel na typ jednotky
	if ( m_pUnitType != NULL )
	{	// ukazatel na typ jednotky je platn�
		// odebere jednotku ze seznamu jednotek typu jednotky
		m_pUnitType->DeleteUnit ( this );
	}

	// zru�� odkaz na jednotku na civilizaci
	m_pZUnit = NULL;

	// odregistruje ID jednotky
	UnregisterID ();

	// zjist�, jsou-li platn� skilly jednotky
	if ( m_pSkillData != NULL )
	{	// skilly jednotky jsou platn�
		// zjist�, je-li platn� ukazatel na typ jednotky
		if ( m_pUnitType != NULL )
		{	// ukazatel na typ jednotky je platn�
			// zni�� skilly jednotky
			m_pUnitType->DestructSkills ( this );
		}
		// zni�� data skill
		delete [] m_pSkillData;
		m_pSkillData = NULL;
	}

	// zni�� seznam vid�n�ch jednotek
	m_cSeenUnitList.RemoveAll ();

	// zne�kodn� data jednotky
	SetEmptyData ();
}

// vypln� objekt ne�kodn�mi daty (zni�� ��ste�n� inicializavan� data)
void CSUnit::SetEmptyData () 
{
	m_nLives = 0;
	m_nLivesMax = 0;
	m_dwViewRadius = 0;
	m_dwViewRadiusSquare = 0;
	m_pointPosition = CPointDW ( DWORD_MAX, DWORD_MAX );
	m_dwVerticalPosition = 0;
	m_strUnitName.Empty ();
	m_nDirection = 0xff;
	m_nMode = 8;
	m_dwInvisibilityState = 0;
	m_dwInvisibilityDetection = 0;
	m_dwAppearanceID = DWORD_MAX;

	m_cFirstCheckPoint.Set ( CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ), 0 );
	m_cSecondCheckPoint.Set ( CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ), 0 );

	m_pZUnit = NULL;
	m_dwID = DWORD_MAX;
	m_pLock = NULL;
	m_pUnitType = NULL;
	m_dwCivilizationIndex = CIVILIZATION_COUNT_MAX;

	m_dwModificationFlags = 0;

	// zjist�, je-li zni�eno pole p��znak� active/deactive skill jednotky
	if ( !m_cActiveSkills.IsDeleted () )
	{	// pole p��znak� active/deactive skill jednotky nen� zni�eno
		// zni�� bitov� pole p��znak� active/deactive skill jednotky
		m_cActiveSkills.Delete ();
	}
	// zjist�, jsou-li alokov�na data skill
	if ( m_pSkillData != NULL )
	{	// data skill jsou alokov�na
		// zni�� data skill
		delete [] m_pSkillData;
		m_pSkillData = NULL;
	}

	// zjist�, je-li alokov�no n�jak� pole civilizac� jednotky
	if ( ( m_aCivilizationVisibility != NULL ) || 
		( m_aCivilizationLastSeenPosition != NULL ) )
	{	// je alokov�na n�jak� pole civilizac� jednotky
		// zamkne memory pooly pol� civilizac� jednotky
		VERIFY ( m_mutexCivilizationArrayMemoryPoolLock.Lock () );

		// zjist�, je-li alokov�no pole po�tu jednotek civilizac� vid�c�ch jednotku
		if ( m_aCivilizationVisibility != NULL )
		{	// pole po�tu jednotek civilizac� vid�c�ch jednotku je alokov�no
			// zni�� pole po�tu jednotek civilizac� vid�c�ch jednotku
			m_cCivilizationVisibilityArrayMemoryPool.Free ( m_aCivilizationVisibility );
			m_aCivilizationVisibility = NULL;
		}
		// zjist�, je-li alokov�no pole posledn�ch pozic, kde byla jednotka civilizacemi vid�na
		if ( m_aCivilizationLastSeenPosition != NULL )
		{	// pole posledn�ch pozic, kde byla jednotka civilizacemi vid�na je alokov�no
			// zni�� pole posledn�ch pozic, kde byla jednotka civilizacemi vid�na
			m_cCivilizationLastSeenPositionArrayMemoryPool.Free ( 
				m_aCivilizationLastSeenPosition );
			m_aCivilizationLastSeenPosition = NULL;
		}

		// odemkne memory pooly pol� civilizac� jednotky
		VERIFY ( m_mutexCivilizationArrayMemoryPoolLock.Unlock () );
	}

	m_dwWatchingCivilizationClientCount = 0;

	m_dwReferenceCounter = 0;
}

// vygeneruje a zaregistruje nov� ID jednotky
void CSUnit::GenerateID () 
{
	// vygeneruje ID a zv�t�� dal�� generovan� ID jednotky
	m_dwID = m_dwNextGeneratedID++;
}

// zaregistruje ID jednotky
void CSUnit::RegisterID () 
{
	m_dwNextGeneratedID = max ( m_dwNextGeneratedID, m_dwID + 1 );
}

// odregistruje ID jednotky
void CSUnit::UnregisterID () 
{
}

// zpracuje zabit� jednotky (lze volat pouze z MainLoop threadu, jednotka mus� b�t zam�ena 
//		pro z�pis, MapSquary mus� b�t zam�eny)
void CSUnit::UnitDied () 
{
	ASSERT ( g_cMap.IsMainLoopThread () );
	ASSERT ( CheckValidData () );

	ASSERT ( m_nLives <= 0 );

	// aktualizuje po�at �ivot� jednotky
	m_nLives = DEAD_UNIT_LIVES;

	// zni�� skilly jednotky
	m_pUnitType->DestructSkills ( this );

	// sma�e jednotku z jednotek mapy
	g_cMap.DeleteUnit ( this );

	// p�id� jednotku do fronty mrtv�ch jednotek
	g_cMap.m_cDeadUnitQueue.Add ( struct CSMap::SDeadUnit ( this, 
		g_cMap.GetTimeSlice () + UNIT_DELETE_DELAY ) );

	// uschov� si pozici jednotky
	CPointDW pointPosition = m_pointPosition;

	// odebere jednotku z mapy, um�st� j� mimo mapu
	g_cMap.RemoveUnit ( this );

	// obnov� pozici jednotky
	// ****************** NEVIM: m_pointPosition = pointPosition;

	// zamkne typ jednotky pro z�pis
	VERIFY ( m_pUnitType->WriterLock () );

	// odebere jednotku ze seznamu jednotek typu jednotky
	m_pUnitType->DeleteUnit ( this );

	// odemkne typ jednotky 
	m_pUnitType->WriterUnlock ();

	// nech� zni�it jednotku na civilizaci
	g_cMap.GetCivilization ( m_dwCivilizationIndex )->m_pZCivilization->UnitDied ( 
		m_pZUnit, pointPosition );
}

// informuje jednotku, �e jednotka na civilizaci ztratila posledn� odkaz na sebe
void CSUnit::ZUnitRefCountIsZero () 
{
	// zjist�, je-li jednotka mrtv�
	if ( m_nLives <= 0 )
	{	// jednotka je mrtv�
		ASSERT ( m_nLives == DEAD_UNIT_LIVES );
		// zjist�, existuj�-li odkazy na jednotku
		if ( m_dwReferenceCounter == 0 )
		{	// na jednotku neexistuj� odkazy
			ASSERT ( m_pZUnit != NULL );
			// zni�� jednotku na civilizaci
			VERIFY ( m_pZUnit->TryDelete () );

			// zamkne seznam smazan�ch jednotek
			VERIFY ( g_cMap.m_mutexDeletedUnitListLock.Lock () );

			// najde jednotku v seznamu smazan�ch jednotek
			POSITION posUnit = g_cMap.m_cDeletedUnitList.Find ( this );
			ASSERT ( posUnit != NULL );
			// sma�e jednotku ze seznamu smazan�ch jednotek
			VERIFY ( g_cMap.m_cDeletedUnitList.RemovePosition ( posUnit ) == this );
			ASSERT ( g_cMap.m_cDeletedUnitList.Find ( this ) == NULL );

			// odemkne seznam smazan�ch jednotek
			VERIFY ( g_cMap.m_mutexDeletedUnitListLock.Unlock () );

			// zni�� jednotku
			Delete ();
			delete this;
		}
		// na jednotku existuj� odkazy
	}
	// jednotka nen� mrtv�
}

//////////////////////////////////////////////////////////////////////
// Ukl�d�n� dat (CPersistentObject metody)
//////////////////////////////////////////////////////////////////////

/*
CSUnit stored

	DWORD m_dwID
	CSUnitType *m_pUnitType
	CZUnit *m_pZUnit
	DWORD m_dwReferenceCounter
	int m_nLives
	int m_nLivesMax
	DWORD m_dwViewRadius
	DWORD dwPositionX
	DWORD dwPositionY
	DWORD m_dwVerticalPosition
	int nUnitNameSize							// m��e b�t i 0 - pak je vynech�no aUnitName
		char aUnitName[nUnitNameSize]
	BYTE m_nDirection
	BYTE m_nMode
	DWORD m_dwInvisibilityState
	DWORD m_dwInvisibilityDetection
	DWORD m_dwAppearanceID
	DWORD m_cFirstCheckPoint.GetPosition ().x
	DWORD m_cFirstCheckPoint.GetPosition ().y
	DWORD m_cFirstCheckPoint.GetTime ()
	DWORD m_cSecondCheckPoint.GetPosition ().x
	DWORD m_cSecondCheckPoint.GetPosition ().y
	DWORD m_cSecondCheckPoint.GetTime ()
	DWORD m_dwModificationFlags
	BYTE aActiveSkills[(m_pUnitType->GetSkillCount () + 7)/8]
	[m_pUnitType->GetSkillCount ()]
		skill data stored
	[dwCivilizationCount]
		DWORD m_aCivilizationLastSeenPosition[dwCivilizationIndex].x
		DWORD m_aCivilizationLastSeenPosition[dwCivilizationIndex].y
		DWORD m_aCivilizationLastSeenPosition[dwCivilizationIndex].z
	DWORD dwSeenUnitCount
		CSUnit *pUnit[dwSeenUnitCount]
*/

// ukl�d�n� dat
void CSUnit::PersistentSave ( CPersistentStorage &storage ) 
{
	// zkontroluje platnost dat jednotky
	ASSERT ( CheckValidData () );

	BRACE_BLOCK ( storage );

	// ulo�� ID jednotky
	storage << m_dwID;
	// ulo�� ukazatel na typ jednotky
	storage << m_pUnitType;
	// ulo�� ukazatel na jednotku na civilizaci
	storage << m_pZUnit;
	// ulo�� po�et odkaz� na jednotku
	storage << m_dwReferenceCounter;

	// ulo�� po�et �ivot�
	storage << m_nLives;
	// ulo�� maxim�ln� po�et �ivot�
	storage << m_nLivesMax;

	// ulo�� polom�r viditelnosti
	storage << m_dwViewRadius;
	// ulo�� x-ovou sou�adnici pozice jednotky
	storage << m_pointPosition.x;
	// ulo�� y-ovou sou�adnici pozice jednotky
	storage << m_pointPosition.y;
	// ulo�� vertik�ln� pozici jednotky
	storage << m_dwVerticalPosition;

	// zjist� d�lku jm�na jednotky
	int nUnitNameSize = m_strUnitName.GetLength ();
	// ulo�� d�lku jm�na jednotky
	storage << nUnitNameSize;
	// zjist�, je-li jm�no jednotky pr�zdn�
	if ( nUnitNameSize > 0 )
	{	// jm�no jednotky nen� pr�zdn�
		// ulo�� jm�no jednotky
		storage.Write ( m_strUnitName, nUnitNameSize );
	}

	// ulo�� nasm�rov�n� jednotky
	storage << m_nDirection;
	// ulo�� m�d jednotky
	storage << m_nMode;
	// ulo�� p��znaky neviditelnosti jednotky
	storage << m_dwInvisibilityState;
	// ulo�� p��znaky detekce neviditelnosti ostatn�ch jednotek
	storage << m_dwInvisibilityDetection;
	// ulo�� ID vzhledu jednotky
	storage << m_dwAppearanceID;

	// ulo�� prvn� kontroln� bod cesty jednotky
	storage << m_cFirstCheckPoint.GetPosition ().x;
	storage << m_cFirstCheckPoint.GetPosition ().y;
	storage << m_cFirstCheckPoint.GetTime ();
	// ulo�� druh� kontroln� bod cesty jednotky
	storage << m_cSecondCheckPoint.GetPosition ().x;
	storage << m_cSecondCheckPoint.GetPosition ().y;
	storage << m_cSecondCheckPoint.GetTime ();

	// ulo�� p��znaky zm�n dat jednotky
	storage << m_dwModificationFlags;

	// zjist� po�et skill jednotky
	DWORD dwSkillCount = m_pUnitType->GetSkillCount ();
	ASSERT ( dwSkillCount > 0 );
	// ulo�� pole p��znak� aktivn�ch skill
	storage.Write ( m_cActiveSkills.GetData (), ( dwSkillCount + 7 ) / 8 );
	// projede v�echny skilly jednotky
	for ( ; dwSkillCount-- > 0; )
	{
		// ulo�� skillu jednotky
		m_pUnitType->GetSkillTypeByIndex ( dwSkillCount )->PersistentSaveSkill ( storage, this );
	}

	// ulo�� pole naposledy vid�n�ch pozic jednotky civilizacemi
	for ( DWORD dwCivilizationIndex = g_cMap.GetCivilizationCount (); 
		dwCivilizationIndex-- > 0; )
	{
		// ulo�� naposledy vid�nou pozici jednotky civilizac� "dwCivilizationIndex"
		storage << m_aCivilizationLastSeenPosition[dwCivilizationIndex].x;
		storage << m_aCivilizationLastSeenPosition[dwCivilizationIndex].y;
		storage << m_aCivilizationLastSeenPosition[dwCivilizationIndex].z;
	}

	// ulo�� po�et vid�n�ch jednotek
	storage << m_cSeenUnitList.GetSize ();

	// zjist� pozici prvn� jednotky v seznamu vid�n�ch jednotek
	POSITION posSeenUnit = m_cSeenUnitList.GetHeadPosition ();
	// vid�n� jednotka
	CSUnit *pSeenUnit;

	// projede jednotky v seznamu vid�n�ch jednotek
	while ( CSUnitList::GetNext ( posSeenUnit, pSeenUnit ) )
	{
		// ulo�� ukazatel na vid�nou jednotku
		storage << pSeenUnit;
	}
}

// nahr�v�n� pouze ulo�en�ch dat s "dwCivilizationCount" civilizacemi
void CSUnit::PersistentLoad ( CPersistentStorage &storage, DWORD dwCivilizationCount ) 
{
	// zkontroluje ne�kodn� data jednotky
	ASSERT ( CheckEmptyData () );

	BRACE_BLOCK ( storage );

	// na�te ID jednotky
	storage >> m_dwID;
	// zaregistruje na�ten� ID jednotky
	RegisterID ();
	// na�te ukazatel na typ jednotky
	storage >> (void *&)m_pUnitType;
	LOAD_ASSERT ( m_pUnitType != NULL );
	// p�elo�� ukazatel na typ jednotky
	m_pUnitType = (CSUnitType *)storage.TranslatePointer ( m_pUnitType );
	// na�te ukazatel na jednotku na civilizaci
	storage >> (void *&)m_pZUnit;
	LOAD_ASSERT ( m_pZUnit != NULL );
	// na�te po�et odkaz� na jednotku
	storage >> m_dwReferenceCounter;
	LOAD_ASSERT ( m_dwReferenceCounter != DWORD_MAX );

	// na�te po�et �ivot�
	storage >> m_nLives;
	// na�te maxim�ln� po�et �ivot�
	storage >> m_nLivesMax;
	LOAD_ASSERT ( m_nLivesMax > 0 );

	// na�te polom�r viditelnosti
	storage >> m_dwViewRadius;
	// na�te x-ovou sou�adnici pozice jednotky
	storage >> m_pointPosition.x;
	// na�te y-ovou sou�adnici pozice jednotky
	storage >> m_pointPosition.y;
	// na�te vertik�ln� pozici jednotky
	storage >> m_dwVerticalPosition;
	LOAD_ASSERT ( m_dwVerticalPosition != 0 );

	// d�lka jm�na jednotky
	int nUnitNameSize;
	// na�te d�lku jm�na jednotky
	storage >> nUnitNameSize;
	// zjist�, je-li jem�no jednotky pr�zdn�
	if ( nUnitNameSize > 0 )
	{	// jm�no jednotky nen� pr�zdn�
		// z�sk� ukazatel na na��tan� jm�no jednotky
		char *szUnitName = m_strUnitName.GetBufferSetLength ( nUnitNameSize );
		// na�te jm�no jednotky
		storage.Read ( szUnitName, nUnitNameSize );
		// ukon�� na��t�n� jm�na jednotky
		m_strUnitName.ReleaseBuffer ( nUnitNameSize );
	}

	// na�te nasm�rov�n� jednotky
	storage >> m_nDirection;
	LOAD_ASSERT ( m_nDirection <= Dir_Random );
	// na�te m�d jednotky
	storage >> m_nMode;
	LOAD_ASSERT ( m_nMode < 8 );
	// na�te p��znaky neviditelnosti jednotky
	storage >> m_dwInvisibilityState;
	// na�te p��znaky detekce neviditelnosti ostatn�ch jednotek
	storage >> m_dwInvisibilityDetection;
	// na�te ID vzhledu jednotky
	storage >> m_dwAppearanceID;
	LOAD_ASSERT ( m_dwAppearanceID != 0 );

	// pozice a �as kontroln�ho bodu cesty jednotky
	DWORD dwCheckPointPositionX, dwCheckPointPositionY, dwCheckPointTime;
	// na�te prvn� kontroln� bod cesty jednotky
	storage >> dwCheckPointPositionX;
	storage >> dwCheckPointPositionY;
	storage >> dwCheckPointTime;
	m_cFirstCheckPoint.Set ( CPointDW ( dwCheckPointPositionX, dwCheckPointPositionY ), 
		dwCheckPointTime );
	// na�te druh� kontroln� bod cesty jednotky
	storage >> dwCheckPointPositionX;
	storage >> dwCheckPointPositionY;
	storage >> dwCheckPointTime;
	m_cSecondCheckPoint.Set ( CPointDW ( dwCheckPointPositionX, dwCheckPointPositionY ), 
		dwCheckPointTime );

	// na�te p��znaky zm�n dat jednotky
	storage >> m_dwModificationFlags;

	// vytvo�� data skill
	m_pSkillData = new BYTE[m_pUnitType->GetSkillDataSize ()];

	// zjist� po�et skill jednotky
	DWORD dwSkillCount = m_pUnitType->GetSkillCount ();
	ASSERT ( dwSkillCount > 0 );
	// vytvo�� bitov� pole p��znak� active/deactive skill jednotky na 0
	m_cActiveSkills.Create ( dwSkillCount );
	// na�te pole p��znak� aktivn�ch skill
	storage.Read ( (void *)m_cActiveSkills.GetData (), ( dwSkillCount + 7 ) / 8 );

	// inicializuje skilly jednotky
	m_pUnitType->InitializeSkills ( this );

	// projede v�echny skilly jednotky
	for ( ; dwSkillCount-- > 0; )
	{
		// na�te skillu jednotky
		m_pUnitType->GetSkillTypeByIndex ( dwSkillCount )->PersistentLoadSkill ( storage, this );
	}

	// vytvo�� pole posledn�ch pozic, kde byla jednotka civilizacemi vid�na
	m_aCivilizationLastSeenPosition = 
		(CSPosition *)m_cCivilizationLastSeenPositionArrayMemoryPool.Allocate ();
	// na�te pole naposledy vid�n�ch pozic jednotky civilizacemi
	for ( DWORD dwCivilizationIndex = dwCivilizationCount; dwCivilizationIndex-- > 0; )
	{
		// na�te naposledy vid�nou pozici jednotky civilizac� "dwCivilizationIndex"
		storage >> m_aCivilizationLastSeenPosition[dwCivilizationIndex].x;
		storage >> m_aCivilizationLastSeenPosition[dwCivilizationIndex].y;
		storage >> m_aCivilizationLastSeenPosition[dwCivilizationIndex].z;
	}

	// po�et jednotek v seznamu vid�n�ch jednotek
	DWORD dwSeenUnitCount;
	// na�te po�et vid�n�ch jednotek
	storage >> dwSeenUnitCount;

	// na�te seznam vid�n�ch jednotek
	while ( dwSeenUnitCount-- > 0 )
	{
		// vid�n� jednotka
		CSUnit *pSeenUnit;
		// na�te ukazatel na vid�nou jednotku
		storage >> (void *&)pSeenUnit;
		LOAD_ASSERT ( pSeenUnit != NULL );
		LOAD_ASSERT ( m_cSeenUnitList.Find ( pSeenUnit ) == NULL );
		// p�id� vid�nou jednotku do seznamu vid�n�ch jednotek
		m_cSeenUnitList.Add ( pSeenUnit );
	}
}

// p�eklad ukazatel�
void CSUnit::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	// p�elo�� ukazatel na jednotku na civilizaci
	m_pZUnit = (CZUnit *)storage.TranslatePointer ( m_pZUnit );
	LOAD_ASSERT ( m_pZUnit != NULL );

	// projede v�echny skilly jednotky
	for ( DWORD dwSkillIndex = m_pUnitType->GetSkillCount (); dwSkillIndex-- > 0; )
	{
		// p�elo�� ukazatele skilly jednotky
		m_pUnitType->GetSkillTypeByIndex ( dwSkillIndex )->PersistentTranslatePointersSkill ( 
			storage, this );
	}

	// zjist� pozici prvn� jednotky v seznamu vid�n�ch jednotek
	POSITION posSeenUnit = m_cSeenUnitList.GetHeadPosition ();
	// vid�n� jednotka v seznamu vid�n�ch jednotek
	CSUnit **pSeenUnit;

	// projede jednotky v seznamu vid�n�ch jednotek
	while ( CSUnitList::GetNext ( posSeenUnit, pSeenUnit ) )
	{
		// p�elo�� ukazatel na vid�nou jednotku
		*pSeenUnit = (CSUnit *)storage.TranslatePointer ( *pSeenUnit );
	}
}

// inicializace nahran�ho objektu
void CSUnit::PersistentInit ( BOOL bDeadDeletedUnit ) 
{
	// zkontroluje kontroln� body cesty jednotky a pozici jednotky
	if ( g_cMap.CheckLoadedMapPosition ( m_pointPosition ) )
	{	// jedn� se o pozici na map�
		// zkontroluje prvn� kontroln� bod cesty jednotky
		LOAD_ASSERT ( g_cMap.CheckLoadedMapPosition ( m_cFirstCheckPoint.GetPosition () ) );
		// zkontroluje druh� kontroln� bod cesty jednotky
		LOAD_ASSERT ( g_cMap.CheckLoadedMapPosition ( m_cSecondCheckPoint.GetPosition () ) );
		// zkontroluje �asov� sou�adnice kontroln�ch bod� cesty jednotky
		LOAD_ASSERT ( m_cFirstCheckPoint.GetTime () <= m_cSecondCheckPoint.GetTime () );
		// zjist�, jedn�-li se o zp�tn� kontroln� body cesty jednotky
		if ( m_cFirstCheckPoint.GetTime () < g_cMap.GetTimeSlice () )
		{	// jedn� se o zp�tn� kontroln� bod cesty jednotky
			// *******************************
			//LOAD_ASSERT ( m_cFirstCheckPoint.GetPosition () == m_pointPosition );
			//LOAD_ASSERT ( m_cFirstCheckPoint.GetTime () == m_cSecondCheckPoint.GetTime () );
		}
	}
	else
	{	// jedn� se o pozici mimo mapu
		// zkontroluje prvn� kontroln� bod cesty jednotky
		LOAD_ASSERT ( !g_cMap.CheckLoadedMapPosition ( m_cFirstCheckPoint.GetPosition () ) );
		LOAD_ASSERT ( m_cFirstCheckPoint.GetTime () <= g_cMap.GetTimeSlice () );
		// zkontroluje druh� kontroln� bod cesty jednotky
		LOAD_ASSERT ( !g_cMap.CheckLoadedMapPosition ( m_cSecondCheckPoint.GetPosition () ) );
		LOAD_ASSERT ( m_cSecondCheckPoint.GetTime () <= g_cMap.GetTimeSlice () );
	}

	// zkontroluje pole naposledy vid�n�ch pozic jednotky civilizacemi
	for ( DWORD dwCivilizationIndex = g_cMap.GetCivilizationCount (); 
		dwCivilizationIndex-- > 0; )
	{
		// zkontroluje naposledy vid�nou pozici jednotky civilizac� "dwCivilizationIndex"
		if ( g_cMap.CheckLoadedMapPosition ( 
			m_aCivilizationLastSeenPosition[dwCivilizationIndex] ) )
		{	// jedn� se o pozici na map�
			LOAD_ASSERT ( m_aCivilizationLastSeenPosition[dwCivilizationIndex].z != 0 );
		}
		else
		{	// jedn� se o pozici mimo mapu
			LOAD_ASSERT ( m_aCivilizationLastSeenPosition[dwCivilizationIndex].z == 0 );
		}
	}

	// inicializuje index civilizace jednotky
	m_dwCivilizationIndex = m_pUnitType->GetCivilization ()->GetCivilizationIndex ();

	// vytvo�� pole po�tu jednotek jednotliv�ch civilizac�, kter� jednotku vid�
	m_aCivilizationVisibility = (WORD *)m_cCivilizationVisibilityArrayMemoryPool.Allocate ();

	// inicializuje pole po�tu jednotek jednotliv�ch civilizac�, kter� jednotku vid�
	memset ( m_aCivilizationVisibility, 0, g_cMap.GetCivilizationCount () * 
		sizeof ( WORD ) );
	m_aCivilizationVisibility[m_dwCivilizationIndex] = 1;
	// inicializuje po�et klient� civilizac� sleduj�c�ch jednotku
	m_dwWatchingCivilizationClientCount = 0;

	// inicializuje polom�r viditelnosti jednotky
	SetViewRadius ( m_dwViewRadius );

	// projede v�echny skilly jednotky
	for ( DWORD dwSkillIndex = m_pUnitType->GetSkillCount (); dwSkillIndex-- > 0; )
	{
		// inicializuje skilly jednotky
		m_pUnitType->GetSkillTypeByIndex ( dwSkillIndex )->PersistentInitSkill ( this );
	}

	// zjist�, jedn�-li se o mrtvou nebo smazanou jednotku
	if ( !bDeadDeletedUnit )
	{	// nejedn� se o mrtvou ani smazanou jednotku
		// p�id� jednotku do seznamu jednotek typu jednotky
		m_pUnitType->AddUnit ( this );
	}
}

// inicializace seznamu vid�n�ch jednotek
void CSUnit::PersistentInitSeenUnitList () 
{
	// zjist� pozici prvn� jednotky v seznamu vid�n�ch jednotek
	POSITION posSeenUnit = m_cSeenUnitList.GetHeadPosition ();
	// vid�n� jednotka
	CSUnit *pSeenUnit;

	// projede jednotky v seznamu vid�n�ch jednotek
	while ( CSUnitList::GetNext ( posSeenUnit, pSeenUnit ) )
	{
		// zkontroluje civilizaci vid�n� jednotky
		LOAD_ASSERT ( pSeenUnit->m_dwCivilizationIndex != m_dwCivilizationIndex );

		// zv��� po�et jednotek sv� civilizace, kter� vid�nou jednotku vid�
		pSeenUnit->m_aCivilizationVisibility[m_dwCivilizationIndex]++;
	}
}

//////////////////////////////////////////////////////////////////////
// Odkazy na instanci jednotky
//////////////////////////////////////////////////////////////////////

// zaznamen� uvoln�n� odkazu na jednotku (ukazatel na jednotku m��e b�t d�le neplatn�)
void CSUnit::Release () 
{
	// sn�� po�et odkaz� na jednotku
	DWORD dwReferenceCounter = ::InterlockedDecrement ( (long *)&m_dwReferenceCounter );
	ASSERT ( dwReferenceCounter != DWORD_MAX );

	// zjist�, jedn�-li se o posledn� odkaz na jednotku
	if ( dwReferenceCounter == 0 )
	{	// jedn� se o posledn� odkaz na jednotku
		ASSERT ( m_pZUnit != NULL );

		// zjist�, je-li jednotka mrtv�
		if ( m_nLives <= 0 )
		{	// jednotka je mrtv�
			// pokus� se zni�it jednotku na civilizaci
			if ( m_pZUnit->TryDelete () )
			{	// poda�ilo se zni�it jednotku na civilizaci
				// zamkne seznam smazan�ch jednotek
				VERIFY ( g_cMap.m_mutexDeletedUnitListLock.Lock () );

				// najde jednotku v seznamu smazan�ch jednotek
				POSITION posUnit = g_cMap.m_cDeletedUnitList.Find ( this );
				ASSERT ( posUnit != NULL );
				// sma�e jednotku ze seznamu smazan�ch jednotek
				VERIFY ( g_cMap.m_cDeletedUnitList.RemovePosition ( posUnit ) == this );
				ASSERT ( g_cMap.m_cDeletedUnitList.Find ( this ) == NULL );

				// odemkne seznam smazan�ch jednotek
				VERIFY ( g_cMap.m_mutexDeletedUnitListLock.Unlock () );

				// zni�� jednotku
				Delete ();
				delete this;
			}
			// nepoda�ilo se zni�it jednotku na civilizaci
		}
		// jednotka nen� mrtv�
	}
	// na jednotku existuj� odkazy
}

//////////////////////////////////////////////////////////////////////
// Info struktury jednotky
//////////////////////////////////////////////////////////////////////

// po�le full info jednotky klientovi "pClient" (jednotka mus� b�t zam�ena pro �ten�)
void CSUnit::SendFullInfo ( struct SUnitFullInfo *pFullInfo, 
	CZClientUnitInfoSender *pClient ) 
{
	// vypln� brief info
	GetBriefInfo ( pFullInfo );

	// vypln� dal�� specifick� informace full infa
	pFullInfo->dwInvisibilityState = m_dwInvisibilityState;
	pFullInfo->dwInvisibilityDetection = m_dwInvisibilityDetection;

	// po�le za��tek full infa jednotky
	pClient->StartSendingFullInfo ( pFullInfo );

	// buffer full infa skilly
	CString strFullInfoBuffer;

	// po�le ID jednotky
#ifdef _DEBUG
	strFullInfoBuffer.Format ( "Unit ID=%d, server ID=%d\n", m_pZUnit->GetID (), m_dwID );
#else //!_DEBUG
	strFullInfoBuffer.Format ( "Unit ID=%d\n", m_pZUnit->GetID () );
#endif //!_DEBUG
	pClient->SendingFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		strFullInfoBuffer.GetLength () );

  {
    // zamknout jednotku pro cteni dat & typu jednotky
    GetZUnit()->GetCivilization()->LockUnitData();

    // po�le jm�no jednotky
    strFullInfoBuffer.Format ( "%s\n", GetZUnit()->GetName());
	  pClient->SendingFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		  strFullInfoBuffer.GetLength () );

    // po�le jm�no skriptu
    CString strScriptType = (CString)*GetZUnit()->m_pIUnitType->m_stiName;
    strFullInfoBuffer.Format ( "Assigned script:\n%s\n", strScriptType);
	  pClient->SendingFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		  strFullInfoBuffer.GetLength () );

    // po�le stav
    strFullInfoBuffer.Format ( "%s\n\n", GetZUnit()->GetStatus() );
	  pClient->SendingFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		  strFullInfoBuffer.GetLength () );
  
    // odemknout jednotku
    GetZUnit()->GetCivilization()->UnlockUnitData();
  }

	// po�le stav �ivot� jednotky
	strFullInfoBuffer.Format ( "Lives: %d%% (%d/%d)", (int)( ( (double)(m_nLives * 100) / 
		(double)m_nLivesMax ) + (double)0.5 ), m_nLives, m_nLivesMax );
	pClient->SendingFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		strFullInfoBuffer.GetLength () );

	// zjist� po�et skill jednotky
	DWORD dwSkillCount = m_pUnitType->GetSkillCount ();
	// projede v�echny skilly jednotky
	for ( ; dwSkillCount-- > 0; )
	{
		// vypr�zdnit buffer
		strFullInfoBuffer.Empty ();
		// vypln� full info skilly
		m_pUnitType->GetSkillTypeByIndex ( dwSkillCount )->FillFullInfo ( this, 
			strFullInfoBuffer );

		// zjist�, je-li full info skilly pr�zdn�
		if ( !strFullInfoBuffer.IsEmpty () )
		{	// full info skilly nen� pr�zdn�
			// zform�tuje full info skilly do ��dky
			CString strFullInfoLine ( "\n" );
			strFullInfoLine += strFullInfoBuffer;
			// po�le full info skilly klientovi
			pClient->SendingFullInfo ( (const BYTE *)(LPCSTR)strFullInfoLine, 
				strFullInfoLine.GetLength () );
		}
	}

	// po�le ukon�en� full infa jednotky
	pClient->StopSendingFullInfo ();
}

// po�le enemy full info jednotky klientovi "pClient" (jednotka mus� b�t zam�ena pro �ten�)
void CSUnit::SendEnemyFullInfo ( struct SUnitEnemyFullInfo *pEnemyFullInfo, 
	CZClientUnitInfoSender *pClient ) 
{
	// vypln� brief info
	GetBriefInfo ( pEnemyFullInfo );

	// vypln� dal�� specifick� informace enemy full infa
	pEnemyFullInfo->dwInvisibilityState = m_dwInvisibilityState;

	// po�le za��tek enemy full infa jednotky
	pClient->StartSendingEnemyFullInfo ( pEnemyFullInfo );

	// buffer enemy full infa skilly
	CString strEnemyFullInfoBuffer;

	// po�le ID jednotky
#ifdef _DEBUG
	strEnemyFullInfoBuffer.Format ( "Unit ID=%d, server ID=%d\n", m_pZUnit->GetID (), m_dwID );
#else //!_DEBUG
	strEnemyFullInfoBuffer.Format ( "Unit ID=%d\n", m_pZUnit->GetID () );
#endif //!_DEBUG
	pClient->SendingEnemyFullInfo ( (const BYTE *)(LPCSTR)strEnemyFullInfoBuffer, 
		strEnemyFullInfoBuffer.GetLength () );

	// po�le stav �ivot� jednotky
	strEnemyFullInfoBuffer.Format ( "Lives: %d%% (%d/%d)", (int)( ( (double)(m_nLives * 100) / 
		(double)m_nLivesMax ) + (double)0.5 ), m_nLives, m_nLivesMax );
	pClient->SendingEnemyFullInfo ( (const BYTE *)(LPCSTR)strEnemyFullInfoBuffer, 
		strEnemyFullInfoBuffer.GetLength () );

	// zjist� po�et skill jednotky
	DWORD dwSkillCount = m_pUnitType->GetSkillCount ();
	// projede v�echny skilly jednotky
	for ( ; dwSkillCount-- > 0; )
	{
		// vypr�zdnit buffer
		strEnemyFullInfoBuffer.Empty ();
		// vypln� enemy full info skilly
		m_pUnitType->GetSkillTypeByIndex ( dwSkillCount )->FillEnemyFullInfo ( this, 
			strEnemyFullInfoBuffer );

		// zjist�, je-li enemy full info skilly pr�zdn�
		if ( !strEnemyFullInfoBuffer.IsEmpty () )
		{	// enemy full info skilly nen� pr�zdn�
			// zform�tuje enemy full info skilly do ��dky
			CString strEnemyFullInfoLine ( "\n" );
			strEnemyFullInfoLine += strEnemyFullInfoBuffer;
			// po�le enemy full info skilly klientovi
			pClient->SendingEnemyFullInfo ( (const BYTE *)(LPCSTR)strEnemyFullInfoLine, 
				strEnemyFullInfoLine.GetLength () );
		}
	}

	// po�le ukon�en� enemy full infa jednotky
	pClient->StopSendingEnemyFullInfo ();
}

// po�le start full info jednotky klientovi "pClient" (jednotka mus� b�t zam�ena pro �ten�)
void CSUnit::SendStartFullInfo ( struct SUnitStartFullInfo *pStartFullInfo, 
	CZClientUnitInfoSender *pClient ) 
{
	// vypln� start info
	GetStartInfo ( pStartFullInfo );

	// vypln� dal�� specifick� informace start full infa
	pStartFullInfo->dwInvisibilityState = m_dwInvisibilityState;
	pStartFullInfo->dwInvisibilityDetection = m_dwInvisibilityDetection;

	// po�le za��tek start full infa jednotky
	pClient->StartSendingStartFullInfo ( pStartFullInfo );

	// buffer full infa skilly
	CString strFullInfoBuffer;

	// po�le ID jednotky
#ifdef _DEBUG
	strFullInfoBuffer.Format ( "Unit ID=%d, server ID=%d\n", m_pZUnit->GetID (), m_dwID );
#else //!_DEBUG
	strFullInfoBuffer.Format ( "Unit ID=%d\n", m_pZUnit->GetID () );
#endif //!_DEBUG
	pClient->SendingStartFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		strFullInfoBuffer.GetLength () );

  {
    // zamknout jednotku pro cteni dat & typu jednotky
    GetZUnit()->GetCivilization()->LockUnitData();

    // po�le jm�no jednotky
    strFullInfoBuffer.Format ( "%s\n", GetZUnit()->GetName() );
	pClient->SendingStartFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		  strFullInfoBuffer.GetLength () );

    // po�le jm�no skriptu
    strFullInfoBuffer.Format ( "Assigned script:\n%s\n", (CString)*GetZUnit()->m_pIUnitType->m_stiName );
	  pClient->SendingStartFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		  strFullInfoBuffer.GetLength () );

    // po�le stav
    strFullInfoBuffer.Format ( "%s\n\n", GetZUnit()->GetStatus() );
	  pClient->SendingStartFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		  strFullInfoBuffer.GetLength () );

    // odemknout jednotku
    GetZUnit()->GetCivilization()->UnlockUnitData();
  }

  // po�le stav �ivot� jednotky
	strFullInfoBuffer.Format ( "Lives: %d%% (%d/%d)", (int)( ( (double)(m_nLives * 100) / 
		(double)m_nLivesMax ) + (double)0.5 ), m_nLives, m_nLivesMax );
	pClient->SendingStartFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		strFullInfoBuffer.GetLength () );

	// zjist� po�et skill jednotky
	DWORD dwSkillCount = m_pUnitType->GetSkillCount ();
	// projede v�echny skilly jednotky
	for ( ; dwSkillCount-- > 0; )
	{
		// vypr�zdnit buffer
		strFullInfoBuffer.Empty ();
		// vypln� full info skilly
		m_pUnitType->GetSkillTypeByIndex ( dwSkillCount )->FillFullInfo ( this, 
			strFullInfoBuffer );

		// zjist�, je-li full info skilly pr�zdn�
		if ( !strFullInfoBuffer.IsEmpty () )
		{	// full info skilly nen� pr�zdn�
			// zform�tuje full info skilly do ��dky
			CString strFullInfoLine ( "\n" );
			strFullInfoLine += strFullInfoBuffer;
			// po�le full info skilly klientovi
			pClient->SendingStartFullInfo ( (const BYTE *)(LPCSTR)strFullInfoLine, 
				strFullInfoLine.GetLength () );
		}
	}

	// po�le ukon�en� start full infa jednotky
	pClient->StopSendingStartFullInfo ();
}

// po�le start enemy full info jednotky klientovi "pClient" (jednotka mus� b�t zam�ena 
//		pro �ten�)
void CSUnit::SendStartEnemyFullInfo ( 
	struct SUnitStartEnemyFullInfo *pStartEnemyFullInfo, CZClientUnitInfoSender *pClient ) 
{
	// vypln� brief info
	GetStartInfo ( pStartEnemyFullInfo );

	// vypln� dal�� specifick� informace start enemy full infa
	pStartEnemyFullInfo->dwInvisibilityState = m_dwInvisibilityState;

	// po�le za��tek start enemy full infa jednotky
	pClient->StartSendingStartEnemyFullInfo ( pStartEnemyFullInfo );

	// buffer enemy full infa skilly
	CString strEnemyFullInfoBuffer;

	// po�le ID jednotky
#ifdef _DEBUG
	strEnemyFullInfoBuffer.Format ( "Unit ID=%d, server ID=%d\n", m_pZUnit->GetID (), m_dwID );
#else //!_DEBUG
	strEnemyFullInfoBuffer.Format ( "Unit ID=%d\n", m_pZUnit->GetID () );
#endif //!_DEBUG
	pClient->SendingStartEnemyFullInfo ( (const BYTE *)(LPCSTR)strEnemyFullInfoBuffer, 
		strEnemyFullInfoBuffer.GetLength () );

	// po�le stav �ivot� jednotky
	strEnemyFullInfoBuffer.Format ( "Lives: %d%% (%d/%d)", (int)( ( (double)(m_nLives * 100) / 
		(double)m_nLivesMax ) + (double)0.5 ), m_nLives, m_nLivesMax );
	pClient->SendingStartEnemyFullInfo ( (const BYTE *)(LPCSTR)strEnemyFullInfoBuffer, 
		strEnemyFullInfoBuffer.GetLength () );

	// zjist� po�et skill jednotky
	DWORD dwSkillCount = m_pUnitType->GetSkillCount ();
	// projede v�echny skilly jednotky
	for ( ; dwSkillCount-- > 0; )
	{
		// vypr�zdnit buffer
		strEnemyFullInfoBuffer.Empty ();
		// vypln� enemy full info skilly
		m_pUnitType->GetSkillTypeByIndex ( dwSkillCount )->FillEnemyFullInfo ( this, 
			strEnemyFullInfoBuffer );

		// zjist�, je-li enemy full info skilly pr�zdn�
		if ( !strEnemyFullInfoBuffer.IsEmpty () )
		{	// enemy full info skilly nen� pr�zdn�
			// zform�tuje enemy full info skilly do ��dky
			CString strEnemyFullInfoLine ( "\n" );
			strEnemyFullInfoLine += strEnemyFullInfoBuffer;
			// po�le enemy full info skilly klientovi
			pClient->SendingStartEnemyFullInfo ( (const BYTE *)(LPCSTR)strEnemyFullInfoLine, 
				strEnemyFullInfoLine.GetLength () );
		}
	}

	// po�le ukon�en� start enemy full infa jednotky
	pClient->StopSendingStartEnemyFullInfo ();
}

//////////////////////////////////////////////////////////////////////
// Informace o typech skill jednotky
//////////////////////////////////////////////////////////////////////

// vr�t� typ skilly s ID jm�na "pNameID" (up�ednost�uje typ skilly povolen� v aktu�ln�m 
//		m�du jednotky) (NULL=typ skilly neexistuje)
CSSkillType *CSUnit::GetSkillTypeByNameID ( CStringTableItem *pNameID ) 
{
	// zjist� ukazatel na prvn� popis typu skilly
	CSUnitType::SSkillTypeDescription *pSkillTypeDescription = 
		m_pUnitType->m_aSkillTypeDescription;
	// po�et typ� skill typu jednotky
	DWORD dwSkillTypeCount = m_pUnitType->GetSkillCount ();

	// maska m�du jednotky
	BYTE nModeMask = 0x01 << m_nMode;

	// prvn� nalezen� typ skilly s ID jm�na "pNameID"
	CSSkillType *pFirstFoundSkillType = NULL;

	// projede v�echny typy skill
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		ASSERT ( pSkillTypeDescription->pNameID != NULL );
		// zjist�, jedn�-li se o hledan� typ skilly
		if ( pSkillTypeDescription->pNameID == pNameID )
		{	// jedn� se o hledan� typ skilly
			ASSERT ( pSkillTypeDescription->pSkillType != NULL );
			// zjist�, jedn�-li se o povolenou skillu
			if ( pSkillTypeDescription->bEnabled && 
				( pSkillTypeDescription->nAllowedModesMask & nModeMask ) )
			{	// jedn� se o povolenou skillu
				// vr�t� typ skilly
				return pSkillTypeDescription->pSkillType;
			}

			// zjist�, jedn�-li se o prvn� nalezen� typ skilly
			if ( pFirstFoundSkillType == NULL )
			{	// jedn� se o prvn� nalezen� typ skilly
				// aktualizuje ukazatel na prvn� nalezen� typ skilly
				pFirstFoundSkillType = pSkillTypeDescription->pSkillType;
			}
		}
	}

	// vr�t� prvn� nalezen� typ skilly
	return pFirstFoundSkillType;
}

//////////////////////////////////////////////////////////////////////
// Get/Set metody hern�ch informac� o jednotce
//////////////////////////////////////////////////////////////////////

// nastav� m�d jednotky (0-7) (jednotka mus� b�t zam�ena pro z�pis, vol� 
//		SetBriefInfoModified, je-li pot�eba)
void CSUnit::SetMode ( BYTE nMode ) 
{
	// zkontroluje ��slo m�du jednotky (0-7)
	ASSERT ( nMode < 8 );
	ASSERT ( m_nMode < 8 );

	// zjist�, jedn�-li se o zm�nu m�du
	if ( m_nMode == nMode )
	{	// nejedn� se o zm�nu m�du
		return;
	}
	// jedn� se o zm�nu m�du

	// maska m�du jednotky
	BYTE nModeMask = 0x01 << nMode;
	// uschov� star� m�d jednotky
	BYTE nOldMode = m_nMode;

// deaktivuje aktivn� skilly jednotky, kter� jsou zak�zan� v nov�m m�du jednotky

	// ukazatel na prvn� popis typu skilly
	CSUnitType::SSkillTypeDescription *pSkillTypeDescription = 
		m_pUnitType->m_aSkillTypeDescription;
	// po�et typ� skill typu jednotky
	DWORD dwSkillTypeCount = m_pUnitType->GetSkillCount ();

	// projede v�echny typy skill
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		// zjist�, je-li skilla aktivn� a zak�zan� v nov�m m�du jednotky
		if ( IsSkillActive ( dwSkillTypeIndex ) && 
			!( pSkillTypeDescription->nAllowedModesMask & nModeMask ) )
		{	// skilla je aktivn� a je zak�zan� v nov�m m�du jednotky
			ASSERT ( pSkillTypeDescription->bEnabled && 
				( pSkillTypeDescription->nAllowedModesMask & ( 0x01 << nOldMode ) ) );
			ASSERT ( pSkillTypeDescription->pSkillType != NULL );
			// deaktivuje skillu jednotky
			pSkillTypeDescription->pSkillType->DeactivateSkill ( this );
		}
	}

// zm�n� m�d jednotky

	// nastav� nov� m�d jednotky
	m_nMode = nMode;

	// aktualizuje informace o jednotce
	m_dwInvisibilityState = m_pUnitType->m_aInvisibilityState[nMode];
	m_dwInvisibilityDetection = m_pUnitType->m_aInvisibilityDetection[nMode];
	m_dwAppearanceID = m_pUnitType->m_aAppearanceID[nMode];

// informuje povolen� skilly jednotky o zm�n� m�du

	// ukazatel na prvn� popis typu skilly
	pSkillTypeDescription = m_pUnitType->m_aSkillTypeDescription;
	// po�et typ� skill typu jednotky
	dwSkillTypeCount = m_pUnitType->GetSkillCount ();

	// projede typy skill
	for ( dwSkillTypeIndex = 0; dwSkillTypeIndex++ < dwSkillTypeCount; 
		pSkillTypeDescription++ )
	{
		// zjist�, je-li skilla povolen� v m�du jednotky
		if ( pSkillTypeDescription->bEnabled && 
			( pSkillTypeDescription->nAllowedModesMask & nModeMask ) )
		{	// skilla je povolen� v m�du jednotky
			ASSERT ( pSkillTypeDescription->pSkillType != NULL );
			// informuje skillu o zm�n� m�du jednotky
			pSkillTypeDescription->pSkillType->ModeChanged ( this, nOldMode );
		}
	}

	// nastav� p��znak zm�ny brief infa
	SetBriefInfoModified ();
}

// nastav� masku m�du jednotky (mus� b�t nastaven pr�v� jeden bit) (jednotka mus� b�t 
//		zam�ena pro z�pis, vol� SetBriefInfoModified, je-li pot�eba)
void CSUnit::SetModeMask ( BYTE nModeMask ) 
{
	// zkontroluje je-li nastaven pr�v� jeden bit
	ASSERT ( ( nModeMask & 0xff ) && !( nModeMask & ( nModeMask - (BYTE)1 ) ) );

	// m�d jednotky
	BYTE nMode;

	// zjist�, o jak� m�d jednotky se jedn�
	if ( nModeMask & 0x0f )
	{	// nastaven� bit je 0-3
		if ( nModeMask & 0x03 )
		{	// nastaven� bit je 0-1
			nMode = ( nModeMask & 0x01 ) ? 0 : 1;
		}
		else
		{	// nastaven� bit je 2-3
			nMode = ( nModeMask & 0x04 ) ? 2 : 3;
		}
	}
	else
	{	// nastaven� bit je 4-7
		if ( nModeMask & 0x30 )
		{	// nastaven� bit je 4-5
			nMode = ( nModeMask & 0x10 ) ? 4 : 5;
		}
		else
		{	// nastaven� bit je 6-7
			nMode = ( nModeMask & 0x40 ) ? 6 : 7;
		}
	}

	// nech� nastavit m�d jednotky
	SetMode ( nMode );
}

//////////////////////////////////////////////////////////////////////
// Debuggovac� informace
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// zkontroluje spr�vn� data objektu (TRUE=OK)
BOOL CSUnit::CheckValidData () 
{
	// zkontroluje kontroln� body cesty jednotky a pozici jednotky
	if ( g_cMap.IsMapPosition ( m_pointPosition ) )
	{
		ASSERT ( g_cMap.IsMapPosition ( m_cFirstCheckPoint.GetPosition () ) );
		ASSERT ( g_cMap.IsMapPosition ( m_cSecondCheckPoint.GetPosition () ) );
		ASSERT ( m_cFirstCheckPoint.GetTime () <= m_cSecondCheckPoint.GetTime () );
		if ( m_cFirstCheckPoint.GetTime () < g_cMap.GetTimeSlice () )
		{
			// *******************************
			//ASSERT ( m_cFirstCheckPoint.GetPosition () == m_pointPosition );
			//ASSERT ( m_cFirstCheckPoint.GetTime () == m_cSecondCheckPoint.GetTime () );
		}
	}
	else
	{
		ASSERT ( !g_cMap.IsMapPosition ( m_cFirstCheckPoint.GetPosition () ) );
		ASSERT ( m_cFirstCheckPoint.GetTime () <= g_cMap.GetTimeSlice () );
		ASSERT ( !g_cMap.IsMapPosition ( m_cSecondCheckPoint.GetPosition () ) );
		ASSERT ( m_cSecondCheckPoint.GetTime () <= g_cMap.GetTimeSlice () );
	}

	ASSERT ( m_dwVerticalPosition != 0 );
	ASSERT ( m_nDirection <= Dir_Random );

	ASSERT ( m_pZUnit != NULL );
	ASSERT ( m_pLock != NULL );
	ASSERT ( m_pUnitType != NULL );
	ASSERT ( m_dwCivilizationIndex < g_cMap.GetCivilizationCount () );
	ASSERT ( m_dwCivilizationIndex >= 0 );
	ASSERT ( m_dwReferenceCounter != DWORD_MAX );

	ASSERT ( m_pSkillData != NULL );

	ASSERT ( m_aCivilizationVisibility != NULL );
	ASSERT ( m_aCivilizationVisibility[m_dwCivilizationIndex] == 1 );

	// nech� zkontrolovat naposledy vid�n� pozice jednotky civilizacemi
	for ( DWORD dwCivilizationIndex = g_cMap.GetCivilizationCount (); 
		dwCivilizationIndex-- > 0; )
	{
		// nech� zkontrolovat naposledy vid�nou pozici jednotky civilizace
		if ( g_cMap.IsMapPosition ( m_aCivilizationLastSeenPosition[dwCivilizationIndex] ) )
		{
			ASSERT ( m_aCivilizationLastSeenPosition[dwCivilizationIndex].z != 0 );
		}
		else
		{
			ASSERT ( m_aCivilizationLastSeenPosition[dwCivilizationIndex].z == 0 );
		}
	}

	return TRUE;
}

// zkontroluje ne�kodn� data objektu (TRUE=OK)
BOOL CSUnit::CheckEmptyData () 
{
	ASSERT ( m_nLives == 0 );
	ASSERT ( m_nLivesMax == 0 );
	ASSERT ( m_dwViewRadius == 0 );
	ASSERT ( m_dwViewRadiusSquare == 0 );
	ASSERT ( m_pointPosition == CPointDW ( DWORD_MAX, DWORD_MAX ) );
	ASSERT ( m_dwVerticalPosition == 0 );
	ASSERT ( m_strUnitName.IsEmpty () );
	ASSERT ( m_nDirection == 0xff );
	ASSERT ( m_nMode == 8 );
	ASSERT ( m_dwInvisibilityState == 0 );
	ASSERT ( m_dwInvisibilityDetection == 0 );
	ASSERT ( m_dwAppearanceID == DWORD_MAX );

	ASSERT ( ( m_cFirstCheckPoint.GetTime () == 0 ) && ( m_cFirstCheckPoint.GetPosition () == 
		CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ) ) );
	ASSERT ( ( m_cSecondCheckPoint.GetTime () == 0 ) && ( m_cSecondCheckPoint.GetPosition () == 
		CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ) ) );

	ASSERT ( m_pZUnit == NULL );
	ASSERT ( m_dwID == DWORD_MAX );
	ASSERT ( m_pLock == NULL );
	ASSERT ( m_pUnitType == NULL );
	ASSERT ( m_dwCivilizationIndex == CIVILIZATION_COUNT_MAX );

	ASSERT ( m_dwModificationFlags == 0 );

	ASSERT ( m_cActiveSkills.IsDeleted () );
	ASSERT ( m_pSkillData == NULL );

	ASSERT ( m_aCivilizationVisibility == NULL );
	ASSERT ( m_dwWatchingCivilizationClientCount == 0 );
	ASSERT ( m_aCivilizationLastSeenPosition == NULL );

	ASSERT ( m_cSeenUnitList.IsEmpty () );

	ASSERT ( m_dwReferenceCounter == 0 );

	ASSERT ( m_pNextUnit == NULL );

	return TRUE;
}

#endif //_DEBUG
