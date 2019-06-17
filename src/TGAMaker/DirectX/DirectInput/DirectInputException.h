// DirectInputException.h: interface for the CDirectInputException class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIRECTINPUTEXCEPTION_H__C9968EE4_4CD8_11D2_8EB5_947204C10000__INCLUDED_)
#define AFX_DIRECTINPUTEXCEPTION_H__C9968EE4_4CD8_11D2_8EB5_947204C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CDirectInputException : public CException  
{
public:
	virtual BOOL GetErrorMessage(LPSTR lpszError, UINT nMaxError, PUINT pnHelpContext);
	CDirectInputException(HRESULT hResult);
  CDirectInputException(CDirectInputException &source);
	virtual ~CDirectInputException();

private:
	HRESULT m_hResult;
};

#define DIRECTINPUT_ERROR(hResult)\
  if(hResult != DI_OK){ \
    TRACE("%s(%d) : exception : DirectInput Exception thrown.\n", __FILE__, __LINE__); \
    THROW(new CDirectInputException(hResult)); \
  }

#endif // !defined(AFX_DIRECTINPUTEXCEPTION_H__C9968EE4_4CD8_11D2_8EB5_947204C10000__INCLUDED_)
