// NetworkException.h: interface for the CNetworkException class
//////////////////////////////////////////////////////////////////////

#if !defined(_NETWORKEXCEPTION__HEADER_INCLUDED_)
#define _NETWORKEXCEPTION__HEADER_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////

class CNetworkException : public CException  
{
    DECLARE_DYNAMIC(CNetworkException)

public:
    enum tagENetworkException { 
        eUnknownError,  // should never be used!  :-)

        eUnknownHostError,  // given server name is invalid
        eDNSError,  // other error in operation with DNS server (like gethostbyname(),...)
        eTCPSendError,  // error during sending data through TCP (send())
        eTCPReceiveError,  // error during receiving data throug TCP (recv())
        eCreateSocketError,  //  error while creating new socket (socket())
        eConnectError,  // error during connecting to server (connect())
        eGetPortError,  // error during getting info about bound port (getsockname())
        eBindError,  // error during binding socket to port (bind())
        eListenError,  // cannot make the socket listen for connections (listen())
        eWinsockStartupError,  // cannot initialize Winsock DLL (WSAStartup())
        eWinsockCleanupError,  // cannot cleanup Winsock DLL (WSACleanup())
        eCreateVirtualConnectionError,  // error during creating new virtual connection operation
        eBadChallenge,  // different versions of network library on client and server
        eServerError,  // internal server error during login
        eLoginFailed,  // unknown login error
        eLoginBadPassword,  // bad login password
        eLoginUnknownUser,  // unknown user
        eLoginAlreadyLoggedOn,  // user is already logged on
        eLoginGameNotRunning,  // game is not running on the server
        eLoginUserDisabled,  // user account is disabled
    };
    typedef tagENetworkException ENetworkException;

    CNetworkException(ENetworkException eError = eUnknownError, int nParam1 = 0, LPCTSTR pSourceName = NULL, DWORD dwLineNumber = 0);
    virtual ~CNetworkException() { }
    virtual void SetError(ENetworkException eError) { m_eError = eError; }
    virtual ENetworkException GetError() const { return m_eError; }
    virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext = NULL);

protected:
    ENetworkException m_eError;
    int m_nParam1;
    CString m_strSourceName;
    DWORD m_dwLineNumber;
};


#endif // !defined(_NETWORKEXCEPTION__HEADER_INCLUDED_)
