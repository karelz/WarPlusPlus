// GameServerAppView.h : interface of the CGameServerAppView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMESERVERAPPVIEW_H__8B3C8566_5C99_11D4_B527_00105ACA8325__INCLUDED_)
#define AFX_GAMESERVERAPPVIEW_H__8B3C8566_5C99_11D4_B527_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CGameServerAppView : public CTreeView
{
protected: // create from serialization only
	CGameServerAppView();
	DECLARE_DYNCREATE(CGameServerAppView)

// Attributes
public:
	CGameServerAppDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGameServerAppView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGameServerAppView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
  // The image list
  CImageList m_ImageList;

// Generated message map functions
protected:
	//{{AFX_MSG(CGameServerAppView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateProperties(CCmdUI* pCmdUI);
	afx_msg void OnProperties();
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCivilizationAddUser();
	afx_msg void OnUpdateCivilizationAddUser(CCmdUI* pCmdUI);
	afx_msg void OnRClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnCivilizationDeleteUser();
    afx_msg void OnUpdateCivilizationDeleteUser(CCmdUI* pCmdUI);
	afx_msg void OnUpdateUserDisable(CCmdUI* pCmdUI);
	afx_msg void OnUserDisable();
	afx_msg void OnUpdateUserEnable(CCmdUI* pCmdUI);
	afx_msg void OnUserEnable();
	afx_msg void OnPropertiesCivilization();
	afx_msg void OnPropertiesUser();
	afx_msg void OnUpdatePropertiesCivilization(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePropertiesUser(CCmdUI* pCmdUI);
    afx_msg void OnUserSendMessage();
    afx_msg void OnUpdateUserSendMessage(CCmdUI *pCmdUI);
    afx_msg void OnCivilizationSendMessage();
    afx_msg void OnUpdateCivilizationSendMessage(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSendMessage(CCmdUI* pCmdUI);
	afx_msg void OnSendMessage();
  afx_msg LRESULT OnProfileUpdated ( WPARAM sParam, LPARAM lParam );
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in GameServerAppView.cpp
inline CGameServerAppDoc* CGameServerAppView::GetDocument()
   { return (CGameServerAppDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GAMESERVERAPPVIEW_H__8B3C8566_5C99_11D4_B527_00105ACA8325__INCLUDED_)
