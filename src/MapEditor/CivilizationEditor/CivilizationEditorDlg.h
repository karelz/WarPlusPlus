#if !defined(AFX_CIVILIZATIONEDITORDLG_H__688CEF78_8A12_11D3_A0A6_FEA9F52CDF31__INCLUDED_)
#define AFX_CIVILIZATIONEDITORDLG_H__688CEF78_8A12_11D3_A0A6_FEA9F52CDF31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CivilizationEditorDlg.h : header file
//

#include "..\DataObjects\EMap.h"

/////////////////////////////////////////////////////////////////////////////
// CCivilizationEditorDlg dialog

class CCivilizationEditorDlg : public CDialog
{
// Construction
public:
	void Create(CEMap *pMap);
	CCivilizationEditorDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCivilizationEditorDlg)
	enum { IDD = IDD_CIVILIZATIONEDITOR };
	CButton	m_wndEdit;
	CButton	m_wndDelete;
	CListCtrl	m_wndList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCivilizationEditorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCivilizationEditorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnEdit();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChangedList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void UpdateSelectedCivilization();
	CEMap * m_pMap;

  BOOL IsCivUnique(CECivilization *pCiv);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CIVILIZATIONEDITORDLG_H__688CEF78_8A12_11D3_A0A6_FEA9F52CDF31__INCLUDED_)
