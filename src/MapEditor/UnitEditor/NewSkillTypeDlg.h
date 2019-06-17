#if !defined(AFX_NEWSKILLTYPEDLG_H__D86699D6_94F0_11D3_A0D2_9EADB408E531__INCLUDED_)
#define AFX_NEWSKILLTYPEDLG_H__D86699D6_94F0_11D3_A0D2_9EADB408E531__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewSkillTypeDlg.h : header file
//

#include "UnitSkillTypeDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CNewSkillTypeDlg dialog

class CNewSkillTypeDlg : public CDialog
{
// Construction
public:
  CUnitSkillTypeDlg *GetSelectedSkillType(){ return m_pCurrentDlg; }
	CNewSkillTypeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewSkillTypeDlg)
	enum { IDD = IDD_NEWSKILLTYPE };
	CButton	m_wndOK;
	CListCtrl	m_wndList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewSkillTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewSkillTypeDlg)
	afx_msg void OnItemChangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClkList(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
  CUnitSkillTypeDlg *m_pCurrentDlg;
  void UpdateSelectedItem();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWSKILLTYPEDLG_H__D86699D6_94F0_11D3_A0D2_9EADB408E531__INCLUDED_)
