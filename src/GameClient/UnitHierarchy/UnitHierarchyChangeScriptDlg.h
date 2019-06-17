// UnitHierarchyChangeScriptDlg.h: interface for the CUnitHierarchyChangeScriptDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNITHIERARCHYCHANGESCRIPTDLG_H__760A9623_80CD_11D4_B0FA_004F49068BD6__INCLUDED_)
#define AFX_UNITHIERARCHYCHANGESCRIPTDLG_H__760A9623_80CD_11D4_B0FA_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUnitHierarchyChangeScriptDlg : public CDialogWindow  
{
  DECLARE_OBSERVER_MAP(CUnitHierarchyChangeScriptDlg);
  DECLARE_DYNAMIC(CUnitHierarchyChangeScriptDlg);
public:
	CUnitHierarchyChangeScriptDlg();
	virtual ~CUnitHierarchyChangeScriptDlg();

  virtual void InitDialog();
  virtual void Delete();

  CString m_strScriptName;

protected:
  CRect GetDefaultPosition();
  CString GetDefaultCaption();

  void OnOK();

private:
  CStaticText m_wndScriptNameStatic;
  CEditBox m_wndScriptName;

  CTextButton m_wndOK;
  CTextButton m_wndCancel;
};

#endif // !defined(AFX_UNITHIERARCHYCHANGESCRIPTDLG_H__760A9623_80CD_11D4_B0FA_004F49068BD6__INCLUDED_)
