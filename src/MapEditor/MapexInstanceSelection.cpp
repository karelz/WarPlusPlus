// MapexInstanceSelection.cpp: implementation of the CMapexInstanceSelection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MapexInstanceSelection.h"

#include "DataObjects\EMap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CMapexInstanceSelection, CObject);

CMapexInstanceSelection::CMapexInstanceSelection()
{

}

CMapexInstanceSelection::~CMapexInstanceSelection()
{
  m_listInstances.RemoveAll();
}

#ifdef _DEBUG

void CMapexInstanceSelection::AssertValid() const
{
  CObject::AssertValid();
}

void CMapexInstanceSelection::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


void CMapexInstanceSelection::Clear()
{
  m_listInstances.RemoveAll();
}

BOOL CMapexInstanceSelection::AddInstance(CEMapexInstance *pInstance)
{
  // if already included -> do nothing
  if(Includes(pInstance)) return FALSE;

  m_listInstances.AddHead(pInstance);
  return TRUE;
}

BOOL CMapexInstanceSelection::Includes(CEMapexInstance *pInstance)
{
  POSITION pos;
  pos = m_listInstances.GetHeadPosition();
  while(pos != NULL){
    if(m_listInstances.GetNext(pos) == pInstance)
      return TRUE;
  }
  return FALSE;
}

void CMapexInstanceSelection::RemoveInstance(CEMapexInstance *pInstance)
{
  POSITION pos;
  pos = m_listInstances.GetHeadPosition();
  while(pos != NULL){
    if(m_listInstances.GetAt(pos) == pInstance){
      m_listInstances.RemoveAt(pos);
      break;
    }
    m_listInstances.GetNext(pos);
  }
}


BOOL CMapexInstanceSelection::SetRect(CRect rcSelection, CEMap *pMap)
{
  DWORD dwXSize, dwYSize, dwXPos, dwYPos;
  dwXSize = rcSelection.Width();
  dwYSize = rcSelection.Height();
  dwXPos = rcSelection.left;
  dwYPos = rcSelection.top;

  // now construct the list of all sqaures that mapexes from can touch our rect
  CTypedPtrList<CPtrList, CEMapSquare *> listDrawSquares;
  int nX, nY;
  // we have to start one square above the affected one
  // becauese mapexes can override square boundaries
  for(nY = dwYPos / MAPSQUARE_HEIGHT - 1; nY <= (int)((dwYPos + dwYSize) / MAPSQUARE_HEIGHT); nY++){
    if((nY < 0) || (nY >= (int)pMap->GetSquareHeight())) continue;
    // the same trick with starting index as for Y coord
    for(nX = dwXPos / MAPSQUARE_WIDTH - 1; nX <= (int)((dwXPos + dwXSize) / MAPSQUARE_WIDTH); nX++){
      if((nX < 0) || (nX >= (int)pMap->GetSquareWidth())) continue;

      listDrawSquares.AddHead(pMap->GetMapSquare(nX, nY));
    }
  }

  // make draw background levels
  DWORD dwLevel;
  CTypedPtrList<CPtrList, CEMapexInstance *> listMapexes;
  CTypedPtrList<CPtrList, CEMapexInstance *> listAddMapexes;
  POSITION pos;
  CEMapSquare *pSquare;
  CEMapexInstance *pMapexInstance;
  CRect rcMapex;

  for(dwLevel = 1; dwLevel <= 3; dwLevel++){

    if((dwLevel == 1) && !pMap->m_bBL1Visible) continue;
    if((dwLevel == 2) && !pMap->m_bBL2Visible) continue;
    if((dwLevel == 3) && !pMap->m_bBL3Visible) continue;

    // collect all mapexes for this level from draw squares
    listMapexes.RemoveAll();

    pos = listDrawSquares.GetHeadPosition();
    while(pos != NULL){
      pSquare = listDrawSquares.GetNext(pos);
      pSquare->AppendBLMapexes(dwLevel, &listMapexes);
    }

    // remove all mapexes that doesn't intersects the given rectangle
    pos = listMapexes.GetHeadPosition();
    while(pos != NULL){
      pMapexInstance = listMapexes.GetNext(pos);

      rcMapex.left = pMapexInstance->GetXPosition();
      rcMapex.top = pMapexInstance->GetYPosition();
      rcMapex.right = rcMapex.left + pMapexInstance->GetMapex()->GetSize().cx;
      rcMapex.bottom = rcMapex.top + pMapexInstance->GetMapex()->GetSize().cy;

      rcMapex.IntersectRect(&rcMapex, &rcSelection);

      // if it intersects the selected rectangle -> add it
      if(!rcMapex.IsRectEmpty()){
        listAddMapexes.AddHead(pMapexInstance);
      }
    }
  }

  BOOL bChanged = FALSE;
  pos = m_listInstances.GetHeadPosition();
  while(pos != NULL){
    pMapexInstance = m_listInstances.GetNext(pos);
    POSITION pos2 = listAddMapexes.GetHeadPosition();
    while(pos2 != NULL){
      if(pMapexInstance == listAddMapexes.GetAt(pos2)){
        goto Found;
      }
      listAddMapexes.GetNext(pos2);
    }
    bChanged = TRUE; // some was not selected
Found:
    ;
  }

  pos = listAddMapexes.GetHeadPosition();
  while(pos != NULL){
    pMapexInstance = listAddMapexes.GetNext(pos);
    if(!Includes(pMapexInstance))
      bChanged = TRUE; // some was missing in selection
  }

  Clear();
  pos = listAddMapexes.GetHeadPosition();
  while(pos != NULL){
    pMapexInstance = listAddMapexes.GetNext(pos);
    AddInstance(pMapexInstance);
  }

  return bChanged;
}

