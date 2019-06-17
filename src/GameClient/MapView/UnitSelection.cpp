// UnitSelection.cpp: implementation of the CUnitSelection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UnitSelection.h"

#include "..\GameClientGlobal.h"
#include "..\DataObjects\CCivilization.h"
#include "..\DataObjects\UnitCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CUnitSelection, CNotifier)

BEGIN_OBSERVER_MAP ( CUnitSelection, CNotifier )
  BEGIN_NOTIFIER ( ID_UnitNotifier )
    EVENT ( CCUnit::E_UnitDisappeared )
      OnUnitDisappeared ( ( CCUnit * ) dwParam );
      return FALSE;
  END_NOTIFIER ()
END_OBSERVER_MAP ( CUnitSelection, CNotifier )

CUnitSelection::CUnitSelection() :
  m_UnitNodePool(50)
{
  m_pSelectedUnitNodes = NULL;
  m_dwCount = 0;
  m_pUnitCache = NULL;
}

CUnitSelection::~CUnitSelection()
{
  ASSERT ( m_pSelectedUnitNodes == NULL );
  ASSERT ( m_pUnitCache == NULL );
}

#ifdef _DEBUG

void CUnitSelection::AssertValid() const
{
  CObject::AssertValid();

  ASSERT ( m_pUnitCache != NULL );
}

void CUnitSelection::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

// Creates empty selection
void CUnitSelection::Create( CUnitCache * pUnitCache )
{
  ASSERT_VALID ( pUnitCache );

  // Copy the unit cache pointer
  m_pUnitCache = pUnitCache;
}

// Deletes the selection
void CUnitSelection::Delete()
{
  // Delete the notifier
  CNotifier::Delete();

  // Clear the selection
  Clear();

  // Forget the cache
  m_pUnitCache = NULL;
}

// Clears the selection
void CUnitSelection::Clear()
{
  SUnitNode *pNode, *pDel;

  VERIFY(m_lockExclusive.Lock());
  // Go through all nodes and delete them
  pNode = m_pSelectedUnitNodes;
  while(pNode != NULL){
    pDel = pNode;
    pNode = pNode->m_pNext;
    
    // Release the unit
    if ( pDel->m_pUnit != NULL )
    {
      pDel->m_pUnit->Disconnect ( this );
      pDel->m_pUnit->Release ();
    }
    m_UnitNodePool.Free(pDel);
  }

  m_pSelectedUnitNodes = NULL;
  m_dwCount = NULL;

  InlayEvent(E_SelectionChanged, 0);

  VERIFY(m_lockExclusive.Unlock());
}

// Appends all units in given selection to this one
void CUnitSelection::Append ( CUnitSelection * pOtherSelection )
{
  // Go through all units in the other selection and add them
  SUnitNode * pNode = pOtherSelection->GetFirstNode ();
  while ( pNode != NULL )
  {
    Add ( pNode->m_dwUnitID, pNode->m_dwUnitTypeID, pNode->m_dwCivilizationID );
    pNode = pOtherSelection->GetNext ( pNode );
  }
}

// Adds unit to the selection
// Adds the unit object
void CUnitSelection::Add(CCUnit *pUnit)
{
  Add(pUnit->GetID(), pUnit->GetUnitType()->GetGeneralUnitType()->GetID(),
    pUnit->GetUnitType()->GetCivilization()->GetID());
}
// Adds unit by IDs
void CUnitSelection::Add(DWORD dwUnitID, DWORD dwUnitTypeID, DWORD dwCivilizationID)
{
  VERIFY(m_lockExclusive.Lock());

  // Try to find the unit in selection
  if(Find(dwUnitID) != NULL){
    VERIFY(m_lockExclusive.Unlock());
    return;
  }

  if(dwCivilizationID == g_pCivilization->GetID()){
    // If the new unit civilization is ours and we have selected enemy units -> clear
    if(IsEnemyCivilization()) Clear();
  }
  else{
    // if the new unit is from enemy civ and we have selected ours -> clear
    if(IsOurCivilization()) Clear();
  }

  // Create new node
  SUnitNode *pNode;
  pNode = m_UnitNodePool.Allocate();
  pNode->m_dwUnitID = dwUnitID;
  pNode->m_dwUnitTypeID = dwUnitTypeID;
  pNode->m_dwCivilizationID = dwCivilizationID;

  // Try to find the unit in our caches
  pNode->m_pUnit = m_pUnitCache->GetUnit ( dwUnitID );
  if ( pNode->m_pUnit != NULL )
  {
    pNode->m_pUnit->Connect ( this, ID_UnitNotifier );
  }
  
  // Add it to the list
  pNode->m_pNext = m_pSelectedUnitNodes;
  m_pSelectedUnitNodes = pNode;

  m_dwCount++;

  InlayEvent(E_SelectionChanged, 0);

  VERIFY(m_lockExclusive.Unlock());
}

