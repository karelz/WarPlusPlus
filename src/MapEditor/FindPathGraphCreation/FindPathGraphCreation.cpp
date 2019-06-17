/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: MapEditor
 *   Autor: Karel Zikmund
 * 
 *   Popis: Vytvoøení grafù pro hledání cesty
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "FindPathGraphCreation.h"

#include <afxtempl.h>

#include "Common\AfxDebugPlus\AfxDebugPlus.h"
#include "Common\MemoryPool\Array2DOnDemand.h"
#include "Common\MemoryPool\SelfPooledList.h"
#include "Common\MemoryPool\SelfPooledQueue.h"
#include "Common\MemoryPool\SelfPooledPriorityQueue.h"
#include "Common\MemoryPool\SelfPooledStack.h"
#include "Common\MemoryPool\PooledList.h"
#include "Common\Map\Map.h"
#include "Common\Map\MFindPathGraphs.h"

#include "GameServer\GameServer\GameServerCompileSettings.h"
#include "GameServer\GameServer\GeneralClasses\GeneralClasses.h"
#include "GameServer\GameServer\AbstractDataClasses\256BitArray.h"

#ifdef _DEBUG
	#define __SHARED_MEMORY__
#endif //_DEBUG

#ifdef __SHARED_MEMORY__
#include "MapDescription.h"
#endif //__SHARED_MEMORY__

#define MAP_BORDER	2

#define MAPCELL_UNINITIALIZED		((signed char)0xff)
#define MAPCELL_ALLOWED				((signed char)0x02)
#define MAPCELL_DISALLOWED			((signed char)0x00)

// velikost nejvìtšího kruhu pro vytváøení grafu pro hledání cesty
#define MAX_CIRCLE_SIZE		25

#define MARK_EXPLORED(cMapCellID)	( (signed char)cMapCellID | (signed char)0x80 )
#define UNMARK_EXPLORED(cMapCellID)	( (signed char)cMapCellID & (signed char)(~(signed char)0x80) )

#ifdef _DEBUG
	BOOL g_bTraceFindPathCreation = TRUE;
	#define TRACE_FIND_PATH_CREATION if ( g_bTraceFindPathCreation ) TRACE_NEXT
#else //!_DEBUG
	#define TRACE_FIND_PATH_CREATION TRACE_NEXT
#endif //!_DEBUG

// zkontroluje podmínku "condition", neplatí-li, vrací ECFPGE_BadMapFile
#define CHECK_MAP_FILE(condition) \
	do { if ( !( condition ) ) { throw new CCFPGMapFileException ( ECFPGE_BadMapFile ); } } \
	while ( 0 )

//////////////////////////////////////////////////////////////////////
// Datové typy
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Tøída výjimky naèítání souboru mapy
class CCFPGMapFileException : public CException 
{
// Metody
public:

	// konstruktor
	CCFPGMapFileException ( enum ECreateFindPathGraphError eError ) 
		{ m_eError = eError; };
	// destruktor
	virtual ~CCFPGMapFileException () {};

	// vrací informaci o výjimce
	virtual BOOL GetErrorMessage ( LPTSTR lpszError, UINT nMaxError, 
		PUINT pnHelpContext = NULL )
	{
		ASSERT ( ( lpszError != NULL ) && AfxIsValidString ( lpszError, nMaxError ) );

		if ( pnHelpContext != NULL )
		{
			*pnHelpContext = 0;
		}

		CString strMessage ( "Map file error" );
		lstrcpyn ( lpszError, strMessage, nMaxError );

		return TRUE;
	};

// Data
public:
	// chybový kód
	enum ECreateFindPathGraphError m_eError;
};

//////////////////////////////////////////////////////////////////////
// Tøída mapy vytváøení grafu pro hledání cesty
class CCFPGMap 
{
// Datové typy
protected:
	// typ Mapexu
	struct SMapexType 
	{
		DWORD dwSizeX;
		DWORD dwSizeY;
	};

	// blok pøebarvování oblasti
	struct SRepaintAreaBlock 
	{
		int nMinX;
		int nMaxX;
		int nY;

		// konstruktor
		SRepaintAreaBlock () {};
		// konstruktor
		SRepaintAreaBlock ( int nInitMinX, int nInitMaxX, int nInitY ) 
			{ nMinX = nInitMinX; nMaxX = nInitMaxX; nY = nInitY; };
		// operátor pøiøazení
		const struct SRepaintAreaBlock &operator= ( const struct SRepaintAreaBlock &sBlock ) 
			{ nMinX = sBlock.nMinX; nMaxX = sBlock.nMaxX; nY = sBlock.nY; return *this; };
	};

public:
	// oblast/most
	struct SAreaBridge;
	// brána mostu
	struct SBridgeGate 
	{
		// pozice brány mostu
		CPoint pointPosition;
		// pozice dotyku oblasti/mostu
		CPoint pointTouchPosition;
		// vzdálenost dotyku oblasti/mostu (10-ti násobek vzdálenosti)
		DWORD dwTouchDistance;

		// ukazatel na oblast brány mostu (mùže být i ukazatel na most brány mostu)
		struct SAreaBridge *pArea;
		// ukazatel na most brány mostu
		struct SAreaBridge *pBridge;

		// index do tabulky bran
		DWORD dwIndex;

		// konstruktor
		SBridgeGate ( CPoint pointInitPosition, struct SAreaBridge *pInitArea ) 
			{ pointPosition = pointInitPosition; pArea = pInitArea; pBridge = NULL; };
		// konstruktor
		SBridgeGate () {};
	};
public:
	// tøída seznamu bran mostu
	typedef CPooledList<struct SBridgeGate *, 1> CBridgeGateList;

	// oblast/most
	struct SAreaBridge 
	{
		// pozice oblasti/mostu na mapì
		CPoint pointPosition;

		// identifikátor MapCellu oblasti/mostu
		signed char cMapCellID;

		// seznam primárních bran mostu/mostù oblasti
		CBridgeGateList cPrimaryBridgeGateList;
		// seznam sekundárních bran mostu/mostù oblasti
		CBridgeGateList cSecondaryBridgeGateList;

		// index do tabulky oblastí
		DWORD dwIndex;

		// konstruktor
		SAreaBridge ( CPoint pointInitPosition, signed char cInitMapCellID ) 
			{ pointPosition = pointInitPosition; cMapCellID = cInitMapCellID; };
		// konstruktor
		SAreaBridge () {};
		// operátor new
		void *operator new ( size_t nSize, 
			CTypedMemoryPool<struct SAreaBridge> &cAreaBridgePool ) 
			{ return (void *)cAreaBridgePool.Allocate (); };
		// operátor delete
		void operator delete ( void *pData, 
			CTypedMemoryPool<struct SAreaBridge> &cAreaBridgePool ) 
			{ cAreaBridgePool.Free ( (struct SAreaBridge *)pData ); };
	};
protected:
	// tøída seznamu oblastí/mostù
	typedef CSelfPooledList<struct SAreaBridge *> CAreaBridgeList;

	// oznaèená oblast
	struct SMarkedArea 
	{
		int nX;
		int nY;

		// konstruktor
		SMarkedArea ( int nInitX, int nInitY ) { nX = nInitX; nY = nInitY; };
		// konstruktor
		SMarkedArea () {};
		// operátor pøiøazení
		const struct SMarkedArea &operator= ( const struct SMarkedArea &sArea ) 
			{ nX = sArea.nX; nY = sArea.nY; return *this; };
	};
	// tøída seznamu oznaèených oblastí
	typedef CSelfPooledStack<struct SMarkedArea> CMarkedAreaStack;

// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CCFPGMap ();
	// destruktor
	~CCFPGMap ();

	// vytvoøí graf pro hledání cesty "dwFindPathGraphIndex" na mapì "cMapArchive", je-li 
	//		nastaven pøíznak "bCheckCompleteLand" kontroluje úplnost povrchu mapy
	enum ECreateFindPathGraphError Create ( DWORD dwFindPathGraphIndex, 
		CDataArchive cMapArchive, BOOL bCheckCompleteLand );
	// znièí mapu
	void Delete ();

// Operace s mapou

	// vytvoøí graf pro hledání cesty v archivu "cFindPathGraphsArchive"
	void CreateFindPathGraph ( CDataArchive cFindPathGraphsArchive );
protected:
	// pøebarví oblast "cOldColor" zaèínající na souøadnicích "nX" a "nY" barvou 
	//		"cNewColor"
	void RepaintArea ( signed char cOldColor, signed char cNewColor, int nX, int nY );
	// prohledá øádek "nY" od "nMinX" ("pLine") do "nMaxX" pro pøekreslování oblasti
	void RepaintAreaSearchLine ( signed char *pLine, int nMinX, int nMaxX, int nY );

	// pøebarví oblast "cOldColor" zaèínající na souøadnicích "nX" a "nY" barvou "cNewColor"
	// zároveò hledá reprezentanta ( mapcell, který má hodnotu "cRepresentative" )
	// dìlá to floodfillem
	// vrací uklazatel na reprezentanta ( nebo NULL )
	signed char * FindAreaRepresentative ( signed char cOldColor, signed char cNewColor, int nX, 
		int nY, signed char cRepresentative );
	// prohledá øádek "nY" od "nMinX" ("pLine") do "nMaxX" pro pøekreslování oblasti
	signed char * FindAreaRepresentativeSearchLine ( signed char *pLine, int nMinX, int nMaxX, int nY, signed char cRepresentative );
	
	// najde brány mostu "cMapCellID" na "pPosition" a vloží je do "cBridgeGateList"
	void GetBridgeGates ( signed char cMapCellID, signed char *pPosition, 
		CBridgeGateList &cBridgeGateList );
	// vrátí oblast/most na pozici "pPosition"
	struct SAreaBridge *FindAreaBridge ( signed char *pPosition );
	// vyplní vzdálenosti od brány mostu "pBridgeGate" k branám mostù "cBridgeGateList" 
	//		oblasti "pAreaBridge" do tabulky vzdáleností bran mostù "pBridgeGateDistances"
	void GetBridgeGateDistances ( struct SAreaBridge *pAreaBridge,
		struct SBridgeGate *pBridgeGate, CBridgeGateList &cBridgeGateList, 
		DWORD *pBridgeGateDistances );
	// najde dotyk brány mostu
	void FindBridgeGateTouch ( struct SBridgeGate *pBridgeGate, 
		struct SAreaBridge *pAreaBridge );

// Informace o mapì

	// vrátí ukazatel na MapCell na souøadnicích "nX", "nY" v souøadnicích mapy s okraji
	inline signed char *GetAt ( int nX, int nY );
	// vrátí ukazatel na MapCell na souøadnicích "pointPosition" v souøadnicích mapy 
	//		s okraji
	signed char *GetAt ( CPoint pointPosition ) 
		{ return GetAt ( pointPosition.x, pointPosition.y ); };
	// vrátí pozici MapCellu "pMapCell" na mapì v souøadnicích mapy s okraji
	inline CPoint GetMapCellPosition ( signed char *pMapCell );

// Pomocné operace

	// vrátí ID MapCellu s kruhem velikosti "nCircleSize"
	inline signed char GetMapCellIDFromCircleSize ( int nCircleSize );
	// vrátí ID MapCellu MapCellu oblasti "cMapCellAreaID"
	inline signed char GetMapCellIDFromMapCellAreaID ( signed char cMapCellAreaID );
	// vrátí ID MapCellu oblasti s kruhem velikosti "nCircleSize"
	inline signed char GetMapCellAreaIDFromCircleSize ( int nCircleSize );
	// vrátí ID MapCellu oblasti MapCellu "cMapCellID"
	inline signed char GetMapCellAreaIDFromMapCellID ( signed char cMapCellID );
	// zjistí, je-li "cMapCellID" ID MapCellu oblasti
	inline BOOL IsMapCellAreaID ( signed char cMapCellID );
	// vrátí vzdálenost èísel "nX" a "nY"
	int GetDistance ( int nX, int nY ) { return ( nX < nY ) ? ( nY - nX ) : ( nX - nY ); };
	// zvìtší tabulku hintù cesty o velikost "dwSize"
	inline void IncreasePathHintTable ( DWORD dwSize );

// Data
private:
	// rozmìry mapy v MapCellech
	int m_nSizeX;
	int m_nSizeY;

	// nakreslená mapa
	signed char *m_pMap;
	// rozmìry nakreslené mapy v MapCellech
	int m_nMapSizeX;
	int m_nMapSizeY;
	// velikost okraje nakreslené mapy
	int m_nMapBorder;
	// hranice mapy
	int m_nMapXMin;
	int m_nMapXMax;
	int m_nMapYMin;
	int m_nMapYMax;

	// jméno grafu pro hledání cesty
	CString m_strFindPathGraphName;

	// kruhy pro vytváøení grafu pro hledání cesty
	static signed char *m_aCircles[MAX_CIRCLE_SIZE-1];

	// seznam oblastí/mostù
	CAreaBridgeList m_cAreaBridgeList;

// data prohledávání øádky pro pøebarvování oblasti

	// barva pøebarvované oblasti
	signed char m_cRepaintAreaSearchLineAreaColor;
	// zásobník blokù pro objevování oblasti
	CSelfPooledStack<struct SRepaintAreaBlock> m_cRepaintAreaBlockStack;

// data hledání bran mostù a fiktivních oblastí

	// offsety okolních MapCellù
	int m_aSurroundingMapCellOffset[8];
	// offsety vzdálených okolních MapCellù
	int m_aFarSurroundingMapCellOffset[16];
	// offsety MapCellù souvislého okolí MapCellu
	int m_aContinuousMapCellOffset[8][4];
	// offsety vzdálených souvislých MapCellù
	int m_aFarContinuousMapCellOffset[8][5];
	// offsety okolních MapCellù vzdáleného MapCellu
	int m_aSurroundingFarMapCellOffset[16][3];
	// offsety okolních pozic
	static CPoint m_aSurroundingPositionOffset[8];
	// zásobník oznaèených oblastí
	CMarkedAreaStack m_cMarkedAreaStack;
	// zásobník ukazatelù na doèasnì oznaèené MapCelly
	CSelfPooledStack<signed char *> m_cTemporarilyMarkedMapCellStack;
	// fronta MapCellù
	CSelfPooledQueue<signed char *> m_cMapCellQueue;
	// zásobník ukazatelù na doèasnì oznaèené MapCelly pro hledání oblastí/mostù
	CSelfPooledStack<signed char *> m_cFindAreaBridgeTemporarilyMarkedMapCellStack;
	// fronta MapCellù
	CSelfPooledQueue<signed char *> m_cFindAreaBridgeMapCellQueue;
	// memory pool bran mostù
	CTypedMemoryPool<struct SBridgeGate> m_cBridgeGatePool;
	// memory pool oblastí/mostù
	CTypedMemoryPool<struct SAreaBridge> m_cAreaBridgePool;
	// prioritní fronta MapCellù
	CSelfPooledPriorityQueue<DWORD, signed char *> m_cPriorityMapCellQueue;
	// pole vzdáleností MapCellù
	CArray2DOnDemand<DWORD> m_cMapCellDistanceArray;
	// tabulka hintù cest
	DWORD *m_pPathHintTable;
	// velikost tabulky hintù cest (v DWORDech)
	DWORD m_dwPathHintTableSize;
	// alokovaná velikost tabulky hintù cest (v DWORDech)
	DWORD m_dwAllocatedPathHintTableSize;

#ifdef __SHARED_MEMORY__
	// zámek sdílené pamìti
	CMutex m_mutexSharedMemoryLock;

	// mapovaný soubor mapy
	HANDLE m_hMap;
	// mapovaný soubor popisu mapy
	HANDLE m_hMapDescription;

	// popis mapy
	struct SMapDescription *m_pMapDescription;
#endif //__SHARED_MEMORY__
};

//////////////////////////////////////////////////////////////////////
// Statická data tøídy CCFPGMap
//////////////////////////////////////////////////////////////////////

// memory pool seznamu bran mostu
CTypedMemoryPool<struct CCFPGMap::CBridgeGateList::SListMember> 
	CCFPGMap::CBridgeGateList::m_cPool ( 1000 );

// offsety okolních pozic
CPoint CCFPGMap::m_aSurroundingPositionOffset[8] = {
	CPoint ( -1, -1 ),
	CPoint ( 0, -1 ),
	CPoint ( 1, -1 ),
	CPoint ( 1, 0 ),
	CPoint ( 1, 1 ),
	CPoint ( 0, 1 ),
	CPoint ( -1, 1 ),
	CPoint ( -1, 0 )
};

// kruhy pro vytváøení grafu pro hledání cesty
static signed char g_aCircle02[] = { 
	1,1,
	1,1
};
static signed char g_aCircle03[] = { 
	1,1,1,
	1,1,1,
	1,1,1
};
static signed char g_aCircle04[] = { 
	1,1,1,1,
	1,1,1,1,
	1,1,1,1,
	1,1,1,0
};
static signed char g_aCircle05[] = { 
	0,1,1,1,0,
	1,1,1,1,1,
	1,1,1,1,1,
	1,1,1,1,1,
	0,1,1,1,0
};
static signed char g_aCircle06[] = { 
	0,1,1,1,1,0,
	1,1,1,1,1,0,
	1,1,1,1,1,1,
	1,1,1,1,1,1,
	1,1,1,1,1,0,
	0,0,1,1,0,0
};
static signed char g_aCircle07[] = { 
	0,0,1,1,1,0,0,
	0,1,1,1,1,1,0,
	1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,
	0,1,1,1,1,1,0,
	0,0,1,1,1,0,0
};
static signed char g_aCircle08[] = { 
	0,0,1,1,1,1,0,0,
	0,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,0,
	0,0,1,1,1,0,0,0
};
static signed char g_aCircle09[] = { 
	0,0,0,1,1,1,0,0,0,
	0,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,0,
	0,0,0,1,1,1,0,0,0
};
static signed char g_aCircle10[] = { 
	0,0,0,1,1,1,1,0,0,0,
	0,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,0,0,0,
	0,0,0,1,1,0,0,0,0,0
};
static signed char g_aCircle11[] = { 
	0,0,0,0,1,1,1,0,0,0,0,
	0,0,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,0,0,
	0,0,0,0,1,1,1,0,0,0,0
};
static signed char g_aCircle12[] = { 
	0,0,0,1,1,1,1,1,1,0,0,0,
	0,1,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,0,0,
	0,0,0,1,1,1,1,1,0,0,0,0
};
static signed char g_aCircle13[] = { 
	0,0,0,0,1,1,1,1,1,0,0,0,0,
	0,0,1,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,0,1,1,1,1,1,0,0,0,0
};
static signed char g_aCircle14[] = { 
	0,0,0,1,1,1,1,1,1,1,0,0,0,0,
	0,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,0,1,1,1,1,0,0,0,0,0
};
static signed char g_aCircle15[] = { 
	0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,0,1,1,1,1,1,0,0,0,0,0
};
static signed char g_aCircle16[] = { 
	0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0
};
static signed char g_aCircle17[] = { 
	0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0
};
static signed char g_aCircle18[] = { 
	0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0
};
static signed char g_aCircle19[] = { 
	0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0
};
static signed char g_aCircle20[] = { 
	0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0
};
static signed char g_aCircle21[] = { 
	0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0
};
static signed char g_aCircle22[] = { 
	0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0
};
static signed char g_aCircle23[] = { 
	0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0
};
static signed char g_aCircle24[] = { 
	0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0
};
static signed char g_aCircle25[] = {
	0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0
};

// tabulka kruhù pro vytváøení grafu pro hledání cesty
signed char *CCFPGMap::m_aCircles[MAX_CIRCLE_SIZE-1] = { 
	g_aCircle02, g_aCircle03, g_aCircle04, g_aCircle05, g_aCircle06, 
	g_aCircle07, g_aCircle08, g_aCircle09, g_aCircle10, g_aCircle11, 
	g_aCircle12, g_aCircle13, g_aCircle14, g_aCircle15, g_aCircle16, 
	g_aCircle17, g_aCircle18, g_aCircle19, g_aCircle20, g_aCircle21,
	g_aCircle22, g_aCircle23, g_aCircle24, g_aCircle25
};

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce tøídy CCFPGMap
//////////////////////////////////////////////////////////////////////

// konstruktor
CCFPGMap::CCFPGMap () : m_cAreaBridgeList ( 1000 ), m_cRepaintAreaBlockStack ( 1000 ), 
	m_cMarkedAreaStack ( 400 ), m_cTemporarilyMarkedMapCellStack ( 100 ), 
	m_cMapCellQueue ( 200 ), m_cFindAreaBridgeTemporarilyMarkedMapCellStack ( 100 ), 
	m_cFindAreaBridgeMapCellQueue ( 200 ), m_cBridgeGatePool ( 1000 ), 
	m_cAreaBridgePool ( 1000 ), m_cPriorityMapCellQueue ( 1000 ), 
	m_cMapCellDistanceArray ( 100 ) 
#ifdef __SHARED_MEMORY__
	, m_mutexSharedMemoryLock ( FALSE, SHARED_MEMORY_LOCK_NAME ) 
#endif //__SHARED_MEMORY__
{
	m_pMap = NULL;

#ifdef __SHARED_MEMORY__
	m_hMap = NULL;
	m_hMapDescription = NULL;
	m_pMapDescription = NULL;
#endif //__SHARED_MEMORY__
}

// destruktor
CCFPGMap::~CCFPGMap () 
{
	ASSERT ( m_pMap == NULL );

#ifdef __SHARED_MEMORY__
	ASSERT ( m_hMap == NULL );
	ASSERT ( m_hMapDescription == NULL );
	ASSERT ( m_pMapDescription == NULL );
#endif //__SHARED_MEMORY__
}

