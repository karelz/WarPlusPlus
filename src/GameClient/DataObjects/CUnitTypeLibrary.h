#ifndef CUNITTYPELIBRARY_H_
#define CUNITTYPELIBRARY_H_

class CCMap;

// class for loading the unit type library
// used only temporarily in loading
class CCUnitTypeLibrary : public CObject
{
  DECLARE_DYNAMIC(CCUnitTypeLibrary);

public:
  // constructor & destructor
  CCUnitTypeLibrary();
  virtual ~CCUnitTypeLibrary();

  // debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// creation
  // Creates the library from the archive
  // must have the map pointer to add all unit types to the map list
  void Create(CDataArchive Archive, CCMap *pMap);

  // deletes the library
  // doesn't delete any unit type
  void Delete();

private:

};

#endif