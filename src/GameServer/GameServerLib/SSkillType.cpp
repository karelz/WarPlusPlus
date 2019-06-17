/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Abstraktn� t��da typu skilly jednotky
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

// implementace t��dy, jej� potomky lze vytv��et za b�hu programu z ID a ze jm�na 
//		potomka
IMPLEMENT_RUN_TIME_ID_NAME_STORAGE ( CSSkillType )

IMPLEMENT_DYNAMIC ( CSSkillType, CPersistentObject )

// implementace interfacu vol�n� ze skript�
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
	// zne�kodn� data typu skilly
	m_pUnitType = NULL;
	m_dwIndex = DWORD_MAX;
	m_dwSkillDataOffset = DWORD_MAX;
}

// destruktor
CSSkillType::~CSSkillType () 
{
	// zkontroluje ne�kodn� data typu skilly
	ASSERT ( m_pUnitType == NULL );
	ASSERT ( m_dwIndex == DWORD_MAX );
	ASSERT ( m_dwSkillDataOffset == DWORD_MAX );
}

// vytvo�� typ skilly s ID "dwID" (NULL=nezn�m� ID) s offsetem dat skilly 
//		"dwSkillDataOffset" a indexem "dwIndex" v typu jednotky "pUnitType"
CSSkillType *CSSkillType::Create ( DWORD dwID, DWORD dwSkillDataOffset, DWORD dwIndex, 
		CSUnitType *pUnitType ) 
{
	// zkontroluje platnost parametr�
	ASSERT ( pUnitType != NULL );
	ASSERT ( dwIndex < pUnitType->GetSkillCount () );

	// nech� vytvo�it typ skilly z ID "dwID"
	CSSkillType *pSkillType = CreateChildByID ( dwID );
	// zjist�, bylo-li ID platn�
	if ( pSkillType == NULL )
	{	// ID nebylo platn�
		// vr�t� p��znak neplatn�ho ID
		return NULL;
	}

	// inicializuje data typu skilly
	pSkillType->m_dwIndex = dwIndex;
	pSkillType->m_pUnitType = pUnitType;
	pSkillType->m_dwSkillDataOffset = dwSkillDataOffset;

	// vr�t� nov� vytvo�en� typ skilly
	return pSkillType;
}

// vytvo�� typ skilly se jm�nem "lpcszName" (NULL=nezn�m� jm�no) s offsetem dat 
//		skilly "dwSkillDataOffset" a indexem "dwIndex" v typu jednotky "pUnitType"
CSSkillType *CSSkillType::Create ( LPCTSTR lpcszName, DWORD dwSkillDataOffset, 
	DWORD dwIndex, CSUnitType *pUnitType ) 
{
	// zkontroluje platnost parametr�
	ASSERT ( lpcszName != NULL );
	ASSERT ( ( strlen ( lpcszName ) > 0 ) && ( strlen ( lpcszName ) <= 30 ) );
	ASSERT ( pUnitType != NULL );
	ASSERT ( dwIndex < pUnitType->GetSkillCount () );

	// nech� vytvo�it typ skilly ze jm�na "lpcszName"
	CSSkillType *pSkillType = CreateChildByName ( lpcszName );
	// zjist�, bylo-li jm�no platn�
	if ( pSkillType == NULL )
	{	// jm�no nebylo platn�
		// vr�t� p��znak neplatn�ho jm�na
		return NULL;
	}

	// inicializuje data typu skilly
	pSkillType->m_dwIndex = dwIndex;
	pSkillType->m_pUnitType = pUnitType;
	pSkillType->m_dwSkillDataOffset = dwSkillDataOffset;

	// vr�t� nov� vytvo�en� typ skilly
	return pSkillType;
}

//////////////////////////////////////////////////////////////////////
// Inicializace a zni�en� dat objektu
//////////////////////////////////////////////////////////////////////

