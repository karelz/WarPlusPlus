/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry
 *   Autor: Tomas Karban
 * 
 *   Popis: DPC pro zjistovani nepratelskych jednotkek
 *          v dosahu moji zbrane
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SAbstractAttackSkillType.h"
#include "SCanAttackUnitDPC.h"

#include "GameServer\GameServer\SDeferredProcedureCall.inl"
#include "GameServer\Skills\SysCalls\IDPCSysCall.h"
#include "TripleS\Interpret\Src\SysObjs\ISOSet.h"
#include "GameServer\GameServer\SUnitList.h"
#include "GameServer\Civilization\ZCivilization.h"

IMPLEMENT_DYNAMIC(CSCanAttackUnitDPC, CSSysCallDPC)
IMPLEMENT_DPC(CSCanAttackUnitDPC, 2)

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSCanAttackUnitDPC::CSCanAttackUnitDPC() :
    m_pSkillType(NULL),
    m_pAttackableUnit(NULL)
{
}

// konstruktor
CSCanAttackUnitDPC::CSCanAttackUnitDPC(CIProcess *pProcess, CIDataStackItem *pReturnValue, CSUnit *pUnit, CSAbstractAttackSkillType *pSkillType, CSUnit *pAttackableUnit) :
    CSSysCallDPC(pProcess, pReturnValue, pUnit)
{
    ASSERT(pSkillType != NULL);
    ASSERT_KINDOF(CSSkillType, pSkillType);
    ASSERT(pAttackableUnit != NULL);
    ASSERT_KINDOF(CSUnit, pAttackableUnit);

    m_pSkillType = pSkillType;
    m_pAttackableUnit = pAttackableUnit;
    m_pAttackableUnit->AddRef();
}

// destruktor
CSCanAttackUnitDPC::~CSCanAttackUnitDPC() 
{
    // snizime pocet referenci u jednotky, na kterou jsme se ptali
    if (m_pAttackableUnit != NULL)
        m_pAttackableUnit->Release();
}


//////////////////////////////////////////////////////////////////////
// Ukladani dat DPC
//////////////////////////////////////////////////////////////////////

// ulozeni dat DPC
void CSCanAttackUnitDPC::PersistentSaveDPC(CPersistentStorage &storage)
{
    ASSERT_VALID(this);

    BRACE_BLOCK(storage);    

    // ulozime data predka
    CSSysCallDPC::PersistentSaveDPC(storage);

    // ulozime svoje data
    storage << m_pSkillType << m_pAttackableUnit;
}

// nahravani ulozenych dat DPC
void CSCanAttackUnitDPC::PersistentLoadDPC(CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    
    // prvne natahneme data predka
    CSSysCallDPC::PersistentLoadDPC(storage);

    // natahneme svoje vlastni data
    storage >> (void *&)m_pSkillType >> (void *&)m_pAttackableUnit;
    LOAD_ASSERT(m_pSkillType != NULL);
    LOAD_ASSERT(m_pAttackableUnit != NULL);
}

// preklad ukazatelu
void CSCanAttackUnitDPC::PersistentTranslatePointersDPC(CPersistentStorage &storage)
{
    // zavolame predka
    CSSysCallDPC::PersistentTranslatePointersDPC(storage);

    // prelozime si svoje pointery
    m_pSkillType = (CSAbstractAttackSkillType *) storage.TranslatePointer(m_pSkillType);
    m_pAttackableUnit = (CSUnit *) storage.TranslatePointer(m_pAttackableUnit);

    // overime spravnost prelozenych pointeru
    LOAD_ASSERT(m_pSkillType->IsKindOf(RUNTIME_CLASS(CSAbstractAttackSkillType)));
    LOAD_ASSERT(m_pAttackableUnit->IsKindOf(RUNTIME_CLASS(CSUnit)));
    
    // zvetsime refcount
//    m_pAttackableUnit->AddRef();
}

// inicializace nahraneho DPC
void CSCanAttackUnitDPC::PersistentInitDPC() 
{
    // zavolame predka
    CSSysCallDPC::PersistentInitDPC();

    // sami nedelame nic...
}


//////////////////////////////////////////////////////////////////////
// Provedeni odlozeneho volani procedury
//////////////////////////////////////////////////////////////////////

void CSCanAttackUnitDPC::Run() 
{
    // zjistime informace o jednotce, na kterou se ptame (pod zamkem)
    VERIFY(m_pAttackableUnit->PrivilegedThreadReaderLock());
    int nAttackableUnitLives = m_pAttackableUnit->GetLives();
    CPointDW pointAttackableUnitPos = m_pAttackableUnit->GetPosition();
    DWORD dwAttackableUnitAltitude = m_pAttackableUnit->GetVerticalPosition();
    ASSERT(dwAttackableUnitAltitude > 0);
    BOOL bIsVisible = m_pAttackableUnit->IsVisibleByCivilization(GetRunningDPCCivilizationIndex());
    m_pAttackableUnit->PrivilegedThreadReaderUnlock();
	
    // zamkne systémové volání
    VERIFY (m_pDPCSysCall->Lock());
	
    // zjistí, je-li systémové volání platné
    if ( m_pDPCSysCall->IsValid () )
    { // systémové volání je platné
		
        // otestujeme, zda je vubec dana jednotka nazivu
        // a viditelna moji civilizaci a zda-li jsem ja sam nazivu
        if ((nAttackableUnitLives > 0) && bIsVisible && (GetUnit()->GetLives() > 0))
        {
            // ano, jednotka zije a je videt
            ASSERT(g_cMap.IsMapPosition(pointAttackableUnitPos));
			
            // zjistime souradnice svoji jednotky
            //     zamkneme ji na cteni, protoze precteni pozice na mape
            //     neni atomicka operace
            CPointDW pointMyUnitPos = GetUnit()->GetPosition();
            ASSERT(g_cMap.IsMapPosition(pointMyUnitPos));
			
            // zjistime dosah strelby vlastni jednotky
            //     typ jednotky nebudeme na cteni zamykat, protoze jsme v main loopu
            DWORD dwAttackRadiusSquare = m_pSkillType->m_dwAttackRadiusSquare;
            DWORD dwAltitudeMin = m_pSkillType->m_dwAttackMinAltitude;
            DWORD dwAltitudeMax = m_pSkillType->m_dwAttackMaxAltitude;
			
            // otestujeme, ze testovana jednotka je v dosahu
            if ((pointMyUnitPos.GetDistanceSquare(pointAttackableUnitPos) <= dwAttackRadiusSquare) &&
				(dwAltitudeMin <= dwAttackableUnitAltitude) &&
				(dwAttackableUnitAltitude <= dwAltitudeMax))
            {
                // hura, na jednotku lze utocit
                m_pDPCSysCall->GetReturnValue()->Set(true);
            }
            else
            {
                // ojoj, jednotka je nejak moc daleko
                m_pDPCSysCall->GetReturnValue()->Set(false);
            }
        }
        else
        {
            // jednotka je mrtva nebo neni videt moji civilizaci
            m_pDPCSysCall->GetReturnValue()->Set(false);
        }
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

void CSCanAttackUnitDPC::AssertValid()
{
    // zavolame predka
    CSSysCallDPC::AssertValid();

    // assertujeme vlastni data
    ASSERT(m_pSkillType != NULL);
    ASSERT(m_pSkillType->IsKindOf(RUNTIME_CLASS(CSAbstractAttackSkillType)));
    ASSERT(m_pAttackableUnit != NULL);
    ASSERT(m_pAttackableUnit->IsKindOf(RUNTIME_CLASS(CSUnit)));
}

#endif  // _DEBUG
