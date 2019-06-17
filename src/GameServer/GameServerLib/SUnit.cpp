/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída jednotky na serveru hry. Obsahuje všechny 
 *          informace o jednotce, které potøebuje GameServer. 
 *          Každá jednotka na mapì (vojáèek, budova, atp.) 
 *          je reprezentována svoji vlastní instancí této tøídy.
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

// poèet TimeSlicù zpoždìní smazání jednotky (minimálnì 1)
#define UNIT_DELETE_DELAY		20

IMPLEMENT_DYNAMIC ( CSUnit, CPersistentObject )

//////////////////////////////////////////////////////////////////////
// Statická data tøídy
//////////////////////////////////////////////////////////////////////

// další generované ID jednotky
DWORD CSUnit::m_dwNextGeneratedID = 0;

// memory pool polí poètu jednotek civilizací vidících jenotku
CMemoryPool CSUnit::m_cCivilizationVisibilityArrayMemoryPool ( 100 );
// memory pool polí posledních pozic, kde byla jednotka civilizacemi vidìna
CMemoryPool CSUnit::m_cCivilizationLastSeenPositionArrayMemoryPool ( 100 );

// tabulka smìrù kladné poloroviny osy X
BYTE CSUnit::m_aDirectionTableXPositive[5] = 
	{ Dir_South, Dir_SouthEast, Dir_East, Dir_NorthEast, Dir_North };
// tabulka smìrù záporné poloroviny osy X
BYTE CSUnit::m_aDirectionTableXNegative[5] = 
	{ Dir_North, Dir_NorthWest, Dir_West, Dir_SouthWest, Dir_South };

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSUnit::CSUnit () 
{
	// zneškodní data jednotky
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
	// zkontroluje neškodná data jednotky
	ASSERT ( CheckEmptyData () );
}

//////////////////////////////////////////////////////////////////////
// Inicializace a znièení dat objektu
//////////////////////////////////////////////////////////////////////

// pøedvytvoøí jednotku typu "pUnitType"
void CSUnit::PreCreate ( CSUnitType *pUnitType ) 
{
	// zkontroluje neškodná data jednotky
	ASSERT ( CheckEmptyData () );

	// zkontroluje inicializaèní data jednotky
	ASSERT ( pUnitType != NULL );

	// inicializuje herní informace o jednotce
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

	// inicializuje serverové informace o jednotce
	GenerateID ();
	m_pUnitType = pUnitType;
	m_dwCivilizationIndex = pUnitType->GetCivilization ()->GetCivilizationIndex ();

//	TRACE ( "Creating unit ID=%d (@0x%x)\n", m_dwID, this );

	m_dwModificationFlags = MF_Clear;

	// inicializuje bitové pole pøíznakù active/deactive skill jednotky na 0
	m_cActiveSkills.Create ( pUnitType->GetSkillCount (), FALSE );
	// alokuje data skill
	m_pSkillData = new BYTE[pUnitType->GetSkillDataSize ()];

	// zamkne memory pooly polí civilizací jednotky
	VERIFY ( m_mutexCivilizationArrayMemoryPoolLock.Lock () );

	// vytvoøí pole poètu jednotek jednotlivých civilizací, které jednotku vidí
	m_aCivilizationVisibility = (WORD *)m_cCivilizationVisibilityArrayMemoryPool.Allocate ();
	// vytvoøí pole posledních pozic, kde byla jednotka civilizacemi vidìna
	m_aCivilizationLastSeenPosition = 
		(CSPosition *)m_cCivilizationLastSeenPositionArrayMemoryPool.Allocate ();

	// odemkne memory pooly polí civilizací jednotky
	VERIFY ( m_mutexCivilizationArrayMemoryPoolLock.Unlock () );

	// inicializuje pole poètu jednotek jednotlivých civilizací, které jednotku vidí
	memset ( m_aCivilizationVisibility, 0, g_cMap.GetCivilizationCount () * 
		sizeof ( WORD ) );
	m_aCivilizationVisibility[m_dwCivilizationIndex] = 1;
	// inicializuje pole posledních pozic, kde byla jednotka civilizacemi vidìna
	for ( DWORD dwCivilizationIndex = g_cMap.GetCivilizationCount (); 
		dwCivilizationIndex-- > 0; )
	{
		m_aCivilizationLastSeenPosition[dwCivilizationIndex].x = NO_MAP_POSITION;
		m_aCivilizationLastSeenPosition[dwCivilizationIndex].y = NO_MAP_POSITION;
		m_aCivilizationLastSeenPosition[dwCivilizationIndex].z = 0;
	}
	// inicializuje poèet klientù civilizací sledujících jednotku
	m_dwWatchingCivilizationClientCount = 0;

	// inicializuje skilly jednotky
	pUnitType->InitializeSkills ( this );

	// zneškodní ukazatel na jednotku na civilizaci
	m_pZUnit = NULL;
}

// vytvoøí jednotku typu "pUnitType" (typ jednotky musí být zamèen pro zápis)
void CSUnit::Create ( CSUnitType *pUnitType ) 
{
	// pøedvytvoøí jednotku
	PreCreate ( pUnitType );

	// vytvoøí jednotku na civilizaci
	m_pZUnit = pUnitType->GetCivilization ()->m_pZCivilization->CreateUnit ( this );
	LOAD_ASSERT ( m_pZUnit != NULL );

	// pøidá jednotku do seznamu jednotek typu jednotky
	pUnitType->AddUnit ( this );
}

// znièí data jednotky
void CSUnit::Delete () 
{
	ASSERT ( m_nLives <= 0 );
	ASSERT ( m_dwReferenceCounter == 0 );

	// zjistí, je-li platný ukazatel na typ jednotky
	if ( m_pUnitType != NULL )
	{	// ukazatel na typ jednotky je platný
		// odebere jednotku ze seznamu jednotek typu jednotky
		m_pUnitType->DeleteUnit ( this );
	}

	// zruší odkaz na jednotku na civilizaci
	m_pZUnit = NULL;

	// odregistruje ID jednotky
	UnregisterID ();

	// zjistí, jsou-li platné skilly jednotky
	if ( m_pSkillData != NULL )
	{	// skilly jednotky jsou platné
		// zjistí, je-li platný ukazatel na typ jednotky
		if ( m_pUnitType != NULL )
		{	// ukazatel na typ jednotky je platný
			// znièí skilly jednotky
			m_pUnitType->DestructSkills ( this );
		}
		// znièí data skill
		delete [] m_pSkillData;
		m_pSkillData = NULL;
	}

	// znièí seznam vidìných jednotek
	m_cSeenUnitList.RemoveAll ();

	// zneškodní data jednotky
	SetEmptyData ();
}

