/***********************************************************
* 
* Projekt: Strategicka hra
*    Cast: Server hry - skilly
*   Autor: Tomas Karban
* 
*   Popis: Bullet Attack skilla -- implementace typu skilly
* 
***********************************************************/

#include "StdAfx.h"

#include "SBulletAttackSkillType.h"
#include "SBulletAttackSkill.h"
#include "Common\Map\MBulletAttackSkillType.h"
#include "..\AbstractDefenseSkill\SAbstractDefenseSkillType.h"
#include "GameServer\Civilization\Src\ZUnit.h"
#include "GameServer\GameServer\SNotifications.h"
#include "..\AbstractAttackSkill\ISysCallAttack.h"

#define SKILLDATA(pUnit) (SSBulletAttackSkill *)GetSkillData(pUnit)
#ifdef _DEBUG
#define ASSERT_VALID_UNIT(pUnitData) AssertValidUnit(pUnitData)
#else  // _DEBUG
#define ASSERT_VALID_UNIT(pUnitData)
#endif  // _DEBUG

// implementace interfacu volani ze skriptu
BEGIN_SKILLCALL_INTERFACE_MAP(CSBulletAttackSkillType, CSAbstractAttackSkillType)
SKILLCALL_BASECLASS,  // IsActive()
SKILLCALL_BASECLASS,  // GetName()
SKILLCALL_BASECLASS,  // GetAttackableUnits()
SKILLCALL_BASECLASS,  // CanAttackUnit()
SKILLCALL_BASECLASS,  // GetAttackInteractionsMask()
SKILLCALL_BASECLASS,  // GetAttackRadius()
SKILLCALL_BASECLASS,  // GetAttackAltMin()
SKILLCALL_BASECLASS,  // GetAttackAltMax()
SKILLCALL_METHOD(SCI_Fire)
END_SKILLCALL_INTERFACE_MAP ()

IMPLEMENT_DYNAMIC(CSBulletAttackSkillType, CSAbstractAttackSkillType);
IMPLEMENT_SKILL_TYPE(CSBulletAttackSkillType, 1, "BulletAttack");

////////////////////////////////////////////////////////////////////////
// Destruktor
////////////////////////////////////////////////////////////////////////

CSBulletAttackSkillType::~CSBulletAttackSkillType()
{
  ASSERT(m_dwLoadTime == 0);
  ASSERT(m_dwFirePeriod == 0);
  ASSERT(m_nAttackIntensity == 0);
  ASSERT(m_dwAppearanceID == 0);
  ASSERT(m_dwAnimationOffset == 0);
  ASSERT(m_dwAnimationLength == 0);
  ASSERT(m_dwAnimationEnd == 0);
}


////////////////////////////////////////////////////////////////////////
// Inicializace a destrukce dat typu skilly
////////////////////////////////////////////////////////////////////////

// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
//    vyjimky: CPersistentLoadException
void CSBulletAttackSkillType::CreateSkillType(void *pData, DWORD dwDataSize)
{
  // data musi mit ocekavanou velikost
  LOAD_ASSERT(dwDataSize == sizeof(SMBulletAttackSkillType));
  // pointer musi ukazovat na rozumne misto v pameti
  ASSERT(::AfxIsValidAddress(pData, dwDataSize, FALSE));
  
  // nacteme data, ktera ma predek
  CSAbstractAttackSkillType::CreateSkillType(pData, sizeof(SMAbstractAttackSkillType));
  // otestujeme jejich spravnost
  LOAD_ASSERT(m_dwAttackRadius >= SM_BULLETATTACK_SKILLTYPE_RADIUS_MIN);
  LOAD_ASSERT(m_dwAttackRadius <= SM_BULLETATTACK_SKILLTYPE_RADIUS_MAX);
  LOAD_ASSERT(SM_BULLETATTACK_SKILLTYPE_ALTITUDE_MIN <= m_dwAttackMinAltitude);
  LOAD_ASSERT(m_dwAttackMinAltitude <= m_dwAttackMaxAltitude);
  LOAD_ASSERT(m_dwAttackMaxAltitude <= SM_BULLETATTACK_SKILLTYPE_ALTITUDE_MAX);
  
  // nacteme cas pred prvnim vystrelem
  m_dwLoadTime = ((SMBulletAttackSkillType *)pData)->m_dwLoadTime;
  LOAD_ASSERT(m_dwLoadTime <= SM_BULLETATTACK_SKILLTYPE_LOADTIME_MAX);
  
  // nacteme periodu strelby
  m_dwFirePeriod = ((SMBulletAttackSkillType *)pData)->m_dwFirePeriod;
  LOAD_ASSERT(m_dwFirePeriod <= SM_BULLETATTACK_SKILLTYPE_FIREPERIOD_MAX);
  
  // nacteme silu utoku
  m_nAttackIntensity = ((SMBulletAttackSkillType *)pData)->m_nAttackIntensity;
  LOAD_ASSERT(m_nAttackIntensity >= SM_BULLETATTACK_SKILLTYPE_INTENSITY_MIN);
  LOAD_ASSERT(m_nAttackIntensity <= SM_BULLETATTACK_SKILLTYPE_INTENSITY_MAX);
  
  // nacteme ID vzhledu jednotky pri strelbe (nelze assertovat)
  m_dwAppearanceID = ((SMBulletAttackSkillType *)pData)->m_dwAppearanceID;
  LOAD_ASSERT(m_dwAppearanceID != 0);
  
  // nacteme offset zacatku animace
  m_dwAnimationOffset = ((SMBulletAttackSkillType *)pData)->m_dwAnimationOffset;
  // offset zacatku animace musi byt alespon tak velky jako cas pred prvnim vystrelem
  LOAD_ASSERT(m_dwAnimationOffset <= m_dwLoadTime);
  
  // nacteme delku animace (nesmi byt nula)
  m_dwAnimationLength = ((SMBulletAttackSkillType *)pData)->m_dwAnimationLength;
  LOAD_ASSERT(m_dwAnimationLength >= SM_BULLETATTACK_SKILLTYPE_ANIMATIONLENGTH_MIN);
  LOAD_ASSERT(m_dwAnimationLength <= m_dwFirePeriod);
  LOAD_ASSERT(m_dwAnimationOffset <= m_dwAnimationLength);
  
  // dopocitame si konec animace
  m_dwAnimationEnd = m_dwFirePeriod - (m_dwAnimationLength - m_dwAnimationOffset);
}

// znici data typu skilly
void CSBulletAttackSkillType::DeleteSkillType()
{
  m_dwLoadTime = 0;
  m_dwFirePeriod = 0;
  m_nAttackIntensity = 0;
  m_dwAppearanceID = 0;
  m_dwAnimationOffset = 0;
  m_dwAnimationLength = 0;
  m_dwAnimationEnd = 0;
  
  CSAbstractAttackSkillType::DeleteSkillType();
}


////////////////////////////////////////////////////////////////////////
// Inicializace a uklid dat skilly u jednotky
////////////////////////////////////////////////////////////////////////

void CSBulletAttackSkillType::InitializeSkill(CSUnit *pUnit)
{
  // prvne zavolame predka
  CSAbstractAttackSkillType::InitializeSkill(pUnit);
  
  // otestujeme spravnost pointeru pUnit
  ASSERT(pUnit != NULL);
  ASSERT_KINDOF(CSUnit, pUnit);
  // sezen pointer na data skilly u jednotky
  SSBulletAttackSkill *pUnitData = SKILLDATA(pUnit);
  // otestuj pointer
  ASSERT(::AfxIsValidAddress(pUnitData, sizeof(SSBulletAttackSkill), TRUE));
  
  // vynuluj data u jednotky
  pUnitData->m_dwRemainingTime = 0;
  pUnitData->m_pAttackedUnit = NULL;
}


void CSBulletAttackSkillType::DestructSkill(CSUnit *pUnit)
{
  // opatrne smazeme data u jednotky
  // (kdyz se konci server a skilla je aktivni, tak po
  // pauznuti hry se nevola DeactivateSkill(), proto
  // zustanou zvednute reference...)
  
  // otestujeme spravnost pointeru pUnit
  ASSERT(pUnit != NULL);
  ASSERT_KINDOF(CSUnit, pUnit);
  // sezen pointer na data skilly u jednotky
  SSBulletAttackSkill *pUnitData = SKILLDATA(pUnit);
  // otestuj pointer
  ASSERT(::AfxIsValidAddress(pUnitData, sizeof(SSBulletAttackSkill), TRUE));
  
  // kdyz byla skilla aktivni, tak si drzela referenci
  // na napadenou jednotku
  if (pUnitData->m_pAttackedUnit != NULL)
    pUnitData->m_pAttackedUnit->Release();
  
  // nakonec zavolame predka
  CSAbstractAttackSkillType::DestructSkill(pUnit);
}


