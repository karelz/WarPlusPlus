#include "stdafx.h"
#include "CMapSquare.h"

#include "CMap.h"
#include "..\LoadException.h"

// Constructor
CCMapSquare::CCMapSquare()
{
  m_pBL1Mapexes = NULL;
  m_pBL2Mapexes = NULL;
  m_pBL3Mapexes = NULL;
  m_pULMapexes = NULL;
  m_pUnits = NULL;
}

// Destructor
CCMapSquare::~CCMapSquare()
{
  Delete();
}


// Implementation ----------------------------------

// Create the object from the disk file
// must get the map pointer to find the mapex prototypes by IDs
void CCMapSquare::Create(CArchiveFile MapFile, CCMap *pMap)
{
  SMapSquareHeader Header;

  // load the header
  LOAD_ASSERT ( MapFile.Read ( &Header, sizeof ( Header ) ) == sizeof ( Header ) );

  // load BL1 mapexes
  {
    CCMapexInstance *pMapexInstance;
    DWORD i;

    for(i = 0; i < Header.m_dwBL1MapexesNum; i++){
      // create new mapex instance
      pMapexInstance = new CCMapexInstance();

      // add it to the list
      pMapexInstance->m_pNext = m_pBL1Mapexes;
      m_pBL1Mapexes = pMapexInstance;

      // load it
      pMapexInstance->Create(MapFile, pMap);
    }
  }

  // load BL2 mapexes
  {
    CCMapexInstance *pMapexInstance;
    DWORD i;

    for(i = 0; i < Header.m_dwBL2MapexesNum; i++){
      // create new mapex instance
      pMapexInstance = new CCMapexInstance();

      // add it to the list
      pMapexInstance->m_pNext = m_pBL2Mapexes;
      m_pBL2Mapexes = pMapexInstance;

      // load it
      pMapexInstance->Create(MapFile, pMap);
    }
  }

  // load BL3 mapexes
  {
    CCMapexInstance *pMapexInstance;
    DWORD i;

    for(i = 0; i < Header.m_dwBL3MapexesNum; i++){
      // create new mapex instance
      pMapexInstance = new CCMapexInstance();

      // add it to the list
      pMapexInstance->m_pNext = m_pBL3Mapexes;
      m_pBL3Mapexes = pMapexInstance;

      // load it
      pMapexInstance->Create(MapFile, pMap);
    }
  }
}

// Deletes the object
void CCMapSquare::Delete()
{
  // Delete all mapex instances

  // Delete BL1 ones
  {
    CCMapexInstance *pMapexInstance, *pDel;
    pMapexInstance = m_pBL1Mapexes;
    while(pMapexInstance != NULL){
      pDel = pMapexInstance;
      pMapexInstance = pMapexInstance->m_pNext;

      pDel->Delete();
      delete pDel;
    }
    m_pBL1Mapexes = NULL;
  }

  // Delete BL2 ones
  {
    CCMapexInstance *pMapexInstance, *pDel;
    pMapexInstance = m_pBL2Mapexes;
    while(pMapexInstance != NULL){
      pDel = pMapexInstance;
      pMapexInstance = pMapexInstance->m_pNext;

      pDel->Delete();
      delete pDel;
    }
    m_pBL2Mapexes = NULL;
  }

  // Delete BL3 ones
  {
    CCMapexInstance *pMapexInstance, *pDel;
    pMapexInstance = m_pBL3Mapexes;
    while(pMapexInstance != NULL){
      pDel = pMapexInstance;
      pMapexInstance = pMapexInstance->m_pNext;

      pDel->Delete();
      delete pDel;
    }
    m_pBL3Mapexes = NULL;
  }
}


// Mapex operations ----------------------

// Appends mapexes from one level to the list
void CCMapSquare::AppendBLMapexes(DWORD dwLevel, CTypedPtrList<CPtrList, CCMapexInstance *> *pList)
{
  // first get the level head
  CCMapexInstance *pMapexInstance = GetBLHead(dwLevel), *pHMapexInstance;

  POSITION pos;

  // go through all mapexes in the list and add them to the given list
  while(pMapexInstance != NULL){
    // we must go through the list and insert it to the right position
    // it means sort the list
    pos = pList->GetHeadPosition();
    while(pos != NULL){
      pHMapexInstance = pList->GetAt(pos);
      if((pHMapexInstance->GetYPosition() > pMapexInstance->GetYPosition()) ||
        ((pHMapexInstance->GetYPosition() == pMapexInstance->GetYPosition() &&
        pHMapexInstance->GetXPosition() > pMapexInstance->GetYPosition()))){
        pList->InsertBefore(pos, pMapexInstance);
        break;
      }
      pList->GetNext(pos);
    }
    if(pos == NULL) pList->AddTail(pMapexInstance);

    pMapexInstance = pMapexInstance->m_pNext;
  }
}

