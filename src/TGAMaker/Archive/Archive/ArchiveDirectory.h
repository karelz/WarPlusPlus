// ArchiveDirectory.h: interface for the CArchiveDirectory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARCHIVEDIRECTORY_H__BE6D3E08_9CA0_11D2_ABB1_BFAA62284960__INCLUDED_)
#define AFX_ARCHIVEDIRECTORY_H__BE6D3E08_9CA0_11D2_ABB1_BFAA62284960__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ArchiveFile.h"

class CArchiveObject;

class CArchiveDirectory : public CArchiveFile  
{
public:
  // return an array of names of files and directories
	virtual CStringArray * GetNames();
  // deletes the object
	virtual void Delete();
  // creates the object
	virtual BOOL Create();
  // constructs the object
	CArchiveDirectory();
  // destructs the object
	virtual ~CArchiveDirectory();

private:
  // array of names
	CStringArray m_aNames;

  friend CArchiveObject;
};

#endif // !defined(AFX_ARCHIVEDIRECTORY_H__BE6D3E08_9CA0_11D2_ABB1_BFAA62284960__INCLUDED_)
