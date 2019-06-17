#if !defined(AFX_SELECTMAPEXDLG_H__2315213C_6E73_11D3_A06B_C85E707FE131__INCLUDED_)
#define AFX_SELECTMAPEXDLG_H__2315213C_6E73_11D3_A06B_C85E707FE131__INCLUDED_

#include "..\Controls\MapexPreview.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectMapexDlg.h : header file
//

#include "..\DataObjects\EMapexLibrary.h"
#include "..\DataObjects\EMapex.h"
#include "..\DataObjects\EMap.h"

/////////////////////////////////////////////////////////////////////////////
// CSelectMapexDlg dialog

class CSelectMapexDlg : public CDialog
{
// Construction
public:
	CEMapex * GetSelected();
	void Create(CEMap *pMap);
	CSelectMapexDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelectMapexDlg)
	enum { IDD = IDD_SELECTMAPEX };
	CListCtrl	m_MapexList;
	CButton	m_OK;
	CComboBox	m_LibraryCombo;
	CString	m_strMapexSize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectMapexDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectMapexDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelendOKLibraryCombo();
	afx_msg void OnDblClkMapexList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChangedMapexList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	static int m_nLastSelectedLibrary;
	CEMap * m_pMap;
	CEMapexLibrary * m_pCurrentMapexLibrary;
	CEMapex * m_pCurrentMapex;
	CMapexPreview m_wndMapexPreview;
	void UpdateSelectedMapex();
	void UpdateSelectedLibrary();
	void UpdateListItems();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTMAPEXDLG_H__2315213C_6E73_11D3_A06B_C85E707FE131__INCLUDED_)
