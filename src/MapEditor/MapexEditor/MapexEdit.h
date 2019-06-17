#if !defined(AFX_MAPEXEDIT_H__80A94715_681A_11D3_A05A_92D31502DF31__INCLUDED_)
#define AFX_MAPEXEDIT_H__80A94715_681A_11D3_A05A_92D31502DF31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MapexEdit.h : header file
//

#include "..\DataObjects\EMapexLibrary.h"
#include "..\DataObjects\EMapex.h"
#include "LandTypeTBButton.h"
#include "MapexEditView.h"	// Added by ClassView

/////////////////////////////////////////////////////////////////////////////
// CMapexEdit dialog

class CMapexEdit : public CDialog
{
// Construction
public:
	static void LoadState(CString strKey);
	static void SaveState(CString strKey);
	HICON m_hIcon;
	void Create(CEMapex *pMapex, CEMapexLibrary *pMapexLibrary);
	CMapexEdit(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMapexEdit)
	enum { IDD = IDD_MAPEX };
	CString	m_strName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapexEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMapexEdit)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnSetGraphics();
  afx_msg void OnSetSize();
  afx_msg void OnViewType();
	afx_msg void OnOk();
	//}}AFX_MSG
  afx_msg void OnLandTypeButton(UINT nID);
  afx_msg void OnLandTypeButtonDblClk(UINT nID, NMHDR *pNotifyStruct, LRESULT *pResult);
  afx_msg void OnDropDownViewGraphics(NMHDR *pNotifyStruct, LRESULT *pResult);
  afx_msg void OnViewGraphics();
  afx_msg void OnViewGrid();
  afx_msg void OnViewBoth();
	DECLARE_MESSAGE_MAP()
private:
  CMapexEditView m_wndView;
  void UpdateStatusBar();
	CLandType * m_pCurrentLandType;
	CStatusBarCtrl m_wndStatusBar;
	CEMapexLibrary * m_pMapexLibrary;
  CTypedPtrArray<CPtrArray, CLandTypeTBButton *> m_aLandTypeButtons;
  CToolBarCtrl m_wndToolBar;
	void AutoSize();
	CEMapex * m_pMapex;

  static BYTE m_aLandTypesSelected[10]; // remeber here ladn types selected
    // in TB buttons
  static int m_nSelectedLandType; // remeber last selected land type
  static RECT m_rectWindow; // last position of the window
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPEXEDIT_H__80A94715_681A_11D3_A05A_92D31502DF31__INCLUDED_)
