/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Abstraktní tøída typu skilly jednotky
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SSkillType.h"

#include "SUnitType.h"

#include "TripleS\Interpret\Src\SysObjs\SystemObjects.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// implementace tøídy, jejíž potomky lze vytváøet za bìhu programu z ID a ze jména 
//		potomka
IMPLEMENT_RUN_TIME_ID_NAME_STORAGE ( CSSkillType )

IMPLEMENT_DYNAMIC ( CSSkillType, CPersistentObject )

// implementace interfacu volání ze skriptù
BEGIN_SKILLCALL_INTERFACE_MAP ( CSSkillType, CSSkillType )
	SKILLCALL_METHOD ( SCI_IsActive ),
	SKILLCALL_METHOD ( SCI_GetName )
END_SKILLCALL_INTERFACE_MAP ()

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSSkillType::CSSkillType () 
{
	// zneškodní data typu skilly
	m_pUnitType = NULL;
	m_dwIndex = DWORD_MAX;
	m_dwSkillDataOffset = DWORD_MAX;
}

// destruktor
CSSkillType::~CSSkillType () 
{
	// zkontroluje neškodná data typu skilly
	ASSERT ( m_pUnitType == NULL );
	ASSERT ( m_dwIndex == DWORD_MAX );
	ASSERT ( m_dwSkillDataOffset == DWORD_MAX );
}

// vytvoøí typ skilly s ID "dwID" (NULL=neznámé ID) s offsetem dat skilly 
//		"dwSkillDataOffset" a indexem "dwIndex" v typu jednotky "pUnitType"
CSSkillType *CSSkillType::Create ( DWORD dwID, DWORD dwSkillDataOffset, DWORD dwIndex, 
		CSUnitType *pUnitType ) 
{
	// zkontroluje platnost parametrù
	ASSERT ( pUnitType != NULL );
	ASSERT ( dwIndex < pUnitType->GetSkillCount () );

	// nechá vytvoøit typ skilly z ID "dwID"
	CSSkillType *pSkillType = CreateChildByID ( dwID );
	// zjistí, bylo-li ID platné
	if ( pSkillType == NULL )
	{	// ID nebylo platné
		// vrátí pøíznak neplatného ID
		return NULL;
	}

	// inicializuje data typu skilly
	pSkillType->m_dwIndex = dwIndex;
	pSkillType->m_pUnitType = pUnitType;
	pSkillType->m_dwSkillDataOffset = dwSkillDataOffset;

	// vrátí novì vytvoøený typ skilly
	return pSkillType;
}

// vytvoøí typ skilly se jménem "lpcszName" (NULL=neznámé jméno) s offsetem dat 
//		skilly "dwSkillDataOffset" a indexem "dwIndex" v typu jednotky "pUnitType"
CSSkillType *CSSkillType::Create ( LPCTSTR lpcszName, DWORD dwSkillDataOffset, 
	DWORD dwIndex, CSUnitType *pUnitType ) 
{
	// zkontroluje platnost parametrù
	ASSERT ( lpcszName != NULL );
	ASSERT ( ( strlen ( lpcszName ) > 0 ) && ( strlen ( lpcszName ) <= 30 ) );
	ASSERT ( pUnitType != NULL );
	ASSERT ( dwIndex < pUnitType->GetSkillCount () );

	// nechá vytvoøit typ skilly ze jména "lpcszName"
	CSSkillType *pSkillType = CreateChildByName ( lpcszName );
	// zjistí, bylo-li jméno platné
	if ( pSkillType == NULL )
	{	// jméno nebylo platné
		// vrátí pøíznak neplatného jména
		return NULL;
	}

	// inicializuje data typu skilly
	pSkillType->m_dwIndex = dwIndex;
	pSkillType->m_pUnitType = pUnitType;
	pSkillType->m_dwSkillDataOffset = dwSkillDataOffset;

	// vrátí novì vytvoøený typ skilly
	return pSkillType;
}

//////////////////////////////////////////////////////////////////////
// Inicializace a znièení dat objektu
//////////////////////////////////////////////////////////////////////