////////////////////////////////////////////////////////////////////////
// Save/Load dat typu skilly
////////////////////////////////////////////////////////////////////////

void CSBulletAttackSkillType::PersistentSaveSkillType(CPersistentStorage &storage)
{
  BRACE_BLOCK(storage);
  
  // otestuj data typu skilly
  ASSERT_VALID(this);
  
  // prvne ulozime data predka
  CSAbstractAttackSkillType::PersistentSaveSkillType(storage);
  
  // pak ulozime svoje data
  storage << m_dwLoadTime << m_dwFirePeriod << m_nAttackIntensity
    << m_dwAppearanceID << m_dwAnimationOffset << m_dwAnimationLength;
}


void CSBulletAttackSkillType::PersistentLoadSkillType(CPersistentStorage &storage)
{
  BRACE_BLOCK(storage);
  
  // nejprve natahneme data predka
  CSAbstractAttackSkillType::PersistentLoadSkillType(storage);
  
  // otestujeme, ze se natahly smysluplna data
  LOAD_ASSERT(m_dwAttackRadius >= SM_BULLETATTACK_SKILLTYPE_RADIUS_MIN);
  LOAD_ASSERT(m_dwAttackRadius <= SM_BULLETATTACK_SKILLTYPE_RADIUS_MAX);
  LOAD_ASSERT(SM_BULLETATTACK_SKILLTYPE_ALTITUDE_MIN <= m_dwAttackMinAltitude);
  LOAD_ASSERT(m_dwAttackMinAltitude <= m_dwAttackMaxAltitude);
  LOAD_ASSERT(m_dwAttackMaxAltitude <= SM_BULLETATTACK_SKILLTYPE_ALTITUDE_MAX);
  
  // natahneme svoje vlastni data ze souboru
  storage >> m_dwLoadTime >> m_dwFirePeriod >> m_nAttackIntensity
    >> m_dwAppearanceID >> m_dwAnimationOffset >> m_dwAnimationLength;
  
  // otestujeme spravnost
  LOAD_ASSERT(m_dwLoadTime <= SM_BULLETATTACK_SKILLTYPE_LOADTIME_MAX);
  LOAD_ASSERT(m_dwFirePeriod <= SM_BULLETATTACK_SKILLTYPE_FIREPERIOD_MAX);
  LOAD_ASSERT(m_nAttackIntensity >= SM_BULLETATTACK_SKILLTYPE_INTENSITY_MIN);
  LOAD_ASSERT(m_nAttackIntensity <= SM_BULLETATTACK_SKILLTYPE_INTENSITY_MAX);
  LOAD_ASSERT(m_dwAppearanceID != 0);
  LOAD_ASSERT(m_dwAnimationOffset <= m_dwLoadTime);
  LOAD_ASSERT(m_dwAnimationLength <= m_dwFirePeriod);
  LOAD_ASSERT(m_dwAnimationOffset <= m_dwAnimationLength);
}


void CSBulletAttackSkillType::PersistentTranslatePointersSkillType(CPersistentStorage &storage)
{
  // zavolame predka
  CSAbstractAttackSkillType::PersistentTranslatePointersSkillType(storage);
  
  // jinak nic...
}


void CSBulletAttackSkillType::PersistentInitSkillType()
{
  // zavolame predka
  CSAbstractAttackSkillType::PersistentInitSkillType();
  
  // dopocitame konec animace
  m_dwAnimationEnd = m_dwFirePeriod - (m_dwAnimationLength - m_dwAnimationOffset);
}


//////////////////////////////////////////////////////////////////////
// Save/Load dat skilly u jednotky
//////////////////////////////////////////////////////////////////////

