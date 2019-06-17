#if !defined(AFX_MAPPROPERTIESDLG_H__6868A9A8_6755_11D3_A059_B7FF8C0EE331__INCLUDED_)
#define AFX_MAPPROPERTIESDLG_H__6868A9A8_6755_11D3_A059_B7FF8C0EE331__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MapPropertiesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMapPropertiesDlg dialog

class CMapPropertiesDlg : public CDialog
{
// Construction
public:
	CMapPropertiesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMapPropertiesDlg)
	enum { IDD = IDD_MAPPROPERTIES };
	CString	m_strFileName;
	CString	m_strMapDescription;
	CString	m_strMapName;
	DWORD	m_dwHeight;
	DWORD	m_dwWidth;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapPropertiesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMapPropertiesDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPPROPERTIESDLG_H__6868A9A8_6755_11D3_A059_B7FF8C0EE331__INCLUDED_)
