// ZMapDownload.h: interface for the CZMapDownload class
//////////////////////////////////////////////////////////////////////

#if !defined(_ZMAPDOWNLOAD__HEADER_INCLUDED_)
#define _ZMAPDOWNLOAD__HEADER_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// CZMapDownload class declaration

class CZMapDownload : public CObserver
{
    DECLARE_DYNAMIC(CZMapDownload)

    DECLARE_OBSERVER_MAP(CZMapDownload)

public:
	CZMapDownload();
	virtual ~CZMapDownload();

    void Create(const CVirtualConnection &VirtualConnection);
    void Delete();

    void OnDownloadRequest();
    void OnVirtualConnectionClosed();

#ifdef _DEBUG
    // check content of object
    virtual void AssertValid() const;
    // dump content of object
    virtual void Dump(CDumpContext &dc) const;
#endif

    enum{ ID_VirtualConnection = 0x0100, };

protected:
    CVirtualConnection m_VirtualConnection;
    CString m_strMapFileName;

#ifdef _DEBUG
    BOOL m_bInitialized;
#endif
};

#endif // !defined(_ZMAPDOWNLOAD__HEADER_INCLUDED_)
