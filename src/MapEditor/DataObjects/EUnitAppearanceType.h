// EUnitAppearanceType.h: interface for the CEUnitAppearanceType class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EUNITAPPEARANCETYPE_H__E43F8243_8FCC_11D3_A0C0_D87876DDE731__INCLUDED_)
#define AFX_EUNITAPPEARANCETYPE_H__E43F8243_8FCC_11D3_A0C0_D87876DDE731__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CEUnitType;

class CEUnitAppearance;

////////////////////////////
// this class covers the unit appearance -> list of animations
// it doesn't contain any instance of the animation
// cause we don't know which color to use for the instancing.. :-)
// just the list
// ... and you know some saving and loading... (we're in editor.. shit)

#define UNITAPPEARANCETYPE_DEFAULTCOLOR RGB32(204, 41, 41)

class CEUnitAppearanceType : public CObject  
{
  DECLARE_DYNAMIC(CEUnitAppearanceType)

public:
  BOOL CheckValid();

  CEUnitType *GetUnitType(){ return m_pUnitType; }
  CDataArchive GetArchive(){ return m_Archive; }

  // creates instance of this (used by dialogs)
	CEUnitAppearance *CreateInstance(DWORD dwColor = UNITAPPEARANCETYPE_DEFAULTCOLOR);
  // clears the cache -> it clears the instance of us (the dialog one)
  // and also the default instance (used by map and so...)
	void ClearCache();
	// returns default instance (some default color)
  // if doesn't exists creates one
  CEUnitAppearance * GetDefaultInstance();

  // returns the instance of us (must be created by CreateInstance)
  CEUnitAppearance *GetInstance(){
    return m_pInstance[0];
  }

  // sets the archive for loading instances from
  void SetArchive(CDataArchive Archive){ m_Archive = Archive; }

  // gets the ID
  DWORD GetID(){ return m_dwID; }

  // gets the name
  CString GetName(){ return m_strName; }
  // sets the name
  void SetName(CString strName){ m_strName = strName; }

  // saves the object to the file
	void SaveToFile(CArchiveFile file);

  // destroy it
	virtual void Delete();
  // creates it from file (loads it)
	BOOL Create(CArchiveFile file, CEUnitType *pUnitType);
  // create the new one just from the ID
	BOOL Create(DWORD dwID, CEUnitType *pUnitType);

  // constructor & the other one ... oh I know .. the destructor
	CEUnitAppearanceType();
	virtual ~CEUnitAppearanceType();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // the directions in numbers
  typedef enum{
    Dir_North = 0,
    Dir_NorthEast = 1,
    Dir_East = 2,
    Dir_SouthEast = 3,
    Dir_South = 4,
    Dir_SouthWest = 5,
    Dir_West = 6,
    Dir_NorthWest = 7
  } EDirections;

private:
  // appearance type ID
  DWORD m_dwID;

  // array of file names of animations for directions
  // if some is empty -> no anim associated there (just the black screen :-) )
  CString m_aDirectionFileNames[8];

  // our name -> we're in editor, we MUST have a name (unfortunetly)
  CString m_strName;

  // some instance of us -> this can be cached (it means not present)
  // for this using some default color (for example red)
  CEUnitAppearance *m_pDefaultAppearance;

  enum{Cache_Size = 3};

  // instance of us -> used in unit dialog only (it loads all graphics for one unit)
  // we have two instance at same time (can be) cause this one is used only by the dialogs
  // all other objects uses the default instance which is in some meaning cached
  CEUnitAppearance *m_pInstance[Cache_Size];

  // the archive for loading instances from
  CDataArchive m_Archive;

  // the parent unit type
  CEUnitType *m_pUnitType;

  friend CEUnitAppearance;
};

#endif // !defined(AFX_EUNITAPPEARANCETYPE_H__E43F8243_8FCC_11D3_A0C0_D87876DDE731__INCLUDED_)
