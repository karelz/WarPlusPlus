/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry - skilly
 *   Autor: V�t Karas & Karel Zikmund
 * 
 *   Popis: Typ pohybov� skilly (move skilla)
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

// minim�ln� a maxim�ln� po�et TimeSlic� �ek�n�, neda�i-li se pohnout s jednotkou
#define WAITING_TIMEOUT_MIN	10
#define WAITING_TIMEOUT_MAX	30
#define WAITING_TIMEOUT			( WAITING_TIMEOUT_MIN + rand () % ( WAITING_TIMEOUT_MAX - WAITING_TIMEOUT_MIN + 1 ) )
// po�et opakov�n� �ek�n�, neda��-li se pohnout s jednotkou
#define WAITING_TRIES	10
// �k�lov�n� opakov�n� �ek�n�, neda��-li se pohnout s jednotkou
#define WAITING_SCALE	( DWORD_MAX / WAITING_TRIES )

// maxim�ln� rychlost
#define SPEED_MAX							1000
// maxim�ln� tolerance vzd�lenosti od c�lov� pozice metody GoCloseTo
#define MAX_GO_CLOSE_TO_TOLERANCE	1000

// implementace interfacu vol�n� ze skript�
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
// Inicializace a zni�en� dat objektu
/////////////////////////////////////////////////////////////////////

// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
//		v�jimky: CPersistentLoadException
void CSMoveSkillType::CreateSkillType ( void *pData, DWORD dwDataSize ) 
{
	LOAD_ASSERT ( dwDataSize == sizeof ( SMMoveSkillType ) );

	ASSERT ( m_dwSpeed == DWORD_MAX );

	m_dwSpeed = ((SMMoveSkillType *)pData)->m_dwSpeed;
	m_dwAppearanceID = ((SMMoveSkillType *)pData)->m_dwMoveAppearanceID;

	LOAD_ASSERT ( m_dwSpeed <= SPEED_MAX );
}

// zni�� data typu skilly
void CSMoveSkillType::DeleteSkillType () 
{
	ASSERT ( m_dwSpeed != DWORD_MAX );

	m_dwSpeed = DWORD_MAX;
}

/////////////////////////////////////////////////////////////////////
// Ukl�d�n� dat potomk� (analogie CPersistentObject metod)
/////////////////////////////////////////////////////////////////////

/*
CSMoveSkillType stored

	DWORD m_dwSpeed
	DWORD m_dwAppearanceID
*/

// ukl�d�n� dat potomka
void CSMoveSkillType::PersistentSaveSkillType ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	ASSERT ( m_dwSpeed <= SPEED_MAX );

	storage << m_dwSpeed;
	storage << m_dwAppearanceID;
}

// nahr�v�n� pouze ulo�en�ch dat potomka
void CSMoveSkillType::PersistentLoadSkillType ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	storage >> m_dwSpeed;
	storage >> m_dwAppearanceID;

	LOAD_ASSERT ( m_dwSpeed <= SPEED_MAX );
}

// p�eklad ukazatel� potomka
void CSMoveSkillType::PersistentTranslatePointersSkillType ( CPersistentStorage &storage ) 
{
}

// inicializace nahran�ho objektu potomka
void CSMoveSkillType::PersistentInitSkillType () 
{
}

/////////////////////////////////////////////////////////////////////
// Get/Set metody serverov�ch informac� o typu skilly jednotky
/////////////////////////////////////////////////////////////////////

// vr�t� velikost dat skilly pro jednotku
DWORD CSMoveSkillType::GetSkillDataSize () 
{
	return sizeof ( SSMoveSkill );
}

// vr�t� masku skupin skill
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

	// znehodnot� data skilly
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
	// inicializuje v�choz� bod
	pData->m_pointStart = CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION );
#endif //_DEBUG
}

// zni�� skillu jednotky "pUnit"
void CSMoveSkillType::DestructSkill ( CSUnit *pUnit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	ASSERT ( pData->m_dwSpeed <= SPEED_MAX );

	// zjist�, je-li cesta platn�
	if ( pData->m_pPath != NULL )
	{	// cesta je platn�
		// zni�� cestu
		pData->m_pPath->Destroy ( pUnit->GetCivilizationIndex () );
		pData->m_pPath = NULL;
	}

	// zjist�, je-li asociov�no syst�mov� vol�n�
	if ( pData->m_pSysCall != NULL )
	{	// je asociov�n� syst�mov� vol�n�
		// ukon�� �ekaj�c� syst�mov� vol�n�
		pData->m_pSysCall->FinishWaiting ( STATUS_SKILL_COLLISION );
		pData->m_pSysCall->Release ();
		pData->m_pSysCall = NULL;
	}
}

// aktivuje skillu jednotky "pUnit" (jednotka mus� b�t zam�ena pro z�pis)
void CSMoveSkillType::ActivateSkill ( CSUnit *pUnit, CSPath *pPath, 
	CISysCallMove *pSysCall, CPointDW pointDestination, DWORD dwToleranceSquareLimit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	// deaktivuje neslu�iteln� skilly
	GetUnitType ()->DeactivateSkillGroup ( pUnit, Group_Move | Group_Attack );

	ASSERT ( pData->m_dwSpeed <= SPEED_MAX );
	ASSERT ( CheckEmptySkillData ( pUnit ) );

	// nastav� syst�mov� vol�n�
	pSysCall->AddRef ();
	pData->m_pSysCall = pSysCall;

	// nastav� cestu pohybu
	pData->m_pPath = pPath;

	// nastav� um�st�n� jednotky
	pData->m_dbXPosition = (double)pUnit->GetPosition ().x + (double)0.5;
	pData->m_dbYPosition = (double)pUnit->GetPosition ().y + (double)0.5;

	// nastav� dobu �ek�n�
	pData->m_dwWaitCount = 0;

	// nastav� c�lovou pozici
	pData->m_pointDestination = pointDestination;
	// nastav� limit �tverce tolerance vzd�lenosti od c�lov� pozice
	pData->m_dwToleranceSquareLimit = dwToleranceSquareLimit;

#ifdef _DEBUG
	// nastav� v�choz� pozici
	pData->m_pointStart = pUnit->GetPosition ();
#endif //_DEBUG

	// aktivuje skillu na jednotce
	CSSkillType::ActivateSkill ( pUnit );
}