// Appends mapexes from the unit level to the list
void CCMapSquare::AppendULMapexes(CTypedPtrList<CPtrList, CCULMapexInstance *> *pList)
{
  // first get the level head
  CCULMapexInstance *pMapexInstance = m_pULMapexes, *pHMapexInstance;

  POSITION pos;

  // go through all mapexes in the list and add them to the given list
  while(pMapexInstance != NULL){
    // we must go through the list and insert it to the right position
    // it means sort the list
    pos = pList->GetHeadPosition();
    while(pos != NULL){
      pHMapexInstance = pList->GetAt(pos);
      if((pHMapexInstance->GetZPosition() >= pMapexInstance->GetZPosition()) ||
        (pHMapexInstance->GetYPosition() > pMapexInstance->GetYPosition()) ||
        ((pHMapexInstance->GetYPosition() == pMapexInstance->GetYPosition() &&
        pHMapexInstance->GetXPosition() > pMapexInstance->GetYPosition()))){
        pList->InsertBefore(pos, pMapexInstance);
        break;
      }
      pList->GetNext(pos);
    }
    if(pos == NULL) pList->AddTail(pMapexInstance);

    pMapexInstance = pMapexInstance->m_pNext;
  }
}


// Helper functions ------------------------

// Returns head of BL
CCMapexInstance *CCMapSquare::GetBLHead(DWORD dwLevel)
{
  switch(dwLevel){
  case 1:
    return m_pBL1Mapexes;
  case 2:
    return m_pBL2Mapexes;
  case 3:
    return m_pBL3Mapexes;
  }
  ASSERT(FALSE);
  return NULL;
}



// Cache functions -------------------------

// returns size of this object in bytes - just a rough guess
DWORD CCMapSquare::GetSize()
{
  return sizeof(this);
}


// *ROMAN v--v

void CCMapSquare::AppendUnit(CCUnit *pUnit)
{
  pUnit->AddRef();

	if( m_pUnits == NULL ) {
		m_pUnits=pUnit;        
		pUnit->SetParentMapSquare(this);
		pUnit->SetNextMapSquareUnit(pUnit);
		pUnit->SetPrevMapSquareUnit(pUnit);
	} else {		
        ASSERT_VALID(m_pUnits);
		pUnit->SetParentMapSquare(this);
		pUnit->SetNextMapSquareUnit(m_pUnits);
		pUnit->SetPrevMapSquareUnit(m_pUnits->GetPrevMapSquareUnit());
		m_pUnits->GetPrevMapSquareUnit()->SetNextMapSquareUnit(pUnit);
		m_pUnits->SetPrevMapSquareUnit(pUnit);
		m_pUnits=pUnit;
	}
}

void CCMapSquare::RemoveUnit(CCUnit *pUnit)
{
	ASSERT_VALID(pUnit);
	ASSERT(m_pUnits != NULL);
	ASSERT_VALID(m_pUnits);
	if(m_pUnits==pUnit) {
		if(m_pUnits->GetNextMapSquareUnit()==m_pUnits) {
      // Pokud nasledujici jednotka za prvni jednotkou v mapsquaru je ona sama...
      // ... je to posledni jednotka a spojak se zrusi
			m_pUnits=NULL;
		} else {
			m_pUnits=m_pUnits->GetNextMapSquareUnit();
		}
	}

  pUnit->GetPrevMapSquareUnit()->SetNextMapSquareUnit(pUnit->GetNextMapSquareUnit());
  pUnit->GetNextMapSquareUnit()->SetPrevMapSquareUnit(pUnit->GetPrevMapSquareUnit());

	pUnit->SetNextMapSquareUnit(pUnit);
	pUnit->SetPrevMapSquareUnit(pUnit);

  pUnit->SetParentMapSquare ( NULL );

  pUnit->Release();
}

// *ROMAN ^--^
