#if !defined(AFX_UNITLIBRARYEDITORDLG_H__E43F8249_8FCC_11D3_A0C0_D87876DDE731__INCLUDED_)
#define AFX_UNITLIBRARYEDITORDLG_H__E43F8249_8FCC_11D3_A0C0_D87876DDE731__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnitLibraryEditorDlg.h : header file
//

#include "..\DataObjects\EUnitLibrary.h"

/////////////////////////////////////////////////////////////////////////////
// CUnitLibraryEditorDlg dialog

class CUnitLibraryEditorDlg : public CDialog
{
// Construction
public:
	BOOL Create(CEUnitLibrary *pLibrary);
	CUnitLibraryEditorDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUnitLibraryEditorDlg)
	enum { IDD = IDD_UNITLIBRARYEDITOR };
	CString	m_strFileName;
	CString	m_strName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUnitLibraryEditorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUnitLibraryEditorDlg)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
  CEUnitLibrary *m_pUnitLibrary;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNITLIBRARYEDITORDLG_H__E43F8249_8FCC_11D3_A0C0_D87876DDE731__INCLUDED_)