// deaktivuje skillu jednotky "pUnit" (jednotka mus� b�t zam�ena pro z�pis)
void CSMoveSkillType::DeactivateSkill ( CSUnit *pUnit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	// zni�� cestu
	ASSERT ( pData->m_pPath != NULL );
	pData->m_pPath->Destroy ( pUnit->GetCivilizationIndex () );
	pData->m_pPath = NULL;

	// zne�kodn� krok pohybu
	pData->m_dbXStep = 0;
	pData->m_dbYStep = 0;

	// zne�kodn� um�st�n� jednotky
	pData->m_dbXPosition = DWORD_MAX;
	pData->m_dbYPosition = DWORD_MAX;

	// zjist�, je-li asociov�no syst�mov� vol�n�
	if ( pData->m_pSysCall != NULL )
	{	// je asociov�n� syst�mov� vol�n�
		// ukon�� �ekaj�c� syst�mov� vol�n�
		pData->m_pSysCall->FinishWaiting ( STATUS_SKILL_COLLISION );
		pData->m_pSysCall->Release ();
		pData->m_pSysCall = NULL;
	}

	// nastav� kontroln� body jednotky na st�n� na m�st�
	pUnit->SetFirstCheckPoint ( pUnit->GetPosition(), 0 );
	pUnit->SetSecondCheckPoint ( pUnit->GetPosition(), 0 );
	// nastav� p��znak zm�ny kontroln�ch bod�
	pUnit->SetCheckPointInfoModified ();

	// znehodnot� data skilly
	pData->m_dwWaitCount = WAITING_SCALE - 1;

	// zne�kodn� c�lovou pozici
	pData->m_pointDestination = CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION );
	// zne�kodn� limit �tverce tolerance vzd�lenosti od c�lov� pozice
	pData->m_dwToleranceSquareLimit = 0;

	// nastav� standardn� vzhled jednotky
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

// ukl�d�n� dat skilly
void CSMoveSkillType::PersistentSaveSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	BRACE_BLOCK ( storage );

	SSMoveSkill *pData = SKILLDATA ( pUnit );

	ASSERT ( pData->m_dwSpeed <= SPEED_MAX );

	// ulo�� data skilly
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

	// zjist�, je-li cesta platn�
	if ( pData->m_pPath != NULL )
	{	// cesta je platn�
		// ulo�� cestu
		pData->m_pPath->PersistentSave ( storage );
	}

	// zjist�, je-li syst�mov� vol�n� platn�
	if ( pData->m_pSysCall != NULL )
	{	// syst�mov� vol�n� je platn�
		// ulo�� syst�mov� vol�n�
		pData->m_pSysCall->PersistentSave ( storage );
	}
}

// nahr�v�n� pouze ulo�en�ch dat skilly
void CSMoveSkillType::PersistentLoadSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	BRACE_BLOCK ( storage );

	SSMoveSkill *pData = SKILLDATA ( pUnit );

	ASSERT ( CheckEmptySkillData ( pUnit ) );

	// na�te data skilly
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

	// na�te star� ukazatel na cestu
	void *pOldPath;
	storage >> pOldPath;

	// na�te p��znak syst�mov�ho vol�n�
	BOOL bSysCallMove;
	storage >> bSysCallMove;
	LOAD_ASSERT ( ( bSysCallMove == TRUE ) || ( bSysCallMove == FALSE ) );

	// zjist�, je-li cesta platn�
	if ( pOldPath != NULL )
	{	// cesta je platn�
		// na�te cestu
		pData->m_pPath = CSPath::PersistentLoadPath ( storage );
		// zaregistruje ukazatel na cestu
		storage.RegisterPointer ( pOldPath, pData->m_pPath );
	}
	else
	{	// cesta je neplatn�
		// aktualizuje ukazatel na cestu
		pData->m_pPath = NULL;
	}

	// zjist�, je-li syst�mov� vol�n� platn�
	if ( bSysCallMove )
	{	// syst�mov� vol�n� je platn�
		// vytvo�� syst�mov� vol�n�
		pData->m_pSysCall = new CISysCallMove;
		// na�te syst�mov� vol�n�
		pData->m_pSysCall->PersistentLoad ( storage );
	}
	else
	{	// syst�mov� vol�n� je neplatn�
		// aktualizuje ukazatel na syst�mov� vol�n�
		pData->m_pSysCall = NULL;
	}
}

// p�eklad ukazatel� dat skilly
void CSMoveSkillType::PersistentTranslatePointersSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	// zjist�, je-li cesta platn�
	if ( pData->m_pPath != NULL )
	{	// cesta je platn�
		// p�elo�� ukazatele cesty
		pData->m_pPath->PersistentTranslatePointers ( storage );
	}

	// zjist�, je-li syst�mov� vol�n� platn�
	if ( pData->m_pSysCall != NULL )
	{	// syst�mov� vol�n� je platn�
		// p�elo�� ukazatele syst�mov�ho vol�n�
		pData->m_pSysCall->PersistentTranslatePointers ( storage );
	}
}

