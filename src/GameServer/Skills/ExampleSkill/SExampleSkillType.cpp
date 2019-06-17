/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry - skilly
 *   Autor: Karel Zikmund
 * 
 *   Popis: P��klad typu skilly
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SExampleSkillType.h"
#include "SExampleSkill.h"

#include "GameServer\GameServer\SMap.h"
#include "GameServer\GameServer\SCivilization.h"

#define SKILLDATA(pUnit) (SSExampleSkill *)GetSkillData(pUnit)

#define WAITING_TIMEOUT 3

// implementace interfacu vol�n� ze skript�
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
// Inicializace a zni�en� dat objektu
/////////////////////////////////////////////////////////////////////

// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
//		v�jimky: CPersistentLoadException
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

// zni�� data typu skilly
void CSExampleSkillType::DeleteSkillType () 
{
	ASSERT ( m_dwWaitCount != DWORD_MAX );
	ASSERT ( m_nOutputUseCount != -1 );

	m_dwWaitCount = DWORD_MAX;
	m_nOutputUseCount = -1;
}

/////////////////////////////////////////////////////////////////////
// Ukl�d�n� dat potomk� (analogie CPersistentObject metod)
/////////////////////////////////////////////////////////////////////

// ukl�d�n� dat potomka
void CSExampleSkillType::PersistentSaveSkillType ( CPersistentStorage &storage ) 
{
    BRACE_BLOCK(storage);

    storage << m_dwWaitCount;
	storage << m_nOutputUseCount;
	storage.Write ( m_szDescription, sizeof ( m_szDescription ) );
}

// nahr�v�n� pouze ulo�en�ch dat potomka
void CSExampleSkillType::PersistentLoadSkillType ( CPersistentStorage &storage ) 
{
    BRACE_BLOCK(storage);

	storage >> m_dwWaitCount;
	storage >> m_nOutputUseCount;
	storage.Read ( m_szDescription, sizeof ( m_szDescription ) );
	LOAD_ASSERT ( strlen ( m_szDescription ) < sizeof ( m_szDescription ) );
}

// p�eklad ukazatel� potomka
void CSExampleSkillType::PersistentTranslatePointersSkillType ( CPersistentStorage &storage ) 
{
}

// inicializace nahran�ho objektu potomka
void CSExampleSkillType::PersistentInitSkillType () 
{
}

/////////////////////////////////////////////////////////////////////
// Get/Set metody serverov�ch informac� o typu skilly jednotky
/////////////////////////////////////////////////////////////////////

// vr�t� velikost dat skilly pro jednotku
DWORD CSExampleSkillType::GetSkillDataSize () 
{
	return sizeof ( SSExampleSkill );
}

// vr�t� masku skupin skill
DWORD CSExampleSkillType::GetGroupMask () 
{
	return Group_None;
}

/////////////////////////////////////////////////////////////////////
// Metody skilly
/////////////////////////////////////////////////////////////////////

// provede TimeSlice na skille jednotky "pUnit"
// jednotka "pUnit" je ji� zam�en� pro z�pis, metoda m��e zamykat libovoln� po�et jin�ch
//		jednotek pro z�pis/�ten� (v�etn� jednotek ze sv�ho bloku)
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
	// zjist�, m�-li se skilla prov�st
	if ( pData->m_dwWaitCount > 0 )
	{	// skilla se je�t� nem� prov�st
		// sn�� dobu �ek�n�
		pData->m_dwWaitCount--;
		// ukon�� prov�d�d�n� TimeSlicu
		return;
	}

	// provede skillu
	m_nOutputUseCount++;
	TRACE_SKILL2 ( "@ CSExampleSkill BODY doing on unit %d with value %d\n", pUnit->GetID (), m_nOutputUseCount );
	// nastav� novou dobu �ek�n�
	pData->m_dwWaitCount = m_dwWaitCount;*/
}

// inicializuje skillu jednotky "pUnit"
void CSExampleSkillType::InitializeSkill ( CSUnit *pUnit ) 
{
	SSExampleSkill *pData = SKILLDATA ( pUnit );

	// znehodnot� data skilly
	pData->m_dwWaitCount = DWORD_MAX;
}

// zni�� skillu jednotky "pUnit"
void CSExampleSkillType::DestructSkill ( CSUnit *pUnit ) 
{
	SSExampleSkill *pData = SKILLDATA ( pUnit );
}

// aktivuje skillu jednotky "pUnit" (jednotka mus� b�t zam�ena pro z�pis)
void CSExampleSkillType::ActivateSkill ( CSUnit *pUnit ) 
{
	SSExampleSkill *pData = SKILLDATA ( pUnit );

	// zjist�, je-li skilla ji� aktivn�
	if ( IsSkillActive ( pUnit ) )
	{	// skilla je aktivn�
		// nejprve se deaktivuje
		DeactivateSkill ( pUnit );
	}

	// nastav� dobu �ek�n�
	pData->m_dwWaitCount = 0;

	// aktivuje skillu na jednotce
	CSSkillType::ActivateSkill ( pUnit );
}

// deaktivuje skillu jednotky "pUnit" (jednotka mus� b�t zam�ena pro z�pis)
void CSExampleSkillType::DeactivateSkill ( CSUnit *pUnit ) 
{
	SSExampleSkill *pData = SKILLDATA ( pUnit );

	// znehodnot� data skilly
	pData->m_dwWaitCount = DWORD_MAX;

	// deaktivuje skillu na jednotce
	CSSkillType::DeactivateSkill ( pUnit );
}

// ukl�d�n� dat skilly
void CSExampleSkillType::PersistentSaveSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
    BRACE_BLOCK(storage);
    
    SSExampleSkill *pData = SKILLDATA ( pUnit );

	storage << pData->m_dwWaitCount;
}

// nahr�v�n� pouze ulo�en�ch dat skilly
void CSExampleSkillType::PersistentLoadSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
    BRACE_BLOCK(storage);

	SSExampleSkill *pData = SKILLDATA ( pUnit );

	storage >> pData->m_dwWaitCount;
}

// p�eklad ukazatel� dat skilly
void CSExampleSkillType::PersistentTranslatePointersSkill ( CPersistentStorage &storage, CSUnit *pUnit ) 
{
}

// inicializace nahran�ch dat skilly
void CSExampleSkillType::PersistentInitSkill ( CSUnit *pUnit ) 
{
}

/////////////////////////////////////////////////////////////////////
// Interface vol�n� ze skript�
/////////////////////////////////////////////////////////////////////

// nastartuje skillu (jednotka je zam�eno pro z�pis)
ESyscallResult CSExampleSkillType::SCI_Start ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSyscall, CIBag *pBag, CIDataStackItem *pReturn, int *pParam ) 
{
	TRACE_SKILLCALL2 ( "@ Example.Start ( %d ) called on unit %d\n", *pParam, pUnit->GetID () );
	ActivateSkill ( pUnit );
	return SYSCALL_SUCCESS;
}

// vypne skillu (jednotka je zam�eno pro z�pis)
ESyscallResult CSExampleSkillType::SCI_Stop ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSyscall, CIBag *pBag, CIDataStackItem *pReturn, int *pParam ) 
{
	TRACE_SKILLCALL2 ( "@ Example.Stop ( %d ) called on unit %d\n", *pParam, pUnit->GetID () );
	DeactivateSkill ( pUnit );
	return SYSCALL_SUCCESS;
}
