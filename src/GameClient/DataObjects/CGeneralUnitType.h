#ifndef CGENERALUNITTYPE_H_
#define CGENERALUNITTYPE_H_

class CCCivilization;
class CCUnitType;

#include "CGeneralUnitAppearance.h"
#include "Common\Map\MapDataTypes.h"

// This one represents one unit type
// this is loaded from unit libraries
// it doesn't know anything about civilization
// All of these objects are loaded at init-time from the map file

// In fact CCunitType is copy of this object but it does know about the civilization
class CCGeneralUnitType : public CObject // or something
{
  DECLARE_DYNAMIC(CCGeneralUnitType);

public:
  // constructor & destructor
  CCGeneralUnitType();
  ~CCGeneralUnitType();

  // debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// creation
  // Creates the GeneralUnitType from the file saved by mapeditor
  void Create(CArchiveFile UnitTypeFile, CDataArchive cArchive_GraphicsClone);

  // Delete the object
  void Delete();


// UnitType relationship
  CCUnitType * CreateInstance(CCCivilization *pCivilization);

  void LoadUnitType(CCUnitType *pUnitType);

// Get/Set methods
  // returns ID
  DWORD GetID(){ ASSERT_VALID(this); return m_dwID; }
  // return name
  CString GetName(){ ASSERT_VALID(this); return m_strName; }
  // returns TRUE if this unit is selectable
  BOOL IsSelectable(){ ASSERT_VALID(this); return m_dwFlags & UnitTypeFlags_Selectable; }
  // returns flags related to life bar
  DWORD GetLifeBarFlags () { ASSERT_VALID ( this ); return m_dwFlags & UnitTypeFlags_LifeBar_Mask; }
  // returns flags
  DWORD GetFlags () { ASSERT_VALID ( this ); return m_dwFlags; }
  // Returns move width for given mode
  DWORD GetMoveWidth ( DWORD dwMode ) const { ASSERT_VALID ( this ); ASSERT ( dwMode < 8 );
    return m_aMoveWidth [ dwMode ]; }
    
private:
// data for this type - these are duplicated in CCUnitType
  // ID
  DWORD m_dwID;
  // name
  CString m_strName;

  // mode names
  CString m_aModeNames[8];
  // Move widths for all modes
  DWORD m_aMoveWidth [ 8 ];

  // Some flags
  DWORD m_dwFlags;

// appearances
  // list of appearances
  CCGeneralUnitAppearance *m_pAppearances;
  // The selection mark appearance
  CCGeneralUnitAppearance *m_pSelectionMarkAppearance;

  // array of default appearances IDs (pointers will be only in the CCUnitType)
  DWORD m_aDefaultAppearances[8];


  // next general unit type in the map list
  CCGeneralUnitType *m_pMapNext;

  // map must be friend because of the list pointer
  friend class CCMap;
};

#endif