// inicializuje data typu skilly
void CSSkillType::Create ( void *pData, DWORD dwDataSize ) 
{
	ASSERT ( ( ( dwDataSize == 0 ) && ( pData == NULL ) ) || 
		( ( dwDataSize != 0 ) && ( pData != NULL ) ) );

	// zkontroluje neškodná data typu skilly
	ASSERT ( m_pUnitType != NULL );
	ASSERT ( m_dwIndex < m_pUnitType->GetSkillCount () );

	// inicilizuje data potomka tøídy
	CreateSkillType ( pData, dwDataSize );
}

// znièí data typu skilly
void CSSkillType::Delete () 
{
	// zkontroluje platnost dat typu skilly
	ASSERT ( m_pUnitType != NULL );
	ASSERT ( m_dwIndex < m_pUnitType->GetSkillCount () );

	// znièí data potomka tøídy
	DeleteSkillType ();

	// zneškodní data typu skilly
	m_pUnitType = NULL;
	m_dwIndex = DWORD_MAX;
	m_dwSkillDataOffset = DWORD_MAX;
}

//////////////////////////////////////////////////////////////////////
// Ukládání dat (CPersistentObject metody)
//////////////////////////////////////////////////////////////////////

// ukládání dat
void CSSkillType::PersistentSave ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	// zkontroluje platnost dat typu skilly
	ASSERT ( m_pUnitType != NULL );
	ASSERT ( m_dwIndex < m_pUnitType->GetSkillCount () );

	// uloží data potomka
	PersistentSaveSkillType ( storage );
}

// nahrávání pouze uložených dat
void CSSkillType::PersistentLoad ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	// zkontroluje platnost dat typu skilly
	ASSERT ( m_pUnitType != NULL );
	ASSERT ( m_dwIndex < m_pUnitType->GetSkillCount () );

	// nahraje data potomka
	PersistentLoadSkillType ( storage );
}

// pøeklad ukazatelù
void CSSkillType::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	// pøeloží ukazatele dat potomka
	PersistentTranslatePointersSkillType ( storage );
}

// inicializace nahraného objektu
void CSSkillType::PersistentInit () 
{
	// inicializuje data potomka
	PersistentInitSkillType ();
}

//////////////////////////////////////////////////////////////////////
// Metody skilly
//////////////////////////////////////////////////////////////////////

// zjistí, je-li typ skilly povolen (jednotka musí být zamèena alespoò pro read)
BOOL CSSkillType::IsEnabled ( CSUnit *pUnit ) 
{
	// zjistí ukazatel na popis typu skilly
	CSUnitType::SSkillTypeDescription *pSkillTypeDescription = 
		GetUnitType ()->m_aSkillTypeDescription + GetIndex ();

	// vrátí pøíznak, je-li jednotka v aktuálním módu povolena
	return pSkillTypeDescription->bEnabled && ( 
		pSkillTypeDescription->nAllowedModesMask && pUnit->GetModeMask () );
}

// zjistí, je-li skilla aktivní (jednotka musí být zamèena alespoò pro read)
BOOL CSSkillType::IsSkillActive ( CSUnit *pUnit ) 
{
	// vrátí pøíznak, je-li skilla aktivní
	return pUnit->IsSkillActive ( GetIndex () );
}

//////////////////////////////////////////////////////////////////////
// Interface volání ze skriptù
//////////////////////////////////////////////////////////////////////

// vrátí pøíznak, je-li skilla aktivní
ESyscallResult CSSkillType::SCI_IsActive ( CSUnit *pUnit, CIProcess *pProcess, 
	CISyscall **ppSyscall, CIBag *pBag, CIDataStackItem *pReturn ) 
{
	pReturn->Set ( IsSkillActive ( pUnit ) != FALSE );
	return SYSCALL_SUCCESS;
}

// vrátí jméno typu skilly
ESyscallResult CSSkillType::SCI_GetName ( CSUnit *pUnit, CIProcess *pProcess, 
	CISyscall **ppSyscall, CIBag *pBag, CIDataStackItem *pReturn ) 
{
	CISOString *pName = (CISOString *)pProcess->m_pInterpret->CreateSystemObject ( 
		SO_STRING );
	if ( pName == NULL ) return SYSCALL_ERROR;

	pName->Set ( GetName () );
	pReturn->Set ( pName );
	pName->Release ();

	return SYSCALL_SUCCESS;
}