// vyplní objekt neškodnými daty (znièí èásteènì inicializavaná data)
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

	// zjistí, je-li znièeno pole pøíznakù active/deactive skill jednotky
	if ( !m_cActiveSkills.IsDeleted () )
	{	// pole pøíznakù active/deactive skill jednotky není znièeno
		// znièí bitové pole pøíznakù active/deactive skill jednotky
		m_cActiveSkills.Delete ();
	}
	// zjistí, jsou-li alokována data skill
	if ( m_pSkillData != NULL )
	{	// data skill jsou alokována
		// znièí data skill
		delete [] m_pSkillData;
		m_pSkillData = NULL;
	}

	// zjistí, je-li alokováno nìjaké pole civilizací jednotky
	if ( ( m_aCivilizationVisibility != NULL ) || 
		( m_aCivilizationLastSeenPosition != NULL ) )
	{	// je alokována nìjaké pole civilizací jednotky
		// zamkne memory pooly polí civilizací jednotky
		VERIFY ( m_mutexCivilizationArrayMemoryPoolLock.Lock () );

		// zjistí, je-li alokováno pole poètu jednotek civilizací vidících jednotku
		if ( m_aCivilizationVisibility != NULL )
		{	// pole poètu jednotek civilizací vidících jednotku je alokováno
			// znièí pole poètu jednotek civilizací vidících jednotku
			m_cCivilizationVisibilityArrayMemoryPool.Free ( m_aCivilizationVisibility );
			m_aCivilizationVisibility = NULL;
		}
		// zjistí, je-li alokováno pole posledních pozic, kde byla jednotka civilizacemi vidìna
		if ( m_aCivilizationLastSeenPosition != NULL )
		{	// pole posledních pozic, kde byla jednotka civilizacemi vidìna je alokováno
			// znièí pole posledních pozic, kde byla jednotka civilizacemi vidìna
			m_cCivilizationLastSeenPositionArrayMemoryPool.Free ( 
				m_aCivilizationLastSeenPosition );
			m_aCivilizationLastSeenPosition = NULL;
		}

		// odemkne memory pooly polí civilizací jednotky
		VERIFY ( m_mutexCivilizationArrayMemoryPoolLock.Unlock () );
	}

	m_dwWatchingCivilizationClientCount = 0;

	m_dwReferenceCounter = 0;
}

