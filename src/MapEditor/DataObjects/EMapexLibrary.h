// EMapexLibrary.h: interface for the CEMapexLibrary class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EMAPEXLIBRARY_H__6868A9A6_6755_11D3_A059_B7FF8C0EE331__INCLUDED_)
#define AFX_EMAPEXLIBRARY_H__6868A9A6_6755_11D3_A059_B7FF8C0EE331__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EMapex.h"

class CEMap;

class CEMapexLibrary : public CObject  
{
  DECLARE_DYNAMIC(CEMapexLibrary);

public:
	void Rebuild();
  // returns the library archive
	CDataArchive GetArchive();

  // deletes the mapex from library
	void DeleteMapex(CEMapex *pMapex);
  // creates the mapex with givven size
	CEMapex * NewMapex(CSize sizeMapex);

  // fills list control with mapexes in this library
  // each item has as its data address of the mapex object (CEMapex)
	void FillMapexesListCtrl(CListCtrl *pListCtrl);

  // sets new library name
	void SetName(CString strName);
  // returns name of the library
	CString GetName();

  // returns only filename of the library
	CString GetFileName();
  // returns full path to the library
	CString GetFullPath();

  // saves the library to the disk
	void SaveLibrary();

  // deletes all mapexes
	void DeleteAll();
  // deletes the library
	virtual void Delete();
  // creates the library
	BOOL Create(CDataArchive Archive, BOOL bCreateNew, CEMap *pMap);

  // constructor
	CEMapexLibrary();
  // destructor
	virtual ~CEMapexLibrary();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
  // loads library from archive
	void LoadLibrary(CDataArchive Archive);
  // data archive for this library
	CDataArchive m_Archive;

  // array of mapexes
  CTypedPtrList<CPtrList, CEMapex *> m_listMapexes;

  // name of the library
  CString m_strName;

  // pointer to the parent map
  CEMap *m_pMap;
};

#endif // !defined(AFX_EMAPEXLIBRARY_H__6868A9A6_6755_11D3_A059_B7FF8C0EE331__INCLUDED_)
