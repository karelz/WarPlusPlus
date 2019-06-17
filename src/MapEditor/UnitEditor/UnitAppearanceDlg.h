#if !defined(AFX_UNITAPPEARANCEDLG_H__8CA351A4_92D4_11D3_A0C7_92DF3C7AE131__INCLUDED_)
#define AFX_UNITAPPEARANCEDLG_H__8CA351A4_92D4_11D3_A0C7_92DF3C7AE131__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnitAppearanceDlg.h : header file
//

#include "..\DataObjects\EUnitAppearanceType.h"
#include "UnitAnimationPreview.h"

/////////////////////////////////////////////////////////////////////////////
// CUnitAppearanceDlg dialog

class CUnitAppearanceDlg : public CDialog
{
// Construction
public:
	void Create(CEUnitAppearanceType *pAppearanceType);
	CUnitAppearanceDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUnitAppearanceDlg)
	enum { IDD = IDD_UNITAPPEARANCE };
	CEdit	m_wndName;
	CString	m_strName;
	//}}AFX_DATA

  BOOL m_bEditName;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUnitAppearanceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUnitAppearanceDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CEUnitAppearanceType * m_pAppearanceType;

  // the animation previews
  CUnitAnimationPreview m_wndNorth;
  CUnitAnimationPreview m_wndNorthEast;
  CUnitAnimationPreview m_wndEast;
  CUnitAnimationPreview m_wndSouthEast;
  CUnitAnimationPreview m_wndSouth;
  CUnitAnimationPreview m_wndSouthWest;
  CUnitAnimationPreview m_wndWest;
  CUnitAnimationPreview m_wndNorthWest;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNITAPPEARANCEDLG_H__8CA351A4_92D4_11D3_A0C7_92DF3C7AE131__INCLUDED_)
