// LogWindow.cpp: implementation of the CLogWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\gameclient.h"
#include "LogWindow.h"

#include "..\GameClientNetwork.h"
#include "Common\ServerClient\VirtualConnectionIDs.h"
#include "Common\ServerClient\LogOutput.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CLogWindow, CCaptionWindow);

BEGIN_OBSERVER_MAP(CLogWindow, CCaptionWindow)
  BEGIN_NOTIFIER(IDC_CLOSE)
    ON_BUTTONCOMMAND(OnClose)
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_CLEAR)
    ON_BUTTONCOMMAND(Clear)
  END_NOTIFIER()

  BEGIN_NOTIFIER(ID_VirtualConnection)
    EVENT(E_PACKETAVAILABLE)
      OnPacketAvailable(); return FALSE;
    EVENT(E_NETWORKERROR)
    EVENT(E_NORMALCLOSE)
    EVENT(E_ABORTIVECLOSE)
      OnNetworkError(); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(ID_Myself)
    EVENT(E_AddBuffer)
      OnAddBuffer((char *)dwParam); return FALSE;
  END_NOTIFIER()
END_OBSERVER_MAP(CLogWindow, CCaptionWindow)

// constructor
CLogWindow::CLogWindow()
{

}

// destructor
CLogWindow::~CLogWindow()
{

}

// debug functions
#ifdef _DEBUG

void CLogWindow::AssertValid() const
{
  CCaptionWindow::AssertValid();
}

void CLogWindow::Dump(CDumpContext &dc) const
{
  CCaptionWindow::Dump(dc);
}

#endif


// creation
void CLogWindow::Create(CScriptEditLayout *pScriptLayout, CCaptionWindowLayout *pCaptionLayout, CWindow *pParent)
{
  // top most
  m_bTopMost = TRUE;

  // create the window
  // set the caption
  CString strCaption;
  strCaption.LoadString(IDS_SCRIPTEDITOR_LOGWINDOW_CAPTION);
  CRect rcBound(50, 50, 450, 250);
  CCaptionWindow::Create(rcBound, strCaption, pCaptionLayout, pParent, TRUE);

  // first create the close button
  {
    CDialogWindowLayout *pLayout = CLayouts::m_pDefaults->GetDialogWindowLayout();

    // create the point and button itself there
    CPoint pt(rcBound.Width() - pLayout->m_dwCloseRightMargin,
      pLayout->m_dwCloseTopMargin);
    m_CloseButton.Create(pt, &(pLayout->m_CloseButtonLayout), this, FALSE);

    // connect it
    m_CloseButton.Connect(this, IDC_CLOSE);

    CString strClear;
    strClear.LoadString(IDS_SCRIPTEDITOR_LOGWINDOW_CLEAR);
    m_ClearButton.Create(CPoint(rcBound.Width() - 110, 2), NULL, strClear, this);
    m_ClearButton.Connect(this, IDC_CLEAR);
  }

  // create the script edit
  CRect rcEdit;
  GetEditRect(&rcEdit);
  m_ScriptEdit.Create(rcEdit, pScriptLayout, this);
  m_ScriptEdit.Connect(this, IDC_SCRIPTEDIT);

  m_ScriptEdit.SetReadOnly(TRUE);

  // create the virtual connection
  ASSERT_VALID(g_pNetworkConnection);
  DWORD dwUserData = VirtualConnection_LogOutput;
  m_VirtualConnection = g_pNetworkConnection->CreateVirtualConnectionEx(PACKET_TYPE1, this, ID_VirtualConnection, g_dwNetworkEventThreadID, &dwUserData, 4);

  Connect(this, ID_Myself);
}

// deletion
void CLogWindow::Delete()
{
  m_CloseButton.Delete();
  m_ScriptEdit.Delete();

  CCaptionWindow::Delete();
}

void CLogWindow::GetEditRect(CRect *pEditRect)
{
  CRect rcBound(GetWindowPosition());

  pEditRect->left = 7;
  pEditRect->top = 26;
  pEditRect->right = rcBound.Width() - 7;
  pEditRect->bottom = rcBound.Height() - 7;
}

void CLogWindow::OnSize(CSize size)
{
  CRect rcEdit;
  GetEditRect(&rcEdit);
  m_ScriptEdit.SetWindowPos(&rcEdit);

  CDialogWindowLayout *pLayout = CLayouts::m_pDefaults->GetDialogWindowLayout();
  CPoint pt(size.cx - pLayout->m_dwCloseRightMargin,
    pLayout->m_dwCloseTopMargin);
  m_CloseButton.SetWindowPosition(&pt);

  pt.x = size.cx - 110;
  pt.y = 2;
  m_ClearButton.SetWindowPosition(&pt);

  CCaptionWindow::OnSize(size);
}

void CLogWindow::Clear()
{
  m_ScriptEdit.DeleteAll();

  // send command to server
  DWORD dwCommand = LogOutput_Command_Clear;
  m_VirtualConnection.SendBlock(&dwCommand, sizeof(dwCommand));
}

void CLogWindow::OnClose()
{
  InlayEvent(E_HIDDEN, 0);
}

void CLogWindow::OnPacketAvailable()
{
  DWORD dwSize = 0;
  char *pBuffer = NULL;

  m_VirtualConnection.ReceiveBlock(pBuffer, dwSize);

  ASSERT(dwSize > 0);
  pBuffer = new char[dwSize + 1];
  VERIFY(m_VirtualConnection.ReceiveBlock(pBuffer, dwSize));
  pBuffer[dwSize] = 0;
  
  InlayEvent(E_AddBuffer, (DWORD)pBuffer, this);
}

void CLogWindow::OnNetworkError()
{
}

void CLogWindow::OnAddBuffer(char *pBuffer)
{
  ASSERT(pBuffer != NULL);
  CString strBuffer = pBuffer;
  m_ScriptEdit.AppendText(strBuffer);
  delete pBuffer;
}