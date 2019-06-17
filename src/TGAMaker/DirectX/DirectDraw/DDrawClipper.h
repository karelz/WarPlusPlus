// DDrawClipper.h: interface for the CDDrawClipper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DDRAWCLIPPER_H__22579AB8_B698_11D2_8BBD_00105ACA8325__INCLUDED_)
#define AFX_DDRAWCLIPPER_H__22579AB8_B698_11D2_8BBD_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DirectDraw.h"

class CDDrawClipper : public CObject
{
public:
	virtual BOOL Create(CWnd *pWnd);
	void SetHWnd(CWnd *pWnd);
	BOOL IsClipListChanged();
	virtual BOOL Create();
	virtual void Delete();
	CDDrawClipper();
	virtual ~CDDrawClipper();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
	HRESULT m_hResult;
	BOOL m_bRemoveFromList;
	CDDrawClipper * m_pNextClipper;
	CDirectDraw * m_pDirectDraw;
	LPDIRECTDRAWCLIPPER m_lpDDClipper;

  friend CDirectDraw;
  friend CDDrawSurface;
};

#endif // !defined(AFX_DDRAWCLIPPER_H__22579AB8_B698_11D2_8BBD_00105ACA8325__INCLUDED_)
