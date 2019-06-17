
#if !defined(_PACKETTYPES__HEADER_INCLUDED_)
#define _PACKETTYPES__HEADER_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// constants used for packet (connection) flags
//////////////////////////////////////////////////////////////////////

#define PACKET_GUARANTEED_DELIVERY ((BYTE) 0x01)
#define PACKET_GUARANTEED_ORDER ((BYTE) 0x02)
    
#define PACKET_TYPE1 ((BYTE) (PACKET_GUARANTEED_DELIVERY | PACKET_GUARANTEED_ORDER))
#define PACKET_TYPE2 ((BYTE) PACKET_GUARANTEED_DELIVERY)
#define PACKET_TYPE3 ((BYTE) PACKET_GUARANTEED_ORDER)
#define PACKET_TYPE4 ((BYTE) 0x00)

//////////////////////////////////////////////////////////////////////
// constants for UDP packet priorities
//////////////////////////////////////////////////////////////////////

#define PACKETPRIORITY_TOTAL_PRIORITIES ((BYTE) 4)

#define PACKETPRIORITY_LOWEST ((BYTE) 0)
#define PACKETPRIORITY_HIGHEST ((BYTE) (PACKETPRIORITY_TOTAL_PRIORITIES - 1))

#define PACKETPRIORITY_IDLE ((BYTE) 0)
#define PACKETPRIORITY_NORMAL ((BYTE) 1)
#define PACKETPRIORITY_ABOVENORMAL ((BYTE) 2)
#define PACKETPRIORITY_HIGH ((BYTE) 3)
    
#define PACKETPRIORITY_ACTUAL ((BYTE) 0xFF)
#define PACKETPRIORITY_DEFAULT PACKETPRIORITY_NORMAL


#endif  // !defined(_PACKETTYPES__HEADER_INCLUDED_)