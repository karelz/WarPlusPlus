// ISkillInterface.cpp: implementation of the CISkillInterface class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\Interpret.h"
#include "ISkillInterface.h"

#include "GameServer\GameServer\SSkillType.h"
#include "GameServer\Civilization\Src\ZUnit.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DEFINE_MULTITHREAD_POOL( CISkillInterface, 100);

CISkillInterface::CISkillInterface()
{
    m_nSOType = SO_SKILLINTERFACE;
}

CISkillInterface::~CISkillInterface()
{
}

bool CISkillInterface::Create( CInterpret *pInterpret, CZUnit *pZUnit, CSSkillType *pSkillType)
{
	ASSERT( pZUnit != NULL);
	ASSERT( pSkillType != NULL);
	ASSERT( pZUnit->m_pSUnit != NULL);

    if (!CISystemObject::Create( pInterpret)) return false;

    m_pZUnit = pZUnit;
    m_pSkillType = pSkillType;

    return true;
}

void CISkillInterface::Delete()
{
}

ESyscallResult CISkillInterface::InvokeMethod( CIProcess *pCallingProcess, long nMethodID, 
                                                CIBag *pBag, CISyscall **ppSyscall)
{
    ASSERT( m_pSkillType != NULL);
    ASSERT( m_pZUnit != NULL);
	ASSERT( m_pZUnit->m_pSUnit != NULL);

	bool bCalledByEnemy = (pCallingProcess->m_pInterpret->m_pCivilization != m_pZUnit->m_pCivilization);

    return m_pSkillType->InvokeMethod( m_pZUnit->m_pSUnit, pCallingProcess, nMethodID, pBag, 
		ppSyscall, bCalledByEnemy);
}

void CISkillInterface::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    
    CISystemObject::PersistentSave( storage);
    
    storage << (DWORD)m_pSkillType;
    storage << (DWORD)m_pZUnit;
}

void CISkillInterface::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    CISystemObject::PersistentLoad( storage);
    
    storage >> (DWORD&)m_pSkillType;
    storage >> (DWORD&)m_pZUnit;
}

void CISkillInterface::PersistentTranslatePointers( CPersistentStorage &storage)
{
	m_pSkillType = (CSSkillType*)storage.TranslatePointer( m_pSkillType);
	m_pZUnit = (CZUnit*) storage.TranslatePointer( m_pZUnit);
    CISystemObject::PersistentTranslatePointers( storage);
}

void CISkillInterface::PersistentInit()
{
    CISystemObject::PersistentInit();
}
