#ifndef __COMMUNICATION_INFO_STRUCTURES_H__INCLUDED__
#define __COMMUNICATION_INFO_STRUCTURES_H__INCLUDED__

#include "Common/ServerClient/UnitInfoStructures.h"
#include "Common/ServerClient/MiniMapStructures.h"

#pragma pack(push)
#pragma pack(1)

// Format hlavicky uzivatelskeho requestu
struct SUnitRequestHeaderFormat 
{
	DWORD dwRequestType;
	DWORD dwRequestDataLength;
};

// Enum - zadosti od uzivatele pro server
enum EUserRequests 
{	
// Sledovane obdelniky
	// User zira na dany obdelnik
	UR_StartWatchingRectangle=1, 
	// User uz na dany obdelnik nezira
	UR_StopWatchingRectangle=2, 
	// Sledovany obdelnik se pohnul
	UR_RectangleMoved=3, 

// Full infos
	// Uzivatel chce full info od dane jednotky
	UR_StartFullInfo=4, 
	// Uzivatel uz full info od dane jednotky nechce
	UR_StopFullInfo=5,

// Miscelaneous
	// Konec spojeni
	UR_CloseComms=6,
	// Zadost o obdelnik minimapy
	UR_MiniMap=7,
};

struct SUserRequestStartWatchingRectangle 
{
	// ID obdelniku
	DWORD dwID;
	// Rozmery obdelniku
	DWORD dwLeft, dwTop, dwWidth, dwHeight;
};

struct SUserRequestStopWatchingRectangle 
{
	// ID obdelniku
	DWORD dwID;
};

struct SUserRequestRectangleMoved 
{
	// ID obdelniku
	DWORD dwID;
	// Nove rozmery obdelniku
	DWORD dwLeft, dwTop, dwWidth, dwHeight;
};

struct SUserRequestStartFullInfo 
{
	// ID jednotky
	DWORD dwID;
};

struct SUserRequestStopFullInfo 
{
	// ID jednotky
	DWORD dwID;
};

struct SUserRequestMiniMap : SMiniMapClip
{
	// Jinak prazdna
};

////////////////////////////////////////////////////////

// Format hlavicky zpravy po control connection
struct SControlConnectionHeaderFormat 
{
	DWORD dwCCMessageType;
	DWORD dwCCMessageDataLength;
};

// Enum - zpravy od serveru pro uzivatele
enum EControlConnectionMessages 
{
	// Start info o jednotce
	CCM_StartInfo=1,
	// Stop info o jednotce
	CCM_StopInfo=2,

	// Start full info
	CCM_StartFullInfo=3,
	// Stop full info
	CCM_StopFullInfo=4,

	// Start enemy full info
	CCM_StartEnemyFullInfo=5,
	// Stop enemy full info
	CCM_StopEnemyFullInfo=6
};

// Struktura s informaci o StartInfo
struct SControlConnectionStartInfo : SControlConnectionHeaderFormat, SUnitStartInfo
{
};

// Struktura s informaci o StopInfo
struct SControlConnectionStopInfo : SControlConnectionHeaderFormat 
{
	DWORD dwID;
};

// Struktura s informaci o StartFullInfo
struct SControlConnectionStartFullInfo : SControlConnectionHeaderFormat, SUnitStartFullInfo
{
};

// Struktura s informaci o StopFullInfo
struct SControlConnectionStopFullInfo : SControlConnectionHeaderFormat 
{
	DWORD dwID;
};

// Struktura s informaci o StartEnemyFullInfo
struct SControlConnectionStartEnemyFullInfo : SControlConnectionHeaderFormat, SUnitStartEnemyFullInfo
{
};

// Struktura s informaci o StopEnemyFullInfo
struct SControlConnectionStopEnemyFullInfo : SControlConnectionHeaderFormat 
{
	DWORD dwID;
};

// Union sdruzujici vsechny mozne control infa
union UControlConnection {
	SControlConnectionHeaderFormat ccHeader;
	
	SControlConnectionStartInfo ccStartInfo;
	SControlConnectionStopInfo ccStopInfo;
	
	SControlConnectionStartFullInfo ccStartFullInfo;
	SControlConnectionStopFullInfo ccStopFullInfo;
	
	SControlConnectionStartEnemyFullInfo ccStartEnemyFullInfo;
	SControlConnectionStopEnemyFullInfo ccStopEnemyFullInfo;
};

#pragma pack(pop)

#endif // __COMMUNICATION_INFO_STRUCTURES_H__INCLUDED__