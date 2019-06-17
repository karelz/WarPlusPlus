#if !defined(AFX_CIVILIZATIONDLG_H__688CEF77_8A12_11D3_A0A6_FEA9F52CDF31__INCLUDED_)
#define AFX_CIVILIZATIONDLG_H__688CEF77_8A12_11D3_A0A6_FEA9F52CDF31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CivilizationDlg.h : header file
//

#include "..\Controls\ColorPicker.h"
#include "..\Controls\ResourceControl.h"
#include "..\DataObjects\EScriptSet.h"

class CEMap;
class CECivilization;

/////////////////////////////////////////////////////////////////////////////
// CCivilizationDlg dialog

class CCivilizationDlg : public CDialog
{
// Construction
public:
	CCivilizationDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCivilizationDlg)
	enum { IDD = IDD_CIVILIZATION };
	CComboBox	m_wndStartPos;
	CComboBox	m_wndScriptSet;
	CString	m_strName;
	//}}AFX_DATA
  DWORD m_dwColor;
  int *m_pResources;
  CEScriptSet *m_pScriptSet;

  void Create(CEMap *pMap, CECivilization *pCivilization){ m_pMap = pMap; m_pCivilization = pCivilization; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCivilizationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCivilizationDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelEndOkScriptSet();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
  CColorPicker m_wndColor;
  CResourceControl m_wndResource;
  CEMap *m_pMap;
  CECivilization *m_pCivilization;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CIVILIZATIONDLG_H__688CEF77_8A12_11D3_A0A6_FEA9F52CDF31__INCLUDED_)