void CSBulletAttackSkillType::PersistentSaveSkill(CPersistentStorage &storage, CSUnit *pUnit)
{
  BRACE_BLOCK(storage);
  
  // otestujeme spravnost pointeru pUnit
  ASSERT(pUnit != NULL);
  ASSERT_KINDOF(CSUnit, pUnit);
  // sezen pointer na data skilly u jednotky
  SSBulletAttackSkill *pUnitData = SKILLDATA(pUnit);
  // otestuj pointer
  ASSERT(AfxIsValidAddress(pUnitData, sizeof(SSBulletAttackSkill), TRUE));
  // otestuj data skilly u jednotky
  ASSERT_VALID_UNIT(pUnitData);
  
  // nejprve zavolame predka
  CSAbstractAttackSkillType::PersistentSaveSkill(storage, pUnit);
  
  // pak ulozime data do souboru
  storage << pUnitData->m_dwRemainingTime << pUnitData->m_pAttackedUnit;
}

void CSBulletAttackSkillType::PersistentLoadSkill(CPersistentStorage &storage, CSUnit *pUnit)
{
  BRACE_BLOCK(storage);
  
  // otestujeme spravnost pointeru pUnit
  ASSERT(pUnit != NULL);
  ASSERT_KINDOF(CSUnit, pUnit);
  // sezen pointer na data skilly u jednotky
  SSBulletAttackSkill *pUnitData = SKILLDATA(pUnit);
  // otestuj pointer
  ASSERT(AfxIsValidAddress(pUnitData, sizeof(SSBulletAttackSkill), TRUE));
  
  // nejprve zavolame predka
  CSAbstractAttackSkillType::PersistentLoadSkill(storage, pUnit);
  
  // pak natahneme data ze souboru
  storage >> pUnitData->m_dwRemainingTime >> (void *&)pUnitData->m_pAttackedUnit;
  
  // otestujeme natazena data ...
  // ... neni co testovat
}

void CSBulletAttackSkillType::PersistentTranslatePointersSkill(CPersistentStorage &storage, CSUnit *pUnit)
{
  // otestujeme spravnost pointeru pUnit
  ASSERT(pUnit != NULL);
  ASSERT_KINDOF(CSUnit, pUnit);
  // sezen pointer na data skilly u jednotky
  SSBulletAttackSkill *pUnitData = SKILLDATA(pUnit);
  // otestuj pointer
  ASSERT(AfxIsValidAddress(pUnitData, sizeof(SSBulletAttackSkill), TRUE));
  
  // nejprve zavolame predka
  CSAbstractAttackSkillType::PersistentTranslatePointersSkill(storage, pUnit);
  
  // mame v sobe ulozeny pointer na napadenou jednotku,
  // takze kdyz je neNULL, tak ho nechame prelozit
  pUnitData->m_pAttackedUnit = (CSUnit *) storage.TranslatePointer(pUnitData->m_pAttackedUnit);
  
  // otestujeme, ze se to rozumne prelozilo
  if (pUnitData->m_pAttackedUnit != NULL)
  {
    LOAD_ASSERT(pUnitData->m_pAttackedUnit->IsKindOf(RUNTIME_CLASS(CSUnit)));
    
    // pridame refcount
//    pUnitData->m_pAttackedUnit->AddRef();
  }
}

void CSBulletAttackSkillType::PersistentInitSkill(CSUnit *pUnit)
{
  // otestujeme parametr
  ASSERT(pUnit != NULL);
  ASSERT_KINDOF(CSUnit, pUnit);
  
  // zavolame predka
  CSAbstractAttackSkillType::PersistentInitSkill(pUnit);
}


////////////////////////////////////////////////////////////////////////
// Aktivace skilly (zapnuti DoTimeSlice)
////////////////////////////////////////////////////////////////////////