// vytvoøí graf pro hledání cesty "dwFindPathGraphIndex" na mapì "cMapArchive", je-li 
//		nastaven pøíznak "bCheckCompleteLand" kontroluje úplnost povrchu mapy
enum ECreateFindPathGraphError CCFPGMap::Create ( DWORD dwFindPathGraphIndex, 
	CDataArchive cMapArchive, BOOL bCheckCompleteLand ) 
{
	ASSERT ( m_pMap == NULL );

	// návratová hodnota
	enum ECreateFindPathGraphError eReturnValue = ECFPGE_OK;
	// tabulka pøekladu ID typu Mapexu na ukazatel na nìj
	CMap<DWORD, DWORD, void *, void *> cMapexTypeTable;

	try
	{
	// naète data mapy ze souboru mapy

		// otevøe soubor mapy
		CArchiveFile cMapFile = cMapArchive.CreateFile ( MAP_FILE_NAME, CFile::modeRead | 
			CFile::shareDenyWrite );

		// poèet knihoven Mapexù
		DWORD dwMapexLibraryCount;
		// poèet civilizací
		DWORD dwCivilizationCount;
		// poèet knihoven jednotek
		DWORD dwUnitTypeLibraryCount;
		// poèet jednotek na mapì
		DWORD dwUnitCount;
		// poèet ScriptSetù
		DWORD dwScriptSetCount;
		// poèet grafù pro hledání cesty
		DWORD dwFindPathGraphCount;
		// použité typy povrchu
		C256BitArray aUsedLandTypes;
		// povolené typy povrchu grafu pro hledání cesty
		C256BitArray aAllowedLandTypes;

	// naète hlavièku verze souboru
		{
			SFileVersionHeader sFileVersionHeader;
			CHECK_MAP_FILE ( cMapFile.Read ( &sFileVersionHeader, sizeof ( sFileVersionHeader ) ) == sizeof ( sFileVersionHeader ) );

			// identifikátor souboru mapy
			BYTE aMapFileID[16] = MAP_FILE_IDENTIFIER;
			ASSERT ( sizeof ( aMapFileID ) == sizeof ( sFileVersionHeader.m_aMapFileID ) );

			// zkontroluje identifikátor souboru mapy
			for ( int nIndex = 16; nIndex-- > 0; )
			{
				CHECK_MAP_FILE ( aMapFileID[nIndex] == sFileVersionHeader.m_aMapFileID[nIndex] );
			}

			ASSERT ( CURRENT_MAP_FILE_VERSION >= COMPATIBLE_MAP_FILE_VERSION );
			// zkontroluje verze formátu mapy
			CHECK_MAP_FILE ( sFileVersionHeader.m_dwFormatVersion >= 
				sFileVersionHeader.m_dwCompatibleFormatVersion );

			// zjistí, jedná-li se o správnou verzi formátu mapy
			if ( sFileVersionHeader.m_dwFormatVersion != CURRENT_MAP_FILE_VERSION )
			{	// nejedná se o správnou verzi formátu mapy
				// zjistí, jedná-li se o starou verzi formátu mapy
				if ( sFileVersionHeader.m_dwFormatVersion < CURRENT_MAP_FILE_VERSION )
				{	// jedná se o starou verzi formátu mapy
					// zkontroluje kompatabilitu verze formátu mapy
					CHECK_MAP_FILE ( sFileVersionHeader.m_dwFormatVersion >= 
						COMPATIBLE_MAP_FILE_VERSION );
				}
				else
				{	// jedná se o mladší verzi formátu mapy
					// zkontroluje kompatabilitu verze formátu mapy
					CHECK_MAP_FILE ( sFileVersionHeader.m_dwCompatibleFormatVersion <= 
						CURRENT_MAP_FILE_VERSION );
				}
			}
		}

	// naète hlavièku mapy
		{
			SMapHeader sMapHeader;
			CHECK_MAP_FILE ( cMapFile.Read ( &sMapHeader, sizeof ( sMapHeader ) ) == sizeof ( sMapHeader ) );

		// zpracuje hlavièku mapy

			// naète velikost mapy
			m_nSizeX = (int)sMapHeader.m_dwWidth;
			CHECK_MAP_FILE ( ( m_nSizeX > 0 ) && ( m_nSizeX % MAPSQUARE_WIDTH == 0 ) );
			m_nSizeY = (int)sMapHeader.m_dwHeight;
			CHECK_MAP_FILE ( ( m_nSizeY > 0 ) && ( m_nSizeY % MAPSQUARE_HEIGHT == 0 ) );

			// zkontroluje velikost mapy
			CHECK_MAP_FILE ( m_nSizeX >= 2 * MAPSQUARE_WIDTH );
			CHECK_MAP_FILE ( m_nSizeY >= 2 * MAPSQUARE_HEIGHT );
			CHECK_MAP_FILE ( m_nSizeX * m_nSizeY > 0 );
			CHECK_MAP_FILE ( m_nSizeX < m_nSizeX * m_nSizeY );
			CHECK_MAP_FILE ( m_nSizeY < m_nSizeX * m_nSizeY );

			// alokuje nakreslenou mapu
			m_nMapBorder = MAP_BORDER;
			m_nMapSizeX = m_nSizeX + 2 * m_nMapBorder;
			m_nMapSizeY = m_nSizeY + 2 * m_nMapBorder;
			m_nMapXMin = -m_nMapBorder;
			m_nMapXMax = m_nSizeX + m_nMapBorder;
			m_nMapYMin = -m_nMapBorder;
			m_nMapYMax = m_nSizeY + m_nMapBorder;

#ifdef __SHARED_MEMORY__
		// sdílené pamìti

			ASSERT ( m_nMapSizeX * m_nMapSizeY <= SHARED_MEMORY_MAP_SIZE );

			// zamkne zámek sdílených pamìtí
			VERIFY ( m_mutexSharedMemoryLock.Lock () );

			// vytvoøí mapování mapy
			m_hMap = CreateFileMapping ( (HANDLE)0xffffffff, NULL, PAGE_READWRITE, 0, 
				SHARED_MEMORY_MAP_SIZE, SHARED_MEMORY_MAP_NAME );
			ASSERT ( m_hMap != NULL );

			// namapuje pamì mapy
			m_pMap = (signed char *)MapViewOfFile ( m_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 
				SHARED_MEMORY_MAP_SIZE );
			ASSERT ( m_pMap != NULL );

			// vytvoøí mapování popisu mapy
			m_hMapDescription = CreateFileMapping ( (HANDLE)0xffffffff, NULL, PAGE_READWRITE, 0, 
				SHARED_MEMORY_MAPDESCRIPTION_SIZE, SHARED_MEMORY_MAPDESCRIPTION_NAME );
			ASSERT ( m_hMapDescription != NULL );

			// namapuje pamì popisu mapy
			m_pMapDescription = (struct SMapDescription *)MapViewOfFile ( m_hMapDescription, 
				FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEMORY_MAPDESCRIPTION_SIZE );
			ASSERT ( m_pMapDescription != NULL );

			m_pMapDescription->m_nSizeX = m_nSizeX;
			m_pMapDescription->m_nSizeY = m_nSizeY;
			m_pMapDescription->m_nMapSizeX = m_nMapSizeX;
			m_pMapDescription->m_nMapSizeY = m_nMapSizeY;
			m_pMapDescription->m_nMapBorder = m_nMapBorder;
			m_pMapDescription->m_nMapXMax = m_nMapXMax;
			m_pMapDescription->m_nMapXMin = m_nMapXMin;
			m_pMapDescription->m_nMapYMax = m_nMapYMax;
			m_pMapDescription->m_nMapYMin = m_nMapYMin;

			// odemkne zámek sdílených pamìtí
			VERIFY ( m_mutexSharedMemoryLock.Unlock () );

			// ukazatel na øádek mapy
			signed char *pLine = m_pMap;
			// inicilizuje horní okraj mapy
			memset ( pLine, MAPCELL_DISALLOWED, m_nMapSizeX * MAP_BORDER + MAP_BORDER );
			// inicializuje støed mapy
			pLine = GetAt ( MAP_BORDER, MAP_BORDER );
			memset ( pLine, MAPCELL_UNINITIALIZED, m_nMapSizeX * m_nSizeY - 2 * MAP_BORDER );
			// inicilizuje dolní okraj mapy
			pLine = GetAt ( 0, m_nSizeY + MAP_BORDER );
			memset ( pLine, MAPCELL_DISALLOWED, m_nMapSizeX * MAP_BORDER );
			ASSERT ( ( pLine + m_nMapSizeX * MAP_BORDER ) == ( m_pMap + m_nMapSizeX * 
				m_nMapSizeY ) );

			// inicializuje levý a pravý okraj mapy
			pLine = GetAt ( m_nSizeX + MAP_BORDER, MAP_BORDER );
			for ( DWORD dwIndexY = m_nSizeY; dwIndexY-- > 0; pLine += m_nMapSizeX )
			{
				// inicializuje levý okraj øádky "pLine" mapy
				memset ( pLine, MAPCELL_DISALLOWED, 2 * MAP_BORDER );
			}
#else //!__SHARED_MEMORY__
			// alokuje mapu
			m_pMap = new signed char[m_nMapSizeX * m_nMapSizeY];

		// inicializuje mapu
			{
				// zjistí, má-li se kontrolovat povrch mapy
				if ( bCheckCompleteLand )
				{	// má se kontrolovat povrch mapy
					// ukazatel na øádek mapy
					signed char *pLine = m_pMap;
					// inicilizuje horní okraj mapy
					memset ( pLine, MAPCELL_DISALLOWED, m_nMapSizeX * MAP_BORDER + MAP_BORDER );
					// inicializuje støed mapy
					pLine = GetAt ( MAP_BORDER, MAP_BORDER );
					memset ( pLine, MAPCELL_UNINITIALIZED, m_nMapSizeX * m_nSizeY - 2 * MAP_BORDER );
					// inicilizuje dolní okraj mapy
					pLine = GetAt ( 0, m_nSizeY + MAP_BORDER );
					memset ( pLine, MAPCELL_DISALLOWED, m_nMapSizeX * MAP_BORDER );
					ASSERT ( ( pLine + m_nMapSizeX * MAP_BORDER ) == ( m_pMap + m_nMapSizeX * 
						m_nMapSizeY ) );

					// inicializuje levý a pravý okraj mapy
					pLine = GetAt ( m_nSizeX + MAP_BORDER, MAP_BORDER );
					for ( DWORD dwIndexY = m_nSizeY; dwIndexY-- > 0; pLine += m_nMapSizeX )
					{
						// inicializuje levý okraj øádky "pLine" mapy
						memset ( pLine, MAPCELL_DISALLOWED, 2 * MAP_BORDER );
					}
				}
				else
				{	// nemá se kontrolovat povrch mapy
					// inicializuje mapu
					memset ( m_pMap, MAPCELL_UNINITIALIZED, m_nMapSizeX * m_nMapSizeY );
#ifdef _DEBUG
					// ukazatel na øádek mapy
					signed char *pLine = m_pMap;
					// inicilizuje horní okraj mapy
					memset ( pLine, MAPCELL_DISALLOWED, m_nMapSizeX * MAP_BORDER + MAP_BORDER );
					// inicilizuje dolní okraj mapy
					pLine = GetAt ( 0, m_nSizeY + MAP_BORDER );
					memset ( pLine, MAPCELL_DISALLOWED, m_nMapSizeX * MAP_BORDER );
					ASSERT ( ( pLine + m_nMapSizeX * MAP_BORDER ) == ( m_pMap + m_nMapSizeX * 
						m_nMapSizeY ) );

					// inicializuje levý a pravý okraj mapy
					pLine = GetAt ( m_nSizeX + MAP_BORDER, MAP_BORDER );
					for ( DWORD dwIndexY = m_nSizeY; dwIndexY-- > 0; pLine += m_nMapSizeX )
					{
						// inicializuje levý okraj øádky "pLine" mapy
						memset ( pLine, MAPCELL_DISALLOWED, 2 * MAP_BORDER );
					}
#endif //_DEBUG
				}
			}
#endif //!__SHARED_MEMORY__

			// zkontroluje poèet knihoven Mapexù
			dwMapexLibraryCount = sMapHeader.m_dwMapexLibraryCount;
			CHECK_MAP_FILE ( dwMapexLibraryCount > 0 );

			// zkontroluje poèet civilizací
			dwCivilizationCount = sMapHeader.m_dwCivilizationsCount;
			CHECK_MAP_FILE ( dwCivilizationCount > 0 );

			// zkontroluje poèet knihoven typù jednotek
			dwUnitTypeLibraryCount = sMapHeader.m_dwUnitTypeLibraryCount;
			CHECK_MAP_FILE ( dwUnitTypeLibraryCount > 0 );

			// zjistí poèet jednotek na mapì
			dwUnitCount = sMapHeader.m_dwUnitCount;

			// zjistí použité typy povrchu
			memcpy ( aUsedLandTypes.GetData (), sMapHeader.m_aUsedLandTypes, 
				sizeof ( sMapHeader.m_aUsedLandTypes ) );
// *******************************************
// *******************************************
// *******************************************
// *******************************************
// *******************************************
//			CHECK_MAP_FILE ( !aUsedLandTypes.GetAt ( TRANSPARENT_LAND_TYPE_ID ) );
// *******************************************
// *******************************************
// *******************************************
// *******************************************
// *******************************************

			// zjistí poèet ScriptSetù
			dwScriptSetCount = sMapHeader.m_dwScriptSetCount;
			CHECK_MAP_FILE ( dwScriptSetCount > 0 );

			// zjistí poèet grafù pro hledání cesty
			dwFindPathGraphCount = sMapHeader.m_dwFindPathGraphCount;
			ASSERT ( dwFindPathGraphIndex < dwFindPathGraphCount );
		}

	// pøeskoèí hlavièky ScriptSetù
		{
			// zjistí velikost hlavièek ScriptSetù
			DWORD dwScriptSetSize = dwScriptSetCount * sizeof ( SScriptSetHeader );

			// pøeskoèí hlavièky ScriptSetù
			CHECK_MAP_FILE ( cMapFile.GetPosition () + dwScriptSetSize <= 
				cMapFile.GetLength () );
			(void)cMapFile.Seek ( dwScriptSetSize, CFile::current );
		}

	// zjistí povolené typy povrchu grafu pro hledání cesty "dwFindPathGraphIndex"
		{
			// pozice odkazù na knihovny Mapexù
			DWORD dwMapexLibraryNodesPosition = cMapFile.GetPosition ();

		// pøeskoèí odkazy na knihovny Mapexù, hlavièky civilizací, odkazy na knihovny typù 
		//		jednotek a pole offsetù MapSquarù
			{
				// zjistí velikost pøeskakovaných dat
				DWORD dwSkippedDataSize = dwMapexLibraryCount * sizeof ( SMapexLibraryNode ) + 
					dwCivilizationCount * sizeof ( SCivilizationHeader ) + 
					dwUnitTypeLibraryCount * sizeof ( SUnitTypeLibraryNode ) + ( m_nSizeX / 
					MAPSQUARE_WIDTH ) * ( m_nSizeY / MAPSQUARE_HEIGHT ) * sizeof ( DWORD );

				// pøeskoèí data
				CHECK_MAP_FILE ( cMapFile.GetPosition () + dwSkippedDataSize <= 
					cMapFile.GetLength () );
				(void)cMapFile.Seek ( dwSkippedDataSize, CFile::current );
			}

		// pøeskoèí informace o MapSquarech
			{
				// rozmìry mapy v MapSquarech
				DWORD dwSizeX = (DWORD)( m_nSizeX / MAPSQUARE_WIDTH );
				DWORD dwSizeY = (DWORD)( m_nSizeY / MAPSQUARE_HEIGHT );

				// hlavièka MapSquaru
				SMapSquareHeader sMapSquareHeader;

				// vykreslí všechny MapSquary
				for ( DWORD dwMapSquareIndexY = 0; dwMapSquareIndexY < dwSizeY; 
					dwMapSquareIndexY++ )
				{
					for ( DWORD dwMapSquareIndexX = 0; dwMapSquareIndexX < dwSizeX; 
						dwMapSquareIndexX++ )
					{
						// naète hlavièku MapSquaru
						CHECK_MAP_FILE ( cMapFile.Read ( &sMapSquareHeader, sizeof ( sMapSquareHeader ) ) == sizeof ( sMapSquareHeader ) );

					// pøeskoèí Background Level Mapexy a Unit Level Mapexy

						// zjistí velikost Unit Level Mapexù
						DWORD dwMapexesSize = ( sMapSquareHeader.m_dwBL1MapexesNum + 
							sMapSquareHeader.m_dwBL2MapexesNum + 
							sMapSquareHeader.m_dwBL3MapexesNum ) * 
							sizeof ( SMapexInstanceHeader ) + 
							sMapSquareHeader.m_dwULMapexesNum * 
							sizeof ( SULMapexInstanceHeader );

						// pøeskoèí Background Level Mapexy a Unit Level Mapexy
						CHECK_MAP_FILE ( cMapFile.GetPosition () + dwMapexesSize <= 
							cMapFile.GetLength () );
						(void)cMapFile.Seek ( dwMapexesSize, CFile::current );
					}
				}
				// všechny MapSquary jsou pøeskoèeny
			}

		// pøeskoèí jednotky, suroviny, neviditelnosti a hlavièky pøedchozích grafù 
		//		pro hledání cesty
			{
				// zjistí velikost pøeskakovaných dat
				DWORD dwSkippedDataSize = dwUnitCount * sizeof ( SUnitHeader ) + 
					RESOURCE_COUNT * ( sizeof ( SResource ) + sizeof ( DWORD ) * 
					RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT ) + INVISIBILITY_COUNT * 
					sizeof ( SInvisibility ) + dwFindPathGraphIndex * 
					sizeof ( SFindPathGraphHeader );

				// pøeskoèí data
				CHECK_MAP_FILE ( cMapFile.GetPosition () + dwSkippedDataSize <= 
					cMapFile.GetLength () );
				(void)cMapFile.Seek ( dwSkippedDataSize, CFile::current );
			}

		// naète hlavièku vytváøeného grafu pro hledání cesty

			SFindPathGraphHeader sFindPathGraphHeader;

			// naète graf pro hledání cesty
			CHECK_MAP_FILE ( cMapFile.Read ( &sFindPathGraphHeader, sizeof ( sFindPathGraphHeader ) ) == sizeof ( sFindPathGraphHeader ) );

			// zjistí jméno grafu pro hledání cesty
			m_strFindPathGraphName = sFindPathGraphHeader.m_pName;

			// zjistí typy povrchu grafu pro hledání cesty
			memcpy ( aAllowedLandTypes.GetData (), sFindPathGraphHeader.m_aAllowedLandTypes, 
				sizeof ( sFindPathGraphHeader.m_aAllowedLandTypes ) );
// *******************************************
// *******************************************
// *******************************************
// *******************************************
// *******************************************
//			CHECK_MAP_FILE ( !aAllowedLandTypes.GetAt ( TRANSPARENT_LAND_TYPE_ID ) );
// *******************************************
// *******************************************
// *******************************************
// *******************************************
// *******************************************

			// zkontroluje povolené typy povrchu
			for ( int nLandTypeIndex = 256; nLandTypeIndex-- > 0; )
			{
				// zjistí, je-li typ povrchu "nLandTypeIndex" v poøádku
				if ( !aUsedLandTypes.GetAt ( nLandTypeIndex ) && 
					aAllowedLandTypes.GetAt ( nLandTypeIndex ) )
				{	// typ povrchu "nLandTypeIndex" není v poøádku
					aAllowedLandTypes.ClearAt ( nLandTypeIndex );
				}
			}

		// nastaví pùvodní pozici v souboru

			// pøesune se na zaèátek odkazù na knihovny Mapexù
			CHECK_MAP_FILE ( (DWORD)cMapFile.Seek ( dwMapexLibraryNodesPosition, 
				CFile::begin ) == dwMapexLibraryNodesPosition );
		}

	// naète odkazy na knihovny Mapexù
		{
			SMapexLibraryNode sMapexLibraryNode;

			// nechá naèíst všechny knihovny Mapexù
			for ( DWORD dwMapexLibraryIndex = dwMapexLibraryCount; dwMapexLibraryIndex-- > 0; )
			{
				// naète odkaz na knihovnu Mapexù
				CHECK_MAP_FILE ( cMapFile.Read ( &sMapexLibraryNode, sizeof ( sMapexLibraryNode ) ) == sizeof ( sMapexLibraryNode ) );

				// zkontroluje délku jména knihovny Mapexù
				int nMapexLibraryFileNameLength = strlen ( sMapexLibraryNode.m_pFileName );
				CHECK_MAP_FILE ( ( nMapexLibraryFileNameLength > 0 ) && 
					( nMapexLibraryFileNameLength < sizeof ( sMapexLibraryNode.m_pFileName ) ) );

				// otevøe archiv knihovny Mapexù
				CDataArchive cMapexLibraryArchive = cMapArchive.CreateArchive ( 
					sMapexLibraryNode.m_pFileName );

				// naète knihovnu Mapexù
				{
					// otevøe hlavièkový soubor knihovny
					CArchiveFile cMapexLibraryHeaderFile = cMapexLibraryArchive.CreateFile ( 
						_T("Library.Header"), CFile::modeRead | CFile::shareDenyWrite );

					// pøeète hlavièku verze knihovny Mapexù
					{
						SMapexFileVersionHeader sMapexLibraryVersionHeader;
						CHECK_MAP_FILE ( cMapexLibraryHeaderFile.Read ( &sMapexLibraryVersionHeader, sizeof ( sMapexLibraryVersionHeader ) ) == sizeof ( sMapexLibraryVersionHeader ) );

						// identifikátor souboru knihovny Mapexù
						BYTE aMapexLibraryFileID[16] = MAPEX_FILE_IDENTIFIER;
						ASSERT ( sizeof ( aMapexLibraryFileID ) == 
							sizeof ( sMapexLibraryVersionHeader.m_aFileID ) );

						// zkontroluje identifikátor souboru knihovny Mapexù
						for ( int nIndex = 16; nIndex-- > 0; )
						{
							CHECK_MAP_FILE ( aMapexLibraryFileID[nIndex] == 
								sMapexLibraryVersionHeader.m_aFileID[nIndex] );
						}

						ASSERT ( CURRENT_MAP_FILE_VERSION >= COMPATIBLE_MAP_FILE_VERSION );
						// zkontroluje èísla verzí formátu knihovny Mapexù
						CHECK_MAP_FILE ( sMapexLibraryVersionHeader.m_dwFormatVersion >= 
							sMapexLibraryVersionHeader.m_dwCompatibleFormatVersion );

						// zjistí, jedná-li se o správnou verzi formátu knihovny Mapexù
						if ( sMapexLibraryVersionHeader.m_dwFormatVersion != CURRENT_MAP_FILE_VERSION )
						{	// nejedná se o správnou verzi formátu knihovny Mapexù
							// zjistí, jedná-li se o starou verzi formátu knihovny Mapexù
							if ( sMapexLibraryVersionHeader.m_dwFormatVersion < CURRENT_MAP_FILE_VERSION )
							{	// jedná se o starou verzi formátu knihovny Mapexù
								// zkontroluje kompatabilitu verze formátu knihovny Mapexù
								CHECK_MAP_FILE ( sMapexLibraryVersionHeader.m_dwFormatVersion >= 
									COMPATIBLE_MAP_FILE_VERSION );
							}
							else
							{	// jedná se o mladší verzi formátu knihovny Mapexù
								// zkontroluje kompatabilitu verze formátu knihovny Mapexù
								CHECK_MAP_FILE ( sMapexLibraryVersionHeader.m_dwCompatibleFormatVersion <= 
									CURRENT_MAP_FILE_VERSION );
							}
						}
					}

					// pøeète hlavièku knihovny Mapexù
					SMapexLibraryHeader sMapexLibraryHeader;
					CHECK_MAP_FILE ( cMapexLibraryHeaderFile.Read ( &sMapexLibraryHeader, sizeof ( sMapexLibraryHeader ) ) == sizeof ( sMapexLibraryHeader ) );

					// naète jména Mapexových souborù
					for ( DWORD dwMapexLibraryIndex = sMapexLibraryHeader.m_dwMapexCount; 
						dwMapexLibraryIndex-- > 0; )
					{
						// pøeète jméno dalšího mapexového souboru
						char szMapexFileName[30];
						CHECK_MAP_FILE ( cMapexLibraryHeaderFile.Read ( &szMapexFileName, sizeof ( szMapexFileName ) ) == sizeof ( szMapexFileName ) );

						// zkontroluje délku jména Mapexového souboru
						int nMapexFileNameLength = strlen ( szMapexFileName );
						CHECK_MAP_FILE ( ( nMapexFileNameLength > 0 ) && 
							( nMapexFileNameLength < sizeof ( szMapexFileName ) ) );

						// otevøe Mapexový soubor
						CArchiveFile cMapexFile = cMapexLibraryArchive.CreateFile ( 
							szMapexFileName, CFile::modeRead | CFile::shareDenyWrite );

						// pøeète hlavièku Mapexového souboru
						SMapexHeader sMapexHeader;
						CHECK_MAP_FILE ( cMapexFile.Read ( &sMapexHeader, sizeof ( sMapexHeader ) ) == sizeof ( sMapexHeader ) );

						// ukazatel na typ Mapexu
						struct SMapexType *pMapexType;

						// zkontroluje unikátnost ID typu Mapexu
						CHECK_MAP_FILE ( !cMapexTypeTable.Lookup ( sMapexHeader.m_dwID, (void *&)pMapexType ) );
						// zkontroluje rozmìry typu Mapexu
						CHECK_MAP_FILE ( sMapexHeader.m_dwXSize <= MAPSQUARE_WIDTH );
						CHECK_MAP_FILE ( sMapexHeader.m_dwYSize <= MAPSQUARE_HEIGHT );

						// zjistí velikost typù povrchu typu Mapexu
						DWORD dwMapexTypeLandTypesSize = sMapexHeader.m_dwXSize * 
							sMapexHeader.m_dwYSize;

						// alokuje pamì pro další typ Mapexu
						pMapexType = (struct SMapexType *)new char[sizeof ( SMapexType ) + 
							dwMapexTypeLandTypesSize];

						// pøidá typ Mapexu do tabulky typù Mapexù
						cMapexTypeTable.SetAt ( sMapexHeader.m_dwID, pMapexType );

						// vyplní typ Mapexu
						pMapexType->dwSizeX = sMapexHeader.m_dwXSize;
						pMapexType->dwSizeY = sMapexHeader.m_dwYSize;

						// naète typy povrchu typu Mapexu
						CHECK_MAP_FILE ( cMapexFile.Read ( pMapexType + 1, dwMapexTypeLandTypesSize ) == dwMapexTypeLandTypesSize );

						// ukazatel na typy povrchu typu Mapexu
						signed char *aMapexTypeLandTypes = (signed char *)(pMapexType + 1);

						// pøepíše typy povrchu typu Mapexu podle povolených typù povrchu
						for ( DWORD dwIndex = dwMapexTypeLandTypesSize; dwIndex-- > 0; )
						{
							// zjistí, jedná-li se o prùhledný typ povrchu
							if ( aMapexTypeLandTypes[dwIndex] == TRANSPARENT_LAND_TYPE_ID )
							{	// jedná se o prùhledný typ povrchu
								aMapexTypeLandTypes[dwIndex] = MAPCELL_UNINITIALIZED;
							}
							else
							{	// nejedná se o prùhledný typ povrchu
								// pøepíše typ povrchu typu Mapexu podle povolených typù povrchu
								aMapexTypeLandTypes[dwIndex] = aAllowedLandTypes.GetAt ( 
									aMapexTypeLandTypes[dwIndex] ) ? MAPCELL_ALLOWED : 
									MAPCELL_DISALLOWED;
							}
						}

						// zkontroluje konec Mapexového souboru
						CHECK_MAP_FILE ( cMapexFile.GetPosition () == cMapexFile.GetLength () );

						// zavøe Mapexový soubor
						cMapexFile.Close ();
					}

					// zkontroluje konec hlavièkového souboru knihovny
					CHECK_MAP_FILE ( cMapexLibraryHeaderFile.GetPosition () == 
						cMapexLibraryHeaderFile.GetLength () );

					// zavøe hlavièkový soubor knihovny
					cMapexLibraryHeaderFile.Close ();
				}
				// knihovna mapexù je naètena

				// zavøe arhiv knihovny Mapexù
				cMapexLibraryArchive.Close ();
			}
			// všechny knihovny Mapexù jsou naèteny
		}

	// pøeskoèí hlavièky civilizací, odkazy na knihovny typù jednotek a pole offsetù 
	//		MapSquarù
		{
			// zjistí velikost pøeskakovaných dat
			DWORD dwSkippedDataSize = dwCivilizationCount * sizeof ( SCivilizationHeader ) + 
				dwUnitTypeLibraryCount * sizeof ( SUnitTypeLibraryNode ) + ( m_nSizeX / 
				MAPSQUARE_WIDTH ) * ( m_nSizeY / MAPSQUARE_HEIGHT ) * sizeof ( DWORD );

			// pøeskoèí data
			CHECK_MAP_FILE ( cMapFile.GetPosition () + dwSkippedDataSize <= 
				cMapFile.GetLength () );
			(void)cMapFile.Seek ( dwSkippedDataSize, CFile::current );
		}

	// naète informace o MapSquarech
		{
			// rozmìry mapy v MapSquarech
			DWORD dwSizeX = (DWORD)( m_nSizeX / MAPSQUARE_WIDTH );
			DWORD dwSizeY = (DWORD)( m_nSizeY / MAPSQUARE_HEIGHT );

			// hlavièka MapSquaru
			SMapSquareHeader sMapSquareHeader;
			// hlavièka instance Mapexu Background Levelu
			SMapexInstanceHeader sBLMapexInstanceHeader;

			// ukazatel na typ Mapexu
			struct SMapexType *pMapexType;

			// pozice MapSquarù v souboru mapy
			DWORD dwMapSquaresPosition = cMapFile.GetPosition ();

			// vykreslí všechny Background Levely
			for ( int nBLIndex = 0; nBLIndex < 3; nBLIndex++ )
			{
				// pøesune se na zaèátek MapSquarù v souboru mapy
				CHECK_MAP_FILE ( (DWORD)cMapFile.Seek ( dwMapSquaresPosition, 
					CFile::begin ) == dwMapSquaresPosition );

				// vykreslí všechny MapSquary
				for ( DWORD dwMapSquareIndexY = 0; dwMapSquareIndexY < dwSizeY; 
					dwMapSquareIndexY++ )
				{
					// pozice MapSquaru
					DWORD dwPositionY = dwMapSquareIndexY * MAPSQUARE_HEIGHT;

					for ( DWORD dwMapSquareIndexX = 0; dwMapSquareIndexX < dwSizeX; 
						dwMapSquareIndexX++ )
					{
						// pozice MapSquaru
						DWORD dwPositionX = dwMapSquareIndexX * MAPSQUARE_WIDTH;

						// naète hlavièku MapSquaru
						CHECK_MAP_FILE ( cMapFile.Read ( &sMapSquareHeader, sizeof ( sMapSquareHeader ) ) == sizeof ( sMapSquareHeader ) );

						// pole poètù Mapexù Background Levelù MapSquaru
						DWORD aBLMapexCount[3] = { sMapSquareHeader.m_dwBL1MapexesNum, 
							sMapSquareHeader.m_dwBL2MapexesNum, 
							sMapSquareHeader.m_dwBL3MapexesNum };

						// pøeète Background Level Mapexy
						for ( int nDrawBLIndex = 0; nDrawBLIndex < 3; nDrawBLIndex++ )
						{
							// zjistí, jedná-li se o vykreslované Background Level Mapexy
							if ( nDrawBLIndex == nBLIndex )
							{	// jedná se o vykreslované Background Level Mapexy
								// naète Background Level Mapexy
								for ( int nBLMapexIndex = aBLMapexCount[nBLIndex]; 
									nBLMapexIndex-- > 0; )
								{
									// naète hlavièku instance Background Level Mapexu
									CHECK_MAP_FILE ( cMapFile.Read ( &sBLMapexInstanceHeader, sizeof ( sBLMapexInstanceHeader ) ) == sizeof ( sBLMapexInstanceHeader ) );

									// zkontroluje umístìní Mapexu v MapSquaru
									CHECK_MAP_FILE ( dwPositionX <= sBLMapexInstanceHeader.m_dwX );
									CHECK_MAP_FILE ( dwPositionX + MAPSQUARE_WIDTH > sBLMapexInstanceHeader.m_dwX );
									CHECK_MAP_FILE ( dwPositionY <= sBLMapexInstanceHeader.m_dwY );
									CHECK_MAP_FILE ( dwPositionY + MAPSQUARE_HEIGHT > sBLMapexInstanceHeader.m_dwY );
									// získá ukazatel na typ Mapexu podle ID Mapexu
									CHECK_MAP_FILE ( cMapexTypeTable.Lookup ( 
										sBLMapexInstanceHeader.m_dwMapexID, (void *&)pMapexType ) );

									// ukazatel na typy povrchu Mapexu
									signed char *pMapexTypeLandTypes = (signed char *)( pMapexType + 1 );

									// zjistí, zasahuje-li Mapex mimo mapu
									if ( ( sBLMapexInstanceHeader.m_dwX + pMapexType->dwSizeX <= 
										(DWORD)m_nSizeX ) && ( sBLMapexInstanceHeader.m_dwY + 
										pMapexType->dwSizeY <= (DWORD)m_nSizeY ) )
									{	// Mapex nezasahuje mimo mapu
										// ukazatel na roh umístìného Mapexu
										signed char *pCorner = GetAt ( 
											MAP_BORDER + (int)sBLMapexInstanceHeader.m_dwX, 
											MAP_BORDER + (int)sBLMapexInstanceHeader.m_dwY );

										// nakreslí Mapex do mapy
										for ( DWORD dwIndexY = pMapexType->dwSizeY; dwIndexY-- > 0; )
										{
											for ( DWORD dwIndexX = pMapexType->dwSizeX; dwIndexX-- > 0; )
											{
												// zjistí, jedná-li se o prùhledný typ povrchu
												if ( pMapexTypeLandTypes[dwIndexX] != 
													MAPCELL_UNINITIALIZED )
												{	// nejedná se o prùhledný typ povrchu
													pCorner[dwIndexX] = pMapexTypeLandTypes[dwIndexX];
												}
											}
											// nechá vykreslit další øádek Mapexu
											pCorner += m_nMapSizeX;
											pMapexTypeLandTypes += pMapexType->dwSizeX;
										}
									}
									else
									{	// Mapex zasahuje mimo mapu
										// ukazatel na zaèátek prvního øádku s vykreslovaným Mapexem
										signed char *pCorner = GetAt ( MAP_BORDER, 
											MAP_BORDER + (int)sBLMapexInstanceHeader.m_dwY );

										// nakreslí Mapex do mapy
										for ( DWORD dwIndexY = sBLMapexInstanceHeader.m_dwY; 
											dwIndexY < pMapexType->dwSizeY; dwIndexY++ )
										{
											// index v Mapexu
											DWORD dwMapexIndexX = 0;
											for ( DWORD dwIndexX = sBLMapexInstanceHeader.m_dwX; 
												dwIndexX < pMapexType->dwSizeX; dwIndexX++, dwMapexIndexX++ )
											{
												// zjistí, jedná-li se o prùhledný typ povrchu
												if ( pMapexTypeLandTypes[dwMapexIndexX] != 
													MAPCELL_UNINITIALIZED )
												{	// nejedná se o prùhledný typ povrchu
													pCorner[dwIndexX] = pMapexTypeLandTypes[dwMapexIndexX];
												}
											}
											// nechá vykreslit další øádek Mapexu
											pCorner += m_nMapSizeX;
											pMapexTypeLandTypes += pMapexType->dwSizeX;
										}
									}
								}
								// naèetl Background Level Mapexy
							}
							else
							{	// nejedná se o vykreslované Background Level Mapexy
								// zjistí velikost Mapexù Background Levelu "nDrawBLIndex"
								DWORD dwBLMapexesSize = aBLMapexCount[nDrawBLIndex] * 
									sizeof ( SMapexInstanceHeader );

								// pøeskoèí Mapexy Background Levelu "nDrawBLIndex"
								CHECK_MAP_FILE ( cMapFile.GetPosition () + dwBLMapexesSize <= 
									cMapFile.GetLength () );
								(void)cMapFile.Seek ( dwBLMapexesSize, CFile::current );
							}
						}
						// Background Level Mapexy jsou pøeèteny

					// pøeskoèí Unit Level Mapexy

						// zjistí velikost Unit Level Mapexù
						DWORD dwULMapexesSize = sMapSquareHeader.m_dwULMapexesNum * 
							sizeof ( SULMapexInstanceHeader );

						// pøeskoèí Unit Level Mapexy
						CHECK_MAP_FILE ( cMapFile.GetPosition () + dwULMapexesSize <= 
							cMapFile.GetLength () );
						(void)cMapFile.Seek ( dwULMapexesSize, CFile::current );
					}
				}
				// všechny MapSquary jsou nakresleny
			}
			// všechny Background Levely jsou vykresleny
		}

	// pøeskoèí jednotky, suroviny, neviditelnosti a grafy pro hledání cesty
		{
			// zjistí velikost pøeskakovaných dat
			DWORD dwSkippedDataSize = dwUnitCount * sizeof ( SUnitHeader ) + 
				RESOURCE_COUNT * ( sizeof ( SResource ) + sizeof ( DWORD ) * 
				RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT ) + INVISIBILITY_COUNT * 
				sizeof ( SInvisibility ) + dwFindPathGraphCount * 
				sizeof ( SFindPathGraphHeader );

			// pøeskoèí data
			CHECK_MAP_FILE ( cMapFile.GetPosition () + dwSkippedDataSize <= 
				cMapFile.GetLength () );
			(void)cMapFile.Seek ( dwSkippedDataSize, CFile::current );
		}

	// ukonèí ètení souboru mapy

		// zkontroluje délku souboru mapy
		CHECK_MAP_FILE ( cMapFile.GetPosition () == cMapFile.GetLength () );
		// zavøe soubor mapy
		cMapFile.Close ();
	}
	catch ( CCFPGMapFileException *pException )
	{
		// znièí výjimku
		pException->Delete ();
		// vrátí pøíznak chybného souboru mapy
		eReturnValue = pException->m_eError;
	}
	catch ( CDataArchiveException *pException )
	{
		// znièí výjimku
		pException->Delete ();
		// vrátí pøíznak chybného souboru mapy
		eReturnValue = ECFPGE_BadMapFile;
	}
	catch ( CMemoryException *pException )
	{
		// znièí výjimku
		pException->Delete ();
		// vrátí pøíznak nedostatku pamìti
		eReturnValue = ECFPGE_NotEnoughMemory;
	}
	catch ( CException *pException )
	{
		// znièí výjimku
		pException->Delete ();
		// vrátí pøíznak neznámé chyby
		eReturnValue = ECFPGE_UnknownError;
	}

	// znièí typy Mapexù
	{
		// pozice prvního typu Mapexu
		POSITION posMapexType = cMapexTypeTable.GetStartPosition ();
		// ID typu Mapexu
		DWORD dwMapexTypeID;
		// ukazatel na typ Mapexu
		struct SMapexType *pMapexType;

		// projede všechny typy Mapexu
		while ( posMapexType != NULL )
		{
			// získá další typ Mapexu
			cMapexTypeTable.GetNextAssoc ( posMapexType, dwMapexTypeID, (void *&)pMapexType );

			// znièí typ Mapexu
			delete [] (char *)pMapexType;
		}
	
		// znièí tabulku typù Mapexù
		cMapexTypeTable.RemoveAll ();
	}

	// zjistí, došlo-li k chybì
	if ( eReturnValue != ECFPGE_OK )
	{	// došlo k chybì
		// zjistí, podaøilo-li se vytvoøit nakreslenou mapu
		if ( m_pMap != NULL )
		{	// podaøilo se vytvoøit nakreslenou mapu
			// znièí nakreslenou mapu
			delete [] m_pMap;
			// zneškodní nakreslenou mapu
			m_pMap = NULL;
		}
	}
	else
	{	// nedošlo k chybì

#ifdef _DEBUG
		// zkontroluje horní okraj mapy
		signed char *pLine = m_pMap;
		for ( DWORD dwIndex = m_nMapSizeX * MAP_BORDER + MAP_BORDER; dwIndex-- > 0; )
		{
			if ( *(pLine++) != MAPCELL_DISALLOWED )
			{
				ASSERT ( FALSE );
			}
		}

		// zkontroluje levý a pravý okraj mapy
		for ( DWORD dwIndexY = m_nSizeY; dwIndexY-- > 0; )
		{
			pLine += m_nSizeX;
			// zkontroluje levý a pravý okraj øádky
			for ( DWORD dwIndexX = 2 * MAP_BORDER; dwIndexX-- > 0; )
			{
				if ( *(pLine++) != MAPCELL_DISALLOWED )
				{
					ASSERT ( FALSE );
				}
			}
		}

		// zkontroluje dolní okraj mapy
		for ( dwIndex = m_nMapSizeX * MAP_BORDER - MAP_BORDER; dwIndex-- > 0; )
		{
			if ( *(pLine++) != MAPCELL_DISALLOWED )
			{
				ASSERT ( FALSE );
			}
		}
#endif //_DEBUG

		// zjistí, má-li se kontrolovat povrch mapy
		if ( bCheckCompleteLand )
		{	// má se kontrolovat povrch mapy
			// ukazatel na øádek
			signed char *pLine = GetAt ( MAP_BORDER, MAP_BORDER ) - 1;

			// zkontroluje øádky mapy
			for ( DWORD dwIndexY = m_nSizeY; dwIndexY-- > 0; )
			{
				// zkontroluje øádku mapy
				for ( DWORD dwIndexX = m_nSizeX; dwIndexX-- > 0; )
				{
					// zjistí, je-li MapCell neinicializovaný
					if ( *(++pLine) == MAPCELL_UNINITIALIZED )
					{	// MapCell je neinicializovaný
						// pøepíše MapCell na zakázaný
						*pLine = MAPCELL_DISALLOWED;
						// nastaví pøíznak chybného povrchu mapy
						eReturnValue = ECFPGE_IncompleteMapLand;
					}
				}
				// nechá zkontrolovat další øádku mapy
				pLine += 2 * MAP_BORDER;
			}
		}
		// nemá se kontrolovat povrch mapy
	}
	// pøípadná chyba byla obsloužena

	// vrátí návratovou hodnotu
	return eReturnValue;
}

