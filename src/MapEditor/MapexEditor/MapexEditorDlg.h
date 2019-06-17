#if !defined(AFX_MAPEXEDITORDLG_H__43E086DA_6434_11D3_A054_ADE3A89A0638__INCLUDED_)
#define AFX_MAPEXEDITORDLG_H__43E086DA_6434_11D3_A054_ADE3A89A0638__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MapexEditorDlg.h : header file
//

#include "..\DataObjects\EMap.h"
#include "..\Controls\MapexPreview.h"	// Added by ClassView

/////////////////////////////////////////////////////////////////////////////
// CMapexEditorDlg dialog

class CMapexEditorDlg : public CDialog
{
// Construction
public:
	CMapexPreview m_wndMapexPreview;
	void Create(CEMap *pMap);
	CMapexEditorDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMapexEditorDlg)
	enum { IDD = IDD_MAPEXEDITOR };
	CListCtrl	m_MapexList;
	CButton	m_EditMapex;
	CButton	m_DeleteMapex;
	CComboBox	m_LibraryCombo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapexEditorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMapexEditorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelEndOkLibraryCombo();
	afx_msg void OnAddMapex();
	afx_msg void OnEditMapex();
	afx_msg void OnDeleteMapex();
	afx_msg void OnDblClkMapexList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChangedMapexList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CEMapex * m_pCurrentMapex;
	void UpdateSelectedMapex();
	void UpdateListItems();
	void UpdateSelectedLibrary();
	CEMapexLibrary * m_pCurrentMapexLibrary;
	CEMap * m_pMap;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPEXEDITORDLG_H__43E086DA_6434_11D3_A054_ADE3A89A0638__INCLUDED_)