void CSBulletAttackSkillType::ActivateSkill(CSUnit *pUnit, CSUnit *pAttackedUnit, CISysCallAttack *pSysCall)
{
  // otestujeme spravnost pointeru
  ASSERT(pUnit != NULL);
  ASSERT_KINDOF(CSUnit, pUnit);
  ASSERT(pAttackedUnit != NULL);
  ASSERT_KINDOF(CSUnit, pAttackedUnit);
  ASSERT(pSysCall != NULL);
  ASSERT_KINDOF(CISysCallAttack, pSysCall);
  
  // sezeneme pointer na data skilly u jednotky
  SSBulletAttackSkill *pUnitData = SKILLDATA(pUnit);
  // otestujeme pointer
  ASSERT(AfxIsValidAddress(pUnitData, sizeof(SSBulletAttackSkill), TRUE));
  // otestujeme data skilly u jednotky
  ASSERT_VALID_UNIT(pUnitData);
  
  // je jiz attack skilla aktivni?
  if (IsSkillActive(pUnit))
  {
    // utoci se na jednotku, na kterou ted chceme strilet?
    if (pAttackedUnit == pUnitData->m_pAttackedUnit)
    {
      // nedelame nic, vzdyt uz utocime...
      return;
    }
    else
    {
      // zastavime palbu (pozdeji zahajime palbu jinam...)
      // BTW toto ustreli skript, ktery volal predchozi attack
      DeactivateSkill(pUnit);
    }
  }
  
  // OK, ted vime, ze skilla neni aktivni
  
  // inicializujeme casovani strelby
  if (g_cMap.GetTimeSlice() < pUnitData->m_dwRemainingTime)
  {
    pUnitData->m_dwRemainingTime -= g_cMap.GetTimeSlice();
  }
  else
  {
    pUnitData->m_dwRemainingTime = 0;
  }
  
  // pridame jeste cas, ktery potrebuje k priprave prvniho vystrelu
  pUnitData->m_dwRemainingTime += m_dwLoadTime;
  
  // nyni mame spocitany presne cas na nabiti pred prvnim vystrelem
  
  // zvysime pocet referenci na jednotce, na kterou budeme strilet
  pAttackedUnit->AddRef();
  // ulozime si pointer na jednotku, na kterou budeme strilet
  pUnitData->m_pAttackedUnit = pAttackedUnit;
  
  // to je zatim cele, DoTimeSlice() bude odcitat, pri nule vypali
  // a counter nastavi zpatky na delku periody
  
  // zavolame predka (ten fyzicky na serveru aktivuje skillu)
  CSAbstractAttackSkillType::ActivateSkill(pUnit, pSysCall);
}


////////////////////////////////////////////////////////////////////////
// Deaktivace skilly (vypnuti DoTimeSlice)
////////////////////////////////////////////////////////////////////////

void CSBulletAttackSkillType::DeactivateSkill(CSUnit *pUnit)
{
  // otestujeme spravnost pointeru pUnit
  ASSERT(pUnit != NULL);
  
  // sezeneme pointer na data skilly u jednotky
  SSBulletAttackSkill *pUnitData = SKILLDATA(pUnit);
  // otestujeme pointer
  ASSERT(AfxIsValidAddress(pUnitData, sizeof(SSBulletAttackSkill), TRUE));
  // otestujeme data skilly u jednotky
  ASSERT_VALID_UNIT(pUnitData);
  
  // potrebujeme si k jednotce ulozit cislo timeslicu, kdy
  // bude mozne tuto attack skillu zase aktivovat
  // (presneji: kdyz to udela driv, tak mu akorat
  // opozdime prvni vystrel -- to resi ActivateSkill() --
  // aby se nedalo "sidit" opakovanym aktivovanim
  // a deaktivovanim strilet rychleji)
  pUnitData->m_dwRemainingTime += g_cMap.GetTimeSlice();
  
  // snizime pocet referenci napadnute jednotky
  pUnitData->m_pAttackedUnit->Release();
  // vynulujeme pointer na napadenou jednotku
  pUnitData->m_pAttackedUnit = NULL;
  
  // zavolame predka (ten fyzicky na serveru deaktivuje skillu)
  CSAbstractAttackSkillType::DeactivateSkill(pUnit);
}


////////////////////////////////////////////////////////////////////////
// DoTimeSlice
////////////////////////////////////////////////////////////////////////

