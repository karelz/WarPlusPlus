// OpenLocalScriptDlg.h: interface for the COpenLocalScriptDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPENLOCALSCRIPTDLG_H__559D7EC3_82B3_11D4_B0FA_004F49068BD6__INCLUDED_)
#define AFX_OPENLOCALSCRIPTDLG_H__559D7EC3_82B3_11D4_B0FA_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class COpenLocalScriptDlg : public CDialogWindow  
{
  DECLARE_OBSERVER_MAP(COpenLocalScriptDlg);
  DECLARE_DYNAMIC(COpenLocalScriptDlg);
public:
	COpenLocalScriptDlg();
	virtual ~COpenLocalScriptDlg();

  virtual void InitDialog();
  virtual void Delete();

  CString m_strName;

protected:
  CRect GetDefaultPosition();
  CString GetDefaultCaption();

  void OnOK();

private:
  CStaticText m_wndNameStatic;
  CEditBox m_wndName;

  CTextButton m_wndOK;
  CTextButton m_wndCancel;
};

#endif // !defined(AFX_OPENLOCALSCRIPTDLG_H__559D7EC3_82B3_11D4_B0FA_004F49068BD6__INCLUDED_)
