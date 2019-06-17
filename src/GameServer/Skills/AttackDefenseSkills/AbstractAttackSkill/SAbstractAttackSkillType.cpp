/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry - skilly
 *   Autor: Tomas Karban
 * 
 *   Popis: Abstraktni predek vsech attack skill -- implementace typu skilly
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SAbstractAttackSkillType.h"
#include "SAbstractAttackSkill.h"
#include "Common\Map\MAbstractAttackSkillType.h"
#include "TripleS\Interpret\Src\SysObjs\ISOSet.h"
#include "SAttackableUnitsDPC.h"
#include "SCanAttackUnitDPC.h"
#include "GameServer\Civilization\ZCivilization.h"
#include "GameServer\Skills\SysCalls\IDPCSysCall.h"
#include "GameServer\Civilization\Src\ZUnit.h"
#include "GameServer\GameServer\SNotifications.h"
#include "ISysCallAttack.h"

IMPLEMENT_DYNAMIC(CSAbstractAttackSkillType, CSSkillType);

#define SKILLDATA(pUnit) (SSAbstractAttackSkill *)GetSkillData(pUnit)
#ifdef _DEBUG
#define ASSERT_VALID_UNIT(pUnitData) AssertValidUnit(pUnitData)
#else  // _DEBUG
#define ASSERT_VALID_UNIT(pUnitData)
#endif  // _DEBUG

// implementace interfacu volani ze skriptu
BEGIN_SKILLCALL_INTERFACE_MAP(CSAbstractAttackSkillType, CSSkillType)
	SKILLCALL_BASECLASS,  // IsActive()
	SKILLCALL_BASECLASS,  // GetName()
    SKILLCALL_METHOD(SCI_GetAttackableUnits),
    SKILLCALL_METHOD(SCI_CanAttackUnit),
    SKILLCALL_METHOD(SCI_GetAttackInteractionsMask),
    SKILLCALL_METHOD(SCI_GetAttackRadius),
    SKILLCALL_METHOD(SCI_GetAttackAltMin),
    SKILLCALL_METHOD(SCI_GetAttackAltMax)
END_SKILLCALL_INTERFACE_MAP ()

/////////////////////////////////////////////////////////////////////
// Konstruktor a destruktor
/////////////////////////////////////////////////////////////////////

// konstruktor je inline

// destruktor
CSAbstractAttackSkillType::~CSAbstractAttackSkillType()
{
    ASSERT(m_dwAttackRadius == 0);
    ASSERT(m_dwAttackRadiusSquare == 0);
    ASSERT(m_dwAttackMinAltitude == 0);
    ASSERT(m_dwAttackMaxAltitude == 0);
}


/////////////////////////////////////////////////////////////////////
// Inicializace a zniceni dat typu skilly
/////////////////////////////////////////////////////////////////////

void CSAbstractAttackSkillType::CreateSkillType(void *pData, DWORD dwDataSize)
{
    // data z mapeditoru musi mit spravnou velikost
    LOAD_ASSERT(dwDataSize == sizeof(SMAbstractAttackSkillType));
    // pointer musi ukazovat na platnou pamet
    ASSERT(::AfxIsValidAddress(pData, dwDataSize, FALSE));

    // nacteme akcni polomer
    m_dwAttackRadius = ((SMAbstractAttackSkillType *)pData)->m_dwAttackRadius;
    // melo by to byt alespon 1
    LOAD_ASSERT(m_dwAttackRadius >= 1);
    // a aby to slo beze ztrat umocnit
    LOAD_ASSERT(m_dwAttackRadius <= 65535);
    // spocitame druhou mocninu polomeru
    m_dwAttackRadiusSquare = m_dwAttackRadius * m_dwAttackRadius;

    // nacteme vyskovy interval
    m_dwAttackMinAltitude = ((SMAbstractAttackSkillType *)pData)->m_dwAttackMinAltitude;
    m_dwAttackMaxAltitude = ((SMAbstractAttackSkillType *)pData)->m_dwAttackMaxAltitude;
    // vyska minimalne 1 (nula je neplatna)
    LOAD_ASSERT(m_dwAttackMinAltitude >= 1);
    // interval by mel jaksi davat nejakej smysl
    LOAD_ASSERT(m_dwAttackMinAltitude <= m_dwAttackMaxAltitude);
}


