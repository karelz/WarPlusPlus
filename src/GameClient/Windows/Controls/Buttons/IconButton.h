// IconButton.h: interface for the CIconButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ICONBUTTON_H__176028F4_AB37_11D3_99C2_99ADDD254479__INCLUDED_)
#define AFX_ICONBUTTON_H__176028F4_AB37_11D3_99C2_99ADDD254479__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ButtonObject.h"
#include "IconButtonLayout.h"
#include "..\ToolTip.h"

class CIconButton : public CButtonObject  
{
  DECLARE_DYNAMIC(CIconButton);
  DECLARE_OBSERVER_MAP(CIconButton);

public:
	virtual void Draw(CDDrawSurface *pSurface, CRect *pRect);
	void SetIcon(CAnimation *pIcon);
	virtual void Delete();
	BOOL Create(CPoint ptPosition, CAnimation *pIcon, CIconButtonLayout *pLayout, CWindow *pParent, BOOL bRepeate = FALSE, CWindow *pDesktopWindow = NULL);
	CIconButton();
	virtual ~CIconButton();

  void SetToolTip(CString strText){ m_wndToolTip.SetText(strText); }
  void SetToolTip(UINT nTextID){ CString str; str.LoadString(nTextID); SetToolTip(str); }

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
	virtual void OnLayoutRelease();
	virtual void OnLayoutPress();
	virtual void OnMouseDeactivate();
	virtual void OnMouseActivate();
	virtual void OnLayoutChange();
  virtual void OnDisableWindow();
	void OnAnimsRepaint(CAnimationInstance *pAnimation);
	void ChooseLayout();

private:
  // animation instance ofr the layout of the button
  CAnimationInstance m_Anim;
  // animation instance for the icon
  CAnimationInstance m_IconAnim;
  // the layout pointer
  CIconButtonLayout *m_pLayout;

  // the icon offset
  CPoint m_ptIconOffset;

  // the overlay buffer
  CScratchSurface m_OverlayBuffer;
  // the draw buffer
  CScratchSurface m_DrawBuffer;

  // the tooltip
  CToolTip m_wndToolTip;
};

#endif // !defined(AFX_ICONBUTTON_H__176028F4_AB37_11D3_99C2_99ADDD254479__INCLUDED_)
