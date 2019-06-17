// SaveRemoteScriptDlg.h: interface for the CSaveRemoteScriptDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SAVEREMOTESCRIPTDLG_H__99426993_04D3_11D4_8485_004F4E0004AA__INCLUDED_)
#define AFX_SAVEREMOTESCRIPTDLG_H__99426993_04D3_11D4_8485_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSaveRemoteScriptDlg : public CDialogWindow  
{
  DECLARE_DYNAMIC(CSaveRemoteScriptDlg);
  DECLARE_OBSERVER_MAP(CSaveRemoteScriptDlg);

public:
  // Constructor & destructor
	CSaveRemoteScriptDlg();
	virtual ~CSaveRemoteScriptDlg();

// Debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // delete the object
	virtual void Delete();

  // Do modal for this dialog
	DWORD DoModal(CPoint pt, CWindow *pParent);

  // the input file list
  CString m_strLocalList;

  // returned file name to save
  CString m_strFileName;

private:
  // Fills the list of scripts
  void FillScriptList();
  // parses one line from the string given by server
  BOOL ParseNextLine(CString &strSource, DWORD &dwPos, CString &strFileName, CString &strSize, CString &strTimeDate);

  // the OK & Cancel buttons
	CTextButton m_wndCancel;
	CTextButton m_wndOpen;
  // the list of scripts
	CListControl m_wndScriptList;
  // the title
	CStaticText m_wndScriptListTitle;
  // the file name editbox
  CEditBox m_wndFileName;

  enum{
    IDC_ScriptList = 0x100, // the script list
  };

protected:
  // called after the dialog window is created
	virtual void InitDialog();

  // called when the OK button is pressed
  virtual void OnOK();

  // called when the selection of the script changes
  void OnSelChange();
  void OnSelected();
};

#endif // !defined(AFX_SAVEREMOTESCRIPTDLG_H__99426993_04D3_11D4_8485_004F4E0004AA__INCLUDED_)

