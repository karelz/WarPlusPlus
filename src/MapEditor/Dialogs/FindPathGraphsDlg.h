#if !defined(AFX_FINDPATHGRAPHSDLG_H__34B13455_80F0_11D4_B0FA_004F49068BD6__INCLUDED_)
#define AFX_FINDPATHGRAPHSDLG_H__34B13455_80F0_11D4_B0FA_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FindPathGraphsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFindPathGraphsDlg dialog

class CEMap;
class CEFindPathGraph;

class CFindPathGraphsDlg : public CDialog
{
// Construction
public:
	CFindPathGraphsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFindPathGraphsDlg)
	enum { IDD = IDD_FINDPATHGRAPHS };
	CListCtrl	m_wndList;
	CButton	m_wndEdit;
	CButton	m_wndDelete;
	CButton	m_wndAdd;
	//}}AFX_DATA


  void Create(CEMap *pMap){ m_pMap = pMap; ASSERT(pMap != NULL); }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindPathGraphsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFindPathGraphsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnItemChangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnEdit();
	afx_msg void OnDblClkList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
  CEMap *m_pMap;

  void UpdateSelectedItem();
  void UpdateList();
  CEFindPathGraph *m_pSelected;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDPATHGRAPHSDLG_H__34B13455_80F0_11D4_B0FA_004F49068BD6__INCLUDED_)