// inicializuje data typu skilly
void CSSkillType::Create ( void *pData, DWORD dwDataSize ) 
{
	ASSERT ( ( ( dwDataSize == 0 ) && ( pData == NULL ) ) || 
		( ( dwDataSize != 0 ) && ( pData != NULL ) ) );

	// zkontroluje ne�kodn� data typu skilly
	ASSERT ( m_pUnitType != NULL );
	ASSERT ( m_dwIndex < m_pUnitType->GetSkillCount () );

	// inicilizuje data potomka t��dy
	CreateSkillType ( pData, dwDataSize );
}

// zni�� data typu skilly
void CSSkillType::Delete () 
{
	// zkontroluje platnost dat typu skilly
	ASSERT ( m_pUnitType != NULL );
	ASSERT ( m_dwIndex < m_pUnitType->GetSkillCount () );

	// zni�� data potomka t��dy
	DeleteSkillType ();

	// zne�kodn� data typu skilly
	m_pUnitType = NULL;
	m_dwIndex = DWORD_MAX;
	m_dwSkillDataOffset = DWORD_MAX;
}

//////////////////////////////////////////////////////////////////////
// Ukl�d�n� dat (CPersistentObject metody)
//////////////////////////////////////////////////////////////////////

// ukl�d�n� dat
void CSSkillType::PersistentSave ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	// zkontroluje platnost dat typu skilly
	ASSERT ( m_pUnitType != NULL );
	ASSERT ( m_dwIndex < m_pUnitType->GetSkillCount () );

	// ulo�� data potomka
	PersistentSaveSkillType ( storage );
}

// nahr�v�n� pouze ulo�en�ch dat
void CSSkillType::PersistentLoad ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	// zkontroluje platnost dat typu skilly
	ASSERT ( m_pUnitType != NULL );
	ASSERT ( m_dwIndex < m_pUnitType->GetSkillCount () );

	// nahraje data potomka
	PersistentLoadSkillType ( storage );
}

// p�eklad ukazatel�
void CSSkillType::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	// p�elo�� ukazatele dat potomka
	PersistentTranslatePointersSkillType ( storage );
}

// inicializace nahran�ho objektu
void CSSkillType::PersistentInit () 
{
	// inicializuje data potomka
	PersistentInitSkillType ();
}

//////////////////////////////////////////////////////////////////////
// Metody skilly
//////////////////////////////////////////////////////////////////////

// zjist�, je-li typ skilly povolen (jednotka mus� b�t zam�ena alespo� pro read)
BOOL CSSkillType::IsEnabled ( CSUnit *pUnit ) 
{
	// zjist� ukazatel na popis typu skilly
	CSUnitType::SSkillTypeDescription *pSkillTypeDescription = 
		GetUnitType ()->m_aSkillTypeDescription + GetIndex ();

	// vr�t� p��znak, je-li jednotka v aktu�ln�m m�du povolena
	return pSkillTypeDescription->bEnabled && ( 
		pSkillTypeDescription->nAllowedModesMask && pUnit->GetModeMask () );
}

// zjist�, je-li skilla aktivn� (jednotka mus� b�t zam�ena alespo� pro read)
BOOL CSSkillType::IsSkillActive ( CSUnit *pUnit ) 
{
	// vr�t� p��znak, je-li skilla aktivn�
	return pUnit->IsSkillActive ( GetIndex () );
}

//////////////////////////////////////////////////////////////////////
// Interface vol�n� ze skript�
//////////////////////////////////////////////////////////////////////

// vr�t� p��znak, je-li skilla aktivn�
ESyscallResult CSSkillType::SCI_IsActive ( CSUnit *pUnit, CIProcess *pProcess, 
	CISyscall **ppSyscall, CIBag *pBag, CIDataStackItem *pReturn ) 
{
	pReturn->Set ( IsSkillActive ( pUnit ) != FALSE );
	return SYSCALL_SUCCESS;
}

// vr�t� jm�no typu skilly
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
