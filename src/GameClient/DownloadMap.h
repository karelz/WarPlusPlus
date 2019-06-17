// DownloadMap.h: interface for the CDownloadMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOWNLOADMAP_H__AF2FA033_1E0E_11D4_84C1_004F4E0004AA__INCLUDED_)
#define AFX_DOWNLOADMAP_H__AF2FA033_1E0E_11D4_84C1_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDownloadMap : public CDialogWindow
{
  DECLARE_DYNAMIC(CDownloadMap);
  DECLARE_OBSERVER_MAP(CDownloadMap);

public:
  // constructor & destructor
	CDownloadMap();
	virtual ~CDownloadMap();

  // debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  virtual void Delete();
  CString GetMapPath(){ return m_strMapPath; }

protected:
  void OnPacketAvailable();
  void OnNetworkError();

  virtual void InitDialog();
  virtual CRect GetDefaultPosition(){ CSize sz(300, 130); return GetCenterPosition(sz); }
  virtual CString GetDefaultCaption(){ CString strCaption; strCaption.LoadString(IDS_DOWNLOADMAP_CAPTION); return strCaption; }

private:
  // the virtual connection
  CVirtualConnection m_VirtualConnection;
  BOOL m_bConnectionCreated;

  // path to the map
  CString m_strMapPath;

  // downloading objects
  DWORD m_dwMapLength;
  DWORD m_dwDownloadedBytes;
  CFile m_fileMap;

  CStaticText m_wndMapName;
  CStaticText m_wndProgressText;
  CStaticText m_wndEstimatedTime;

  CTextButton m_wndCancel;

  CTime m_timeStarted;

  typedef enum{
    State_WaitingForMap = 1,
    State_BeforeDownload = 2,
    State_Downloading = 3,
  } EDownloadState;

  EDownloadState m_eState;

  BYTE *m_pDownloadBuffer;

  enum{ ID_VirtualConnection = 0x0100, };
};

#endif // !defined(AFX_DOWNLOADMAP_H__AF2FA033_1E0E_11D4_84C1_004F4E0004AA__INCLUDED_)
