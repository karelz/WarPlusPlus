// AnimationEditorDoc.h : interface of the CAnimationEditorDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANIMATIONEDITORDOC_H__39A64357_8327_439E_B8F3_BEA6EB37EEBA__INCLUDED_)
#define AFX_ANIMATIONEDITORDOC_H__39A64357_8327_439E_B8F3_BEA6EB37EEBA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DataObjects\AAnimation.h"

class CAnimationEditorDoc : public CDocument
{
protected: // create from serialization only
	CAnimationEditorDoc();
	DECLARE_DYNCREATE(CAnimationEditorDoc)

// Attributes
public:
  enum EUpdates
  {
    Update_NoSpecial = 0,
    Update_NewAnimation = 1,
  };

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimationEditorDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void DeleteContents();
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAnimationEditorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

  // Returns the animation object
  CAAnimation * GetAnimation () const { ASSERT_VALID ( this ); return const_cast < CAAnimation * > ( &m_cAnimation ); }

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CAnimationEditorDoc)
	afx_msg void OnOk();
	afx_msg void OnColoringHueLess();
	afx_msg void OnColoringHueMore();
	afx_msg void OnUpdateColoringHueMore(CCmdUI* pCmdUI);
	afx_msg void OnUpdateColoringHueLess(CCmdUI* pCmdUI);
	afx_msg void OnColoringSatMore();
	afx_msg void OnColoringSatLess();
	afx_msg void OnUpdateColoringSatMore(CCmdUI* pCmdUI);
	afx_msg void OnUpdateColoringSatLess(CCmdUI* pCmdUI);
	afx_msg void OnColoringValMore();
	afx_msg void OnColoringValLess();
	afx_msg void OnUpdateColoringValMore(CCmdUI* pCmdUI);
	afx_msg void OnUpdateColoringValLess(CCmdUI* pCmdUI);
	afx_msg void OnColoringSourceColor();
	afx_msg void OnUpdateAnimationLoop(CCmdUI* pCmdUI);
	afx_msg void OnAnimationLoop();
	afx_msg void OnUpdateAnimationCrop(CCmdUI* pCmdUI);
	afx_msg void OnAnimationCrop();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
// Data members
  // The animation represented by this document
  CAAnimation m_cAnimation;

// Helper functions
  // Updates coloring informations
  // If the param is false , the coloring info is written to controls
  // elseway it's readfrom controls to memory structures
  void UpdateColoring ( bool bSave );
  // Updates current frame informations
  // If the param is false, the current frame is written to controls
  // elseway it's read from the controls to memory structures
  void UpdateCurrentFrame ( bool bSave );
  // Updates delays informations
  void UpdateDelays ( bool bSave );
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMATIONEDITORDOC_H__39A64357_8327_439E_B8F3_BEA6EB37EEBA__INCLUDED_)
