// UnitSelection.h: interface for the CUnitSelection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNITSELECTION_H__B40B79C4_5A33_11D4_B0BB_004F49068BD6__INCLUDED_)
#define AFX_UNITSELECTION_H__B40B79C4_5A33_11D4_B0BB_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\DataObjects\CUnit.h"

// Represents selected units
class CUnitSelection : public CNotifier
{
  DECLARE_DYNAMIC ( CUnitSelection );
  DECLARE_OBSERVER_MAP ( CUnitSelection );

public:
  // Constructor & destructor
	CUnitSelection();
	virtual ~CUnitSelection();

  // Debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // Creates an empty selection
  void Create ( CUnitCache * pUnitCache );
  // Deletes the selection
  virtual void Delete();

  // Struct representing one selected unit
  struct tagSUnitNode{
    // ID of the selected unit
    DWORD m_dwUnitID;
    // ID of the unit type
    DWORD m_dwUnitTypeID;
    // ID of the civilization
    DWORD m_dwCivilizationID;
    // Pointer to the unit (can be NULL)
    CCUnit * m_pUnit;

    struct tagSUnitNode *m_pNext;
  };
  typedef struct tagSUnitNode SUnitNode;
  
  // Returns number of units in the selection
  DWORD GetCount(){ return m_dwCount; }
  
  // Clears the selection
  void Clear();

  // Appends all units in given selection to this one
  // You don't have to lock this selection for this action
  void Append ( CUnitSelection * pOtherSelection );

  // Adds unit to the selection
  // Locks the selection inside
  // If you select unit of enemy civ into the selection of our civ
  //   it will clear the selection first
  // If you select unit of our civ into the selection of enemy civ
  //   it will clear the selection first too
  void Add(CCUnit *pUnit);
  void Add(DWORD dwUnitID, DWORD dwUnitTypeID, DWORD dwCivilizationID);
  // Removes unit from the selection
  // Locks the selection inside
  void Remove(CCUnit *pUnit);
  void Remove(DWORD dwUnitID);

  // Finds the unit and returns its node
  // Doesn't lock the selection (You must lock it by yourself)
  SUnitNode *Find(CCUnit *pUnit){ return Find(pUnit->GetID()); }
  SUnitNode *Find(DWORD dwUnitID);

  // Returns TRUE if the selection is empty
  BOOL IsEmpty(){ return (m_pSelectedUnitNodes == NULL); }

  // Iteration
  // Returns first node in the selection
  // No locking (you should lock it by yourself)
  SUnitNode *GetFirstNode(){ return m_pSelectedUnitNodes; }
  // Returns next node in the selection (if NULL -> end)
  SUnitNode *GetNext(SUnitNode *pNode){ return pNode->m_pNext; }

  // Locks the selection
  void Lock(){ VERIFY(m_lockExclusive.Lock()); }
  // Unlocks the selection
  void Unlock(){ VERIFY(m_lockExclusive.Unlock()); }

  enum{ // Events sent by this object
    E_SelectionChanged = 1, // The selection has changed
  };

  // Returns TRUE if the selection contains the client's civilization (our)
  BOOL IsOurCivilization();
  // Returns TRUE if the selection contains the enemy civilization unit
  BOOL IsEnemyCivilization();

  // Returns the unit cache this selection is assigned to
  CUnitCache * GetUnitCache () { return m_pUnitCache; }

protected:
  enum
  {
    // Notifier ID for all selected ( and connected ) units
    ID_UnitNotifier = 0x0100,
  };
  // Reaction on unit disappeared event
  void OnUnitDisappeared ( CCUnit * pUnit );

private:
  // Exclusive lock for this object
  CMutex m_lockExclusive;

  // Memory pool for unit nodes
  CTypedMemoryPool<SUnitNode> m_UnitNodePool;

  // List of selected unit nodes
  SUnitNode *m_pSelectedUnitNodes;
  // Number of units in the list
  DWORD m_dwCount;

  // Pointer to unit cache
  // If some unit is added to the selection, it's found in the cache
  // and if the unit disappears it's removed from the selection
  CUnitCache * m_pUnitCache;
};

#endif // !defined(AFX_UNITSELECTION_H__B40B79C4_5A33_11D4_B0BB_004F49068BD6__INCLUDED_)