// inicializace nahran�ch dat skilly
void CSMoveSkillType::PersistentInitSkill ( CSUnit *pUnit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	// zjist�, je-li cesta platn�
	if ( pData->m_pPath != NULL )
	{	// cesta je platn�
		// inicializuje cestu
		pData->m_pPath->PersistentInit ();
	}

	// zjist�, je-li syst�mov� vol�n� platn�
	if ( pData->m_pSysCall != NULL )
	{	// syst�mov� vol�n� je platn�
		// inicializuje syst�mov� vol�n�
		pData->m_pSysCall->PersistentInit ();
	}

#ifdef _DEBUG
	// inicializuje v�choz� bod
	pData->m_pointStart = CPointDW ( NO_MAP_POSITION, NO_MAP_POSITION );
#endif //_DEBUG
}

// vypln� full info "strInfo" skilly jednotky "pUnit"
void CSMoveSkillType::FillFullInfo ( CSUnit *pUnit, CString &strInfo ) 
{
	// zjist�, je-li skilla povolen�
	if ( IsEnabled ( pUnit ) )
	{	// skilla je povolen�
		// vypln� full info jednotky
		strInfo.Format ( "Move: speed %u", SKILLDATA ( pUnit )->m_dwSpeed );
	}
	else
	{	// skilla nen� povolen�
		// vypln� full info jednotky
		strInfo = "Move: (disabled)";
	}
}

// vypln� enemy full info "strInfo" skilly jednotky "pUnit"
void CSMoveSkillType::FillEnemyFullInfo ( CSUnit *pUnit, CString &strInfo ) 
{
	// vypln� enemy full info jednotky
	strInfo = "Move Skill";
}

