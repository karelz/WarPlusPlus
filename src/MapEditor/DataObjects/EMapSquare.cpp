// EMapSquare.cpp: implementation of the CEMapSquare class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EMapSquare.h"

#include "..\MapFormats.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEMapSquare::CEMapSquare()
{
  m_bLock = FALSE;

  m_pUnits = NULL;
}

CEMapSquare::~CEMapSquare()
{

}

BOOL CEMapSquare::Create()
{
  m_pBL1Mapexes = NULL;
  m_pBL2Mapexes = NULL;
  m_pBL3Mapexes = NULL;
  m_pULMapexes = NULL;
  m_pUnits = NULL;

  return TRUE;
}

void CEMapSquare::Delete()
{
  DeleteBLMapexes(m_pBL1Mapexes); m_pBL1Mapexes = NULL;
  DeleteBLMapexes(m_pBL2Mapexes); m_pBL2Mapexes = NULL;
  DeleteBLMapexes(m_pBL3Mapexes); m_pBL3Mapexes = NULL;

  CEULMapexInstance *pMapex, *pDelMapex;
  pMapex = m_pULMapexes;
  while(pMapex != NULL){
    pDelMapex = pMapex;
    pMapex = pMapex->m_pNext;
    delete pDelMapex;
  }
  m_pULMapexes = NULL;

  m_pUnits = NULL;
}

void CEMapSquare::DeleteBLMapexes(CEMapexInstance *pHead)
{
  CEMapexInstance *pMapex, *pDelMapex;
  pMapex = pHead;
  while(pMapex != NULL){
    pDelMapex = pMapex;
    pMapex = pMapex->m_pNext;
    delete pDelMapex;
  }
}

BOOL CEMapSquare::Create(CArchiveFile file)
{
  if(g_dwMapFileVersion < MAP_FILE_VERSION(1, 0)){
    SMapSquareHeader_Old h;

    file.Read(&h, sizeof(h));

    // load BLs mapexes
    m_pBL1Mapexes = LoadBLMapexes(file, h.m_dwBL1MapexesNum);
    m_pBL2Mapexes = LoadBLMapexes(file, h.m_dwBL2MapexesNum);
    m_pBL3Mapexes = LoadBLMapexes(file, h.m_dwBL3MapexesNum);

    // load UL mapexes
    CEULMapexInstance *pHead = NULL, *pTail = NULL, *pNew;
    DWORD i;
    for(i = 0; i < h.m_dwULMapexesNum; i++){
      pNew = new CEULMapexInstance();
      if(!pNew->Create(file)){
        delete pNew;
        continue;
      }
      pNew->m_pNext = NULL;

      if(pHead == NULL) pHead = pNew; // if the first one -> set head
      if(pTail != NULL) pTail->m_pNext = pNew;  // if some tail add it to the end of the list
      pTail = pNew; // last one is this one
    }
    m_pULMapexes = pHead;
  }
  else{
    SMapSquareHeader h;

    file.Read(&h, sizeof(h));

    // load BLs mapexes
    m_pBL1Mapexes = LoadBLMapexes(file, h.m_dwBL1MapexesNum);
    m_pBL2Mapexes = LoadBLMapexes(file, h.m_dwBL2MapexesNum);
    m_pBL3Mapexes = LoadBLMapexes(file, h.m_dwBL3MapexesNum);

    // load UL mapexes
    CEULMapexInstance *pHead = NULL, *pTail = NULL, *pNew;
    DWORD i;
    for(i = 0; i < h.m_dwULMapexesNum; i++){
      pNew = new CEULMapexInstance();
      if(!pNew->Create(file)){
        delete pNew;
        continue;
      }
      pNew->m_pNext = NULL;

      if(pHead == NULL) pHead = pNew; // if the first one -> set head
      if(pTail != NULL) pTail->m_pNext = pNew;  // if some tail add it to the end of the list
      pTail = pNew; // last one is this one
    }
    m_pULMapexes = pHead;
  }

  return TRUE;
}

