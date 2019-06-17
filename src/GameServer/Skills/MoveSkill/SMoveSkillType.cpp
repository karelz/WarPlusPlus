/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry - skilly
 *   Autor: Vít Karas & Karel Zikmund
 * 
 *   Popis: Typ pohybové skilly (move skilla)
 * 
 ***********************************************************/

#include "StdAfx.h"

#include <math.h>

#include "SMoveSkillType.h"
#include "SMoveSkill.h"
#include "Common\Map\MMoveSkillType.h"

#include "GameServer\GameServer\SPath.h"
#include "ISysCallMove.h"
#include "TripleS\Interpret\Src\SysObjs\ISOPosition.h"

#include <stdlib.h>

#include "SMoveSkillType.inl"

#define SKILLDATA(pUnit) ((SSMoveSkill *)GetSkillData ( pUnit ))

// minimální a maximální poèet TimeSlicù èekání, nedaøi-li se pohnout s jednotkou
#define WAITING_TIMEOUT_MIN	10
#define WAITING_TIMEOUT_MAX	30
#define WAITING_TIMEOUT			( WAITING_TIMEOUT_MIN + rand () % ( WAITING_TIMEOUT_MAX - WAITING_TIMEOUT_MIN + 1 ) )
// poèet opakování èekání, nedaøí-li se pohnout s jednotkou
#define WAITING_TRIES	10
// škálování opakování èekání, nedaøí-li se pohnout s jednotkou
#define WAITING_SCALE	( DWORD_MAX / WAITING_TRIES )

// maximální rychlost
#define SPEED_MAX							1000
// maximální tolerance vzdálenosti od cílové pozice metody GoCloseTo
#define MAX_GO_CLOSE_TO_TOLERANCE	1000

// implementace interfacu volání ze skriptù
BEGIN_SKILLCALL_INTERFACE_MAP ( CSMoveSkillType, CSSkillType )
	SKILLCALL_BASECLASS,
	SKILLCALL_BASECLASS,
	SKILLCALL_METHOD ( SCI_Go ),
	SKILLCALL_METHOD ( SCI_GetSpeed ),
	SKILLCALL_METHOD ( SCI_GoPos ),
	SKILLCALL_METHOD ( SCI_GoCloseTo ),
	SKILLCALL_METHOD ( SCI_GoCloseToPos ),
END_SKILLCALL_INTERFACE_MAP ()

// implementace typu skilly
IMPLEMENT_SKILL_TYPE ( CSMoveSkillType, 0, "Move" );

/////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
/////////////////////////////////////////////////////////////////////

// konstruktor
CSMoveSkillType::CSMoveSkillType () 
{
	m_dwSpeed = DWORD_MAX;
}

// destruktor
CSMoveSkillType::~CSMoveSkillType () 
{
	ASSERT ( m_dwSpeed == DWORD_MAX );
}

/////////////////////////////////////////////////////////////////////
// Inicializace a znièení dat objektu
/////////////////////////////////////////////////////////////////////

// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
//		výjimky: CPersistentLoadException
void CSMoveSkillType::CreateSkillType ( void *pData, DWORD dwDataSize ) 
{
	LOAD_ASSERT ( dwDataSize == sizeof ( SMMoveSkillType ) );

	ASSERT ( m_dwSpeed == DWORD_MAX );

	m_dwSpeed = ((SMMoveSkillType *)pData)->m_dwSpeed;
	m_dwAppearanceID = ((SMMoveSkillType *)pData)->m_dwMoveAppearanceID;

	LOAD_ASSERT ( m_dwSpeed <= SPEED_MAX );
}

// znièí data typu skilly
void CSMoveSkillType::DeleteSkillType () 
{
	ASSERT ( m_dwSpeed != DWORD_MAX );

	m_dwSpeed = DWORD_MAX;
}

/////////////////////////////////////////////////////////////////////
// Ukládání dat potomkù (analogie CPersistentObject metod)
/////////////////////////////////////////////////////////////////////

/*
CSMoveSkillType stored

	DWORD m_dwSpeed
	DWORD m_dwAppearanceID
*/

// ukládání dat potomka
void CSMoveSkillType::PersistentSaveSkillType ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	ASSERT ( m_dwSpeed <= SPEED_MAX );

	storage << m_dwSpeed;
	storage << m_dwAppearanceID;
}

// nahrávání pouze uložených dat potomka
void CSMoveSkillType::PersistentLoadSkillType ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	storage >> m_dwSpeed;
	storage >> m_dwAppearanceID;

	LOAD_ASSERT ( m_dwSpeed <= SPEED_MAX );
}

// pøeklad ukazatelù potomka
void CSMoveSkillType::PersistentTranslatePointersSkillType ( CPersistentStorage &storage ) 
{
}

// inicializace nahraného objektu potomka
void CSMoveSkillType::PersistentInitSkillType () 
{
}

/////////////////////////////////////////////////////////////////////
// Get/Set metody serverových informací o typu skilly jednotky
/////////////////////////////////////////////////////////////////////

// vrátí velikost dat skilly pro jednotku
DWORD CSMoveSkillType::GetSkillDataSize () 
{
	return sizeof ( SSMoveSkill );
}

// vrátí masku skupin skill
DWORD CSMoveSkillType::GetGroupMask () 
{
	return Group_Move;
}

/////////////////////////////////////////////////////////////////////
// Metody skilly
/////////////////////////////////////////////////////////////////////

// inicializuje skillu jednotky "pUnit"
void CSMoveSkillType::InitializeSkill ( CSUnit *pUnit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	ASSERT ( m_dwSpeed <= SPEED_MAX );

	pData->m_dwSpeed = m_dwSpeed;

	// znehodnotí data skilly
	pData->m_pPath = NULL;
	pData->m_pSysCall = NULL;

	pData->m_dbXStep = 0;
	pData->m_dbYStep = 0;

	pData->m_dbXPosition = DWORD_MAX;
	pData->m_dbYPosition = DWORD_MAX;

	pData->m_dwWaitCount = WAITING_SCALE - 1;

	pData->m_pointDestination = CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION );
	pData->m_dwToleranceSquareLimit = 0;

