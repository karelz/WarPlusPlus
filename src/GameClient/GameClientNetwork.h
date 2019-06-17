#ifndef GAMECLIENTNETWORK_H_
#define GAMECLIENTNETWORK_H_

// the only network connection on the client
extern CNetworkConnection *g_pNetworkConnection;

// the network event thread ID
extern DWORD g_dwNetworkEventThreadID;

#endif