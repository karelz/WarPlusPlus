// CaptionWindow.h: interface for the CCaptionWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAPTIONWINDOW_H__7A991695_E478_11D2_AC2E_8C7702B45660__INCLUDED_)
#define AFX_CAPTIONWINDOW_H__7A991695_E478_11D2_AC2E_8C7702B45660__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FrameWindow.h"
#include "CaptionWindowLayout.h"

class CCaptionWindow : public CFrameWindow  
{
  DECLARE_DYNAMIC(CCaptionWindow)

public:
	void SetCaption(CString strCaption);
	CString GetCaption();
	virtual void Draw(CDDrawSurface *pSurface, CRect *pRect);
	virtual void Delete();
	BOOL Create(CRect &rcBound, CString strCaption, CCaptionWindowLayout *pLayout, CWindow *pParent, BOOL bResizeable);
	CCaptionWindow();
	virtual ~CCaptionWindow();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif
private:
	CString m_strCaption;
protected:
	virtual void OnDeactivate();
	CCaptionWindowLayout * m_pCaptionLayout;
};

#endif // !defined(AFX_CAPTIONWINDOW_H__7A991695_E478_11D2_AC2E_8C7702B45660__INCLUDED_)
