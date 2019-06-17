#ifndef CCIVILIZATION_H_
#define CCIVILIZATION_H_

#include "UnitTypeCache.h"

// The civilization class - represents one civilization in the game
class CCCivilization : public CObject
{
  DECLARE_DYNAMIC(CCCivilization);
public:
  // constructor & destructor
  CCCivilization();
  virtual ~CCCivilization();

  // debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// creation
  // Creates the civilization object from the map file
  void Create(CArchiveFile MapFile, DWORD dwIndex);

  // Deletes the civilization object
  void Delete();

// Get/Set functions
  // Returns ID
  DWORD GetID(){ ASSERT_VALID(this); return m_dwID; }
  // Returns index
  DWORD GetIndex(){ ASSERT_VALID(this); return m_dwIndex; }
  // Returns the civilization's color
  DWORD GetColor(){ ASSERT_VALID(this); return m_dwColor; }
  // Returns the name
  CString GetName(){ ASSERT_VALID(this); return m_strName; }

  // creates and adds new unit type
  CCUnitType *CreateUnitType(CCGeneralUnitType *pUnitType);
  // removes unit types
  void RemoveUnitType(CCUnitType *pUnitType);

// Unit types
  // returns the unit type by ID
  CCUnitType *GetUnitType(DWORD dwID){ ASSERT_VALID(this); return m_UnitTypeCache.GetType(dwID); }

private:
  // ID
  DWORD m_dwID;

  // Index of the civilization
  DWORD m_dwIndex;

  // color of the civilization
  DWORD m_dwColor;

  // name
  CString m_strName;

  // pointer to next civ in the list of all civs
  CCCivilization *m_pMapNext;

// Unit types
  // Unit type cache for this civilization
  CUnitTypeCache m_UnitTypeCache;
  // list of all unit types
  CCUnitType *m_pUnitTypes;
  CMutex m_lockUnitTypes;

  friend class CCMap;
};

#endif