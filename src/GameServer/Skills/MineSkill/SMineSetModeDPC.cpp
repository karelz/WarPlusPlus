/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry
 *   Autor: Roman Zenka
 * 
 *   Popis: DPC pro nastavovani modu pro Vitka
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SMineSkillType.h"
#include "SMineSetModeDPC.h"

#include "GameServer\GameServer\SDeferredProcedureCall.inl"
#include "GameServer\Skills\SysCalls\IDPCSysCall.h"
#include "TripleS\Interpret\Src\SysObjs\ISOSet.h"
#include "GameServer\GameServer\SUnitList.h"
#include "GameServer\Civilization\ZCivilization.h"

IMPLEMENT_DYNAMIC(CSMineSetModeDPC, CSSysCallDPC)
IMPLEMENT_DPC(CSMineSetModeDPC, 3)

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSMineSetModeDPC::CSMineSetModeDPC() :
    m_pSkillType(NULL),
    m_nMode(0)
{
}

// konstruktor
CSMineSetModeDPC::CSMineSetModeDPC(CIProcess *pProcess, CIDataStackItem *pReturnValue, CSUnit *pUnit, CSMineSkillType *pSkillType, int nMode) :
    CSSysCallDPC(pProcess, pReturnValue, pUnit)
{
    ASSERT(pSkillType != NULL);
    ASSERT_KINDOF(CSSkillType, pSkillType);	

    m_pSkillType = pSkillType;
	m_nMode = nMode;
}

// destruktor
CSMineSetModeDPC::~CSMineSetModeDPC() 
{
}


//////////////////////////////////////////////////////////////////////
// Ukladani dat DPC
//////////////////////////////////////////////////////////////////////

// ulozeni dat DPC
void CSMineSetModeDPC::PersistentSaveDPC(CPersistentStorage &storage)
{
    ASSERT_VALID(this);

    BRACE_BLOCK(storage);    

    // ulozime data predka
    CSSysCallDPC::PersistentSaveDPC(storage);

    // ulozime svoje data
    storage << m_pSkillType << m_nMode;
}

// nahravani ulozenych dat DPC
void CSMineSetModeDPC::PersistentLoadDPC(CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    
    // prvne natahneme data predka
    CSSysCallDPC::PersistentLoadDPC(storage);

    // natahneme svoje vlastni data
    storage >> (void *&)m_pSkillType >> (int&)m_nMode;
    LOAD_ASSERT(m_pSkillType != NULL);
}

// preklad ukazatelu
void CSMineSetModeDPC::PersistentTranslatePointersDPC(CPersistentStorage &storage)
{
    // zavolame predka
    CSSysCallDPC::PersistentTranslatePointersDPC(storage);

    // prelozime si svoje pointery
    m_pSkillType = (CSMineSkillType *) storage.TranslatePointer(m_pSkillType);

    // overime spravnost prelozenych pointeru
    LOAD_ASSERT(m_pSkillType->IsKindOf(RUNTIME_CLASS(CSMineSkillType)));
}

// inicializace nahraneho DPC
void CSMineSetModeDPC::PersistentInitDPC() 
{
    // zavolame predka
    CSSysCallDPC::PersistentInitDPC();

    // sami nedelame nic...
}


//////////////////////////////////////////////////////////////////////
// Provedeni odlozeneho volani procedury
//////////////////////////////////////////////////////////////////////

void CSMineSetModeDPC::Run() 
{
    // zamkne systémové volání
    VERIFY (m_pDPCSysCall->Lock());
	
    // zjistí, je-li systémové volání platné
    if ( m_pDPCSysCall->IsValid () )
    {	// systémové volání je platné
    
        // Set new mode of the unit
        GetUnit()->SetMode ( m_nMode );
    }

    // odemkne systémové volání
    VERIFY (m_pDPCSysCall->Unlock());

    m_pDPCSysCall->FinishWaiting(STATUS_SUCCESS);
    m_pDPCSysCall->Release();
    m_pDPCSysCall = NULL;
}


//////////////////////////////////////////////////////////////////////
// Debugovaci metody
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

void CSMineSetModeDPC::AssertValid()
{
    // zavolame predka
    CSSysCallDPC::AssertValid();

    // assertujeme vlastni data
    ASSERT(m_pSkillType != NULL);
    ASSERT(m_pSkillType->IsKindOf(RUNTIME_CLASS(CSMineSkillType)));
}

#endif  // _DEBUG
