// DownloadMap.cpp: implementation of the CDownloadMap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "gameclient.h"
#include "DownloadMap.h"

#include "GameClientNetwork.h"
#include "Common\ServerClient\VirtualConnectionIds.h"
#include "Common\ServerClient\MapDownloadStructures.h"

#include "Common\CommonExceptions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CDownloadMap, CDialogWindow)

BEGIN_OBSERVER_MAP(CDownloadMap, CDialogWindow)
  BEGIN_NOTIFIER(ID_VirtualConnection)
    EVENT(E_PACKETAVAILABLE)
      OnPacketAvailable(); return FALSE;
    EVENT(E_NETWORKERROR)
    EVENT(E_NORMALCLOSE)
    EVENT(E_ABORTIVECLOSE)
    EVENT(E_VIRTUALCONNECTIONCLOSED)
      OnNetworkError(); return FALSE;
  END_NOTIFIER()
END_OBSERVER_MAP(CDownloadMap, CDialogWindow)

// constructor
CDownloadMap::CDownloadMap()
{
  m_pDownloadBuffer = NULL;
  m_bConnectionCreated = FALSE;
}

// destructor
CDownloadMap::~CDownloadMap()
{
}

// debug functions
#ifdef _DEBUG

void CDownloadMap::AssertValid() const
{
  CDialogWindow::AssertValid();
}

void CDownloadMap::Dump(CDumpContext &dc) const
{
  CDialogWindow::Dump(dc);
}

#endif


void CDownloadMap::InitDialog()
{
  DWORD dwUserData = VirtualConnection_MapDownload;

  m_pDownloadBuffer = new BYTE[g_pNetworkConnection->GetMaximumMessageSize()];

  // create the interface
  CRect rcBound;
  rcBound.SetRect(15, 35, 285, 55);
  m_wndMapName.Create(rcBound, NULL, " ", this);

  CString strText;
  strText.LoadString(IDS_DOWNLOADMAP_STARTING);
  rcBound.SetRect(15, 55, 285, 75);
  m_wndProgressText.Create(rcBound, NULL, strText, this);

  rcBound.SetRect(15, 75, 285, 95);
  m_wndEstimatedTime.Create(rcBound, NULL, " ", this);

  rcBound.SetRect(100, 95, 200, 120);
  strText.LoadString(IDS_CANCEL);
  m_wndCancel.Create(&rcBound, NULL, strText, this);
  m_wndCancel.Connect(this, CDialogWindow::IDC_CANCEL);

  m_eState = CDownloadMap::State_WaitingForMap;

  // first create the virtual connection
  m_VirtualConnection = g_pNetworkConnection->CreateVirtualConnectionEx(PACKET_TYPE1, this, ID_VirtualConnection, 0,
    &dwUserData, sizeof(dwUserData), PACKETPRIORITY_IDLE);
  m_bConnectionCreated = TRUE;
}

void CDownloadMap::Delete()
{
  if(m_bConnectionCreated){
    m_VirtualConnection.DeleteVirtualConnection();
    m_bConnectionCreated = FALSE;
  }

  m_wndMapName.Delete();
  m_wndProgressText.Delete();
  m_wndEstimatedTime.Delete();
  m_wndCancel.Delete();

  if(m_pDownloadBuffer != NULL){
    delete m_pDownloadBuffer;
    m_pDownloadBuffer = NULL;
  }

  CDialogWindow::Delete();
}

