// StringException.h: interface for the CStringException class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRINGEXCEPTION_H__99A05D84_4106_11D3_A010_D031AF9E3D38__INCLUDED_)
#define AFX_STRINGEXCEPTION_H__99A05D84_4106_11D3_A010_D031AF9E3D38__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CStringException : public CException  
{
public:
	CStringException(CString strMessage);
  CStringException(UINT nResourceID);
  CStringException(CStringException &source);
	virtual ~CStringException();
  virtual BOOL GetErrorMessage(LPSTR lpszError, UINT nMaxError, PUINT pnHelpContext);

  CString m_strMessage;
};

#endif // !defined(AFX_STRINGEXCEPTION_H__99A05D84_4106_11D3_A010_D031AF9E3D38__INCLUDED_)