// znièí mapu
void CCFPGMap::Delete () 
{
#ifndef __SHARED_MEMORY__
	// zjistí, je-li mapa platná
	if ( m_pMap != NULL )
	{	// mapa je platná
		// znièí nakreslenou mapu
		delete [] m_pMap;
		// zneškodní nakreslenou mapu
		m_pMap = NULL;
	}
#else //__SHARED_MEMORY__
	ASSERT ( m_pMapDescription != NULL );
	ASSERT ( m_hMapDescription != NULL );
	ASSERT ( m_hMap != NULL );

	VERIFY ( UnmapViewOfFile ( m_pMapDescription ) );
	m_pMapDescription = NULL;
	VERIFY ( UnmapViewOfFile ( m_pMap ) );
	m_pMap = NULL;
	CloseHandle ( m_hMap );
	m_hMap = NULL;
	CloseHandle ( m_hMapDescription );
	m_hMapDescription = NULL;
#endif //__SHARED_MEMORY__
}

// vytvoøí graf pro hledání cesty v archivu "cFindPathGraphsArchive"
void CCFPGMap::CreateFindPathGraph ( CDataArchive cFindPathGraphsArchive ) 
{
// vytvoøí tabulku hintù cest
	ASSERT ( ( m_pPathHintTable == NULL ) && ( m_dwAllocatedPathHintTableSize == 0 ) && 
		( m_dwPathHintTableSize == 0 ) );
	if ( ( m_pPathHintTable = (DWORD *)malloc ( ( m_dwAllocatedPathHintTableSize = 100 ) * 
		sizeof ( DWORD ) ) ) == NULL )
	{
		AfxThrowMemoryException ();
	}
	m_dwPathHintTableSize = 0;

// pokryje mapu kruhy
	{
		// identifikátor MapCellu støedu starého kruhu
		signed char cOldMapCellID = GetMapCellIDFromCircleSize ( 1 );
		// zpracuje všechny velikosti kruhù
		for ( int nCircleSize = 2; nCircleSize <= MAX_CIRCLE_SIZE; 
			nCircleSize++ )
		{	// pokryje mapu kruhy velikosti "nCircleSize"
			// posun kruhu vùèi jeho støedu
			int nCircleOffset = ( nCircleSize - 1 ) / 2;
			// ukazatel na støed kruhu v mapì
			signed char *pMap = GetAt ( MAP_BORDER + 1 + nCircleOffset, 
				MAP_BORDER + 1 + nCircleOffset );
			// ukazatel na originální kruh
			signed char *pOriginalCircle = m_aCircles[nCircleSize-2];
			// identifikátor MapCellu støedu nového kruhu
			signed char cNewMapCellID = GetMapCellIDFromCircleSize ( nCircleSize );

			// zpracuje mapu
			for ( int nPositionY = m_nSizeY - nCircleSize; nPositionY-- > 0; 
				pMap += 2 * MAP_BORDER + nCircleSize )
			{
				for ( int nPositionX = m_nSizeX - nCircleSize; nPositionX-- > 0; pMap++ )
				{
					// zjistí, jedná-li se o potenciální MapCell støedu kruhu
					if ( *pMap == cOldMapCellID )
					{	// jedná se o potenciální støed kruhu
						// ukazatel na kruh v mapì
						signed char *pMapCircle = pMap - nCircleOffset * m_nMapSizeX - 
							nCircleOffset;
						// ukazatel na kruh
						signed char *pCircle = pOriginalCircle;
						// pøíznak umístìní kruhu na mapì
						BOOL bCirclePlaced = TRUE;

						// zkontroluje umístìní kruhu
						for ( int nIndex = nCircleSize * nCircleSize; nIndex-- > 0; 
							pMapCircle++, pCircle++ )
						{
							// zjistí, lze-li kruh umístit
							if ( ( *pCircle == 1 ) && ( *pMapCircle == 0 ) )
							{	// kruh nelze umístit
								// nastaví pøíznak neumístìní kruhu
								bCirclePlaced = FALSE;
								break;
							}

							// zjistí, má-li se pøejít na další øádek
							if ( nIndex % nCircleSize == 0 )
							{	// má se pøejít na další øádek
								// posune se na mapì na další øádek
								pMapCircle += m_nMapSizeX - nCircleSize;
							}
						}

						// zjistí, lze-li kruh umístit na mapu
						if ( bCirclePlaced )
						{	// kruh lze umístit na mapu
							// umístí MapCell støedu kruhu na mapu
							*pMap = cNewMapCellID;
						}
					}
					// nejedná se o potenciální støed kruhu
				}
			}
			// mapa je zpracována

			// aktualizuje identifiátor MapCellu støedu starého kruhu
			cOldMapCellID = cNewMapCellID;
		}
		// všechny velikosti kruhù jsou zpracovány
	}
	// mapa je pokryta kruhy

// najde oblasti nejvìtších kruhù
	{
		// omezení umístìní nejvìtšího kruhu
		int nMaxPositionX = m_nMapXMax - ( MAX_CIRCLE_SIZE / 2 );
		int nMaxPositionY = m_nMapYMax - ( MAX_CIRCLE_SIZE / 2 );
		int nStartPosition = MAP_BORDER + ( ( MAX_CIRCLE_SIZE + 1 ) / 2 );

		// ukazatel do mapy
		signed char *pMap = GetAt ( nStartPosition, nStartPosition );

		// zpracuje mapu
		for ( int nPositionY = nStartPosition; nPositionY < nMaxPositionY; nPositionY++, 
			pMap += 2 * MAP_BORDER + MAX_CIRCLE_SIZE )
		{
			for ( int nPositionX = nStartPosition; nPositionX < nMaxPositionX; 
				nPositionX++, pMap++ )
			{
				// zjistí, jedná-li se o MapCell s nejvìtším kruhem
				if ( *pMap == GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ) )
				{	// jedná se o MapCell s nejvìtším kruhem
					// nechá oznaèit oblast nejvìtšího kruhu
					RepaintArea ( GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ), 
						MARK_EXPLORED ( GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ) ), 
						nPositionX, nPositionY );
					ASSERT ( *pMap == MARK_EXPLORED ( GetMapCellIDFromCircleSize ( 
						MAX_CIRCLE_SIZE ) ) );
					// pøidá oblast do seznamu oblastí/mostù
					m_cAreaBridgeList.Add ( new ( m_cAreaBridgePool ) SAreaBridge 
						( CPoint ( nPositionX, nPositionY ), GetMapCellIDFromCircleSize ( 
						MAX_CIRCLE_SIZE ) ) );
				}
			}
		}
		// mapa je zpracována
	}
	// jsou nalezeny oblasti nejvìtších kruhù

// zruší oznaèení oblastí nejvìtších kruhù
	{
		// ukazatel na oblast
		struct SAreaBridge *pArea;
		// pozice oblasti v seznamu oblastí/mostù
		POSITION posArea = m_cAreaBridgeList.GetHeadPosition ();

		// projede všechny oblasti nejvìtších kruhù
		while ( m_cAreaBridgeList.GetNext ( posArea, pArea ) )
		{
			ASSERT ( pArea->cMapCellID == GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ) );
			// zruší oznaèení oblasti nejvìtšího kruhu
			RepaintArea ( MARK_EXPLORED ( GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ) ), 
				GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ), pArea->pointPosition.x, 
				pArea->pointPosition.y );
			ASSERT ( *GetAt ( pArea->pointPosition.x, pArea->pointPosition.y ) == 
				GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ) );
			// oznaèí MapCell za reprezentanta oblasti
			*GetAt ( pArea->pointPosition.x, pArea->pointPosition.y ) = 
				GetMapCellAreaIDFromCircleSize ( MAX_CIRCLE_SIZE );
		}
	}
	// jsou zrušena oznaèení oblastí nejvìtších kruhù