// new packet arrived
void CDownloadMap::OnPacketAvailable()
{
  switch(m_eState){
  case CDownloadMap::State_WaitingForMap:
    // the first packet arrived -> read it
    {
      struct SMapDownload_MapInfo info;
      DWORD dwSize = sizeof(info);
      VERIFY(m_VirtualConnection.BeginReceiveCompoundBlock());
      VERIFY(m_VirtualConnection.ReceiveBlock(&info, dwSize));
      ASSERT(dwSize == sizeof(info));

      // get the map name
      char *pMapName = new char[info.dwMapNameLength];
      dwSize = info.dwMapNameLength;
      VERIFY(m_VirtualConnection.ReceiveBlock(pMapName, dwSize));

      CString strMapName = pMapName;
      delete pMapName;
      DWORD dwMapVersion = info.dwMapVersion;

      VERIFY(m_VirtualConnection.EndReceiveCompoundBlock());

      // compute the map path
      m_strMapPath = g_strStartDirectory + "\\Maps\\" + strMapName;

      CreateDirectory(g_strStartDirectory + "\\Maps", NULL);

      // here we have in the strMapName, the name of the map
      // and in the dwMapVersion, the version
      // so try to find the map
      try
      {
        // open the archive
        CDataArchive archive;
        archive.Create(m_strMapPath, CArchiveFile::modeReadWrite, 0);

        // open the map file
        CArchiveFile MapFile = archive.CreateFile("Map", CArchiveFile::modeReadWrite);

        MapFile.Seek(sizeof(SFileVersionHeader), CFile::begin);
        // read the header
        SMapHeader sHeader;
        MapFile.Read(&sHeader, sizeof(sHeader));

        // test if it's the same version
        if(sHeader.m_dwMapVersion == dwMapVersion){
          // YES we have the right map
          // so just end it all
          EndDialog(CDialogWindow::IDC_OK);

          return;
        }
      }
      catch(CException *e){
        // some error -> no map
        e->Delete();

        // so let's download it
      }

      // no map, or wrong version
      // so delete the old map
      DeleteFile(m_strMapPath);

      // and start the download
      m_eState = CDownloadMap::State_BeforeDownload;

      m_wndMapName.SetText(strMapName);

      // send the packet to the server -> we want to download the map
      struct SMapDownload_DownloadRequest sRequest;
      sRequest.dwDummy = 42;
      VERIFY(m_VirtualConnection.SendBlock(&sRequest, sizeof(sRequest)));

      // and that's all, just wait for the server to answer
    }
    break;
  case CDownloadMap::State_BeforeDownload:
    // here should be packet with the file size
    {
      // so recieve it
      struct SMapDownload_MapSize sMapSize;
      DWORD dwSize = sizeof(sMapSize);

      VERIFY(m_VirtualConnection.ReceiveBlock(&sMapSize, dwSize));

      // copy it
      m_dwMapLength = sMapSize.dwMapFileSize;

      // open the map
      m_fileMap.Open(m_strMapPath, CFile::modeReadWrite | CFile::modeCreate, NULL);

      m_dwDownloadedBytes = 0;

      m_eState = CDownloadMap::State_Downloading;

      m_timeStarted = CTime::GetCurrentTime();

      CString strText;
      strText.Format(IDS_DOWNLOADMAP_PROGRESS, 0, 0);
      m_wndProgressText.SetText(strText);
    }
    break;
  case CDownloadMap::State_Downloading:
    // some packet arrived
    {
      // determine the size of the packet
      DWORD dwPacketSize = 0;
      m_VirtualConnection.ReceiveBlock(NULL, dwPacketSize);

      // so read the packet
      VERIFY(m_VirtualConnection.ReceiveBlock(m_pDownloadBuffer, dwPacketSize));

      // write it to file
      m_fileMap.Write(m_pDownloadBuffer, dwPacketSize);

      m_dwDownloadedBytes += dwPacketSize;

      if(m_dwDownloadedBytes >= m_dwMapLength){
        // Yes we've got it all
        // so close the file and quit
        m_fileMap.Close();

        EndDialog(CDialogWindow::IDC_OK);
        return;
      }

      DWORD dwRate;
      CTime timeCurrent = CTime::GetCurrentTime();
      CTimeSpan timespan = timeCurrent - m_timeStarted;
      if(timespan.GetTotalSeconds() == 0){
        dwRate = m_dwDownloadedBytes / 1024;
      }
      else{
        dwRate = (m_dwDownloadedBytes / timespan.GetTotalSeconds()) / 1024;
      }

      DWORD dwEstTime;
      if(m_dwDownloadedBytes == 0)
        dwEstTime = 0;
      else
        dwEstTime= (m_dwMapLength - m_dwDownloadedBytes) * timespan.GetTotalSeconds() / m_dwDownloadedBytes;

      // just another packet, continue
      CString strText;
      strText.Format(IDS_DOWNLOADMAP_PROGRESS, m_dwDownloadedBytes * 100 / m_dwMapLength, dwRate);
      m_wndProgressText.SetText(strText);

      strText.Format(IDS_DOWNLOADMAP_ESTIMATED, dwEstTime / 60, dwEstTime % 60);
      m_wndEstimatedTime.SetText(strText);
    }
    break;
  default:
    ASSERT(FALSE);
  }
}

// oops some network error
void CDownloadMap::OnNetworkError()
{
  // here we must get some error string and throw the exception
  CString strError;
  strError.LoadString(IDS_DOWNLOADMAP_NETWORKERROR);

  throw new CStringException(strError);
}
