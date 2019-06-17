#if !defined(AFX_IMAGETBBUTTON_H__AA867AC5_68EB_11D3_A05D_B2109F41DA31__INCLUDED_)
#define AFX_IMAGETBBUTTON_H__AA867AC5_68EB_11D3_A05D_B2109F41DA31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImageTBButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImageTBButton window

class CImageTBButton : public CWnd
{
// Construction
public:
	CImageTBButton();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageTBButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetSelected(BOOL bSelected);
	BOOL GetSelected();
	void Insert(CToolBarCtrl *pToolBar, int nIndex, UINT nID);
	void Create(CRect &rcBound, CWnd *pParent, UINT nID);
	virtual ~CImageTBButton();

  enum{ N_DOUBLECLICKED = 1 };

	// Generated message map functions
protected:
	virtual void BeforePaint();
	CScratchSurface m_Buffer;
	CDDrawClipper m_Clipper;
	BOOL m_bSelected;
	CDDrawSurface * m_pImage;
	DWORD m_dwColor;
	//{{AFX_MSG(CImageTBButton)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGETBBUTTON_H__AA867AC5_68EB_11D3_A05D_B2109F41DA31__INCLUDED_)
