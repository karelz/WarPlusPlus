/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry - skilly
 *   Autor: Tomas Karban
 * 
 *   Popis: Bullet Defense skilla -- implementace typu skilly
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SBulletDefenseSkillType.h"
#include "SBulletDefenseSkill.h"
#include "Common\Map\MBulletDefenseSkillType.h"

#define SKILLDATA(pUnit) (SSBulletDefenseSkill *)GetSkillData(pUnit)
#ifdef _DEBUG
#define ASSERT_VALID_UNIT(pUnitData) AssertValidUnit(pUnitData)
#else  // _DEBUG
#define ASSERT_VALID_UNIT(pUnitData)
#endif  // _DEBUG

////////////////////////////////////////////////////////////////////////
// Implementace interfacu volani ze skriptu
////////////////////////////////////////////////////////////////////////

BEGIN_SKILLCALL_INTERFACE_MAP(CSBulletDefenseSkillType, CSAbstractDefenseSkillType)
	SKILLCALL_BASECLASS,  // 0: bool IsActive()
	SKILLCALL_BASECLASS,  // 1: String GetName()
  SKILLCALL_BASECLASS,  // 2: int GetDefenseInteractionsMask()
  SKILLCALL_BASECLASS,  // 3: int QueryDefense(int nAttackIntensity, int nInteractionsMask)
END_SKILLCALL_INTERFACE_MAP ()

IMPLEMENT_DYNAMIC(CSBulletDefenseSkillType, CSAbstractDefenseSkillType);
IMPLEMENT_SKILL_TYPE(CSBulletDefenseSkillType, 2, "BulletDefense");

////////////////////////////////////////////////////////////////////////
// Destruktor
////////////////////////////////////////////////////////////////////////

CSBulletDefenseSkillType::~CSBulletDefenseSkillType()
{
    ASSERT(m_nLinearCoefficient == 1000);
    ASSERT(m_nAbsoluteCoefficient == 0);
}


////////////////////////////////////////////////////////////////////////
// Inicializace a destrukce typu skilly
////////////////////////////////////////////////////////////////////////

void CSBulletDefenseSkillType::CreateSkillType(void *pData, DWORD dwDataSize)
{
    // velikost, kterou jsem dostal jako parametr, se musi rovnat
    // velikosti struktury, kterou uklada MapEditor
    LOAD_ASSERT(dwDataSize == sizeof(SMBulletDefenseSkillType));
    // pointer v parametru musi ukazovat nekam smysluplne
    ASSERT(::AfxIsValidAddress(pData, dwDataSize, FALSE));

    // zkopiruju si linearni koeficient
    m_nLinearCoefficient = ((SMBulletDefenseSkillType *)pData)->m_dwLinearCoefficient;
    LOAD_ASSERT(m_nLinearCoefficient >= SM_BULLETDEFENSE_SKILLTYPE_LIN_MIN);
    LOAD_ASSERT(m_nLinearCoefficient <= SM_BULLETDEFENSE_SKILLTYPE_LIN_MAX);

    // zkopiruju si absolutni koeficient
    m_nAbsoluteCoefficient = ((SMBulletDefenseSkillType *)pData)->m_dwAbsoluteCoefficient;
    LOAD_ASSERT(m_nAbsoluteCoefficient >= SM_BULLETDEFENSE_SKILLTYPE_ABS_MIN);
    LOAD_ASSERT(m_nAbsoluteCoefficient <= SM_BULLETDEFENSE_SKILLTYPE_ABS_MAX);
}


void CSBulletDefenseSkillType::DeleteSkillType()
{
    m_nLinearCoefficient = 1000;
    m_nAbsoluteCoefficient = 0;
}


//////////////////////////////////////////////////////////////////////
// Load/Save dat typu skilly
//////////////////////////////////////////////////////////////////////

void CSBulletDefenseSkillType::PersistentSaveSkillType(CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);
    
    // otestuj data typu skilly
    ASSERT_VALID(this);
    
    // uloz data do souboru
    storage << m_nLinearCoefficient << m_nAbsoluteCoefficient;
}

void CSBulletDefenseSkillType::PersistentLoadSkillType(CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);    
    
    // natahni data ze souboru
    storage >> m_nLinearCoefficient >> m_nAbsoluteCoefficient;

    // otestuj spravnost natazenych dat
    LOAD_ASSERT(m_nLinearCoefficient >= 0);
    LOAD_ASSERT(m_nLinearCoefficient <= 1000);
    LOAD_ASSERT(m_nAbsoluteCoefficient >= 0);
}

void CSBulletDefenseSkillType::PersistentTranslatePointersSkillType(CPersistentStorage &)
{
    // nic
}

