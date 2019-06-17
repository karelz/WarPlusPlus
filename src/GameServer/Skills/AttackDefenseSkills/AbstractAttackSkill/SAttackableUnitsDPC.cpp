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
#include "SAttackableUnitsDPC.h"

#include "GameServer\GameServer\SDeferredProcedureCall.inl"
#include "GameServer\Skills\SysCalls\IDPCSysCall.h"
#include "TripleS\Interpret\Src\SysObjs\ISOSet.h"
#include "GameServer\GameServer\SUnitList.h"

IMPLEMENT_DYNAMIC(CSAttackableUnitsDPC, CSSysCallDPC)
IMPLEMENT_DPC(CSAttackableUnitsDPC, 1)

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSAttackableUnitsDPC::CSAttackableUnitsDPC() :
m_pSkillType(NULL)
{
}

// konstruktor
CSAttackableUnitsDPC::CSAttackableUnitsDPC(CIProcess *pProcess, CIDataStackItem *pReturnValue, CSUnit *pUnit, CSAbstractAttackSkillType *pSkillType) :
CSSysCallDPC(pProcess, pReturnValue, pUnit)
{
  ASSERT(pSkillType != NULL);
  ASSERT_KINDOF(CSSkillType, pSkillType);
  
  m_pSkillType = pSkillType;
}


// destruktor
CSAttackableUnitsDPC::~CSAttackableUnitsDPC() 
{
}


//////////////////////////////////////////////////////////////////////
// Ukladani dat DPC
//////////////////////////////////////////////////////////////////////

// ulozeni dat
void CSAttackableUnitsDPC::PersistentSaveDPC(CPersistentStorage &storage)
{
  ASSERT_VALID(this);
  
  BRACE_BLOCK(storage);
  
  // ulozime data predka
  CSSysCallDPC::PersistentSaveDPC(storage);
  
  // ulozime svoje data
  storage << (void *)m_pSkillType;
}

// nahrávání pouze uložených dat
void CSAttackableUnitsDPC::PersistentLoadDPC(CPersistentStorage &storage)
{
  BRACE_BLOCK(storage);
  
  // prvne natahneme data predka
  CSSysCallDPC::PersistentLoadDPC(storage);
  
  // natahneme svoje vlastni data
  storage >> (void *&)m_pSkillType;
  LOAD_ASSERT(m_pSkillType != NULL);
}

// preklad ukazatelu
void CSAttackableUnitsDPC::PersistentTranslatePointersDPC(CPersistentStorage &storage)
{
  // zavolame predka
  CSSysCallDPC::PersistentTranslatePointersDPC(storage);
  
  m_pSkillType = (CSAbstractAttackSkillType *) storage.TranslatePointer(m_pSkillType);
  
  LOAD_ASSERT(m_pSkillType->IsKindOf(RUNTIME_CLASS(CSAbstractAttackSkillType)));
}

// inicializace nahraneho DPC
void CSAttackableUnitsDPC::PersistentInitDPC() 
{
  // zavolame predka
  CSSysCallDPC::PersistentInitDPC();
  
  // sami nedelame nic...
}


//////////////////////////////////////////////////////////////////////
// Provedeni odlozeneho volani procedury
//////////////////////////////////////////////////////////////////////

void CSAttackableUnitsDPC::Run() 
{
  ASSERT_VALID(this);
  
  VERIFY( m_pDPCSysCall->Lock());

  // pripravime si spojak jednotek
  CSUnitList cUnitList;

  // okopirujeme si pozici jednotky
  CPointDW pointCenter = GetUnit()->GetPosition();

  if (g_cMap.IsMapPosition(pointCenter))
  {
    // na mape zjistime vsechny nepratelske jednotky v danem polomeru
    g_cMap.GetUnitsInAreaEnemy(
      GetRunningDPCCivilizationIndex(),
      pointCenter,
      m_pSkillType->m_dwAttackRadius,
      cUnitList);
  }

  if (m_pDPCSysCall->IsValid())
  {
    // inicializace mnoziny
    CISOSet<CZUnit *> *pSet = (CISOSet<CZUnit *> *) m_pDPCSysCall->GetReturnValue()->GetSystemObject();
    ASSERT(pSet != NULL);

    CSUnit *pUnit;
    // cyklus pres vsechny vracene jednotky
    while (cUnitList.RemoveFirst(pUnit))
    {
      // zjistime vysku jednotky
      //     nebudeme zamykat, protoze je to jen jeden DWORD
      DWORD dwAltitude = pUnit->GetVerticalPosition();
     
      // kdyz padne do vyskoveho intervalu, tak ji pridame do vysledne mnoziny
      if ((m_pSkillType->m_dwAttackMinAltitude <= dwAltitude) &&
        (dwAltitude <= m_pSkillType->m_dwAttackMaxAltitude))
      {
        pSet->AddItemToSet(pUnit->GetZUnit());
      }
    }
  }
  
  VERIFY(m_pDPCSysCall->Unlock());

  m_pDPCSysCall->FinishWaiting(STATUS_SUCCESS);
  m_pDPCSysCall->Release();
  m_pDPCSysCall = NULL;
}


//////////////////////////////////////////////////////////////////////
// Debugovaci metody
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

void CSAttackableUnitsDPC::AssertValid()
{
  // zavolame predka
  CSSysCallDPC::AssertValid();
  
  // assertujeme vlastni data
  ASSERT(m_pSkillType != NULL);
  ASSERT_KINDOF(CSAbstractAttackSkillType, m_pSkillType);
}

#endif  // _DEBUG