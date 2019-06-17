/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry - skilly
 *   Autor: Karel Zikmund
 * 
 *   Popis: Pøíklad typu skilly
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SExampleSkillType.h"
#include "SExampleSkill.h"

#include "GameServer\GameServer\SMap.h"
#include "GameServer\GameServer\SCivilization.h"

#define SKILLDATA(pUnit) (SSExampleSkill *)GetSkillData(pUnit)

#define WAITING_TIMEOUT 3

// implementace interfacu volání ze skriptù
BEGIN_SKILLCALL_INTERFACE_MAP ( CSExampleSkillType, CSSkillType )
	SKILLCALL_BASECLASS,
	SKILLCALL_BASECLASS,
	SKILLCALL_METHOD ( SCI_Start ),
	SKILLCALL_METHOD ( SCI_Stop )
END_SKILLCALL_INTERFACE_MAP ()

// implementace typu skilly
IMPLEMENT_SKILL_TYPE ( CSExampleSkillType, 97, "Example" );

/////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
/////////////////////////////////////////////////////////////////////

// konstruktor
CSExampleSkillType::CSExampleSkillType () 
{
	m_dwWaitCount = DWORD_MAX;
	m_nOutputUseCount = -1;
}

// destruktor
CSExampleSkillType::~CSExampleSkillType () 
{
	ASSERT ( m_dwWaitCount == DWORD_MAX );
	ASSERT ( m_nOutputUseCount == -1 );
}

/////////////////////////////////////////////////////////////////////
// Inicializace a znièení dat objektu
/////////////////////////////////////////////////////////////////////

// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
//		výjimky: CPersistentLoadException
void CSExampleSkillType::CreateSkillType ( void *pData, DWORD dwDataSize ) 
{
	LOAD_ASSERT ( dwDataSize == sizeof ( m_szDescription ) );

	ASSERT ( m_dwWaitCount == DWORD_MAX );
	ASSERT ( m_nOutputUseCount == -1 );

	m_dwWaitCount = WAITING_TIMEOUT;
	m_nOutputUseCount = 0;

	memcpy ( m_szDescription, pData, sizeof ( m_szDescription ) );
	LOAD_ASSERT ( strlen ( m_szDescription ) < sizeof ( m_szDescription ) );
}

// znièí data typu skilly
void CSExampleSkillType::DeleteSkillType () 
{
	ASSERT ( m_dwWaitCount != DWORD_MAX );
	ASSERT ( m_nOutputUseCount != -1 );

	m_dwWaitCount = DWORD_MAX;
	m_nOutputUseCount = -1;
}

/////////////////////////////////////////////////////////////////////
// Ukládání dat potomkù (analogie CPersistentObject metod)
/////////////////////////////////////////////////////////////////////

// ukládání dat potomka
void CSExampleSkillType::PersistentSaveSkillType ( CPersistentStorage &storage ) 
{
    BRACE_BLOCK(storage);

    storage << m_dwWaitCount;
	storage << m_nOutputUseCount;
	storage.Write ( m_szDescription, sizeof ( m_szDescription ) );
}

// nahrávání pouze uložených dat potomka
void CSExampleSkillType::PersistentLoadSkillType ( CPersistentStorage &storage ) 
{
    BRACE_BLOCK(storage);

	storage >> m_dwWaitCount;
	storage >> m_nOutputUseCount;
	storage.Read ( m_szDescription, sizeof ( m_szDescription ) );
	LOAD_ASSERT ( strlen ( m_szDescription ) < sizeof ( m_szDescription ) );
}

// pøeklad ukazatelù potomka
void CSExampleSkillType::PersistentTranslatePointersSkillType ( CPersistentStorage &storage ) 
{
}

// inicializace nahraného objektu potomka
void CSExampleSkillType::PersistentInitSkillType () 
{
}

/////////////////////////////////////////////////////////////////////
// Get/Set metody serverových informací o typu skilly jednotky
/////////////////////////////////////////////////////////////////////

// vrátí velikost dat skilly pro jednotku
DWORD CSExampleSkillType::GetSkillDataSize () 
{
	return sizeof ( SSExampleSkill );
}

// vrátí masku skupin skill
DWORD CSExampleSkillType::GetGroupMask () 
{
	return Group_None;
}

/////////////////////////////////////////////////////////////////////
// Metody skilly
/////////////////////////////////////////////////////////////////////

