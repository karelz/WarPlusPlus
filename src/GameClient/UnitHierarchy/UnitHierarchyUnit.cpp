// UnitHierarchyUnit.cpp: implementation of the CUnitHierarchyUnit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UnitHierarchyUnit.h"

#include "Common\ServerClient\UnitHierarchyConnection.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CUnitHierarchyUnit, CObject)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUnitHierarchyUnit::CUnitHierarchyUnit()
{
  m_pCommander = NULL;
  m_dwZUnitID = 0;
  m_dwSUnitID = 0x0FFFFFFFF;
  m_dwSUnitTypeID = 0;
  m_pNextSibbling = NULL;
  m_pFirstInferior = NULL;
}

CUnitHierarchyUnit::~CUnitHierarchyUnit()
{
  ASSERT(m_dwZUnitID == 0);
  ASSERT(m_pNextSibbling == NULL);
  ASSERT(m_pFirstInferior == NULL);
}

#ifdef _DEBUG

void CUnitHierarchyUnit::AssertValid() const
{
  CObject::AssertValid();
  ASSERT(m_dwZUnitID != 0);
}

void CUnitHierarchyUnit::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


void CUnitHierarchyUnit::Create(CVirtualConnection *pVirtualConnection, CUnitHierarchyUnit *pCommander)
{
  SUnitHierarchyAnswer_UnitRecord header;

  // read the header
  DWORD dwSize = sizeof(header);
  VERIFY(pVirtualConnection->ReceiveBlock(&header, dwSize));
  ASSERT(dwSize == sizeof(header));

  m_pCommander = pCommander;
  if(pCommander != NULL){
    ASSERT(pCommander->GetZUnitID() == header.m_dwZCommanderID);
    m_dwHierarchyLevel = pCommander->GetHierarchyLevel() + 1;
  }
  else{
    m_dwHierarchyLevel = 0;
  }

  m_dwZUnitID = header.m_dwZUnitID;
  m_dwSUnitID = header.m_dwSUnitID;
  m_dwSUnitTypeID = header.m_dwSUnitTypeID;

  dwSize = header.m_dwScriptNameLength;
  VERIFY(pVirtualConnection->ReceiveBlock(m_strScriptName.GetBuffer(dwSize), dwSize));
  m_strScriptName.ReleaseBuffer();
  ASSERT(dwSize == header.m_dwScriptNameLength);

  dwSize = header.m_dwUnitNameLength;
  VERIFY(pVirtualConnection->ReceiveBlock(m_strName.GetBuffer(dwSize), dwSize));
  m_strName.ReleaseBuffer();
  ASSERT(dwSize == header.m_dwUnitNameLength);

  DWORD dwInferior;
  CUnitHierarchyUnit *pNew, *pPrev = NULL;

  for(dwInferior = 0; dwInferior < header.m_dwInferiorCount; dwInferior++){
    // Allocate new object
    pNew = new CUnitHierarchyUnit();

    // Add it to our list
    if(pPrev != NULL) pPrev->m_pNextSibbling = pNew;
    else m_pFirstInferior = pNew;
    pPrev = pNew;

    // and load it from the network
    pNew->Create(pVirtualConnection, this);
  }

  // And that's all
}

void CUnitHierarchyUnit::Delete()
{
  // Clear us
  m_dwZUnitID = 0;

  // and delete all inferiors
  CUnitHierarchyUnit *pInferior = m_pFirstInferior, *pDel;
  while(pInferior != NULL){
    pDel = pInferior;
    pInferior = pInferior->GetNextSibbling();

    pDel->Delete();
    delete pDel;
  }
  m_pFirstInferior = NULL;
  m_pNextSibbling = NULL;
}