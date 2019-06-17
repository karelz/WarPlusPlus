// ToolTip.h: interface for the CToolTip class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TOOLTIP_H__71BFAD55_E2CE_11D3_A8C4_00105ACA8325__INCLUDED_)
#define AFX_TOOLTIP_H__71BFAD55_E2CE_11D3_A8C4_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\WINDOW\Window.h"
#include "ToolTipLayout.h"

class CToolTip : public CWindow  
{
  DECLARE_DYNAMIC(CToolTip)

public:
	virtual void Draw(CDDrawSurface *pSurface, CRect *pRect);
	virtual void Show();
	virtual void Hide();
  BOOL IsEmpty(){ return m_strText.IsEmpty(); }
	void SetText(CString strText);
	virtual void Delete();
	BOOL Create(CString strText, CToolTipLayout *pLayout, CWindow *pParent, CWindow *pDesktopWindow = NULL);
	CToolTip();
	virtual ~CToolTip();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
	virtual void OnDeactivate();
  // our layout
  CToolTipLayout *m_pLayout;

  // the text
  CString m_strText;

  // the owner window (have to remeber, cause we will create the window more than once)
  CWindow *m_pOwner;

  // Window which we will be the child of
  CWindow *m_pParentWindow;
private:
  // buffer -> for drawing the window
  CScratchSurface m_Buffer;

  // buffer width
  enum{ BufferWidth = 20 };

  // TRUE is the tooltip is visible (active)
  BOOL m_bVisible;

};

#endif // !defined(AFX_TOOLTIP_H__71BFAD55_E2CE_11D3_A8C4_00105ACA8325__INCLUDED_)