// provede TimeSlice na skille jednotky "pUnit", jednotka "pUnit" je ji� zam�en� pro 
//		z�pis, metoda m��e zamykat libovoln� po�et jin�ch jednotek pro z�pis/�ten� (v�etn� 
//		jednotek ze sv�ho bloku)
void CSMoveSkillType::DoTimeSlice ( CSUnit *pUnit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	ASSERT ( pData->m_pPath != NULL );

	// zjist�, stav cesty
	switch ( pData->m_pPath->GetState () )
	{
	// cesta je v po��dku
	case CSPath::EPS_Valid :
		// pokra�uje v p�esunov�n� jednotky po cest�
		break;
	// cesta byla nalezena (mo�n� i pr�zdn�)
	case CSPath::EPS_Found :
		// p�ijme cestu
		pData->m_pPath->Confirm ();

		// vynuluje ��ta� �ek�n�
		pData->m_dwWaitCount = 0;

		// vypln� kontroln� body jednotky a zjist�, je-li cesta pr�zdn�
		if ( !FillCheckPoints ( pUnit ) )
		{	// cesta je pr�zdn�
			// ukon�� skillu
			Finish ( pUnit, EGR_OK );
			return;
		}
		// cesta dosud nen� pr�zdn�

		// aktualizuje ID vzhledu jednotky
		pUnit->SetAppearanceID ( m_dwAppearanceID );

		// p�iprav� se na p�esun k prvn�mu kontroln�mu bodu
		GoToFirstCheckPoint ( pUnit );

		// pokra�uje v p�esunov�n� jednotky po cest�
		break;
	// cesta se hled�
	case CSPath::EPS_Searching :
		// po�k� na nalezen� cesty
		return;
	// cesta neexistuje (jednotka m��e b�t zablokov�na jednotkami)
	case CSPath::EPS_NotExists :
		ASSERT ( ( pData->m_dwWaitCount % WAITING_SCALE ) == 0 );
		// zjist�, m�-li se znovu �ekat
		if ( ( pData->m_dwWaitCount / WAITING_SCALE == ( WAITING_TRIES - 1 ) ) || 
			( pData->m_dwWaitCount == 0 ) )
		{	// cesta v�bec neexistuje nebo se ji� nem� znovu �ekat
			// ukon�� skillu
			Finish ( pUnit, EGR_PathNotExists );
			return;
		}
		// p�ijme nenalezenou cestu
		pData->m_pPath->Confirm ();

		// zkus� je�t� po�kat
		pData->m_dwWaitCount += WAITING_TIMEOUT;
		// pokra�uje v aktivn�m �ek�n� na voln� m�sto
		return;
	// cesta je zablokov�na (lok�ln� cesta neexistuje)
	case CSPath::EPS_Blocked :
		ASSERT ( ( pData->m_dwWaitCount % WAITING_SCALE ) == 0 );
		//ASSERT ( ( pData->m_dwWaitCount / WAITING_SCALE ) != 0 );
		// ****************** ZDE hled�n� zablokovan� cesty
		// ukon�� skillu
		Finish ( pUnit, EGR_PathBlocked );
		return;
	// nemo�n� p��pad
	case CSPath::EPS_Uninitialized :
	default :
		ASSERT ( FALSE );
	}

	// zjist�, �ek�-li se na voln� m�sto
	if ( pData->m_dwWaitCount == 0 )
	{	// ne�ek� se na voln� m�sto
	// ********************************
//		ASSERT ( g_cMap.GetTimeSlice () <= pUnit->GetFirstCheckPoint ().GetTime () );

		// spo��t� novou pozici jednotky
		double dbNewXPosition = pData->m_dbXPosition + pData->m_dbXStep;
		double dbNewYPosition = pData->m_dbYPosition + pData->m_dbYStep;

		// zjist�, nen�-li nov� pozice a� za prvn�m kontroln�m bodem
		if ( ( pData->m_dbXStep * ( (double)pData->m_pPath->GetCheckPointPosition ()->x + 
			(double)0.5 - dbNewXPosition ) <= 0 ) && ( pData->m_dbYStep * 
			( (double)pData->m_pPath->GetCheckPointPosition ()->y + (double)0.5 - 
			dbNewYPosition ) <= 0 ) )
		{	// nov� pozice je ji� za prvn�m kontroln�m bodem
			// nastav� novou pozici jednotky na kontroln� bod
			dbNewXPosition = (double)pData->m_pPath->GetCheckPointPosition ()->x + 
				(double)0.5;
			dbNewYPosition = (double)pData->m_pPath->GetCheckPointPosition ()->y + 
				(double)0.5;

			// ur�� novou pozici jednotky
			CPointDW pointPosition ( (DWORD)( pData->m_dbXPosition = dbNewXPosition ), 
				(DWORD)( pData->m_dbYPosition = dbNewYPosition ) );

			// zjist�, zda se jednotka m� skute�n� pohnout
			if ( pUnit->GetPosition () != pointPosition )
			{	// jednotka se m� skute�n� pohnout
				// pokus� se pohnout jednotkou
				if ( !g_cMap.MoveUnit ( pUnit, pointPosition ) )
				{	// jednotkou se nepoda�ilo pohnout
					// nastav� um�st�n� jednotky
					pData->m_dbXPosition = (double)pUnit->GetPosition ().x + (double)0.5;
					pData->m_dbYPosition = (double)pUnit->GetPosition ().y + (double)0.5;

					// nech� naj�t lok�ln� cestu
					if ( pData->m_pPath->FindLocalPath ( pUnit ) )
					{	// prvn� kontroln� bod cesty byl p�esko�en
						// vypln� kontroln� body jednotky a zjist�, je-li cesta pr�zdn�
						if ( !FillCheckPoints ( pUnit ) )
						{	// cesta je pr�zdn�
							// ukon�� skillu
							Finish ( pUnit, EGR_OK );
							return;
						}
						// cesta dosud nen� pr�zdn�

						// p�iprav� se na p�esun k prvn�mu kontroln�mu bodu
						GoToFirstCheckPoint ( pUnit );

						// pokra�uje v pohybu ke kontroln�mu bodu
						return;
					}
					// byl zad�n po�adavek na hled�n� lok�ln� cesty

					// nastav� kontroln� body na st�n�
					pUnit->SetFirstCheckPoint ( pUnit->GetPosition (), 0 );
					pUnit->SetSecondCheckPoint ( pUnit->GetPosition (), 0 );

					// nastav� p��znak zm�ny kontroln�ch bod�
					pUnit->SetCheckPointInfoModified ();

					// aktualizuje ID vzhledu jednotky
					pUnit->SetDefaultAppearanceID ();

					// po�k� na nalezen� cesty
					return;
				}
				// jednotkou se poda�ilo pohnout

				TRACE_SKILL3 ( "@ MoveSkill moved unit %d to position ( %d, %d )\n", 
					pUnit->GetID (), pUnit->GetPosition ().x, pUnit->GetPosition ().y );

				// zjist�, je-li vzd�lenost od c�le cesty v tolerovan� vzd�lenosti
				if ( pUnit->GetPosition ().GetDistanceSquare ( pData->m_pointDestination ) < 
					pData->m_dwToleranceSquareLimit )
				{	// vzd�lenost od c�le cesty je v tolerovan� vzd�lenosti
					// odstran� zb�vaj�c� kontroln� body cesty
					while ( pData->m_pPath->GetCheckPointPosition () != NULL )
					{
						// odstran� dal�� kontroln� bod cesty
						pData->m_pPath->RemoveCheckPoint ();
					}
					// ukon�� skillu
					Finish ( pUnit, EGR_OK );
					return;
				}
				// jednotka dos�hla kontroln�ho bodu
			}
			// jednotka dos�hla kontroln�ho bodu

			ASSERT ( pUnit->GetPosition () == pUnit->GetFirstCheckPoint ().GetPosition () );

			// odstran� dosa�en� kontroln� bod cesty
			pData->m_pPath->RemoveCheckPoint ();

			// vypln� kontroln� body jednotky a zjist�, je-li cesta pr�zdn�
			if ( !FillCheckPoints ( pUnit ) )
			{	// cesta je pr�zdn�
				// ukon�� skillu
				Finish ( pUnit, EGR_OK );
				return;
			}
			// cesta dosud nen� pr�zdn�

			// p�iprav� se na p�esun k prvn�mu kontroln�mu bodu
			GoToFirstCheckPoint ( pUnit );

			// pokra�uje v pohybu ke kontroln�mu bodu
			return;
		}

		// ur�� novou pozici jednotky
		CPointDW pointPosition ( (DWORD)( pData->m_dbXPosition = dbNewXPosition ), 
			(DWORD)( pData->m_dbYPosition = dbNewYPosition ) );

		// zjist�, zda se jednotka m� skute�n� pohnout
		if ( pUnit->GetPosition () != pointPosition )
		{	// jednotka se m� skute�n� pohnout
			// pokus� se pohnout jednotkou
			if ( !g_cMap.MoveUnit ( pUnit, pointPosition ) )
			{	// jednotkou se nepoda�ilo pohnout
				// nastav� um�st�n� jednotky
				pData->m_dbXPosition = (double)pUnit->GetPosition ().x + (double)0.5;
				pData->m_dbYPosition = (double)pUnit->GetPosition ().y + (double)0.5;

				// nech� naj�t lok�ln� cestu
				if ( pData->m_pPath->FindLocalPath ( pUnit ) )
				{	// prvn� kontroln� bod cesty byl p�esko�en
					// vypln� kontroln� body jednotky a zjist�, je-li cesta pr�zdn�
					if ( !FillCheckPoints ( pUnit ) )
					{	// cesta je pr�zdn�
						// ukon�� skillu
						Finish ( pUnit, EGR_OK );
						return;
					}
					// cesta dosud nen� pr�zdn�

					// p�iprav� se na p�esun k prvn�mu kontroln�mu bodu
					GoToFirstCheckPoint ( pUnit );

					// pokra�uje v pohybu ke kontroln�mu bodu
					return;
				}
				// byl zad�n po�adavek na hled�n� lok�ln� cesty

				// nastav� kontroln� body na st�n�
				pUnit->SetFirstCheckPoint ( pUnit->GetPosition (), 0 );
				pUnit->SetSecondCheckPoint ( pUnit->GetPosition (), 0 );

				// nastav� p��znak zm�ny kontroln�ch bod�
				pUnit->SetCheckPointInfoModified ();

				// aktualizuje ID vzhledu jednotky
				pUnit->SetDefaultAppearanceID ();

				// po�k� na nalezen� cesty
				return;
			}
			// jednotkou se poda�ilo pohnout

			TRACE_SKILL3 ( "@ MoveSkill moved unit %d to position ( %d, %d )\n", 
				pUnit->GetID (), pUnit->GetPosition ().x, pUnit->GetPosition ().y );

			// zjist�, je-li vzd�lenost od c�le cesty v tolerovan� vzd�lenosti
			if ( pUnit->GetPosition ().GetDistanceSquare ( pData->m_pointDestination ) < 
				pData->m_dwToleranceSquareLimit )
			{	// vzd�lenost od c�le cesty je v tolerovan� vzd�lenosti
				// odstran� zb�vaj�c� kontroln� body cesty
				while ( pData->m_pPath->GetCheckPointPosition () != NULL )
				{
					// odstran� dal�� kontroln� bod cesty
					pData->m_pPath->RemoveCheckPoint ();
				}
				// ukon�� skillu
				Finish ( pUnit, EGR_OK );
				return;
			}
			// pokra�uje v pohybu ke kontroln�mu bodu
			return;
		}
		// jednotka se je�t� nem� pohnout

		// pokra�uje v pohybu ke kontroln�mu bodu
		return;
	}
	else
	{	// m� se �ekat na voln� m�sto
		// zjist�, m�-li se �ek�n� na voln� m�sto ukon�it
		if ( ( ( --pData->m_dwWaitCount ) % WAITING_SCALE ) != 0 )
		{	// �ek�n� na voln� m�sto se dosud nem� ukon�it
			// pokra�uje v aktivn�m �ek�n� na voln� m�sto
			return;
		}
		// bylo ukon�eno �ek�n� na voln� m�sto

		// zjist� novou pozici jednotky na map�
		CPointDW pointPosition ( (DWORD)pData->m_dbXPosition, (DWORD)pData->m_dbYPosition );

		// pokus� se pohnout jednotkou
		if ( !g_cMap.MoveUnit ( pUnit, pointPosition ) )
		{	// jednotkou se nepoda�ilo pohnout
			// nastav� um�st�n� jednotky
			pData->m_dbXPosition = (double)pUnit->GetPosition ().x + (double)0.5;
			pData->m_dbYPosition = (double)pUnit->GetPosition ().y + (double)0.5;

			// aktualizuje ��ta� po�tu �ek�n�
			pData->m_dwWaitCount += WAITING_SCALE;

			// nech� naj�t lok�ln� cestu
			if ( !pData->m_pPath->FindLocalPath ( pUnit ) )
			{	// byl zad�n po�adavek na hled�n� lok�ln� cesty
				// po�k� na nalezen� cesty
				return;
			}
			// prvn� kontroln� bod cesty byl p�esko�en
		}
		// jednotkou se poda�ilo pohnout

		// vynuluje ��ta� po�tu �ek�n�
		pData->m_dwWaitCount = 0;

		// vypln� kontroln� body jednotky a zjist�, je-li cesta pr�zdn�
		if ( !FillCheckPoints ( pUnit ) )
		{	// cesta je pr�zdn�
			// ukon�� skillu
			Finish ( pUnit, EGR_OK );
			return;
		}
		// cesta dosud nen� pr�zdn�

		// p�iprav� se na p�esun k prvn�mu kontroln�mu bodu
		GoToFirstCheckPoint ( pUnit );

		// pokra�uje v pohybu ke kontroln�mu bodu
		return;
	}
}

