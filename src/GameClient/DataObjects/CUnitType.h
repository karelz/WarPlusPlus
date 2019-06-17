#ifndef CUNITTYPE_H_
#define CUNITTYPE_H_

#include "CUnitAppearance.h"
#include "CGeneralUnitType.h"

class CCUnit;
class CUnitCache;

// this is unit type dependent on civilization
// it means there's one instance for every civilization
// This object also can be created runtime by the cache
// cause there's to many of them and most of them are not used
// (#civs * #unittypes -> very big number)
class CCUnitType : public CIDCachedObject
{
  DECLARE_DYNAMIC(CCUnitType);

public:
  // constructor & destructor
  CCUnitType();
  virtual ~CCUnitType();

  // debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // Creation
  void Create(CCGeneralUnitType *pGeneralUnitType, CCCivilization *pCivilization);
  virtual void Delete();

  // creates instance of the unit
  void CreateInstance(CCUnit *pUnit, CUnitCache *pUnitCache);

  // returns the genral unit type
  CCGeneralUnitType *GetGeneralUnitType(){ ASSERT_VALID(this); return m_pGeneralUnitType; }

  // returns civcilization
  CCCivilization *GetCivilization(){ ASSERT_VALID(this); return m_pCivilization; }

  // returns appearance by it's ID
  CCUnitAppearance *GetAppearance(DWORD dwID);

  // returns selection mark appearance
  CCUnitAppearance *GetSelectionMarkAppearance(){ ASSERT_VALID(this); return m_pSelectionMarkAppearance; }

  // Cache loading
  virtual void Load();
  virtual void Unload();

private:
// unit type data
  // The general unit type
  CCGeneralUnitType *m_pGeneralUnitType;

  // civilization
  CCCivilization *m_pCivilization;
  
// appearances
  // list of all appearances for this unit type
  // these are the colored one - used by CCunit
  CCUnitAppearance *m_pFirstAppearance;

  // selection mark appearance
  CCUnitAppearance *m_pSelectionMarkAppearance;

  // next unit type at general unit type
  CCUnitType *m_pNextUnitType;
  
  friend class CCCivilization;
  friend class CCGeneralUnitType;
};

#endif