void CSAbstractAttackSkillType::DeleteSkillType()
{
    m_dwAttackRadius = 0;
    m_dwAttackRadiusSquare = 0;
    m_dwAttackMinAltitude = 0;
    m_dwAttackMaxAltitude = 0;
}


/////////////////////////////////////////////////////////////////////
// Aktivace a deaktivace skilly
/////////////////////////////////////////////////////////////////////

void CSAbstractAttackSkillType::ActivateSkill(CSUnit *pUnit, CISysCallAttack *pSysCall)
{
    // overime spravnost parametru
    ASSERT(pUnit != NULL);
    ASSERT_KINDOF(CSUnit, pUnit);
    ASSERT(pSysCall != NULL);
    ASSERT_KINDOF(CISysCallAttack, pSysCall);

    // k jednotce do dat musime ulozit syscall
    // sezeneme pointer na data skilly u jednotky
    SSAbstractAttackSkill *pUnitData = SKILLDATA(pUnit);
    // otestujeme pointer
    ASSERT(AfxIsValidAddress(pUnitData, sizeof(SSAbstractAttackSkill), TRUE));
    // otestujeme data skilly u jednotky
    ASSERT_VALID_UNIT(pUnitData);
    // ulozime syscall
    pSysCall->AddRef();
    pUnitData->m_pSysCall = pSysCall;

    // nakonec zavolame predka
    CSSkillType::ActivateSkill(pUnit); 
}

// DeactivateSkill() je inline, jen vola predka


/////////////////////////////////////////////////////////////////////
// Persistent load/save typu skilly
/////////////////////////////////////////////////////////////////////

void CSAbstractAttackSkillType::PersistentSaveSkillType(CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    
    // pouze ulozime svoje data, assertujeme, co se da...
    ASSERT(m_dwAttackRadius >= 1);
    ASSERT(m_dwAttackRadius <= 65535);
    ASSERT(m_dwAttackMinAltitude >= 1);
    ASSERT(m_dwAttackMinAltitude <= m_dwAttackMaxAltitude);
    storage << m_dwAttackRadius << m_dwAttackMinAltitude << m_dwAttackMaxAltitude;
}

void CSAbstractAttackSkillType::PersistentLoadSkillType(CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    // jen precteme data, assertujeme, co se da...
    storage >> m_dwAttackRadius >> m_dwAttackMinAltitude >> m_dwAttackMaxAltitude;
    LOAD_ASSERT(m_dwAttackRadius >= 1);
    LOAD_ASSERT(m_dwAttackRadius <= 65535);
    LOAD_ASSERT(m_dwAttackMinAltitude >= 1);
    LOAD_ASSERT(m_dwAttackMinAltitude <= m_dwAttackMaxAltitude);
}

void CSAbstractAttackSkillType::PersistentTranslatePointersSkillType(CPersistentStorage &)
{
    // nic...
}

void CSAbstractAttackSkillType::PersistentInitSkillType()
{
    // dopocitame druhou mocninu akcniho polomeru
    m_dwAttackRadiusSquare = m_dwAttackRadius * m_dwAttackRadius;
}


////////////////////////////////////////////////////////////////////////
// Ukladani dat skilly u jednotky
////////////////////////////////////////////////////////////////////////

