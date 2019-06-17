#if !defined(AFX_RESOURCESDLG_H__25FAD953_557F_11D4_B0B1_004F49068BD6__INCLUDED_)
#define AFX_RESOURCESDLG_H__25FAD953_557F_11D4_B0B1_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResourcesDlg.h : header file
//

#include "..\DataObjects\EMap.h"
#include "..\Controls\ResourcePreview.h"

/////////////////////////////////////////////////////////////////////////////
// CResourcesDlg dialog

class CResourcesDlg : public CDialog
{
// Construction
public:
	CResourcesDlg(CWnd* pParent = NULL);   // standard constructor

  void Create(CEMap *pMap);
  void Delete();

// Dialog Data
	//{{AFX_DATA(CResourcesDlg)
	enum { IDD = IDD_RESOURCES };
	CButton	m_wndSetImage;
	CListCtrl	m_wndList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResourcesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CResourcesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEndLabelEditResources(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetImage();
	afx_msg void OnItemChangedResources(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
  void UpdateSelectedResource();

  CEMap *m_pMap;
  CImageList m_ImageList;
  CEResource *m_pSelected;
  CResourcePreview m_wndPreview;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESOURCESDLG_H__25FAD953_557F_11D4_B0B1_004F49068BD6__INCLUDED_)
