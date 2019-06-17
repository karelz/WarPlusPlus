// DirectSoundException.h: interface for the CDirectSoundException class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIRECTSOUNDEXCEPTION_H__E65A6AB5_9E29_11D2_ABB3_DA1727655660__INCLUDED_)
#define AFX_DIRECTSOUNDEXCEPTION_H__E65A6AB5_9E29_11D2_ABB3_DA1727655660__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDirectSoundException : public CException  
{
public:
	virtual BOOL GetErrorMessage(LPSTR lpszError, UINT nMaxError, PUINT pnHelpContext);
	CDirectSoundException(HRESULT hResult);
  CDirectSoundException(CDirectSoundException &source);
	virtual ~CDirectSoundException();

private:
  HRESULT m_hResult;
};

#define DIRECTSOUND_ERROR(hResult) \
  if(hResult != DS_OK){ \
    TRACE("%s(%d) : exception : DirectSound Exception thrown.\n", __FILE__, __LINE__); \
    THROW(new CDirectSoundException(hResult)); \
  }

#endif // !defined(AFX_DIRECTSOUNDEXCEPTION_H__E65A6AB5_9E29_11D2_ABB3_DA1727655660__INCLUDED_)