void CEMapSquare::SaveToFile(CArchiveFile file)
{
  SMapSquareHeader h;

  h.m_dwBL1MapexesNum = CountBLMapexes(m_pBL1Mapexes);
  h.m_dwBL2MapexesNum = CountBLMapexes(m_pBL2Mapexes);
  h.m_dwBL3MapexesNum = CountBLMapexes(m_pBL3Mapexes);

  CEULMapexInstance *pMapex;
  DWORD dwCount = 0;
  pMapex = m_pULMapexes;
  while(pMapex != NULL){
    dwCount++;
    pMapex = pMapex->m_pNext;
  }
  h.m_dwULMapexesNum = dwCount;

  file.Write(&h, sizeof(h));
  SortBLMapexes(&m_pBL1Mapexes);
  SaveBLMapexes(m_pBL1Mapexes, file);
  SortBLMapexes(&m_pBL2Mapexes);
  SaveBLMapexes(m_pBL2Mapexes, file);
  SortBLMapexes(&m_pBL3Mapexes);
  SaveBLMapexes(m_pBL3Mapexes, file);

  pMapex = m_pULMapexes;
  while(pMapex != NULL){
    pMapex->SaveToFile(file);
    pMapex = pMapex->m_pNext;
  }
}

DWORD CEMapSquare::CountBLMapexes(CEMapexInstance *pFirst)
{
  CEMapexInstance *pMapex;
  DWORD dwCount = 0;
  pMapex = pFirst;
  while(pMapex != NULL){
    dwCount++;
    pMapex = pMapex->m_pNext;
  }
  return dwCount;
}

void CEMapSquare::SortBLMapexes(CEMapexInstance **ppFirst)
{
  CEMapexInstance *pMapex, *pHlp;
  CTypedPtrList<CPtrList, CEMapexInstance *> list;
  POSITION pos, posAfter;

  // We'll sort it in new list and then remake the original one
  pMapex = *ppFirst;
  while(pMapex != NULL){
    pos = list.GetHeadPosition();
    posAfter = NULL;
    while(pos != NULL){
      posAfter = pos;
      pHlp = list.GetNext(pos);
      if(pHlp->GetYPosition() > pMapex->GetYPosition()) break;
      if((pHlp->GetYPosition() == pMapex->GetYPosition()) &&
        (pHlp->GetXPosition() > pMapex->GetXPosition())) break;
    }
    if(pos == NULL){
      list.AddTail(pMapex);
    }
    else{
      list.InsertBefore(posAfter, pMapex);
    }

    pMapex = pMapex->m_pNext;
  }
  // Now remake the original list
  pos = list.GetHeadPosition();
  CEMapexInstance **ppPrev = ppFirst;
  while(pos != NULL){
    pMapex = list.GetNext(pos);
    *ppPrev = pMapex;
    ppPrev = &(pMapex->m_pNext);
  }
  *ppPrev = NULL;
}

void CEMapSquare::SaveBLMapexes(CEMapexInstance *pFirst, CArchiveFile file)
{
  CEMapexInstance *pMapex = pFirst;
  while(pMapex != NULL){
    pMapex->SaveToFile(file);
    pMapex = pMapex->m_pNext;
  }
}

CEMapexInstance * CEMapSquare::LoadBLMapexes(CArchiveFile file, DWORD dwCount)
{
  CEMapexInstance *pHead = NULL, *pTail = NULL, *pNew;
  DWORD i;
  for(i = 0; i < dwCount; i++){
    pNew = new CEMapexInstance();
    if(!pNew->Create(file)){
      delete pNew;
      continue;
    }
    pNew->m_pNext = NULL;

    if(pHead == NULL) pHead = pNew; // if the first one -> set head
    if(pTail != NULL) pTail->m_pNext = pNew;  // if some tail add it to the end of the list
    pTail = pNew; // last one is this one
  }

  return pHead;
}