// vygeneruje a zaregistruje nové ID jednotky
void CSUnit::GenerateID () 
{
	// vygeneruje ID a zvìtší další generované ID jednotky
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

// zpracuje zabití jednotky (lze volat pouze z MainLoop threadu, jednotka musí být zamèena 
//		pro zápis, MapSquary musí být zamèeny)
void CSUnit::UnitDied () 
{
	ASSERT ( g_cMap.IsMainLoopThread () );
	ASSERT ( CheckValidData () );

	ASSERT ( m_nLives <= 0 );

	// aktualizuje poèat životù jednotky
	m_nLives = DEAD_UNIT_LIVES;

	// znièí skilly jednotky
	m_pUnitType->DestructSkills ( this );

	// smaže jednotku z jednotek mapy
	g_cMap.DeleteUnit ( this );

	// pøidá jednotku do fronty mrtvých jednotek
	g_cMap.m_cDeadUnitQueue.Add ( struct CSMap::SDeadUnit ( this, 
		g_cMap.GetTimeSlice () + UNIT_DELETE_DELAY ) );

	// uschová si pozici jednotky
	CPointDW pointPosition = m_pointPosition;

	// odebere jednotku z mapy, umístí jí mimo mapu
	g_cMap.RemoveUnit ( this );

	// obnoví pozici jednotky
	// ****************** NEVIM: m_pointPosition = pointPosition;

	// zamkne typ jednotky pro zápis
	VERIFY ( m_pUnitType->WriterLock () );

	// odebere jednotku ze seznamu jednotek typu jednotky
	m_pUnitType->DeleteUnit ( this );

	// odemkne typ jednotky 
	m_pUnitType->WriterUnlock ();

	// nechá znièit jednotku na civilizaci
	g_cMap.GetCivilization ( m_dwCivilizationIndex )->m_pZCivilization->UnitDied ( 
		m_pZUnit, pointPosition );
}

// informuje jednotku, že jednotka na civilizaci ztratila poslední odkaz na sebe
void CSUnit::ZUnitRefCountIsZero () 
{
	// zjistí, je-li jednotka mrtvá
	if ( m_nLives <= 0 )
	{	// jednotka je mrtvá
		ASSERT ( m_nLives == DEAD_UNIT_LIVES );
		// zjistí, existují-li odkazy na jednotku
		if ( m_dwReferenceCounter == 0 )
		{	// na jednotku neexistují odkazy
			ASSERT ( m_pZUnit != NULL );
			// znièí jednotku na civilizaci
			VERIFY ( m_pZUnit->TryDelete () );

			// zamkne seznam smazaných jednotek
			VERIFY ( g_cMap.m_mutexDeletedUnitListLock.Lock () );

			// najde jednotku v seznamu smazaných jednotek
			POSITION posUnit = g_cMap.m_cDeletedUnitList.Find ( this );
			ASSERT ( posUnit != NULL );
			// smaže jednotku ze seznamu smazaných jednotek
			VERIFY ( g_cMap.m_cDeletedUnitList.RemovePosition ( posUnit ) == this );
			ASSERT ( g_cMap.m_cDeletedUnitList.Find ( this ) == NULL );

			// odemkne seznam smazaných jednotek
			VERIFY ( g_cMap.m_mutexDeletedUnitListLock.Unlock () );

			// znièí jednotku
			Delete ();
			delete this;
		}
		// na jednotku existují odkazy
	}
	// jednotka není mrtvá
}

//////////////////////////////////////////////////////////////////////
// Ukládání dat (CPersistentObject metody)
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
	int nUnitNameSize							// mùže být i 0 - pak je vynecháno aUnitName
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

// ukládání dat
void CSUnit::PersistentSave ( CPersistentStorage &storage ) 
{
	// zkontroluje platnost dat jednotky
	ASSERT ( CheckValidData () );

	BRACE_BLOCK ( storage );

	// uloží ID jednotky
	storage << m_dwID;
	// uloží ukazatel na typ jednotky
	storage << m_pUnitType;
	// uloží ukazatel na jednotku na civilizaci
	storage << m_pZUnit;
	// uloží poèet odkazù na jednotku
	storage << m_dwReferenceCounter;

	// uloží poèet životù
	storage << m_nLives;
	// uloží maximální poèet životù
	storage << m_nLivesMax;

	// uloží polomìr viditelnosti
	storage << m_dwViewRadius;
	// uloží x-ovou souøadnici pozice jednotky
	storage << m_pointPosition.x;
	// uloží y-ovou souøadnici pozice jednotky
	storage << m_pointPosition.y;
	// uloží vertikální pozici jednotky
	storage << m_dwVerticalPosition;

	// zjistí délku jména jednotky
	int nUnitNameSize = m_strUnitName.GetLength ();
	// uloží délku jména jednotky
	storage << nUnitNameSize;
	// zjistí, je-li jméno jednotky prázdné
	if ( nUnitNameSize > 0 )
	{	// jméno jednotky není prázdné
		// uloží jméno jednotky
		storage.Write ( m_strUnitName, nUnitNameSize );
	}

	// uloží nasmìrování jednotky
	storage << m_nDirection;
	// uloží mód jednotky
	storage << m_nMode;
	// uloží pøíznaky neviditelnosti jednotky
	storage << m_dwInvisibilityState;
	// uloží pøíznaky detekce neviditelnosti ostatních jednotek
	storage << m_dwInvisibilityDetection;
	// uloží ID vzhledu jednotky
	storage << m_dwAppearanceID;

	// uloží první kontrolní bod cesty jednotky
	storage << m_cFirstCheckPoint.GetPosition ().x;
	storage << m_cFirstCheckPoint.GetPosition ().y;
	storage << m_cFirstCheckPoint.GetTime ();
	// uloží druhý kontrolní bod cesty jednotky
	storage << m_cSecondCheckPoint.GetPosition ().x;
	storage << m_cSecondCheckPoint.GetPosition ().y;
	storage << m_cSecondCheckPoint.GetTime ();

	// uloží pøíznaky zmìn dat jednotky
	storage << m_dwModificationFlags;

	// zjistí poèet skill jednotky
	DWORD dwSkillCount = m_pUnitType->GetSkillCount ();
	ASSERT ( dwSkillCount > 0 );
	// uloží pole pøíznakù aktivních skill
	storage.Write ( m_cActiveSkills.GetData (), ( dwSkillCount + 7 ) / 8 );
	// projede všechny skilly jednotky
	for ( ; dwSkillCount-- > 0; )
	{
		// uloží skillu jednotky
		m_pUnitType->GetSkillTypeByIndex ( dwSkillCount )->PersistentSaveSkill ( storage, this );
	}

	// uloží pole naposledy vidìných pozic jednotky civilizacemi
	for ( DWORD dwCivilizationIndex = g_cMap.GetCivilizationCount (); 
		dwCivilizationIndex-- > 0; )
	{
		// uloží naposledy vidìnou pozici jednotky civilizací "dwCivilizationIndex"
		storage << m_aCivilizationLastSeenPosition[dwCivilizationIndex].x;
		storage << m_aCivilizationLastSeenPosition[dwCivilizationIndex].y;
		storage << m_aCivilizationLastSeenPosition[dwCivilizationIndex].z;
	}

	// uloží poèet vidìných jednotek
	storage << m_cSeenUnitList.GetSize ();

	// zjistí pozici první jednotky v seznamu vidìných jednotek
	POSITION posSeenUnit = m_cSeenUnitList.GetHeadPosition ();
	// vidìná jednotka
	CSUnit *pSeenUnit;

	// projede jednotky v seznamu vidìných jednotek
	while ( CSUnitList::GetNext ( posSeenUnit, pSeenUnit ) )
	{
		// uloží ukazatel na vidìnou jednotku
		storage << pSeenUnit;
	}
}

// nahrávání pouze uložených dat s "dwCivilizationCount" civilizacemi
void CSUnit::PersistentLoad ( CPersistentStorage &storage, DWORD dwCivilizationCount ) 
{
	// zkontroluje neškodná data jednotky
	ASSERT ( CheckEmptyData () );

	BRACE_BLOCK ( storage );

	// naète ID jednotky
	storage >> m_dwID;
	// zaregistruje naètené ID jednotky
	RegisterID ();
	// naète ukazatel na typ jednotky
	storage >> (void *&)m_pUnitType;
	LOAD_ASSERT ( m_pUnitType != NULL );
	// pøeloží ukazatel na typ jednotky
	m_pUnitType = (CSUnitType *)storage.TranslatePointer ( m_pUnitType );
	// naète ukazatel na jednotku na civilizaci
	storage >> (void *&)m_pZUnit;
	LOAD_ASSERT ( m_pZUnit != NULL );
	// naète poèet odkazù na jednotku
	storage >> m_dwReferenceCounter;
	LOAD_ASSERT ( m_dwReferenceCounter != DWORD_MAX );

	// naète poèet životù
	storage >> m_nLives;
	// naète maximální poèet životù
	storage >> m_nLivesMax;
	LOAD_ASSERT ( m_nLivesMax > 0 );

	// naète polomìr viditelnosti
	storage >> m_dwViewRadius;
	// naète x-ovou souøadnici pozice jednotky
	storage >> m_pointPosition.x;
	// naète y-ovou souøadnici pozice jednotky
	storage >> m_pointPosition.y;
	// naète vertikální pozici jednotky
	storage >> m_dwVerticalPosition;
	LOAD_ASSERT ( m_dwVerticalPosition != 0 );

	// délka jména jednotky
	int nUnitNameSize;
	// naète délku jména jednotky
	storage >> nUnitNameSize;
	// zjistí, je-li jeméno jednotky prázdné
	if ( nUnitNameSize > 0 )
	{	// jméno jednotky není prázdné
		// získá ukazatel na naèítané jméno jednotky
		char *szUnitName = m_strUnitName.GetBufferSetLength ( nUnitNameSize );
		// naète jméno jednotky
		storage.Read ( szUnitName, nUnitNameSize );
		// ukonèí naèítání jména jednotky
		m_strUnitName.ReleaseBuffer ( nUnitNameSize );
	}

	// naète nasmìrování jednotky
	storage >> m_nDirection;
	LOAD_ASSERT ( m_nDirection <= Dir_Random );
	// naète mód jednotky
	storage >> m_nMode;
	LOAD_ASSERT ( m_nMode < 8 );
	// naète pøíznaky neviditelnosti jednotky
	storage >> m_dwInvisibilityState;
	// naète pøíznaky detekce neviditelnosti ostatních jednotek
	storage >> m_dwInvisibilityDetection;
	// naète ID vzhledu jednotky
	storage >> m_dwAppearanceID;
	LOAD_ASSERT ( m_dwAppearanceID != 0 );

	// pozice a èas kontrolního bodu cesty jednotky
	DWORD dwCheckPointPositionX, dwCheckPointPositionY, dwCheckPointTime;
	// naète první kontrolní bod cesty jednotky
	storage >> dwCheckPointPositionX;
	storage >> dwCheckPointPositionY;
	storage >> dwCheckPointTime;
	m_cFirstCheckPoint.Set ( CPointDW ( dwCheckPointPositionX, dwCheckPointPositionY ), 
		dwCheckPointTime );
	// naète druhý kontrolní bod cesty jednotky
	storage >> dwCheckPointPositionX;
	storage >> dwCheckPointPositionY;
	storage >> dwCheckPointTime;
	m_cSecondCheckPoint.Set ( CPointDW ( dwCheckPointPositionX, dwCheckPointPositionY ), 
		dwCheckPointTime );

	// naète pøíznaky zmìn dat jednotky
	storage >> m_dwModificationFlags;

	// vytvoøí data skill
	m_pSkillData = new BYTE[m_pUnitType->GetSkillDataSize ()];

	// zjistí poèet skill jednotky
	DWORD dwSkillCount = m_pUnitType->GetSkillCount ();
	ASSERT ( dwSkillCount > 0 );
	// vytvoøí bitové pole pøíznakù active/deactive skill jednotky na 0
	m_cActiveSkills.Create ( dwSkillCount );
	// naète pole pøíznakù aktivních skill
	storage.Read ( (void *)m_cActiveSkills.GetData (), ( dwSkillCount + 7 ) / 8 );

	// inicializuje skilly jednotky
	m_pUnitType->InitializeSkills ( this );

	// projede všechny skilly jednotky
	for ( ; dwSkillCount-- > 0; )
	{
		// naète skillu jednotky
		m_pUnitType->GetSkillTypeByIndex ( dwSkillCount )->PersistentLoadSkill ( storage, this );
	}

	// vytvoøí pole posledních pozic, kde byla jednotka civilizacemi vidìna
	m_aCivilizationLastSeenPosition = 
		(CSPosition *)m_cCivilizationLastSeenPositionArrayMemoryPool.Allocate ();
	// naète pole naposledy vidìných pozic jednotky civilizacemi
	for ( DWORD dwCivilizationIndex = dwCivilizationCount; dwCivilizationIndex-- > 0; )
	{
		// naète naposledy vidìnou pozici jednotky civilizací "dwCivilizationIndex"
		storage >> m_aCivilizationLastSeenPosition[dwCivilizationIndex].x;
		storage >> m_aCivilizationLastSeenPosition[dwCivilizationIndex].y;
		storage >> m_aCivilizationLastSeenPosition[dwCivilizationIndex].z;
	}

	// poèet jednotek v seznamu vidìných jednotek
	DWORD dwSeenUnitCount;
	// naète poèet vidìných jednotek
	storage >> dwSeenUnitCount;

	// naète seznam vidìných jednotek
	while ( dwSeenUnitCount-- > 0 )
	{
		// vidìná jednotka
		CSUnit *pSeenUnit;
		// naète ukazatel na vidìnou jednotku
		storage >> (void *&)pSeenUnit;
		LOAD_ASSERT ( pSeenUnit != NULL );
		LOAD_ASSERT ( m_cSeenUnitList.Find ( pSeenUnit ) == NULL );
		// pøidá vidìnou jednotku do seznamu vidìných jednotek
		m_cSeenUnitList.Add ( pSeenUnit );
	}
}

// pøeklad ukazatelù
void CSUnit::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	// pøeloží ukazatel na jednotku na civilizaci
	m_pZUnit = (CZUnit *)storage.TranslatePointer ( m_pZUnit );
	LOAD_ASSERT ( m_pZUnit != NULL );

	// projede všechny skilly jednotky
	for ( DWORD dwSkillIndex = m_pUnitType->GetSkillCount (); dwSkillIndex-- > 0; )
	{
		// pøeloží ukazatele skilly jednotky
		m_pUnitType->GetSkillTypeByIndex ( dwSkillIndex )->PersistentTranslatePointersSkill ( 
			storage, this );
	}

	// zjistí pozici první jednotky v seznamu vidìných jednotek
	POSITION posSeenUnit = m_cSeenUnitList.GetHeadPosition ();
	// vidìná jednotka v seznamu vidìných jednotek
	CSUnit **pSeenUnit;

	// projede jednotky v seznamu vidìných jednotek
	while ( CSUnitList::GetNext ( posSeenUnit, pSeenUnit ) )
	{
		// pøeloží ukazatel na vidìnou jednotku
		*pSeenUnit = (CSUnit *)storage.TranslatePointer ( *pSeenUnit );
	}
}

