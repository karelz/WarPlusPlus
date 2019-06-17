// ScriptMDIFrame.h: interface for the CScriptMDIFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTMDIFRAME_H__B8924BE1_B074_11D3_99CC_FCDCB93F9371__INCLUDED_)
#define AFX_SCRIPTMDIFRAME_H__B8924BE1_B074_11D3_99CC_FCDCB93F9371__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ScriptView.h"

class CScriptEditor;

class CScriptMDIFrame : public CWindow  
{
  DECLARE_DYNAMIC(CScriptMDIFrame);
  DECLARE_OBSERVER_MAP(CScriptMDIFrame);

public:

  // maximizes the view
  void MaximizeView(CScriptView *pView);
  // restores the view (Reverse to maximize)
  void RestoreView(CScriptView *pView);

  CScriptView *FindView(CString strFileName, BOOL bLocal);
    
  void BackupViews(CString &strBackupDirectory);
  BOOL CloseAllViews();
  BOOL SaveAllViews();
  CScriptView * GetActiveView(){ return m_pActiveView; }
	void DeleteView(CScriptView *pDeleteView);
	CScriptView * CreateView();
	virtual void Draw(CDDrawSurface *pSurface, CRect *pRect);
	virtual void Delete();
	BOOL Create(CRect &rcBound, CWindow *pParent, CScriptEditor *pScriptEditor);
  // constructor & destructor
	CScriptMDIFrame();
	virtual ~CScriptMDIFrame();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // active view, also this is the pointer to the cyclic list of views
  CScriptView *m_pActiveView;
protected:
	void ActiveViewChanged(CScriptView *pView);
	BOOL OnKeyAccelerator(UINT nChar, DWORD dwFlags);
	void DeleteAllViews();

  virtual BOOL OnActivate ();

  virtual void OnSize(CSize size);

  // pointer to our parent (for saving views)
  CScriptEditor *m_pScriptEditor;

  friend class CScriptView;
};

#endif // !defined(AFX_SCRIPTMDIFRAME_H__B8924BE1_B074_11D3_99CC_FCDCB93F9371__INCLUDED_)
