#include "stdafx.h"
#include "CGeneralUnitType.h"

#include "Common\Map\Map.h"

#include "CUnitType.h"
#include "CGeneralUnitAppearance.h"
#include "..\LoadException.h"

IMPLEMENT_DYNAMIC(CCGeneralUnitType, CObject);

// constructor
CCGeneralUnitType::CCGeneralUnitType()
{
  m_pAppearances = NULL;
  m_dwID = 0;
  m_pSelectionMarkAppearance = NULL;
}

// destructor
CCGeneralUnitType::~CCGeneralUnitType()
{
  ASSERT(m_dwID == 0);
  ASSERT(m_pAppearances == NULL);
  ASSERT(m_pSelectionMarkAppearance == NULL);
}

// debug functions
#ifdef _DEBUG

void CCGeneralUnitType::AssertValid() const
{
  CObject::AssertValid();
  ASSERT(m_dwID != 0);
}

void CCGeneralUnitType::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
  dc << "Unit type name : " << m_strName << "\n";
}

#endif

// Implementation -----------------------------------

void CCGeneralUnitType::Create(CArchiveFile UnitTypeFile, CDataArchive cArchive_GraphicsClone)
{
  SUnitTypeHeader Header;

  // read the header
  LOAD_ASSERT ( UnitTypeFile.Read ( &Header, sizeof ( Header ) ) == sizeof ( Header ) );

  // copy data
  m_dwID = Header.m_dwID;
  m_strName = Header.m_pName;
  m_dwFlags = Header.m_dwFlags;
  {
    int i;
    for(i = 0; i < 8; i++){
      m_aModeNames[i] = Header.m_aModeNames[i];
      m_aMoveWidth [ i ] = Header.m_aMoveWidth [ i ];
    }
  }

  // load appearances
  {
    DWORD dwAppearance;
    CCGeneralUnitAppearance *pAppearance;

    // loop through all appearances in the file and create them
    for(dwAppearance = 0; dwAppearance < Header.m_dwAppearancesNumber; dwAppearance ++){
      // allocate new appearance object
      pAppearance = new CCGeneralUnitAppearance();

      // load it from the file
      pAppearance->Create(UnitTypeFile, cArchive_GraphicsClone);

      // if it has some special ID - remember it somewhere else
      if(pAppearance->GetID() == 0){
        m_pSelectionMarkAppearance = pAppearance;
      }
      else{
        // Add it to our list
        pAppearance->m_pNext = m_pAppearances;
        m_pAppearances = pAppearance;
      }
    }
  }

  ASSERT(m_pSelectionMarkAppearance != NULL);

  // fill the default appearance array
  memcpy(m_aDefaultAppearances, Header.m_aDefaultAppearances, 8 * sizeof(DWORD));

  // load skill types
  // I think no skill types will be loaded here
  // cause all the data must be recieved from the server
}

void CCGeneralUnitType::Delete()
{
  m_dwID = 0;

  // go through all appearances and delete them
  {
    CCGeneralUnitAppearance *pAppearance = m_pAppearances, *pDel;

    while(pAppearance != NULL){
      pDel = pAppearance;
      pAppearance = pAppearance->m_pNext;

      pDel->m_pNext = NULL;
      pDel->Delete();
      delete pDel;
    }

    m_pAppearances = NULL;

    if ( m_pSelectionMarkAppearance != NULL )
    {
      m_pSelectionMarkAppearance->Delete();
      delete m_pSelectionMarkAppearance;
      m_pSelectionMarkAppearance = NULL;
    }
  }
}

CCUnitType *CCGeneralUnitType::CreateInstance(CCCivilization *pCivilization)
{
  CCUnitType *pUnitType = new CCUnitType();
  pUnitType->Create(this, pCivilization);

  return pUnitType;
}

void CCGeneralUnitType::LoadUnitType(CCUnitType *pUnitType)
{
  ASSERT(pUnitType != NULL);
  CCGeneralUnitAppearance *pGeneralAppearance = m_pAppearances;
  CCUnitAppearance *pAppearance;
  while(pGeneralAppearance != NULL){
    pAppearance = pGeneralAppearance->CreateInstance(pUnitType->GetCivilization());
    pAppearance->m_pNext = pUnitType->m_pFirstAppearance;
    pUnitType->m_pFirstAppearance = pAppearance;

    pGeneralAppearance = pGeneralAppearance->m_pNext;
  }

  pAppearance = m_pSelectionMarkAppearance->CreateInstance(pUnitType->GetCivilization());
  pUnitType->m_pSelectionMarkAppearance = pAppearance;
}