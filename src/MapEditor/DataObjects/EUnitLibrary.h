// EUnitLibrary.h: interface for the CEUnitLibrary class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EUNITLIBRARY_H__A2ABF065_8EA7_11D3_A877_00105ACA8325__INCLUDED_)
#define AFX_EUNITLIBRARY_H__A2ABF065_8EA7_11D3_A877_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EUnitType.h"

class CEMap;

class CEUnitLibrary : public CObject  
{
  DECLARE_DYNAMIC(CEUnitLibrary)

public:
	void Rebuild();
	void DeleteUnitType(CEUnitType *pUnitType);
	CEUnitType * NewUnitType();
	void FillUnitTypesListCtrl(CListCtrl *pListCtrl);
	CString GetFullPath();
	CString GetFileName();
	virtual void Delete();
	void SaveLibrary();
	// creates the library
  // if the bNew is FALSE -> loads library from archive
  // elseway it just creates new empty library
  // bothways -> the archive must exists
  BOOL Create(CDataArchive Archive, BOOL bNew, CEMap *pMap);

  // gets the name
  CString GetName(){ return m_strName; }
  // sets the name
  void SetName(CString strName){ m_strName = strName; }

  // constructor & destructor
	CEUnitLibrary();
	virtual ~CEUnitLibrary();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
	void DeleteAll();
	void LoadLibrary(CDataArchive Archive);

  // the name
  CString m_strName;

  // the unit list
  CTypedPtrList<CPtrList, CEUnitType *> m_listUnits;

  // the archive of the library
  CDataArchive m_Archive;

  // pointer to the map object
  CEMap *m_pMap;
};

#endif // !defined(AFX_EUNITLIBRARY_H__A2ABF065_8EA7_11D3_A877_00105ACA8325__INCLUDED_)
