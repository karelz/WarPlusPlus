
#if !defined(_NETWORKEVENTS__HEADER_INCLUDED_)
#define _NETWORKEVENTS__HEADER_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////////
// set of events that can be sent from the "network project"
//////////////////////////////////////////////////////////////////////////////

// global server notifier ID
enum tagENetworkNotifiers  
{
    ID_NetworkServer = 0x0FE000000
};

enum tagENetworkEvents {
    E_NOEVENT,  // for debugging, should not be sent

    // following events are sent by CNetworkConnection object
    E_NETWORKERROR,  // unrecoverable network error
    E_NORMALCLOSE,  // the peer has sent normal close notification
    E_ABORTIVECLOSE,  // the peer has sent abortive close notification
    E_NETWORKCONGESTION,  // sending of packets is delayed due to the network congestion

    E_WAITERROR,  // WaitFor??Object() failed in one of the threads
    E_MEMORYERROR,  // memory allocation error
    E_NETWORKEVENTSELECTERROR,  //  WSAEventSelect() failed

    E_NEWVIRTUALCONNECTION,  // client is asking for new virtual connection

    // following message is sent only to observer attached to virtual connection
    E_PACKETAVAILABLE,  // next packet can be read from the queue
    E_VIRTUALCONNECTIONCLOSED,  // peer has closed virtual connection

    // following events are sent by CNetworkServer object
    E_NEWCLIENTCONNECTED,  // new client has just connected
    E_CLIENTDISCONNECTED,  // client was disconnected
    E_ACCEPTERERROR  // CNetworkServer::NetworkServerAccepterThread() function failed
};
typedef tagENetworkEvents ENetworkEvents, *PENetworkEvents;


#endif  // !defined(_NETWORKEVENTS__HEADER_INCLUDED_)
