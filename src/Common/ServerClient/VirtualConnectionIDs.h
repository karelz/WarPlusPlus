#ifndef VIRTUALCONNECTIONIDS_H_
#define VIRTUALCONNECTIONIDS_H_

typedef enum tagEVirtualConnectionID{
  VirtualConnection_Control = 0x00001, // Control connection - basic infos

  VirtualConnection_MapDownload = 0x01000, // virtual connection for map downloading

  VirtualConnection_MapView = 0x05000,  // Map view connection
  VirtualConnection_UnitHierarchy = 0x05001,  // Unit hierarchy connection

  VirtualConnection_LogOutput = 0x06000, // The log output from interpret

  VirtualConnection_ScriptEditor = 0x07000, // The script editor virtual connection
  VirtualConnection_ScriptErrorOutput = 0x07001, // The error output from compiler

  VirtualConnection_ClientControl =	0x08000, // Control connection
  VirtualConnection_ClientRequests = 0x08001, // Client requests transmission

  VirtualConnection_ClientBriefInfos = 0x08002, // Brief info transmission
  VirtualConnection_ClientCheckPointInfos = 0x08003, // CheckPoint info transmission  
  VirtualConnection_ClientFullInfos = 0x08004, // Full info transmission
  VirtualConnection_ClientEnemyFullInfos = 0x08005, // Enemy full info transmission
  VirtualConnection_ClientPing = 0x08006, // Pinging the client with current time

  VirtualConnection_Resources = 0x08007, // Connection for sending resource counts
  VirtualConnection_MiniMap = 0x08008, // For sending compressed minimap to client

} EVirtualConnectionID;

#endif