void CSBulletAttackSkillType::DoTimeSlice(CSUnit *pUnit)
{
  // otestujeme spravnost pointeru pUnit
  ASSERT(pUnit != NULL);
  
  // sezeneme pointer na data skilly u jednotky
  SSBulletAttackSkill *pUnitData = SKILLDATA(pUnit);
  // otestujeme pointer
  ASSERT(AfxIsValidAddress(pUnitData, sizeof(SSBulletAttackSkill), TRUE));
  // otestujeme data skilly u jednotky
  ASSERT_VALID_UNIT(pUnitData);
  ASSERT(pUnitData->m_pAttackedUnit != NULL);
  
  // zjistime vzdalenost a vysku napadene jednotky
  VERIFY(pUnitData->m_pAttackedUnit->PrivilegedThreadReaderLock());
  CPointDW pointPosition = pUnitData->m_pAttackedUnit->GetPosition();
  DWORD dwAltitude = pUnitData->m_pAttackedUnit->GetVerticalPosition();
  pUnitData->m_pAttackedUnit->PrivilegedThreadReaderUnlock();
  
  // kazdy timeslice nastavime smer jednotky na napadenou jednotku
  if (g_cMap.IsMapPosition(pointPosition))
    pUnit->SetDirection(pointPosition);
  
    /*
    // kdyz je counter vetsi nez nula, tak jen menime animace a testujeme, jestli
    // uz nahodou neni napadena jednotka mrtva
    if (pUnitData->m_dwRemainingTime > 0)
    {
    // pokud je delka vystrelu vetsi, nez delka animace,
    // tak bude animace na chvili vypnuta a jen se bude testovat
    // smrt nepritele
    if (m_dwFirePeriod > m_dwAnimationLength)
    {
    // nemame nahodou ukoncit animaci predchoziho vystrelu?
    if (pUnitData->m_dwRemainingTime == m_dwAnimationEnd)
    {
    pUnit->SetDefaultAppearanceID();
    }
    
      // kdyz zrovna neanimujeme (tj. zbyvajici cas je neco mezi vypnutim
      // a zapnutim animace), tak testujeme smrt nepritele
      if ((m_dwAnimationEnd <= pUnitData->m_dwRemainingTime) &&
      (pUnitData->m_dwRemainingTime <= m_dwAnimationOffset) &&
      (pUnitData->m_pAttackedUnit->GetLives() <= 0))
      {
      // nastavime vysledek syscallu na uspech
      pUnitData->m_pSysCall->GetBag()->ReturnBool(true);
      // ukoncit syscall
      pUnitData->m_pSysCall->FinishWaiting(STATUS_SUCCESS);
      // zapomenout na syscall (sam se zrusil)
      pUnitData->m_pSysCall = NULL;
      
        // nastavovat defaultni vzhled nemusime, to uz je hotove
        
          // deaktivujeme attack skillu
          DeactivateSkill(pUnit);
          
            return;
            }
            
              // nemame nahodou zahajit animaci jednoho vystrelu?
              if (pUnitData->m_dwRemainingTime == m_dwAnimationOffset)
              {
              pUnit->SetAppearanceID(m_dwAppearanceID);
              }
              }
              else
              {
              // delka animace je stejna jako delka vystrelu, takze
              // s animaci nedelame nic, jen testujeme, jestli neni
              // nepratelska jednotka mrtva;
              // to delame prave v okamziku, kdy animace "konci" a "zacina"
              if ((pUnitData->m_dwRemainingTime == m_dwAnimationOffset) &&
              (pUnitData->m_pAttackedUnit->GetLives() <= 0))
              {
              // nastavime vysledek syscallu na uspech
              pUnitData->m_pSysCall->GetBag()->ReturnBool(true);
              // ukoncit syscall
              pUnitData->m_pSysCall->FinishWaiting(STATUS_SUCCESS);
              // zapomenout na syscall (sam se zrusil)
              pUnitData->m_pSysCall = NULL;
              
                // musime nastavit defaultni vzhled
                pUnit->SetDefaultAppearanceID();
                
                  // deaktivujeme attack skillu
                  DeactivateSkill(pUnit);
                  
                    return;
                    }
                    }
                    
                      pUnitData->m_dwRemainingTime--;
                      return;
                      }
  */
  
  // kdyz je okamzik mezi dvema animacemi, tak testujeme vzdalenost a smrt nepritele
  if ((m_dwAnimationEnd >= pUnitData->m_dwRemainingTime) &&
    (pUnitData->m_dwRemainingTime >= m_dwAnimationOffset))
  {
    // je nepratelska jednotka mrtva?
    if (pUnitData->m_pAttackedUnit->GetLives() <= 0)
    {
      // nastavime vysledek syscallu na uspech
      pUnitData->m_pSysCall->GetBag()->ReturnBool(true);
      // ukoncit syscall
      pUnitData->m_pSysCall->FinishWaiting(STATUS_SUCCESS);
      // zapomenout na syscall 
      pUnitData->m_pSysCall->Release();
      pUnitData->m_pSysCall = NULL;
      
      // nastavime defaultni vzhled
      pUnit->SetDefaultAppearanceID();
      
      // deaktivujeme attack skillu
      DeactivateSkill(pUnit);
      
      return;
    }
    
    // spocitame vzdalenost nepratelske jednotky
    DWORD dwDistance = pointPosition.GetDistanceSquare(pUnit->GetPosition());
    
    // otestujeme, jestli je jednotka porad jeste v dosahu
    // nasi palby; kdyz neni, tak deaktivujeme skillu a nastavime
    // v syscallu navratovou hodnotu
    if ((dwDistance > m_dwAttackRadiusSquare) ||
      (m_dwAttackMinAltitude > dwAltitude) ||
      (m_dwAttackMaxAltitude < dwAltitude))
    {
      // nastavit spravnou navratovou hodnotu do syscallu
      pUnitData->m_pSysCall->GetBag()->ReturnBool(false);
      // ukoncit syscall
      pUnitData->m_pSysCall->FinishWaiting(STATUS_SUCCESS);
      // zapomenout na syscall 
      pUnitData->m_pSysCall->Release();
      pUnitData->m_pSysCall = NULL;
      
      // nastavime defaultni vzhled
      pUnit->SetDefaultAppearanceID();
      
      // deaktivujeme attack skillu
      DeactivateSkill(pUnit);
      
      return;
    }
  }
  
  // otestujeme konec animace
  // (toto se musi udelat PRED zahajenim animace; kdyz je
  // totiz animace stejne dlouha jako delka vystrelu,
  // tak by byla porad zapnuta a klient, kdyz by animaci
  // explicitne neloopoval, tak by se provedla jen jednou)
  if (pUnitData->m_dwRemainingTime == m_dwAnimationEnd)
  {
    // koncime animaci
    pUnit->SetDefaultAppearanceID();
  }
  
  // otestujeme zacatek animace
  if (pUnitData->m_dwRemainingTime == m_dwAnimationOffset)
  {
    // zaciname animovat
    pUnit->SetAppearanceID(m_dwAppearanceID);
  }
  
  // otestujeme, jestli je spravny okamzik na samotny vystrel
  if (pUnitData->m_dwRemainingTime > 0)
  {
    pUnitData->m_dwRemainingTime--;
    return;
  }
  
  // OK. Jsme na nule. Jdeme palit. HURAAA!!!
  
  // prvne zase nahodime counter pro dalsi vystrel, at se na to nezapomene
  pUnitData->m_dwRemainingTime = m_dwFirePeriod;
  
  // Ted prolezeme VSECHNY defense skilly, kazde rekneme,
  // jak moc do ni strilime, ony nam vrati, kolik ubrani,
  // takze se postupne bude snizovat sila naseho utoku.
  // Nakonec, kdyz zbyde jeste nejaka utocna sila,
  // tak sekneme primo do ziveho masa...
  
  DWORD dwSkillIndex = 0;
  CSSkillType *pSkillType;
  int nAttackIntensity = m_nAttackIntensity;
  
  // nejprve potrebujeme zamknout napadenou jednotku na zapis,
  // protoze na ni budeme volat defense skilly, ktere mohou
  // na jednotce neco delat, no a nakonec doufejme taky neco odecteme
  VERIFY(pUnitData->m_pAttackedUnit->PrivilegedThreadWriterLock());
  
  while ((pSkillType = pUnitData->m_pAttackedUnit->GetUnitType()->GetNextSkillType(dwSkillIndex, Group_Defense)) != NULL)
  {
    // paranoia
    ASSERT_KINDOF(CSAbstractDefenseSkillType, pSkillType);
    
    // nasli jsme nejakou defense skillu,
    // zavolame na ni DoDefense()
    nAttackIntensity -= ((CSAbstractDefenseSkillType *)pSkillType)->DoDefense(pUnitData->m_pAttackedUnit, pUnit, m_nAttackIntensity, Interaction_Bullet);
  }
  
  // vyborne, mame spocitano, kolik opravdu ublizi tento utok napadene jednotce,
  // ted to odecteme; kdyz uz te jednotce nateklo do bot, tak ji server
  // sam zabije, o to se nebudeme starat...
  pUnitData->m_pAttackedUnit->DecreaseLives(nAttackIntensity);
  
  // posleme jednotce notifikaci, ze se na ni pali
  // ??? jak se ma resit viditelnost agresora?
  NotifyAttackedUnit(pUnit, pUnitData->m_pAttackedUnit, nAttackIntensity);
  
  // tak to je cely, co jsme s tou jednotkou chteli udelat,
  // odemkneme a smytec...
  pUnitData->m_pAttackedUnit->PrivilegedThreadWriterUnlock();
}