#ifdef _DEBUG
	// inicializuje výchozí bod
	pData->m_pointStart = CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION );
#endif //_DEBUG
}

// znièí skillu jednotky "pUnit"
void CSMoveSkillType::DestructSkill ( CSUnit *pUnit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	ASSERT ( pData->m_dwSpeed <= SPEED_MAX );

	// zjistí, je-li cesta platná
	if ( pData->m_pPath != NULL )
	{	// cesta je platná
		// znièí cestu
		pData->m_pPath->Destroy ( pUnit->GetCivilizationIndex () );
		pData->m_pPath = NULL;
	}

	// zjistí, je-li asociováno systémové volání
	if ( pData->m_pSysCall != NULL )
	{	// je asociování systémové volání
		// ukonèí èekající systémové volání
		pData->m_pSysCall->FinishWaiting ( STATUS_SKILL_COLLISION );
		pData->m_pSysCall->Release ();
		pData->m_pSysCall = NULL;
	}
}

// aktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
void CSMoveSkillType::ActivateSkill ( CSUnit *pUnit, CSPath *pPath, 
	CISysCallMove *pSysCall, CPointDW pointDestination, DWORD dwToleranceSquareLimit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	// deaktivuje nesluèitelné skilly
	GetUnitType ()->DeactivateSkillGroup ( pUnit, Group_Move | Group_Attack );

	ASSERT ( pData->m_dwSpeed <= SPEED_MAX );
	ASSERT ( CheckEmptySkillData ( pUnit ) );

	// nastaví systémové volání
	pSysCall->AddRef ();
	pData->m_pSysCall = pSysCall;

	// nastaví cestu pohybu
	pData->m_pPath = pPath;

	// nastaví umístìní jednotky
	pData->m_dbXPosition = (double)pUnit->GetPosition ().x + (double)0.5;
	pData->m_dbYPosition = (double)pUnit->GetPosition ().y + (double)0.5;

	// nastaví dobu èekání
	pData->m_dwWaitCount = 0;

	// nastaví cílovou pozici
	pData->m_pointDestination = pointDestination;
	// nastaví limit ètverce tolerance vzdálenosti od cílové pozice
	pData->m_dwToleranceSquareLimit = dwToleranceSquareLimit;

#ifdef _DEBUG
	// nastaví výchozí pozici
	pData->m_pointStart = pUnit->GetPosition ();
#endif //_DEBUG

	// aktivuje skillu na jednotce
	CSSkillType::ActivateSkill ( pUnit );
}

// deaktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
void CSMoveSkillType::DeactivateSkill ( CSUnit *pUnit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	// znièí cestu
	ASSERT ( pData->m_pPath != NULL );
	pData->m_pPath->Destroy ( pUnit->GetCivilizationIndex () );
	pData->m_pPath = NULL;

	// zneškodní krok pohybu
	pData->m_dbXStep = 0;
	pData->m_dbYStep = 0;

	// zneškodní umístìní jednotky
	pData->m_dbXPosition = DWORD_MAX;
	pData->m_dbYPosition = DWORD_MAX;

	// zjistí, je-li asociováno systémové volání
	if ( pData->m_pSysCall != NULL )
	{	// je asociování systémové volání
		// ukonèí èekající systémové volání
		pData->m_pSysCall->FinishWaiting ( STATUS_SKILL_COLLISION );
		pData->m_pSysCall->Release ();
		pData->m_pSysCall = NULL;
	}

	// nastaví kontrolní body jednotky na stání na místì
	pUnit->SetFirstCheckPoint ( pUnit->GetPosition(), 0 );
	pUnit->SetSecondCheckPoint ( pUnit->GetPosition(), 0 );
	// nastaví pøíznak zmìny kontrolních bodù
	pUnit->SetCheckPointInfoModified ();

	// znehodnotí data skilly
	pData->m_dwWaitCount = WAITING_SCALE - 1;

	// zneškodní cílovou pozici
	pData->m_pointDestination = CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION );
	// zneškodní limit ètverce tolerance vzdálenosti od cílové pozice
	pData->m_dwToleranceSquareLimit = 0;

	// nastaví standardní vzhled jednotky
	pUnit->SetDefaultAppearanceID ();

	// deaktivuje skillu na jednotce
	CSSkillType::DeactivateSkill ( pUnit );
}

/*
MoveSkill stored

	DWORD m_dwSpeed
	double m_dbXPosition
	double m_dbYPosition
	double m_dbXStep
	double m_dbYStep
	DWORD m_dwWaitCount
	DWORD m_pointDestination.x
	DWORD m_pointDestination.y
	DWORD m_dwToleranceSquareLimit
	CSPath *m_pPath
	BOOL bSysCallMove
	if ( m_pPath != NULL )
		CSPath stored
	if ( bSysCallMove )
		CISysCallMove stored
*/

// ukládání dat skilly
void CSMoveSkillType::PersistentSaveSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	BRACE_BLOCK ( storage );

	SSMoveSkill *pData = SKILLDATA ( pUnit );

	ASSERT ( pData->m_dwSpeed <= SPEED_MAX );

	// uloží data skilly
	storage << pData->m_dwSpeed;
	storage << pData->m_dbXPosition;
	storage << pData->m_dbYPosition;
	storage << pData->m_dbXStep;
	storage << pData->m_dbYStep;
	storage << pData->m_dwWaitCount;
	storage << pData->m_pointDestination.x;
	storage << pData->m_pointDestination.y;
	storage << pData->m_dwToleranceSquareLimit;
	storage << pData->m_pPath;
	storage << (BOOL)( pData->m_pSysCall != NULL );

	// zjistí, je-li cesta platná
	if ( pData->m_pPath != NULL )
	{	// cesta je platná
		// uloží cestu
		pData->m_pPath->PersistentSave ( storage );
	}

	// zjistí, je-li systémové volání platné
	if ( pData->m_pSysCall != NULL )
	{	// systémové volání je platné
		// uloží systémové volání
		pData->m_pSysCall->PersistentSave ( storage );
	}
}