/////////////////////////////////////////////////////////////////////
// Pomocn� metody skilly
/////////////////////////////////////////////////////////////////////

// vypln� kontroln� body jednotky (jednotka mus� b�t zam�ena pro z�pis) (FALSE=cesta je 
//		pr�zdn�)
BOOL CSMoveSkillType::FillCheckPoints ( CSUnit *pUnit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	// �tverec vzd�lenosti pozice jednotky od prvn�ho kontroln�ho bodu cesty
	DWORD dwDistanceSquare;
	// ukazatel na prvn� kontroln� bod cesty
	CPointDW *pFirstCheckPointPosition;

	// p�esko�� dosa�en� kontroln� body cesty
	for ( ; ; )
	{
		// zjist� ukazatel na prvn� kontroln� bod cesty
		pFirstCheckPointPosition = pData->m_pPath->GetCheckPointPosition ();
		// zjist�, je-li cesta pr�zdn�
		if ( pFirstCheckPointPosition == NULL )
		{	// cesta je pr�zdn�
			return FALSE;
		}

		// zjist� �tverec vzd�lenosti pozice jednotky od prvn�ho kontroln�ho bodu cesty
		dwDistanceSquare = pUnit->GetPosition ().GetDistanceSquare ( 
			*pFirstCheckPointPosition );

		// zjist�, je-li kontroln� bod cesty shodn� s pozic� jednotky
		if ( dwDistanceSquare != 0 )
		{	// jedn� se o kontroln� bod cesty r�zn� od pozice jednotky
			// nech� vyplnit kontroln� body jednotky
			break;
		}
		// jedn� se o kontroln� bod cesty shodn� s pozic� jednotky

		// odstran� dosa�en� kontroln� bod cesty
		pData->m_pPath->RemoveCheckPoint ();
	}

	// zjist� aktu�ln� TimeSlice
	DWORD dwTime = g_cMap.GetTimeSlice ();

	// spo��t� TimeSlice dosa�en� prvn�ho kontroln�ho bodu
	// ********************************
	dwTime += (DWORD)ceil( ( sqrt ( dwDistanceSquare ) * (double)100 ) / 
		(double)pData->m_dwSpeed );
	// nastav� prvn� kontroln� bod jednotky
	pUnit->SetFirstCheckPoint ( *pFirstCheckPointPosition, dwTime );

	// ukazatel na druh� kontroln� bod cesty
	CPointDW *pSecondCheckPointPosition = pData->m_pPath->GetNextCheckPointPosition ();

	// spo��t� TimeSlice dosa�en� druh�ho kontroln�ho bodu
	// ********************************
	dwTime += (DWORD)ceil( sqrt ( pFirstCheckPointPosition->GetDistanceSquare ( 
		*pSecondCheckPointPosition ) ) * (double)100 / 
		(double)pData->m_dwSpeed );
	// nastav� druh� kontroln� bod jednotky
	pUnit->SetSecondCheckPoint ( *pSecondCheckPointPosition, dwTime );

	// nastav� p��znak zm�ny kontroln�ch bod� jednotky
	pUnit->SetCheckPointInfoModified ();

	// vr�t� p��znak nepr�zdn� cesty
	return TRUE;
}

