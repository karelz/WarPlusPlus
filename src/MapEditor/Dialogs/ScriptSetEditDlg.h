#if !defined(AFX_SCRIPTSETEDITDLG_H__1C8EC1B6_79AB_11D4_B0ED_004F49068BD6__INCLUDED_)
#define AFX_SCRIPTSETEDITDLG_H__1C8EC1B6_79AB_11D4_B0ED_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScriptSetEditDlg.h : header file
//

#include "..\DataObjects\EScriptSet.h"
class CEMap;

/////////////////////////////////////////////////////////////////////////////
// CScriptSetEditDlg dialog

class CScriptSetEditDlg : public CDialog
{
// Construction
public:
	CScriptSetEditDlg(CWnd* pParent = NULL);   // standard constructor

  void Create(CEMap *pMap);

// Dialog Data
	//{{AFX_DATA(CScriptSetEditDlg)
	enum { IDD = IDD_SCRIPTSETEDIT };
	CButton	m_wndEdit;
	CButton	m_wndAdd;
	CButton	m_wndDelete;
	CListCtrl	m_wndList;
	CString	m_strPath;
	//}}AFX_DATA


  CEScriptSet *m_pSelected;

  void UpdateList();
  void UpdateSelected();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScriptSetEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CScriptSetEditDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDelete();
	afx_msg void OnAdd();
	afx_msg void OnEdit();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
  CEMap *m_pMap;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCRIPTSETEDITDLG_H__1C8EC1B6_79AB_11D4_B0ED_004F49068BD6__INCLUDED_)
