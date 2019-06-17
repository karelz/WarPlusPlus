// ErrorOutput.cpp: implementation of the CErrorOutput class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ErrorOutput.h"

#include "Common\ServerClient\VirtualConnectionIDs.h"

#include "..\GameClientNetwork.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CErrorOutput, CFrameWindow);

BEGIN_OBSERVER_MAP(CErrorOutput, CFrameWindow)
  BEGIN_NOTIFIER(ID_VirtualConnection)
    EVENT(E_PACKETAVAILABLE)
      OnPacketAvailable(); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(ID_Edit)
    EVENT(CErrorOutputEdit::E_LineSelected)
      OnLineSelected(); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(ID_Myself)
    EVENT(E_AddBuffer)
      OnAddBuffer((char *)dwParam); return FALSE;
  END_NOTIFIER()
END_OBSERVER_MAP(CErrorOutput, CFrameWindow)

// Constructor
CErrorOutput::CErrorOutput()
{
  m_dwLastErrorLine = (DWORD)-1;
  m_dwErrorFileLine = 0;
}

// Destructor
CErrorOutput::~CErrorOutput()
{

}


// Debug functions
#ifdef _DEBUG

void CErrorOutput::AssertValid() const
{
  CFrameWindow::AssertValid();
}

void CErrorOutput::Dump(CDumpContext &dc) const
{
  CFrameWindow::Dump(dc);
}

#endif


// Creation

// Creates the window
void CErrorOutput::Create(CScriptEditLayout *pScriptEditLayout, CFrameWindowLayout *pFrameLayout, CRect rcWindow,
                          CWindow *pParent)
{
  ASSERT_VALID(pScriptEditLayout);
  ASSERT_VALID(pFrameLayout);
  ASSERT_VALID(pParent);

  // create us
  CFrameWindow::Create(rcWindow, pFrameLayout, pParent, FALSE);

  CRect rcScript;
  rcScript.SetRect(5, 5, rcWindow.Width() - 5, rcWindow.Height() - 5);


  // create the script edit window
  m_ScriptEdit.Create(rcScript, pScriptEditLayout, this);
  m_ScriptEdit.SetReadOnly();
  m_ScriptEdit.Connect(this, ID_Edit);

  // create the virtual connection
  {
    DWORD dwUserData = VirtualConnection_ScriptErrorOutput;
    m_VirtualConnection = g_pNetworkConnection->CreateVirtualConnectionEx(PACKET_GUARANTEED_DELIVERY | PACKET_GUARANTEED_ORDER,
      this, ID_VirtualConnection, g_dwNetworkEventThreadID, &dwUserData, sizeof(dwUserData));
  }

  // connect us to us
  Connect(this, ID_Myself);
}

// Deletes the window
void CErrorOutput::Delete()
{
  // delete the virtual connection
  if(g_pNetworkConnection != NULL)
    g_pNetworkConnection->DeleteVirtualConnection(m_VirtualConnection);

  // delete the script edit
  m_ScriptEdit.Delete();

  // delete us
  CFrameWindow::Delete();
}


// Adds text to the output
void CErrorOutput::AddText(CString &strText)
{
  ASSERT_VALID(this);

  // just add it to the script edit
  m_ScriptEdit.AppendText(strText);

  m_ScriptEdit.ScrollToPosition(m_ScriptEdit.GetLinesCount() - 1, 0);
}


// Reactions on network events

// New packet arrived
void CErrorOutput::OnPacketAvailable()
{
  // just read the whole packet
  DWORD dwSize;
  char *pBuffer = NULL;

  // determine the size of the packet
  dwSize = 0;
  m_VirtualConnection.ReceiveBlock(pBuffer, dwSize);

  // allocate buffer
  pBuffer = new char[dwSize + 1];

  // get the buffer
  m_VirtualConnection.ReceiveBlock(pBuffer, dwSize);
  pBuffer[dwSize] = 0;

  // inlay event to us to add the buffer to the window
  InlayEvent(E_AddBuffer, (DWORD)pBuffer, this);
}



// Reactions on our events

// Add buffer
void CErrorOutput::OnAddBuffer(char *pBuffer)
{
  ASSERT(pBuffer != NULL);

  CString str(pBuffer);

  // delete the buffer
  delete pBuffer;

  AddText(str);
}

void CErrorOutput::Clear()
{
  m_ScriptEdit.SetSelection(0, 0, 0, 0);
  m_ScriptEdit.DeleteAll();

  m_dwLastErrorLine = (DWORD) -1;
}

void CErrorOutput::OnLineSelected()
{
  // get current line
  DWORD dwCurrentLine = m_ScriptEdit.GetCaretLine();

  // is it the error line
  if(ParseErrorLine(dwCurrentLine))
    InlayEvent(E_SelectErrorLine, 0);
}

BOOL CErrorOutput::ParseErrorLine(DWORD dwLine)
{
  // get the line text
  CString strLine;
  strLine = m_ScriptEdit.GetLine(dwLine);

  // find the first '(' - before is the file name
  int nLeftBracketPos = strLine.Find('(');
  if(nLeftBracketPos == -1) return FALSE;
  CString strFileName = strLine.Left(nLeftBracketPos);

  // then find the right bracket pos
  int nRightBracketPos = strLine.Find(')', nLeftBracketPos);
  if(nRightBracketPos == -1) return FALSE;
  CString strLineNumber = strLine.Mid(nLeftBracketPos + 1, nRightBracketPos - nLeftBracketPos - 1);

  // also, there must be some double dot after the right bracket
  int nDoubleDotPos = strLine.Find(':', nRightBracketPos);
  if(nDoubleDotPos == -1) return FALSE;

  // now parse the line number
  DWORD dwFileLine = atoi(strLineNumber);
  if(dwFileLine == 0) return FALSE;

  // All was successfull -> copy results
  m_strErrorFileName = strFileName;
  m_dwErrorFileLine = dwFileLine - 1;
  m_dwLastErrorLine = dwLine;

  // select the error line
  m_ScriptEdit.SetSelectionLine(dwLine, dwLine);
  m_ScriptEdit.UpdateRect();

  return TRUE;
}

void CErrorOutput::NextError()
{
  DWORD dwLine;

  if(m_dwLastErrorLine == (DWORD)-1)
    dwLine = 0;
  else
    dwLine = m_dwLastErrorLine + 1;

  // find the next error
  while(dwLine < m_ScriptEdit.GetLinesCount()){
    // parse the next line
    if(ParseErrorLine(dwLine)){
      m_ScriptEdit.ScrollToPosition(dwLine, 0);
      InlayEvent(E_SelectErrorLine, 0);
      return;
    }

    dwLine++;
  }

  // no error line found
  m_ScriptEdit.SetSelection(0, 0, 0, 0);
  m_ScriptEdit.UpdateRect();
}

void CErrorOutput::PrevError()
{
  DWORD dwLine;

  if(m_dwLastErrorLine == (DWORD)-1)
    return;
  else{
    if(m_dwLastErrorLine == 0) return;
    dwLine = m_dwLastErrorLine - 1;
  }

  // find the next error
  while((int)dwLine >= 0){
    // parse the next line
    if(ParseErrorLine(dwLine)){
      m_ScriptEdit.ScrollToPosition(dwLine, 0);
      InlayEvent(E_SelectErrorLine, 0);
      return;
    }

    (int)dwLine--;
  }

  // no error line found
  m_ScriptEdit.SetSelection(0, 0, 0, 0);
  m_ScriptEdit.UpdateRect();
}