// p�iprav� data skilly na p�esun k prvn�mu kontroln�mu bodu jednotky (jednotka mus� b�t 
//		zam�ena pro z�pis)
void CSMoveSkillType::GoToFirstCheckPoint ( CSUnit *pUnit ) 
{
	SSMoveSkill *pData = SKILLDATA ( pUnit );

	// zjist� vzd�lenost prvn�ho kontroln�ho bodu od pozice jednotky
	int nXDifference = pUnit->GetFirstCheckPoint ().GetPosition ().x - 
		pUnit->GetPosition ().x;
	int nYDifference = pUnit->GetFirstCheckPoint ().GetPosition ().y - 
		pUnit->GetPosition ().y;

	// zjist� �as na p�esun k prvn�mu kontroln�mu bodu od pozice jednotky
	double dbTime = (double)( sqrt ( pUnit->GetPosition ().GetDistanceSquare ( 
		pUnit->GetFirstCheckPoint ().GetPosition () ) ) * (double)100 ) / 
		(double)pData->m_dwSpeed;

	// zjist�, je-li jednotka ji� na prvn�m kontroln�m bod�
	if ( dbTime == 0 )
	{	// jednotka je ji� na prvn�m kontroln�m bod�
		// vynuluje zm�nu sou�adnic za TimeSlice
		pData->m_dbXStep = pData->m_dbYStep = 0;
	}
	else
	{	// jednotka dosud nen� na prvn�m kontroln�m bod�
		// spo��t� zm�nu sou�adnic za TimeSlice
		pData->m_dbXStep = (double)nXDifference / dbTime;
		pData->m_dbYStep = (double)nYDifference / dbTime;

		// aktualizuje sm�r pohybu jednotky
		pUnit->SetDirectionFromDifference ( nXDifference, nYDifference );
	}
}

/////////////////////////////////////////////////////////////////////
// Interface vol�n� ze skript�
/////////////////////////////////////////////////////////////////////

// aktivuje skillu pro pohyb jednotky na pozici se sou�adnicemi "pX" a "pY" (jednotka je 
//		zam�ena pro z�pis)
ESyscallResult CSMoveSkillType::SCI_Go ( CSUnit *pUnit, CIProcess *pProcess, 
	CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, int *pX, int *pY ) 
{
	// c�lov� pozice
	CPointDW pointDestination ( (DWORD)*pX, (DWORD)*pY );

	TRACE_SKILLCALL3 ( "@ Move.Go ( %d, %d ) called on unit %d ", pointDestination.x, pointDestination.y, pUnit->GetID () );
	TRACE_SKILLCALL2 ( "from position ( %d, %d )\n", pUnit->GetPosition ().x, pUnit->GetPosition ().y );

	// zjist�, jedn�-li se o pozici na map�
	if ( ( pointDestination.x < g_cMap.GetMapSizeMapCell ().cx ) && 
		( pointDestination.y < g_cMap.GetMapSizeMapCell ().cy ) )
	{	// jedn� se o pozici na map�
		// vytvo�� nov� syst�mov� vol�n�
		CISysCallMove *pSysCall = new CISysCallMove;
		VERIFY ( pSysCall->Create ( pProcess, pUnit, this, pBag ) );

		// vytvo�� nov� objekt cesty
		CSPath *pPath = CSPath::FindPath ( pUnit->GetPosition (), pointDestination, 
			pUnit->GetFindPathGraph (), pUnit->GetMoveWidth (), 
			pUnit->GetCivilizationIndex () );

		// aktivuje skillu
		ActivateSkill ( pUnit, pPath, pSysCall, pointDestination, 0 );

		// nastav� ukazatel na syst�mov� vol�n� (reference pro n�j zv��ena v Create)
		*ppSysCall = pSysCall;

		// vr�t� p��znak usp�n� procesu
		return SYSCALL_SUCCESS_SLEEP;
	}

	TRACE_SKILLCALL2 ( "@ Move.Go failed on unit %d (returning %d\n", pUnit->GetID (), EGR_NoMapDestination );

	// vr�t� p��znak chybn� c�lov� pozice
	pBag->ReturnInt ( EGR_NoMapDestination );

	// vr�t� p��znak �sp�chu
	return SYSCALL_SUCCESS;
}