// najde brány mostù a fiktivní oblasti
	{
		// inicializuje offsety okolních MapCellù
		m_aSurroundingMapCellOffset[0] = -m_nMapSizeX - 1;
		m_aSurroundingMapCellOffset[1] = -m_nMapSizeX;
		m_aSurroundingMapCellOffset[2] = -m_nMapSizeX + 1;
		m_aSurroundingMapCellOffset[3] = 1;
		m_aSurroundingMapCellOffset[4] = m_nMapSizeX + 1;
		m_aSurroundingMapCellOffset[5] = m_nMapSizeX;
		m_aSurroundingMapCellOffset[6] = m_nMapSizeX - 1;
		m_aSurroundingMapCellOffset[7] = -1;

		// inicializuje offsety souvislých MapCellù
		m_aContinuousMapCellOffset[0][0] = 0;						// * o .
		m_aContinuousMapCellOffset[0][1] = -1;						// o - .
		m_aContinuousMapCellOffset[0][2] = -m_nMapSizeX;		// . . .
		m_aContinuousMapCellOffset[0][3] = 0;
		m_aContinuousMapCellOffset[1][0] = -1;						// o * o
		m_aContinuousMapCellOffset[1][1] = -m_nMapSizeX - 1;	// o - o
		m_aContinuousMapCellOffset[1][2] = -m_nMapSizeX + 1;	// . . .
		m_aContinuousMapCellOffset[1][3] = 1;
		m_aContinuousMapCellOffset[2][0] = 0;						// . o *
		m_aContinuousMapCellOffset[2][1] = -m_nMapSizeX;		// . - o
		m_aContinuousMapCellOffset[2][2] = 1;						// . . .
		m_aContinuousMapCellOffset[2][3] = 0;
		m_aContinuousMapCellOffset[3][0] = -m_nMapSizeX;		// . o o
		m_aContinuousMapCellOffset[3][1] = -m_nMapSizeX + 1;	// . - *
		m_aContinuousMapCellOffset[3][2] = m_nMapSizeX + 1;	// . o o
		m_aContinuousMapCellOffset[3][3] = m_nMapSizeX;
		m_aContinuousMapCellOffset[4][0] = 0;						// . . .
		m_aContinuousMapCellOffset[4][1] = 1;						// . - o
		m_aContinuousMapCellOffset[4][2] = m_nMapSizeX;			// . o *
		m_aContinuousMapCellOffset[4][3] = 0;
		m_aContinuousMapCellOffset[5][0] = 1;						// . . .
		m_aContinuousMapCellOffset[5][1] = m_nMapSizeX + 1;	// o - o
		m_aContinuousMapCellOffset[5][2] = m_nMapSizeX - 1;	// o * o
		m_aContinuousMapCellOffset[5][3] = -1;
		m_aContinuousMapCellOffset[6][0] = 0;						// . . .
		m_aContinuousMapCellOffset[6][1] = m_nMapSizeX;			// o - .
		m_aContinuousMapCellOffset[6][2] = -1;						// * o .
		m_aContinuousMapCellOffset[6][3] = 0;
		m_aContinuousMapCellOffset[7][0] = m_nMapSizeX;			// o o .
		m_aContinuousMapCellOffset[7][1] = m_nMapSizeX - 1;	// * - .
		m_aContinuousMapCellOffset[7][2] = -m_nMapSizeX - 1;	// o o .
		m_aContinuousMapCellOffset[7][3] = -m_nMapSizeX;

		// inicializuje offsety vzdálených okolních MapCellù
		m_aFarSurroundingMapCellOffset[0] = -2 * m_nMapSizeX - 2;
		m_aFarSurroundingMapCellOffset[1] = -2 * m_nMapSizeX - 1;
		m_aFarSurroundingMapCellOffset[2] = -2 * m_nMapSizeX;
		m_aFarSurroundingMapCellOffset[3] = -2 * m_nMapSizeX + 1;
		m_aFarSurroundingMapCellOffset[4] = -2 * m_nMapSizeX + 2;
		m_aFarSurroundingMapCellOffset[5] = -m_nMapSizeX + 2;
		m_aFarSurroundingMapCellOffset[6] = 2;
		m_aFarSurroundingMapCellOffset[7] = m_nMapSizeX + 2;
		m_aFarSurroundingMapCellOffset[8] = 2 * m_nMapSizeX + 2;
		m_aFarSurroundingMapCellOffset[9] = 2 * m_nMapSizeX + 1;
		m_aFarSurroundingMapCellOffset[10] = 2 * m_nMapSizeX;
		m_aFarSurroundingMapCellOffset[11] = 2 * m_nMapSizeX - 1;
		m_aFarSurroundingMapCellOffset[12] = 2 * m_nMapSizeX - 2;
		m_aFarSurroundingMapCellOffset[13] = m_nMapSizeX - 2;
		m_aFarSurroundingMapCellOffset[14] = -2;
		m_aFarSurroundingMapCellOffset[15] = -m_nMapSizeX - 2;

		// inicializuje offsety vzdálených souvislých MapCellù
		m_aFarContinuousMapCellOffset[0][0] = -2;							// o o o . .
		m_aFarContinuousMapCellOffset[0][1] = -m_nMapSizeX - 2;		// o * . . .
		m_aFarContinuousMapCellOffset[0][2] = -2 * m_nMapSizeX - 2;	// o . - . .
		m_aFarContinuousMapCellOffset[0][3] = -2 * m_nMapSizeX - 1;	// . . . . .
		m_aFarContinuousMapCellOffset[0][4] = -2 * m_nMapSizeX;		// . . . . .
		m_aFarContinuousMapCellOffset[1][0] = 0;							// . o o o .
		m_aFarContinuousMapCellOffset[1][1] = -2 * m_nMapSizeX - 1;	// . . * . .
		m_aFarContinuousMapCellOffset[1][2] = -2 * m_nMapSizeX;		// . . - . .
		m_aFarContinuousMapCellOffset[1][3] = -2 * m_nMapSizeX + 1;	// . . . . .
		m_aFarContinuousMapCellOffset[1][4] = 0;							// . . . . .
		m_aFarContinuousMapCellOffset[2][0] = -2 * m_nMapSizeX;		// . . o o o
		m_aFarContinuousMapCellOffset[2][1] = -2 * m_nMapSizeX + 1;	// . . . * o
		m_aFarContinuousMapCellOffset[2][2] = -2 * m_nMapSizeX + 2;	// . . - . o
		m_aFarContinuousMapCellOffset[2][3] = -m_nMapSizeX + 2;		// . . . . .
		m_aFarContinuousMapCellOffset[2][4] = 2;							// . . . . .
		m_aFarContinuousMapCellOffset[3][0] = 0;							// . . . . .
		m_aFarContinuousMapCellOffset[3][1] = -m_nMapSizeX + 2;		// . . . . o
		m_aFarContinuousMapCellOffset[3][2] = 2;							// . . - * o
		m_aFarContinuousMapCellOffset[3][3] = m_nMapSizeX + 2;		// . . . . o
		m_aFarContinuousMapCellOffset[3][4] = 0;							// . . . . .
		m_aFarContinuousMapCellOffset[4][0] = 2;							// . . . . .
		m_aFarContinuousMapCellOffset[4][1] = m_nMapSizeX + 2;		// . . . . .
		m_aFarContinuousMapCellOffset[4][2] = 2 * m_nMapSizeX + 2;	// . . - . o
		m_aFarContinuousMapCellOffset[4][3] = 2 * m_nMapSizeX + 1;	// . . . * o
		m_aFarContinuousMapCellOffset[4][4] = 2 * m_nMapSizeX;		// . . o o o
		m_aFarContinuousMapCellOffset[5][0] = 0;							// . . . . .
		m_aFarContinuousMapCellOffset[5][1] = 2 * m_nMapSizeX + 1;	// . . . . .
		m_aFarContinuousMapCellOffset[5][2] = 2 * m_nMapSizeX;		// . . - . .
		m_aFarContinuousMapCellOffset[5][3] = 2 * m_nMapSizeX - 1;	// . . * . .
		m_aFarContinuousMapCellOffset[5][4] = 0;							// . o o o .
		m_aFarContinuousMapCellOffset[6][0] = 2 * m_nMapSizeX;		// . . . . .
		m_aFarContinuousMapCellOffset[6][1] = 2 * m_nMapSizeX - 1;	// . . . . .
		m_aFarContinuousMapCellOffset[6][2] = 2 * m_nMapSizeX - 2;	// o . - . .
		m_aFarContinuousMapCellOffset[6][3] = m_nMapSizeX - 2;		// o * . . .
		m_aFarContinuousMapCellOffset[6][4] = -2;							// o o o . .
		m_aFarContinuousMapCellOffset[7][0] = 0;							// . . . . .
		m_aFarContinuousMapCellOffset[7][1] = m_nMapSizeX - 2;		// o . . . .
		m_aFarContinuousMapCellOffset[7][2] = -2;							// o * - . .
		m_aFarContinuousMapCellOffset[7][3] = -m_nMapSizeX - 2;		// o . . . .
		m_aFarContinuousMapCellOffset[7][4] = 0;							// . . . . .

		// inicializuje offsety souvislých MapCellù
		m_aSurroundingFarMapCellOffset[0][0] = 0;
		m_aSurroundingFarMapCellOffset[0][1] = -m_nMapSizeX - 1;
		m_aSurroundingFarMapCellOffset[0][2] = 0;
		m_aSurroundingFarMapCellOffset[1][0] = 0;
		m_aSurroundingFarMapCellOffset[1][1] = -m_nMapSizeX - 1;
		m_aSurroundingFarMapCellOffset[1][2] = -m_nMapSizeX;
		m_aSurroundingFarMapCellOffset[2][0] = -m_nMapSizeX - 1;
		m_aSurroundingFarMapCellOffset[2][1] = -m_nMapSizeX;
		m_aSurroundingFarMapCellOffset[2][2] = -m_nMapSizeX + 1;
		m_aSurroundingFarMapCellOffset[3][0] = -m_nMapSizeX;
		m_aSurroundingFarMapCellOffset[3][1] = -m_nMapSizeX + 1;
		m_aSurroundingFarMapCellOffset[3][2] = 0;
		m_aSurroundingFarMapCellOffset[4][0] = 0;
		m_aSurroundingFarMapCellOffset[4][1] = -m_nMapSizeX + 1;
		m_aSurroundingFarMapCellOffset[4][2] = 0;
		m_aSurroundingFarMapCellOffset[5][0] = 0;
		m_aSurroundingFarMapCellOffset[5][1] = -m_nMapSizeX + 1;
		m_aSurroundingFarMapCellOffset[5][2] = 1;
		m_aSurroundingFarMapCellOffset[6][0] = -m_nMapSizeX + 1;
		m_aSurroundingFarMapCellOffset[6][1] = 1;
		m_aSurroundingFarMapCellOffset[6][2] = m_nMapSizeX + 1;
		m_aSurroundingFarMapCellOffset[7][0] = 1;
		m_aSurroundingFarMapCellOffset[7][1] = m_nMapSizeX + 1;
		m_aSurroundingFarMapCellOffset[7][2] = 0;
		m_aSurroundingFarMapCellOffset[8][0] = 0;
		m_aSurroundingFarMapCellOffset[8][1] = m_nMapSizeX + 1;
		m_aSurroundingFarMapCellOffset[8][2] = 0;
		m_aSurroundingFarMapCellOffset[9][0] = 0;
		m_aSurroundingFarMapCellOffset[9][1] = m_nMapSizeX + 1;
		m_aSurroundingFarMapCellOffset[9][2] = m_nMapSizeX;
		m_aSurroundingFarMapCellOffset[10][0] = m_nMapSizeX + 1;
		m_aSurroundingFarMapCellOffset[10][1] = m_nMapSizeX;
		m_aSurroundingFarMapCellOffset[10][2] = m_nMapSizeX - 1;
		m_aSurroundingFarMapCellOffset[11][0] = m_nMapSizeX;
		m_aSurroundingFarMapCellOffset[11][1] = m_nMapSizeX - 1;
		m_aSurroundingFarMapCellOffset[11][2] = 0;
		m_aSurroundingFarMapCellOffset[12][0] = 0;
		m_aSurroundingFarMapCellOffset[12][1] = m_nMapSizeX - 1;
		m_aSurroundingFarMapCellOffset[12][2] = 0;
		m_aSurroundingFarMapCellOffset[13][0] = 0;
		m_aSurroundingFarMapCellOffset[13][1] = m_nMapSizeX - 1;
		m_aSurroundingFarMapCellOffset[13][2] = -1;
		m_aSurroundingFarMapCellOffset[14][0] = m_nMapSizeX - 1;
		m_aSurroundingFarMapCellOffset[14][1] = -1;
		m_aSurroundingFarMapCellOffset[14][2] = -m_nMapSizeX - 1;
		m_aSurroundingFarMapCellOffset[15][0] = -1;
		m_aSurroundingFarMapCellOffset[15][1] = -m_nMapSizeX - 1;
		m_aSurroundingFarMapCellOffset[15][2] = 0;

		// zpracuje sestupnì zbývající velikosti kruhù (tj. bez maximální velikosti)
		for ( int nCircleSize = MAX_CIRCLE_SIZE; --nCircleSize >= 2; )
		{	// najde mosty a fiktivní oblasti velikosti kruhu "nCircleSize"
			// ID MapCellu støedu kruhu "nCircleSize"
			signed char cMapCellID = GetMapCellIDFromCircleSize ( nCircleSize );

			// omezení umístìní kruhu
			int nMaxPositionX = m_nMapXMax - ( nCircleSize / 2 );
			int nMaxPositionY = m_nMapYMax - ( nCircleSize / 2 );
			int nStartPosition = MAP_BORDER + ( ( nCircleSize + 1 ) / 2 );

			// ukazatel do mapy
			signed char *pMap = GetAt ( nStartPosition, nStartPosition );

			// zpracuje mapu
			for ( int nPositionY = nStartPosition; nPositionY < nMaxPositionY; nPositionY++, 
				pMap += 2 * MAP_BORDER + nCircleSize )
			{
				for ( int nPositionX = nStartPosition; nPositionX < nMaxPositionX; 
					nPositionX++, pMap++ )
				{
					// zjistí, jedná-li se o MapCell s hledaným kruhem
					if ( *pMap == cMapCellID )
					{	// jedná se o MapCell s hledaným kruhem
						// zjistí, je-li v okolí MapCellu vìtší kruh
						if ( ( *(pMap - 1) <= cMapCellID ) && ( *(pMap + 1) <= cMapCellID ) && 
							( *(pMap - m_nMapSizeX - 1) <= cMapCellID ) && 
							( *(pMap - m_nMapSizeX) <= cMapCellID ) && 
							( *(pMap - m_nMapSizeX + 1) <= cMapCellID ) && 
							( *(pMap + m_nMapSizeX - 1) <= cMapCellID ) && 
							( *(pMap + m_nMapSizeX) <= cMapCellID ) && 
							( *(pMap + m_nMapSizeX + 1) <= cMapCellID ) )
						{	// v okolí MapCellu není vìtší kruh - jedná se o potenciální most
							ASSERT ( pMap == GetAt ( nPositionX, nPositionY ) );
							// seznam bran mostu
							CBridgeGateList cBridgeGateList;
							// nechá najít brány mostu a oznaèí most
							GetBridgeGates ( cMapCellID, pMap, cBridgeGateList );
							// pøidá oznaèený most na zásobník oznaèených oblastí
							m_cMarkedAreaStack.Push ( SMarkedArea ( nPositionX, nPositionY ) );

							// zjistí, jedná-li se o novou fiktivní oblast, výbìžek nebo most
							switch ( cBridgeGateList.GetSize () )
							{
								// jedná se o novou fiktivní oblast
								case 0 :
								{
									// pøidá fiktivní oblast do seznamu oblastí/mostù
									m_cAreaBridgeList.Add ( new ( m_cAreaBridgePool ) 
										SAreaBridge ( CPoint ( nPositionX, nPositionY), 
										cMapCellID ) );
									break;
								}
								// jedná se o výbìžek
								case 1 :
								{
									// vyjme bránu mostu ze seznamu bran mostu
									struct SBridgeGate *pBridgeGate = 
										cBridgeGateList.RemoveFirst ();
									ASSERT ( pBridgeGate != NULL );
									ASSERT ( cBridgeGateList.IsEmpty () );
									// znièí bránu mostu
									m_cBridgeGatePool.Free ( pBridgeGate );
									break;
								}
								// jedná se o most
								default:
								{
									// ukazatel na oblast/most
									struct SAreaBridge *pAreaBridge;
									// pozice v seznamu oblastí/mostù
									POSITION posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();
									// pøíznak nalezení vlízké oblasti/mostu
									BOOL bCloseAreaBridgeFound = FALSE;

									// projede oblasti/mosty "cMapCellID"
									while ( m_cAreaBridgeList.GetNext ( posAreaBridge, 
										pAreaBridge ) )
									{
										// zjistí, jedná-li se o hledanou oblast/most
										if ( pAreaBridge->cMapCellID != cMapCellID )
										{	// nejedná se o hledané oblasti/mosty
											// ukonèí kontrolu oblastí/mostù "cMapCellID"
											break;
										}
										// jedná se o hledanou oblast/most

										// zjistí, je-li oblast/most blízko nového mostu
										if ( ( GetDistance ( pAreaBridge->pointPosition.x, 
											nPositionX ) <= 1 ) && ( GetDistance ( 
											pAreaBridge->pointPosition.y, nPositionY ) <= 1 ) )
										{	// oblast/most je blízko nového mostu
											// nastaví pøíznak nalezení blízké oblasti/mostu
											bCloseAreaBridgeFound = TRUE;
											// ukonèí kontrolu oblastí/mostù "cMapCellID"
											break;
										}
									}

									// zjistí, byla-li nalezena blízká oblast/most
									if ( bCloseAreaBridgeFound )
									{	// byla nalezena blízká oblast/most
										// znièí brány nového mostu
										while ( !cBridgeGateList.IsEmpty () )
										{
											// odebere další bránu nového mostu
											struct SBridgeGate *pNewBridgeGate = 
												cBridgeGateList.RemoveFirst ();
											// ukazatel na bránu mostu
											struct SBridgeGate *pBridgeGate;
											// pozice v seznamu bran mostu
											POSITION posBridgeGate = 
												pAreaBridge->cPrimaryBridgeGateList.GetHeadPosition ();
											// projede brány mostu
											while ( pAreaBridge->cPrimaryBridgeGateList.GetNext ( 
												posBridgeGate, pBridgeGate ) )
											{
												// zjistí, jedná-li se o stejnou bránu mostu
												if ( ( pNewBridgeGate->pArea == 
													pBridgeGate->pArea ) && 
													( GetDistance ( pNewBridgeGate->pointPosition.x, 
													pBridgeGate->pointPosition.x ) <= 1 ) && 
													( GetDistance ( pNewBridgeGate->pointPosition.y, 
													pBridgeGate->pointPosition.y ) <= 1 ) )
												{	// jedná se o duplicitní bránu mostu
													// znièí duplicitní bránu nového mostu
													m_cBridgeGatePool.Free ( pNewBridgeGate );
													pNewBridgeGate = NULL;
													// ukonèí prohledávání bran mostu
													break;
												}
												// nejedná se o stejnou bránu
											}
											// zjistí, byla-li brána mostu duplicitní
											if ( pNewBridgeGate != NULL )
											{	// brána mostu nebyla duplicitní
												// inicializuje most brány mostu
												pNewBridgeGate->pBridge = pAreaBridge;
												// pøidá most brány do sekundárních bran mostù oblasti
												pNewBridgeGate->pArea->cSecondaryBridgeGateList.Add ( 
													pNewBridgeGate );
												// pøidá bránu mostu k mostu
												pAreaBridge->cPrimaryBridgeGateList.Add ( 
													pNewBridgeGate );
											}
											// brána mostu byla duplicitní
										}
										// brány nového mostu byly znièeny
									}
									else
									{	// nebyla nalezena blízká oblast/most
										// vytvoøí nový most
										struct SAreaBridge *pBridge = new ( m_cAreaBridgePool ) 
											SAreaBridge ( CPoint ( nPositionX, nPositionY ), 
											cMapCellID );
										// pøidá most do seznamu oblastí/mostù
										m_cAreaBridgeList.Add ( pBridge );

										// ukazatel na bránu mostu
										struct SBridgeGate *pBridgeGate;
										// pozice v seznamu bran mostu
										POSITION posBridgeGate = cBridgeGateList.GetHeadPosition ();
										// inicializuje brány mostu
										while ( cBridgeGateList.GetNext ( posBridgeGate, 
											pBridgeGate ) )
										{
											ASSERT ( ( pBridgeGate->pArea != NULL ) && 
												( pBridgeGate->pBridge == NULL ) );
											// inicializuje most brány mostu
											pBridgeGate->pBridge = pBridge;
											// pøidá most brány do sekundárních bran mostù oblasti
											pBridgeGate->pArea->cSecondaryBridgeGateList.Add ( 
												pBridgeGate );
										}
										// brány mostu byly inicializovány

										// inicializuje primární brány pøidaného mostu
										cBridgeGateList.MoveList ( pBridge->cPrimaryBridgeGateList );
									}
									break;
								}
							}
							// zpracoval fiktivní oblast, výbìžek i most
						}
						else
						{	// v okolí MapCellu je vìtší kruh
							// poèet souvislých oblastí vìtších kruhù
							int nContinuousAreaCount = 0;

							// spoèítá poèet souvislých oblastí vìtších kruhù v okolní MapCellu
							for ( int i = 8; i-- > 0; )
							{
								// ukazatel na MapCell v okolí
								signed char *pMapCell = pMap + m_aSurroundingMapCellOffset[i];

								// zjistí, jedná-li se o vìtší MapCell
								if ( *pMapCell > cMapCellID )
								{	// jedná se o vìtší MapCell
									// zjistí, je-li MapCell pokraèováním souvislé oblasti vìtších 
									//		MapCellù
									if ( ( *(pMap + m_aContinuousMapCellOffset[i][1]) <= 
										cMapCellID ) && ( ( m_aContinuousMapCellOffset[i][0] == 0 ) || 
										( *(pMap + m_aContinuousMapCellOffset[i][0]) <= 
										cMapCellID ) ) )
									{	// nejedná se o pokraèování souvislé oblasti vìtších kruhù
										// zvýší poèet souvislých oblastí vìtších kruhù
										nContinuousAreaCount++;
									}
								}
								// okolní MapCell byl zpracován
							}
							// spoèítal souvislé oblasti vìtších kruhù v okolí MapCellu

							// zjistí poèet oblastí vìtších kruhù v okolí MapCellu
							if ( nContinuousAreaCount > 1 )
							{	// v okolí MapCellu je více oblastí vìtších kruhù - jedná se 
								//		o most
								// seznam bran mostu
								CBridgeGateList cBridgeGateList;
								// nechá najít brány mostu a oznaèí most
								GetBridgeGates ( cMapCellID, pMap, cBridgeGateList );
								ASSERT ( cBridgeGateList.GetSize () >= 2 );
								// pøidá oznaèený most na zásobník oznaèených oblastí
								m_cMarkedAreaStack.Push ( SMarkedArea ( nPositionX, nPositionY ) );

								// ukazatel na oblast/most
								struct SAreaBridge *pAreaBridge;
								// pozice v seznamu oblastí/mostù
								POSITION posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();
								// pøíznak nalezení blízké oblasti/mostu
								BOOL bCloseAreaBridgeFound = FALSE;

								// projede oblasti/mosty "cMapCellID"
								while ( m_cAreaBridgeList.GetNext ( posAreaBridge, 
									pAreaBridge ) )
								{
									// zjistí, jedná-li se o hledanou oblast/most
									if ( pAreaBridge->cMapCellID != cMapCellID )
									{	// nejedná se o hledané oblasti/mosty
										// ukonèí kontrolu oblastí/mostù "cMapCellID"
										break;
									}
									// jedná se o hledanou oblast/most

									// zjistí, je-li oblast/most blízko nového mostu
									if ( ( GetDistance ( pAreaBridge->pointPosition.x, 
										nPositionX ) <= 2 ) && ( GetDistance ( 
										pAreaBridge->pointPosition.y, nPositionY ) <= 2 ) )
									{	// oblast/most je blízko nového mostu
										// nastaví pøíznak nalezení blízké oblasti/mostu
										bCloseAreaBridgeFound = TRUE;
										// ukonèí kontrolu oblastí/mostù "cMapCellID"
										break;
									}
								}

								// zjistí, byla-li nalezena blízká oblast/most
								if ( bCloseAreaBridgeFound )
								{	// byla nalezena blízká oblast/most
									// znièí brány mostu
									while ( !cBridgeGateList.IsEmpty () )
									{
										// vyjme bránu mostu ze seznamu
										struct SBridgeGate *pBridgeGate = 
											cBridgeGateList.RemoveFirst ();
										ASSERT ( ( pBridgeGate->pArea != NULL ) && 
											( pBridgeGate->pBridge == NULL ) );
										// znièí bránu mostu
										m_cBridgeGatePool.Free ( pBridgeGate );
									}
									// brány mostu byly znièeny
								}
								else
								{	// nebyla nalezena blízká oblast/most
									// vytvoøí nový most
									struct SAreaBridge *pBridge = new ( m_cAreaBridgePool ) 
										SAreaBridge ( CPoint ( nPositionX, nPositionY ), 
										cMapCellID );
									// pøidá most do seznamu oblastí/mostù
									m_cAreaBridgeList.Add ( pBridge );
		
									// ukazatel na bránu mostu
									struct SBridgeGate *pBridgeGate;
									// pozice v seznamu bran mostu
									POSITION posBridgeGate = cBridgeGateList.GetHeadPosition ();
									// projede brány mostu
									while ( cBridgeGateList.GetNext ( posBridgeGate, 
										pBridgeGate ) )
									{
										ASSERT ( ( pBridgeGate->pArea != NULL ) && 
											( pBridgeGate->pBridge == NULL ) );
										// inicializuje most brány mostu
										pBridgeGate->pBridge = pBridge;
										// pøidá most brány do sekundárních bran mostù oblasti
										pBridgeGate->pArea->cSecondaryBridgeGateList.Add ( 
											pBridgeGate );
									}

									// inicializuje primární brány pøidaného mostu
									cBridgeGateList.MoveList ( pBridge->cPrimaryBridgeGateList );
								}
								// most byl zpracován
							}
							else
							{	// v okolí MapCellu je jediná oblast vìtších kruhù - jedná se 
								//		o potenciální most délky 2
								ASSERT ( m_cTemporarilyMarkedMapCellStack.IsEmpty () );

								// oznaèí oblast vìtších kruhù v okolí a vzdáleném okolí MapCellu
								for ( i = 8; i-- > 0; )
								{
									// ukazatel na MapCell v okolí
									signed char *pMapCell = pMap + m_aSurroundingMapCellOffset[i];

									// zjistí, jedná-li se o vìtší MapCell
									if ( *pMapCell > cMapCellID )
									{	// jedná se o vìtší MapCell
										// projede vzdálené okolní MapCelly
										for ( int j = 5; j-- > 0; )
										{
											// offset vzdáleného MapCellu
											int nFarContinuousMapCellOffset = 
												m_aFarContinuousMapCellOffset[i][j];
											// zjistí, jedná-li se o platný offset MapCellu
											if ( nFarContinuousMapCellOffset == 0 )
											{	// nejedná se o platný offset MapCellu
												continue;
											}

											// ukazatel na MapCell ze vzdáleného okolí
											signed char *pFarMapCell = pMap + 
												nFarContinuousMapCellOffset;

											// zjistí, jedná-li se o vìtší MapCell
											if ( *pFarMapCell > cMapCellID )
											{	// jedná se o vìtší MapCell
												// oznaèí vzdálený MapCell
												*pFarMapCell = MARK_EXPLORED ( *pFarMapCell );
												m_cTemporarilyMarkedMapCellStack.Push ( pFarMapCell );

												// index vzdáleného MapCellu
												int nFarIndex = ( i + 7 ) * 2 + j;

												// oznaèí následující sousední vzdálené vìtší MapCelly
												for ( int k = nFarIndex; ; )
												{
													k = ( k + 1 ) & 0x0f;	// k = ( k + 1 ) % 16
													// ukazatel na následující sousední vzdálený MapCell
													signed char *pNextFarMapCell = pMap + 
														m_aFarSurroundingMapCellOffset[k];
													// zjistí, jedná-li se o vìtší MapCell
													if ( *pNextFarMapCell <= cMapCellID )
													{	// nejedná se o vìtší MapCell
														// ukonèí oznaèování následujících vìtších 
														//		vzdálených MapCellù
														break;
													}
													// oznaèí vzdálený MapCell
													*pNextFarMapCell = MARK_EXPLORED ( 
														*pNextFarMapCell );
													m_cTemporarilyMarkedMapCellStack.Push ( 
														pNextFarMapCell );
												}
												// následující sousední vzdálené vìtší MapCelly jsou 
												//		oznaèeny

												// oznaèí pøedchozí sousední vzdálené vìtší MapCelly
												for ( k = nFarIndex; ; )
												{
													k = ( k + 15 ) & 0x0f;		// k = ( k - 1 ) % 16
													// ukazatel na pøedchozí sousední vzdálený MapCell
													signed char *pPreviousFarMapCell = pMap + 
														m_aFarSurroundingMapCellOffset[k];
													// zjistí, jedná-li se o vìtší MapCell
													if ( *pPreviousFarMapCell <= cMapCellID )
													{	// nejedná se o vìtší MapCell
														// ukonèí oznaèování pøedcházejících vìtších 
														//		vzdálených MapCellù
														break;
													}
													// oznaèí vzdálený MapCell
													*pPreviousFarMapCell = MARK_EXPLORED ( 
														*pPreviousFarMapCell );
													m_cTemporarilyMarkedMapCellStack.Push ( 
														pPreviousFarMapCell );
												}
												// pøedchozí sousední vzdálené vìtší MapCelly jsou 
												//		oznaèeny
											}
											// vìtší vzdálený MapCell byl zpracován
										}
										// vzdálené okolní MapCelly jsou projety
									}
									// okolní MapCell byl zpracován
								}
								// okolní a sousední vzdálené okolní vìtší kruhy byly oznaèeny

								// seznam oblastí v okolí MapCellu = seznam bran mostu
								ASSERT ( m_cMapCellQueue.IsEmpty () );

								// najde další oblasti ve vzdáleném okolí MapCellu
								for ( i = 16; i-- > 0; )
								{
									// ukazatel na MapCell ze vzdáleného okolí
									signed char *pFarMapCell = pMap + 
										m_aFarSurroundingMapCellOffset[i];
									// zjistí, jedná-li se o vìtší MapCell
									if ( *pFarMapCell > cMapCellID )
									{	// jedná se o vìtší MapCell
										// ukazatel na následující MapCell
										signed char *pNextFarMapCell = pMap + 
											m_aFarSurroundingMapCellOffset[( i + 1 ) & 0x0f];
												// ( i + 1 ) % 16
										// zjistí, jedná-li se o konec souvislé oblasti
										if ( *pNextFarMapCell <= cMapCellID )
										{	// jedná se o konec souvislé oblasti
											// zjistí, je-li oblast dostupná
											for ( int j = i + 1; ; )
											{
												j = ( j + 15 ) & 0x0f;	// j = ( j - 1 ) % 16
												// zjistí, je-li pøedcházející MapCell souèástí 
												//		oblasti
												if ( *(pMap + m_aFarSurroundingMapCellOffset[j]) <= 
													cMapCellID )
												{	// pøedcházející MapCell není souèástí oblasti
													// ukonèí zjišování dostupnosti oblasti
													break;
												}
												// zjistí, je-li MapCell dostupný
												if ( ( ( m_aSurroundingFarMapCellOffset[j][0] != 0 ) && 
													( *(pMap + m_aSurroundingFarMapCellOffset[j][0]) >= 
													cMapCellID ) ) || 
													( *(pMap + m_aSurroundingFarMapCellOffset[j][1]) >= 
													cMapCellID ) || 
													( ( m_aSurroundingFarMapCellOffset[j][2] != 0 ) && 
													( *(pMap + m_aSurroundingFarMapCellOffset[j][2]) >= 
													cMapCellID ) ) )
												{	// MapCell je dostupný
													// pøidá MapCell do fronty bran mostu
													m_cMapCellQueue.Add ( pFarMapCell );
													// ukonèí zjišování dostupnosti oblasti
													break;
												}
												// MapCell není dostupný
											}
											// zjistil dostupnost oblasti
										}
										// nejdená se o konec souvislé oblasti
									}
									// nejedná se o vzdálený vìtší MapCell
								}
								// oblasti ve vzdáleném okolí byly nalezeny

								// ukazatel na oznaèený MapCell
								signed char *pMarkedMapCell;
								// odznaèí doèasnì oznaèené MapCelly
								while ( m_cTemporarilyMarkedMapCellStack.Pop ( pMarkedMapCell ) )
								{
									ASSERT ( *pMarkedMapCell == MARK_EXPLORED ( *pMarkedMapCell ) );
									// odznaèí oznaèený MapCell
									*pMarkedMapCell = UNMARK_EXPLORED ( *pMarkedMapCell );
								}
								// doèasnì oznaèené MapCelly byly odznaèeny

								// zjistí, jsou-li ve vzdáleném okolí MapCellu další oblasti = 
								//		brány mostu
								if ( !m_cMapCellQueue.IsEmpty () )
								{	// ve vzdáleném okolí MapCellu jsou další oblasti
									// ukazatel na oblast/most
									struct SAreaBridge *pAreaBridge;
									// pozice v seznamu oblastí/mostù
									POSITION posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();
									// pøíznak nalezení vlízké oblasti/mostu
									BOOL bCloseAreaBridgeFound = FALSE;

									// projede oblasti/mosty "cMapCellID"
									while ( m_cAreaBridgeList.GetNext ( posAreaBridge, 
										pAreaBridge ) )
									{
										// zjistí, jedná-li se o hledanou oblast/most
										if ( pAreaBridge->cMapCellID != cMapCellID )
										{	// nejedná se o hledané oblasti/mosty
											// ukonèí kontrolu oblastí/mostù "cMapCellID"
											break;
										}
										// jedná se o hledanou oblast/most

										// zjistí, je-li oblast/most blízko nového mostu
										if ( ( GetDistance ( pAreaBridge->pointPosition.x, 
											nPositionX ) <= 2 ) && ( GetDistance ( 
											pAreaBridge->pointPosition.y, nPositionY ) <= 2 ) )
										{	// oblast/most je blízko nového mostu
											// nastaví pøíznak nalezení blízké oblasti/mostu
											bCloseAreaBridgeFound = TRUE;
											// ukonèí kontrolu oblastí/mostù "cMapCellID"
											break;
										}
									}

									// zjistí, byla-li nalezena blízká oblast/most
									if ( bCloseAreaBridgeFound )
									{	// byla nalezena blízká oblast/most
										// znièí frontu MapCellù bran mostu
										m_cMapCellQueue.RemoveAll ();
									}
									else
									{	// nebyla nalezena blízká oblast/most
										// vytvoøí nový most
										struct SAreaBridge *pBridge = new ( m_cAreaBridgePool ) 
											SAreaBridge ( CPoint ( nPositionX, nPositionY ), 
											cMapCellID );
										// pøidá most do seznamu oblastí/mostù
										m_cAreaBridgeList.Add ( pBridge );

										// ukazatel na nejvìtší MapCell v okolí
										signed char *pMaximumMapCell = pMap;

										// projede okolní MapCelly
										for ( int i = 8; i-- > 0; )
										{
											// ukazatel na MapCell v okolí
											signed char *pMapCell = pMap + 
												m_aSurroundingMapCellOffset[i];

											// zjistí, jedná-li se o vìtší MapCell
											if ( *pMapCell > *pMaximumMapCell )
											{	// jedná se o vìtší MapCell
												// uschová si ukazatel na vìtší MapCell v okolí
												pMaximumMapCell = pMapCell;
											}
											// okolní MapCell byl zpracován
										}
										// našel nejvìtší MapCell v okolí
										ASSERT ( *pMaximumMapCell > *pMap );

										// pøidá nejvìtší MapCell v okolí do fronty bran mostù
										m_cMapCellQueue.Add ( pMaximumMapCell );

										// ukazatel na MapCell brány mostu
										signed char *pMapCell;
										// vyrobí brány mostu
										while ( m_cMapCellQueue.RemoveFirst ( pMapCell ) )
										{
											// nechá najít oblast/most MapCellu
											struct SAreaBridge *pAreaBridge = FindAreaBridge ( 
												pMapCell );

											// vyrobí novou bránu mostu
											struct SBridgeGate *pBridgeGate = 
												m_cBridgeGatePool.Allocate ();
											// inicializuje novì vytvoøenou bránu mostu
											pBridgeGate->pointPosition = GetMapCellPosition ( 
												pMapCell );
											pBridgeGate->pArea = pAreaBridge;
											pBridgeGate->pBridge = pBridge;

											// pøidá bránu mostu do sekundárních bran mostù oblasti
											pAreaBridge->cSecondaryBridgeGateList.Add ( pBridgeGate );
											// pøidá vytvoøenou bránu mostu do seznamu bran mostu
											pBridge->cPrimaryBridgeGateList.Add ( pBridgeGate );
										}
										// brány mostu jsou vyrobeny
									}
									// blízká oblast/most byla zpracována
								}
								// ve vzdáleném okolí MapCellu nejsou další oblasti
							}
							// nejedná se o most
						}
						// MapCell s hledaným kruhem byl zpracován
					}
					// MapCell byl zpracován
				}
			}
			// mapa je zpracována

		// zruší oznaèení oznaèených oblastí

			// oznaèená oblast
			struct SMarkedArea sMarkedArea;

			// projede všechny oznaèené oblasti
			while ( m_cMarkedAreaStack.Pop ( sMarkedArea ) )
			{
				// zjistí, je-li oblast stále oznaèena
				if ( *GetAt ( sMarkedArea.nX, sMarkedArea.nY ) != MARK_EXPLORED ( cMapCellID ) )
				{	// oblast již není oznaèena
					// pokraèuje s další oznaèenou oblastí
					continue;
				}
				// zruší oznaèení oblasti
				RepaintArea ( MARK_EXPLORED ( cMapCellID ), cMapCellID, sMarkedArea.nX, 
					sMarkedArea.nY );
				ASSERT ( *GetAt ( sMarkedArea.nX, sMarkedArea.nY ) == cMapCellID );
			}

		// oznaèí reprezentanty mostù a fiktivních oblastí "cMapCellID"

			// ukazatel na oblast/most
			struct SAreaBridge *pAreaBridge;
			// pozice v seznamu oblastí/mostù
			POSITION posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();

			// projede všechny oblasti/mosty "cMapCellID"
			while ( m_cAreaBridgeList.GetNext ( posAreaBridge, pAreaBridge ) )
			{
				// zjistí, jedná-li se ještì o oblast/most "cMapCellID"
				if ( pAreaBridge->cMapCellID != cMapCellID )
				{	// nejedná se již o oblast/most "cMapCellID"
					break;
				}
				// jedná se o oblast/most "cMapCellID"
				ASSERT ( *GetAt ( pAreaBridge->pointPosition ) == cMapCellID );
				// oznaèí MapCell za reprezentanta oblasti/mostu "cMapCellID"
				*GetAt ( pAreaBridge->pointPosition ) = GetMapCellAreaIDFromMapCellID ( 
					cMapCellID );
			}
		}
		// zpracoval všechny zbývající velikosti kruhù
	}
	// jsou nalezeny brány mostù a fiktivní oblasti

	// ukazatel na oblast/most
	struct SAreaBridge *pAreaBridge;
	// pozice v seznamu oblastí/mostù
	POSITION posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();

	// zruší oznaèení reprezentantù oblastí/mostù
	while ( m_cAreaBridgeList.GetNext ( posAreaBridge, pAreaBridge ) )
	{
		ASSERT ( pAreaBridge->cMapCellID != GetMapCellAreaIDFromMapCellID ( 
			pAreaBridge->cMapCellID ) );
		ASSERT ( GetMapCellAreaIDFromMapCellID ( pAreaBridge->cMapCellID ) == 
			*GetAt ( pAreaBridge->pointPosition ) );
		// zruší oznaèení reprezentanta oblasti/mostu
		*GetAt ( pAreaBridge->pointPosition ) = pAreaBridge->cMapCellID;
	}
	// zrušil oznaèení reprezentanta oblasti/mostu

	// velikost tabulky oblastí (v poètu DWORDù)
	DWORD dwAreaTableSize = 0;
	// poèet bran mostù
	DWORD dwBridgeGateCount = 0;