void CEMapSquare::AppendBLMapexes(DWORD dwLevel, CTypedPtrList<CPtrList, CEMapexInstance *> *pList)
{
  CEMapexInstance *pMapex = GetBLFirst(dwLevel);
  CEMapexInstance *pHMapex;

  POSITION pos;
  while(pMapex != NULL){
    pos = pList->GetHeadPosition();
    while(pos != NULL){
      pHMapex = pList->GetAt(pos);
      if((pHMapex->GetYPosition() > pMapex->GetYPosition()) ||
        ((pHMapex->GetYPosition() == pMapex->GetYPosition() &&
        pHMapex->GetXPosition() > pMapex->GetYPosition()))){
        pList->InsertBefore(pos, pMapex);
        break;
      }
      pList->GetNext(pos);
    }
    if(pos == NULL) pList->AddTail(pMapex);
    pMapex = pMapex->m_pNext;
  }
}

void CEMapSquare::AppendULMapexes(CTypedPtrList<CPtrList, CEULMapexInstance *> *pList)
{
  CEULMapexInstance *pMapex = m_pULMapexes, *pHMapex;

  POSITION pos;
  while(pMapex != NULL){
    pos = pList->GetHeadPosition();
    while(pos != NULL){
      pHMapex = pList->GetAt(pos);
      if((pHMapex->GetZPosition() >= pMapex->GetZPosition()) ||
        (pHMapex->GetYPosition() > pMapex->GetYPosition()) ||
        ((pHMapex->GetYPosition() == pMapex->GetYPosition() &&
        pHMapex->GetXPosition() > pMapex->GetYPosition()))){
        pList->InsertBefore(pos, pMapex);
        break;
      }
      pList->GetNext(pos);
    }
    if(pos == NULL) pList->AddTail(pMapex);
    pMapex = pMapex->m_pNext;
  }
}

void CEMapSquare::AppendUnits(CTypedPtrList<CPtrList, CEUnit *> *pList)
{
  CEUnit *pUnit = m_pUnits, *pHUnit;

  POSITION pos;
  while(pUnit != NULL){
    pos = pList->GetHeadPosition();
    while(pos != NULL){
      pHUnit = pList->GetAt(pos);
      if(pHUnit->GetZPos() >= pUnit->GetZPos()){
        pList->InsertBefore(pos, pUnit);
        break;
      }
      pList->GetNext(pos);
    }
    if(pos == NULL) pList->AddTail(pUnit);
    pUnit = pUnit->m_pNext;
  }
}

void CEMapSquare::AddBLMapexInstance(DWORD dwLevel, CEMapexInstance *pMapexInstance)
{
  switch(dwLevel){
  case 1:
    pMapexInstance->m_pNext = m_pBL1Mapexes;
    m_pBL1Mapexes = pMapexInstance;
    break;
  case 2:
    pMapexInstance->m_pNext = m_pBL2Mapexes;
    m_pBL2Mapexes = pMapexInstance;
    break;
  case 3:
    pMapexInstance->m_pNext = m_pBL3Mapexes;
    m_pBL3Mapexes = pMapexInstance;
    break;
  }
}

void CEMapSquare::AddULMapexInstance(CEULMapexInstance *pMapexInstance)
{
  pMapexInstance->m_pNext = m_pULMapexes;
  m_pULMapexes = pMapexInstance;
}

CEMapexInstance * CEMapSquare::GetBLFirst(DWORD dwLevel)
{
  switch(dwLevel){
  case 1: return m_pBL1Mapexes; break;
  case 2: return m_pBL2Mapexes; break;
  case 3: return m_pBL3Mapexes; break;
  default: return NULL;
  }
}

