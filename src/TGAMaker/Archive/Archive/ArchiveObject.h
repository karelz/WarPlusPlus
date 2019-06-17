// ArchiveObject.h: interface for the CArchiveObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARCHIVEOBJECT_H__BE6D3E07_9CA0_11D2_ABB1_BFAA62284960__INCLUDED_)
#define AFX_ARCHIVEOBJECT_H__BE6D3E07_9CA0_11D2_ABB1_BFAA62284960__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ArchiveDirectory.h"
#include "ArchiveFile.h"

class CArchiveObject : public CObject  
{
public:
	virtual BOOL CreateDirectory(CString strDirectory, CArchiveDirectory *pDirectory);
  // creates the file object for the file in this archive
  // FALSE - some error
  virtual BOOL CreateFile(CString strFile, CArchiveFile *pFile);
  // closes the archive (closes the file)
	virtual void Close();
  // opens the archive (opens the file)
	virtual void Open();
  // deletes the object
  virtual void Delete();
  // creates the archive for the givven file name
  virtual BOOL Create(CString strFile);
	// constructs the object
  CArchiveObject();
  // destructs the object
	virtual ~CArchiveObject();

private:
	CString m_strPath;
};

#endif // !defined(AFX_ARCHIVEOBJECT_H__BE6D3E07_9CA0_11D2_ABB1_BFAA62284960__INCLUDED_)