#ifdef _DEBUG
	// pozice oblasti/mostu v seznamu
	posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();

	// znehodnot indexy bran mostù oblasti
	while ( m_cAreaBridgeList.GetNext ( posAreaBridge, pAreaBridge ) )
	{
		// ukazatel na bránu mostu
		struct SBridgeGate *pBridgeGate;
		// pozice první primární brány mostu oblasti
		POSITION posBridgeGate = pAreaBridge->cPrimaryBridgeGateList.GetHeadPosition ();

		// znehodnotí indexy primárních bran mostù
		while ( CBridgeGateList::GetNext ( posBridgeGate, pBridgeGate ) )
		{
			// znehodnotí index brány mostu
			pBridgeGate->dwIndex = 0xffffffff;
		}
	}
#endif //_DEBUG

	// pozice oblasti/mostu v seznamu
	posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();

	// zjistí velikost tabulky oblastí, poèet bran a inicializuje indexy oblastí a bran 
	//		mostù
	while ( m_cAreaBridgeList.GetNext ( posAreaBridge, pAreaBridge ) )
	{
		ASSERT ( ( pAreaBridge->cMapCellID != GetMapCellIDFromCircleSize ( 
			MAX_CIRCLE_SIZE ) ) || ( pAreaBridge->cPrimaryBridgeGateList.IsEmpty () ) );

		// poèet bran mostù oblasti
		DWORD dwAreaBridgeGateCount = 0;

		// ukazatel na bránu mostu
		struct SBridgeGate *pBridgeGate;
		// pozice první primární brány mostu oblasti
		POSITION posBridgeGate = pAreaBridge->cPrimaryBridgeGateList.GetHeadPosition ();

		// inicializuje indexy primárních bran mostù
		while ( CBridgeGateList::GetNext ( posBridgeGate, pBridgeGate ) )
		{
			// aktualizuje poèet bran mostù oblasti
			dwAreaBridgeGateCount++;

			// inicializuje index brány mostu
			ASSERT ( pBridgeGate->dwIndex == 0xffffffff );
			pBridgeGate->dwIndex = dwBridgeGateCount;
			// aktualizuje poèet bran mostù
			dwBridgeGateCount++;

			ASSERT ( pBridgeGate->pBridge == pAreaBridge );
		}
		ASSERT ( dwAreaBridgeGateCount != 1 );

		// pozice první sekundární brány mostu oblasti
		posBridgeGate = pAreaBridge->cSecondaryBridgeGateList.GetHeadPosition ();

		// inicializuje indexy sekundárních bran mostù
		while ( CBridgeGateList::GetNext ( posBridgeGate, pBridgeGate ) )
		{
			// aktualizuje poèet bran mostù oblasti
			dwAreaBridgeGateCount++;

			ASSERT ( pBridgeGate->pArea == pAreaBridge );
			// najde dotyk brány mostu k oblasti
			FindBridgeGateTouch ( pBridgeGate, pBridgeGate->pArea );
		}

		// zjistí, jedná-li se o samostatnou oblast
		if ( dwAreaBridgeGateCount == 0 )
		{	// jedná se o samostatnou oblast
			continue;
		}
		// nejedná se o samostatnou oblast

		// inicializuje index oblasti
		pAreaBridge->dwIndex = dwAreaTableSize;

		// aktualizuje velikost tabulky oblastí
		dwAreaTableSize += dwAreaBridgeGateCount * dwAreaBridgeGateCount + 2;
	}
	// zjistil velikost tabulky oblastí, poèet bran a inicializoval indexy oblastí a bran 
	//		mostù

	// alokuje tabulku oblastí
	DWORD *aAreaTable = new DWORD[dwAreaTableSize];
	// alokuje tabulku bran mostù
	DWORD *aBridgeGateTable = new DWORD[4 * dwBridgeGateCount];
  // Fill it with nonsense values
  memset ( aBridgeGateTable, 0xFF, sizeof ( DWORD ) * 4 * dwBridgeGateCount );

	// pozice první oblasti/mostu v seznamu
	posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();

	// index do tabulky oblastí
	DWORD dwAreaIndex = 0;
	// index do tabulky bran mostù
	DWORD dwBridgeGateIndex = 0;

	// vytvoøí pole vzdáleností MapCellù
	m_cMapCellDistanceArray.Create ( m_nMapSizeX, m_nMapSizeY, MAPSQUARE_WIDTH, 
		INFINITE_DISTANCE );

	// zpracuje seznam oblastí/mostù
	while ( m_cAreaBridgeList.GetNext ( posAreaBridge, pAreaBridge ) )
	{
		// poèet bran mostù oblasti
		DWORD dwAreaBridgeGateCount = pAreaBridge->cPrimaryBridgeGateList.GetSize () + 
			pAreaBridge->cSecondaryBridgeGateList.GetSize ();

		TRACE_FIND_PATH_CREATION ( "-------Area %d - (%d,%d) - %d, gates: %d\n", dwAreaIndex, 
			pAreaBridge->pointPosition.x, pAreaBridge->pointPosition.y, 
			pAreaBridge->cMapCellID, dwAreaBridgeGateCount );

		// zjistí, jedná-li se o samostatnou oblast
		if ( dwAreaBridgeGateCount == 0 )
		{	// jedná se o samostatnou oblast
			continue;
		}
		// nejedná se o samostatnou oblast

		ASSERT ( pAreaBridge->dwIndex == dwAreaIndex );

		// zapíše do tabulky oblastí poèet bran mostù oblasti a šíøku oblasti
		aAreaTable[dwAreaIndex++] = dwAreaBridgeGateCount;
		aAreaTable[dwAreaIndex++] = (DWORD)( pAreaBridge->cMapCellID / 2 );

		// ukazatel na bránu mostu
		struct SBridgeGate *pBridgeGate;
		// pozice první primární brány mostu oblasti
		POSITION posBridgeGate = pAreaBridge->cPrimaryBridgeGateList.GetHeadPosition ();

		// seznam bran mostù oblasti
		CBridgeGateList cBridgeGateList;
		// pozice brány mostu v seznamu
		POSITION posNewBridgeGate;
		// brána mostu v seznamu
		struct SBridgeGate *pNewBridgeGate;

		// zapíše do tabulky oblastí indexy primárních bran mostù oblasti a do tabulky bran 
		//		mostù indexy oblastí
		while ( CBridgeGateList::GetNext ( posBridgeGate, pBridgeGate ) )
		{
			TRACE_FIND_PATH_CREATION ( "\tBridgeGate %d - (%d,%d)\n", pBridgeGate->dwIndex, 
				pBridgeGate->pointPosition.x, pBridgeGate->pointPosition.y );

			// zapíše do tabulky oblastí index primární brány mostu oblasti
			aAreaTable[dwAreaIndex++] = pBridgeGate->dwIndex;
			// zapíše do tabulky bran mostù indexy oblastí
			ASSERT ( pBridgeGate->pBridge == pAreaBridge );
/*			aBridgeGateTable[dwBridgeGateIndex++] = pBridgeGate->pointPosition.x;
			aBridgeGateTable[dwBridgeGateIndex++] = pBridgeGate->pointPosition.y;
			aBridgeGateTable[dwBridgeGateIndex++] = pBridgeGate->pBridge->dwIndex;
			aBridgeGateTable[dwBridgeGateIndex++] = pBridgeGate->pArea->dwIndex;*/

      // Is it already written
      if ( aBridgeGateTable [ 4 * pBridgeGate->dwIndex ] == 0x0FFFFFFFF )
      {
        // No

			  aBridgeGateTable[4 * pBridgeGate->dwIndex] = pBridgeGate->pointPosition.x;
			  aBridgeGateTable[4 * pBridgeGate->dwIndex + 1] = pBridgeGate->pointPosition.y;
			  aBridgeGateTable[4 * pBridgeGate->dwIndex + 2] = pBridgeGate->pBridge->dwIndex;
			  aBridgeGateTable[4 * pBridgeGate->dwIndex + 3] = pBridgeGate->pArea->dwIndex;
      }

			// pøidá bránu mostu na konec seznamu bran mostù
			posNewBridgeGate = cBridgeGateList.GetHeadPosition ();
			while ( CBridgeGateList::GetNext ( posNewBridgeGate, pNewBridgeGate ) );
			cBridgeGateList.Insert ( posNewBridgeGate, pBridgeGate );
		}

		// pozice první sekundární brány mostu oblasti
		posBridgeGate = pAreaBridge->cSecondaryBridgeGateList.GetHeadPosition ();

		// zapíše do tabulky oblastí indexy sekundárních bran mostù oblasti a do tabulky bran 
		//		mostù indexy obalstí
		while ( CBridgeGateList::GetNext ( posBridgeGate, pBridgeGate ) )
		{
			TRACE_FIND_PATH_CREATION ( "\tBridgeGate %d - (%d,%d)\n", pBridgeGate->dwIndex, 
				pBridgeGate->pointPosition.x, pBridgeGate->pointPosition.y );

			// zapíše do tabulky oblastí index sekundární brány mostu oblasti
			aAreaTable[dwAreaIndex++] = pBridgeGate->dwIndex;
			// zapíše do tabulky bran mostù indexy oblastí
			ASSERT ( ( pBridgeGate->pBridge == pAreaBridge ) || 
				( pBridgeGate->pArea == pAreaBridge ) );
/*			aBridgeGateTable[dwBridgeGateIndex++] = pBridgeGate->pointPosition.x;
			aBridgeGateTable[dwBridgeGateIndex++] = pBridgeGate->pointPosition.y;
			aBridgeGateTable[dwBridgeGateIndex++] = pBridgeGate->pBridge->dwIndex;
			aBridgeGateTable[dwBridgeGateIndex++] = pBridgeGate->pArea->dwIndex;*/

      // Is it already written
      if ( aBridgeGateTable [ 4 * pBridgeGate->dwIndex ] == 0x0FFFFFFFF )
      {
        // No

			  aBridgeGateTable[4 * pBridgeGate->dwIndex] = pBridgeGate->pointPosition.x;
			  aBridgeGateTable[4 * pBridgeGate->dwIndex + 1] = pBridgeGate->pointPosition.y;
			  aBridgeGateTable[4 * pBridgeGate->dwIndex + 2] = pBridgeGate->pBridge->dwIndex;
			  aBridgeGateTable[4 * pBridgeGate->dwIndex + 3] = pBridgeGate->pArea->dwIndex;
      }

			// pøidá bránu mostu na konec seznamu bran mostù
			posNewBridgeGate = cBridgeGateList.GetHeadPosition ();
			while ( CBridgeGateList::GetNext ( posNewBridgeGate, pNewBridgeGate ) );
			cBridgeGateList.Insert ( posNewBridgeGate, pBridgeGate );
		}

		// poèet bran mostù oblasti
		DWORD dwBridgeGateCount = cBridgeGateList.GetSize ();

		// zjistí vzdálenosti bran mostù oblasti
		while ( !cBridgeGateList.IsEmpty () )
		{
			// vyjme první bránu mostu oblasti
			pBridgeGate = cBridgeGateList.RemoveFirst ();

			// zjistí, jedná-li se o poslední bránu mostu oblasti
			if ( cBridgeGateList.IsEmpty () )
			{	// jedná se o poslední bránu mostu oblasti
				// ukonèí zjišování vzdáleností bran mostù oblasti
				break;
			}
			// nejedná se o poslední bránu mostu oblasti

			// aktualizuje poèet bran mostù oblasti
			dwBridgeGateCount--;
			ASSERT ( dwBridgeGateCount > 0 );

			// zjistí vzdálenosti bran mostù oblasti
			GetBridgeGateDistances ( pAreaBridge, pBridgeGate, cBridgeGateList, 
				&aAreaTable[dwAreaIndex] );
			// aktualizuje index do tabulky oblastí
			dwAreaIndex += 2 * dwBridgeGateCount;
		}
	}
	// seznam oblastí/mostù byl zpracován

	// znièí pole vzdáleností MapCellù bran mostù
	m_cMapCellDistanceArray.Delete ();

	// znièí seznam oblastí/mostù a oznaèí brány mostù a pøeloží jejich souøadnice
	while ( !m_cAreaBridgeList.IsEmpty () )
	{
		// vyjme oblast/most ze seznamu
		pAreaBridge = m_cAreaBridgeList.RemoveFirst ();

		// znièí seznam primárních bran mostù oblasti
		while ( !pAreaBridge->cPrimaryBridgeGateList.IsEmpty () )
		{
			// vyjme další bránu mostu ze seznamu
			struct SBridgeGate *pBridgeGate = 
				pAreaBridge->cPrimaryBridgeGateList.RemoveFirst ();

			// ukazatel na MapCell brány mostu
			signed char *pMapCell = GetAt ( pBridgeGate->pointPosition );
			// oznaèí bránu mostu
			*pMapCell = GetMapCellAreaIDFromMapCellID ( *pMapCell );
			// pøeloží souøadnice brány mostu
			ASSERT ( aBridgeGateTable[4 * pBridgeGate->dwIndex] >= MAP_BORDER );
			ASSERT ( pBridgeGate->pointPosition.x == (int)aBridgeGateTable[4 * pBridgeGate->dwIndex] );
			aBridgeGateTable[4 * pBridgeGate->dwIndex] -= MAP_BORDER;
			ASSERT ( aBridgeGateTable[4 * pBridgeGate->dwIndex + 1] >= MAP_BORDER );
			ASSERT ( pBridgeGate->pointPosition.y == (int)aBridgeGateTable[4 * pBridgeGate->dwIndex + 1] );
			aBridgeGateTable[4 * pBridgeGate->dwIndex + 1] -= MAP_BORDER;

			// znièí bránu mostu
			m_cBridgeGatePool.Free ( pBridgeGate );
		}

		// znièí seznam sekundárních bran mostù
		pAreaBridge->cSecondaryBridgeGateList.RemoveAll ();

		// znièí oblast/most
		SAreaBridge::operator delete ( pAreaBridge, m_cAreaBridgePool );
	}
	// seznam oblastí/mostù byl znièen

	// velikost tabulky bran mostù
	DWORD dwBridgeGateTableSize = 4 * dwBridgeGateCount;

	// vytvoøí adresáø grafu pro hledání cesty
	cFindPathGraphsArchive.MakeDirectory ( m_strFindPathGraphName );
	// otevøe archiv grafu pro hledání cesty
	CDataArchive cFindPathGraphArchive = cFindPathGraphsArchive.CreateArchive ( 
		m_strFindPathGraphName );

	// vytvoøí hlavièkový soubor grafu pro hledání cesty
	{
		// hlavièkový soubor grafu pro hledání cesty
		CArchiveFile cHeaderFile = cFindPathGraphArchive.CreateFile ( 
			FIND_PATH_GRAPH_HEADER_FILE_NAME, CFile::modeWrite | CFile::modeCreate );

		// vyplní hlavièku grafu pro hledání cesty
		SMFindPathGraphHeader sFindPathGraphHeader;
		sFindPathGraphHeader.m_dwAreaTableSize = dwAreaTableSize;
		sFindPathGraphHeader.m_dwBridgeGateTableSize = dwBridgeGateTableSize;
		sFindPathGraphHeader.m_dwPathHintTableSize = m_dwPathHintTableSize;

		// uloží do hlavièkového souboru hlavièku grafu pro hledání cesty
		cHeaderFile.Write ( &sFindPathGraphHeader, sizeof ( sFindPathGraphHeader ) );
		// uloží do hlavièkového souboru tabulky grafu pro hledání cesty
		cHeaderFile.Write ( aAreaTable, dwAreaTableSize * sizeof ( DWORD ) );
		cHeaderFile.Write ( aBridgeGateTable, dwBridgeGateTableSize * sizeof ( DWORD ) );
		cHeaderFile.Write ( m_pPathHintTable, m_dwPathHintTableSize * sizeof ( DWORD ) );
	}

	// znièí tabulku oblastí
	if ( aAreaTable != NULL )
	{
		delete [] aAreaTable;
	}
	// znièí tabulku bran mostù
	if ( aBridgeGateTable != NULL )
	{
		delete [] aBridgeGateTable;
	}
	// znièí tabulku hintù cest
	if ( m_pPathHintTable != NULL )
	{
		ASSERT ( m_dwPathHintTableSize <= m_dwAllocatedPathHintTableSize );

		free ( m_pPathHintTable );
		m_pPathHintTable = NULL;
		m_dwPathHintTableSize = 0;
		m_dwAllocatedPathHintTableSize = 0;
	}
	else
	{
		ASSERT ( ( m_dwPathHintTableSize == 0 ) && ( m_dwAllocatedPathHintTableSize == 0 ) );
	}

	// vytvoøí datový soubor grafu pro hledání cesty
	{
		// datový soubor grafu pro hledání cesty
		CArchiveFile cFile = cFindPathGraphArchive.CreateFile ( _T("beta.version.data"), 
			CFile::modeWrite | CFile::modeCreate );

		// ukazatel na øádek mapy
		signed char *pLine = GetAt ( MAP_BORDER, MAP_BORDER );

		// uloží do datového souboru kruhy mapy
		for ( int nLineIndex = m_nSizeY; nLineIndex-- > 0; pLine += m_nMapSizeX )
		{
			// uloží do datového souboru další øádek kruhù mapy
			cFile.Write ( pLine, m_nSizeX );
		}
	}
}

// pøebarví oblast "cOldColor" zaèínající na souøadnicích "nX" a "nY" barvou "cNewColor"
void CCFPGMap::RepaintArea ( signed char cOldColor, signed char cNewColor, int nX, 
	int nY ) 
{
	ASSERT ( nX >= MAP_BORDER );
	ASSERT ( nX < MAP_BORDER + m_nSizeX );
	ASSERT ( nY >= MAP_BORDER );
	ASSERT ( nY < MAP_BORDER + m_nSizeY );
	ASSERT ( cOldColor != cNewColor );

	// aktualizuje barvu pøebarvované oblasti
	m_cRepaintAreaSearchLineAreaColor = cOldColor;

	struct SRepaintAreaBlock sBlock;

	signed char *pLine = GetAt ( nX, nY );
	ASSERT ( *pLine == cOldColor );

	// nechá prohledat øádku v okolí bodu
	RepaintAreaSearchLine ( pLine, nX, nX, nY );

	// zpracuje zásobník
	while ( m_cRepaintAreaBlockStack.Pop ( sBlock ) )
	{
		// zjistí ukazatel na zaèátek øádku
		pLine = GetAt ( sBlock.nMinX, sBlock.nY );
		// zjistí, je-li øádek již zpracován
		if ( *pLine == cOldColor )
		{	// øádek dosud nebyl zpracován
			// uschová si zaèátek øádku
			signed char *pLineStart = pLine - 1;
			// oznaèí øádek za objevený
			for ( int i = sBlock.nMaxX - sBlock.nMinX + 1; i-- > 0; pLine++ )
			{
				ASSERT ( *pLine == cOldColor );
				*pLine = cNewColor;
			}
			// prohledá horní øádku (vèetnì pøechodu pøes roh)
			RepaintAreaSearchLine ( pLineStart - m_nMapSizeX, sBlock.nMinX - 1, 
				sBlock.nMaxX + 1, sBlock.nY - 1 );
			// prohledá dolní øádku (vèetnì pøechodu pøes roh)
			RepaintAreaSearchLine ( pLineStart + m_nMapSizeX, sBlock.nMinX - 1, 
				sBlock.nMaxX + 1, sBlock.nY + 1 );
		}
		// øádek byl zpacován
	}
	// zpracoval celý zásobník
}

