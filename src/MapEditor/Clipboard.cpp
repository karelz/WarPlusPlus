// Clipboard.cpp: implementation of the CClipboard class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Clipboard.h"

#include "DataObjects\EMap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CClipboard, CObject)

CClipboard::CClipboard()
{

}

CClipboard::~CClipboard()
{
  Clear();
}

#ifdef _DEBUG

void CClipboard::AssertValid() const
{
  CObject::AssertValid();
}

void CClipboard::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

void CClipboard::Clear()
{
  POSITION pos;
  SNode *pNode;

  pos = m_listNodes.GetHeadPosition();
  while(pos != NULL){
    pNode = m_listNodes.GetNext(pos);
    delete pNode;
  }
  m_listNodes.RemoveAll();
}

void CClipboard::SetMapexInstanceSelection(CMapexInstanceSelection *pSelection, CEMap *pMap)
{
  Clear();

  POSITION pos;
  SNode *pNode;
  CEMapexInstance *pInstance;
  CRect rcSel = pSelection->GetBoundingRect();
  pos = pSelection->m_listInstances.GetHeadPosition();
  while(pos != NULL){
    pInstance = pSelection->m_listInstances.GetNext(pos);
    pNode = new SNode();
    pNode->m_dwMapexID = pInstance->GetMapex()->GetID();
    pNode->m_dwX = pInstance->GetXPosition() - rcSel.left;
    pNode->m_dwY = pInstance->GetYPosition() - rcSel.top;
    pNode->m_dwLevel = pMap->GetMapSquareFromCell(pInstance->GetXPosition(), pInstance->GetYPosition())->GetBLMapexLevel(pInstance);
    m_listNodes.AddHead(pNode);
  }
}

BOOL CClipboard::IsEmpty()
{
  return m_listNodes.IsEmpty();
}

POSITION CClipboard::GetFirstPosition()
{
  return m_listNodes.GetHeadPosition();
}

CClipboard::SNode * CClipboard::GetNextPosition(POSITION &pos)
{
  return m_listNodes.GetNext(pos);
}