void CSBulletDefenseSkillType::PersistentInitSkillType()
{
    // nic
}


//////////////////////////////////////////////////////////////////////
// Save/Load dat skilly u jednotky
//////////////////////////////////////////////////////////////////////

void CSBulletDefenseSkillType::PersistentSaveSkill(CPersistentStorage &storage, CSUnit *pUnit)
{
    BRACE_BLOCK(storage);
    
    // otestujeme spravnost pointeru pUnit
    ASSERT(pUnit != NULL);
	// sezen pointer na data skilly u jednotky
    SSBulletDefenseSkill *pUnitData = SKILLDATA(pUnit);
    // otestuj pointer
    ASSERT(AfxIsValidAddress(pUnitData, sizeof(SSBulletDefenseSkill), TRUE));
    // otestuj data skilly u jednotky
    ASSERT_VALID_UNIT(pUnitData);

    // uloz data do souboru
    storage << pUnitData->m_nPostLinearCoefficient << pUnitData->m_nPostAbsoluteCoefficient;
}

void CSBulletDefenseSkillType::PersistentLoadSkill(CPersistentStorage &storage, CSUnit *pUnit)
{
    BRACE_BLOCK(storage);    
    
    // otestujeme spravnost pointeru pUnit
    ASSERT(pUnit != NULL);
	// sezen pointer na data skilly u jednotky
    SSBulletDefenseSkill *pUnitData = SKILLDATA(pUnit);
    // otestuj pointer
    ASSERT(AfxIsValidAddress(pUnitData, sizeof(SSBulletDefenseSkill), TRUE));

    // natahni data ze souboru
    storage >> pUnitData->m_nPostLinearCoefficient >> pUnitData->m_nPostAbsoluteCoefficient;

    // otestuj natazena data
    LOAD_ASSERT(pUnitData->m_nPostLinearCoefficient >= 0);
    LOAD_ASSERT(pUnitData->m_nPostLinearCoefficient <= 1000);
    LOAD_ASSERT(pUnitData->m_nPostAbsoluteCoefficient >= 0);
}

void CSBulletDefenseSkillType::PersistentTranslatePointersSkill(CPersistentStorage &, CSUnit *)
{
    // nic
}

void CSBulletDefenseSkillType::PersistentInitSkill(CSUnit *)
{
    // nic
}


//////////////////////////////////////////////////////////////////////
// Specificke operace na skille
//////////////////////////////////////////////////////////////////////

void CSBulletDefenseSkillType::DoTimeSlice(CSUnit *)
{
    // tato skilla nemuze byt aktivni,
    // takze volat DoTimeSlice je nesmysl
    ASSERT(FALSE);
}


// inicializuje skillu jednotky "pUnit"
void CSBulletDefenseSkillType::InitializeSkill(CSUnit *pUnit)
{
    // otestujeme spravnost pointeru pUnit
    ASSERT(pUnit != NULL);
    // sezen pointer na data skilly u jednotky
    SSBulletDefenseSkill *pUnitData = SKILLDATA(pUnit);
    // otestuj pointer
    ASSERT(AfxIsValidAddress(pUnitData, sizeof(SSBulletDefenseSkill), TRUE));

    // inicializuj na hodnoty "zadny sliz"
    pUnitData->m_nPostLinearCoefficient = 1000;
    pUnitData->m_nPostAbsoluteCoefficient = 0;
}

    
// znici skillu jednotky "pUnit"
void CSBulletDefenseSkillType::DestructSkill(CSUnit *)
{
    // neni co na praci, zadne dynamicke veci, zadne pointery
    // na cizi objekty s pocitanymi referencemi...
}
    

// aktivuje skillu jednotky "pUnit" (jednotka musi byt zamcena pro zapis)
void CSBulletDefenseSkillType::ActivateSkill(CSUnit *)
{
    // tato skilla nemuze byt aktivovana
    ASSERT(FALSE);
}


// deaktivuje skillu jednotky "pUnit" (jednotka musi byt zamcena pro zapis)
void CSBulletDefenseSkillType::DeactivateSkill(CSUnit *)
{
    // tato skilla nemuze byt deaktivovana
    ASSERT(FALSE);
}
    

//////////////////////////////////////////////////////////////////////
// Specificke operace na skille
//////////////////////////////////////////////////////////////////////

// vraci masku interakci, ktere je schopna tato defense skilla branit
DWORD CSBulletDefenseSkillType::GetDefenseInteractionsMask()
{
    return Interaction_Bullet;
}


