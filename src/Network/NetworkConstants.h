// NetworkConstants.h: basic constants for the Network library
//////////////////////////////////////////////////////////////////////

#if !defined(_NETWORKCONSTANTS__HEADER_INCLUDED_)
#define _NETWORKCONSTANTS__HEADER_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// sender thread - sleep times
//////////////////////////////////////////////////////////////////////

// minimum time in milliseconds to sleep if sent packets link list is too long
// (this is also the default)
#define MIN_SENDER_SLEEP_TIME 50
// maximum time in milliseconds to sleep if sent packets link list is too long
// should be (2^x) * MIN_SENDER_SLEEP_TIME  for some x
#define MAX_SENDER_SLEEP_TIME 3200


//////////////////////////////////////////////////////////////////////
// sender thread - sent packets queue parameters
//////////////////////////////////////////////////////////////////////

// initial length of sent packets link list
#define INIT_SENTPACKETS_NUMBER 200
// initial size of sent packets in the link list
#define INIT_SENTPACKETS_SIZE 262144  // 256kB
// minimum length of sent packets link list
#define MIN_SENTPACKETS_NUMBER 20
// minimum size of sent packets link list
#define MIN_SENTPACKETS_SIZE 32768  // 32kB
// maximum length of sent packets link list
#define MAX_SENTPACKETS_NUMBER 1000
// maximum size of sent packets link list
#define MAX_SENTPACKETS_SIZE 1048576  // 1MB


//////////////////////////////////////////////////////////////////////
// timer thread - lost packets request parameters
//////////////////////////////////////////////////////////////////////

// initial length of the request for resending lost packets
#define INIT_LOSTPACKET_REQUEST_LEN 100
// minimum length of the request for resending lost packets
#define MIN_LOSTPACKET_REQUEST_LEN 10
// maximum length of the request for resending lost packets
#define MAX_LOSTPACKET_REQUEST_LEN 500


//////////////////////////////////////////////////////////////////////
// timer thread - timers initial period lenghts
//////////////////////////////////////////////////////////////////////

// initial synchronization packet timer period length (milliseconds)
#define INIT_SYNTIMER_PERIOD 50
// initial lost packet timer period length (milliseconds)
#define INIT_LOSTTIMER_PERIOD 100

// UDP alive checking -- number of ticks (in lost packet
// timer units) to wait until reporting a network error
// if the symbol is not defined, no alive checking is performed
#define UDP_ALIVE_CHECKING 100

//////////////////////////////////////////////////////////////////////
// time-outs
//////////////////////////////////////////////////////////////////////

// time to wait for threads to finish (in milliseconds)
#define NETWORKCONNECTION_THREAD_TIMEOUT 5000


//////////////////////////////////////////////////////////////////////
// number of retries to recover lost connection
//////////////////////////////////////////////////////////////////////

#define NETWORK_LOSTCONNECTION_RETRIES 0


//////////////////////////////////////////////////////////////////////
// miscellaneous defines
//////////////////////////////////////////////////////////////////////

#define NETWORK_FORCESMALLPACKETS 4096



#endif  // !defined(_NETWORKCONSTANTS__HEADER_INCLUDED_)
