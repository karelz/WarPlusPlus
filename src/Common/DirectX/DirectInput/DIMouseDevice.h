// DIMouseDevice.h: interface for the CDIMouseDevice class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIMOUSEDEVICE_H__C9968EE6_4CD8_11D2_8EB5_947204C10000__INCLUDED_)
#define AFX_DIMOUSEDEVICE_H__C9968EE6_4CD8_11D2_8EB5_947204C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "DirectInputDevice.h"

class CDIMouseDevice : public CDirectInputDevice  
{
  DECLARE_DYNAMIC(CDIMouseDevice);

public:
	CWnd * m_pWnd;
	virtual BOOL Create(LPDIRECTINPUTDEVICE lpDID, CWnd *pWnd);
	virtual BOOL PreCreate(const GUID *&rguid);
	CDIMouseDevice();
	virtual ~CDIMouseDevice();

#ifdef _DEBUG
  virtual void Dump(CDumpContext &dc) const;
  virtual void AssertValid() const;
#endif

};

#endif // !defined(AFX_DIMOUSEDEVICE_H__C9968EE6_4CD8_11D2_8EB5_947204C10000__INCLUDED_)