// prohledá øádek "nY" od "nMinX" ("pLine") do "nMaxX" pro pøekreslování oblasti
void CCFPGMap::RepaintAreaSearchLine ( signed char *pLine, int nMinX, int nMaxX, int nY ) 
{
	ASSERT ( pLine == GetAt ( nMinX, nY ) );

	// zjistí barvu pøebarvované oblasti
	signed char cAreaColor = m_cRepaintAreaSearchLineAreaColor;
	// ukazatel na zaèátek øádky
	signed char *pLineStart;

	// index zaèátku oblasti na øádce
	int nStartX;

	// zjistí, zaèínáme-li již na oblasti
	if ( *pLine != cAreaColor )
	{	// zaèínáme mimo oblast
		// najde zaèátek oblasti do "nMaxX"
		do
		{
			nMinX++;
			pLine++;
		} while ( ( nMinX <= nMaxX ) && ( *pLine != cAreaColor ) );

		// zjistí, našel-li zaèátek oblasti do "nMaxX"
		if ( nMinX > nMaxX )
		{	// nenašel zaèátek oblasti do "nMaxX"
			// ukonèí prohledávání øádky
			return;
		}

		// nastaví zaèátek oblasti øádky ("nStartX", "pLine" a "pLineStart")
		nStartX = nMinX;
		pLineStart = ++pLine;
	}
	else
	{	// zaèínáme na oblasti
		// najde zaèátek oblasti na øádce
		pLineStart = pLine;
		do
		{
			pLine--;
		} while ( *pLine == cAreaColor );
		pLine++;

		// nastaví zaèátek oblasti øádky ("nStartX", "pLine" a "pLineStart")
		nStartX = nMinX - ( pLineStart - pLine );
		pLine = ++pLineStart;
	}
	// "pLine" = "pLineStart" je za "nStartX" o 1 MapCell

	// index konce oblasti na øádce
	int nStopX = nMinX + 1;
	// najde všechny oblasti na øádce
	do
	{
		// "pLine" = "pLineStart" je na "nStopX" >= "nStartX"

		// najde konec oblasti na øádce
		while ( *pLine == cAreaColor )
		{
			pLine++;
		}
		nStopX += pLine - pLineStart - 1;
		// "pLine" je za "nStopX" o 1 MapCell

		// pøidá oblast øádky do zásobníku blokù pro objevování oblastí
		m_cRepaintAreaBlockStack.Push ( SRepaintAreaBlock ( nStartX, nStopX, nY ) );

		// zaène za nalezenou oblastí na øádce
		nStartX = nStopX + 1;
		// "pLine" je na "nStartX"

		// najde zaèátek další oblasti do "nMaxX"
		do
		{
			pLine++;
			nStartX++;
		} while ( ( nStartX <= nMaxX ) && ( *pLine != cAreaColor ) );

		// nastaví zaèátek oblasti øádky ("nStartX", "pLine" a "pLineStart")
		nStopX = nStartX;
		pLineStart = pLine;
		// zjistí, jedná-li se o oblast øádky za "nMaxX"
	} while ( nStartX <= nMaxX );
	// našel všechny oblasti na øádce od "nMinX" do "nMaxX" a pøidal je na zásobník
}


// pøebarví oblast "cOldColor" zaèínající na souøadnicích "nX" a "nY" barvou "cNewColor"
// zároveò hledá reprezentanta ( mapcell, který má hodnotu "cRepresentative" )
// dìlá to floodfillem
// vrací uklazatel na reprezentanta ( nebo NULL )
signed char * CCFPGMap::FindAreaRepresentative ( signed char cOldColor, signed char cNewColor, int nX, 
	int nY, signed char cRepresentative ) 
{
	ASSERT ( nX >= 0 );
	ASSERT ( nX < m_nSizeX );
	ASSERT ( nY >= 0 );
	ASSERT ( nY < m_nSizeY );
	ASSERT ( cOldColor != cNewColor );
	ASSERT ( cRepresentative != cOldColor );
	ASSERT ( cRepresentative != cNewColor );

	// aktualizuje barvu pøebarvované oblasti
	m_cRepaintAreaSearchLineAreaColor = cOldColor;

	struct SRepaintAreaBlock sBlock;

	// výsledek dá sem
	signed char *pRepresentative = NULL, *pRep;

	signed char *pLine = GetAt ( nX, nY );
	ASSERT ( *pLine == cOldColor );

	// nechá prohledat øádku v okolí bodu
	if ( ( pRep = FindAreaRepresentativeSearchLine ( pLine, nX, nX, nY, cRepresentative ) ) != NULL )
	{
		// Super už jsme ho našli
		ASSERT ( pRepresentative == NULL );
		pRepresentative = pRep;
	}

	// zpracuje zásobník
	while ( m_cRepaintAreaBlockStack.Pop ( sBlock ) )
	{
		// zjistí ukazatel na zaèátek øádku
		pLine = GetAt ( sBlock.nMinX, sBlock.nY );
		// zjistí, je-li øádek již zpracován
		if ( ( *pLine == cOldColor ) ||  ( ( *pLine == cRepresentative ) && ( *( pLine + 1 ) != cNewColor ) ) )
		{	// øádek dosud nebyl zpracován
			// uschová si zaèátek øádku
			signed char *pLineStart = pLine - 1;
			// oznaèí øádek za objevený
			for ( int i = sBlock.nMaxX - sBlock.nMinX + 1; i-- > 0; pLine++ )
			{
				if ( *pLine == cRepresentative ) continue;

				ASSERT ( *pLine == cOldColor );
				*pLine = cNewColor;
			}
			// prohledá horní øádku (vèetnì pøechodu pøes roh)
			if ( ( pRep = FindAreaRepresentativeSearchLine ( pLineStart - m_nMapSizeX, sBlock.nMinX - 1, 
				sBlock.nMaxX + 1, sBlock.nY - 1, cRepresentative ) ) != NULL )
			{
//				ASSERT ( pRepresentative == NULL );
				pRepresentative = pRep;
			}
			// prohledá dolní øádku (vèetnì pøechodu pøes roh)
			if ( ( pRep = FindAreaRepresentativeSearchLine ( pLineStart + m_nMapSizeX, sBlock.nMinX - 1, 
				sBlock.nMaxX + 1, sBlock.nY + 1, cRepresentative ) ) != NULL )
			{
//				ASSERT ( pRepresentative == NULL );
				pRepresentative = pRep;
			}
		}
		// øádek byl zpracován
	}
	// zpracoval celý zásobník

	return pRepresentative;
}

// prohledá øádek "nY" od "nMinX" ("pLine") do "nMaxX" pro pøekreslování oblasti
signed char * CCFPGMap::FindAreaRepresentativeSearchLine ( signed char *pLine, int nMinX, int nMaxX, int nY, signed char cRepresentative ) 
{
	ASSERT ( pLine == GetAt ( nMinX, nY ) );

	// zjistí barvu pøebarvované oblasti
	signed char cAreaColor = m_cRepaintAreaSearchLineAreaColor;
	// ukazatel na zaèátek øádky
	signed char *pLineStart;

	// tohle je návratová hodnota
	signed char *pRepresentative = NULL;

	// index zaèátku oblasti na øádce
	int nStartX;

	// zjistí, zaèínáme-li již na oblasti
	if ( ( *pLine != cAreaColor ) && ( *pLine != cRepresentative )  )
	{	// zaèínáme mimo oblast
		// najde zaèátek oblasti do "nMaxX"
		do
		{
			if ( *pLine == cRepresentative )
			{
				ASSERT ( pRepresentative == NULL );
				pRepresentative = pLine;
			}

			nMinX++;
			pLine++;
		} while ( ( nMinX <= nMaxX ) && ( ( *pLine != cAreaColor ) || ( *pLine == cRepresentative ) ) );

		// zjistí, našel-li zaèátek oblasti do "nMaxX"
		if ( nMinX > nMaxX )
		{	// nenašel zaèátek oblasti do "nMaxX"
			// ukonèí prohledávání øádky
			return pRepresentative;
		}

		// nastaví zaèátek oblasti øádky ("nStartX", "pLine" a "pLineStart")
		nStartX = nMinX;
		pLineStart = ++pLine;
	}
	else
	{	// zaèínáme na oblasti
		// najde zaèátek oblasti na øádce
		pLineStart = pLine;
		do
		{
			if ( *pLine == cRepresentative )
			{
				ASSERT ( pRepresentative == NULL );
				pRepresentative = pLine;
			}

			pLine--;
		} while ( ( *pLine == cAreaColor ) || ( *pLine == cRepresentative ) );
		pLine++;

		// nastaví zaèátek oblasti øádky ("nStartX", "pLine" a "pLineStart")
		nStartX = nMinX - ( pLineStart - pLine );
		pLine = ++pLineStart;
	}
	// "pLine" = "pLineStart" je za "nStartX" o 1 MapCell

	// index konce oblasti na øádce
	int nStopX = nMinX + 1;
	// najde všechny oblasti na øádce
	do
	{
		// "pLine" = "pLineStart" je na "nStopX" >= "nStartX"

		// najde konec oblasti na øádce
		while ( (*pLine == cAreaColor) || (*pLine == cRepresentative) )
		{
			if ( *pLine == cRepresentative )
			{
				ASSERT ( pRepresentative == NULL );
				pRepresentative = pLine;
			}
			pLine++;
		}
		nStopX += pLine - pLineStart - 1;
		// "pLine" je za "nStopX" o 1 MapCell

		// pøidá oblast øádky do zásobníku blokù pro objevování oblastí
		m_cRepaintAreaBlockStack.Push ( SRepaintAreaBlock ( nStartX, nStopX, nY ) );

		// zaène za nalezenou oblastí na øádce
		nStartX = nStopX + 1;
		// "pLine" je na "nStartX"

		// najde zaèátek další oblasti do "nMaxX"
		do
		{
			if ( *pLine == cRepresentative )
			{
				ASSERT ( pRepresentative == NULL );
				pRepresentative = pLine;
			}
			pLine++;
			nStartX++;
		} while ( ( nStartX <= nMaxX ) && ( ( *pLine != cAreaColor ) || ( *pLine == cRepresentative ) ) );

		// nastaví zaèátek oblasti øádky ("nStartX", "pLine" a "pLineStart")
		nStopX = nStartX;
		pLineStart = pLine;
		// zjistí, jedná-li se o oblast øádky za "nMaxX"
	} while ( nStartX <= nMaxX );
	// našel všechny oblasti na øádce od "nMinX" do "nMaxX" a pøidal je na zásobník

	return pRepresentative;
}


// najde brány mostu "cMapCellID" na "pPosition" a vloží je do "cBridgeGateList"
void CCFPGMap::GetBridgeGates ( signed char cMapCellID, signed char *pPosition, 
	CBridgeGateList &cBridgeGateList ) 
{
	ASSERT ( cBridgeGateList.IsEmpty () );
	ASSERT ( !IsMapCellAreaID ( cMapCellID ) );
	ASSERT ( *pPosition == cMapCellID );

	// ID oznaèeného MapCellu
	signed char cMarkedMapCellID = MARK_EXPLORED ( cMapCellID );
	ASSERT ( cMapCellID != cMarkedMapCellID );

	ASSERT ( m_cMapCellQueue.IsEmpty () );
	ASSERT ( m_cTemporarilyMarkedMapCellStack.IsEmpty () );

	// oznaèí MapCell mostu
	*pPosition = cMarkedMapCellID;

	// pøidá do fronty blok prvního MapCellu
	m_cMapCellQueue.Add ( pPosition );

	// zpracuje frontu
	while ( m_cMapCellQueue.RemoveFirst ( pPosition ) )
	{
		ASSERT ( *pPosition == cMarkedMapCellID );

		// zjistí, je-li v okolí MapCellu vìtší kruh
		if ( ( *(pPosition - 1) <= cMapCellID ) && 
			( *(pPosition + 1) <= cMapCellID ) && 
			( *(pPosition - m_nMapSizeX - 1) <= cMapCellID ) && 
			( *(pPosition - m_nMapSizeX) <= cMapCellID ) && 
			( *(pPosition - m_nMapSizeX + 1) <= cMapCellID ) && 
			( *(pPosition + m_nMapSizeX - 1) <= cMapCellID ) && 
			( *(pPosition + m_nMapSizeX) <= cMapCellID ) && 
			( *(pPosition + m_nMapSizeX + 1) <= cMapCellID ) )
		{	// v okolí MapCellu není vìtší kruh
			// zpracuje okolní MapCelly
			for ( int i = 8; i-- > 0; )
			{
				// ukazatel na sousední MapCell
				signed char *pMapCell = pPosition + m_aSurroundingMapCellOffset[i];
				// zjistí, jedná-li se o hledaný MapCell
				if ( *pMapCell == cMapCellID )
				{	// jedná se o hledaný MapCell
					// oznaèí MapCell
					*pMapCell = cMarkedMapCellID;
					// pøidá do fronty blok MapCellu
					m_cMapCellQueue.Add ( pMapCell );
				}
				// nejedná se o hledaný MapCell
			}
			// okolní MapCelly byly zpracovány
		}
		else
		{	// v okolí MapCellu je vìtší kruh
			// doèasnì oznaèí sousední MapCelly a oznaèené vyjme z fronty
			for ( int i = 0; i < 8; i++ )
			{
				// ukazatel na sousední MapCell
				signed char *pMapCell = pPosition + m_aSurroundingMapCellOffset[i];
				// zjistí, jedná-li se o hledaný MapCell
				if ( *pMapCell == cMapCellID )
				{	// jedná se o hledaný MapCell
					// oznaèí MapCell
					*pMapCell = cMarkedMapCellID;
					// pøidá MapCell do zásobníku doèasnì oznaèených MapCellù
					m_cTemporarilyMarkedMapCellStack.Push ( pMapCell );
				}
				else if ( *pMapCell == cMarkedMapCellID )
				{	// jedná se o oznaèený MapCell
					// odebere z fronty blok MapCellu
					(void)m_cMapCellQueue.RemoveElement ( pMapCell );
				}
				else if ( *pMapCell > cMapCellID )
				{	// jedná se o MapCell vìtšího kruhu
					ASSERT ( *pMapCell != GetMapCellAreaIDFromMapCellID ( cMapCellID ) );

					// doèasnì oznaèí okolní MapCelly a oznaèené vyjme z fronty
					for ( int j = 8; j-- > 0; )
					{
						// ukazatel na oznaèovaný MapCell
						signed char *pMarkedMapCell = pMapCell + 
							m_aSurroundingMapCellOffset[j];

						// zjití, jedná-li se o hledaný MapCell
						if ( *pMarkedMapCell == cMapCellID )
						{	// jedná se o hledaný MapCell
							// oznaèí MapCell
							*pMarkedMapCell = cMarkedMapCellID;
							// pøidá MapCell do zásobníku doèasnì oznaèených MapCellù
							m_cTemporarilyMarkedMapCellStack.Push ( pMarkedMapCell );
						}
						else if ( *pMarkedMapCell == cMarkedMapCellID )
						{	// jedná se o již oznaèený MapCell
							// odebere z fronty blok MapCellu
							(void)m_cMapCellQueue.RemoveElement ( pMarkedMapCell );
						}
						// MapCell byl zpracován
					}
					// okolní MapCelly byly zpracovány

					// zjistí, jedná-li se o pokraèování souvislé oblasti vìtších kruhù
					if ( ( *(pPosition + m_aContinuousMapCellOffset[i][1]) <= cMapCellID ) && 
						( ( m_aContinuousMapCellOffset[i][0] == 0 ) || ( *(pPosition + 
						m_aContinuousMapCellOffset[i][0]) <= cMapCellID ) ) )
					{	// nejedná se o pokraèování souvislé oblasti vìtších kruhù
						// ukazatel na nejvìtší kruh v okolí MapCellu "pMapCell"
						signed char *pMaximumMapCell = pMapCell;

						// najde v souvislé oblasti okolních MapCellù nejvìtší kruh
						for ( j = i + 1; ; j++ )
						{
							// zjistí index MapCellu
							j &= 0x07;		// j %= 8
							ASSERT ( i != j );
							// zjistí, je-li MapCell pokraèováním souvislé oblasti vìtších MapCellù
							if ( ( *(pPosition + m_aContinuousMapCellOffset[j][1]) <= 
								cMapCellID ) && ( ( m_aContinuousMapCellOffset[j][0] == 0 ) || 
								( *(pPosition + m_aContinuousMapCellOffset[j][0]) <= cMapCellID ) ) )
							{	// nejedná se o pokraèování souvislé oblasti vìtších kruhù
								// ukonèí hledání nejvìtšího kruhu v souvislé oblasti okolních 
								//		MapCellù
								break;
							}

							// zjistí, jedná-li se o nejvìtší kruh
							if ( *pMaximumMapCell < *(pPosition + m_aSurroundingMapCellOffset[j]) )
							{	// jedná se o nejvìtší kruh
								// aktualizuje ukazatel na nejvìtší kruh
								pMaximumMapCell = pPosition + m_aSurroundingMapCellOffset[j];
							}
						}
						// byl nalezen nejvìtší kruh v okolí MapCellu

						// nechá najít oblast/most MapCellu
						struct SAreaBridge *pAreaBridge = FindAreaBridge ( pMaximumMapCell );

						// vyrobí novou bránu mostu
						struct SBridgeGate *pBridgeGate = m_cBridgeGatePool.Allocate ();
						// inicializuje novì vytvoøenou bránu mostu
						pBridgeGate->pointPosition = GetMapCellPosition ( pMaximumMapCell );
						pBridgeGate->pArea = pAreaBridge;
						pBridgeGate->pBridge = NULL;

						// pøidá vytvoøenou bránu mostu do seznamu bran mostu
						cBridgeGateList.Add ( pBridgeGate );
					}
					// jedná se o pokraèování souvislé oblasti vìtších kruhù
				}
				// jedná se o MapCell menšího kruhu
			}
			// okolní MapCelly byly zpracovány
		}
		// okolí MapCellu bylo zpracováno
	}
	// fronta je zpracována

	// odznaèí doèasnì oznaèené MapCelly
	while ( m_cTemporarilyMarkedMapCellStack.Pop ( pPosition ) )
	{
		ASSERT ( *pPosition == cMarkedMapCellID );
		// odznaèí oznaèený MapCell
		*pPosition = cMapCellID;
	}
}

// vrátí oblast/most na pozici "pPosition"
struct CCFPGMap::SAreaBridge *CCFPGMap::FindAreaBridge ( signed char *pPosition ) 
{
	ASSERT ( *pPosition != MARK_EXPLORED ( *pPosition ) );

	ASSERT ( m_cFindAreaBridgeMapCellQueue.IsEmpty () );
	ASSERT ( m_cFindAreaBridgeTemporarilyMarkedMapCellStack.IsEmpty () );

	// zjistí, jedná-li se o nejvìtší oblast
	while ( *pPosition < GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ) )
	{	// nejedná se dosud o nejvìtší oblast
		// ID hledaného MapCellu
		signed char cMapCellID = *pPosition;
		ASSERT ( cMapCellID != MARK_EXPLORED ( cMapCellID ) );
		// ID MapCellu oblasti
		signed char cMapCellAreaID = GetMapCellAreaIDFromMapCellID ( cMapCellID );

		// zjistí, jedná-li se o MapCell oblasti
		if ( cMapCellID != cMapCellAreaID )
		{	// nejedná se o MapCell oblasti
			// ID oznaèeného MapCellu
			signed char cMarkedMapCellID = MARK_EXPLORED ( cMapCellID );
			// ukazatel na první MapCell
			signed char *pStartPosition = pPosition;

			// ukazatel na vìtší sousední oblast
			signed char *pNeighbourBiggerArea = NULL;
#ifdef _DEBUG
			// pøíznak více vìtších sousedních oblastí
			BOOL bMultiNeighbourBiggerArea = FALSE;
#endif //_DEBUG

			// oznaèí první MapCell
			*pPosition = cMarkedMapCellID;
			// pøidá do fronty blok prvního MapCellu
			ASSERT ( m_cFindAreaBridgeMapCellQueue.IsEmpty () );
			m_cFindAreaBridgeMapCellQueue.Add ( pPosition );

			// zpracuje frontu
			while ( m_cFindAreaBridgeMapCellQueue.RemoveFirst ( pPosition ) )
			{
				ASSERT ( *pPosition == cMarkedMapCellID );

				// zjistí, je-li v okolí MapCellu vìtší kruh
				if ( ( *(pPosition - 1) <= cMapCellID ) && 
					( *(pPosition + 1) <= cMapCellID ) && 
					( *(pPosition - m_nMapSizeX - 1) <= cMapCellID ) && 
					( *(pPosition - m_nMapSizeX) <= cMapCellID ) && 
					( *(pPosition - m_nMapSizeX + 1) <= cMapCellID ) && 
					( *(pPosition + m_nMapSizeX - 1) <= cMapCellID ) && 
					( *(pPosition + m_nMapSizeX) <= cMapCellID ) && 
					( *(pPosition + m_nMapSizeX + 1) <= cMapCellID ) )
				{	// v okolí MapCellu není vìtší kruh
					// zpracuje okolní MapCelly
					for ( int i = 8; i-- > 0; )
					{
						// ukazatel na sousední MapCell
						signed char *pMapCell = pPosition + m_aSurroundingMapCellOffset[i];
						// zjistí, jedná-li se o hledaný MapCell
						if ( *pMapCell == cMapCellID )
						{	// jedná se o hledaný MapCell
							// oznaèí MapCell
							*pMapCell = cMarkedMapCellID;
							// pøidá do fronty blok MapCellu
							m_cFindAreaBridgeMapCellQueue.Add ( pMapCell );
						}
						// nejedná se o hledaný MapCell
					}
					// okolní MapCelly byly zpracovány
				}
				else
				{	// v okolí MapCellu je vìtší kruh
					// zpracuje okolní MapCelly
					for ( int i = 0; i < 8; i++ )
					{
						// ukazatel na sousední MapCell
						signed char *pMapCell = pPosition + m_aSurroundingMapCellOffset[i];
						// zjistí, jedná-li se o hledaný MapCell
						if ( *pMapCell == cMapCellID )
						{	// jedná se o hledaný MapCell
							// oznaèí MapCell
							*pMapCell = cMarkedMapCellID;
							// pøidá MapCell do zásobníku doèasnì oznaèených MapCellù
							m_cFindAreaBridgeTemporarilyMarkedMapCellStack.Push ( pMapCell );
						}
						else if ( *pMapCell == cMarkedMapCellID )
						{	// jedná se o oznaèený MapCell
							// odebere z fronty blok MapCellu
							(void)m_cFindAreaBridgeMapCellQueue.RemoveElement ( pMapCell );
						}
						else if ( *pMapCell > cMapCellAreaID )
						{	// jedná se o MapCell vìtšího kruhu
							ASSERT ( *pMapCell != GetMapCellAreaIDFromMapCellID ( 
								cMapCellID ) );

							// projede okolní MapCelly
							for ( int j = 8; j-- > 0; )
							{
								// ukazatel na oznaèovaný MapCell
								signed char *pMarkedMapCell = pMapCell + 
									m_aSurroundingMapCellOffset[j];

								// zjistí, jedná-li se o hledaný MapCell
								if ( *pMarkedMapCell == cMapCellID )
								{	// jedná se o hledaný MapCell
									// oznaèí MapCell
									*pMarkedMapCell = cMarkedMapCellID;
									// pøidá MapCell do zásobníku doèasnì oznaèených MapCellù
									m_cFindAreaBridgeTemporarilyMarkedMapCellStack.Push ( 
										pMarkedMapCell );
								}
								else if ( *pMarkedMapCell == cMarkedMapCellID )
								{	// jedná se o již oznaèený MapCell
									// odebere z fronty blok MapCellu
									(void)m_cFindAreaBridgeMapCellQueue.RemoveElement ( 
										pMarkedMapCell );
								}
								// MapCell byl zpracován
							}
							// okolní MapCelly byly zpracovány

							// zjistí, jedná-li se o první vìtší sousední oblast
							if ( pNeighbourBiggerArea == NULL )
							{	// jedná se o první vìtší sousední oblast
								// zjistí, jedná-li se o pokraèování souvislé oblasti vìtších kruhù
								if ( ( *(pPosition + m_aContinuousMapCellOffset[i][1]) <= 
									cMapCellID ) && ( ( m_aContinuousMapCellOffset[i][0] == 0 ) || 
									( *(pPosition + m_aContinuousMapCellOffset[i][0]) <= 
									cMapCellID ) ) )
								{	// nejedná se o pokraèování souvislé oblasti vìtších kruhù
									// ukazatel na nejvìtší kruh v okolí MapCellu "pMapCell"
									signed char *pMaximumMapCell = pMapCell;

									// najde v souvislé oblasti okolních MapCellù nejvìtší kruh
									for ( int j = i + 1; ; j++ )
									{
										// zjistí index MapCellu
										j &= 0x07;		// j %= 8
										ASSERT ( i != j );
										// zjistí, je-li MapCell pokraèováním souvislé oblasti 
										//		vìtších MapCellù
										if ( ( *(pPosition + m_aContinuousMapCellOffset[j][1]) <= 
											cMapCellID ) && ( ( m_aContinuousMapCellOffset[j][0] == 0 ) 
											|| ( *(pPosition + m_aContinuousMapCellOffset[j][0]) <= 
											cMapCellID ) ) )
										{	// nejedná se o pokraèování souvislé oblasti vìtších kruhù
											// ukonèí hledání nejvìtšího kruhu v souvislé oblasti 
											//		okolních MapCellù
											break;
										}

										// zjistí, jedná-li se o nejvìtší kruh
										if ( *pMaximumMapCell < *(pPosition + 
											m_aSurroundingMapCellOffset[j]) )
										{	// jedná se o nejvìtší kruh
											// aktualizuje ukazatel na nejvìtší kruh
											pMaximumMapCell = pPosition + 
												m_aSurroundingMapCellOffset[j];
										}
									}
									// byl nalezen nejvìtší kruh v okolí MapCellu

									// aktualizuje ukazatel na sousední vìtší oblast
									pNeighbourBiggerArea = pMaximumMapCell;
								}
								// jedná se o pokraèování souvislé oblasti vìtších kruhù
							}
							else
							{	// nejedná se o první sousední vìtší oblast
#ifdef _DEBUG
								// zjistí, jedná-li se o pokraèování souvislé oblasti vìtších kruhù
								if ( ( *(pPosition + m_aContinuousMapCellOffset[i][1]) <= 
									cMapCellID ) && ( ( m_aContinuousMapCellOffset[i][0] == 0 ) || 
									( *(pPosition + m_aContinuousMapCellOffset[i][0]) <= 
									cMapCellID ) ) )
								{	// nejedná se o pokraèování souvislé oblasti vìtších kruhù
									// pøíznak více vìtších sousedních oblastí
									bMultiNeighbourBiggerArea = TRUE;
								}
#endif //_DEBUG
							}
							// sousední vìtší oblast byla zpracována
						}
						else if ( *pMapCell == cMapCellAreaID )
						{	// jedná se o MapCell oblasti
							// vyprázdní frontu MapCellù
							m_cFindAreaBridgeMapCellQueue.RemoveAll ();
							// aktualizuje ukazatel na MapCell
							pPosition = pMapCell;
							// ukonèí zpracovávání okolních MapCellù
							break;
						}
						// jedná se o MapCell menšího kruhu
					}
					// okolní MapCelly byly zpracovány
				}
				// okolí MapCellu bylo zpracováno
			}
			// fronta je zpracována

			// ukazatel na oznaèený MapCell
			signed char *pMarkedMapCell;
			// odznaèí doèasnì oznaèené MapCelly
			while ( m_cFindAreaBridgeTemporarilyMarkedMapCellStack.Pop ( pMarkedMapCell ) )
			{
				ASSERT ( *pMarkedMapCell == cMarkedMapCellID );
				// odznaèí oznaèený MapCell
				*pMarkedMapCell = cMapCellID;
			}

			// odznaèí první MapCell
			ASSERT ( *pStartPosition == cMarkedMapCellID );
			*pStartPosition = cMapCellID;
			// pøidá do fronty blok prvního MapCellu
			m_cFindAreaBridgeMapCellQueue.Add ( pStartPosition );

			// zpracuje frontu odznaèovaných MapCellù
			while ( m_cFindAreaBridgeMapCellQueue.RemoveFirst ( pStartPosition ) )
			{
				ASSERT ( *pStartPosition == cMapCellID );

				// zpracuje okolní MapCelly
				for ( int i = 8; i-- > 0; )
				{
					// ukazatel na sousední MapCell
					signed char *pMapCell = pStartPosition + m_aSurroundingMapCellOffset[i];
					// zjistí, jedná-li se o oznaèný MapCell
					if ( *pMapCell == cMarkedMapCellID )
					{	// jedná se o oznaèený MapCell
						// odoznaèí MapCell
						*pMapCell = cMapCellID;
						// pøidá MapCell do fronty
						m_cFindAreaBridgeMapCellQueue.Add ( pMapCell );
					}
					// nejedná se o oznaèený MapCell
				}
				// okolní MapCelly byly zpracovány
			}
			// fronta odznaèovaných MapCellù je zpracována

			// zjistí, jedná-li se o MapCell oblasti
			if ( *pPosition == cMapCellAreaID )
			{	// jedná se o MapCell oblasti
				// ukonèí hledání MapCellu oblasti
				break;
			}
			else
			{	// nejedná se o MapCell oblasti
				ASSERT ( pNeighbourBiggerArea != NULL );
				ASSERT ( !bMultiNeighbourBiggerArea );
				// nechá najít MapCell oblasti v sousední vìtší oblasti
				pPosition = pNeighbourBiggerArea;
			}
		}
		else
		{	// jedná se o MapCell oblasti
			// ukonèí hledání MapCellu oblasti
			break;
		}
	}
	// jedná se o MapCell oblasti nebo nejvìtšího kruhu

	// zjistí, jedná-li se o MapCell nejvìtšího kruhu
	if ( *pPosition == GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ) )
	{	// jedná se o MapCell nejvìtšího kruhu
		// ID MapCellu nejvìtšího kruhu
		signed char cMaxMapCellID = GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE );
		// zjistí ID MapCellu oblasti nejvìtších kruhù
		signed char cMapCellAreaID = GetMapCellAreaIDFromCircleSize ( MAX_CIRCLE_SIZE );

		// index vnìjšího MapCellu
		int nOutsideMapCellIndex = 0;

		// najde vnìjší MapCell
		for ( ; nOutsideMapCellIndex < 8; nOutsideMapCellIndex++ )
		{
			// zjistí, jedná-li se o vnìjší MapCell
			if ( *(pPosition + m_aSurroundingMapCellOffset[nOutsideMapCellIndex]) < 
				cMaxMapCellID )
			{	// jedná se o vnìjší MapCell
				// ukonèí hledání vnìjšího MapCellu
				break;
			}
		}
		// byl nalezen vnìjší MapCell
		ASSERT ( nOutsideMapCellIndex < 8 );

		ASSERT ( *pPosition == cMaxMapCellID );
		// doèasnì oznaè
		*pPosition = cMapCellAreaID;
		signed char *pStartPosition = pPosition;

		// najde MapCell oblasti nejvìtších kruhù
		do
		{
			ASSERT ( ( ( *pPosition == cMaxMapCellID ) && ( *(pPosition + 
				m_aSurroundingMapCellOffset[nOutsideMapCellIndex]) < cMaxMapCellID ) ) ||
				( *pPosition == cMapCellAreaID ) );

			// najde další MapCell oblasti nejvìtších kruhù
			while ( *(pPosition + m_aSurroundingMapCellOffset[nOutsideMapCellIndex]) < 
				cMaxMapCellID )
			{
				// nOutsideMapCellIndex = ( nOutsideMapCellIndex + 1 ) % 8
				nOutsideMapCellIndex = ( nOutsideMapCellIndex + 1 ) & 0x07;
			}

			// pøesune se na další MapCell oblasti nejvìtších kruhù
			pPosition += m_aSurroundingMapCellOffset[nOutsideMapCellIndex];
			// nOutsideMapCellIndex = ( ( ( nOutsideMapCellIndex + 1 ) / 2 ) * 2 + 5 ) % 8
			nOutsideMapCellIndex = ( ( ( nOutsideMapCellIndex + 1 ) & ~0x01 ) + 5 ) & 0x07;
		} while ( *pPosition != cMapCellAreaID );
		// byl nalezen MapCell oblasti nejvìtších kruhù

		// smaže znaèku
		*pStartPosition = cMaxMapCellID;

		if ( pPosition == pStartPosition )
		{
			signed char cMarkedMaxMapCellID = MARK_EXPLORED ( cMaxMapCellID );
			// pustí flood fill
			// ten najde representanta a oznackuje celou oblast
			CPoint ptPosition = GetMapCellPosition ( pStartPosition );
			pPosition = FindAreaRepresentative ( cMaxMapCellID, cMarkedMaxMapCellID, ptPosition.x,
				ptPosition.y, cMapCellAreaID );
			ASSERT ( pPosition != NULL );
			// Ted ještì odoznaèit celou oblast
			// Pustím teda ten floodfill znova, akorat obracenì a zapomenu výsledek ( mìl by to bý ten samý )
			VERIFY ( FindAreaRepresentative ( cMarkedMaxMapCellID, cMaxMapCellID, ptPosition.x,
				ptPosition.y, cMapCellAreaID ) == pPosition );
		}
	}
	// byl nalezen MapCell oblasti/mostu

	ASSERT ( *pPosition > 0 );
	ASSERT ( *pPosition <= GetMapCellAreaIDFromCircleSize ( MAX_CIRCLE_SIZE ) );
	ASSERT ( *pPosition == GetMapCellAreaIDFromMapCellID ( *pPosition ) );

	// zjistí souøadnice MapCellu oblasti/mostu
	int nX = GetMapCellPosition ( pPosition ).x;
	int nY = GetMapCellPosition ( pPosition ).y;

	// pozice v seznamu oblastí/mostù
	POSITION posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();
	// ukazatel na oblast/most
	struct SAreaBridge *pAreaBridge;

	// najde záznam oblasti/mostu "pPosition"
	for ( ; ; )
	{
		VERIFY ( m_cAreaBridgeList.GetNext ( posAreaBridge, pAreaBridge ) );

		// zjistí, jedná-li se o hledanou oblast/most
		if ( ( pAreaBridge->pointPosition.x == nX ) && 
			( pAreaBridge->pointPosition.y == nY ) )
		{	// jedná se o hledanou oblast/most
			ASSERT ( pAreaBridge->cMapCellID == GetMapCellIDFromMapCellAreaID ( *pPosition ) );
			// vrátí ukazatel na nalezenou oblast/most
			return pAreaBridge;
		}
	}
}

