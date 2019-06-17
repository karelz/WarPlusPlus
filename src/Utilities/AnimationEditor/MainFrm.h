// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__87BA749E_98AA_4B6C_A184_598666D74007__INCLUDED_)
#define AFX_MAINFRM_H__87BA749E_98AA_4B6C_A184_598666D74007__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Controls\ColorPicker.h"

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

  DWORD GetCurrentFrameFromControl ();
  void SetCurrentFrameForControl ( DWORD dwCurrentFrame );

  double GetHueToleranceFromControl () { CString strText; m_wndHueValue.GetWindowText ( strText ); return atof ( strText ); }
  void SetHueToleranceForControl ( double dbHueTolerance ) { CString strText; strText.Format ( "%.02f", dbHueTolerance ); m_wndHueValue.SetWindowText ( strText ); }
  double GetSatToleranceFromControl () { CString strText; m_wndSatValue.GetWindowText ( strText ); return atof ( strText ); }
  void SetSatToleranceForControl ( double dbSatTolerance ) { CString strText; strText.Format ( "%.02f", dbSatTolerance ); m_wndSatValue.SetWindowText ( strText ); }
  double GetValToleranceFromControl () { CString strText; m_wndValValue.GetWindowText ( strText ); return atof ( strText ); }
  void SetValToleranceForControl ( double dbValTolerance ) { CString strText; strText.Format ( "%.02f", dbValTolerance ); m_wndValValue.SetWindowText ( strText ); }
  DWORD GetSourceColorFromControl () { return m_wndSourceColor.GetColor (); }
  void SetSourceColorForControl ( DWORD dwSourceColor ) { m_wndSourceColor.SetColor ( dwSourceColor ); }
  DWORD GetDefaultAnimationDelayFromControl () { CString strText; m_wndAnimationDelay.GetWindowText ( strText ); return atol ( strText ); }
  void SetDefaultAnimationDelayForControl ( DWORD dwDelay ) { CString strText; strText.Format ( "%d", dwDelay ); m_wndAnimationDelay.SetWindowText ( strText ); }
  DWORD GetFrameDelayFromControl () { CString strText; m_wndFrameDelay.GetWindowText ( strText ); return atol ( strText ); }
  void SetFrameDelayForControl ( DWORD dwDelay ) { CString strText; strText.Format ( "%d", dwDelay ); m_wndFrameDelay.SetWindowText ( strText ); }

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
  CToolBar    m_wndAnimationToolBar;
  CToolBar    m_wndColoringToolBar;
  CToolBar    m_wndFrameToolBar;

  // The current frame edit box
  CEdit m_wndCurrentFrame;
  CFont m_cCurrentFrameFont;

  //The animation controls
  CEdit m_wndAnimationDelay;
  CStatic m_wndAnimationDelayUnder;

  // The frame controls
  CEdit m_wndFrameDelay;
  CStatic m_wndFrameDelayUnder;

  // The coloring controls
  CEdit m_wndHueValue;
  CStatic m_wndHueValueUnder;
  CEdit m_wndSatValue;
  CStatic m_wndSatValueUnder;
  CEdit m_wndValValue;
  CStatic m_wndValValueUnder;
  CStatic m_wndHueCaption;
  CStatic m_wndSatCaption;
  CStatic m_wndValCaption;
  CColorPicker m_wndSourceColor;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewAnimationToolbar();
	afx_msg void OnUpdateViewAnimationToolbar(CCmdUI* pCmdUI);
	afx_msg void OnClose();
	afx_msg void OnUpdateViewColoringToolbar(CCmdUI* pCmdUI);
	afx_msg void OnViewColoringToolbar();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CMainFrame * g_pMainFrame;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__87BA749E_98AA_4B6C_A184_598666D74007__INCLUDED_)
