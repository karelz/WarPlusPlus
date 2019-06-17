// ToolbarsErrorOutput.h: interface for the CToolbarsErrorOutput class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TOOLBARSERROROUTPUT_H__EDA298D4_2D9B_11D4_84D4_004F4E0004AA__INCLUDED_)
#define AFX_TOOLBARSERROROUTPUT_H__EDA298D4_2D9B_11D4_84D4_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Object used to write errors, which occure while parsing the toolbars
class CToolbarsErrorOutput : public CObject  
{
  DECLARE_DYNAMIC(CToolbarsErrorOutput);

public:
  // Constructor & destructor
	CToolbarsErrorOutput();
	virtual ~CToolbarsErrorOutput();

  // Debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // Creation
  // Creates the error output file in the givven file (name and archive)
  void Create(CString strFileName, CDataArchive Archive);

  // Deletes the object (this will close the file)
  virtual void Delete();

  // Writes error string to the file
  // DO NOT include the EOL it will be added automaticaly
  void WriteError(CString strError);

  // Writes exception string to the file
  void WriteError(CException *pException);

private:
  // The file with errors (this is opened while the object is created)
  CArchiveFile m_File;
};

#endif // !defined(AFX_TOOLBARSERROROUTPUT_H__EDA298D4_2D9B_11D4_84D4_004F4E0004AA__INCLUDED_)
