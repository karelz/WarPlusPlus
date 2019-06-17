#ifndef CUNITAPPEARANCE_H_
#define CUNITAPPEARANCE_H_

#include "CUnitAnimation.h"

class CCGeneralUnitAppearance;

// class representing the unit appearance really used in the game
// this includes the animations colored for specified civilization
class CCUnitAppearance : public CObject
{
  DECLARE_DYNAMIC(CCUnitAppearance);
public:
  // constructor & destructor
  CCUnitAppearance();
  ~CCUnitAppearance();

  // debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// creation
  // create it by calling CCGeneralUnitAppearance::CreateInstance

  // deletes the object
  void Delete();

// Get/Set methods
  CCUnitAnimation *GetDirection(DWORD dwDirection){
    ASSERT_VALID(this); ASSERT(dwDirection < 8);
    return m_aDirections[dwDirection];
  }

  // returns general unit appearance
  CCGeneralUnitAppearance *GetGeneralUnitAppearance(){ ASSERT_VALID(this); return m_pGeneralAppearance; }

private:
// Directions
  // array of animations -> these are cached
  CCUnitAnimation *m_aDirections[8];

  // pointer to the next appearance on the unit type
  CCUnitAppearance *m_pNext;

  // pointer to general unit appearance object
  CCGeneralUnitAppearance *m_pGeneralAppearance;

  // allow access to general unit appearance -> it will fill us with reasonable data
  friend class CCGeneralUnitAppearance;
  friend class CCGeneralUnitType;
  friend class CCUnitType;
};

#endif