// vyplní vzdálenosti od brány mostu "pBridgeGate" k branám mostù "cBridgeGateList" 
//		oblasti "pAreaBridge" do tabulky vzdáleností bran mostù "pBridgeGateDistances"
void CCFPGMap::GetBridgeGateDistances ( struct SAreaBridge *pAreaBridge,
	struct SBridgeGate *pBridgeGate, CBridgeGateList &cBridgeGateList, 
	DWORD *pBridgeGateDistances ) 
{
	ASSERT ( !cBridgeGateList.IsEmpty () );
	ASSERT ( m_cPriorityMapCellQueue.IsEmpty () );

	// pozice prvního MapCellu brány mostu oblasti
	CPoint pointBridgeGatePosition;
	// vzdálenost prvního MapCellu brány mostu oblasti
	DWORD dwBridgeGateDistance;

	// zjistí, jedná-li se o primární bránu mostu oblasti
	if ( pAreaBridge->cPrimaryBridgeGateList.Find ( pBridgeGate ) != NULL )
	{	// jedná se o primární bránu mostu oblasti
		// inicializuje první MapCell brány mostu na bránu mostu
		pointBridgeGatePosition = pBridgeGate->pointPosition;
		dwBridgeGateDistance = 0;
	}
	else
	{	// jedná se o sekundární bránu mostu oblasti
		ASSERT ( pAreaBridge->cSecondaryBridgeGateList.Find ( pBridgeGate ) != NULL );
		// inicializuje první MapCell brány mostu na dotyk brány mostu
		pointBridgeGatePosition = pBridgeGate->pointTouchPosition;
		dwBridgeGateDistance = pBridgeGate->dwTouchDistance;
	}

	// ID MapCellu oblasti
	signed char cAreaID = pAreaBridge->cMapCellID;
	// ID oznaèeného MapCellu oblasti
	signed char cMarkedAreaID = MARK_EXPLORED ( cAreaID );
	ASSERT ( cAreaID != cMarkedAreaID );

	// vymaže pole vzdáleností
	m_cMapCellDistanceArray.Clear ();

	// ukazatel na první MapCell brány mostu oblasti
	signed char *pBridgeGateMapCell = GetAt ( pointBridgeGatePosition );
	// zjistí, jedná-li se o MapCell oblasti
	if ( *pBridgeGateMapCell == cAreaID )
	{	// jedná se o MapCell oblasti
		// oznaèí první MapCell brány mostu oblasti
		*pBridgeGateMapCell = cMarkedAreaID;
	}

	// inicializuje vzdálenost dotyku výchozí brány mostu
	m_cMapCellDistanceArray.GetAt ( pointBridgeGatePosition ) = 0;
	// pøidá MapCell do prioritní fronty
	m_cPriorityMapCellQueue.Add ( 0, pBridgeGateMapCell );

	// popis MapCellu v prioritní frontì
	DWORD dwMapCellPriorityQueuePriority;
	signed char *pMapCellPriorityQueueMapCell;

	// zpracuje prioritní frontu
	while ( m_cPriorityMapCellQueue.RemoveFirst ( dwMapCellPriorityQueuePriority, 
		pMapCellPriorityQueueMapCell ) )
	{
		// projede okolní MapCelly
		for ( int nIndex = 8; nIndex-- > 0; )
		{
			// ukazatel na sousední MapCell
			signed char *pMapCell = pMapCellPriorityQueueMapCell + 
				m_aSurroundingMapCellOffset[nIndex];
			// zjistí, jedná-li se o MapCell oblasti
			if ( *pMapCell == cAreaID )
			{	// jedná se o MapCell mostu
				// oznaèí MapCell
				*pMapCell = cMarkedAreaID;
				// zjistí vzdálenost MapCellu
				DWORD dwDistance = dwMapCellPriorityQueuePriority + ( ( nIndex & 0x01 ) ? 10 : 
					14 );
				// pøidá MapCell do prioritní fronty
				m_cPriorityMapCellQueue.Add ( dwDistance, pMapCell );
				// zjistí pozici MapCellu
				CPoint pointPosition = GetMapCellPosition ( pMapCell );
				// zapíše vzdálenost MapCellu do pole
				m_cMapCellDistanceArray.GetAt ( pointPosition ) = dwDistance;
			}
			// nejedná se o MapCell oblasti
		}
		// zpracoval okolní MapCelly
	}
	// zpracoval prioritní frontu

	ASSERT ( m_cTemporarilyMarkedMapCellStack.IsEmpty () );

	// oznaèený MapCell
	signed char *pMapCell;
	// pøídá první MapCell brány mostu do zásobníku
	m_cTemporarilyMarkedMapCellStack.Push ( pBridgeGateMapCell );
	// zjistí, jedná-li se o oznaèený MapCell oblasti
	if ( *pBridgeGateMapCell == cMarkedAreaID )
	{	// jedná se o MapCell oblasti
		// oznaèí první MapCell brány mostu oblasti
		*pBridgeGateMapCell = cAreaID;
	}
	ASSERT ( *pBridgeGateMapCell == GetMapCellIDFromMapCellAreaID ( 
		*pBridgeGateMapCell ) );
	// odznaèí doèasnì oznaèené MapCelly
	while ( m_cTemporarilyMarkedMapCellStack.Pop ( pMapCell ) )
	{
		// projede okolní MapCelly
		for ( int nIndex = 8; nIndex-- > 0; )
		{
			// ukazatel na sousední MapCell
			signed char *pSurroundingMapCell = pMapCell + 
				m_aSurroundingMapCellOffset[nIndex];
			// zjistí, jedná-li se o oznaèený MapCell
			if ( *pSurroundingMapCell == cMarkedAreaID )
			{	// jedná se o oznaèený MapCell
				// odznaèí doèasnì oznaèený MapCell
				*pSurroundingMapCell = cAreaID;
				// pøidá MapCell do zásobníku
				m_cTemporarilyMarkedMapCellStack.Push ( pSurroundingMapCell );
			}
			// nejedná se o oznaèený MapCell
		}
		// zpracoval okolní MapCelly
	}
	// odznaèil doèasnì oznaèené MapCelly

	// pozice další brány mostu v seznamu bran mostù oblasti
	POSITION posNextBridgeGate = cBridgeGateList.GetHeadPosition ();
	// další brána mostu oblasti
	struct SBridgeGate *pNextBridgeGate;
	// aktualizuje vzdálenosti bran mostù
	while ( CBridgeGateList::GetNext ( posNextBridgeGate, pNextBridgeGate ) )
	{
		// pozice prvního MapCellu brány mostu oblasti
		CPoint pointNextBridgeGatePosition;
		// vzdálenost prvního MapCellu brány mostu oblasti
		DWORD dwNextBridgeGateDistance;

		// zjistí, jedná-li se o primární bránu mostu oblasti
		if ( pAreaBridge->cPrimaryBridgeGateList.Find ( pNextBridgeGate ) != NULL )
		{	// jedná se o primární bránu mostu oblasti
			// inicializuje první MapCell brány mostu na bránu mostu
			pointNextBridgeGatePosition = pNextBridgeGate->pointPosition;
			dwNextBridgeGateDistance = 0;
		}
		else
		{	// jedná se o sekundární bránu mostu oblasti
			ASSERT ( pAreaBridge->cSecondaryBridgeGateList.Find ( pNextBridgeGate ) != NULL );
			// inicializuje první MapCell brány mostu na dotyk brány mostu
			pointNextBridgeGatePosition = pNextBridgeGate->pointTouchPosition;
			dwNextBridgeGateDistance = pNextBridgeGate->dwTouchDistance;
		}

		// ukazatel na první MapCell brány mostu oblasti
		signed char *pNextBridgeGateMapCell = GetAt ( pointNextBridgeGatePosition );

		// vzdálnost dotyku brány
		DWORD &rdwNextBridgeGateTouchDistance = m_cMapCellDistanceArray.GetAt ( 
			pointNextBridgeGatePosition );
		// zjistí, je-li dotyk brány mostu dosažitelný pøímo
		if ( rdwNextBridgeGateTouchDistance == INFINITE_DISTANCE )
		{	// dotyk brány není dosažitelný pøímo
			// zjistí vzdálenost dotyku brány mostu
			for ( int nIndex = 8; nIndex--; )
			{
				// ukazatel na sousední MapCell
				signed char *pSurroundingMapCell = pNextBridgeGateMapCell + 
					m_aSurroundingMapCellOffset[nIndex];
				// spoèítá vzdálenost pøes sousední MapCell
				DWORD dwSurroundingMapCellDistance = m_cMapCellDistanceArray.GetAt ( 
					GetMapCellPosition ( pSurroundingMapCell ) ) + ( ( nIndex & 0x01 ) ? 10 : 
					14 );
				// zjistí, jedná-li se o kratší vzdálenost
				if ( dwSurroundingMapCellDistance < rdwNextBridgeGateTouchDistance )
				{	// jedná se o kratší vzdálenost
					// aktualizuje vzdálenost dotyku brány mostu
					rdwNextBridgeGateTouchDistance = dwSurroundingMapCellDistance;
				}
			}
			// zjistil vzdálenost dotyku brány mostu
			ASSERT ( rdwNextBridgeGateTouchDistance < INFINITE_DISTANCE );
		}
		// dotyk brány je dosažitelný pøímo

		// aktualizuje vzdálenost bran mostù
		*(pBridgeGateDistances++) = ( rdwNextBridgeGateTouchDistance + 
			dwNextBridgeGateDistance + dwBridgeGateDistance + 5 ) / 10;
		ASSERT ( *(pBridgeGateDistances - 1) < INFINITE_DISTANCE );

		// koncový zpracovávaný bod  cesty
		CPoint pointEnd = pointNextBridgeGatePosition;
		// poslední zpracovaný bod cesty
		CPoint pointLastPosition = pointNextBridgeGatePosition;
		// vzdálenost posledního zpracovaného bodu cesty
		DWORD dwLastPositionDistance = m_cMapCellDistanceArray.GetAt ( 
			pointNextBridgeGatePosition );

		// fronta pozic kontrolních bodù hintu cesty
		static CSelfPooledList<CPoint> cCheckPointPositionList ( 100 );
		ASSERT ( cCheckPointPositionList.IsEmpty () );

		// pøevede cestu na kontrolní body
		while ( dwLastPositionDistance != 0 )
		{
			// další bod cesty
			CPoint pointNextPosition;
			// vzdálenost dalšího bodu cesty
			DWORD dwNextPositionDistance;

			// najde další bod cesty
			for ( int nIndex = 8; nIndex-- > 0; )
			{
				// zjistí sousední pozici na mapì
				pointNextPosition = pointLastPosition + m_aSurroundingPositionOffset[nIndex];

				// vzdálenost sousední pozice na mapì
				dwNextPositionDistance = m_cMapCellDistanceArray.GetAt ( pointNextPosition );

				// zjistí, jedná-li se o prozkoumanou pozici na mapì
				if ( dwNextPositionDistance == INFINITE_DISTANCE )
				{	// jedná se o neprozkoumanou pozici na mapì
					// ukonèí zpracovávání sousední pozice
					continue;
				}
				ASSERT ( dwNextPositionDistance < INFINITE_DISTANCE );

				ASSERT ( ( dwNextPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) >= 
					dwLastPositionDistance );

				// zjistí, jedná-li se o následující bod cesty
				if ( ( dwNextPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) == 
					dwLastPositionDistance )
				{	// jedná se o následující bod cesty
					// nechá zpracovat bod cesty
					break;
				}
			}
			// byl nalezen další bod cesty
			ASSERT ( nIndex >= 0 );

		// zjistí, existuje-li úseèka mezi nelezeným dalším bodem a koncem cesty

			// vzdálenost koncù úseèky v jednotlivých osách
			CSize sizeDistance ( abs ( pointEnd.x - pointNextPosition.x ), 
				abs ( pointEnd.y - pointNextPosition.y ) );

			// zjistí, je-li úseèka pøíliš dlouhá
			if ( ( sizeDistance.cx * sizeDistance.cx + sizeDistance.cy * sizeDistance.cy ) >= 
				MAX_CHECK_POINT_DISTANCE * MAX_CHECK_POINT_DISTANCE )
			{	// úseèka je pøíliš dlouhá
				// pøidá kontrolní bod koncového bodu cesty
				cCheckPointPositionList.Add ( pointLastPosition );
				// aktualizuje konec cesty
				pointEnd = pointLastPosition;
				// aktualizuje vzdálenost bodù úseèky v jednotlivých osách
				sizeDistance = CSize ( abs ( pointEnd.x - pointNextPosition.x ), 
				abs ( pointEnd.y - pointNextPosition.y ) );
			}

			// zjistí, ve kterém rozmìru je vzdálenost koncù úseèky vìtší
			if ( sizeDistance.cy < sizeDistance.cx )
			{	// vzdálenost koncù úseèky je vìtší v ose X
				// pozice bodu úseèky
				CPoint pointPosition;

				// pozice bodu úseèky v ose X
				pointPosition.x = pointNextPosition.x;
				// pøírùstek v ose X
				int nXDifference = ( pointNextPosition.x < pointEnd.x ) ? 1 : -1;

				// pøesná pozice bodu úseèky v ose Y
				double dbYPosition = (double)pointNextPosition.y + 0.5;
				// pøesný pøírùstek v ose Y
				double dbYDifference = ( (double)pointEnd.y - (double)pointNextPosition.y ) / 
					(double)sizeDistance.cx;

				ASSERT ( pointPosition.x != pointEnd.x );

				// "nulty" krok iterace, osa X je implicitne posunuta o "pul mapcellu",
				// na ose y pridame polovinu dbYDifference k prvni hranici mapcellu
				pointPosition.x += nXDifference;
				dbYPosition += dbYDifference / 2;
				// pozice bodu úseèky v ose Y
				pointPosition.y = (int)dbYPosition;

				// zkontroluje MapCelly bodù úseèky
				while ( pointPosition.x != pointEnd.x )
				{
				// zkontroluje první okraj MapCellu úseèky

					// vzdálenost bodu úseèky
					DWORD dwPositionDistance = m_cMapCellDistanceArray.GetAt ( pointPosition );

					// zjistí, je-li MapCell bodu úseèky povolený
					if ( dwPositionDistance == INFINITE_DISTANCE )
					{	// MapCell bodu úseèky je zakázáný nebo dosud nenavštívený
						// pøidá kontrolní bod koncového bodu cesty
						cCheckPointPositionList.Add ( pointLastPosition );
						// aktualizuje konec cesty
						pointEnd = pointLastPosition;
						// ukonèí kontrolu MapCellù bodù úseèky
						break;
					}
					// MapCell bodu úseèky je povolený
					ASSERT ( dwPositionDistance < INFINITE_DISTANCE );

				// posune pozici na druhý okraj MapCellu

					// posune pozici na druhý okraj MapCellu
					dbYPosition += dbYDifference;
					// pozice bodu úseèky v ose Y
					pointPosition.y = (int)dbYPosition;

				// zkontroluje druhý okraj MapCellu úseèky

					// aktualizuje vzdálenost bodu úseèky
					dwPositionDistance = m_cMapCellDistanceArray.GetAt ( pointPosition );

					// zjistí, je-li MapCell bodu úseèky povolený
					if ( dwPositionDistance == INFINITE_DISTANCE )
					{	// MapCell bodu úseèky je zakázáný nebo dosud nenavštívený
						// pøidá kontrolní bod koncového bodu cesty
						cCheckPointPositionList.Add ( pointLastPosition );
						// aktualizuje konec cesty
						pointEnd = pointLastPosition;
						// ukonèí kontrolu MapCellù bodù úseèky
						break;
					}
					// MapCell bodu úseèky je povolený
					ASSERT ( dwPositionDistance < INFINITE_DISTANCE );

				// posune se na další bod úseèky

					// posune se na další bod úseèky
					pointPosition.x += nXDifference;
				}
				// MapCelly bodù úseèky jsou zkontrolovány
			}
			else if ( sizeDistance.cy > sizeDistance.cx )
			{	// vzdálenost koncù úseèky je vìtší v ose Y
				// pozice bodu úseèky
				CPoint pointPosition;

				// pozice bodu úseèky v ose Y
				pointPosition.y = pointNextPosition.y;
				// pøírùstek v ose Y
				int nYDifference = ( pointNextPosition.y < pointEnd.y ) ? 1 : -1;

				// pøesná pozice v ose X
				double dbXPosition = (double)pointNextPosition.x + 0.5;
				// pøesný pøírùstek v ose X
				double dbXDifference = ( (double)pointEnd.x - (double)pointNextPosition.x ) / 
					(double)sizeDistance.cy;

				ASSERT ( pointPosition.y != pointEnd.y );

				// "nulty" krok iterace, osa Y je implicitne posunuta o "pul mapcellu",
				// na ose X pridame polovinu dbXDifference k prvni hranici mapcellu
				pointPosition.y += nYDifference;
				dbXPosition += dbXDifference / 2;
				// pozice bodu úseèky v ose X
				pointPosition.x = (int)dbXPosition;

				// zkontroluje MapCelly bodù úseèky
				while ( pointPosition.y != pointEnd.y )
				{
				// zkontroluje první okraj MapCellu úseèky

					// vzdálenost bodu úseèky
					DWORD dwPositionDistance = m_cMapCellDistanceArray.GetAt ( 
						pointPosition );

					// zjistí, je-li MapCell bodu úseèky povolený
					if ( dwPositionDistance == INFINITE_DISTANCE )
					{	// MapCell bodu úseèky je zakázáný nebo dosud nenavštívený
						// pøidá kontrolní bod koncového bodu cesty
						cCheckPointPositionList.Add ( pointLastPosition );
						// aktualizuje konec cesty
						pointEnd = pointLastPosition;
						// ukonèí kontrolu MapCellù bodù úseèky
						break;
					}
					// MapCell bodu úseèky je povolený
					ASSERT ( dwPositionDistance < INFINITE_DISTANCE );

				// posune pozici na druhý okraj MapCellu

					// posune pozici na druhý okraj MapCellu
					dbXPosition += dbXDifference;
					// pozice bodu úseèky v ose X
					pointPosition.x = (int)dbXPosition;

				// zkontroluje druhý okraj MapCellu úseèky
					// aktualizuje vzdálenost bodu úseèky
					dwPositionDistance = m_cMapCellDistanceArray.GetAt ( 
						pointPosition );

					// zjistí, je-li MapCell bodu úseèky povolený
					if ( dwPositionDistance == INFINITE_DISTANCE )
					{	// MapCell bodu úseèky je zakázáný nebo dosud nenavštívený
						// pøidá kontrolní bod koncového bodu cesty
						cCheckPointPositionList.Add ( pointLastPosition );
						// aktualizuje konec cesty
						pointEnd = pointLastPosition;
						// ukonèí kontrolu MapCellù bodù úseèky
						break;
					}
					// MapCell bodu úseèky je povolený
					ASSERT ( dwPositionDistance < INFINITE_DISTANCE );

				// posune se na další bod úseèky

					// posune se na další bod úseèky
					pointPosition.y += nYDifference;
				}
				// MapCelly bodù úseèky jsou zkontrolovány
			}
			else
			{	// vzdálenosti koncù úseèky jsou stejné v ose X i Y
				// pozice bodu úseèky
				CPoint pointPosition = pointNextPosition;
				// pøírùstek v osách X a Y
				CPoint pointDifference ( ( pointNextPosition.x < pointEnd.x ) ? 1 : -1, 
					( pointNextPosition.y < pointEnd.y ) ? 1 : -1 );

				ASSERT ( pointNextPosition != pointEnd );

				// posune se na další MapCell
				pointPosition += pointDifference;

				// zkontroluje MapCelly bodù úseèky
				while ( pointPosition != pointEnd )
				{
					// vzdálenost bodu úseèky
					DWORD dwPositionDistance = m_cMapCellDistanceArray.GetAt ( pointPosition );

					// zjistí, je-li MapCell bodu úseèky povolený
					if ( dwPositionDistance == INFINITE_DISTANCE )
					{	// MapCell bodu úseèky je zakázáný nebo dosud nenavštívený
						// pøidá kontrolní bod koncového bodu cesty
						cCheckPointPositionList.Add ( pointLastPosition );
						// aktualizuje konec cesty
						pointEnd = pointLastPosition;
						// ukonèí kontrolu MapCellù bodù úseèky
						break;
					}
					// MapCell bodu úseèky je povolený
					ASSERT ( dwPositionDistance < INFINITE_DISTANCE );

					// posune se na další MapCell
					pointPosition += pointDifference;
				}
				// MapCelly bodù úseèky jsou zkontrolovány
			}
			// úseèka je zpracována

			// aktualizuje poslední zpracovaný bod cesty
			pointLastPosition = pointNextPosition;
			// aktualizuje vzdálenost posledního zpracovaného bodu cesty
			dwLastPositionDistance = dwNextPositionDistance;
		}
		// cesta byla pøevedena na kontrolní body

		// zjistí poèet kontrolních bodù hintu cesty
		DWORD dwCheckPointCount = cCheckPointPositionList.GetSize ();

		// zjistí, je-li seznam kontrolních bodù hintu cesty prázdná
		if ( dwCheckPointCount > 0 )
		{	// seznam kontrolních bodù cesty není prázdný
			// uschová index nového hintu cesty
			DWORD dwPathHintIndex = m_dwPathHintTableSize;

			// aktualizuje hinty cesty mezi branami mostù
			*(pBridgeGateDistances++) = dwPathHintIndex;

			// zvìtší pole hintù cest
			IncreasePathHintTable ( 2 * dwCheckPointCount + 1 );
			// vyplní poèet kontrolních bodù hintu cesty
			m_pPathHintTable[dwPathHintIndex++] = dwCheckPointCount;

			// kontrolní bod hintu cesty
			CPoint pointCheckPoint;

			// vyplní kontrolní body hintu cesty
			while ( cCheckPointPositionList.RemoveFirst ( pointCheckPoint ) )
			{
				// pøeloží souøadnice kotnrolního bodu hintu cesty
				ASSERT ( pointCheckPoint.x >= MAP_BORDER );
				pointCheckPoint.x -= MAP_BORDER;
				ASSERT ( pointCheckPoint.x < m_nSizeX );
				ASSERT ( pointCheckPoint.y >= MAP_BORDER );
				pointCheckPoint.y -= MAP_BORDER;
				ASSERT ( pointCheckPoint.y < m_nSizeY );
				// uloží kontrolní bod hintu cesty do tabulky hintù cesty
				m_pPathHintTable[dwPathHintIndex++] = (DWORD)pointCheckPoint.x;
				m_pPathHintTable[dwPathHintIndex++] = (DWORD)pointCheckPoint.y;
			}
			ASSERT ( dwPathHintIndex == m_dwPathHintTableSize );
		}
		else
		{	// seznam kontrolních bodù cesty je prázdný
			// aktualizuje hinty cesty mezi branami mostù
			*(pBridgeGateDistances++) = INVALID_PATH_HINT;
		}

		TRACE_FIND_PATH_CREATION ( "GetAreaDistance %d ... (%d,%d)[%d,%d]->[%d,%d](%d,%d)\n", 
			*(pBridgeGateDistances - 2), pBridgeGate->pointPosition.x, 
			pBridgeGate->pointPosition.y, pointBridgeGatePosition.x, 
			pointBridgeGatePosition.y, pointNextBridgeGatePosition.x, 
			pointNextBridgeGatePosition.y, pNextBridgeGate->pointPosition.x, 
			pNextBridgeGate->pointPosition.y );
	}

	// ukonèí zjišování vzdáleností bran mostù oblasti
	return;
}