void CEMapSquare::DeleteBLMapexInstance(CEMapexInstance *pMapexInstance)
{
  DWORD dwLevel;
  CEMapexInstance *pPrevMapexInstance;

  // find the level in which it is
  for(dwLevel = 1; dwLevel <= 3; dwLevel ++){
    if((pPrevMapexInstance = IsBLMapexInstanceInLevel(dwLevel, pMapexInstance)) != (CEMapexInstance *)0xFFFFFFFF)
      break;
  }
  if(dwLevel > 3) return; // no such mapex in our lists

  // remove it from list
  if(pPrevMapexInstance == NULL){
    switch(dwLevel){
    case 1: m_pBL1Mapexes = pMapexInstance->m_pNext; break;
    case 2: m_pBL2Mapexes = pMapexInstance->m_pNext; break;
    case 3: m_pBL3Mapexes = pMapexInstance->m_pNext; break;
    }
  }
  else{
    pPrevMapexInstance->m_pNext = pMapexInstance->m_pNext;
  }
  pMapexInstance->m_pNext = NULL;
}

void CEMapSquare::DeleteULMapexInstance(CEULMapexInstance *pMapexInstance)
{
  CEULMapexInstance *pPrevMapexInstance, *pCurInstance;

  pCurInstance = m_pULMapexes;
  pPrevMapexInstance = NULL;
  while(pCurInstance != NULL){
    if(pCurInstance == pMapexInstance){
      break;
    }
    pPrevMapexInstance = pCurInstance;
    pCurInstance = pCurInstance->m_pNext;
  }
  if(pPrevMapexInstance != NULL){
    pPrevMapexInstance->m_pNext = pMapexInstance->m_pNext;
  }
  else{
    m_pULMapexes = pMapexInstance->m_pNext;
  }
  pMapexInstance->m_pNext = NULL;
}

CEMapexInstance *CEMapSquare::IsBLMapexInstanceInLevel(DWORD dwLevel, CEMapexInstance *pMapexInstance)
{
  CEMapexInstance *pMI = GetBLFirst(dwLevel), *pOld = NULL;
  while(pMI != NULL){
    if(pMI == pMapexInstance)
      return pOld;
    pOld = pMI;
    pMI = pMI->m_pNext;
  }
  return (CEMapexInstance *)0xFFFFFFFF;
}

DWORD CEMapSquare::GetBLMapexLevel(CEMapexInstance *pMapexInstance)
{
  DWORD dwLevel;
  for(dwLevel = 1; dwLevel <= 3; dwLevel++){
    if(IsBLMapexInstanceInLevel(dwLevel, pMapexInstance) != (CEMapexInstance *)0xFFFFFFFF)
      return dwLevel;
  }
  return 0;
}

void CEMapSquare::AddUnit(CEUnit *pUnit)
{
  ASSERT(pUnit != NULL);

  // add the unit to the head of the list
  pUnit->m_pNext = m_pUnits;
  m_pUnits = pUnit;
}

void CEMapSquare::DeleteUnit(CEUnit *pUnit)
{
  CEUnit *pPrevInstance, *pCurInstance;

  pCurInstance = m_pUnits;
  pPrevInstance = NULL;
  while(pCurInstance != NULL){
    if(pCurInstance == pUnit){
      break;
    }
    pPrevInstance = pCurInstance;
    pCurInstance = pCurInstance->m_pNext;
  }
  if(pPrevInstance != NULL){
    pPrevInstance->m_pNext = pUnit->m_pNext;
  }
  else{
    m_pUnits = pUnit->m_pNext;
  }
  pUnit->m_pNext = NULL;
}

DWORD CEMapSquare::GetSaveSize()
{
  DWORD dwSize = sizeof(SMapSquareHeader);

  DWORD dwCount;
  dwCount = CountBLMapexes(m_pBL1Mapexes);
  dwCount += CountBLMapexes(m_pBL2Mapexes);
  dwCount += CountBLMapexes(m_pBL3Mapexes);
  dwSize += dwCount * sizeof(SMapexInstanceHeader);

  dwCount = 0;
  CEULMapexInstance *pULMapex = m_pULMapexes;
  while(pULMapex != NULL){
    dwCount ++;
    pULMapex = pULMapex->m_pNext;
  }
  dwSize += dwCount * sizeof(SULMapexInstanceHeader);

  return dwSize;
}
