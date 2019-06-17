// ConnectDlg.cpp: implementation of the CConnectDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\gameclient.h"
#include "ConnectDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CConnectDlg, CDialogWindow)

BEGIN_OBSERVER_MAP(CConnectDlg, CDialogWindow)
  THECOMMAND(IDC_OK, OnOK)
END_OBSERVER_MAP(CConnectDlg, CDialogWindow)

CConnectDlg::CConnectDlg()
{
  m_strServerName = "";
  m_dwServerPort = 0;
  m_strLogin = "";
  m_strPassword = "";
}

CConnectDlg::~CConnectDlg()
{

}

#ifdef _DEBUG

void CConnectDlg::AssertValid() const
{
  CDialogWindow::AssertValid();
}

void CConnectDlg::Dump(CDumpContext &dc) const
{
  CDialogWindow::Dump(dc);
}

#endif

CRect CConnectDlg::GetDefaultPosition()
{
  CSize sz(350, 230);

  return GetCenterPosition(sz);
}

CString CConnectDlg::GetDefaultCaption()
{
  CString str;
  str.LoadString(IDS_CONNECT_CAPTION);
  return str;
}

void CConnectDlg::InitDialog()
{
  CRect rc;
  CString str;

  CDialogWindow::InitDialog();
  
  // the login
  rc.SetRect(15, 95, 200, 110);
  str.LoadString(IDS_CONNECT_LOGIN);
  m_LoginTitle.Create(rc, NULL, str, this);
  m_LoginTitle.SetAlign(CStaticText::AlignLeft);
  rc.SetRect(15, 110, 200, 1130);
  m_Login.Create(rc, NULL, this);

  // the password
  rc.SetRect(15, 135, 200, 150);
  str.LoadString(IDS_CONNECT_PASSWORD);
  m_PasswordTitle.Create(rc, NULL, str, this);
  m_PasswordTitle.SetAlign(CStaticText::AlignLeft);
  rc.SetRect(15, 150, 200, 170);
  m_Password.Create(rc, NULL, this);
  m_Password.SetPassword();

  // the ok button
  rc.SetRect(145, 192, 235, 215);
  str.LoadString(IDS_OK);
  m_OK.Create(&rc, NULL, str, this);
  m_OK.Connect(this, IDC_OK);
  m_OK.SetKeyAcc(VK_RETURN, 0);

  // the cancel button
  rc.SetRect(245, 192, 335, 215);
  str.LoadString(IDS_CANCEL);
  m_Cancel.Create(&rc, NULL, str, this);
  m_Cancel.Connect(this, IDC_CANCEL);


  // the server name (address)
  rc.SetRect(15, 35, 250, 50);
  str.LoadString(IDS_CONNECT_SERVERNAME);
  m_ServerNameTitle.Create(rc, NULL, str, this);
  m_ServerNameTitle.SetAlign(CStaticText::AlignLeft);
  rc.SetRect(15, 50, 250, 75);
  m_ServerName.Create(rc, NULL, this);

  // the server port
  rc.SetRect(260, 35, 335, 50);
  str.LoadString(IDS_CONNECT_SERVERPORT);
  m_ServerPortTitle.Create(rc, NULL, str, this);
  m_ServerPortTitle.SetAlign(CStaticText::AlignLeft);
  rc.SetRect(260, 50, 335, 75);
  m_ServerPort.Create(rc, NULL, this);


  m_ServerName.SetText(m_strServerName);
  str.Format("%u", m_dwServerPort);
  m_ServerPort.SetText(str);
  m_Login.SetText(m_strLogin);
  m_Password.SetText(m_strPassword);

  if(!m_strLogin.IsEmpty()) m_Password.SetFocus();
}

void CConnectDlg::OnOK()
{
  CString str;

  m_strServerName = m_ServerName.GetText();
  str = m_ServerPort.GetText();
  m_dwServerPort = (DWORD)atoi(str);
  if(m_dwServerPort == 0){
    CString str2;
    str.LoadString(IDS_CONNECT_BADPORT);
    str2.LoadString(IDR_MAINFRAME);
    CMessageBox::OK(str, str2, this);
    m_ServerPort.SetFocus();
    return;
  }
  m_strLogin = m_Login.GetText();
  m_strPassword = m_Password.GetText();

  EndDialog(IDC_OK);
}