// najde dotyk brány mostu
void CCFPGMap::FindBridgeGateTouch ( struct SBridgeGate *pBridgeGate, 
	struct SAreaBridge *pAreaBridge ) 
{
	// ukazatel na první MapCell brány mostu
	signed char *pPosition = GetAt ( pBridgeGate->pointPosition );
	ASSERT ( *pPosition != MARK_EXPLORED ( *pPosition ) );

	ASSERT ( m_cPriorityMapCellQueue.IsEmpty () );
	ASSERT ( m_cTemporarilyMarkedMapCellStack.IsEmpty () );

	// ID hledaného MapCellu
	signed char cMapCellID = *pPosition;
	ASSERT ( cMapCellID != MARK_EXPLORED ( cMapCellID ) );

	// ID MapCellu hledané oblasti
	signed char cAreaID = pAreaBridge->cMapCellID;
	ASSERT ( cAreaID != MARK_EXPLORED ( cAreaID ) );
	ASSERT ( cAreaID != GetMapCellAreaIDFromMapCellID ( cAreaID ) );

	// zjistí, jedná-li se o MapCell hledané oblasti
	if ( cMapCellID < cAreaID )
	{	// nejedná se o MapCell hledané oblasti
		// ID oznaèeného MapCellu
		signed char cMarkedMapCellID = MARK_EXPLORED ( cMapCellID );

		// oznaèí první MapCell
		*pPosition = cMarkedMapCellID;
		// pøidá první MapCell do prioritní fronty
		m_cPriorityMapCellQueue.Add ( 0, pPosition );

		// priorita MapCellu v prioritní frontì
		DWORD dwPriority;

		// vyjme z prioritní fronty MapCell
		while ( m_cPriorityMapCellQueue.RemoveFirst ( dwPriority, pPosition ) )
		{
			ASSERT ( *pPosition == cMarkedMapCellID );

			// zjistí, je-li v okolí MapCellu vìtší kruh
			if ( ( *(pPosition - 1) <= cMapCellID ) && 
				( *(pPosition + 1) <= cMapCellID ) && 
				( *(pPosition - m_nMapSizeX - 1) <= cMapCellID ) && 
				( *(pPosition - m_nMapSizeX) <= cMapCellID ) && 
				( *(pPosition - m_nMapSizeX + 1) <= cMapCellID ) && 
				( *(pPosition + m_nMapSizeX - 1) <= cMapCellID ) && 
				( *(pPosition + m_nMapSizeX) <= cMapCellID ) && 
				( *(pPosition + m_nMapSizeX + 1) <= cMapCellID ) )
			{	// v okolí MapCellu není vìtší kruh
				// zpracuje okolní MapCelly
				for ( int nIndex = 8; nIndex-- > 0; )
				{
					// ukazatel na sousední MapCell
					signed char *pSurroundingMapCell = pPosition + 
						m_aSurroundingMapCellOffset[nIndex];

					// zjistí, jedná-li se o hledaný MapCell
					if ( *pSurroundingMapCell == cMapCellID )
					{	// jedná se o hledaný MapCell
						// oznaèí MapCell
						*pSurroundingMapCell = cMarkedMapCellID;
						// pøidá MapCell do prioritní fronty
						m_cPriorityMapCellQueue.Add ( dwPriority + 
							( ( nIndex & 0x01 ) ? 10 : 14 ), pSurroundingMapCell );
					}
					// nejedná se o hledaný MapCell
				}
				// okolní MapCelly byly zpracovány
			}
			else
			{	// v okolí MapCellu je vìtší kruh
				// ukazatel na nejvìtší kruh v okolí MapCellu
				signed char *pMaximumMapCell;
				// priorita nejvìtšího kruhu v okolí MapCellu
				DWORD dwMaximumMapCellPriority;
#ifdef _DEBUG
				// pøíznak nalezení nejvìtšího kruhu v okolí MapCellu
				BOOL bMaximumMapCellFound = FALSE;
#endif //_DEBUG

				// zpracuje okolní MapCelly
				for ( int i = 0; i < 8; i++ )
				{
					// ukazatel na sousední MapCell
					signed char *pMapCell = pPosition + m_aSurroundingMapCellOffset[i];
					// zjistí, jedná-li se o vìtší MapCell
					if ( *pMapCell > cMapCellID )
					{	// jedná se o MapCell vìtšího kruhu
						// zjistí, jedná-li se o pokraèování souvislé oblasti vìtších kruhù
						if ( ( *(pPosition + m_aContinuousMapCellOffset[i][1]) <= 
							cMapCellID ) && ( ( m_aContinuousMapCellOffset[i][0] == 0 ) || 
							( *(pPosition + m_aContinuousMapCellOffset[i][0]) <= 
							cMapCellID ) ) )
						{	// nejedná se o pokraèování souvislé oblasti vìtších kruhù
							ASSERT ( !bMaximumMapCellFound );

							// inicializuje ukazatel na nejvìtší kruh v okolí MapCellu "pMapCell"
							pMaximumMapCell = pMapCell;
							// inicializuje prioritu nejvìtšího kruhu v okolí MapCellu "pMapCell"
							dwMaximumMapCellPriority = dwPriority;

							// najde v souvislé oblasti okolních MapCellù nejvìtší kruh
							for ( int j = i + 1; ; j++ )
							{
								// zjistí index MapCellu
								j &= 0x07;		// j %= 8
								ASSERT ( i != j );
								// zjistí, je-li MapCell pokraèováním souvislé oblasti 
								//		vìtších MapCellù
								if ( ( *(pPosition + m_aContinuousMapCellOffset[j][1]) <= 
									cMapCellID ) && ( ( m_aContinuousMapCellOffset[j][0] == 0 ) 
									|| ( *(pPosition + m_aContinuousMapCellOffset[j][0]) <= 
									cMapCellID ) ) )
								{	// nejedná se o pokraèování souvislé oblasti vìtších kruhù
									// ukonèí hledání nejvìtšího kruhu v souvislé oblasti 
									//		okolních MapCellù
									break;
								}

								// zjistí, jedná-li se o nejvìtší kruh
								if ( *pMaximumMapCell < *(pPosition + 
									m_aSurroundingMapCellOffset[j]) )
								{	// jedná se o nejvìtší kruh
									// aktualizuje ukazatel na nejvìtší kruh
									pMaximumMapCell = pPosition + 
										m_aSurroundingMapCellOffset[j];
									// aktualizuje prioritu nejvìtšího kruhu
									dwMaximumMapCellPriority = dwPriority + ( ( j & 0x01 ) ? 10 : 
										14 );
								}
								else
								{	// nejedná se o nejvìtší kruh
									ASSERT ( ( *(pPosition + m_aSurroundingMapCellOffset[j]) == 
										MARK_EXPLORED ( *(pPosition + 
										m_aSurroundingMapCellOffset[j]) ) ) || !IsMapCellAreaID ( 
										*(pPosition + m_aSurroundingMapCellOffset[j]) ) );
								}
							}
							// byl nalezen nejvìtší kruh v okolí MapCellu

#ifdef _DEBUG
							// nastaví pøíznak nalezení nejvìtšího kruhu v okolí MapCellu
							bMaximumMapCellFound = TRUE;
#else //_DEBUG
							// ukonèí zpracovávání okolních MapCellù
							break;
#endif //!_DEBUG
						}
						// jedná se o pokraèování souvislé oblasti vìtších kruhù
					}
					// nejedná se o vìtší MapCell
				}
				// okolní MapCelly byly zpracovány

				ASSERT ( bMaximumMapCellFound );

				// smaže prioritní frontu MapCellù
				m_cPriorityMapCellQueue.RemoveAll ();

				// zjistí, jedná-li se o MapCell hledané oblasti
				if ( *pMaximumMapCell >= cAreaID )
				{	// jedná se o MapCell hledané oblasti
					ASSERT ( GetMapCellIDFromMapCellAreaID ( *pMaximumMapCell ) == cAreaID );

					// aktualizuje dotyk brány mostu
					pBridgeGate->pointTouchPosition = GetMapCellPosition ( pMaximumMapCell );
					pBridgeGate->dwTouchDistance = dwMaximumMapCellPriority;

					// ukazatel na první doèasnì oznaèený MapCell
					pPosition = GetAt ( pBridgeGate->pointPosition );
					// odznaèí první doèasnì oznaèený MapCell
					ASSERT ( *pPosition == MARK_EXPLORED ( *pPosition ) );
					*pPosition = UNMARK_EXPLORED ( *pPosition );
					// pøídá první MapCell brány mostu do zásobníku
					m_cTemporarilyMarkedMapCellStack.Push ( pPosition );
					// odznaèí doèasnì oznaèené MapCelly
					while ( m_cTemporarilyMarkedMapCellStack.Pop ( pPosition ) )
					{
						ASSERT ( *pPosition == UNMARK_EXPLORED ( *pPosition ) );
	
						// projede okolní MapCelly
						for ( int nIndex = 8; nIndex-- > 0; )
						{
							// ukazatel na sousední MapCell
							signed char *pSurroundingMapCell = pPosition + 
								m_aSurroundingMapCellOffset[nIndex];
							// zjistí, jedná-li se o oznaèený MapCell
							if ( *pSurroundingMapCell == MARK_EXPLORED ( *pSurroundingMapCell ) )
							{	// jedná se o oznaèený MapCell
								// odznaèí doèasnì oznaèený MapCell
								*pSurroundingMapCell = UNMARK_EXPLORED ( *pSurroundingMapCell );
								// pøidá MapCell do zásobníku MapCellù
								m_cTemporarilyMarkedMapCellStack.Push ( pSurroundingMapCell );
							}
							// nejedná se o oznaèený MapCell
						}
						// zpracoval okolní MapCelly
					}
					// odznaèil doèasnì oznaèené MapCelly

					// ukonèí hledání dotyku brány mostu
					return;
				}
				// nejedná se o MapCell hledané oblasti

				// aktualizuje ID hledaného MapCellu
				cMapCellID = *pMaximumMapCell;
				ASSERT ( cMapCellID != MARK_EXPLORED ( cMapCellID ) );
				ASSERT ( cMapCellID == GetMapCellIDFromMapCellAreaID ( cMapCellID ) );

				// aktualizuje ID oznaèeného MapCellu
				cMarkedMapCellID = MARK_EXPLORED ( cMapCellID );

				// oznaèí nejvìtší okolní MapCell
				*pMaximumMapCell = cMarkedMapCellID;
				// pøidá nejvìtší okolní MapCell do prioritní fronty
				m_cPriorityMapCellQueue.Add ( dwMaximumMapCellPriority, pMaximumMapCell );
			}
			// okolí MapCellu bylo zpracováno
		}
		// fronta je zpracována - nemožný pøípad
		ASSERT ( FALSE );
	}
	else
	{	// jedná se o MapCell hledané oblasti
		ASSERT ( GetMapCellIDFromMapCellAreaID ( cMapCellID ) == cAreaID );

		// aktualizuje dotyk brány mostu
		pBridgeGate->pointTouchPosition = pBridgeGate->pointPosition;
		pBridgeGate->dwTouchDistance = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// Informace o mapì tøídy CCFPGMap
//////////////////////////////////////////////////////////////////////

// vrátí ukazatel na MapCell na souøadnicích "nX", "nY" v souøadnicích mapy s okraji
inline signed char *CCFPGMap::GetAt ( int nX, int nY ) 
{
	ASSERT ( ( nX >= 0 ) && ( nX < m_nMapSizeX ) );
	ASSERT ( ( nY >= 0 ) && ( nY < m_nMapSizeY ) );

	return &m_pMap[nY * m_nMapSizeX + nX];
}

// vrátí pozici MapCellu "pMapCell" na mapì v souøadnicích mapy s okraji
inline CPoint CCFPGMap::GetMapCellPosition ( signed char *pMapCell ) 
{
	return CPoint ( ( pMapCell - m_pMap ) % m_nMapSizeX, ( pMapCell - m_pMap ) / 
		m_nMapSizeX );
}

//////////////////////////////////////////////////////////////////////
// Pomocné operace
//////////////////////////////////////////////////////////////////////

// vrátí ID MapCellu s kruhem velikosti "nCircleSize"
inline signed char CCFPGMap::GetMapCellIDFromCircleSize ( int nCircleSize ) 
{
	ASSERT ( ( nCircleSize <= MAX_CIRCLE_SIZE ) && ( nCircleSize >= 1 ) );

	// vrátí ID MapCellu s kruhem velikosti "nCircleSize"
	return 2 * nCircleSize;
}

// vrátí ID MapCellu MapCellu oblasti "cMapCellAreaID"
inline signed char CCFPGMap::GetMapCellIDFromMapCellAreaID ( signed char cMapCellAreaID ) 
{
	ASSERT ( ( cMapCellAreaID >= 0 ) && ( cMapCellAreaID <= ( 2 * MAX_CIRCLE_SIZE + 1 ) ) );

	// vrátí ID MapCellu
	return ( cMapCellAreaID & ~0x01 );
}

// vrátí ID MapCellu oblasti s kruhem velikosti "nCircleSize"
inline signed char CCFPGMap::GetMapCellAreaIDFromCircleSize ( int nCircleSize ) 
{
	ASSERT ( ( nCircleSize <= MAX_CIRCLE_SIZE ) && ( nCircleSize >= 2 ) );

	// vrátí ID MapCellu oblasti s kruhem velikosti "nCircleSize"
	return 2 * nCircleSize + 1;
}

// vrátí ID MapCellu oblasti MapCellu "cMapCellID"
inline signed char CCFPGMap::GetMapCellAreaIDFromMapCellID ( signed char cMapCellID ) 
{
	ASSERT ( ( cMapCellID >= 0 ) && ( cMapCellID <= ( 2 * MAX_CIRCLE_SIZE + 1 ) ) );

	// vrátí ID MapCellu oblasti
	return ( cMapCellID | 0x01 );
}

// zjistí, je-li "cMapCellID" ID MapCellu oblasti
inline BOOL CCFPGMap::IsMapCellAreaID ( signed char cMapCellID ) 
{
	ASSERT ( ( cMapCellID >= 0 ) && ( cMapCellID <= ( 2 * MAX_CIRCLE_SIZE + 1 ) ) );

	// vrátí pøíznak ID MapCellu oblasti
	return ( cMapCellID & 0x01 );
}

// zvìtší tabulku hintù cesty o velikost "dwSize"
inline void CCFPGMap::IncreasePathHintTable ( DWORD dwSize ) 
{
	ASSERT ( ( dwSize > 0 ) && ( m_dwPathHintTableSize + dwSize >= dwSize ) );

	ASSERT ( ( m_pPathHintTable != NULL ) && ( m_dwAllocatedPathHintTableSize > 0 ) && 
		( m_dwPathHintTableSize <= m_dwAllocatedPathHintTableSize ) );

	// zjistí, je-li tabulka hintù cest dostateènì veliká
	if ( m_dwAllocatedPathHintTableSize < m_dwPathHintTableSize + dwSize )
	{	// tabulka hintù cest je malá
		ASSERT ( m_dwAllocatedPathHintTableSize < 2 * m_dwAllocatedPathHintTableSize );
		// alokuje potøebnou velikost tabulky
		if ( ( m_pPathHintTable = (DWORD *)realloc ( m_pPathHintTable, 
			( m_dwAllocatedPathHintTableSize = max ( 2 * m_dwAllocatedPathHintTableSize, 
			m_dwPathHintTableSize + dwSize ) ) * sizeof ( DWORD ) ) ) == NULL )
		{	// nepodaøilo se alokovat požadovanou pamì
			// zneškodní data tabulky hintù cest
			m_dwPathHintTableSize = 0;
			m_dwAllocatedPathHintTableSize = 0;
			// obslouží chybu pamìti
			AfxThrowMemoryException ();
		}
	}
	// tabulka hintù cest je dostateènì velká

	// zvìtší velikost tabulky hintù
	m_dwPathHintTableSize += dwSize;

	ASSERT ( ( m_dwPathHintTableSize > 0 ) && ( m_dwPathHintTableSize <= 
		m_dwAllocatedPathHintTableSize ) );
}

//////////////////////////////////////////////////////////////////////
// Globální funkce
//////////////////////////////////////////////////////////////////////

// vytvoøí grafy pro hledání cesty na mapì "cMapArchive"
enum ECreateFindPathGraphError CreateFindPathGraphs ( CDataArchive cMapArchive ) 
{
	// návratová hodnota
	enum ECreateFindPathGraphError eReturnValue = ECFPGE_OK;
	// mapa vytváøení grafu pro hledání cesty
	class CCFPGMap cMap;

	// vytvoøí adresáø grafù pro hledání cesty
	cMapArchive.MakeDirectory ( FIND_PATH_GRAPHS_DIRECTORY );
	// otevøe archiv grafù pro hledání cesty
	CDataArchive cFindPathGraphsArchive = cMapArchive.CreateArchive ( 
		FIND_PATH_GRAPHS_DIRECTORY );

	// poèet grafù pro hledání cesty
	DWORD dwFindPathGraphCount;

	// zjistí poèet grafù pro hledání cesty
	try
	{
		// otevøe soubor mapy
		CArchiveFile cMapFile = cMapArchive.CreateFile ( MAP_FILE_NAME, CFile::modeRead | 
			CFile::shareDenyWrite );

	// naète hlavièku verze souboru
		{
			SFileVersionHeader sFileVersionHeader;
			CHECK_MAP_FILE ( cMapFile.Read ( &sFileVersionHeader, sizeof ( sFileVersionHeader ) ) == sizeof ( sFileVersionHeader ) );

			// identifikátor souboru mapy
			BYTE aMapFileID[16] = MAP_FILE_IDENTIFIER;
			ASSERT ( sizeof ( aMapFileID ) == sizeof ( sFileVersionHeader.m_aMapFileID ) );

			// zkontroluje identifikátor souboru mapy
			for ( int nIndex = 16; nIndex-- > 0; )
			{
				CHECK_MAP_FILE ( aMapFileID[nIndex] == sFileVersionHeader.m_aMapFileID[nIndex] );
			}

			ASSERT ( CURRENT_MAP_FILE_VERSION >= COMPATIBLE_MAP_FILE_VERSION );
			// zkontroluje verze formátu mapy
			CHECK_MAP_FILE ( sFileVersionHeader.m_dwFormatVersion >= 
				sFileVersionHeader.m_dwCompatibleFormatVersion );

			// zjistí, jedná-li se o správnou verzi formátu mapy
			if ( sFileVersionHeader.m_dwFormatVersion != CURRENT_MAP_FILE_VERSION )
			{	// nejedná se o správnou verzi formátu mapy
				// zjistí, jedná-li se o starou verzi formátu mapy
				if ( sFileVersionHeader.m_dwFormatVersion < CURRENT_MAP_FILE_VERSION )
				{	// jedná se o starou verzi formátu mapy
					// zkontroluje kompatabilitu verze formátu mapy
					CHECK_MAP_FILE ( sFileVersionHeader.m_dwFormatVersion >= 
						COMPATIBLE_MAP_FILE_VERSION );
				}
				else
				{	// jedná se o mladší verzi formátu mapy
					// zkontroluje kompatabilitu verze formátu mapy
					CHECK_MAP_FILE ( sFileVersionHeader.m_dwCompatibleFormatVersion <= 
						CURRENT_MAP_FILE_VERSION );
				}
			}
		}

	// naète hlavièku mapy
		{
			SMapHeader sMapHeader;
			CHECK_MAP_FILE ( cMapFile.Read ( &sMapHeader, sizeof ( sMapHeader ) ) == sizeof ( sMapHeader ) );

			// zjistí poèet grafù pro hledání cesty
			dwFindPathGraphCount = sMapHeader.m_dwFindPathGraphCount;
		}
	}
	catch ( CCFPGMapFileException *pException )
	{
		// znièí výjimku
		pException->Delete ();
		// vrátí pøíznak chybného souboru mapy
		return pException->m_eError;
	}
	catch ( CDataArchiveException *pException )
	{
		// znièí výjimku
		pException->Delete ();
		// vrátí pøíznak chybného souboru mapy
		return ECFPGE_BadMapFile;
	}
	catch ( CMemoryException *pException )
	{
		// znièí výjimku
		pException->Delete ();
		// vrátí pøíznak nedostatku pamìti
		return ECFPGE_NotEnoughMemory;
	}
	catch ( CException *pException )
	{
		// znièí výjimku
		pException->Delete ();
		// vrátí pøíznak neznámé chyby
		return ECFPGE_UnknownError;
	}

	// nechá vytvoøit grafy pro hledání cesty
	for ( DWORD dwFindPathGraphIndex = dwFindPathGraphCount; dwFindPathGraphIndex-- > 0; )
	{
		// vytvoøí mapu pro graf pro hledání cesty "dwFindPathGraphIndex"
		enum ECreateFindPathGraphError eError = cMap.Create ( dwFindPathGraphIndex, 
			cMapArchive, ( dwFindPathGraphIndex == 0 ) );
		// zjistí, podaøilo-li se vytvoøit mapu
		switch ( eError )
		{
		case ECFPGE_IncompleteMapLand :
			ASSERT ( dwFindPathGraphIndex == 0 );
			eReturnValue = eError;
		case ECFPGE_OK :
			break;
		default :
			return eError;
		}

		// vytvoøí graf pro hledání cesty "dwFindPathGraphIndex"
		cMap.CreateFindPathGraph ( cFindPathGraphsArchive );

		// znièí vytvoøenou mapu
		cMap.Delete ();
	}

	ASSERT ( ( eReturnValue == ECFPGE_OK ) || ( eReturnValue == 
		ECFPGE_IncompleteMapLand ) );
	// vrátí pøíznak vytvoøení grafù pro hledání cesty
	return eReturnValue;
}
