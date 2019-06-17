#if !defined(AFX_LANDTYPESELECTDLG_H__39607C57_6A80_11D3_8C66_00105ACA8325__INCLUDED_)
#define AFX_LANDTYPESELECTDLG_H__39607C57_6A80_11D3_8C66_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LandTypeSelectDlg.h : header file
//

#include "..\DataObjects\LandType.h"
#include "..\DataObjects\EMap.h"

/////////////////////////////////////////////////////////////////////////////
// CLandTypeSelectDlg dialog

class CLandTypeSelectDlg : public CDialog
{
// Construction
public:
	CLandType * GetSelected();
	void Create(CEMap *pMap, CLandType *pSelected);
	CLandTypeSelectDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLandTypeSelectDlg)
	enum { IDD = IDD_LANDTYPESELECT };
	CListCtrl	m_List;
	CButton	m_OK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLandTypeSelectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLandTypeSelectDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblClkLandTypeList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChangedLandTypeList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void UpdateSelected();
	CLandType * m_pSelected;
	CEMap * m_pMap;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LANDTYPESELECTDLG_H__39607C57_6A80_11D3_8C66_00105ACA8325__INCLUDED_)
