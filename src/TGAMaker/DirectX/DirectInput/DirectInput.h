#if !defined(AFX_DIRECTINPUT_H__C9968EE3_4CD8_11D2_8EB5_947204C10000__INCLUDED_)
#define AFX_DIRECTINPUT_H__C9968EE3_4CD8_11D2_8EB5_947204C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DirectInput.h : header file
//

#include "DirectInputException.h"
#include "DirectInputDevice.h"
#include "..\..\Events\Events.h"

/////////////////////////////////////////////////////////////////////////////
// CDirectInput command target

class CDirectInput : public CObserver
{
	DECLARE_DYNAMIC(CDirectInput)
  DECLARE_OBSERVER_MAP(CDirectInput)

public:
  CDirectInput();           // protected constructor used by dynamic creation
  virtual ~CDirectInput();

// Attributes
public:

// Operations
public:

// Implementation
public:
	BOOL CreateDevice(CDirectInputDevice &DIDevice);
	LPDIRECTINPUT GetLP();
	void Delete();
	BOOL Create(HINSTANCE hInstance, CWnd *pWnd);

#ifdef _DEBUG
  virtual void Dump(CDumpContext &dc) const;
  virtual void AssertValid() const;
#endif

private:
	CWnd * m_pWnd;
	LPDIRECTINPUT m_lpDI;
	HRESULT m_hResult;
protected:
	void OnAbort(DWORD dwExitCode);
};

extern CDirectInput *g_pDirectInput;

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_DIRECTINPUT_H__C9968EE3_4CD8_11D2_8EB5_947204C10000__INCLUDED_)