// vr�t� rychlost jednotky (jednotka je zam�ena pro �ten�)
ESyscallResult CSMoveSkillType::SCI_GetSpeed ( CSUnit *pUnit, CIProcess *pProcess, 
	CISyscall **ppSysCall, CIBag *pBag, bool bCalledByEnemy, CIDataStackItem *pReturn ) 
{
	// rychlost jednotky
	DWORD dwSpeed = SKILLDATA ( pUnit )->m_dwSpeed;

	TRACE_SKILLCALL1 ( "@ Move.GetSpeed () called, returning %d\n", dwSpeed );

	// vr�t� rychlost jednotky
	pBag->ReturnInt ( (int)dwSpeed );

	// vr�t� p��znak �sp�chu
	return SYSCALL_SUCCESS;
}

// aktivuje skillu pro pohyb jednotky na pozici "pPosition" (jednotka je zam�ena pro z�pis)
ESyscallResult CSMoveSkillType::SCI_GoPos ( CSUnit *pUnit, CIProcess *pProcess, 
	CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, 
	CISystemObject *pPosition ) 
{
	ASSERT ( pPosition->GetObjectType () == SO_POSITION );

	// c�lov� pozice
	CPointDW pointDestination ( (DWORD)((CISOPosition *)pPosition)->GetXPosition (), 
		(DWORD)((CISOPosition *)pPosition)->GetYPosition () );

	TRACE_SKILLCALL3 ( "@ Move.GoPos ( %d, %d ) called on unit %d ", pointDestination.x, pointDestination.y, pUnit->GetID () );
	TRACE_SKILLCALL2 ( "from position ( %d, %d )\n", pUnit->GetPosition ().x, pUnit->GetPosition ().y );

	// zjist�, jedn�-li se o pozici na map�
	if ( ( pointDestination.x < g_cMap.GetMapSizeMapCell ().cx ) && 
		( pointDestination.y < g_cMap.GetMapSizeMapCell ().cy ) )
	{	// jedn� se o pozici na map�
		// vytvo�� nov� syst�mov� vol�n�
		CISysCallMove *pSysCall = new CISysCallMove;
		VERIFY ( pSysCall->Create ( pProcess, pUnit, this, pBag ) );

		// vytvo�� nov� objekt cesty
		CSPath *pPath = CSPath::FindPath ( pUnit->GetPosition (), pointDestination, 
			pUnit->GetFindPathGraph (), pUnit->GetMoveWidth (), 
			pUnit->GetCivilizationIndex () );

		// aktivuje skillu
		ActivateSkill ( pUnit, pPath, pSysCall, pointDestination, 0 );

		// nastav� ukazatel na syst�mov� vol�n� (reference pro n�j zv��ena v Create)
		*ppSysCall = pSysCall;

		// vr�t� p��znak usp�n� procesu
		return SYSCALL_SUCCESS_SLEEP;
	}

	TRACE_SKILLCALL2 ( "@ Move.GoPos failed on unit %d (returning %d)\n", pUnit->GetID (), EGR_NoMapDestination );

	// vr�t� p��znak chybn� c�lov� pozice
	pBag->ReturnInt ( EGR_NoMapDestination );

	// vr�t� p��znak �sp�chu
	return SYSCALL_SUCCESS;
}