// nahrávání pouze uložených dat skilly
void CSMoveSkillType::PersistentLoadSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	BRACE_BLOCK ( storage );

	SSMoveSkill *pData = SKILLDATA ( pUnit );

	ASSERT ( CheckEmptySkillData ( pUnit ) );

	// naète data skilly
	storage >> pData->m_dwSpeed;
	LOAD_ASSERT ( pData->m_dwSpeed <= SPEED_MAX );
	storage >> pData->m_dbXPosition;
	storage >> pData->m_dbYPosition;
	storage >> pData->m_dbXStep;
	storage >> pData->m_dbYStep;
	storage >> pData->m_dwWaitCount;
	storage >> pData->m_pointDestination.x;
	storage >> pData->m_pointDestination.y;
	storage >> pData->m_dwToleranceSquareLimit;

	// naète starý ukazatel na cestu
	void *pOldPath;
	storage >> pOldPath;

	// naète pøíznak systémového volání
	BOOL bSysCallMove;
	storage >> bSysCallMove;
	LOAD_ASSERT ( ( bSysCallMove == TRUE ) || ( bSysCallMove == FALSE ) );

	// zjistí, je-li cesta platná
	if ( pOldPath != NULL )
	{	// cesta je platná
		// naète cestu
		pData->m_pPath = CSPath::PersistentLoadPath ( storage );
		// zaregistruje ukazatel na cestu
		storage.RegisterPointer ( pOldPath, pData->m_pPath );
	}
	else
	{	// cesta je neplatná
		// aktualizuje ukazatel na cestu
		pData->m_pPath = NULL;
	}

	// zjistí, je-li systémové volání platné
	if ( bSysCallMove )
	{	// systémové volání je platné
		// vytvoøí systémové volání
		pData->m_pSysCall = new CISysCallMove;
		// naète systémové volání
		pData->m_pSysCall->PersistentLoad ( storage );
	}
	else
	{	// systémové volání je neplatné
		// aktualizuje ukazatel na systémové volání
		pData->m_pSysCall = NULL;
	}
}

// pøeklad ukazatelù dat skilly
void CSMoveSkillType::PersistentTranslatePointersSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	// zjistí, je-li cesta platná
	if ( pData->m_pPath != NULL )
	{	// cesta je platná
		// pøeloží ukazatele cesty
		pData->m_pPath->PersistentTranslatePointers ( storage );
	}

	// zjistí, je-li systémové volání platné
	if ( pData->m_pSysCall != NULL )
	{	// systémové volání je platné
		// pøeloží ukazatele systémového volání
		pData->m_pSysCall->PersistentTranslatePointers ( storage );
	}
}

// inicializace nahraných dat skilly
void CSMoveSkillType::PersistentInitSkill ( CSUnit *pUnit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	// zjistí, je-li cesta platná
	if ( pData->m_pPath != NULL )
	{	// cesta je platná
		// inicializuje cestu
		pData->m_pPath->PersistentInit ();
	}

	// zjistí, je-li systémové volání platné
	if ( pData->m_pSysCall != NULL )
	{	// systémové volání je platné
		// inicializuje systémové volání
		pData->m_pSysCall->PersistentInit ();
	}

#ifdef _DEBUG
	// inicializuje výchozí bod
	pData->m_pointStart = CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION );
#endif //_DEBUG
}

// vyplní full info "strInfo" skilly jednotky "pUnit"
void CSMoveSkillType::FillFullInfo ( CSUnit *pUnit, CString &strInfo ) 
{
	// zjistí, je-li skilla povolená
	if ( IsEnabled ( pUnit ) )
	{	// skilla je povolená
		// vyplní full info jednotky
		strInfo.Format ( "Move: speed %u", SKILLDATA ( pUnit )->m_dwSpeed );
	}
	else
	{	// skilla není povolená
		// vyplní full info jednotky
		strInfo = "Move: (disabled)";
	}
}

// vyplní enemy full info "strInfo" skilly jednotky "pUnit"
void CSMoveSkillType::FillEnemyFullInfo ( CSUnit *pUnit, CString &strInfo ) 
{
	// vyplní enemy full info jednotky
	strInfo = "Move Skill";
}

