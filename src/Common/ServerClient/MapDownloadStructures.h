// MapDownloadStructures.h: helper structures for map downloading
//////////////////////////////////////////////////////////////////////

#if !defined(_MAPDOWNLOADSTRUCTURES__HEADER_INCLUDED_)
#define _MAPDOWNLOADSTRUCTURES__HEADER_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// initial packet (sent from server to client)
struct SMapDownload_MapInfo
{
    // version of the map (unique)
    DWORD dwMapVersion;
    // the length of the following string 
    // (the name of the map -- client will display it in the progress dialog)
    DWORD dwMapNameLength;
};

// if client does not have the proper map, he will request it (answer from client to server)
struct SMapDownload_DownloadRequest
{
    // absolutely unimportant number...
    DWORD dwDummy;
};

// the size of the map (sent from server to client)
struct SMapDownload_MapSize
{
    // the size of the file (map) which will be send just after this structure
    DWORD dwMapFileSize;
};


#endif // !defined(_MAPDOWNLOADSTRUCTURES__HEADER_INCLUDED_)
