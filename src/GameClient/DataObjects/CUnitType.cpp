#include "stdafx.h"
#include "CUnitType.h"

#include "UnitCache.h"
#include "CUnit.h"
#include "CCivilization.h"

IMPLEMENT_DYNAMIC(CCUnitType, CIDCachedObject)

CCUnitType::CCUnitType()
{
  m_pGeneralUnitType = NULL;
  m_pFirstAppearance = NULL;
  m_pCivilization = NULL;
  m_pSelectionMarkAppearance = NULL;
}

CCUnitType::~CCUnitType()
{
  ASSERT(m_pGeneralUnitType == NULL);
  ASSERT(m_pCivilization == NULL);
  ASSERT(m_pFirstAppearance == NULL);
  ASSERT(m_pSelectionMarkAppearance == NULL);
}

// debug functions
#ifdef _DEBUG

void CCUnitType::AssertValid() const
{
  CIDCachedObject::AssertValid();
  ASSERT(m_pGeneralUnitType != NULL);
  ASSERT(m_pCivilization != NULL);
}

void CCUnitType::Dump(CDumpContext &dc) const
{
  CIDCachedObject::Dump(dc);
}

#endif

void CCUnitType::CreateInstance(CCUnit *pUnit, CUnitCache *pUnitCache)
{
  ASSERT(pUnit != NULL);

  pUnit->Create(this, pUnitCache);
}

void CCUnitType::Create(CCGeneralUnitType *pGeneralUnitType, CCCivilization *pCivilization)
{
  ASSERT_VALID(pCivilization);
  ASSERT_VALID(pGeneralUnitType);

  m_pGeneralUnitType = pGeneralUnitType;
  m_pCivilization = pCivilization;

  SetID(pGeneralUnitType->GetID());
}

void CCUnitType::Delete()
{
  Unload();
  m_pGeneralUnitType = NULL;
  m_pCivilization = NULL;
}

void CCUnitType::Load()
{
  m_pGeneralUnitType->LoadUnitType(this);
}

void CCUnitType::Unload()
{
  CCUnitAppearance *pAppearance = m_pFirstAppearance, *pDelAppearance;
  while(pAppearance != NULL){
    pDelAppearance = pAppearance;
    pAppearance = pAppearance->m_pNext;
    pDelAppearance->Delete();
    delete pDelAppearance;
  }
  m_pFirstAppearance = NULL;

  if(m_pSelectionMarkAppearance != NULL){
    m_pSelectionMarkAppearance->Delete();
    delete m_pSelectionMarkAppearance;
    m_pSelectionMarkAppearance = NULL;
  }
}

CCUnitAppearance *CCUnitType::GetAppearance(DWORD dwID)
{
  CCUnitAppearance *pAppearance;

  pAppearance = m_pFirstAppearance;
  while(pAppearance != NULL){
    if(pAppearance->GetGeneralUnitAppearance()->GetID() == dwID){
      return pAppearance;
    }
    pAppearance = pAppearance->m_pNext;
  }
  return NULL;
}