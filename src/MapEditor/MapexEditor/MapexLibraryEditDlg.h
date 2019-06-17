#if !defined(AFX_MAPEXLIBRARYEDITDLG_H__6868A9A9_6755_11D3_A059_B7FF8C0EE331__INCLUDED_)
#define AFX_MAPEXLIBRARYEDITDLG_H__6868A9A9_6755_11D3_A059_B7FF8C0EE331__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MapexLibraryEditDlg.h : header file
//

#include "..\DataObjects\EMapexLibrary.h"

/////////////////////////////////////////////////////////////////////////////
// CMapexLibraryEditDlg dialog

class CMapexLibraryEditDlg : public CDialog
{
// Construction
public:
	void Create(CEMapexLibrary *pMapexLibrary);
	CMapexLibraryEditDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMapexLibraryEditDlg)
	enum { IDD = IDD_MAPEXLIBRARYEDITOR };
	CString	m_strFileName;
	CString	m_strName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapexLibraryEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMapexLibraryEditDlg)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CEMapexLibrary * m_pMapexLibrary;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPEXLIBRARYEDITDLG_H__6868A9A9_6755_11D3_A059_B7FF8C0EE331__INCLUDED_)