// inicializace nahraného objektu
void CSUnit::PersistentInit ( BOOL bDeadDeletedUnit ) 
{
	// zkontroluje kontrolní body cesty jednotky a pozici jednotky
	if ( g_cMap.CheckLoadedMapPosition ( m_pointPosition ) )
	{	// jedná se o pozici na mapì
		// zkontroluje první kontrolní bod cesty jednotky
		LOAD_ASSERT ( g_cMap.CheckLoadedMapPosition ( m_cFirstCheckPoint.GetPosition () ) );
		// zkontroluje druhý kontrolní bod cesty jednotky
		LOAD_ASSERT ( g_cMap.CheckLoadedMapPosition ( m_cSecondCheckPoint.GetPosition () ) );
		// zkontroluje èasové souøadnice kontrolních bodù cesty jednotky
		LOAD_ASSERT ( m_cFirstCheckPoint.GetTime () <= m_cSecondCheckPoint.GetTime () );
		// zjistí, jedná-li se o zpìtné kontrolní body cesty jednotky
		if ( m_cFirstCheckPoint.GetTime () < g_cMap.GetTimeSlice () )
		{	// jedná se o zpìtný kontrolní bod cesty jednotky
			// *******************************
			//LOAD_ASSERT ( m_cFirstCheckPoint.GetPosition () == m_pointPosition );
			//LOAD_ASSERT ( m_cFirstCheckPoint.GetTime () == m_cSecondCheckPoint.GetTime () );
		}
	}
	else
	{	// jedná se o pozici mimo mapu
		// zkontroluje první kontrolní bod cesty jednotky
		LOAD_ASSERT ( !g_cMap.CheckLoadedMapPosition ( m_cFirstCheckPoint.GetPosition () ) );
		LOAD_ASSERT ( m_cFirstCheckPoint.GetTime () <= g_cMap.GetTimeSlice () );
		// zkontroluje druhý kontrolní bod cesty jednotky
		LOAD_ASSERT ( !g_cMap.CheckLoadedMapPosition ( m_cSecondCheckPoint.GetPosition () ) );
		LOAD_ASSERT ( m_cSecondCheckPoint.GetTime () <= g_cMap.GetTimeSlice () );
	}

	// zkontroluje pole naposledy vidìných pozic jednotky civilizacemi
	for ( DWORD dwCivilizationIndex = g_cMap.GetCivilizationCount (); 
		dwCivilizationIndex-- > 0; )
	{
		// zkontroluje naposledy vidìnou pozici jednotky civilizací "dwCivilizationIndex"
		if ( g_cMap.CheckLoadedMapPosition ( 
			m_aCivilizationLastSeenPosition[dwCivilizationIndex] ) )
		{	// jedná se o pozici na mapì
			LOAD_ASSERT ( m_aCivilizationLastSeenPosition[dwCivilizationIndex].z != 0 );
		}
		else
		{	// jedná se o pozici mimo mapu
			LOAD_ASSERT ( m_aCivilizationLastSeenPosition[dwCivilizationIndex].z == 0 );
		}
	}

	// inicializuje index civilizace jednotky
	m_dwCivilizationIndex = m_pUnitType->GetCivilization ()->GetCivilizationIndex ();

	// vytvoøí pole poètu jednotek jednotlivých civilizací, které jednotku vidí
	m_aCivilizationVisibility = (WORD *)m_cCivilizationVisibilityArrayMemoryPool.Allocate ();

	// inicializuje pole poètu jednotek jednotlivých civilizací, které jednotku vidí
	memset ( m_aCivilizationVisibility, 0, g_cMap.GetCivilizationCount () * 
		sizeof ( WORD ) );
	m_aCivilizationVisibility[m_dwCivilizationIndex] = 1;
	// inicializuje poèet klientù civilizací sledujících jednotku
	m_dwWatchingCivilizationClientCount = 0;

	// inicializuje polomìr viditelnosti jednotky
	SetViewRadius ( m_dwViewRadius );

	// projede všechny skilly jednotky
	for ( DWORD dwSkillIndex = m_pUnitType->GetSkillCount (); dwSkillIndex-- > 0; )
	{
		// inicializuje skilly jednotky
		m_pUnitType->GetSkillTypeByIndex ( dwSkillIndex )->PersistentInitSkill ( this );
	}

	// zjistí, jedná-li se o mrtvou nebo smazanou jednotku
	if ( !bDeadDeletedUnit )
	{	// nejedná se o mrtvou ani smazanou jednotku
		// pøidá jednotku do seznamu jednotek typu jednotky
		m_pUnitType->AddUnit ( this );
	}
}