// ukladani dat skilly
void CSAbstractAttackSkillType::PersistentSaveSkill(CPersistentStorage &storage, CSUnit *pUnit)
{
    BRACE_BLOCK(storage);

    // otestujeme spravnost parametru
    ASSERT(pUnit != NULL);
    ASSERT_KINDOF(CSUnit, pUnit);

    // sezeneme ukazatel na data skilly u jednotky
    SSAbstractAttackSkill *pUnitData = SKILLDATA(pUnit);
    // otestujeme ukazatel
    ASSERT(AfxIsValidAddress(pUnitData, sizeof(SSAbstractAttackSkill), TRUE));
    // otestujeme data skilly u jednotky
    ASSERT_VALID_UNIT(pUnitData);
    
    // na jednotce je jen pointer na syscall
    if (pUnitData->m_pSysCall != NULL)
    {
        // kdyz je neNULL, tak ulozime true a potom samotny syscall
        storage << TRUE;
        pUnitData->m_pSysCall->PersistentSave(storage);
    }
    else
    {
        // kdyz je NULL, tak ulozime false
        storage << FALSE;
    }    
}


// nahravani pouze ulozenych dat skilly
void CSAbstractAttackSkillType::PersistentLoadSkill(CPersistentStorage &storage, CSUnit *pUnit)
{
    BRACE_BLOCK(storage);

    // otestujeme spravnost parametru
    ASSERT(pUnit != NULL);
    ASSERT_KINDOF(CSUnit, pUnit);

    // sezeneme ukazatel na data skilly u jednotky
    SSAbstractAttackSkill *pUnitData = SKILLDATA(pUnit);
    // otestujeme ukazatel
    ASSERT(AfxIsValidAddress(pUnitData, sizeof(SSAbstractAttackSkill), TRUE));
    // otestujeme data skilly u jednotky
    ASSERT_VALID_UNIT(pUnitData);
    
    // prvne natahneme boolean, jestli je ulozeny i syscall
    BOOL bSysCallStored;
    storage >> bSysCallStored;

    if (bSysCallStored)
    {
        // syscall je ulozeny, tak ho vytvorime a zavolame na nem load
        pUnitData->m_pSysCall = new CISysCallAttack();
        pUnitData->m_pSysCall->PersistentLoad(storage);

        // !!! on sam vola registraci, on sam si totiz ulozil
        // starej pointer... fujky fujky hnusak...
    }
    else
    {
        // syscall neni ulozeny, vyplnime data NULLem
        pUnitData->m_pSysCall = NULL;
    }
}


// preklad ukazatelu dat skilly
void CSAbstractAttackSkillType::PersistentTranslatePointersSkill(CPersistentStorage &storage, CSUnit *pUnit)
{
    // sezeneme ukazatel na data skilly u jednotky
    SSAbstractAttackSkill *pUnitData = SKILLDATA(pUnit);

    if (pUnitData->m_pSysCall != NULL) pUnitData->m_pSysCall->PersistentTranslatePointers( storage);
}


// inicializace nahranych dat skilly
void CSAbstractAttackSkillType::PersistentInitSkill(CSUnit *pUnit)
{
    // sezeneme ukazatel na data skilly u jednotky
    SSAbstractAttackSkill *pUnitData = SKILLDATA(pUnit);

    if (pUnitData->m_pSysCall != NULL) pUnitData->m_pSysCall->PersistentInit();
}


////////////////////////////////////////////////////////////////////////
// Notifikace napadene jednotky
////////////////////////////////////////////////////////////////////////

void CSAbstractAttackSkillType::NotifyAttackedUnit(CSUnit *pAggressor, CSUnit *pAttackedUnit, int nAttackIntensity)
{
    // posleme notifikaci, ze napadena jednotka utekla z dosahu zbrane

    // pripravime bag
    CIBag cBag;
    cBag.Create();

    // zjistime, zdali napadena jednotka (presneji civilizace) muze videt agresora
    if (pAggressor->IsVisibleByCivilization(pAttackedUnit->GetCivilizationIndex()))
    {
        // ano, do notifikace dame agresora
        cBag.AddUnit(pAggressor->GetZUnit(), NULL);
        TRACE_NOTIFICATION(". Unit (ID=%d) is attacked by unit (ID=%d)\n", pAttackedUnit->GetID(), pAggressor->GetID());
    }
    else
    {
        // ne, dame mu NULL; nedozvi se, od koho byl napaden
        cBag.AddUnit(NULL, NULL);
        TRACE_NOTIFICATION(". Unit (ID=%d) is attacked by invisible unit\n", pAttackedUnit->GetID());
    }

    // pridame silu utoku
    cBag.AddConstantInt(nAttackIntensity);
    // pošleme jednotce notifikaci
    SendNotification(pAttackedUnit->GetZUnit(), NOTIFICATION_BEING_ATTACKED, &cBag);
    // a bag smazeme (notifikace si ho uz nekam zkopirovala)
    cBag.Delete();
}