// provede TimeSlice na skille jednotky "pUnit", jednotka "pUnit" je již zamèená pro 
//		zápis, metoda mùže zamykat libovolný poèet jiných jednotek pro zápis/ètení (vèetnì 
//		jednotek ze svého bloku)
void CSMoveSkillType::DoTimeSlice ( CSUnit *pUnit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	ASSERT ( pData->m_pPath != NULL );

	// zjistí, stav cesty
	switch ( pData->m_pPath->GetState () )
	{
	// cesta je v poøádku
	case CSPath::EPS_Valid :
		// pokraèuje v pøesunování jednotky po cestì
		break;
	// cesta byla nalezena (možná i prázdná)
	case CSPath::EPS_Found :
		// pøijme cestu
		pData->m_pPath->Confirm ();

		// vynuluje èítaè èekání
		pData->m_dwWaitCount = 0;

		// vyplní kontrolní body jednotky a zjistí, je-li cesta prázdná
		if ( !FillCheckPoints ( pUnit ) )
		{	// cesta je prázdná
			// ukonèí skillu
			Finish ( pUnit, EGR_OK );
			return;
		}
		// cesta dosud není prázdná

		// aktualizuje ID vzhledu jednotky
		pUnit->SetAppearanceID ( m_dwAppearanceID );

		// pøipraví se na pøesun k prvnímu kontrolnímu bodu
		GoToFirstCheckPoint ( pUnit );

		// pokraèuje v pøesunování jednotky po cestì
		break;
	// cesta se hledá
	case CSPath::EPS_Searching :
		// poèká na nalezení cesty
		return;
	// cesta neexistuje (jednotka mùže být zablokována jednotkami)
	case CSPath::EPS_NotExists :
		ASSERT ( ( pData->m_dwWaitCount % WAITING_SCALE ) == 0 );
		// zjistí, má-li se znovu èekat
		if ( ( pData->m_dwWaitCount / WAITING_SCALE == ( WAITING_TRIES - 1 ) ) || 
			( pData->m_dwWaitCount == 0 ) )
		{	// cesta vùbec neexistuje nebo se již nemá znovu èekat
			// ukonèí skillu
			Finish ( pUnit, EGR_PathNotExists );
			return;
		}
		// pøijme nenalezenou cestu
		pData->m_pPath->Confirm ();

		// zkusí ještì poèkat
		pData->m_dwWaitCount += WAITING_TIMEOUT;
		// pokraèuje v aktivním èekání na volné místo
		return;
	// cesta je zablokována (lokální cesta neexistuje)
	case CSPath::EPS_Blocked :
		ASSERT ( ( pData->m_dwWaitCount % WAITING_SCALE ) == 0 );
		//ASSERT ( ( pData->m_dwWaitCount / WAITING_SCALE ) != 0 );
		// ****************** ZDE hledání zablokované cesty
		// ukonèí skillu
		Finish ( pUnit, EGR_PathBlocked );
		return;
	// nemožný pøípad
	case CSPath::EPS_Uninitialized :
	default :
		ASSERT ( FALSE );
	}

	// zjistí, èeká-li se na volné místo
	if ( pData->m_dwWaitCount == 0 )
	{	// neèeká se na volné místo
	// ********************************
//		ASSERT ( g_cMap.GetTimeSlice () <= pUnit->GetFirstCheckPoint ().GetTime () );

		// spoèítá novou pozici jednotky
		double dbNewXPosition = pData->m_dbXPosition + pData->m_dbXStep;
		double dbNewYPosition = pData->m_dbYPosition + pData->m_dbYStep;

		// zjistí, není-li nová pozice až za prvním kontrolním bodem
		if ( ( pData->m_dbXStep * ( (double)pData->m_pPath->GetCheckPointPosition ()->x + 
			(double)0.5 - dbNewXPosition ) <= 0 ) && ( pData->m_dbYStep * 
			( (double)pData->m_pPath->GetCheckPointPosition ()->y + (double)0.5 - 
			dbNewYPosition ) <= 0 ) )
		{	// nová pozice je již za prvním kontrolním bodem
			// nastaví novou pozici jednotky na kontrolní bod
			dbNewXPosition = (double)pData->m_pPath->GetCheckPointPosition ()->x + 
				(double)0.5;
			dbNewYPosition = (double)pData->m_pPath->GetCheckPointPosition ()->y + 
				(double)0.5;

			// urèí novou pozici jednotky
			CPointDW pointPosition ( (DWORD)( pData->m_dbXPosition = dbNewXPosition ), 
				(DWORD)( pData->m_dbYPosition = dbNewYPosition ) );

			// zjistí, zda se jednotka má skuteènì pohnout
			if ( pUnit->GetPosition () != pointPosition )
			{	// jednotka se má skuteènì pohnout
				// pokusí se pohnout jednotkou
				if ( !g_cMap.MoveUnit ( pUnit, pointPosition ) )
				{	// jednotkou se nepodaøilo pohnout
					// nastaví umístìní jednotky
					pData->m_dbXPosition = (double)pUnit->GetPosition ().x + (double)0.5;
					pData->m_dbYPosition = (double)pUnit->GetPosition ().y + (double)0.5;

					// nechá najít lokální cestu
					if ( pData->m_pPath->FindLocalPath ( pUnit ) )
					{	// první kontrolní bod cesty byl pøeskoèen
						// vyplní kontrolní body jednotky a zjistí, je-li cesta prázdná
						if ( !FillCheckPoints ( pUnit ) )
						{	// cesta je prázdná
							// ukonèí skillu
							Finish ( pUnit, EGR_OK );
							return;
						}
						// cesta dosud není prázdná

						// pøipraví se na pøesun k prvnímu kontrolnímu bodu
						GoToFirstCheckPoint ( pUnit );

						// pokraèuje v pohybu ke kontrolnímu bodu
						return;
					}
					// byl zadán požadavek na hledání lokální cesty

					// nastaví kontrolní body na stání
					pUnit->SetFirstCheckPoint ( pUnit->GetPosition (), 0 );
					pUnit->SetSecondCheckPoint ( pUnit->GetPosition (), 0 );

					// nastaví pøíznak zmìny kontrolních bodù
					pUnit->SetCheckPointInfoModified ();

					// aktualizuje ID vzhledu jednotky
					pUnit->SetDefaultAppearanceID ();

					// poèká na nalezení cesty
					return;
				}
				// jednotkou se podaøilo pohnout

				TRACE_SKILL3 ( "@ MoveSkill moved unit %d to position ( %d, %d )\n", 
					pUnit->GetID (), pUnit->GetPosition ().x, pUnit->GetPosition ().y );

				// zjistí, je-li vzdálenost od cíle cesty v tolerované vzdálenosti
				if ( pUnit->GetPosition ().GetDistanceSquare ( pData->m_pointDestination ) < 
					pData->m_dwToleranceSquareLimit )
				{	// vzdálenost od cíle cesty je v tolerované vzdálenosti
					// odstraní zbývající kontrolní body cesty
					while ( pData->m_pPath->GetCheckPointPosition () != NULL )
					{
						// odstraní další kontrolní bod cesty
						pData->m_pPath->RemoveCheckPoint ();
					}
					// ukonèí skillu
					Finish ( pUnit, EGR_OK );
					return;
				}
				// jednotka dosáhla kontrolního bodu
			}
			// jednotka dosáhla kontrolního bodu

			ASSERT ( pUnit->GetPosition () == pUnit->GetFirstCheckPoint ().GetPosition () );

			// odstraní dosažený kontrolní bod cesty
			pData->m_pPath->RemoveCheckPoint ();

			// vyplní kontrolní body jednotky a zjistí, je-li cesta prázdná
			if ( !FillCheckPoints ( pUnit ) )
			{	// cesta je prázdná
				// ukonèí skillu
				Finish ( pUnit, EGR_OK );
				return;
			}
			// cesta dosud není prázdná

			// pøipraví se na pøesun k prvnímu kontrolnímu bodu
			GoToFirstCheckPoint ( pUnit );

			// pokraèuje v pohybu ke kontrolnímu bodu
			return;
		}

		// urèí novou pozici jednotky
		CPointDW pointPosition ( (DWORD)( pData->m_dbXPosition = dbNewXPosition ), 
			(DWORD)( pData->m_dbYPosition = dbNewYPosition ) );

		// zjistí, zda se jednotka má skuteènì pohnout
		if ( pUnit->GetPosition () != pointPosition )
		{	// jednotka se má skuteènì pohnout
			// pokusí se pohnout jednotkou
			if ( !g_cMap.MoveUnit ( pUnit, pointPosition ) )
			{	// jednotkou se nepodaøilo pohnout
				// nastaví umístìní jednotky
				pData->m_dbXPosition = (double)pUnit->GetPosition ().x + (double)0.5;
				pData->m_dbYPosition = (double)pUnit->GetPosition ().y + (double)0.5;

				// nechá najít lokální cestu
				if ( pData->m_pPath->FindLocalPath ( pUnit ) )
				{	// první kontrolní bod cesty byl pøeskoèen
					// vyplní kontrolní body jednotky a zjistí, je-li cesta prázdná
					if ( !FillCheckPoints ( pUnit ) )
					{	// cesta je prázdná
						// ukonèí skillu
						Finish ( pUnit, EGR_OK );
						return;
					}
					// cesta dosud není prázdná

					// pøipraví se na pøesun k prvnímu kontrolnímu bodu
					GoToFirstCheckPoint ( pUnit );

					// pokraèuje v pohybu ke kontrolnímu bodu
					return;
				}
				// byl zadán požadavek na hledání lokální cesty

				// nastaví kontrolní body na stání
				pUnit->SetFirstCheckPoint ( pUnit->GetPosition (), 0 );
				pUnit->SetSecondCheckPoint ( pUnit->GetPosition (), 0 );

				// nastaví pøíznak zmìny kontrolních bodù
				pUnit->SetCheckPointInfoModified ();

				// aktualizuje ID vzhledu jednotky
				pUnit->SetDefaultAppearanceID ();

				// poèká na nalezení cesty
				return;
			}
			// jednotkou se podaøilo pohnout

			TRACE_SKILL3 ( "@ MoveSkill moved unit %d to position ( %d, %d )\n", 
				pUnit->GetID (), pUnit->GetPosition ().x, pUnit->GetPosition ().y );

			// zjistí, je-li vzdálenost od cíle cesty v tolerované vzdálenosti
			if ( pUnit->GetPosition ().GetDistanceSquare ( pData->m_pointDestination ) < 
				pData->m_dwToleranceSquareLimit )
			{	// vzdálenost od cíle cesty je v tolerované vzdálenosti
				// odstraní zbývající kontrolní body cesty
				while ( pData->m_pPath->GetCheckPointPosition () != NULL )
				{
					// odstraní další kontrolní bod cesty
					pData->m_pPath->RemoveCheckPoint ();
				}
				// ukonèí skillu
				Finish ( pUnit, EGR_OK );
				return;
			}
			// pokraèuje v pohybu ke kontrolnímu bodu
			return;
		}
		// jednotka se ještì nemá pohnout

		// pokraèuje v pohybu ke kontrolnímu bodu
		return;
	}
	else
	{	// má se èekat na volné místo
		// zjistí, má-li se èekání na volné místo ukonèit
		if ( ( ( --pData->m_dwWaitCount ) % WAITING_SCALE ) != 0 )
		{	// èekání na volné místo se dosud nemá ukonèit
			// pokraèuje v aktivním èekání na volné místo
			return;
		}
		// bylo ukonèeno èekání na volné místo

		// zjistí novou pozici jednotky na mapì
		CPointDW pointPosition ( (DWORD)pData->m_dbXPosition, (DWORD)pData->m_dbYPosition );

		// pokusí se pohnout jednotkou
		if ( !g_cMap.MoveUnit ( pUnit, pointPosition ) )
		{	// jednotkou se nepodaøilo pohnout
			// nastaví umístìní jednotky
			pData->m_dbXPosition = (double)pUnit->GetPosition ().x + (double)0.5;
			pData->m_dbYPosition = (double)pUnit->GetPosition ().y + (double)0.5;

			// aktualizuje èítaè poètu èekání
			pData->m_dwWaitCount += WAITING_SCALE;

			// nechá najít lokální cestu
			if ( !pData->m_pPath->FindLocalPath ( pUnit ) )
			{	// byl zadán požadavek na hledání lokální cesty
				// poèká na nalezení cesty
				return;
			}
			// první kontrolní bod cesty byl pøeskoèen
		}
		// jednotkou se podaøilo pohnout

		// vynuluje èítaè poètu èekání
		pData->m_dwWaitCount = 0;

		// vyplní kontrolní body jednotky a zjistí, je-li cesta prázdná
		if ( !FillCheckPoints ( pUnit ) )
		{	// cesta je prázdná
			// ukonèí skillu
			Finish ( pUnit, EGR_OK );
			return;
		}
		// cesta dosud není prázdná

		// pøipraví se na pøesun k prvnímu kontrolnímu bodu
		GoToFirstCheckPoint ( pUnit );

		// pokraèuje v pohybu ke kontrolnímu bodu
		return;
	}
}

