// NetworkException.cpp: implementation of the CNetworkException class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetworkException.h"

IMPLEMENT_DYNAMIC(CNetworkException, CException)

// constructor
CNetworkException::CNetworkException(ENetworkException eError, int nParam1, LPCTSTR pSourceName, DWORD dwLineNumber)
{
    m_eError = eError;
    m_nParam1 = nParam1;
    if (pSourceName != NULL)
    {
        m_strSourceName = pSourceName;
    }
    else
    {
        m_strSourceName = "";
    }
    m_dwLineNumber = dwLineNumber; 
}


// returns error message
BOOL CNetworkException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT)
{
    ASSERT(lpszError != NULL);

    PTCHAR p;
    switch (m_eError)
    {
    case eUnknownError:
        p = _T("Unknown error");
        break;
    case eUnknownHostError:
        p = _T("Unknown hostname");
        break;
    case eDNSError:
        p = _T("DNS error");
        break;
    case eTCPSendError:
        p = _T("TCP send failed");
        break;
    case eTCPReceiveError:
        p = _T("TCP receive failed");
        break;
    case eCreateSocketError:
        p = _T("Unable to create socket");
        break;
    case eConnectError:
        p = _T("Unable to connect to server");
        break;
    case eGetPortError:
        p = _T("Unable to get port number");
        break;
    case eBindError:
        p = _T("Unable to bind socket");
        break;
    case eListenError:
        p = _T("Unable to listen for incoming connections");
        break;
    case eWinsockStartupError:
        p = _T("Unable to initialize Winsock DLL");
        break;
    case eWinsockCleanupError:
        p = _T("Unable to cleanup Winsock DLL");
        break;
    case eCreateVirtualConnectionError:
        p = _T("Cannot create new virtual connection");
        break;
    case eBadChallenge:
        p = _T("Error in initial client-server communication (uncompatible network versions?)");
        break;
    case eServerError:
        p = _T("Internal server error during login procedure");
        break;
    case eLoginFailed:
        p = _T("Unknown login error");
        break;
    case eLoginBadPassword:
        p = _T("Bad password");
        break;
    case eLoginUnknownUser:
        p = _T("Unknown username");
        break;
    case eLoginAlreadyLoggedOn:
        p = _T("User is already logged on");
        break;
    case eLoginGameNotRunning:
        p = _T("No game is running on the server");
        break;
    case eLoginUserDisabled:
        p = _T("User account is disabled");
        break;

    default:
        TRACE_NETWORK("CNetworkException::GetErrorMessage() - undefined network exception\n");
        ASSERT(FALSE);
        p = _T("Undefined(!) network exception");
    }

    if (m_dwLineNumber > 0)
    {
        // format the exception string
        CString buf;
        if (m_nParam1 != 0)
        {
            // format incl. parameter
            buf.Format("%s, parameter %i (\"%s\", line %i)", p, m_nParam1, m_strSourceName, m_dwLineNumber);
        }
        else
        {
            // parameter is not relevant
            buf.Format("%s (\"%s\", line %i)", p, m_strSourceName, m_dwLineNumber);
        }
        
        // copy at most (nMaxError - 1) chars
        _tcsncpy(lpszError, buf, nMaxError - 1);
    }
    else
    {
        // copy one character less than the length of buffer
        _tcsncpy(lpszError, p, nMaxError - 1);
    }

    // add null to the end
    lpszError[nMaxError - 1] = '\0';
    return TRUE;
}
