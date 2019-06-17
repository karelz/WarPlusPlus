// OpenRemoteScriptDlg.h: interface for the COpenRemoteScriptDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPENREMOTESCRIPTDLG_H__D5219783_DA55_11D3_A8B8_00105ACA8325__INCLUDED_)
#define AFX_OPENREMOTESCRIPTDLG_H__D5219783_DA55_11D3_A8B8_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class COpenRemoteScriptDlg : public CDialogWindow  
{
  DECLARE_DYNAMIC(COpenRemoteScriptDlg);
  DECLARE_OBSERVER_MAP(COpenRemoteScriptDlg);
public:
	virtual void Delete();
	DWORD DoModal(CPoint pt, CWindow *pParent);
	COpenRemoteScriptDlg();
	virtual ~COpenRemoteScriptDlg();

  CString m_strLocalList;
  CString m_strGlobalList;

  CString m_strFileName;
  BOOL m_bLocalScript;

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif
private:
  void FillScriptList();
  BOOL ParseNextLine(CString &strSource, DWORD &dwPos, CString &strFileName, CString &strSize, CString &strTimeDate);
  void SetToggleGlobalsText();

	CTextButton m_wndCancel;
	CTextButton m_wndOpen;
  CTextButton m_wndToggleGlobals;
  static BOOL m_bShowGlobals;
	CListControl m_wndScriptList;
	CStaticText m_wndScriptListTitle;
  CEditBox m_wndFileName;

  enum{
    IDC_ScriptList = 0x100,
    IDC_ToggleGlobals = 0x101,
  };
protected:
	virtual void InitDialog();

  void OnSelChange();
  void OnSelected();
  void OnOK(int nLocalAdvise = 0);
  void OnToggleGlobals();
};

#endif // !defined(AFX_OPENREMOTESCRIPTDLG_H__D5219783_DA55_11D3_A8B8_00105ACA8325__INCLUDED_)