/////////////////////////////////////////////////////////////////////
// Pomocné metody skilly
/////////////////////////////////////////////////////////////////////

// vyplní kontrolní body jednotky (jednotka musí být zamèena pro zápis) (FALSE=cesta je 
//		prázdná)
BOOL CSMoveSkillType::FillCheckPoints ( CSUnit *pUnit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	// ètverec vzdálenosti pozice jednotky od prvního kontrolního bodu cesty
	DWORD dwDistanceSquare;
	// ukazatel na první kontrolní bod cesty
	CPointDW *pFirstCheckPointPosition;

	// pøeskoèí dosažené kontrolní body cesty
	for ( ; ; )
	{
		// zjistí ukazatel na první kontrolní bod cesty
		pFirstCheckPointPosition = pData->m_pPath->GetCheckPointPosition ();
		// zjistí, je-li cesta prázdná
		if ( pFirstCheckPointPosition == NULL )
		{	// cesta je prázdná
			return FALSE;
		}

		// zjistí ètverec vzdálenosti pozice jednotky od prvního kontrolního bodu cesty
		dwDistanceSquare = pUnit->GetPosition ().GetDistanceSquare ( 
			*pFirstCheckPointPosition );

		// zjistí, je-li kontrolní bod cesty shodný s pozicí jednotky
		if ( dwDistanceSquare != 0 )
		{	// jedná se o kontrolní bod cesty rùzný od pozice jednotky
			// nechá vyplnit kontrolní body jednotky
			break;
		}
		// jedná se o kontrolní bod cesty shodný s pozicí jednotky

		// odstraní dosažený kontrolní bod cesty
		pData->m_pPath->RemoveCheckPoint ();
	}

	// zjistí aktuální TimeSlice
	DWORD dwTime = g_cMap.GetTimeSlice ();

	// spoèítá TimeSlice dosažení prvního kontrolního bodu
	// ********************************
	dwTime += (DWORD)ceil( ( sqrt ( dwDistanceSquare ) * (double)100 ) / 
		(double)pData->m_dwSpeed );
	// nastaví první kontrolní bod jednotky
	pUnit->SetFirstCheckPoint ( *pFirstCheckPointPosition, dwTime );

	// ukazatel na druhý kontrolní bod cesty
	CPointDW *pSecondCheckPointPosition = pData->m_pPath->GetNextCheckPointPosition ();

	// spoèítá TimeSlice dosažení druhého kontrolního bodu
	// ********************************
	dwTime += (DWORD)ceil( sqrt ( pFirstCheckPointPosition->GetDistanceSquare ( 
		*pSecondCheckPointPosition ) ) * (double)100 / 
		(double)pData->m_dwSpeed );
	// nastaví druhý kontrolní bod jednotky
	pUnit->SetSecondCheckPoint ( *pSecondCheckPointPosition, dwTime );

	// nastaví pøíznak zmìny kontrolních bodù jednotky
	pUnit->SetCheckPointInfoModified ();

	// vrátí pøíznak neprázdné cesty
	return TRUE;
}

