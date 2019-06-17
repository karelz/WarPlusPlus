#ifndef CMAPEXLIBRARY_H_
#define CMAPEXLIBRARY_H_

class CCMap;

// mapex library
// this object will probably exist only at init-time
// represents one mapex library (archive) on the disk
class CCMapexLibrary : public CObject
{
  DECLARE_DYNAMIC(CCMapexLibrary);

public:
  // constructor & destructor
  CCMapexLibrary();
  virtual ~CCMapexLibrary();
  
  // loads mapexes in the libray to memory
  // no graphic will be loaded
  // must have a pointer to the map object to insert new mapexes to the map
  void Create(CDataArchive archive, CCMap *pMap);
  
  // closes this library
  // no mapex will be deleted here, just library info will blow up
  void Delete();


// debug functions
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext &dc) const;
#endif

private:
  // some variables
};

#endif