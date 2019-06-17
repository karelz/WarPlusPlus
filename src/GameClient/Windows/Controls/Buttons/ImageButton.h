// ImageButton.h: interface for the CImageButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGEBUTTON_H__7D1437A1_A8DF_11D3_99B8_E94672EEC579__INCLUDED_)
#define AFX_IMAGEBUTTON_H__7D1437A1_A8DF_11D3_99B8_E94672EEC579__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ButtonObject.h"
#include "ImageButtonLayout.h"
#include "..\ToolTip.h"

class CImageButton : public CButtonObject  
{
  DECLARE_DYNAMIC(CImageButton);
  DECLARE_OBSERVER_MAP(CImageButton);

public:
	virtual void Draw(CDDrawSurface *pSurface, CRect *pRect);
	virtual void Delete();
	BOOL Create(CPoint ptPosition, CImageButtonLayout *pLayout, CWindow *pParent, BOOL bRepeate = FALSE, CWindow *pDesktopWindow = NULL);
	CImageButton();
	virtual ~CImageButton();

  void SetToolTip(CString strText){ m_wndToolTip.SetText(strText); }
  void SetToolTip(UINT nTextID){ CString str; str.LoadString(nTextID); SetToolTip(str); }

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
  // pointer to the layout object
  CImageButtonLayout *m_pLayout;

  // the animation - it's the way the button looks right now
  CAnimationInstance m_Anim;

  // the tooltip
  CToolTip m_wndToolTip;

protected:
	virtual void OnLayoutRelease();
	virtual void OnLayoutPress();
	virtual void OnMouseDeactivate();
	virtual void OnMouseActivate();
	virtual void OnLayoutChange();
  virtual void OnDisableWindow();
	void OnAnimsRepaint(CAnimationInstance *pAnimation);

	virtual void ChooseLayout();
};

#endif // !defined(AFX_IMAGEBUTTON_H__7D1437A1_A8DF_11D3_99B8_E94672EEC579__INCLUDED_)