// pøipraví data skilly na pøesun k prvnímu kontrolnímu bodu jednotky (jednotka musí být 
//		zamèena pro zápis)
void CSMoveSkillType::GoToFirstCheckPoint ( CSUnit *pUnit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	// zjistí vzdálenost prvního kontrolního bodu od pozice jednotky
	int nXDifference = pUnit->GetFirstCheckPoint ().GetPosition ().x - 
		pUnit->GetPosition ().x;
	int nYDifference = pUnit->GetFirstCheckPoint ().GetPosition ().y - 
		pUnit->GetPosition ().y;

	// zjistí èas na pøesun k prvnímu kontrolnímu bodu od pozice jednotky
	double dbTime = (double)( sqrt ( pUnit->GetPosition ().GetDistanceSquare ( 
		pUnit->GetFirstCheckPoint ().GetPosition () ) ) * (double)100 ) / 
		(double)pData->m_dwSpeed;

	// zjistí, je-li jednotka již na prvním kontrolním bodì
	if ( dbTime == 0 )
	{	// jednotka je již na prvním kontrolním bodì
		// vynuluje zmìnu souøadnic za TimeSlice
		pData->m_dbXStep = pData->m_dbYStep = 0;
	}
	else
	{	// jednotka dosud není na prvním kontrolním bodì
		// spoèítá zmìnu souøadnic za TimeSlice
		pData->m_dbXStep = (double)nXDifference / dbTime;
		pData->m_dbYStep = (double)nYDifference / dbTime;

		// aktualizuje smìr pohybu jednotky
		pUnit->SetDirectionFromDifference ( nXDifference, nYDifference );
	}
}

/////////////////////////////////////////////////////////////////////
// Interface volání ze skriptù
/////////////////////////////////////////////////////////////////////

// aktivuje skillu pro pohyb jednotky na pozici se souøadnicemi "pX" a "pY" (jednotka je 
//		zamèena pro zápis)
ESyscallResult CSMoveSkillType::SCI_Go ( CSUnit *pUnit, CIProcess *pProcess, 
	CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, int *pX, int *pY ) 
{
	// cílová pozice
	CPointDW pointDestination ( (DWORD)*pX, (DWORD)*pY );

	TRACE_SKILLCALL3 ( "@ Move.Go ( %d, %d ) called on unit %d ", pointDestination.x, pointDestination.y, pUnit->GetID () );
	TRACE_SKILLCALL2 ( "from position ( %d, %d )\n", pUnit->GetPosition ().x, pUnit->GetPosition ().y );

	// zjistí, jedná-li se o pozici na mapì
	if ( ( pointDestination.x < g_cMap.GetMapSizeMapCell ().cx ) && 
		( pointDestination.y < g_cMap.GetMapSizeMapCell ().cy ) )
	{	// jedná se o pozici na mapì
		// vytvoøí nové systémové volání
		CISysCallMove *pSysCall = new CISysCallMove;
		VERIFY ( pSysCall->Create ( pProcess, pUnit, this, pBag ) );

		// vytvoøí nový objekt cesty
		CSPath *pPath = CSPath::FindPath ( pUnit->GetPosition (), pointDestination, 
			pUnit->GetFindPathGraph (), pUnit->GetMoveWidth (), 
			pUnit->GetCivilizationIndex () );

		// aktivuje skillu
		ActivateSkill ( pUnit, pPath, pSysCall, pointDestination, 0 );

		// nastaví ukazatel na systémové volání (reference pro nìj zvýšena v Create)
		*ppSysCall = pSysCall;

		// vrátí pøíznak uspání procesu
		return SYSCALL_SUCCESS_SLEEP;
	}

	TRACE_SKILLCALL2 ( "@ Move.Go failed on unit %d (returning %d\n", pUnit->GetID (), EGR_NoMapDestination );

	// vrátí pøíznak chybné cílové pozice
	pBag->ReturnInt ( EGR_NoMapDestination );

	// vrátí pøíznak úspìchu
	return SYSCALL_SUCCESS;
}