////////////////////////////////////////////////////////////////////////
// Inicializace a zniceni dat skilly u jednotky
////////////////////////////////////////////////////////////////////////

void CSAbstractAttackSkillType::InitializeSkill(CSUnit *pUnit)
{
    // otestujeme spravnost pointeru pUnit
    ASSERT(pUnit != NULL);
    ASSERT_KINDOF(CSUnit, pUnit);
	// sezen pointer na data skilly u jednotky
    SSAbstractAttackSkill *pUnitData = SKILLDATA(pUnit);
    // otestuj pointer
    ASSERT(::AfxIsValidAddress(pUnitData, sizeof(SSAbstractAttackSkill), TRUE));

    // vynuluj data u jednotky
    pUnitData->m_pSysCall = NULL;
}


void CSAbstractAttackSkillType::DestructSkill(CSUnit *pUnit)
{
    // otestujeme spravnost pointeru pUnit
    ASSERT(pUnit != NULL);
    ASSERT_KINDOF(CSUnit, pUnit);
	// sezen pointer na data skilly u jednotky
    SSAbstractAttackSkill *pUnitData = SKILLDATA(pUnit);
    // otestuj pointer
    ASSERT(::AfxIsValidAddress(pUnitData, sizeof(SSAbstractAttackSkill), TRUE));

    // kdyz je syscall != NULL, tak ho znicime
    // (smrt jednotky nebo ukonceni serveru)
    if (pUnitData->m_pSysCall != NULL)
    {
        pUnitData->m_pSysCall->FinishWaiting(STATUS_SKILL_COLLISION);
		pUnitData->m_pSysCall->Release();
        pUnitData->m_pSysCall = NULL;
    }
}


////////////////////////////////////////////////////////////////////////
// Metody pro interface do skriptu
////////////////////////////////////////////////////////////////////////

ESyscallResult CSAbstractAttackSkillType::SCI_GetAttackableUnits(CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag * /*pBag*/, CIDataStackItem *pReturn)
{
    TRACE_SKILLCALL("@ AbstractAttack - SCI_GetAttackableUnits() called on unit %d\n", pUnit->GetID());

    CISOSet<CZUnit*> *pSet;
    CType Type;
    Type.SetUnit(g_StringTable.AddItem(DEFAULT_UNIT_PARENT, false));
    pSet = (CISOSet<CZUnit*> *)pProcess->m_pInterpret->CreateSet(&Type, TRUE);

    if (pSet == NULL)
    {
        TRACE_SKILLCALL("@ AbstractAttack - SCI_GetAttackableUnits() -- syscall error while creating a set\n");
        return SYSCALL_ERROR;
    }

    pReturn->Set(pSet);
    pSet->Release(); 

    CSAttackableUnitsDPC *pDPC = new CSAttackableUnitsDPC(pProcess, pReturn, pUnit, this);
    g_cMap.RegisterDPC(pDPC, pProcess->m_pInterpret->m_pCivilization->GetCivilizationIndex());

    *ppSysCall = pDPC->GetDPCSysCall();

    TRACE_SKILLCALL("@ AbstractAttack - SCI_GetAttackableUnits() -- success sleep\n");
    return SYSCALL_SUCCESS_SLEEP;
}


