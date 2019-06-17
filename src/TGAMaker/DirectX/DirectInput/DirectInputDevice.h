#if !defined(AFX_DIRECTINPUTDEVICE_H__C9968EE5_4CD8_11D2_8EB5_947204C10000__INCLUDED_)
#define AFX_DIRECTINPUTDEVICE_H__C9968EE5_4CD8_11D2_8EB5_947204C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DirectInputDevice.h : header file
//

#ifdef DIRECTINPUT_VERSION
#undef DIRECTINPUT_VERSION
#endif // DIRECTINPUT_VERSION

#define DIRECTINPUT_VERSION 0x0300 // Define the DirectX 3 version
#include <dinput.h>

/////////////////////////////////////////////////////////////////////////////
// CDirectInputDevice command target

class CDirectInputDevice : public CObject
{
	DECLARE_DYNAMIC(CDirectInputDevice)

public:	
  CDirectInputDevice();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Implementation
public:
	virtual BOOL IsValid();
	HRESULT GetDeviceData(LPDIDEVICEOBJECTDATA rgdod, LPDWORD dwElements, DWORD dwFlags = 0);
	void GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph);
	void SetProperty(REFGUID rguid, LPCDIPROPHEADER pdiph);
	void SetEventNotification(HANDLE hEvent);
	void Unacquire();
	void Acquire();
	virtual BOOL PreCreate(const GUID *&rguid);
	LPDIRECTINPUTDEVICE GetLP();
	void Delete();
	virtual BOOL Create(LPDIRECTINPUTDEVICE lpDID, CWnd *pWnd);
  virtual ~CDirectInputDevice();

#ifdef _DEBUG
  virtual void Dump(CDumpContext &dc) const;
  virtual void AssertValid() const;
#endif

protected:
	const GUID *m_pguid;
	HRESULT m_hResult;
	LPDIRECTINPUTDEVICE m_lpDID;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIRECTINPUTDEVICE_H__C9968EE5_4CD8_11D2_8EB5_947204C10000__INCLUDED_)
