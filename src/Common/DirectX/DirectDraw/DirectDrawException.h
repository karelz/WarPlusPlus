// DirectDrawException.h: interface for the CDirectDrawException class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIRECTDRAWEXCEPTION_H__A07C15B4_48B6_11D2_8EB5_CC7204C10000__INCLUDED_)
#define AFX_DIRECTDRAWEXCEPTION_H__A07C15B4_48B6_11D2_8EB5_CC7204C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CDirectDrawException : public CException  
{
public:
	virtual BOOL GetErrorMessage(LPSTR lpszError, UINT nMaxError, PUINT pnHelpContext);
	CDirectDrawException(HRESULT hresult);
  CDirectDrawException(CDirectDrawException &source);
	virtual ~CDirectDrawException();

private:
	HRESULT m_hResult;
};

#define DIRECTDRAW_ERROR(hResult) \
  if(hResult != DD_OK){ \
    if(hResult == DDERR_SURFACEBUSY){ CString s; s.Format("File %s Line %d Class %s", __FILE__, __LINE__, GetRuntimeClass()->m_lpszClassName); ::MessageBox(NULL, s, s, MB_OK); } \
    TRACE("%s(%d) : exception : DirectDraw Exception thrown.\n", __FILE__, __LINE__); \
    THROW(new CDirectDrawException(hResult));\
  }

#endif // !defined(AFX_DIRECTDRAWEXCEPTION_H__A07C15B4_48B6_11D2_8EB5_CC7204C10000__INCLUDED_)
