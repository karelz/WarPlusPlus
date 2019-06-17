#include "stdafx.h"
#include "CCivilization.h"

#include "Common\Map\Map.h"

#include "CUnitType.h"
#include "..\LoadException.h"

IMPLEMENT_DYNAMIC(CCCivilization, CObject);

// constructor
CCCivilization::CCCivilization()
{
  m_dwID = 0x0FFFFFFFF;
  m_pUnitTypes = NULL;
}

// destructor
CCCivilization::~CCCivilization()
{
  ASSERT(m_pUnitTypes == NULL);
}

// Debug functions
#ifdef _DEBUG

void CCCivilization::AssertValid() const
{
  CObject::AssertValid();
}

void CCCivilization::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);

  dc << "Civilization name : " << m_strName << "\n";
}

#endif


// Implementation ----------------------------------------

void CCCivilization::Create(CArchiveFile MapFile, DWORD dwIndex)
{
  SCivilizationHeader Header;

  m_dwIndex = dwIndex;

  // load the header
  LOAD_ASSERT ( MapFile.Read(&Header, sizeof(Header)) == sizeof ( Header ) );

  // copy data
  m_dwID = Header.m_dwID;
  m_dwColor = Header.m_dwColor;
  m_strName = Header.m_aName;

  // OK all was done
}

void CCCivilization::Delete()
{
  CCUnitType *pUnitType, *pDelUnitType;

  pUnitType = m_pUnitTypes;
  while(pUnitType != NULL){
    pDelUnitType = pUnitType;
    pUnitType = pUnitType->m_pNextUnitType;
    m_UnitTypeCache.RemoveObject(pDelUnitType);
    pDelUnitType->Delete();
    delete pDelUnitType;
  }
  m_pUnitTypes = NULL;
}

CCUnitType * CCCivilization::CreateUnitType(CCGeneralUnitType *pGeneralUnitType)
{
  ASSERT(pGeneralUnitType != NULL);
  CCUnitType *pUnitType = pGeneralUnitType->CreateInstance(this);

  VERIFY(m_lockUnitTypes.Lock());
  pUnitType->m_pNextUnitType = m_pUnitTypes;
  m_pUnitTypes = pUnitType;
  VERIFY(m_lockUnitTypes.Unlock());

  m_UnitTypeCache.InsertObject(pUnitType);

  return pUnitType;
}

void CCCivilization::RemoveUnitType(CCUnitType *pUnitType)
{
  ASSERT(pUnitType != NULL);
  m_UnitTypeCache.RemoveObject(pUnitType);

  VERIFY(m_lockUnitTypes.Lock());
  CCUnitType *pUT = m_pUnitTypes;
  if(m_pUnitTypes == pUnitType){
    m_pUnitTypes = pUnitType->m_pNextUnitType;
  }
  else{
    while(pUT != NULL){
      if(pUT->m_pNextUnitType == pUnitType){
        pUT->m_pNextUnitType = pUnitType->m_pNextUnitType;
        break;
      }
      pUT = pUT->m_pNextUnitType;
    }
  }
  VERIFY(m_lockUnitTypes.Unlock());
}