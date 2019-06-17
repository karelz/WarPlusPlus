#ifndef CMAPSQUARE_H_
#define CMAPSQUARE_H_

#include "CMapexInstance.h"
#include "CULMapexInstance.h"
#include "CUnit.h"

class CCMap;

// Class representing one map square
class CCMapSquare : public CCachedObject
{
public:
  // Constructor & destructor
  CCMapSquare();
  virtual ~CCMapSquare();

// Creation
  // Creates the object from the file - the file position is set to the start of the mapsquare
  void Create(CArchiveFile MapFile, CCMap *pMap);

  // Deletes the object
  void Delete();

// *ROMAN v--v
  // Appends new unit
  void AppendUnit(CCUnit *pUnit);

  // Removes unit
  void RemoveUnit(CCUnit *pUnit);
// *ROMAN ^--^

// Operations with mapexes
  // appends all mapexes in this mapsquare for given level to the tail of the given list
  void AppendBLMapexes(DWORD dwLevel, CTypedPtrList<CPtrList, CCMapexInstance *> *pList);
  // appends all mapexes in this mapsquare for unit level to the tail of the given list
  void AppendULMapexes(CTypedPtrList<CPtrList, CCULMapexInstance *> *pList);

// Operations with units
  CCUnit *GetUnitList(){ return m_pUnits; }

// Cache functions
protected:
  // returns the size of this object (very poor precision - really just a geuss)
  // it would cost too much time to compute the real size
  virtual DWORD GetSize();

private:
  // lists of mapexes for Background Levels
  CCMapexInstance *m_pBL1Mapexes;
  CCMapexInstance *m_pBL2Mapexes;
  CCMapexInstance *m_pBL3Mapexes;
  CCULMapexInstance *m_pULMapexes;

  CCUnit *m_pUnits;

// Helper functions
  // returns the head of Background level (first mapex pointer)
  CCMapexInstance *GetBLHead(DWORD dwLevel);
};

#endif