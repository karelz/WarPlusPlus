// ArchiveFile.h: interface for the CArchiveFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARCHIVEFILE_H__BE6D3E06_9CA0_11D2_ABB1_BFAA62284960__INCLUDED_)
#define AFX_ARCHIVEFILE_H__BE6D3E06_9CA0_11D2_ABB1_BFAA62284960__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CArchiveObject;

class CArchiveFile : public CFile  
{
public:
  // returns the length of the file
	virtual DWORD GetLength() const;
  // returns the current file pointer position
  virtual DWORD GetPosition() const;
  // returns the full path to the file (includes the file name)
  virtual CString GetFilePath() const;
  // returns the name of the file (without path)
  virtual CString GetFileName() const;
  // reads bytes from the current position
  // returns bytes count actually read
  virtual UINT Read(void *pBuffer, UINT nCount);
  // sets the file pointer to the new position
  // returns current position of the file pointer
  virtual LONG Seek(LONG lOffset, UINT uFlags);
  // closes the file
	virtual void Close();
  // opens the file
  virtual void Open();
	// deletes the object
  virtual void Delete();
  // creates the object (just fill it with nulls)
  // FALSE - some error
	virtual BOOL Create();
  // assignment
  void operator =(CArchiveFile &src){m_strFileName = src.m_strFileName; }
  // copy constructor
  CArchiveFile(const CArchiveFile &src);
	// just a constructor
  CArchiveFile();
  // destructs the object
	virtual ~CArchiveFile();



private:
	// name of the file
  CString m_strFileName;

  friend CArchiveObject;
};

#endif // !defined(AFX_ARCHIVEFILE_H__BE6D3E06_9CA0_11D2_ABB1_BFAA62284960__INCLUDED_)
