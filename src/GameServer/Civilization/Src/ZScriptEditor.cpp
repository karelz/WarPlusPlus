// ZScriptEditor.cpp: implementation of the CZScriptEditor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZScriptEditor.h"

#include "Common\ServerClient\ScriptEditor.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CZScriptEditor, CObserver);

BEGIN_OBSERVER_MAP(CZScriptEditor, CObserver)
  BEGIN_NOTIFIER(ID_VirtualConnection)
    EVENT(E_PACKETAVAILABLE)
      OnPacketAvailable(); return FALSE;
  END_NOTIFIER()
END_OBSERVER_MAP(CZScriptEditor, CObserver)

// Constructor
CZScriptEditor::CZScriptEditor()
{
  m_pCompiler = NULL;
  m_pCodeManager = NULL;
}

// Destructor
CZScriptEditor::~CZScriptEditor()
{
}

// Debug functions
#ifdef _DEBUG

void CZScriptEditor::AssertValid() const
{
  CObserver::AssertValid();
  ASSERT(m_pCompiler != NULL);
  ASSERT(m_pCodeManager != NULL);
}

void CZScriptEditor::Dump(CDumpContext &dc) const
{
  CObserver::Dump(dc);
}

#endif


// Creations

// Creates the object for existing virtual connection
void CZScriptEditor::Create(CVirtualConnection VirtualConnection, CCompiler *pCompiler, CCodeManager *pCodeManager)
{
  ASSERT_VALID(pCompiler);
  ASSERT_VALID(pCodeManager);

  m_pCompiler = pCompiler;
  m_pCodeManager = pCodeManager;

  m_VirtualConnection = VirtualConnection;
  m_VirtualConnection.SetVirtualConnectionObserver(this, ID_VirtualConnection);
}

// Deletes the object
void CZScriptEditor::Delete()
{
  // clear variables
  m_pCompiler = NULL;
  m_pCodeManager = NULL;

  m_VirtualConnection.DeleteVirtualConnection();

  // delete the error output
  m_ErrorOutput.Delete();
}