// vrátí rychlost jednotky (jednotka je zamèena pro ètení)
ESyscallResult CSMoveSkillType::SCI_GetSpeed ( CSUnit *pUnit, CIProcess *pProcess, 
	CISyscall **ppSysCall, CIBag *pBag, bool bCalledByEnemy, CIDataStackItem *pReturn ) 
{
	// rychlost jednotky
	DWORD dwSpeed = SKILLDATA ( pUnit )->m_dwSpeed;

	TRACE_SKILLCALL1 ( "@ Move.GetSpeed () called, returning %d\n", dwSpeed );

	// vrátí rychlost jednotky
	pBag->ReturnInt ( (int)dwSpeed );

	// vrátí pøíznak úspìchu
	return SYSCALL_SUCCESS;
}

// aktivuje skillu pro pohyb jednotky na pozici "pPosition" (jednotka je zamèena pro zápis)
ESyscallResult CSMoveSkillType::SCI_GoPos ( CSUnit *pUnit, CIProcess *pProcess, 
	CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, 
	CISystemObject *pPosition ) 
{
	ASSERT ( pPosition->GetObjectType () == SO_POSITION );

	// cílová pozice
	CPointDW pointDestination ( (DWORD)((CISOPosition *)pPosition)->GetXPosition (), 
		(DWORD)((CISOPosition *)pPosition)->GetYPosition () );

	TRACE_SKILLCALL3 ( "@ Move.GoPos ( %d, %d ) called on unit %d ", pointDestination.x, pointDestination.y, pUnit->GetID () );
	TRACE_SKILLCALL2 ( "from position ( %d, %d )\n", pUnit->GetPosition ().x, pUnit->GetPosition ().y );

	// zjistí, jedná-li se o pozici na mapì
	if ( ( pointDestination.x < g_cMap.GetMapSizeMapCell ().cx ) && 
		( pointDestination.y < g_cMap.GetMapSizeMapCell ().cy ) )
	{	// jedná se o pozici na mapì
		// vytvoøí nové systémové volání
		CISysCallMove *pSysCall = new CISysCallMove;
		VERIFY ( pSysCall->Create ( pProcess, pUnit, this, pBag ) );

		// vytvoøí nový objekt cesty
		CSPath *pPath = CSPath::FindPath ( pUnit->GetPosition (), pointDestination, 
			pUnit->GetFindPathGraph (), pUnit->GetMoveWidth (), 
			pUnit->GetCivilizationIndex () );

		// aktivuje skillu
		ActivateSkill ( pUnit, pPath, pSysCall, pointDestination, 0 );

		// nastaví ukazatel na systémové volání (reference pro nìj zvýšena v Create)
		*ppSysCall = pSysCall;

		// vrátí pøíznak uspání procesu
		return SYSCALL_SUCCESS_SLEEP;
	}

	TRACE_SKILLCALL2 ( "@ Move.GoPos failed on unit %d (returning %d)\n", pUnit->GetID (), EGR_NoMapDestination );

	// vrátí pøíznak chybné cílové pozice
	pBag->ReturnInt ( EGR_NoMapDestination );

	// vrátí pøíznak úspìchu
	return SYSCALL_SUCCESS;
}

// aktivuje skillu pro pohyb jednotky na pozici se souøadnicemi "pX" a "pY", s tolerancí 
//		"pTolerance" (jednotka je zamèena pro zápis)
ESyscallResult CSMoveSkillType::SCI_GoCloseTo ( CSUnit *pUnit, CIProcess *pProcess, 
	CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, int *pX, int *pY, 
	int *pTolerance ) 
{
	// cílová pozice
	CPointDW pointDestination ( (DWORD)*pX, (DWORD)*pY );

	// zkontroluje parametr tolerance vzdálenosti od cílové pozice
	if ( *pTolerance > MAX_GO_CLOSE_TO_TOLERANCE )
	{	// parametr tolerance vzdálenosti od cílové pozice je pøíliš velký
		// zahlásí chybu parametru tolerance
		pProcess->m_pInterpret->ReportError ( "Move.GoCloseTo: Tolerance too large." );
		// opraví parametr tolerance
		*pTolerance = MAX_GO_CLOSE_TO_TOLERANCE;
	}
	else if ( *pTolerance < 0 )
	{	// parametr tolerance vzdálenosti od cílové pozice je záporný
		// zahlásí chybu parametru tolerance
		pProcess->m_pInterpret->ReportError ( "Move.GoCloseTo: Tolerance negative." );
		// opraví parametr tolerance
		*pTolerance = 0;
	}

	TRACE_SKILLCALL ( "@ Move.GoCloseTo ( %d, %d ) with tolerance %d called on unit %d ", pointDestination.x, pointDestination.y, *pTolerance, pUnit->GetID () );
	TRACE_SKILLCALL2 ( "from position ( %d, %d )\n", pUnit->GetPosition ().x, pUnit->GetPosition ().y );

	// zjistí, jedná-li se o pozici na mapì
	if ( ( pointDestination.x < g_cMap.GetMapSizeMapCell ().cx ) && 
		( pointDestination.y < g_cMap.GetMapSizeMapCell ().cy ) )
	{	// jedná se o pozici na mapì
		// vytvoøí nové systémové volání
		CISysCallMove *pSysCall = new CISysCallMove;
		VERIFY ( pSysCall->Create ( pProcess, pUnit, this, pBag ) );

		// vytvoøí nový objekt cesty
		CSPath *pPath = CSPath::FindPath ( pUnit->GetPosition (), pointDestination, 
			pUnit->GetFindPathGraph (), pUnit->GetMoveWidth (), 
			pUnit->GetCivilizationIndex () );

		// aktivuje skillu
		ActivateSkill ( pUnit, pPath, pSysCall, pointDestination, ((DWORD)*pTolerance) * 
			((DWORD)*pTolerance) + 1 );

		// nastaví ukazatel na systémové volání (reference pro nìj zvýšena v Create)
		*ppSysCall = pSysCall;

		// vrátí pøíznak uspání procesu
		return SYSCALL_SUCCESS_SLEEP;
	}

	TRACE_SKILLCALL2 ( "@ Move.GoCloseTo failed on unit %d (returning %d)\n", pUnit->GetID (), EGR_NoMapDestination );

	// vrátí pøíznak chybné cílové pozice
	pBag->ReturnInt ( EGR_NoMapDestination );

	// vrátí pøíznak úspìchu
	return SYSCALL_SUCCESS;
}