CRect CMapexInstanceSelection::GetBoundingRect()
{
  CRect rcBound(0, 0, 0, 0);
  CRect rcMapex;

  POSITION pos;
  CEMapexInstance *pInstance;
  pos = m_listInstances.GetHeadPosition();
  while(pos != NULL){
    pInstance = m_listInstances.GetNext(pos);

    rcMapex.left = pInstance->GetXPosition();
    rcMapex.top = pInstance->GetYPosition();
    rcMapex.right = rcMapex.left + pInstance->GetMapex()->GetSize().cx;
    rcMapex.bottom = rcMapex.top + pInstance->GetMapex()->GetSize().cy;

    if(rcBound.IsRectEmpty())
      rcBound = rcMapex;
    else
      rcBound.UnionRect(&rcBound, &rcMapex);
  }
  return rcBound;
}

CString CMapexInstanceSelection::GetStatusBarText(CEMap *pMap)
{
  CString str;
  if(m_listInstances.IsEmpty()){
    str.Empty();
    return str;
  }
  if(m_listInstances.GetCount() > 1){
    str = "Multiple selection";
    return str;
  }

  DWORD dwLevel;
  CEMapexInstance *pMapexInstance = m_listInstances.GetHead();
  dwLevel = pMap->GetMapSquareFromCell(pMapexInstance->GetXPosition(),
    pMapexInstance->GetYPosition())->GetBLMapexLevel(pMapexInstance);
  str.Format("%d - %s", dwLevel, pMapexInstance->GetMapex()->GetName());
  return str;
}

void CMapexInstanceSelection::DeleteSelection(CEMap *pMap)
{
  POSITION pos;
  CEMapexInstance *pInstance;

  pos = m_listInstances.GetHeadPosition();
  while(pos != NULL){
    pInstance = m_listInstances.GetNext(pos);

    CEMapSquare *pSquare;
    pSquare = pMap->GetMapSquareFromCell(pInstance->GetXPosition(), pInstance->GetYPosition());
    pMap->LockSquare(pSquare);
    pSquare->DeleteBLMapexInstance(pInstance);
    pMap->ReleaseSquare(pSquare);
    delete pInstance;
  }
  Clear();
}

BOOL CMapexInstanceSelection::IsEmpty()
{
  return m_listInstances.IsEmpty();
}

BOOL CMapexInstanceSelection::PtInSelection(DWORD dwX, DWORD dwY)
{
  POSITION pos;
  CEMapexInstance *pInstance;
  pos = m_listInstances.GetHeadPosition();
  while(pos != NULL){
    pInstance = m_listInstances.GetNext(pos);

    if(pInstance->PtInMapex(dwX, dwY)){
      return TRUE;
    }
  }
  return FALSE;
}