// inicializace seznamu vidìných jednotek
void CSUnit::PersistentInitSeenUnitList () 
{
	// zjistí pozici první jednotky v seznamu vidìných jednotek
	POSITION posSeenUnit = m_cSeenUnitList.GetHeadPosition ();
	// vidìná jednotka
	CSUnit *pSeenUnit;

	// projede jednotky v seznamu vidìných jednotek
	while ( CSUnitList::GetNext ( posSeenUnit, pSeenUnit ) )
	{
		// zkontroluje civilizaci vidìné jednotky
		LOAD_ASSERT ( pSeenUnit->m_dwCivilizationIndex != m_dwCivilizationIndex );

		// zvýší poèet jednotek své civilizace, které vidìnou jednotku vidí
		pSeenUnit->m_aCivilizationVisibility[m_dwCivilizationIndex]++;
	}
}

//////////////////////////////////////////////////////////////////////
// Odkazy na instanci jednotky
//////////////////////////////////////////////////////////////////////

// zaznamená uvolnìní odkazu na jednotku (ukazatel na jednotku mùže být dále neplatný)
void CSUnit::Release () 
{
	// sníží poèet odkazù na jednotku
	DWORD dwReferenceCounter = ::InterlockedDecrement ( (long *)&m_dwReferenceCounter );
	ASSERT ( dwReferenceCounter != DWORD_MAX );

	// zjistí, jedná-li se o poslední odkaz na jednotku
	if ( dwReferenceCounter == 0 )
	{	// jedná se o poslední odkaz na jednotku
		ASSERT ( m_pZUnit != NULL );

		// zjistí, je-li jednotka mrtvá
		if ( m_nLives <= 0 )
		{	// jednotka je mrtvá
			// pokusí se znièit jednotku na civilizaci
			if ( m_pZUnit->TryDelete () )
			{	// podaøilo se znièit jednotku na civilizaci
				// zamkne seznam smazaných jednotek
				VERIFY ( g_cMap.m_mutexDeletedUnitListLock.Lock () );

				// najde jednotku v seznamu smazaných jednotek
				POSITION posUnit = g_cMap.m_cDeletedUnitList.Find ( this );
				ASSERT ( posUnit != NULL );
				// smaže jednotku ze seznamu smazaných jednotek
				VERIFY ( g_cMap.m_cDeletedUnitList.RemovePosition ( posUnit ) == this );
				ASSERT ( g_cMap.m_cDeletedUnitList.Find ( this ) == NULL );

				// odemkne seznam smazaných jednotek
				VERIFY ( g_cMap.m_mutexDeletedUnitListLock.Unlock () );

				// znièí jednotku
				Delete ();
				delete this;
			}
			// nepodaøilo se znièit jednotku na civilizaci
		}
		// jednotka není mrtvá
	}
	// na jednotku existují odkazy
}

//////////////////////////////////////////////////////////////////////
// Info struktury jednotky
//////////////////////////////////////////////////////////////////////