////////////////////////////////////////////////////////////////////////
// Full Info
////////////////////////////////////////////////////////////////////////

void CSBulletAttackSkillType::FillFullInfo(CSUnit *pUnit, CString &strInfo)
{
  // je skilla enabled na dane jednotce?
  if (IsEnabled(pUnit))
  {
    strInfo.Format("Bullet Attack:\n  Period: %u\n  Intensity: %i", m_dwFirePeriod, m_nAttackIntensity);
  }    
  else
  {
    strInfo = "Bullet Attack:\n  (disabled)";
  }
}

void CSBulletAttackSkillType::FillEnemyFullInfo(CSUnit *pUnit, CString &strInfo)
{
  // vyplnime jen fakt, ze jednotka ma bullet attack
  strInfo = "Bullet Attack Skill";
}


////////////////////////////////////////////////////////////////////////
// Interface ze skriptu - metoda "Fire"
////////////////////////////////////////////////////////////////////////

ESyscallResult CSBulletAttackSkillType::SCI_Fire(CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, CZUnit *pAttackedUnit)
{
  // zkontrolujeme parametry
  ASSERT(pUnit != NULL);
  ASSERT_KINDOF(CSUnit, pUnit);
  ASSERT(pAttackedUnit != NULL);
  ASSERT_KINDOF(CZUnit, pAttackedUnit);
  
  TRACE_SKILLCALL("@ BulletAttack.Fire(%d) called on unit %d", pAttackedUnit->GetSUnit()->GetID(), pUnit->GetID());
  
  // vytvorime syscall
  CISysCallAttack *pSysCall = new CISysCallAttack;
  // vyplnime syscall daty
  VERIFY(pSysCall->Create(pProcess, pUnit, this, pBag));
  // aktivujeme skillu
  ActivateSkill(pUnit, pAttackedUnit->GetSUnit(), pSysCall);
  
  // vratime pointer syscall interpretu (je addrefovany z Create)
  *ppSysCall = pSysCall;
  
  // uspime proces, protoze to bude dlouho trvat
  return SYSCALL_SUCCESS_SLEEP;
}