// Removes the unit from the selection
// Removes the unit by pointer
void CUnitSelection::Remove(CCUnit *pUnit)
{
  Remove(pUnit->GetID());
}
// Removes the unit by ID
void CUnitSelection::Remove(DWORD dwUnitID)
{
  VERIFY(m_lockExclusive.Lock());

  // Find the node
  SUnitNode *pNode, **pPrev;

  pNode = m_pSelectedUnitNodes;
  pPrev = &m_pSelectedUnitNodes;
  while(pNode != NULL){
    if(pNode->m_dwUnitID == dwUnitID) break;
    pPrev = &(pNode->m_pNext);
    pNode = pNode->m_pNext;
  }

  // If found -> delete it
  if(pNode != NULL){
    // Remove it from the list
    *pPrev = pNode->m_pNext;

    // Release the unit
    if ( pNode->m_pUnit != NULL )
    {
      pNode->m_pUnit->Disconnect ( this );
      pNode->m_pUnit->Release ();
    }

    // Delete it
    m_UnitNodePool.Free(pNode);

    m_dwCount--;

    InlayEvent(E_SelectionChanged, 0);
  }

  VERIFY(m_lockExclusive.Unlock());
}


// Finds the unit in selection
CUnitSelection::SUnitNode *CUnitSelection::Find(DWORD dwUnitID)
{
  SUnitNode *pNode;
  pNode = m_pSelectedUnitNodes;
  while(pNode != NULL){
    if(pNode->m_dwUnitID == dwUnitID) return pNode;
    pNode = pNode->m_pNext;
  }
  return NULL;
}

// Returns TRUE if the selection contains the client's civilization (our)
BOOL CUnitSelection::IsOurCivilization()
{
  BOOL bReturn = FALSE;
  // Get the first node
  if((m_pSelectedUnitNodes != NULL) &&
    (m_pSelectedUnitNodes->m_dwCivilizationID == g_pCivilization->GetID()))
    bReturn = TRUE;
  return bReturn;
}

// Returns TRUE if the selection contains the enemy civilization unit
BOOL CUnitSelection::IsEnemyCivilization()
{
  BOOL bReturn = FALSE;
  // Get the first node
  if((m_pSelectedUnitNodes != NULL) &&
    (m_pSelectedUnitNodes->m_dwCivilizationID != g_pCivilization->GetID()))
    bReturn = TRUE;
  return bReturn;
}

// Reaction on unit disappeared event
void CUnitSelection::OnUnitDisappeared ( CCUnit * pUnit )
{
  // We must lock the structures
  Lock ();

  // Find the node for the unit
  SUnitNode * pNode = Find ( pUnit );

  if ( pNode != NULL )
  {
    // If it's our unit, do not remove it, just forget the unit pointer
    if ( pNode->m_dwCivilizationID == g_pCivilization->GetID () )
    {
      // It's our unit -> just forget the pointer
      if ( pNode->m_pUnit != NULL )
      {
        pNode->m_pUnit->Disconnect ( this );
        pNode->m_pUnit->Release ();
        pNode->m_pUnit = NULL;
      }
    }
    else
    {
      // It's enemy unit -> remove it from the selection
      Remove ( pUnit );
    }
  }

  // Unlock structures
  Unlock ();
}