// pošle full info jednotky klientovi "pClient" (jednotka musí být zamèena pro ètení)
void CSUnit::SendFullInfo ( struct SUnitFullInfo *pFullInfo, 
	CZClientUnitInfoSender *pClient ) 
{
	// vyplní brief info
	GetBriefInfo ( pFullInfo );

	// vyplní další specifické informace full infa
	pFullInfo->dwInvisibilityState = m_dwInvisibilityState;
	pFullInfo->dwInvisibilityDetection = m_dwInvisibilityDetection;

	// pošle zaèátek full infa jednotky
	pClient->StartSendingFullInfo ( pFullInfo );

	// buffer full infa skilly
	CString strFullInfoBuffer;

	// pošle ID jednotky
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

    // pošle jméno jednotky
    strFullInfoBuffer.Format ( "%s\n", GetZUnit()->GetName());
	  pClient->SendingFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		  strFullInfoBuffer.GetLength () );

    // pošle jméno skriptu
    CString strScriptType = (CString)*GetZUnit()->m_pIUnitType->m_stiName;
    strFullInfoBuffer.Format ( "Assigned script:\n%s\n", strScriptType);
	  pClient->SendingFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		  strFullInfoBuffer.GetLength () );

    // pošle stav
    strFullInfoBuffer.Format ( "%s\n\n", GetZUnit()->GetStatus() );
	  pClient->SendingFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		  strFullInfoBuffer.GetLength () );
  
    // odemknout jednotku
    GetZUnit()->GetCivilization()->UnlockUnitData();
  }

	// pošle stav životù jednotky
	strFullInfoBuffer.Format ( "Lives: %d%% (%d/%d)", (int)( ( (double)(m_nLives * 100) / 
		(double)m_nLivesMax ) + (double)0.5 ), m_nLives, m_nLivesMax );
	pClient->SendingFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		strFullInfoBuffer.GetLength () );

	// zjistí poèet skill jednotky
	DWORD dwSkillCount = m_pUnitType->GetSkillCount ();
	// projede všechny skilly jednotky
	for ( ; dwSkillCount-- > 0; )
	{
		// vyprázdnit buffer
		strFullInfoBuffer.Empty ();
		// vyplní full info skilly
		m_pUnitType->GetSkillTypeByIndex ( dwSkillCount )->FillFullInfo ( this, 
			strFullInfoBuffer );

		// zjistí, je-li full info skilly prázdné
		if ( !strFullInfoBuffer.IsEmpty () )
		{	// full info skilly není prázdné
			// zformátuje full info skilly do øádky
			CString strFullInfoLine ( "\n" );
			strFullInfoLine += strFullInfoBuffer;
			// pošle full info skilly klientovi
			pClient->SendingFullInfo ( (const BYTE *)(LPCSTR)strFullInfoLine, 
				strFullInfoLine.GetLength () );
		}
	}

	// pošle ukonèení full infa jednotky
	pClient->StopSendingFullInfo ();
}

// pošle enemy full info jednotky klientovi "pClient" (jednotka musí být zamèena pro ètení)
void CSUnit::SendEnemyFullInfo ( struct SUnitEnemyFullInfo *pEnemyFullInfo, 
	CZClientUnitInfoSender *pClient ) 
{
	// vyplní brief info
	GetBriefInfo ( pEnemyFullInfo );

	// vyplní další specifické informace enemy full infa
	pEnemyFullInfo->dwInvisibilityState = m_dwInvisibilityState;

	// pošle zaèátek enemy full infa jednotky
	pClient->StartSendingEnemyFullInfo ( pEnemyFullInfo );

	// buffer enemy full infa skilly
	CString strEnemyFullInfoBuffer;

	// pošle ID jednotky
#ifdef _DEBUG
	strEnemyFullInfoBuffer.Format ( "Unit ID=%d, server ID=%d\n", m_pZUnit->GetID (), m_dwID );
#else //!_DEBUG
	strEnemyFullInfoBuffer.Format ( "Unit ID=%d\n", m_pZUnit->GetID () );
#endif //!_DEBUG
	pClient->SendingEnemyFullInfo ( (const BYTE *)(LPCSTR)strEnemyFullInfoBuffer, 
		strEnemyFullInfoBuffer.GetLength () );

	// pošle stav životù jednotky
	strEnemyFullInfoBuffer.Format ( "Lives: %d%% (%d/%d)", (int)( ( (double)(m_nLives * 100) / 
		(double)m_nLivesMax ) + (double)0.5 ), m_nLives, m_nLivesMax );
	pClient->SendingEnemyFullInfo ( (const BYTE *)(LPCSTR)strEnemyFullInfoBuffer, 
		strEnemyFullInfoBuffer.GetLength () );

	// zjistí poèet skill jednotky
	DWORD dwSkillCount = m_pUnitType->GetSkillCount ();
	// projede všechny skilly jednotky
	for ( ; dwSkillCount-- > 0; )
	{
		// vyprázdnit buffer
		strEnemyFullInfoBuffer.Empty ();
		// vyplní enemy full info skilly
		m_pUnitType->GetSkillTypeByIndex ( dwSkillCount )->FillEnemyFullInfo ( this, 
			strEnemyFullInfoBuffer );

		// zjistí, je-li enemy full info skilly prázdné
		if ( !strEnemyFullInfoBuffer.IsEmpty () )
		{	// enemy full info skilly není prázdné
			// zformátuje enemy full info skilly do øádky
			CString strEnemyFullInfoLine ( "\n" );
			strEnemyFullInfoLine += strEnemyFullInfoBuffer;
			// pošle enemy full info skilly klientovi
			pClient->SendingEnemyFullInfo ( (const BYTE *)(LPCSTR)strEnemyFullInfoLine, 
				strEnemyFullInfoLine.GetLength () );
		}
	}

	// pošle ukonèení enemy full infa jednotky
	pClient->StopSendingEnemyFullInfo ();
}

