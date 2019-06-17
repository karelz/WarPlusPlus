// PersistentObject.h: interface for the CPersistentObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PERSISTENTOBJECT_H__75673152_E965_11D3_844B_004F4E0004AA__INCLUDED_)
#define AFX_PERSISTENTOBJECT_H__75673152_E965_11D3_844B_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PersistentStorage.h"

// class for all objects that want to be able of persistent storage (saving&loading)

class CPersistentObject : public CObject  
{
  DECLARE_DYNAMIC(CPersistentObject);
public:

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // constructor
  CPersistentObject();
  // destructor
  virtual ~CPersistentObject();

  // Called when the object is about to be saved
  //   use the storage object to store the data to.
  // Include all pointers to the saved structures (we are able to translate them lately)
  // If you have some subobject in your object, save also that object, and write the pointer
  //   to it before you call the PersistentSave on the subobject
  // Please try to save all needed data, but also try to save only the necessery ones
  // if you are able to compute some data from another, don't save them
  // and recompute them in PersistentInit method after the load
  void PersistentSave(CPersistentStorage &storage){};

  // Called when the object is about to be loaded
  // Here just load all data structures to memory
  // and allocate all subobjects and call the PersistentLoad on them
  // also when you are allocating some subobject, you should have the old pointer to it
  // loaded in memory from the storage. Then call RegisterPointer member function
  // on the storage object and pass both pointers to it (the old one, and the new one)
  void PersistentLoad(CPersistentStorage &storage){};

  // Called after all objects are in memory
  // Translate all pointers in your object
  // call TranslatePointer methon on storage object to do so
  // still you can't call some functional methods on other objects, they're not in
  // OK state
  void PersistentTranslatePointers(CPersistentStorage &storage){};

  // Called after all pointer translations where done
  // do some additional init here
  void PersistentInit(){};
};

// same as above but without the CObject base calss
// use this one if you need a small (in memory) object
class CPersistentObjectPlain
{
public:
  CPersistentObjectPlain(){}
  ~CPersistentObjectPlain(){}

  void PersistentSave(CPersistentStorage &storage){};
  void PersistentLoad(CPersistentStorage &storage){};
  void PersistentTranslatePointers(CPersistentStorage &storage){};
  void PersistentInit(){};
};

#endif // !defined(AFX_PERSISTENTOBJECT_H__75673152_E965_11D3_844B_004F4E0004AA__INCLUDED_)
