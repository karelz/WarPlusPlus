#ifndef CGENERALUNITAPPEARANCE_H_
#define CGENERALUNITAPPEARANCE_H_

class CCUnitAppearance;
class CCCivilization;

// unit appearance object
// this is independent on civilizations
class CCGeneralUnitAppearance : public CObject
{
  DECLARE_DYNAMIC(CCGeneralUnitAppearance);

public:
  // constructor & destructor
  CCGeneralUnitAppearance();
  virtual ~CCGeneralUnitAppearance();

  // debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// creation
  // Creates the appearance from the file
  void Create(CArchiveFile UnitTypeFile, CDataArchive cArchive_GraphicsClone);

  // Deletes the object
  void Delete();

  // Creates an instance for this object
  // You must specify the civilization for which this appearance is created
  CCUnitAppearance *CreateInstance(CCCivilization *pCivilization);

// Get/Set functions
  DWORD GetID(){ ASSERT_VALID(this); return m_dwID; }

private:
  // ID
  DWORD m_dwID;

// Directions data
  // files for directions
  CString m_aDirections[8];

  // archive from which to load appearances (Here we'll create the files)
  CDataArchive m_Archive;
  // archive from which to load the graphics itself
  // (It's a clone of the above archive
  CDataArchive m_cArchive_GraphicsClone;

  // pointer to the next appearance in the list of appearances on the unit type object
  CCGeneralUnitAppearance *m_pNext;

  // Allow access to the m_pNext member for the unit type (it will manage the list)
  friend class CCGeneralUnitType;
};

#endif