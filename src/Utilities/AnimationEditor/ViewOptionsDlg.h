#if !defined(AFX_VIEWOPTIONSDLG_H__31705974_FBAC_4773_900C_246DA2C63089__INCLUDED_)
#define AFX_VIEWOPTIONSDLG_H__31705974_FBAC_4773_900C_246DA2C63089__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewOptionsDlg.h : header file
//

#include "Controls\ColorPicker.h"

/////////////////////////////////////////////////////////////////////////////
// CViewOptionsDlg dialog

class CViewOptionsDlg : public CDialog
{
// Construction
public:
	CViewOptionsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CViewOptionsDlg)
	enum { IDD = IDD_VIEWOPTIONS };
	BOOL	m_bShowCrossHair;
  DWORD m_dwBackgroundColor;
  DWORD m_dwAnimationColor;
  DWORD m_dwCrossHairColor;
	CString	m_strBkgAnimation;
	BOOL	m_bDisplayBackgroundAsOverlay;
	BOOL	m_bShowBoundingRectangle;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewOptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CViewOptionsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseBkgAnimation();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
  CColorPicker m_wndBackgroundColor;
  CColorPicker m_wndAnimationColor;
  CColorPicker m_wndCrossHairColor;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWOPTIONSDLG_H__31705974_FBAC_4773_900C_246DA2C63089__INCLUDED_)