////////////////////////////////////////////////////////////////////////
// Debugovaci metody
////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

void CSBulletAttackSkillType::AssertValid() const
{
  // zavolame predka
  CSAbstractAttackSkillType::AssertValid();
  
  ASSERT(m_dwLoadTime <= 1000);
  ASSERT(m_dwFirePeriod <= 1000);
  ASSERT(m_nAttackIntensity >= 1);
  ASSERT(m_nAttackIntensity <= 10000);
  ASSERT(m_dwAttackRadius >= 1);
  ASSERT(m_dwAttackRadius <= 128);
  ASSERT(m_dwAttackRadiusSquare == m_dwAttackRadius * m_dwAttackRadius);
}

void CSBulletAttackSkillType::AssertValidUnit(SSBulletAttackSkill *pUnitData) const
{
  // zavolame predka
  CSAbstractAttackSkillType::AssertValidUnit(pUnitData);
  
  // pointer na napadenou jednotku je bud NULL nebo neco poradneho
  if (pUnitData->m_pAttackedUnit != NULL)
    ASSERT_KINDOF(CSUnit, pUnitData->m_pAttackedUnit);
}

void CSBulletAttackSkillType::Dump(CDumpContext &dc) const
{
  // zavolame predka
  CSAbstractAttackSkillType::Dump(dc);
  
  dc << "Bullet attack skill type:" << endl;
  dc << "  load time: " << m_dwLoadTime << endl;
  dc << "  period: " << m_dwFirePeriod << endl;
  dc << "  intensity: " << m_nAttackIntensity << endl;
  dc << "  radius: " << m_dwAttackRadius << endl;
}

#endif  // _DEBUG