// provede TimeSlice na skille jednotky "pUnit"
// jednotka "pUnit" je již zamèená pro zápis, metoda mùže zamykat libovolný poèet jiných
//		jednotek pro zápis/ètení (vèetnì jednotek ze svého bloku)
void CSExampleSkillType::DoTimeSlice ( CSUnit *pUnit ) 
{
	SSExampleSkill *pData = SKILLDATA ( pUnit );
/*
	CSCivilization *pCivilization = GetUnitType ()->GetCivilization ();
	CReadWriteLock *pCivilizationLock = pCivilization->GetCivilizationLock ();
	int (* pResources)[RESOURCE_COUNT] = pCivilization->GetResources ();
	VERIFY ( pCivilizationLock->WriterLock () );
	int nIndex = g_cMap.GetTimeSlice () % 3;
	if ( nIndex == 0 )
		(*pResources)[0]--;
	else if ( nIndex == 1 )
		(*pResources)[1] -= 821;
	else
		(*pResources)[2] += 12910;
	TRACE_SKILL3 ( "@ CSExampleSkill BODY doing on unit %d, resource [%d] = %d\n", pUnit->GetID (), nIndex, (*pResources)[nIndex] );
	pCivilizationLock->WriterUnlock ();*/
/*
	// zjistí, má-li se skilla provést
	if ( pData->m_dwWaitCount > 0 )
	{	// skilla se ještì nemá provést
		// sníží dobu èekání
		pData->m_dwWaitCount--;
		// ukonèí provádádìní TimeSlicu
		return;
	}

	// provede skillu
	m_nOutputUseCount++;
	TRACE_SKILL2 ( "@ CSExampleSkill BODY doing on unit %d with value %d\n", pUnit->GetID (), m_nOutputUseCount );
	// nastaví novou dobu èekání
	pData->m_dwWaitCount = m_dwWaitCount;*/
}

// inicializuje skillu jednotky "pUnit"
void CSExampleSkillType::InitializeSkill ( CSUnit *pUnit ) 
{
	SSExampleSkill *pData = SKILLDATA ( pUnit );

	// znehodnotí data skilly
	pData->m_dwWaitCount = DWORD_MAX;
}

// znièí skillu jednotky "pUnit"
void CSExampleSkillType::DestructSkill ( CSUnit *pUnit ) 
{
	SSExampleSkill *pData = SKILLDATA ( pUnit );
}

// aktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
void CSExampleSkillType::ActivateSkill ( CSUnit *pUnit ) 
{
	SSExampleSkill *pData = SKILLDATA ( pUnit );

	// zjistí, je-li skilla již aktivní
	if ( IsSkillActive ( pUnit ) )
	{	// skilla je aktivní
		// nejprve se deaktivuje
		DeactivateSkill ( pUnit );
	}

	// nastaví dobu èekání
	pData->m_dwWaitCount = 0;

	// aktivuje skillu na jednotce
	CSSkillType::ActivateSkill ( pUnit );
}

// deaktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
void CSExampleSkillType::DeactivateSkill ( CSUnit *pUnit ) 
{
	SSExampleSkill *pData = SKILLDATA ( pUnit );

	// znehodnotí data skilly
	pData->m_dwWaitCount = DWORD_MAX;

	// deaktivuje skillu na jednotce
	CSSkillType::DeactivateSkill ( pUnit );
}

// ukládání dat skilly
void CSExampleSkillType::PersistentSaveSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
    BRACE_BLOCK(storage);
    
    SSExampleSkill *pData = SKILLDATA ( pUnit );

	storage << pData->m_dwWaitCount;
}

// nahrávání pouze uložených dat skilly
void CSExampleSkillType::PersistentLoadSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
    BRACE_BLOCK(storage);

	SSExampleSkill *pData = SKILLDATA ( pUnit );

	storage >> pData->m_dwWaitCount;
}

// pøeklad ukazatelù dat skilly
void CSExampleSkillType::PersistentTranslatePointersSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
}

// inicializace nahraných dat skilly
void CSExampleSkillType::PersistentInitSkill ( CSUnit *pUnit ) 
{
}

/////////////////////////////////////////////////////////////////////
// Interface volání ze skriptù
/////////////////////////////////////////////////////////////////////

// nastartuje skillu (jednotka je zamèeno pro zápis)
ESyscallResult CSExampleSkillType::SCI_Start ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSyscall, CIBag *pBag, CIDataStackItem *pReturn, int *pParam ) 
{
	TRACE_SKILLCALL2 ( "@ Example.Start ( %d ) called on unit %d\n", *pParam, pUnit->GetID () );
	ActivateSkill ( pUnit );
	return SYSCALL_SUCCESS;
}

// vypne skillu (jednotka je zamèeno pro zápis)
ESyscallResult CSExampleSkillType::SCI_Stop ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSyscall, CIBag *pBag, CIDataStackItem *pReturn, int *pParam ) 
{
	TRACE_SKILLCALL2 ( "@ Example.Stop ( %d ) called on unit %d\n", *pParam, pUnit->GetID () );
	DeactivateSkill ( pUnit );
	return SYSCALL_SUCCESS;
}
