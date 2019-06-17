// ZMapDownload.cpp: implementation of the CZMapDownload class
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZMapDownload.h"
#include "Common\ServerClient\MapDownloadStructures.h"

#include "GameServer\GameServer\SMap.h"

IMPLEMENT_DYNAMIC(CZMapDownload, CObserver)

//////////////////////////////////////////////////////////////////////
// Observer map
//////////////////////////////////////////////////////////////////////

BEGIN_OBSERVER_MAP(CZMapDownload, CObserver)

    BEGIN_NOTIFIER(ID_VirtualConnection)

        EVENT(E_PACKETAVAILABLE)
            OnDownloadRequest();
            return TRUE;

        EVENT(E_VIRTUALCONNECTIONCLOSED)
            OnVirtualConnectionClosed();
            return TRUE;

        EVENT(E_NETWORKERROR)
        EVENT(E_NORMALCLOSE)
        EVENT(E_ABORTIVECLOSE)
        EVENT(E_MEMORYERROR)
            {
                TRACE0("Warning - object was not deleted correctly\n");
            }
            return TRUE;

        default:
            TRACE3("Unexpected event (NotifierID %x, Event %i, Param %i)\n", dwNotID, dwEvent, dwParam);

    END_NOTIFIER()

    default:
        TRACE3("Unexpected event from unknown notifier (NotifierID %x, Event %i, Param %i)\n", dwNotID, dwEvent, dwParam);

END_OBSERVER_MAP(CZMapDownload, CObserver)


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CZMapDownload::CZMapDownload()
{
#ifdef _DEBUG
    m_bInitialized = FALSE;
#endif //_DEBUG

}

CZMapDownload::~CZMapDownload()
{

}


//////////////////////////////////////////////////////////////////////
// Create/Delete
//////////////////////////////////////////////////////////////////////

void CZMapDownload::Create(const CVirtualConnection &VirtualConnection)
{
#ifdef _DEBUG
    // should not be called twice
    ASSERT(!m_bInitialized);
    m_bInitialized = TRUE;
#endif //_DEBUG

    // copy the virtual connection to member variable
    m_VirtualConnection = VirtualConnection;
    m_VirtualConnection.SetVirtualConnectionObserver(this, ID_VirtualConnection);
    
    try{
      // prepare initial structure
      SMapDownload_MapInfo sInitStructure;
      
      // get map version from server
      sInitStructure.dwMapVersion = g_cMap.GetMapVersion();
      
      // get map filename from server    
      m_strMapFileName = g_cMap.GetMapArchiveName();
      
      // get map name from filename (discard the path)
      CString strMapName;
      {
        int nIndex = m_strMapFileName.ReverseFind('\\');
        if (nIndex == -1)
        {
          // copy it as a whole (the backslash was not found)
          strMapName = m_strMapFileName;
        }
        else
        {
          // copy only the part behind the final backslash (with index nIndex)
          strMapName = (LPCTSTR) m_strMapFileName + nIndex + 1;
        }
      }
      
      // get length of the map name
      sInitStructure.dwMapNameLength = strMapName.GetLength() + 1;
    
      VERIFY(VirtualConnection.BeginSendCompoundBlock(TRUE));
      // send init structure
      VERIFY(VirtualConnection.SendBlock(&sInitStructure, sizeof(sInitStructure), TRUE));
      // send string
      VERIFY(VirtualConnection.SendBlock((LPCTSTR) strMapName, sInitStructure.dwMapNameLength, TRUE));

      VERIFY(VirtualConnection.EndSendCompoundBlock());
      
      // that's all for now...
      // if client does not have this map, he will ask in the packet, which we'll get
      // in the reaction for network event E_PACKETAVAILABLE
    }
    catch(CException *e){
      char txt[1024];
      e->GetErrorMessage(txt, 1024);
      TRACE1("Error sending information about map to the client:\n%s\n", txt);
      e->Delete();
      m_VirtualConnection.DeleteVirtualConnection();
    }
}


void CZMapDownload::Delete()
{
    // clean the map filename
    m_strMapFileName.Empty();

#ifdef _DEBUG
    // consider this object as not initialized
    m_bInitialized = FALSE;
#endif //_DEBUG
}


//////////////////////////////////////////////////////////////////////
// Download request
//////////////////////////////////////////////////////////////////////

void CZMapDownload::OnDownloadRequest()
{
    ASSERT_VALID(this);
    
    // read the dummy request from the network
    {
        // prepare dummy structure
        SMapDownload_DownloadRequest sTemp;
        DWORD dwTempSize = sizeof(sTemp);
        VERIFY(m_VirtualConnection.ReceiveBlock(&sTemp, dwTempSize));
        ASSERT(dwTempSize == sizeof(sTemp));
        // and do NOTHING with it... it's dummy!  :o)
    }
    
    // "ideal" block size
    const DWORD dwBlockSize = m_VirtualConnection.GetMaximumMessageSize();

    // prepare buffer for copying...
    BYTE *pBuffer = new BYTE[dwBlockSize];

    try{
      // open the map file (this can throw a CFileException)
      CFile fileMap(m_strMapFileName, CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary);

      // get file size
      DWORD dwFileSize = fileMap.GetLength();
      
      // send the file size first
      {
        SMapDownload_MapSize sTemp;
        sTemp.dwMapFileSize = dwFileSize;
        m_VirtualConnection.SendBlock(&sTemp, sizeof(sTemp), TRUE);
      }
      
      // copy the whole file...  wheeee... that's almost a quazillion bytes!!!
      {
        DWORD dwActualSize;
        DWORD dwTotalSize = 0;
        while ((dwActualSize = fileMap.Read(pBuffer, dwBlockSize)) > 0)
        {
          dwTotalSize += dwActualSize;
          VERIFY(m_VirtualConnection.SendBlock(pBuffer, dwActualSize, TRUE));
        }
        
        ASSERT(dwTotalSize == dwFileSize);
      }

      // CFile will close itself in its destructor... no problem  :o)
      // that's all
    }
    catch(CException *e){
      // Ooops some error
      // we'll just trace it
      char txt[1024];
      e->GetErrorMessage(txt, 1024);
      TRACE1("Download map error: %s", txt);

      // and close the virtual connection (this is an error message to the client)
      m_VirtualConnection.DeleteVirtualConnection();

      e->Delete();
    }

    // delete buffer for copying
    delete pBuffer;
}



//////////////////////////////////////////////////////////////////////
// Virtual connection close
//////////////////////////////////////////////////////////////////////

void CZMapDownload::OnVirtualConnectionClosed()
{
    ASSERT_VALID(this);

    m_VirtualConnection.DeleteVirtualConnection();

    // clean the map filename
    m_strMapFileName.Empty();
}


//////////////////////////////////////////////////////////////////////
// Debug Methods
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// check content of object
void CZMapDownload::AssertValid() const
{
    CObject::AssertValid();
    ASSERT(m_bInitialized);
    ASSERT_VALID(&m_VirtualConnection);
}

// dump content of object
void CZMapDownload::Dump(CDumpContext &dc) const
{
    dc << "CZMapDownload object ";
    if (m_bInitialized)
    {
        dc << "(initialized) :\n";

        // TODO: add some dumping here...
    }
    else
    {
        dc << "(uninitialized)\n";
    }

    CObject::Dump(dc);
}

#endif
