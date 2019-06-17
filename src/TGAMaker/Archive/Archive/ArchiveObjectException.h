// ArchiveException.h: interface for the CArchiveException class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARCHIVEOBJECTEXCEPTION_H__2A02EFD4_E521_11D2_AC2F_8B60B3636260__INCLUDED_)
#define AFX_ARCHIVEOBJECTEXCEPTION_H__2A02EFD4_E521_11D2_AC2F_8B60B3636260__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CArchiveObjectException : public CException  
{
public:
	CArchiveObjectException(int nType, CString strArhive, CString strFile);
  CArchiveObjectException(CArchiveObjectException &source);
	virtual ~CArchiveObjectException();
  virtual BOOL GetErrorMessage(LPSTR lpszError, UINT nMaxError, PUINT pnHelpContext);

  typedef enum {
    NoError = 0,
    FileNotExists = 1,
    ArchiveNotExists = 2
  } ETypes;

protected:
	int m_nType;
	CString m_strFile;
	CString m_strArchive;
};

#endif // !defined(AFX_ARCHIVEOBJECTEXCEPTION_H__2A02EFD4_E521_11D2_AC2F_8B60B3636260__INCLUDED_)