// pošle start full info jednotky klientovi "pClient" (jednotka musí být zamèena pro ètení)
void CSUnit::SendStartFullInfo ( struct SUnitStartFullInfo *pStartFullInfo, 
	CZClientUnitInfoSender *pClient ) 
{
	// vyplní start info
	GetStartInfo ( pStartFullInfo );

	// vyplní další specifické informace start full infa
	pStartFullInfo->dwInvisibilityState = m_dwInvisibilityState;
	pStartFullInfo->dwInvisibilityDetection = m_dwInvisibilityDetection;

	// pošle zaèátek start full infa jednotky
	pClient->StartSendingStartFullInfo ( pStartFullInfo );

	// buffer full infa skilly
	CString strFullInfoBuffer;

	// pošle ID jednotky
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

    // pošle jméno jednotky
    strFullInfoBuffer.Format ( "%s\n", GetZUnit()->GetName() );
	pClient->SendingStartFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		  strFullInfoBuffer.GetLength () );

    // pošle jméno skriptu
    strFullInfoBuffer.Format ( "Assigned script:\n%s\n", (CString)*GetZUnit()->m_pIUnitType->m_stiName );
	  pClient->SendingStartFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		  strFullInfoBuffer.GetLength () );

    // pošle stav
    strFullInfoBuffer.Format ( "%s\n\n", GetZUnit()->GetStatus() );
	  pClient->SendingStartFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		  strFullInfoBuffer.GetLength () );

    // odemknout jednotku
    GetZUnit()->GetCivilization()->UnlockUnitData();
  }

  // pošle stav životù jednotky
	strFullInfoBuffer.Format ( "Lives: %d%% (%d/%d)", (int)( ( (double)(m_nLives * 100) / 
		(double)m_nLivesMax ) + (double)0.5 ), m_nLives, m_nLivesMax );
	pClient->SendingStartFullInfo ( (const BYTE *)(LPCSTR)strFullInfoBuffer, 
		strFullInfoBuffer.GetLength () );

	// zjistí poèet skill jednotky
	DWORD dwSkillCount = m_pUnitType->GetSkillCount ();
	// projede všechny skilly jednotky
	for ( ; dwSkillCount-- > 0; )
	{
		// vyprázdnit buffer
		strFullInfoBuffer.Empty ();
		// vyplní full info skilly
		m_pUnitType->GetSkillTypeByIndex ( dwSkillCount )->FillFullInfo ( this, 
			strFullInfoBuffer );

		// zjistí, je-li full info skilly prázdné
		if ( !strFullInfoBuffer.IsEmpty () )
		{	// full info skilly není prázdné
			// zformátuje full info skilly do øádky
			CString strFullInfoLine ( "\n" );
			strFullInfoLine += strFullInfoBuffer;
			// pošle full info skilly klientovi
			pClient->SendingStartFullInfo ( (const BYTE *)(LPCSTR)strFullInfoLine, 
				strFullInfoLine.GetLength () );
		}
	}

	// pošle ukonèení start full infa jednotky
	pClient->StopSendingStartFullInfo ();
}

// pošle start enemy full info jednotky klientovi "pClient" (jednotka musí být zamèena 
//		pro ètení)
void CSUnit::SendStartEnemyFullInfo ( 
	struct SUnitStartEnemyFullInfo *pStartEnemyFullInfo, CZClientUnitInfoSender *pClient ) 
{
	// vyplní brief info
	GetStartInfo ( pStartEnemyFullInfo );

	// vyplní další specifické informace start enemy full infa
	pStartEnemyFullInfo->dwInvisibilityState = m_dwInvisibilityState;

	// pošle zaèátek start enemy full infa jednotky
	pClient->StartSendingStartEnemyFullInfo ( pStartEnemyFullInfo );

	// buffer enemy full infa skilly
	CString strEnemyFullInfoBuffer;

	// pošle ID jednotky
#ifdef _DEBUG
	strEnemyFullInfoBuffer.Format ( "Unit ID=%d, server ID=%d\n", m_pZUnit->GetID (), m_dwID );
#else //!_DEBUG
	strEnemyFullInfoBuffer.Format ( "Unit ID=%d\n", m_pZUnit->GetID () );
#endif //!_DEBUG
	pClient->SendingStartEnemyFullInfo ( (const BYTE *)(LPCSTR)strEnemyFullInfoBuffer, 
		strEnemyFullInfoBuffer.GetLength () );

	// pošle stav životù jednotky
	strEnemyFullInfoBuffer.Format ( "Lives: %d%% (%d/%d)", (int)( ( (double)(m_nLives * 100) / 
		(double)m_nLivesMax ) + (double)0.5 ), m_nLives, m_nLivesMax );
	pClient->SendingStartEnemyFullInfo ( (const BYTE *)(LPCSTR)strEnemyFullInfoBuffer, 
		strEnemyFullInfoBuffer.GetLength () );

	// zjistí poèet skill jednotky
	DWORD dwSkillCount = m_pUnitType->GetSkillCount ();
	// projede všechny skilly jednotky
	for ( ; dwSkillCount-- > 0; )
	{
		// vyprázdnit buffer
		strEnemyFullInfoBuffer.Empty ();
		// vyplní enemy full info skilly
		m_pUnitType->GetSkillTypeByIndex ( dwSkillCount )->FillEnemyFullInfo ( this, 
			strEnemyFullInfoBuffer );

		// zjistí, je-li enemy full info skilly prázdné
		if ( !strEnemyFullInfoBuffer.IsEmpty () )
		{	// enemy full info skilly není prázdné
			// zformátuje enemy full info skilly do øádky
			CString strEnemyFullInfoLine ( "\n" );
			strEnemyFullInfoLine += strEnemyFullInfoBuffer;
			// pošle enemy full info skilly klientovi
			pClient->SendingStartEnemyFullInfo ( (const BYTE *)(LPCSTR)strEnemyFullInfoLine, 
				strEnemyFullInfoLine.GetLength () );
		}
	}

	// pošle ukonèení start enemy full infa jednotky
	pClient->StopSendingStartEnemyFullInfo ();
}

//////////////////////////////////////////////////////////////////////
// Informace o typech skill jednotky
//////////////////////////////////////////////////////////////////////

