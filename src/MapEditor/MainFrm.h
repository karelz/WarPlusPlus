// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__43E086CB_6434_11D3_A054_ADE3A89A0638__INCLUDED_)
#define AFX_MAINFRM_H__43E086CB_6434_11D3_A054_ADE3A89A0638__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Controls\MapexToolBar.h"
#include "Controls\UnitToolBar.h"
#include "MiniMapFrame.h"	// Added by ClassView
#include "Controls\ZPosCombo.h"

class CEMap;
class CFinishDlg;

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
  void CloseToolbars();
  void OpenToolbars();
	void ResetUnitDrawingSelection();
	void ResetMapexDrawingSelection();
	void EnableZPosCombo(BOOL bEnable);
	DWORD GetZPosComboValue();
	void SetZPosComboValue(DWORD dwZPos);
	void SetDrawingMapexPaneText(CString strText);
	BOOL m_bMaximized;
	void SetSelectedMapexPaneText(CString strText);
	void OnNewMap(CEMap *pMap);
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

  CMapexToolBar m_wndMapexBar;
  CUnitToolBar m_wndUnitBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnUpdateViewMinimap(CCmdUI* pCmdUI);
	afx_msg void OnViewMinimap();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnUpdateViewMapexbar(CCmdUI* pCmdUI);
	afx_msg void OnViewMapexbar();
  afx_msg void OnSelEndMapexZPos();
	afx_msg void OnOk();
	afx_msg void OnFileRebuildLibraries();
	afx_msg void OnFileFinish();
	afx_msg void OnViewUnitbar();
	afx_msg void OnUpdateViewUnitbar(CCmdUI* pCmdUI);
	afx_msg void OnFileFinishExit();
	//}}AFX_MSG
  afx_msg void OnDropDownGrid(NMHDR *pNotifyStruct, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
private:
  void ExportMinimaps(CEMap *pMap, CDataArchive Archive, CFinishDlg *pFinishDlg);
  void FinishMap(BOOL bExit);

	CMiniMapFrame *m_pMiniMapFrame;
	CEMap * m_pMap;
  CZPosCombo m_ZPosCombo;
  CFont m_ComboFont;

  // The toolbars image lists
  CImageList m_imglsToolBarColor;
  CImageList m_imglsToolBarGray;
  CImageList m_imglsToolBarDisabled;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__43E086CB_6434_11D3_A054_ADE3A89A0638__INCLUDED_)