// aktivuje skillu pro pohyb jednotky na pozici "pPosition" s tolerancí "pTolerance" 
//		(jednotka je zamèena pro zápis)
ESyscallResult CSMoveSkillType::SCI_GoCloseToPos ( CSUnit *pUnit, CIProcess *pProcess, 
	CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, 
	CISystemObject *pPosition, int *pTolerance ) 
{
	ASSERT ( pPosition->GetObjectType () == SO_POSITION );

	// cílová pozice
	CPointDW pointDestination ( (DWORD)((CISOPosition *)pPosition)->GetXPosition (), 
		(DWORD)((CISOPosition *)pPosition)->GetYPosition () );

	// zkontroluje parametr tolerance vzdálenosti od cílové pozice
	if ( *pTolerance > MAX_GO_CLOSE_TO_TOLERANCE )
	{	// parametr tolerance vzdálenosti od cílové pozice je pøíliš velký
		// zahlásí chybu parametru tolerance
		pProcess->m_pInterpret->ReportError ( "Move.GoCloseToPos: Tolerance too large." );
		// opraví parametr tolerance
		*pTolerance = MAX_GO_CLOSE_TO_TOLERANCE;
	}
	else if ( *pTolerance < 0 )
	{	// parametr tolerance vzdálenosti od cílové pozice je záporný
		// zahlásí chybu parametru tolerance
		pProcess->m_pInterpret->ReportError ( "Move.GoCloseToPos: Tolerance negative." );
		// opraví parametr tolerance
		*pTolerance = 0;
	}

	TRACE_SKILLCALL ( "@ Move.GoCloseToPos ( %d, %d ) with tolerance %d called on unit %d ", pointDestination.x, pointDestination.y, *pTolerance, pUnit->GetID () );
	TRACE_SKILLCALL2 ( "from position ( %d, %d )\n", pUnit->GetPosition ().x, pUnit->GetPosition ().y );

	// zjistí, jedná-li se o pozici na mapì
	if ( ( pointDestination.x < g_cMap.GetMapSizeMapCell ().cx ) && 
		( pointDestination.y < g_cMap.GetMapSizeMapCell ().cy ) )
	{	// jedná se o pozici na mapì
		// vytvoøí nové systémové volání
		CISysCallMove *pSysCall = new CISysCallMove;
		VERIFY ( pSysCall->Create ( pProcess, pUnit, this, pBag ) );

		// vytvoøí nový objekt cesty
		CSPath *pPath = CSPath::FindPath ( pUnit->GetPosition (), pointDestination, 
			pUnit->GetFindPathGraph (), pUnit->GetMoveWidth (), 
			pUnit->GetCivilizationIndex () );

		// aktivuje skillu
		ActivateSkill ( pUnit, pPath, pSysCall, pointDestination, ((DWORD)*pTolerance) * 
			((DWORD)*pTolerance) + 1 );

		// nastaví ukazatel na systémové volání (reference pro nìj zvýšena v Create)
		*ppSysCall = pSysCall;

		// vrátí pøíznak uspání procesu
		return SYSCALL_SUCCESS_SLEEP;
	}

	TRACE_SKILLCALL2 ( "@ Move.GoCloseToPos failed on unit %d (returning %d)\n", pUnit->GetID (), EGR_NoMapDestination );

	// vrátí pøíznak chybné cílové pozice
	pBag->ReturnInt ( EGR_NoMapDestination );

	// vrátí pøíznak úspìchu
	return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// Debuggovací informace
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// zkontroluje neškodná data skilly (TRUE=OK)
BOOL CSMoveSkillType::CheckEmptySkillData ( CSUnit *pUnit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	ASSERT ( pData->m_pPath == NULL );
	ASSERT ( pData->m_pSysCall == NULL );

	ASSERT ( pData->m_dbXStep == 0 );
	ASSERT ( pData->m_dbYStep == 0 );

	ASSERT ( pData->m_dbXPosition == DWORD_MAX );
	ASSERT ( pData->m_dbYPosition == DWORD_MAX );

	ASSERT ( pData->m_dwWaitCount == WAITING_SCALE - 1 );

	ASSERT ( pData->m_pointDestination == CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION ) );
	ASSERT ( pData->m_dwToleranceSquareLimit == 0 );

	return TRUE;
}

#endif //_DEBUG