// vrátí typ skilly s ID jména "pNameID" (upøednostòuje typ skilly povolený v aktuálním 
//		módu jednotky) (NULL=typ skilly neexistuje)
CSSkillType *CSUnit::GetSkillTypeByNameID ( CStringTableItem *pNameID ) 
{
	// zjistí ukazatel na první popis typu skilly
	CSUnitType::SSkillTypeDescription *pSkillTypeDescription = 
		m_pUnitType->m_aSkillTypeDescription;
	// poèet typù skill typu jednotky
	DWORD dwSkillTypeCount = m_pUnitType->GetSkillCount ();

	// maska módu jednotky
	BYTE nModeMask = 0x01 << m_nMode;

	// první nalezený typ skilly s ID jména "pNameID"
	CSSkillType *pFirstFoundSkillType = NULL;

	// projede všechny typy skill
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		ASSERT ( pSkillTypeDescription->pNameID != NULL );
		// zjistí, jedná-li se o hledaný typ skilly
		if ( pSkillTypeDescription->pNameID == pNameID )
		{	// jedná se o hledaný typ skilly
			ASSERT ( pSkillTypeDescription->pSkillType != NULL );
			// zjistí, jedná-li se o povolenou skillu
			if ( pSkillTypeDescription->bEnabled && 
				( pSkillTypeDescription->nAllowedModesMask & nModeMask ) )
			{	// jedná se o povolenou skillu
				// vrátí typ skilly
				return pSkillTypeDescription->pSkillType;
			}

			// zjistí, jedná-li se o první nalezený typ skilly
			if ( pFirstFoundSkillType == NULL )
			{	// jedná se o první nalezený typ skilly
				// aktualizuje ukazatel na první nalezený typ skilly
				pFirstFoundSkillType = pSkillTypeDescription->pSkillType;
			}
		}
	}

	// vrátí první nalezený typ skilly
	return pFirstFoundSkillType;
}

//////////////////////////////////////////////////////////////////////
// Get/Set metody herních informací o jednotce
//////////////////////////////////////////////////////////////////////

// nastaví mód jednotky (0-7) (jednotka musí být zamèena pro zápis, volá 
//		SetBriefInfoModified, je-li potøeba)
void CSUnit::SetMode ( BYTE nMode ) 
{
	// zkontroluje èíslo módu jednotky (0-7)
	ASSERT ( nMode < 8 );
	ASSERT ( m_nMode < 8 );

	// zjistí, jedná-li se o zmìnu módu
	if ( m_nMode == nMode )
	{	// nejedná se o zmìnu módu
		return;
	}
	// jedná se o zmìnu módu

	// maska módu jednotky
	BYTE nModeMask = 0x01 << nMode;
	// uschová starý mód jednotky
	BYTE nOldMode = m_nMode;

// deaktivuje aktivní skilly jednotky, které jsou zakázané v novém módu jednotky

	// ukazatel na první popis typu skilly
	CSUnitType::SSkillTypeDescription *pSkillTypeDescription = 
		m_pUnitType->m_aSkillTypeDescription;
	// poèet typù skill typu jednotky
	DWORD dwSkillTypeCount = m_pUnitType->GetSkillCount ();

	// projede všechny typy skill
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		// zjistí, je-li skilla aktivní a zakázaná v novém módu jednotky
		if ( IsSkillActive ( dwSkillTypeIndex ) && 
			!( pSkillTypeDescription->nAllowedModesMask & nModeMask ) )
		{	// skilla je aktivní a je zakázaná v novém módu jednotky
			ASSERT ( pSkillTypeDescription->bEnabled && 
				( pSkillTypeDescription->nAllowedModesMask & ( 0x01 << nOldMode ) ) );
			ASSERT ( pSkillTypeDescription->pSkillType != NULL );
			// deaktivuje skillu jednotky
			pSkillTypeDescription->pSkillType->DeactivateSkill ( this );
		}
	}

// zmìní mód jednotky

	// nastaví nový mód jednotky
	m_nMode = nMode;

	// aktualizuje informace o jednotce
	m_dwInvisibilityState = m_pUnitType->m_aInvisibilityState[nMode];
	m_dwInvisibilityDetection = m_pUnitType->m_aInvisibilityDetection[nMode];
	m_dwAppearanceID = m_pUnitType->m_aAppearanceID[nMode];

// informuje povolené skilly jednotky o zmìnì módu

	// ukazatel na první popis typu skilly
	pSkillTypeDescription = m_pUnitType->m_aSkillTypeDescription;
	// poèet typù skill typu jednotky
	dwSkillTypeCount = m_pUnitType->GetSkillCount ();

	// projede typy skill
	for ( dwSkillTypeIndex = 0; dwSkillTypeIndex++ < dwSkillTypeCount; 
		pSkillTypeDescription++ )
	{
		// zjistí, je-li skilla povolená v módu jednotky
		if ( pSkillTypeDescription->bEnabled && 
			( pSkillTypeDescription->nAllowedModesMask & nModeMask ) )
		{	// skilla je povolená v módu jednotky
			ASSERT ( pSkillTypeDescription->pSkillType != NULL );
			// informuje skillu o zmìnì módu jednotky
			pSkillTypeDescription->pSkillType->ModeChanged ( this, nOldMode );
		}
	}

	// nastaví pøíznak zmìny brief infa
	SetBriefInfoModified ();
}

// nastaví masku módu jednotky (musí být nastaven právì jeden bit) (jednotka musí být 
//		zamèena pro zápis, volá SetBriefInfoModified, je-li potøeba)
void CSUnit::SetModeMask ( BYTE nModeMask ) 
{
	// zkontroluje je-li nastaven právì jeden bit
	ASSERT ( ( nModeMask & 0xff ) && !( nModeMask & ( nModeMask - (BYTE)1 ) ) );

	// mód jednotky
	BYTE nMode;

	// zjistí, o jaký mód jednotky se jedná
	if ( nModeMask & 0x0f )
	{	// nastavený bit je 0-3
		if ( nModeMask & 0x03 )
		{	// nastavený bit je 0-1
			nMode = ( nModeMask & 0x01 ) ? 0 : 1;
		}
		else
		{	// nastavený bit je 2-3
			nMode = ( nModeMask & 0x04 ) ? 2 : 3;
		}
	}
	else
	{	// nastavený bit je 4-7
		if ( nModeMask & 0x30 )
		{	// nastavený bit je 4-5
			nMode = ( nModeMask & 0x10 ) ? 4 : 5;
		}
		else
		{	// nastavený bit je 6-7
			nMode = ( nModeMask & 0x40 ) ? 6 : 7;
		}
	}

	// nechá nastavit mód jednotky
	SetMode ( nMode );
}

//////////////////////////////////////////////////////////////////////
// Debuggovací informace
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// zkontroluje správná data objektu (TRUE=OK)
BOOL CSUnit::CheckValidData () 
{
	// zkontroluje kontrolní body cesty jednotky a pozici jednotky
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

	// nechá zkontrolovat naposledy vidìné pozice jednotky civilizacemi
	for ( DWORD dwCivilizationIndex = g_cMap.GetCivilizationCount (); 
		dwCivilizationIndex-- > 0; )
	{
		// nechá zkontrolovat naposledy vidìnou pozici jednotky civilizace
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

// zkontroluje neškodná data objektu (TRUE=OK)
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