void CMapexInstanceSelection::SetPosition(DWORD dwX, DWORD dwY, CEMap *pMap)
{
  CRect rcSel = GetBoundingRect();

  DWORD dwMX, dwMY;

  POSITION pos;
  CEMapexInstance *pInstance;
  CEMapSquare *pOldSquare, *pNewSquare;
  pos = m_listInstances.GetHeadPosition();
  while(pos != NULL){
    pInstance = m_listInstances.GetNext(pos);
    dwMX = dwX + (pInstance->GetXPosition() - rcSel.left);
    dwMY = dwY + (pInstance->GetYPosition() - rcSel.top);

    pOldSquare = pMap->GetMapSquareFromCell(pInstance->GetXPosition(), pInstance->GetYPosition());
    pNewSquare = pMap->GetMapSquareFromCell(dwMX, dwMY);

    pMap->LockSquare(pOldSquare);

    if(pNewSquare != pOldSquare){
      pMap->LockSquare(pNewSquare);

      DWORD dwLevel = pOldSquare->GetBLMapexLevel(pInstance);
      pOldSquare->DeleteBLMapexInstance(pInstance);
      pNewSquare->AddBLMapexInstance(dwLevel, pInstance);
      pInstance->SetPosition(dwMX, dwMY);
      pMap->ReleaseSquare(pNewSquare);
    }
    else{
      pInstance->SetPosition(dwMX, dwMY);
    }

    pMap->ReleaseSquare(pOldSquare);
  }
}

void CMapexInstanceSelection::Copy(CMapexInstanceSelection *pSource)
{
  POSITION pos;
  CEMapexInstance *pInstance;
  
  m_listInstances.RemoveAll();
  pos = pSource->m_listInstances.GetHeadPosition();
  while(pos != NULL){
    pInstance = pSource->m_listInstances.GetNext(pos);
    m_listInstances.AddHead(pInstance);
  }
}

CEMapexInstance * CMapexInstanceSelection::GetMapexFromCell(DWORD dwX, DWORD dwY)
{
  POSITION pos;
  CEMapexInstance *pInstance;

  pos  = m_listInstances.GetHeadPosition();
  while(pos != NULL){
    pInstance = m_listInstances.GetNext(pos);
    if(pInstance->PtInMapex(dwX, dwY)) return pInstance;
  }
  return NULL;
}

DWORD CMapexInstanceSelection::GetHighestLevel(CEMap *pMap)
{
  POSITION pos;
  DWORD dwLevel = 1, dwI;
  CEMapexInstance *pInstance;
  pos = m_listInstances.GetHeadPosition();
  while(pos != NULL){
    pInstance = m_listInstances.GetNext(pos);
    dwI = pMap->GetMapSquareFromCell(pInstance->GetXPosition(), pInstance->GetYPosition())->GetBLMapexLevel(pInstance);
    if(dwLevel < dwI) dwLevel = dwI;
  }
  return dwLevel;
}

DWORD CMapexInstanceSelection::GetLowestLevel(CEMap *pMap)
{
  POSITION pos;
  DWORD dwLevel = 3, dwI;
  CEMapexInstance *pInstance;
  pos = m_listInstances.GetHeadPosition();
  while(pos != NULL){
    pInstance = m_listInstances.GetNext(pos);
    dwI = pMap->GetMapSquareFromCell(pInstance->GetXPosition(), pInstance->GetYPosition())->GetBLMapexLevel(pInstance);
    if(dwLevel > dwI) dwLevel = dwI;
  }
  return dwLevel;
}

void CMapexInstanceSelection::MoveSelectionDown(CEMap *pMap)
{
  POSITION pos;
  DWORD dwLevel;
  CEMapexInstance *pInstance;
  CEMapSquare *pMapSquare;
  pos = m_listInstances.GetHeadPosition();
  while(pos != NULL){
    pInstance = m_listInstances.GetNext(pos);

    pMapSquare = pMap->GetMapSquareFromCell(pInstance->GetXPosition(), pInstance->GetYPosition());
    pMap->LockSquare(pMapSquare);
    dwLevel = pMapSquare->GetBLMapexLevel(pInstance);
    pMapSquare->DeleteBLMapexInstance(pInstance);
    if(dwLevel > 0) dwLevel--;
    pMapSquare->AddBLMapexInstance(dwLevel, pInstance);
    pMap->ReleaseSquare(pMapSquare);
  }
}

void CMapexInstanceSelection::MoveSelectionUp(CEMap *pMap)
{
  POSITION pos;
  DWORD dwLevel;
  CEMapexInstance *pInstance;
  CEMapSquare *pMapSquare;
  pos = m_listInstances.GetHeadPosition();
  while(pos != NULL){
    pInstance = m_listInstances.GetNext(pos);

    pMapSquare = pMap->GetMapSquareFromCell(pInstance->GetXPosition(), pInstance->GetYPosition());
    pMap->LockSquare(pMapSquare);
    dwLevel = pMapSquare->GetBLMapexLevel(pInstance);
    pMapSquare->DeleteBLMapexInstance(pInstance);
    if(dwLevel < 3) dwLevel++;
    pMapSquare->AddBLMapexInstance(dwLevel, pInstance);
    pMap->ReleaseSquare(pMapSquare);
  }
}