// aktivuje skillu pro pohyb jednotky na pozici se sou�adnicemi "pX" a "pY", s toleranc� 
//		"pTolerance" (jednotka je zam�ena pro z�pis)
ESyscallResult CSMoveSkillType::SCI_GoCloseTo ( CSUnit *pUnit, CIProcess *pProcess, 
	CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, int *pX, int *pY, 
	int *pTolerance ) 
{
	// c�lov� pozice
	CPointDW pointDestination ( (DWORD)*pX, (DWORD)*pY );

	// zkontroluje parametr tolerance vzd�lenosti od c�lov� pozice
	if ( *pTolerance > MAX_GO_CLOSE_TO_TOLERANCE )
	{	// parametr tolerance vzd�lenosti od c�lov� pozice je p��li� velk�
		// zahl�s� chybu parametru tolerance
		pProcess->m_pInterpret->ReportError ( "Move.GoCloseTo: Tolerance too large." );
		// oprav� parametr tolerance
		*pTolerance = MAX_GO_CLOSE_TO_TOLERANCE;
	}
	else if ( *pTolerance < 0 )
	{	// parametr tolerance vzd�lenosti od c�lov� pozice je z�porn�
		// zahl�s� chybu parametru tolerance
		pProcess->m_pInterpret->ReportError ( "Move.GoCloseTo: Tolerance negative." );
		// oprav� parametr tolerance
		*pTolerance = 0;
	}

	TRACE_SKILLCALL ( "@ Move.GoCloseTo ( %d, %d ) with tolerance %d called on unit %d ", pointDestination.x, pointDestination.y, *pTolerance, pUnit->GetID () );
	TRACE_SKILLCALL2 ( "from position ( %d, %d )\n", pUnit->GetPosition ().x, pUnit->GetPosition ().y );

	// zjist�, jedn�-li se o pozici na map�
	if ( ( pointDestination.x < g_cMap.GetMapSizeMapCell ().cx ) && 
		( pointDestination.y < g_cMap.GetMapSizeMapCell ().cy ) )
	{	// jedn� se o pozici na map�
		// vytvo�� nov� syst�mov� vol�n�
		CISysCallMove *pSysCall = new CISysCallMove;
		VERIFY ( pSysCall->Create ( pProcess, pUnit, this, pBag ) );

		// vytvo�� nov� objekt cesty
		CSPath *pPath = CSPath::FindPath ( pUnit->GetPosition (), pointDestination, 
			pUnit->GetFindPathGraph (), pUnit->GetMoveWidth (), 
			pUnit->GetCivilizationIndex () );

		// aktivuje skillu
		ActivateSkill ( pUnit, pPath, pSysCall, pointDestination, ((DWORD)*pTolerance) * 
			((DWORD)*pTolerance) + 1 );

		// nastav� ukazatel na syst�mov� vol�n� (reference pro n�j zv��ena v Create)
		*ppSysCall = pSysCall;

		// vr�t� p��znak usp�n� procesu
		return SYSCALL_SUCCESS_SLEEP;
	}

	TRACE_SKILLCALL2 ( "@ Move.GoCloseTo failed on unit %d (returning %d)\n", pUnit->GetID (), EGR_NoMapDestination );

	// vr�t� p��znak chybn� c�lov� pozice
	pBag->ReturnInt ( EGR_NoMapDestination );

	// vr�t� p��znak �sp�chu
	return SYSCALL_SUCCESS;
}

// aktivuje skillu pro pohyb jednotky na pozici "pPosition" s toleranc� "pTolerance" 
//		(jednotka je zam�ena pro z�pis)
ESyscallResult CSMoveSkillType::SCI_GoCloseToPos ( CSUnit *pUnit, CIProcess *pProcess, 
	CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, 
	CISystemObject *pPosition, int *pTolerance ) 
{
	ASSERT ( pPosition->GetObjectType () == SO_POSITION );

	// c�lov� pozice
	CPointDW pointDestination ( (DWORD)((CISOPosition *)pPosition)->GetXPosition (), 
		(DWORD)((CISOPosition *)pPosition)->GetYPosition () );

	// zkontroluje parametr tolerance vzd�lenosti od c�lov� pozice
	if ( *pTolerance > MAX_GO_CLOSE_TO_TOLERANCE )
	{	// parametr tolerance vzd�lenosti od c�lov� pozice je p��li� velk�
		// zahl�s� chybu parametru tolerance
		pProcess->m_pInterpret->ReportError ( "Move.GoCloseToPos: Tolerance too large." );
		// oprav� parametr tolerance
		*pTolerance = MAX_GO_CLOSE_TO_TOLERANCE;
	}
	else if ( *pTolerance < 0 )
	{	// parametr tolerance vzd�lenosti od c�lov� pozice je z�porn�
		// zahl�s� chybu parametru tolerance
		pProcess->m_pInterpret->ReportError ( "Move.GoCloseToPos: Tolerance negative." );
		// oprav� parametr tolerance
		*pTolerance = 0;
	}

	TRACE_SKILLCALL ( "@ Move.GoCloseToPos ( %d, %d ) with tolerance %d called on unit %d ", pointDestination.x, pointDestination.y, *pTolerance, pUnit->GetID () );
	TRACE_SKILLCALL2 ( "from position ( %d, %d )\n", pUnit->GetPosition ().x, pUnit->GetPosition ().y );

	// zjist�, jedn�-li se o pozici na map�
	if ( ( pointDestination.x < g_cMap.GetMapSizeMapCell ().cx ) && 
		( pointDestination.y < g_cMap.GetMapSizeMapCell ().cy ) )
	{	// jedn� se o pozici na map�
		// vytvo�� nov� syst�mov� vol�n�
		CISysCallMove *pSysCall = new CISysCallMove;
		VERIFY ( pSysCall->Create ( pProcess, pUnit, this, pBag ) );

		// vytvo�� nov� objekt cesty
		CSPath *pPath = CSPath::FindPath ( pUnit->GetPosition (), pointDestination, 
			pUnit->GetFindPathGraph (), pUnit->GetMoveWidth (), 
			pUnit->GetCivilizationIndex () );

		// aktivuje skillu
		ActivateSkill ( pUnit, pPath, pSysCall, pointDestination, ((DWORD)*pTolerance) * 
			((DWORD)*pTolerance) + 1 );

		// nastav� ukazatel na syst�mov� vol�n� (reference pro n�j zv��ena v Create)
		*ppSysCall = pSysCall;

		// vr�t� p��znak usp�n� procesu
		return SYSCALL_SUCCESS_SLEEP;
	}

	TRACE_SKILLCALL2 ( "@ Move.GoCloseToPos failed on unit %d (returning %d)\n", pUnit->GetID (), EGR_NoMapDestination );

	// vr�t� p��znak chybn� c�lov� pozice
	pBag->ReturnInt ( EGR_NoMapDestination );

	// vr�t� p��znak �sp�chu
	return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// Debuggovac� informace
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// zkontroluje ne�kodn� data skilly (TRUE=OK)
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