ESyscallResult CSAbstractAttackSkillType::SCI_CanAttackUnit(CSUnit *pUnit, CIProcess *pProcess, 
	CISyscall **ppSysCall, CIBag * /*pBag*/, CIDataStackItem *pReturn, CZUnit *pAttackableUnit)
{
    TRACE_SKILLCALL("@ AbstractAttack - SCI_CanAttackUnit() called on unit %d asking for unit %d\n", pUnit->GetID(), pAttackableUnit->GetSUnit()->GetID());

    CSCanAttackUnitDPC *pDPC = new CSCanAttackUnitDPC(pProcess, pReturn, pUnit, this, pAttackableUnit->GetSUnit());
    *ppSysCall = pDPC->GetDPCSysCall();
    g_cMap.RegisterDPC(pDPC, pProcess->m_pInterpret->m_pCivilization->GetCivilizationIndex());

    return SYSCALL_SUCCESS_SLEEP;
}


ESyscallResult CSAbstractAttackSkillType::SCI_GetAttackInteractionsMask(CSUnit * /*pUnit*/, CIProcess * /*pProcess*/, CISyscall ** /*ppSysCall*/, CIBag * /*pBag*/, CIDataStackItem *pReturn)
{
	TRACE_SKILLCALL("@ Abstract attack - SCI_GetAttackInteractionsMask() -- returns %#.4x\n", GetAttackInteractionsMask());

    // pripravime navratovou hodnotu
    pReturn->Set((int) GetAttackInteractionsMask());

	// uspech
	return SYSCALL_SUCCESS;
}


ESyscallResult CSAbstractAttackSkillType::SCI_GetAttackRadius(CSUnit * /*pUnit*/, CIProcess * /*pProcess*/, CISyscall ** /*ppSysCall*/, CIBag * /*pBag*/, CIDataStackItem *pReturn)
{
	TRACE_SKILLCALL("@ Abstract attack - SCI_GetAttackRadius() -- radius %u\n", m_dwAttackRadius);

    // okopirujeme hodnoty do parametru (predanych odkazem)
    pReturn->Set((int)m_dwAttackRadius);

    // uspech
	return SYSCALL_SUCCESS;
}


ESyscallResult CSAbstractAttackSkillType::SCI_GetAttackAltMin(CSUnit * /*pUnit*/, CIProcess * /*pProcess*/, CISyscall ** /*ppSysCall*/, CIBag * /*pBag*/, CIDataStackItem *pReturn)
{
	TRACE_SKILLCALL("@ Abstract attack - SCI_GetAttackAltMin() -- min altitude %u\n", m_dwAttackMinAltitude);

    // okopirujeme hodnoty do parametru (predanych odkazem)
    pReturn->Set((int)m_dwAttackMinAltitude);

    // uspech
	return SYSCALL_SUCCESS;
}


ESyscallResult CSAbstractAttackSkillType::SCI_GetAttackAltMax(CSUnit * /*pUnit*/, CIProcess * /*pProcess*/, CISyscall ** /*ppSysCall*/, CIBag * /*pBag*/, CIDataStackItem *pReturn)
{
	TRACE_SKILLCALL("@ Abstract attack - SCI_GetAttackAltMax() -- max altitude %u\n", m_dwAttackMaxAltitude);

    // okopirujeme hodnoty do parametru (predanych odkazem)
    pReturn->Set((int)m_dwAttackMaxAltitude);

    // uspech
	return SYSCALL_SUCCESS;
}


////////////////////////////////////////////////////////////////////////
// Debugovaci metody
////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

void CSAbstractAttackSkillType::AssertValidUnit(SSAbstractAttackSkill *pUnitData) const
{
    // otestujeme spravnost pointeru na syscall
    if (pUnitData->m_pSysCall != NULL)
        ASSERT_KINDOF(CISysCallAttack, pUnitData->m_pSysCall);
}

// ostatni jsou inline a prazdne

#endif  // _DEBUG
