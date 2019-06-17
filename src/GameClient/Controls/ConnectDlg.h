// ConnectDlg.h: interface for the CConnectDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONNECTDLG_H__40EEB9D7_E61A_11D3_8444_004F4E0004AA__INCLUDED_)
#define AFX_CONNECTDLG_H__40EEB9D7_E61A_11D3_8444_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CConnectDlg : public CDialogWindow  
{
  DECLARE_DYNAMIC(CConnectDlg);
  DECLARE_OBSERVER_MAP(CConnectDlg);

public:
	CConnectDlg();
	virtual ~CConnectDlg();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif
protected:
	void OnOK();
	virtual void InitDialog();
	virtual CString GetDefaultCaption();
	virtual CRect GetDefaultPosition();

private:
  CStaticText m_ServerNameTitle;
  CStaticText m_ServerPortTitle;
  CStaticText m_LoginTitle;
  CStaticText m_PasswordTitle;

  CEditBox m_ServerName;
  CEditBox m_ServerPort;
  CEditBox m_Login;
  CEditBox m_Password;

  CTextButton m_OK;
  CTextButton m_Cancel;

public:
  CString m_strServerName;
  DWORD m_dwServerPort;
  CString m_strLogin;
  CString m_strPassword;
};

#endif // !defined(AFX_CONNECTDLG_H__40EEB9D7_E61A_11D3_8444_004F4E0004AA__INCLUDED_)