// dotaz na silu obrany pri zadanem utoku
int CSBulletDefenseSkillType::QueryDefense(CSUnit *pUnit, int nAttackIntensity, DWORD dwInteractionMask)
{
    // zavolej DoDefense() -- ta je v tomto specialnim pripade stejna
    // (az na to, kdo je utocnik, dame tam NULL, protoze tu defense
    // skillu to stejne nezajima -- POUZE V TOMTO PRIPADE!)
    return DoDefense(pUnit, NULL, nAttackIntensity, dwInteractionMask);
}


//////////////////////////////////////////////////////////////////////
// Vypocetni vzorec pro bullet defense
//////////////////////////////////////////////////////////////////////

int CSBulletDefenseSkillType::DoDefense(CSUnit *pAttackedUnit, CSUnit * /*pAggressor*/, int nAttackIntensity, DWORD dwInteractionMask)
{
    // otestuj data typu skilly
    ASSERT_VALID(this);

    // otestuj parametry metody
    ASSERT(pAttackedUnit != NULL);
    ASSERT(nAttackIntensity >= 0);
    ASSERT((dwInteractionMask & ~((DWORD)Interaction_All)) == 0);

	// sezen pointer na data skilly u jednotky
    SSBulletDefenseSkill *pUnitData = SKILLDATA(pAttackedUnit);
    // otestuj pointer
    ASSERT(AfxIsValidAddress(pUnitData, sizeof(SSBulletDefenseSkill), TRUE));
    // otestuj data skilly u jednotky
    ASSERT_VALID_UNIT(pUnitData);

    if ((dwInteractionMask & GetDefenseInteractionsMask()) != 0)
    {
        // umime branit
        
        // odecteme absolutni clen
        int nTmp = nAttackIntensity - m_nAbsoluteCoefficient;

        // kdyz jsme pod nulou (branime vic, nez se utoci),
        // tak dorovname na nulu
        if (nTmp < 0)
            nTmp = 0;

        // vynasobime linearnim clenem
        nTmp = +(nTmp * m_nLinearCoefficient) / 1000;

        // promenna nTmp ted obsahuje silu, "jak moc to boli";
        // dopocitame, kolik je rozdil od puvodni sily
        nTmp = nAttackIntensity - nTmp;

        // nyni aplikujeme ucinek pripadneho slizu
        // promenna nTmp obsahuje ubytek utocne sily po aplikaci obrany
        // sliz tento ubytek muze snizit (tj. snizit efektivitu obranu)
        nTmp -= pUnitData->m_nPostAbsoluteCoefficient;

        // kdyz se dostaneme pod nulu (sliz uplne eliminuje obranu),
        // tak dorovname na nulu
        if (nTmp < 0)
            nTmp = 0;

        // vynasobime linearnim clenem
        nTmp = +(nTmp * pUnitData->m_nPostLinearCoefficient) / 1000;

        // a ted je hotovo, nTmp obsahuje cislo, kolik jsme ubranili
        // ze zadaneho utoku
        return nTmp;
    }
    else
    {
        // neumime branit, vracime 0
        return 0;
    }
}


////////////////////////////////////////////////////////////////////////
// Full Info
////////////////////////////////////////////////////////////////////////

void CSBulletDefenseSkillType::FillFullInfo(CSUnit *pUnit, CString &strInfo)
{
    // je skilla enabled na dane jednotce?
    if (IsEnabled(pUnit))
    {
        strInfo.Format("Bullet Defense:\n  Abs: %d\n  Lin: %d%%", -m_nAbsoluteCoefficient, (int)(100.0 - (double)m_nLinearCoefficient / 10.0 + 0.5));
    }
    else
    {
        strInfo = "Bullet Defense (disabled)";
    }
}


////////////////////////////////////////////////////////////////////////
// Debug metody
////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

void CSBulletDefenseSkillType::AssertValid() const
{
    CSAbstractDefenseSkillType::AssertValid();

    ASSERT(m_nLinearCoefficient >= 0);
    ASSERT(m_nLinearCoefficient <= 1000);
    ASSERT(m_nAbsoluteCoefficient >= 0);
}

void CSBulletDefenseSkillType::AssertValidUnit(SSBulletDefenseSkill *pUnitData) const
{
    ASSERT(pUnitData->m_nPostLinearCoefficient >= 0);
    ASSERT(pUnitData->m_nPostLinearCoefficient <= 1000);
    ASSERT(pUnitData->m_nPostAbsoluteCoefficient >= 0);
}

void CSBulletDefenseSkillType::Dump(CDumpContext &dc) const
{
    dc << "Bullet defense skill type:" << endl;
    dc << "  linear coefficient: " << m_nLinearCoefficient << endl;
    dc << "  absolute coefficient: " << m_nAbsoluteCoefficient << endl;
}

#endif  // _DEBUG