// Called every time new packet arrived
void CZScriptEditor::OnPacketAvailable()
{
  DWORD dwCommand, dwSize;

  VERIFY(m_VirtualConnection.BeginReceiveCompoundBlock());

  // first recieve the packet command
  // get the command
  dwSize = sizeof(dwCommand);
  VERIFY(m_VirtualConnection.ReceiveBlock(&dwCommand, dwSize));

  // switch acording to the command
  switch(dwCommand){

  // Get the list of local/global files
  case ScriptEditor_Command_ListFiles:
    {
      // create the response
      SScriptEditor_ListFiles header;
      DWORD dwAnswer = ScriptEditor_Answer_ListFiles;
      CString strLocalResult, strGlobalResult;

      try{
        // read the file list from the code manager
        m_pCodeManager->ListSourceFiles(CCodeManager::USER_SOURCE, strLocalResult);
        m_pCodeManager->ListSourceFiles(CCodeManager::SYSTEM_SOURCE, strGlobalResult);
      }
      catch(CException *e){
        SendError(e);
        break;
      }

      header.m_dwLocalLength = strLocalResult.GetLength() + 1;
      header.m_dwGlobalLength = strGlobalResult.GetLength() + 1;

      // send the type
      VERIFY(m_VirtualConnection.BeginSendCompoundBlock());
      VERIFY(m_VirtualConnection.SendBlock(&dwAnswer, sizeof(dwAnswer)));

      // send the header
      VERIFY(m_VirtualConnection.SendBlock(&header, sizeof(header)));

      // send the result
      VERIFY(m_VirtualConnection.SendBlock((LPCSTR)strLocalResult, strLocalResult.GetLength() + 1));
      VERIFY(m_VirtualConnection.SendBlock((LPCSTR)strGlobalResult, strGlobalResult.GetLength() + 1));

      VERIFY(m_VirtualConnection.EndSendCompoundBlock());
    }
    break;

  // Save the script on the server
  case ScriptEditor_Command_SaveFile:
    {
      // recieve the header
      SScriptEditor_SaveFile header;
      dwSize = sizeof(header);
      VERIFY(m_VirtualConnection.ReceiveBlock(&header, dwSize));

      // recieve the file name
      char *pFileName = new char[header.m_dwFileNameLength];
      VERIFY(m_VirtualConnection.ReceiveBlock(pFileName, header.m_dwFileNameLength));

      // recieve the file itself
      char *pFile = new char[header.m_dwFileLength];
      VERIFY(m_VirtualConnection.ReceiveBlock(pFile, header.m_dwFileLength));

      try{
        // open the code manager upload channel
        CString strFileName(pFileName);
        CCompilerOutput *pOutput;
        pOutput = m_pCodeManager->UploadScriptSourceStart(strFileName);

        if(pOutput == NULL) SendError(NULL);

        // write the file into it
        pOutput->PutChars(pFile, header.m_dwFileLength - 1);
        pOutput->Flush();

        // close the upload channel
        if(!m_pCodeManager->UploadScriptSourceEnd()) SendError(NULL);

        delete [] pFile;
        delete [] pFileName;
      }
      catch(CException *e){
        delete [] pFile;
        delete [] pFileName;
        SendError(e);
        break;
      }

      // prepeare the answer packet
      DWORD dwAnswer = ScriptEditor_Answer_SaveFile_Success;

      // and send it
      VERIFY(m_VirtualConnection.BeginSendCompoundBlock());
      VERIFY(m_VirtualConnection.SendBlock(&dwAnswer, sizeof(dwAnswer)));
      VERIFY(m_VirtualConnection.EndSendCompoundBlock());
    }
    break;

  case ScriptEditor_Command_LoadFile:
    {
      // recieve the header
      SScriptEditor_LoadFile_Command header;
      dwSize = sizeof(header);
      VERIFY(m_VirtualConnection.ReceiveBlock(&header, dwSize));

      // recieve the file name
      char *pFileName = new char[header.m_dwFileNameLength];
      dwSize = header.m_dwFileNameLength;
      VERIFY(m_VirtualConnection.ReceiveBlock(pFileName, dwSize));

      CString strFile, strFileName = pFileName;
	    delete pFileName;
      try{
        CCodeManager::ESourceType eSourceType;
        CCompilerInput *pInput;
        int nChar;

        if(header.m_eFileTypes == ScriptEditor_LoadFile_UserFile){
          eSourceType = CCodeManager::USER_SOURCE;
        }
        else{
          eSourceType = CCodeManager::SYSTEM_SOURCE;
        }

        pInput = m_pCodeManager->DownloadScriptSourceStart(eSourceType, strFileName);

        while((nChar = pInput->GetChar()) != EOF){
          strFile += (char)nChar;
        }

        m_pCodeManager->DownloadScriptEnd();
      }
      catch(CException *e){
        SendError(e);
        break;
      }

      // now send resuts
      SScriptEditor_LoadFile_Answer h;
      h.m_dwFileLength = strFile.GetLength() + 1;
      h.m_dwFileNameLength = strFileName.GetLength() + 1;

      VERIFY(m_VirtualConnection.BeginSendCompoundBlock());

      DWORD dwAnswer = ScriptEditor_Answer_LoadFile;
      VERIFY(m_VirtualConnection.SendBlock(&dwAnswer, sizeof(dwAnswer)));

      VERIFY(m_VirtualConnection.SendBlock(&h, sizeof(h)));

      VERIFY(m_VirtualConnection.SendBlock((LPCSTR)strFileName, strFileName.GetLength() + 1));
      VERIFY(m_VirtualConnection.SendBlock((LPCSTR)strFile, strFile.GetLength() + 1));

      VERIFY(m_VirtualConnection.EndSendCompoundBlock());
    }
    break;

  case ScriptEditor_Command_Compile:
    {
      // recieve the header
      SScriptEditor_Compile_Command header;
      dwSize = sizeof(header);
      VERIFY(m_VirtualConnection.ReceiveBlock(&header, dwSize));

      // recieve the file name
      char *pFileName = new char[header.m_dwFileNameLength];
      dwSize = header.m_dwFileNameLength;
      VERIFY(m_VirtualConnection.ReceiveBlock(pFileName, dwSize));
      CString strFileName = pFileName;
      delete pFileName;

      // now open the compiler input from codemanager
      CCompilerInput *pInput;
      // must be the user source
      CCodeManager::ESourceType eSourceType = CCodeManager::USER_SOURCE;
      try{
        pInput = m_pCodeManager->DownloadScriptSourceStart(eSourceType, strFileName);

        m_pCompiler->Compile(pInput, m_ErrorOutput.Open(), (header.m_dwUpdate != 0)?TRUE:FALSE);
        m_ErrorOutput.Close();

        // close the input
        m_pCodeManager->DownloadScriptEnd();
      }
      catch(CException *e){
        e->Delete();
//        SendError(e);
        break;
      }
    }
    break;

  case ScriptEditor_Command_MultiCompile:
    {
      // no more data will arrive, so don't read anything more

      try{
        CString strSources, strPath;

        // we'll ask the code manager for all modified scripts
        m_pCodeManager->GetChangedScripts(strSources, strPath);

        // and now compile all of them (if there is some)
        if(!strSources.IsEmpty()){
          CCompilerInputMultipleFile input;

          // open the multi input
          if(!input.Open(strPath, strSources, m_pCompiler, NULL)) break;

          // compile it
          m_pCompiler->Compile(&input, m_ErrorOutput.Open(), TRUE);
          m_ErrorOutput.Close();

          // close the input
          input.Close();
        }
      }
      catch(CException *e){
        e->Delete();
        break;
      }
    }
    break;

  case ScriptEditor_Command_GetProfilingInfo:
    {
      // no more data will arrive

      // Get the profiling info from the code manager
      CString strInfo;
      m_pCodeManager->GetCounters ( strInfo );

      VERIFY(m_VirtualConnection.BeginSendCompoundBlock());
      // Send the reply
      DWORD dwAnswer = ScriptEditor_Answer_GetProfilingInfo;
      VERIFY(m_VirtualConnection.SendBlock(&dwAnswer, sizeof(dwAnswer)));

      SScriptEditor_GetProfilingInfo_Answer h;
      h.m_dwLength = strInfo.GetLength () + 1;
      h.m_nLocalCounter = m_pCodeManager->m_pZCivilization->GetInterpret ()->GetLocalProfileCounter ();
      h.m_nGlobalCounter = m_pCodeManager->m_pZCivilization->GetInterpret ()->GetGlobalProfileCounter ();
      VERIFY(m_VirtualConnection.SendBlock(&h, sizeof(h)));

      VERIFY(m_VirtualConnection.SendBlock((LPCSTR)strInfo, strInfo.GetLength() + 1));

      VERIFY(m_VirtualConnection.EndSendCompoundBlock());
    }
    break;

  default:
    TRACE0("Uknown packet recieved.\n");
    ASSERT(FALSE);
    break;
  }

  VERIFY(m_VirtualConnection.EndReceiveCompoundBlock());
}

// Sends an error packet to the client
void CZScriptEditor::SendError(CException *e)
{
  // just delete the exception -> no error param to the client
  e->Delete();

  DWORD dwCommand = ScriptEditor_Command_Error;

  // send the packet with error
  VERIFY(m_VirtualConnection.SendBlock(&dwCommand, sizeof(dwCommand)));
}

// Create the error output
void CZScriptEditor::CreateErrorOutput(CVirtualConnection VirtualConnection)
{
  // just create it
  m_ErrorOutput.Create(VirtualConnection);
}