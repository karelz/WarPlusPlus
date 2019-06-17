/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: MapEditor
 *   Autor: Karel Zikmund
 * 
 *   Popis: Vytvo�en� graf� pro hled�n� cesty
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

// velikost nejv�t��ho kruhu pro vytv��en� grafu pro hled�n� cesty
#define MAX_CIRCLE_SIZE		25

#define MARK_EXPLORED(cMapCellID)	( (signed char)cMapCellID | (signed char)0x80 )
#define UNMARK_EXPLORED(cMapCellID)	( (signed char)cMapCellID & (signed char)(~(signed char)0x80) )

#ifdef _DEBUG
	BOOL g_bTraceFindPathCreation = TRUE;
	#define TRACE_FIND_PATH_CREATION if ( g_bTraceFindPathCreation ) TRACE_NEXT
#else //!_DEBUG
	#define TRACE_FIND_PATH_CREATION TRACE_NEXT
#endif //!_DEBUG

// zkontroluje podm�nku "condition", neplat�-li, vrac� ECFPGE_BadMapFile
#define CHECK_MAP_FILE(condition) \
	do { if ( !( condition ) ) { throw new CCFPGMapFileException ( ECFPGE_BadMapFile ); } } \
	while ( 0 )

//////////////////////////////////////////////////////////////////////
// Datov� typy
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// T��da v�jimky na��t�n� souboru mapy
class CCFPGMapFileException : public CException 
{
// Metody
public:

	// konstruktor
	CCFPGMapFileException ( enum ECreateFindPathGraphError eError ) 
		{ m_eError = eError; };
	// destruktor
	virtual ~CCFPGMapFileException () {};

	// vrac� informaci o v�jimce
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
	// chybov� k�d
	enum ECreateFindPathGraphError m_eError;
};

//////////////////////////////////////////////////////////////////////
// T��da mapy vytv��en� grafu pro hled�n� cesty
class CCFPGMap 
{
// Datov� typy
protected:
	// typ Mapexu
	struct SMapexType 
	{
		DWORD dwSizeX;
		DWORD dwSizeY;
	};

	// blok p�ebarvov�n� oblasti
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
		// oper�tor p�i�azen�
		const struct SRepaintAreaBlock &operator= ( const struct SRepaintAreaBlock &sBlock ) 
			{ nMinX = sBlock.nMinX; nMaxX = sBlock.nMaxX; nY = sBlock.nY; return *this; };
	};

public:
	// oblast/most
	struct SAreaBridge;
	// br�na mostu
	struct SBridgeGate 
	{
		// pozice br�ny mostu
		CPoint pointPosition;
		// pozice dotyku oblasti/mostu
		CPoint pointTouchPosition;
		// vzd�lenost dotyku oblasti/mostu (10-ti n�sobek vzd�lenosti)
		DWORD dwTouchDistance;

		// ukazatel na oblast br�ny mostu (m��e b�t i ukazatel na most br�ny mostu)
		struct SAreaBridge *pArea;
		// ukazatel na most br�ny mostu
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
	// t��da seznamu bran mostu
	typedef CPooledList<struct SBridgeGate *, 1> CBridgeGateList;

	// oblast/most
	struct SAreaBridge 
	{
		// pozice oblasti/mostu na map�
		CPoint pointPosition;

		// identifik�tor MapCellu oblasti/mostu
		signed char cMapCellID;

		// seznam prim�rn�ch bran mostu/most� oblasti
		CBridgeGateList cPrimaryBridgeGateList;
		// seznam sekund�rn�ch bran mostu/most� oblasti
		CBridgeGateList cSecondaryBridgeGateList;

		// index do tabulky oblast�
		DWORD dwIndex;

		// konstruktor
		SAreaBridge ( CPoint pointInitPosition, signed char cInitMapCellID ) 
			{ pointPosition = pointInitPosition; cMapCellID = cInitMapCellID; };
		// konstruktor
		SAreaBridge () {};
		// oper�tor new
		void *operator new ( size_t nSize, 
			CTypedMemoryPool<struct SAreaBridge> &cAreaBridgePool ) 
			{ return (void *)cAreaBridgePool.Allocate (); };
		// oper�tor delete
		void operator delete ( void *pData, 
			CTypedMemoryPool<struct SAreaBridge> &cAreaBridgePool ) 
			{ cAreaBridgePool.Free ( (struct SAreaBridge *)pData ); };
	};
protected:
	// t��da seznamu oblast�/most�
	typedef CSelfPooledList<struct SAreaBridge *> CAreaBridgeList;

	// ozna�en� oblast
	struct SMarkedArea 
	{
		int nX;
		int nY;

		// konstruktor
		SMarkedArea ( int nInitX, int nInitY ) { nX = nInitX; nY = nInitY; };
		// konstruktor
		SMarkedArea () {};
		// oper�tor p�i�azen�
		const struct SMarkedArea &operator= ( const struct SMarkedArea &sArea ) 
			{ nX = sArea.nX; nY = sArea.nY; return *this; };
	};
	// t��da seznamu ozna�en�ch oblast�
	typedef CSelfPooledStack<struct SMarkedArea> CMarkedAreaStack;

// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CCFPGMap ();
	// destruktor
	~CCFPGMap ();

	// vytvo�� graf pro hled�n� cesty "dwFindPathGraphIndex" na map� "cMapArchive", je-li 
	//		nastaven p��znak "bCheckCompleteLand" kontroluje �plnost povrchu mapy
	enum ECreateFindPathGraphError Create ( DWORD dwFindPathGraphIndex, 
		CDataArchive cMapArchive, BOOL bCheckCompleteLand );
	// zni�� mapu
	void Delete ();

// Operace s mapou

	// vytvo�� graf pro hled�n� cesty v archivu "cFindPathGraphsArchive"
	void CreateFindPathGraph ( CDataArchive cFindPathGraphsArchive );
protected:
	// p�ebarv� oblast "cOldColor" za��naj�c� na sou�adnic�ch "nX" a "nY" barvou 
	//		"cNewColor"
	void RepaintArea ( signed char cOldColor, signed char cNewColor, int nX, int nY );
	// prohled� ��dek "nY" od "nMinX" ("pLine") do "nMaxX" pro p�ekreslov�n� oblasti
	void RepaintAreaSearchLine ( signed char *pLine, int nMinX, int nMaxX, int nY );

	// p�ebarv� oblast "cOldColor" za��naj�c� na sou�adnic�ch "nX" a "nY" barvou "cNewColor"
	// z�rove� hled� reprezentanta ( mapcell, kter� m� hodnotu "cRepresentative" )
	// d�l� to floodfillem
	// vrac� uklazatel na reprezentanta ( nebo NULL )
	signed char * FindAreaRepresentative ( signed char cOldColor, signed char cNewColor, int nX, 
		int nY, signed char cRepresentative );
	// prohled� ��dek "nY" od "nMinX" ("pLine") do "nMaxX" pro p�ekreslov�n� oblasti
	signed char * FindAreaRepresentativeSearchLine ( signed char *pLine, int nMinX, int nMaxX, int nY, signed char cRepresentative );
	
	// najde br�ny mostu "cMapCellID" na "pPosition" a vlo�� je do "cBridgeGateList"
	void GetBridgeGates ( signed char cMapCellID, signed char *pPosition, 
		CBridgeGateList &cBridgeGateList );
	// vr�t� oblast/most na pozici "pPosition"
	struct SAreaBridge *FindAreaBridge ( signed char *pPosition );
	// vypln� vzd�lenosti od br�ny mostu "pBridgeGate" k bran�m most� "cBridgeGateList" 
	//		oblasti "pAreaBridge" do tabulky vzd�lenost� bran most� "pBridgeGateDistances"
	void GetBridgeGateDistances ( struct SAreaBridge *pAreaBridge,
		struct SBridgeGate *pBridgeGate, CBridgeGateList &cBridgeGateList, 
		DWORD *pBridgeGateDistances );
	// najde dotyk br�ny mostu
	void FindBridgeGateTouch ( struct SBridgeGate *pBridgeGate, 
		struct SAreaBridge *pAreaBridge );

// Informace o map�

	// vr�t� ukazatel na MapCell na sou�adnic�ch "nX", "nY" v sou�adnic�ch mapy s okraji
	inline signed char *GetAt ( int nX, int nY );
	// vr�t� ukazatel na MapCell na sou�adnic�ch "pointPosition" v sou�adnic�ch mapy 
	//		s okraji
	signed char *GetAt ( CPoint pointPosition ) 
		{ return GetAt ( pointPosition.x, pointPosition.y ); };
	// vr�t� pozici MapCellu "pMapCell" na map� v sou�adnic�ch mapy s okraji
	inline CPoint GetMapCellPosition ( signed char *pMapCell );

// Pomocn� operace

	// vr�t� ID MapCellu s kruhem velikosti "nCircleSize"
	inline signed char GetMapCellIDFromCircleSize ( int nCircleSize );
	// vr�t� ID MapCellu MapCellu oblasti "cMapCellAreaID"
	inline signed char GetMapCellIDFromMapCellAreaID ( signed char cMapCellAreaID );
	// vr�t� ID MapCellu oblasti s kruhem velikosti "nCircleSize"
	inline signed char GetMapCellAreaIDFromCircleSize ( int nCircleSize );
	// vr�t� ID MapCellu oblasti MapCellu "cMapCellID"
	inline signed char GetMapCellAreaIDFromMapCellID ( signed char cMapCellID );
	// zjist�, je-li "cMapCellID" ID MapCellu oblasti
	inline BOOL IsMapCellAreaID ( signed char cMapCellID );
	// vr�t� vzd�lenost ��sel "nX" a "nY"
	int GetDistance ( int nX, int nY ) { return ( nX < nY ) ? ( nY - nX ) : ( nX - nY ); };
	// zv�t�� tabulku hint� cesty o velikost "dwSize"
	inline void IncreasePathHintTable ( DWORD dwSize );

// Data
private:
	// rozm�ry mapy v MapCellech
	int m_nSizeX;
	int m_nSizeY;

	// nakreslen� mapa
	signed char *m_pMap;
	// rozm�ry nakreslen� mapy v MapCellech
	int m_nMapSizeX;
	int m_nMapSizeY;
	// velikost okraje nakreslen� mapy
	int m_nMapBorder;
	// hranice mapy
	int m_nMapXMin;
	int m_nMapXMax;
	int m_nMapYMin;
	int m_nMapYMax;

	// jm�no grafu pro hled�n� cesty
	CString m_strFindPathGraphName;

	// kruhy pro vytv��en� grafu pro hled�n� cesty
	static signed char *m_aCircles[MAX_CIRCLE_SIZE-1];

	// seznam oblast�/most�
	CAreaBridgeList m_cAreaBridgeList;

// data prohled�v�n� ��dky pro p�ebarvov�n� oblasti

	// barva p�ebarvovan� oblasti
	signed char m_cRepaintAreaSearchLineAreaColor;
	// z�sobn�k blok� pro objevov�n� oblasti
	CSelfPooledStack<struct SRepaintAreaBlock> m_cRepaintAreaBlockStack;

// data hled�n� bran most� a fiktivn�ch oblast�

	// offsety okoln�ch MapCell�
	int m_aSurroundingMapCellOffset[8];
	// offsety vzd�len�ch okoln�ch MapCell�
	int m_aFarSurroundingMapCellOffset[16];
	// offsety MapCell� souvisl�ho okol� MapCellu
	int m_aContinuousMapCellOffset[8][4];
	// offsety vzd�len�ch souvisl�ch MapCell�
	int m_aFarContinuousMapCellOffset[8][5];
	// offsety okoln�ch MapCell� vzd�len�ho MapCellu
	int m_aSurroundingFarMapCellOffset[16][3];
	// offsety okoln�ch pozic
	static CPoint m_aSurroundingPositionOffset[8];
	// z�sobn�k ozna�en�ch oblast�
	CMarkedAreaStack m_cMarkedAreaStack;
	// z�sobn�k ukazatel� na do�asn� ozna�en� MapCelly
	CSelfPooledStack<signed char *> m_cTemporarilyMarkedMapCellStack;
	// fronta MapCell�
	CSelfPooledQueue<signed char *> m_cMapCellQueue;
	// z�sobn�k ukazatel� na do�asn� ozna�en� MapCelly pro hled�n� oblast�/most�
	CSelfPooledStack<signed char *> m_cFindAreaBridgeTemporarilyMarkedMapCellStack;
	// fronta MapCell�
	CSelfPooledQueue<signed char *> m_cFindAreaBridgeMapCellQueue;
	// memory pool bran most�
	CTypedMemoryPool<struct SBridgeGate> m_cBridgeGatePool;
	// memory pool oblast�/most�
	CTypedMemoryPool<struct SAreaBridge> m_cAreaBridgePool;
	// prioritn� fronta MapCell�
	CSelfPooledPriorityQueue<DWORD, signed char *> m_cPriorityMapCellQueue;
	// pole vzd�lenost� MapCell�
	CArray2DOnDemand<DWORD> m_cMapCellDistanceArray;
	// tabulka hint� cest
	DWORD *m_pPathHintTable;
	// velikost tabulky hint� cest (v DWORDech)
	DWORD m_dwPathHintTableSize;
	// alokovan� velikost tabulky hint� cest (v DWORDech)
	DWORD m_dwAllocatedPathHintTableSize;

#ifdef __SHARED_MEMORY__
	// z�mek sd�len� pam�ti
	CMutex m_mutexSharedMemoryLock;

	// mapovan� soubor mapy
	HANDLE m_hMap;
	// mapovan� soubor popisu mapy
	HANDLE m_hMapDescription;

	// popis mapy
	struct SMapDescription *m_pMapDescription;
#endif //__SHARED_MEMORY__
};

//////////////////////////////////////////////////////////////////////
// Statick� data t��dy CCFPGMap
//////////////////////////////////////////////////////////////////////

// memory pool seznamu bran mostu
CTypedMemoryPool<struct CCFPGMap::CBridgeGateList::SListMember> 
	CCFPGMap::CBridgeGateList::m_cPool ( 1000 );

// offsety okoln�ch pozic
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

// kruhy pro vytv��en� grafu pro hled�n� cesty
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

// tabulka kruh� pro vytv��en� grafu pro hled�n� cesty
signed char *CCFPGMap::m_aCircles[MAX_CIRCLE_SIZE-1] = { 
	g_aCircle02, g_aCircle03, g_aCircle04, g_aCircle05, g_aCircle06, 
	g_aCircle07, g_aCircle08, g_aCircle09, g_aCircle10, g_aCircle11, 
	g_aCircle12, g_aCircle13, g_aCircle14, g_aCircle15, g_aCircle16, 
	g_aCircle17, g_aCircle18, g_aCircle19, g_aCircle20, g_aCircle21,
	g_aCircle22, g_aCircle23, g_aCircle24, g_aCircle25
};

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce t��dy CCFPGMap
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

// vytvo�� graf pro hled�n� cesty "dwFindPathGraphIndex" na map� "cMapArchive", je-li 
//		nastaven p��znak "bCheckCompleteLand" kontroluje �plnost povrchu mapy
enum ECreateFindPathGraphError CCFPGMap::Create ( DWORD dwFindPathGraphIndex, 
	CDataArchive cMapArchive, BOOL bCheckCompleteLand ) 
{
	ASSERT ( m_pMap == NULL );

	// n�vratov� hodnota
	enum ECreateFindPathGraphError eReturnValue = ECFPGE_OK;
	// tabulka p�ekladu ID typu Mapexu na ukazatel na n�j
	CMap<DWORD, DWORD, void *, void *> cMapexTypeTable;

	try
	{
	// na�te data mapy ze souboru mapy

		// otev�e soubor mapy
		CArchiveFile cMapFile = cMapArchive.CreateFile ( MAP_FILE_NAME, CFile::modeRead | 
			CFile::shareDenyWrite );

		// po�et knihoven Mapex�
		DWORD dwMapexLibraryCount;
		// po�et civilizac�
		DWORD dwCivilizationCount;
		// po�et knihoven jednotek
		DWORD dwUnitTypeLibraryCount;
		// po�et jednotek na map�
		DWORD dwUnitCount;
		// po�et ScriptSet�
		DWORD dwScriptSetCount;
		// po�et graf� pro hled�n� cesty
		DWORD dwFindPathGraphCount;
		// pou�it� typy povrchu
		C256BitArray aUsedLandTypes;
		// povolen� typy povrchu grafu pro hled�n� cesty
		C256BitArray aAllowedLandTypes;

	// na�te hlavi�ku verze souboru
		{
			SFileVersionHeader sFileVersionHeader;
			CHECK_MAP_FILE ( cMapFile.Read ( &sFileVersionHeader, sizeof ( sFileVersionHeader ) ) == sizeof ( sFileVersionHeader ) );

			// identifik�tor souboru mapy
			BYTE aMapFileID[16] = MAP_FILE_IDENTIFIER;
			ASSERT ( sizeof ( aMapFileID ) == sizeof ( sFileVersionHeader.m_aMapFileID ) );

			// zkontroluje identifik�tor souboru mapy
			for ( int nIndex = 16; nIndex-- > 0; )
			{
				CHECK_MAP_FILE ( aMapFileID[nIndex] == sFileVersionHeader.m_aMapFileID[nIndex] );
			}

			ASSERT ( CURRENT_MAP_FILE_VERSION >= COMPATIBLE_MAP_FILE_VERSION );
			// zkontroluje verze form�tu mapy
			CHECK_MAP_FILE ( sFileVersionHeader.m_dwFormatVersion >= 
				sFileVersionHeader.m_dwCompatibleFormatVersion );

			// zjist�, jedn�-li se o spr�vnou verzi form�tu mapy
			if ( sFileVersionHeader.m_dwFormatVersion != CURRENT_MAP_FILE_VERSION )
			{	// nejedn� se o spr�vnou verzi form�tu mapy
				// zjist�, jedn�-li se o starou verzi form�tu mapy
				if ( sFileVersionHeader.m_dwFormatVersion < CURRENT_MAP_FILE_VERSION )
				{	// jedn� se o starou verzi form�tu mapy
					// zkontroluje kompatabilitu verze form�tu mapy
					CHECK_MAP_FILE ( sFileVersionHeader.m_dwFormatVersion >= 
						COMPATIBLE_MAP_FILE_VERSION );
				}
				else
				{	// jedn� se o mlad�� verzi form�tu mapy
					// zkontroluje kompatabilitu verze form�tu mapy
					CHECK_MAP_FILE ( sFileVersionHeader.m_dwCompatibleFormatVersion <= 
						CURRENT_MAP_FILE_VERSION );
				}
			}
		}

	// na�te hlavi�ku mapy
		{
			SMapHeader sMapHeader;
			CHECK_MAP_FILE ( cMapFile.Read ( &sMapHeader, sizeof ( sMapHeader ) ) == sizeof ( sMapHeader ) );

		// zpracuje hlavi�ku mapy

			// na�te velikost mapy
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
		// sd�len� pam�ti

			ASSERT ( m_nMapSizeX * m_nMapSizeY <= SHARED_MEMORY_MAP_SIZE );

			// zamkne z�mek sd�len�ch pam�t�
			VERIFY ( m_mutexSharedMemoryLock.Lock () );

			// vytvo�� mapov�n� mapy
			m_hMap = CreateFileMapping ( (HANDLE)0xffffffff, NULL, PAGE_READWRITE, 0, 
				SHARED_MEMORY_MAP_SIZE, SHARED_MEMORY_MAP_NAME );
			ASSERT ( m_hMap != NULL );

			// namapuje pam� mapy
			m_pMap = (signed char *)MapViewOfFile ( m_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 
				SHARED_MEMORY_MAP_SIZE );
			ASSERT ( m_pMap != NULL );

			// vytvo�� mapov�n� popisu mapy
			m_hMapDescription = CreateFileMapping ( (HANDLE)0xffffffff, NULL, PAGE_READWRITE, 0, 
				SHARED_MEMORY_MAPDESCRIPTION_SIZE, SHARED_MEMORY_MAPDESCRIPTION_NAME );
			ASSERT ( m_hMapDescription != NULL );

			// namapuje pam� popisu mapy
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

			// odemkne z�mek sd�len�ch pam�t�
			VERIFY ( m_mutexSharedMemoryLock.Unlock () );

			// ukazatel na ��dek mapy
			signed char *pLine = m_pMap;
			// inicilizuje horn� okraj mapy
			memset ( pLine, MAPCELL_DISALLOWED, m_nMapSizeX * MAP_BORDER + MAP_BORDER );
			// inicializuje st�ed mapy
			pLine = GetAt ( MAP_BORDER, MAP_BORDER );
			memset ( pLine, MAPCELL_UNINITIALIZED, m_nMapSizeX * m_nSizeY - 2 * MAP_BORDER );
			// inicilizuje doln� okraj mapy
			pLine = GetAt ( 0, m_nSizeY + MAP_BORDER );
			memset ( pLine, MAPCELL_DISALLOWED, m_nMapSizeX * MAP_BORDER );
			ASSERT ( ( pLine + m_nMapSizeX * MAP_BORDER ) == ( m_pMap + m_nMapSizeX * 
				m_nMapSizeY ) );

			// inicializuje lev� a prav� okraj mapy
			pLine = GetAt ( m_nSizeX + MAP_BORDER, MAP_BORDER );
			for ( DWORD dwIndexY = m_nSizeY; dwIndexY-- > 0; pLine += m_nMapSizeX )
			{
				// inicializuje lev� okraj ��dky "pLine" mapy
				memset ( pLine, MAPCELL_DISALLOWED, 2 * MAP_BORDER );
			}
#else //!__SHARED_MEMORY__
			// alokuje mapu
			m_pMap = new signed char[m_nMapSizeX * m_nMapSizeY];

		// inicializuje mapu
			{
				// zjist�, m�-li se kontrolovat povrch mapy
				if ( bCheckCompleteLand )
				{	// m� se kontrolovat povrch mapy
					// ukazatel na ��dek mapy
					signed char *pLine = m_pMap;
					// inicilizuje horn� okraj mapy
					memset ( pLine, MAPCELL_DISALLOWED, m_nMapSizeX * MAP_BORDER + MAP_BORDER );
					// inicializuje st�ed mapy
					pLine = GetAt ( MAP_BORDER, MAP_BORDER );
					memset ( pLine, MAPCELL_UNINITIALIZED, m_nMapSizeX * m_nSizeY - 2 * MAP_BORDER );
					// inicilizuje doln� okraj mapy
					pLine = GetAt ( 0, m_nSizeY + MAP_BORDER );
					memset ( pLine, MAPCELL_DISALLOWED, m_nMapSizeX * MAP_BORDER );
					ASSERT ( ( pLine + m_nMapSizeX * MAP_BORDER ) == ( m_pMap + m_nMapSizeX * 
						m_nMapSizeY ) );

					// inicializuje lev� a prav� okraj mapy
					pLine = GetAt ( m_nSizeX + MAP_BORDER, MAP_BORDER );
					for ( DWORD dwIndexY = m_nSizeY; dwIndexY-- > 0; pLine += m_nMapSizeX )
					{
						// inicializuje lev� okraj ��dky "pLine" mapy
						memset ( pLine, MAPCELL_DISALLOWED, 2 * MAP_BORDER );
					}
				}
				else
				{	// nem� se kontrolovat povrch mapy
					// inicializuje mapu
					memset ( m_pMap, MAPCELL_UNINITIALIZED, m_nMapSizeX * m_nMapSizeY );
#ifdef _DEBUG
					// ukazatel na ��dek mapy
					signed char *pLine = m_pMap;
					// inicilizuje horn� okraj mapy
					memset ( pLine, MAPCELL_DISALLOWED, m_nMapSizeX * MAP_BORDER + MAP_BORDER );
					// inicilizuje doln� okraj mapy
					pLine = GetAt ( 0, m_nSizeY + MAP_BORDER );
					memset ( pLine, MAPCELL_DISALLOWED, m_nMapSizeX * MAP_BORDER );
					ASSERT ( ( pLine + m_nMapSizeX * MAP_BORDER ) == ( m_pMap + m_nMapSizeX * 
						m_nMapSizeY ) );

					// inicializuje lev� a prav� okraj mapy
					pLine = GetAt ( m_nSizeX + MAP_BORDER, MAP_BORDER );
					for ( DWORD dwIndexY = m_nSizeY; dwIndexY-- > 0; pLine += m_nMapSizeX )
					{
						// inicializuje lev� okraj ��dky "pLine" mapy
						memset ( pLine, MAPCELL_DISALLOWED, 2 * MAP_BORDER );
					}
#endif //_DEBUG
				}
			}
#endif //!__SHARED_MEMORY__

			// zkontroluje po�et knihoven Mapex�
			dwMapexLibraryCount = sMapHeader.m_dwMapexLibraryCount;
			CHECK_MAP_FILE ( dwMapexLibraryCount > 0 );

			// zkontroluje po�et civilizac�
			dwCivilizationCount = sMapHeader.m_dwCivilizationsCount;
			CHECK_MAP_FILE ( dwCivilizationCount > 0 );

			// zkontroluje po�et knihoven typ� jednotek
			dwUnitTypeLibraryCount = sMapHeader.m_dwUnitTypeLibraryCount;
			CHECK_MAP_FILE ( dwUnitTypeLibraryCount > 0 );

			// zjist� po�et jednotek na map�
			dwUnitCount = sMapHeader.m_dwUnitCount;

			// zjist� pou�it� typy povrchu
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

			// zjist� po�et ScriptSet�
			dwScriptSetCount = sMapHeader.m_dwScriptSetCount;
			CHECK_MAP_FILE ( dwScriptSetCount > 0 );

			// zjist� po�et graf� pro hled�n� cesty
			dwFindPathGraphCount = sMapHeader.m_dwFindPathGraphCount;
			ASSERT ( dwFindPathGraphIndex < dwFindPathGraphCount );
		}

	// p�esko�� hlavi�ky ScriptSet�
		{
			// zjist� velikost hlavi�ek ScriptSet�
			DWORD dwScriptSetSize = dwScriptSetCount * sizeof ( SScriptSetHeader );

			// p�esko�� hlavi�ky ScriptSet�
			CHECK_MAP_FILE ( cMapFile.GetPosition () + dwScriptSetSize <= 
				cMapFile.GetLength () );
			(void)cMapFile.Seek ( dwScriptSetSize, CFile::current );
		}

	// zjist� povolen� typy povrchu grafu pro hled�n� cesty "dwFindPathGraphIndex"
		{
			// pozice odkaz� na knihovny Mapex�
			DWORD dwMapexLibraryNodesPosition = cMapFile.GetPosition ();

		// p�esko�� odkazy na knihovny Mapex�, hlavi�ky civilizac�, odkazy na knihovny typ� 
		//		jednotek a pole offset� MapSquar�
			{
				// zjist� velikost p�eskakovan�ch dat
				DWORD dwSkippedDataSize = dwMapexLibraryCount * sizeof ( SMapexLibraryNode ) + 
					dwCivilizationCount * sizeof ( SCivilizationHeader ) + 
					dwUnitTypeLibraryCount * sizeof ( SUnitTypeLibraryNode ) + ( m_nSizeX / 
					MAPSQUARE_WIDTH ) * ( m_nSizeY / MAPSQUARE_HEIGHT ) * sizeof ( DWORD );

				// p�esko�� data
				CHECK_MAP_FILE ( cMapFile.GetPosition () + dwSkippedDataSize <= 
					cMapFile.GetLength () );
				(void)cMapFile.Seek ( dwSkippedDataSize, CFile::current );
			}

		// p�esko�� informace o MapSquarech
			{
				// rozm�ry mapy v MapSquarech
				DWORD dwSizeX = (DWORD)( m_nSizeX / MAPSQUARE_WIDTH );
				DWORD dwSizeY = (DWORD)( m_nSizeY / MAPSQUARE_HEIGHT );

				// hlavi�ka MapSquaru
				SMapSquareHeader sMapSquareHeader;

				// vykresl� v�echny MapSquary
				for ( DWORD dwMapSquareIndexY = 0; dwMapSquareIndexY < dwSizeY; 
					dwMapSquareIndexY++ )
				{
					for ( DWORD dwMapSquareIndexX = 0; dwMapSquareIndexX < dwSizeX; 
						dwMapSquareIndexX++ )
					{
						// na�te hlavi�ku MapSquaru
						CHECK_MAP_FILE ( cMapFile.Read ( &sMapSquareHeader, sizeof ( sMapSquareHeader ) ) == sizeof ( sMapSquareHeader ) );

					// p�esko�� Background Level Mapexy a Unit Level Mapexy

						// zjist� velikost Unit Level Mapex�
						DWORD dwMapexesSize = ( sMapSquareHeader.m_dwBL1MapexesNum + 
							sMapSquareHeader.m_dwBL2MapexesNum + 
							sMapSquareHeader.m_dwBL3MapexesNum ) * 
							sizeof ( SMapexInstanceHeader ) + 
							sMapSquareHeader.m_dwULMapexesNum * 
							sizeof ( SULMapexInstanceHeader );

						// p�esko�� Background Level Mapexy a Unit Level Mapexy
						CHECK_MAP_FILE ( cMapFile.GetPosition () + dwMapexesSize <= 
							cMapFile.GetLength () );
						(void)cMapFile.Seek ( dwMapexesSize, CFile::current );
					}
				}
				// v�echny MapSquary jsou p�esko�eny
			}

		// p�esko�� jednotky, suroviny, neviditelnosti a hlavi�ky p�edchoz�ch graf� 
		//		pro hled�n� cesty
			{
				// zjist� velikost p�eskakovan�ch dat
				DWORD dwSkippedDataSize = dwUnitCount * sizeof ( SUnitHeader ) + 
					RESOURCE_COUNT * ( sizeof ( SResource ) + sizeof ( DWORD ) * 
					RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT ) + INVISIBILITY_COUNT * 
					sizeof ( SInvisibility ) + dwFindPathGraphIndex * 
					sizeof ( SFindPathGraphHeader );

				// p�esko�� data
				CHECK_MAP_FILE ( cMapFile.GetPosition () + dwSkippedDataSize <= 
					cMapFile.GetLength () );
				(void)cMapFile.Seek ( dwSkippedDataSize, CFile::current );
			}

		// na�te hlavi�ku vytv��en�ho grafu pro hled�n� cesty

			SFindPathGraphHeader sFindPathGraphHeader;

			// na�te graf pro hled�n� cesty
			CHECK_MAP_FILE ( cMapFile.Read ( &sFindPathGraphHeader, sizeof ( sFindPathGraphHeader ) ) == sizeof ( sFindPathGraphHeader ) );

			// zjist� jm�no grafu pro hled�n� cesty
			m_strFindPathGraphName = sFindPathGraphHeader.m_pName;

			// zjist� typy povrchu grafu pro hled�n� cesty
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

			// zkontroluje povolen� typy povrchu
			for ( int nLandTypeIndex = 256; nLandTypeIndex-- > 0; )
			{
				// zjist�, je-li typ povrchu "nLandTypeIndex" v po��dku
				if ( !aUsedLandTypes.GetAt ( nLandTypeIndex ) && 
					aAllowedLandTypes.GetAt ( nLandTypeIndex ) )
				{	// typ povrchu "nLandTypeIndex" nen� v po��dku
					aAllowedLandTypes.ClearAt ( nLandTypeIndex );
				}
			}

		// nastav� p�vodn� pozici v souboru

			// p�esune se na za��tek odkaz� na knihovny Mapex�
			CHECK_MAP_FILE ( (DWORD)cMapFile.Seek ( dwMapexLibraryNodesPosition, 
				CFile::begin ) == dwMapexLibraryNodesPosition );
		}

	// na�te odkazy na knihovny Mapex�
		{
			SMapexLibraryNode sMapexLibraryNode;

			// nech� na��st v�echny knihovny Mapex�
			for ( DWORD dwMapexLibraryIndex = dwMapexLibraryCount; dwMapexLibraryIndex-- > 0; )
			{
				// na�te odkaz na knihovnu Mapex�
				CHECK_MAP_FILE ( cMapFile.Read ( &sMapexLibraryNode, sizeof ( sMapexLibraryNode ) ) == sizeof ( sMapexLibraryNode ) );

				// zkontroluje d�lku jm�na knihovny Mapex�
				int nMapexLibraryFileNameLength = strlen ( sMapexLibraryNode.m_pFileName );
				CHECK_MAP_FILE ( ( nMapexLibraryFileNameLength > 0 ) && 
					( nMapexLibraryFileNameLength < sizeof ( sMapexLibraryNode.m_pFileName ) ) );

				// otev�e archiv knihovny Mapex�
				CDataArchive cMapexLibraryArchive = cMapArchive.CreateArchive ( 
					sMapexLibraryNode.m_pFileName );

				// na�te knihovnu Mapex�
				{
					// otev�e hlavi�kov� soubor knihovny
					CArchiveFile cMapexLibraryHeaderFile = cMapexLibraryArchive.CreateFile ( 
						_T("Library.Header"), CFile::modeRead | CFile::shareDenyWrite );

					// p�e�te hlavi�ku verze knihovny Mapex�
					{
						SMapexFileVersionHeader sMapexLibraryVersionHeader;
						CHECK_MAP_FILE ( cMapexLibraryHeaderFile.Read ( &sMapexLibraryVersionHeader, sizeof ( sMapexLibraryVersionHeader ) ) == sizeof ( sMapexLibraryVersionHeader ) );

						// identifik�tor souboru knihovny Mapex�
						BYTE aMapexLibraryFileID[16] = MAPEX_FILE_IDENTIFIER;
						ASSERT ( sizeof ( aMapexLibraryFileID ) == 
							sizeof ( sMapexLibraryVersionHeader.m_aFileID ) );

						// zkontroluje identifik�tor souboru knihovny Mapex�
						for ( int nIndex = 16; nIndex-- > 0; )
						{
							CHECK_MAP_FILE ( aMapexLibraryFileID[nIndex] == 
								sMapexLibraryVersionHeader.m_aFileID[nIndex] );
						}

						ASSERT ( CURRENT_MAP_FILE_VERSION >= COMPATIBLE_MAP_FILE_VERSION );
						// zkontroluje ��sla verz� form�tu knihovny Mapex�
						CHECK_MAP_FILE ( sMapexLibraryVersionHeader.m_dwFormatVersion >= 
							sMapexLibraryVersionHeader.m_dwCompatibleFormatVersion );

						// zjist�, jedn�-li se o spr�vnou verzi form�tu knihovny Mapex�
						if ( sMapexLibraryVersionHeader.m_dwFormatVersion != CURRENT_MAP_FILE_VERSION )
						{	// nejedn� se o spr�vnou verzi form�tu knihovny Mapex�
							// zjist�, jedn�-li se o starou verzi form�tu knihovny Mapex�
							if ( sMapexLibraryVersionHeader.m_dwFormatVersion < CURRENT_MAP_FILE_VERSION )
							{	// jedn� se o starou verzi form�tu knihovny Mapex�
								// zkontroluje kompatabilitu verze form�tu knihovny Mapex�
								CHECK_MAP_FILE ( sMapexLibraryVersionHeader.m_dwFormatVersion >= 
									COMPATIBLE_MAP_FILE_VERSION );
							}
							else
							{	// jedn� se o mlad�� verzi form�tu knihovny Mapex�
								// zkontroluje kompatabilitu verze form�tu knihovny Mapex�
								CHECK_MAP_FILE ( sMapexLibraryVersionHeader.m_dwCompatibleFormatVersion <= 
									CURRENT_MAP_FILE_VERSION );
							}
						}
					}

					// p�e�te hlavi�ku knihovny Mapex�
					SMapexLibraryHeader sMapexLibraryHeader;
					CHECK_MAP_FILE ( cMapexLibraryHeaderFile.Read ( &sMapexLibraryHeader, sizeof ( sMapexLibraryHeader ) ) == sizeof ( sMapexLibraryHeader ) );

					// na�te jm�na Mapexov�ch soubor�
					for ( DWORD dwMapexLibraryIndex = sMapexLibraryHeader.m_dwMapexCount; 
						dwMapexLibraryIndex-- > 0; )
					{
						// p�e�te jm�no dal��ho mapexov�ho souboru
						char szMapexFileName[30];
						CHECK_MAP_FILE ( cMapexLibraryHeaderFile.Read ( &szMapexFileName, sizeof ( szMapexFileName ) ) == sizeof ( szMapexFileName ) );

						// zkontroluje d�lku jm�na Mapexov�ho souboru
						int nMapexFileNameLength = strlen ( szMapexFileName );
						CHECK_MAP_FILE ( ( nMapexFileNameLength > 0 ) && 
							( nMapexFileNameLength < sizeof ( szMapexFileName ) ) );

						// otev�e Mapexov� soubor
						CArchiveFile cMapexFile = cMapexLibraryArchive.CreateFile ( 
							szMapexFileName, CFile::modeRead | CFile::shareDenyWrite );

						// p�e�te hlavi�ku Mapexov�ho souboru
						SMapexHeader sMapexHeader;
						CHECK_MAP_FILE ( cMapexFile.Read ( &sMapexHeader, sizeof ( sMapexHeader ) ) == sizeof ( sMapexHeader ) );

						// ukazatel na typ Mapexu
						struct SMapexType *pMapexType;

						// zkontroluje unik�tnost ID typu Mapexu
						CHECK_MAP_FILE ( !cMapexTypeTable.Lookup ( sMapexHeader.m_dwID, (void *&)pMapexType ) );
						// zkontroluje rozm�ry typu Mapexu
						CHECK_MAP_FILE ( sMapexHeader.m_dwXSize <= MAPSQUARE_WIDTH );
						CHECK_MAP_FILE ( sMapexHeader.m_dwYSize <= MAPSQUARE_HEIGHT );

						// zjist� velikost typ� povrchu typu Mapexu
						DWORD dwMapexTypeLandTypesSize = sMapexHeader.m_dwXSize * 
							sMapexHeader.m_dwYSize;

						// alokuje pam� pro dal�� typ Mapexu
						pMapexType = (struct SMapexType *)new char[sizeof ( SMapexType ) + 
							dwMapexTypeLandTypesSize];

						// p�id� typ Mapexu do tabulky typ� Mapex�
						cMapexTypeTable.SetAt ( sMapexHeader.m_dwID, pMapexType );

						// vypln� typ Mapexu
						pMapexType->dwSizeX = sMapexHeader.m_dwXSize;
						pMapexType->dwSizeY = sMapexHeader.m_dwYSize;

						// na�te typy povrchu typu Mapexu
						CHECK_MAP_FILE ( cMapexFile.Read ( pMapexType + 1, dwMapexTypeLandTypesSize ) == dwMapexTypeLandTypesSize );

						// ukazatel na typy povrchu typu Mapexu
						signed char *aMapexTypeLandTypes = (signed char *)(pMapexType + 1);

						// p�ep�e typy povrchu typu Mapexu podle povolen�ch typ� povrchu
						for ( DWORD dwIndex = dwMapexTypeLandTypesSize; dwIndex-- > 0; )
						{
							// zjist�, jedn�-li se o pr�hledn� typ povrchu
							if ( aMapexTypeLandTypes[dwIndex] == TRANSPARENT_LAND_TYPE_ID )
							{	// jedn� se o pr�hledn� typ povrchu
								aMapexTypeLandTypes[dwIndex] = MAPCELL_UNINITIALIZED;
							}
							else
							{	// nejedn� se o pr�hledn� typ povrchu
								// p�ep�e typ povrchu typu Mapexu podle povolen�ch typ� povrchu
								aMapexTypeLandTypes[dwIndex] = aAllowedLandTypes.GetAt ( 
									aMapexTypeLandTypes[dwIndex] ) ? MAPCELL_ALLOWED : 
									MAPCELL_DISALLOWED;
							}
						}

						// zkontroluje konec Mapexov�ho souboru
						CHECK_MAP_FILE ( cMapexFile.GetPosition () == cMapexFile.GetLength () );

						// zav�e Mapexov� soubor
						cMapexFile.Close ();
					}

					// zkontroluje konec hlavi�kov�ho souboru knihovny
					CHECK_MAP_FILE ( cMapexLibraryHeaderFile.GetPosition () == 
						cMapexLibraryHeaderFile.GetLength () );

					// zav�e hlavi�kov� soubor knihovny
					cMapexLibraryHeaderFile.Close ();
				}
				// knihovna mapex� je na�tena

				// zav�e arhiv knihovny Mapex�
				cMapexLibraryArchive.Close ();
			}
			// v�echny knihovny Mapex� jsou na�teny
		}

	// p�esko�� hlavi�ky civilizac�, odkazy na knihovny typ� jednotek a pole offset� 
	//		MapSquar�
		{
			// zjist� velikost p�eskakovan�ch dat
			DWORD dwSkippedDataSize = dwCivilizationCount * sizeof ( SCivilizationHeader ) + 
				dwUnitTypeLibraryCount * sizeof ( SUnitTypeLibraryNode ) + ( m_nSizeX / 
				MAPSQUARE_WIDTH ) * ( m_nSizeY / MAPSQUARE_HEIGHT ) * sizeof ( DWORD );

			// p�esko�� data
			CHECK_MAP_FILE ( cMapFile.GetPosition () + dwSkippedDataSize <= 
				cMapFile.GetLength () );
			(void)cMapFile.Seek ( dwSkippedDataSize, CFile::current );
		}

	// na�te informace o MapSquarech
		{
			// rozm�ry mapy v MapSquarech
			DWORD dwSizeX = (DWORD)( m_nSizeX / MAPSQUARE_WIDTH );
			DWORD dwSizeY = (DWORD)( m_nSizeY / MAPSQUARE_HEIGHT );

			// hlavi�ka MapSquaru
			SMapSquareHeader sMapSquareHeader;
			// hlavi�ka instance Mapexu Background Levelu
			SMapexInstanceHeader sBLMapexInstanceHeader;

			// ukazatel na typ Mapexu
			struct SMapexType *pMapexType;

			// pozice MapSquar� v souboru mapy
			DWORD dwMapSquaresPosition = cMapFile.GetPosition ();

			// vykresl� v�echny Background Levely
			for ( int nBLIndex = 0; nBLIndex < 3; nBLIndex++ )
			{
				// p�esune se na za��tek MapSquar� v souboru mapy
				CHECK_MAP_FILE ( (DWORD)cMapFile.Seek ( dwMapSquaresPosition, 
					CFile::begin ) == dwMapSquaresPosition );

				// vykresl� v�echny MapSquary
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

						// na�te hlavi�ku MapSquaru
						CHECK_MAP_FILE ( cMapFile.Read ( &sMapSquareHeader, sizeof ( sMapSquareHeader ) ) == sizeof ( sMapSquareHeader ) );

						// pole po�t� Mapex� Background Level� MapSquaru
						DWORD aBLMapexCount[3] = { sMapSquareHeader.m_dwBL1MapexesNum, 
							sMapSquareHeader.m_dwBL2MapexesNum, 
							sMapSquareHeader.m_dwBL3MapexesNum };

						// p�e�te Background Level Mapexy
						for ( int nDrawBLIndex = 0; nDrawBLIndex < 3; nDrawBLIndex++ )
						{
							// zjist�, jedn�-li se o vykreslovan� Background Level Mapexy
							if ( nDrawBLIndex == nBLIndex )
							{	// jedn� se o vykreslovan� Background Level Mapexy
								// na�te Background Level Mapexy
								for ( int nBLMapexIndex = aBLMapexCount[nBLIndex]; 
									nBLMapexIndex-- > 0; )
								{
									// na�te hlavi�ku instance Background Level Mapexu
									CHECK_MAP_FILE ( cMapFile.Read ( &sBLMapexInstanceHeader, sizeof ( sBLMapexInstanceHeader ) ) == sizeof ( sBLMapexInstanceHeader ) );

									// zkontroluje um�st�n� Mapexu v MapSquaru
									CHECK_MAP_FILE ( dwPositionX <= sBLMapexInstanceHeader.m_dwX );
									CHECK_MAP_FILE ( dwPositionX + MAPSQUARE_WIDTH > sBLMapexInstanceHeader.m_dwX );
									CHECK_MAP_FILE ( dwPositionY <= sBLMapexInstanceHeader.m_dwY );
									CHECK_MAP_FILE ( dwPositionY + MAPSQUARE_HEIGHT > sBLMapexInstanceHeader.m_dwY );
									// z�sk� ukazatel na typ Mapexu podle ID Mapexu
									CHECK_MAP_FILE ( cMapexTypeTable.Lookup ( 
										sBLMapexInstanceHeader.m_dwMapexID, (void *&)pMapexType ) );

									// ukazatel na typy povrchu Mapexu
									signed char *pMapexTypeLandTypes = (signed char *)( pMapexType + 1 );

									// zjist�, zasahuje-li Mapex mimo mapu
									if ( ( sBLMapexInstanceHeader.m_dwX + pMapexType->dwSizeX <= 
										(DWORD)m_nSizeX ) && ( sBLMapexInstanceHeader.m_dwY + 
										pMapexType->dwSizeY <= (DWORD)m_nSizeY ) )
									{	// Mapex nezasahuje mimo mapu
										// ukazatel na roh um�st�n�ho Mapexu
										signed char *pCorner = GetAt ( 
											MAP_BORDER + (int)sBLMapexInstanceHeader.m_dwX, 
											MAP_BORDER + (int)sBLMapexInstanceHeader.m_dwY );

										// nakresl� Mapex do mapy
										for ( DWORD dwIndexY = pMapexType->dwSizeY; dwIndexY-- > 0; )
										{
											for ( DWORD dwIndexX = pMapexType->dwSizeX; dwIndexX-- > 0; )
											{
												// zjist�, jedn�-li se o pr�hledn� typ povrchu
												if ( pMapexTypeLandTypes[dwIndexX] != 
													MAPCELL_UNINITIALIZED )
												{	// nejedn� se o pr�hledn� typ povrchu
													pCorner[dwIndexX] = pMapexTypeLandTypes[dwIndexX];
												}
											}
											// nech� vykreslit dal�� ��dek Mapexu
											pCorner += m_nMapSizeX;
											pMapexTypeLandTypes += pMapexType->dwSizeX;
										}
									}
									else
									{	// Mapex zasahuje mimo mapu
										// ukazatel na za��tek prvn�ho ��dku s vykreslovan�m Mapexem
										signed char *pCorner = GetAt ( MAP_BORDER, 
											MAP_BORDER + (int)sBLMapexInstanceHeader.m_dwY );

										// nakresl� Mapex do mapy
										for ( DWORD dwIndexY = sBLMapexInstanceHeader.m_dwY; 
											dwIndexY < pMapexType->dwSizeY; dwIndexY++ )
										{
											// index v Mapexu
											DWORD dwMapexIndexX = 0;
											for ( DWORD dwIndexX = sBLMapexInstanceHeader.m_dwX; 
												dwIndexX < pMapexType->dwSizeX; dwIndexX++, dwMapexIndexX++ )
											{
												// zjist�, jedn�-li se o pr�hledn� typ povrchu
												if ( pMapexTypeLandTypes[dwMapexIndexX] != 
													MAPCELL_UNINITIALIZED )
												{	// nejedn� se o pr�hledn� typ povrchu
													pCorner[dwIndexX] = pMapexTypeLandTypes[dwMapexIndexX];
												}
											}
											// nech� vykreslit dal�� ��dek Mapexu
											pCorner += m_nMapSizeX;
											pMapexTypeLandTypes += pMapexType->dwSizeX;
										}
									}
								}
								// na�etl Background Level Mapexy
							}
							else
							{	// nejedn� se o vykreslovan� Background Level Mapexy
								// zjist� velikost Mapex� Background Levelu "nDrawBLIndex"
								DWORD dwBLMapexesSize = aBLMapexCount[nDrawBLIndex] * 
									sizeof ( SMapexInstanceHeader );

								// p�esko�� Mapexy Background Levelu "nDrawBLIndex"
								CHECK_MAP_FILE ( cMapFile.GetPosition () + dwBLMapexesSize <= 
									cMapFile.GetLength () );
								(void)cMapFile.Seek ( dwBLMapexesSize, CFile::current );
							}
						}
						// Background Level Mapexy jsou p�e�teny

					// p�esko�� Unit Level Mapexy

						// zjist� velikost Unit Level Mapex�
						DWORD dwULMapexesSize = sMapSquareHeader.m_dwULMapexesNum * 
							sizeof ( SULMapexInstanceHeader );

						// p�esko�� Unit Level Mapexy
						CHECK_MAP_FILE ( cMapFile.GetPosition () + dwULMapexesSize <= 
							cMapFile.GetLength () );
						(void)cMapFile.Seek ( dwULMapexesSize, CFile::current );
					}
				}
				// v�echny MapSquary jsou nakresleny
			}
			// v�echny Background Levely jsou vykresleny
		}

	// p�esko�� jednotky, suroviny, neviditelnosti a grafy pro hled�n� cesty
		{
			// zjist� velikost p�eskakovan�ch dat
			DWORD dwSkippedDataSize = dwUnitCount * sizeof ( SUnitHeader ) + 
				RESOURCE_COUNT * ( sizeof ( SResource ) + sizeof ( DWORD ) * 
				RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT ) + INVISIBILITY_COUNT * 
				sizeof ( SInvisibility ) + dwFindPathGraphCount * 
				sizeof ( SFindPathGraphHeader );

			// p�esko�� data
			CHECK_MAP_FILE ( cMapFile.GetPosition () + dwSkippedDataSize <= 
				cMapFile.GetLength () );
			(void)cMapFile.Seek ( dwSkippedDataSize, CFile::current );
		}

	// ukon�� �ten� souboru mapy

		// zkontroluje d�lku souboru mapy
		CHECK_MAP_FILE ( cMapFile.GetPosition () == cMapFile.GetLength () );
		// zav�e soubor mapy
		cMapFile.Close ();
	}
	catch ( CCFPGMapFileException *pException )
	{
		// zni�� v�jimku
		pException->Delete ();
		// vr�t� p��znak chybn�ho souboru mapy
		eReturnValue = pException->m_eError;
	}
	catch ( CDataArchiveException *pException )
	{
		// zni�� v�jimku
		pException->Delete ();
		// vr�t� p��znak chybn�ho souboru mapy
		eReturnValue = ECFPGE_BadMapFile;
	}
	catch ( CMemoryException *pException )
	{
		// zni�� v�jimku
		pException->Delete ();
		// vr�t� p��znak nedostatku pam�ti
		eReturnValue = ECFPGE_NotEnoughMemory;
	}
	catch ( CException *pException )
	{
		// zni�� v�jimku
		pException->Delete ();
		// vr�t� p��znak nezn�m� chyby
		eReturnValue = ECFPGE_UnknownError;
	}

	// zni�� typy Mapex�
	{
		// pozice prvn�ho typu Mapexu
		POSITION posMapexType = cMapexTypeTable.GetStartPosition ();
		// ID typu Mapexu
		DWORD dwMapexTypeID;
		// ukazatel na typ Mapexu
		struct SMapexType *pMapexType;

		// projede v�echny typy Mapexu
		while ( posMapexType != NULL )
		{
			// z�sk� dal�� typ Mapexu
			cMapexTypeTable.GetNextAssoc ( posMapexType, dwMapexTypeID, (void *&)pMapexType );

			// zni�� typ Mapexu
			delete [] (char *)pMapexType;
		}
	
		// zni�� tabulku typ� Mapex�
		cMapexTypeTable.RemoveAll ();
	}

	// zjist�, do�lo-li k chyb�
	if ( eReturnValue != ECFPGE_OK )
	{	// do�lo k chyb�
		// zjist�, poda�ilo-li se vytvo�it nakreslenou mapu
		if ( m_pMap != NULL )
		{	// poda�ilo se vytvo�it nakreslenou mapu
			// zni�� nakreslenou mapu
			delete [] m_pMap;
			// zne�kodn� nakreslenou mapu
			m_pMap = NULL;
		}
	}
	else
	{	// nedo�lo k chyb�

#ifdef _DEBUG
		// zkontroluje horn� okraj mapy
		signed char *pLine = m_pMap;
		for ( DWORD dwIndex = m_nMapSizeX * MAP_BORDER + MAP_BORDER; dwIndex-- > 0; )
		{
			if ( *(pLine++) != MAPCELL_DISALLOWED )
			{
				ASSERT ( FALSE );
			}
		}

		// zkontroluje lev� a prav� okraj mapy
		for ( DWORD dwIndexY = m_nSizeY; dwIndexY-- > 0; )
		{
			pLine += m_nSizeX;
			// zkontroluje lev� a prav� okraj ��dky
			for ( DWORD dwIndexX = 2 * MAP_BORDER; dwIndexX-- > 0; )
			{
				if ( *(pLine++) != MAPCELL_DISALLOWED )
				{
					ASSERT ( FALSE );
				}
			}
		}

		// zkontroluje doln� okraj mapy
		for ( dwIndex = m_nMapSizeX * MAP_BORDER - MAP_BORDER; dwIndex-- > 0; )
		{
			if ( *(pLine++) != MAPCELL_DISALLOWED )
			{
				ASSERT ( FALSE );
			}
		}
#endif //_DEBUG

		// zjist�, m�-li se kontrolovat povrch mapy
		if ( bCheckCompleteLand )
		{	// m� se kontrolovat povrch mapy
			// ukazatel na ��dek
			signed char *pLine = GetAt ( MAP_BORDER, MAP_BORDER ) - 1;

			// zkontroluje ��dky mapy
			for ( DWORD dwIndexY = m_nSizeY; dwIndexY-- > 0; )
			{
				// zkontroluje ��dku mapy
				for ( DWORD dwIndexX = m_nSizeX; dwIndexX-- > 0; )
				{
					// zjist�, je-li MapCell neinicializovan�
					if ( *(++pLine) == MAPCELL_UNINITIALIZED )
					{	// MapCell je neinicializovan�
						// p�ep�e MapCell na zak�zan�
						*pLine = MAPCELL_DISALLOWED;
						// nastav� p��znak chybn�ho povrchu mapy
						eReturnValue = ECFPGE_IncompleteMapLand;
					}
				}
				// nech� zkontrolovat dal�� ��dku mapy
				pLine += 2 * MAP_BORDER;
			}
		}
		// nem� se kontrolovat povrch mapy
	}
	// p��padn� chyba byla obslou�ena

	// vr�t� n�vratovou hodnotu
	return eReturnValue;
}

// zni�� mapu
void CCFPGMap::Delete () 
{
#ifndef __SHARED_MEMORY__
	// zjist�, je-li mapa platn�
	if ( m_pMap != NULL )
	{	// mapa je platn�
		// zni�� nakreslenou mapu
		delete [] m_pMap;
		// zne�kodn� nakreslenou mapu
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

// vytvo�� graf pro hled�n� cesty v archivu "cFindPathGraphsArchive"
void CCFPGMap::CreateFindPathGraph ( CDataArchive cFindPathGraphsArchive ) 
{
// vytvo�� tabulku hint� cest
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
		// identifik�tor MapCellu st�edu star�ho kruhu
		signed char cOldMapCellID = GetMapCellIDFromCircleSize ( 1 );
		// zpracuje v�echny velikosti kruh�
		for ( int nCircleSize = 2; nCircleSize <= MAX_CIRCLE_SIZE; 
			nCircleSize++ )
		{	// pokryje mapu kruhy velikosti "nCircleSize"
			// posun kruhu v��i jeho st�edu
			int nCircleOffset = ( nCircleSize - 1 ) / 2;
			// ukazatel na st�ed kruhu v map�
			signed char *pMap = GetAt ( MAP_BORDER + 1 + nCircleOffset, 
				MAP_BORDER + 1 + nCircleOffset );
			// ukazatel na origin�ln� kruh
			signed char *pOriginalCircle = m_aCircles[nCircleSize-2];
			// identifik�tor MapCellu st�edu nov�ho kruhu
			signed char cNewMapCellID = GetMapCellIDFromCircleSize ( nCircleSize );

			// zpracuje mapu
			for ( int nPositionY = m_nSizeY - nCircleSize; nPositionY-- > 0; 
				pMap += 2 * MAP_BORDER + nCircleSize )
			{
				for ( int nPositionX = m_nSizeX - nCircleSize; nPositionX-- > 0; pMap++ )
				{
					// zjist�, jedn�-li se o potenci�ln� MapCell st�edu kruhu
					if ( *pMap == cOldMapCellID )
					{	// jedn� se o potenci�ln� st�ed kruhu
						// ukazatel na kruh v map�
						signed char *pMapCircle = pMap - nCircleOffset * m_nMapSizeX - 
							nCircleOffset;
						// ukazatel na kruh
						signed char *pCircle = pOriginalCircle;
						// p��znak um�st�n� kruhu na map�
						BOOL bCirclePlaced = TRUE;

						// zkontroluje um�st�n� kruhu
						for ( int nIndex = nCircleSize * nCircleSize; nIndex-- > 0; 
							pMapCircle++, pCircle++ )
						{
							// zjist�, lze-li kruh um�stit
							if ( ( *pCircle == 1 ) && ( *pMapCircle == 0 ) )
							{	// kruh nelze um�stit
								// nastav� p��znak neum�st�n� kruhu
								bCirclePlaced = FALSE;
								break;
							}

							// zjist�, m�-li se p�ej�t na dal�� ��dek
							if ( nIndex % nCircleSize == 0 )
							{	// m� se p�ej�t na dal�� ��dek
								// posune se na map� na dal�� ��dek
								pMapCircle += m_nMapSizeX - nCircleSize;
							}
						}

						// zjist�, lze-li kruh um�stit na mapu
						if ( bCirclePlaced )
						{	// kruh lze um�stit na mapu
							// um�st� MapCell st�edu kruhu na mapu
							*pMap = cNewMapCellID;
						}
					}
					// nejedn� se o potenci�ln� st�ed kruhu
				}
			}
			// mapa je zpracov�na

			// aktualizuje identifi�tor MapCellu st�edu star�ho kruhu
			cOldMapCellID = cNewMapCellID;
		}
		// v�echny velikosti kruh� jsou zpracov�ny
	}
	// mapa je pokryta kruhy

// najde oblasti nejv�t��ch kruh�
	{
		// omezen� um�st�n� nejv�t��ho kruhu
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
				// zjist�, jedn�-li se o MapCell s nejv�t��m kruhem
				if ( *pMap == GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ) )
				{	// jedn� se o MapCell s nejv�t��m kruhem
					// nech� ozna�it oblast nejv�t��ho kruhu
					RepaintArea ( GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ), 
						MARK_EXPLORED ( GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ) ), 
						nPositionX, nPositionY );
					ASSERT ( *pMap == MARK_EXPLORED ( GetMapCellIDFromCircleSize ( 
						MAX_CIRCLE_SIZE ) ) );
					// p�id� oblast do seznamu oblast�/most�
					m_cAreaBridgeList.Add ( new ( m_cAreaBridgePool ) SAreaBridge 
						( CPoint ( nPositionX, nPositionY ), GetMapCellIDFromCircleSize ( 
						MAX_CIRCLE_SIZE ) ) );
				}
			}
		}
		// mapa je zpracov�na
	}
	// jsou nalezeny oblasti nejv�t��ch kruh�

// zru�� ozna�en� oblast� nejv�t��ch kruh�
	{
		// ukazatel na oblast
		struct SAreaBridge *pArea;
		// pozice oblasti v seznamu oblast�/most�
		POSITION posArea = m_cAreaBridgeList.GetHeadPosition ();

		// projede v�echny oblasti nejv�t��ch kruh�
		while ( m_cAreaBridgeList.GetNext ( posArea, pArea ) )
		{
			ASSERT ( pArea->cMapCellID == GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ) );
			// zru�� ozna�en� oblasti nejv�t��ho kruhu
			RepaintArea ( MARK_EXPLORED ( GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ) ), 
				GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ), pArea->pointPosition.x, 
				pArea->pointPosition.y );
			ASSERT ( *GetAt ( pArea->pointPosition.x, pArea->pointPosition.y ) == 
				GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ) );
			// ozna�� MapCell za reprezentanta oblasti
			*GetAt ( pArea->pointPosition.x, pArea->pointPosition.y ) = 
				GetMapCellAreaIDFromCircleSize ( MAX_CIRCLE_SIZE );
		}
	}
	// jsou zru�ena ozna�en� oblast� nejv�t��ch kruh�

// najde br�ny most� a fiktivn� oblasti
	{
		// inicializuje offsety okoln�ch MapCell�
		m_aSurroundingMapCellOffset[0] = -m_nMapSizeX - 1;
		m_aSurroundingMapCellOffset[1] = -m_nMapSizeX;
		m_aSurroundingMapCellOffset[2] = -m_nMapSizeX + 1;
		m_aSurroundingMapCellOffset[3] = 1;
		m_aSurroundingMapCellOffset[4] = m_nMapSizeX + 1;
		m_aSurroundingMapCellOffset[5] = m_nMapSizeX;
		m_aSurroundingMapCellOffset[6] = m_nMapSizeX - 1;
		m_aSurroundingMapCellOffset[7] = -1;

		// inicializuje offsety souvisl�ch MapCell�
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

		// inicializuje offsety vzd�len�ch okoln�ch MapCell�
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

		// inicializuje offsety vzd�len�ch souvisl�ch MapCell�
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

		// inicializuje offsety souvisl�ch MapCell�
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

		// zpracuje sestupn� zb�vaj�c� velikosti kruh� (tj. bez maxim�ln� velikosti)
		for ( int nCircleSize = MAX_CIRCLE_SIZE; --nCircleSize >= 2; )
		{	// najde mosty a fiktivn� oblasti velikosti kruhu "nCircleSize"
			// ID MapCellu st�edu kruhu "nCircleSize"
			signed char cMapCellID = GetMapCellIDFromCircleSize ( nCircleSize );

			// omezen� um�st�n� kruhu
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
					// zjist�, jedn�-li se o MapCell s hledan�m kruhem
					if ( *pMap == cMapCellID )
					{	// jedn� se o MapCell s hledan�m kruhem
						// zjist�, je-li v okol� MapCellu v�t�� kruh
						if ( ( *(pMap - 1) <= cMapCellID ) && ( *(pMap + 1) <= cMapCellID ) && 
							( *(pMap - m_nMapSizeX - 1) <= cMapCellID ) && 
							( *(pMap - m_nMapSizeX) <= cMapCellID ) && 
							( *(pMap - m_nMapSizeX + 1) <= cMapCellID ) && 
							( *(pMap + m_nMapSizeX - 1) <= cMapCellID ) && 
							( *(pMap + m_nMapSizeX) <= cMapCellID ) && 
							( *(pMap + m_nMapSizeX + 1) <= cMapCellID ) )
						{	// v okol� MapCellu nen� v�t�� kruh - jedn� se o potenci�ln� most
							ASSERT ( pMap == GetAt ( nPositionX, nPositionY ) );
							// seznam bran mostu
							CBridgeGateList cBridgeGateList;
							// nech� naj�t br�ny mostu a ozna�� most
							GetBridgeGates ( cMapCellID, pMap, cBridgeGateList );
							// p�id� ozna�en� most na z�sobn�k ozna�en�ch oblast�
							m_cMarkedAreaStack.Push ( SMarkedArea ( nPositionX, nPositionY ) );

							// zjist�, jedn�-li se o novou fiktivn� oblast, v�b�ek nebo most
							switch ( cBridgeGateList.GetSize () )
							{
								// jedn� se o novou fiktivn� oblast
								case 0 :
								{
									// p�id� fiktivn� oblast do seznamu oblast�/most�
									m_cAreaBridgeList.Add ( new ( m_cAreaBridgePool ) 
										SAreaBridge ( CPoint ( nPositionX, nPositionY), 
										cMapCellID ) );
									break;
								}
								// jedn� se o v�b�ek
								case 1 :
								{
									// vyjme br�nu mostu ze seznamu bran mostu
									struct SBridgeGate *pBridgeGate = 
										cBridgeGateList.RemoveFirst ();
									ASSERT ( pBridgeGate != NULL );
									ASSERT ( cBridgeGateList.IsEmpty () );
									// zni�� br�nu mostu
									m_cBridgeGatePool.Free ( pBridgeGate );
									break;
								}
								// jedn� se o most
								default:
								{
									// ukazatel na oblast/most
									struct SAreaBridge *pAreaBridge;
									// pozice v seznamu oblast�/most�
									POSITION posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();
									// p��znak nalezen� vl�zk� oblasti/mostu
									BOOL bCloseAreaBridgeFound = FALSE;

									// projede oblasti/mosty "cMapCellID"
									while ( m_cAreaBridgeList.GetNext ( posAreaBridge, 
										pAreaBridge ) )
									{
										// zjist�, jedn�-li se o hledanou oblast/most
										if ( pAreaBridge->cMapCellID != cMapCellID )
										{	// nejedn� se o hledan� oblasti/mosty
											// ukon�� kontrolu oblast�/most� "cMapCellID"
											break;
										}
										// jedn� se o hledanou oblast/most

										// zjist�, je-li oblast/most bl�zko nov�ho mostu
										if ( ( GetDistance ( pAreaBridge->pointPosition.x, 
											nPositionX ) <= 1 ) && ( GetDistance ( 
											pAreaBridge->pointPosition.y, nPositionY ) <= 1 ) )
										{	// oblast/most je bl�zko nov�ho mostu
											// nastav� p��znak nalezen� bl�zk� oblasti/mostu
											bCloseAreaBridgeFound = TRUE;
											// ukon�� kontrolu oblast�/most� "cMapCellID"
											break;
										}
									}

									// zjist�, byla-li nalezena bl�zk� oblast/most
									if ( bCloseAreaBridgeFound )
									{	// byla nalezena bl�zk� oblast/most
										// zni�� br�ny nov�ho mostu
										while ( !cBridgeGateList.IsEmpty () )
										{
											// odebere dal�� br�nu nov�ho mostu
											struct SBridgeGate *pNewBridgeGate = 
												cBridgeGateList.RemoveFirst ();
											// ukazatel na br�nu mostu
											struct SBridgeGate *pBridgeGate;
											// pozice v seznamu bran mostu
											POSITION posBridgeGate = 
												pAreaBridge->cPrimaryBridgeGateList.GetHeadPosition ();
											// projede br�ny mostu
											while ( pAreaBridge->cPrimaryBridgeGateList.GetNext ( 
												posBridgeGate, pBridgeGate ) )
											{
												// zjist�, jedn�-li se o stejnou br�nu mostu
												if ( ( pNewBridgeGate->pArea == 
													pBridgeGate->pArea ) && 
													( GetDistance ( pNewBridgeGate->pointPosition.x, 
													pBridgeGate->pointPosition.x ) <= 1 ) && 
													( GetDistance ( pNewBridgeGate->pointPosition.y, 
													pBridgeGate->pointPosition.y ) <= 1 ) )
												{	// jedn� se o duplicitn� br�nu mostu
													// zni�� duplicitn� br�nu nov�ho mostu
													m_cBridgeGatePool.Free ( pNewBridgeGate );
													pNewBridgeGate = NULL;
													// ukon�� prohled�v�n� bran mostu
													break;
												}
												// nejedn� se o stejnou br�nu
											}
											// zjist�, byla-li br�na mostu duplicitn�
											if ( pNewBridgeGate != NULL )
											{	// br�na mostu nebyla duplicitn�
												// inicializuje most br�ny mostu
												pNewBridgeGate->pBridge = pAreaBridge;
												// p�id� most br�ny do sekund�rn�ch bran most� oblasti
												pNewBridgeGate->pArea->cSecondaryBridgeGateList.Add ( 
													pNewBridgeGate );
												// p�id� br�nu mostu k mostu
												pAreaBridge->cPrimaryBridgeGateList.Add ( 
													pNewBridgeGate );
											}
											// br�na mostu byla duplicitn�
										}
										// br�ny nov�ho mostu byly zni�eny
									}
									else
									{	// nebyla nalezena bl�zk� oblast/most
										// vytvo�� nov� most
										struct SAreaBridge *pBridge = new ( m_cAreaBridgePool ) 
											SAreaBridge ( CPoint ( nPositionX, nPositionY ), 
											cMapCellID );
										// p�id� most do seznamu oblast�/most�
										m_cAreaBridgeList.Add ( pBridge );

										// ukazatel na br�nu mostu
										struct SBridgeGate *pBridgeGate;
										// pozice v seznamu bran mostu
										POSITION posBridgeGate = cBridgeGateList.GetHeadPosition ();
										// inicializuje br�ny mostu
										while ( cBridgeGateList.GetNext ( posBridgeGate, 
											pBridgeGate ) )
										{
											ASSERT ( ( pBridgeGate->pArea != NULL ) && 
												( pBridgeGate->pBridge == NULL ) );
											// inicializuje most br�ny mostu
											pBridgeGate->pBridge = pBridge;
											// p�id� most br�ny do sekund�rn�ch bran most� oblasti
											pBridgeGate->pArea->cSecondaryBridgeGateList.Add ( 
												pBridgeGate );
										}
										// br�ny mostu byly inicializov�ny

										// inicializuje prim�rn� br�ny p�idan�ho mostu
										cBridgeGateList.MoveList ( pBridge->cPrimaryBridgeGateList );
									}
									break;
								}
							}
							// zpracoval fiktivn� oblast, v�b�ek i most
						}
						else
						{	// v okol� MapCellu je v�t�� kruh
							// po�et souvisl�ch oblast� v�t��ch kruh�
							int nContinuousAreaCount = 0;

							// spo��t� po�et souvisl�ch oblast� v�t��ch kruh� v okoln� MapCellu
							for ( int i = 8; i-- > 0; )
							{
								// ukazatel na MapCell v okol�
								signed char *pMapCell = pMap + m_aSurroundingMapCellOffset[i];

								// zjist�, jedn�-li se o v�t�� MapCell
								if ( *pMapCell > cMapCellID )
								{	// jedn� se o v�t�� MapCell
									// zjist�, je-li MapCell pokra�ov�n�m souvisl� oblasti v�t��ch 
									//		MapCell�
									if ( ( *(pMap + m_aContinuousMapCellOffset[i][1]) <= 
										cMapCellID ) && ( ( m_aContinuousMapCellOffset[i][0] == 0 ) || 
										( *(pMap + m_aContinuousMapCellOffset[i][0]) <= 
										cMapCellID ) ) )
									{	// nejedn� se o pokra�ov�n� souvisl� oblasti v�t��ch kruh�
										// zv��� po�et souvisl�ch oblast� v�t��ch kruh�
										nContinuousAreaCount++;
									}
								}
								// okoln� MapCell byl zpracov�n
							}
							// spo��tal souvisl� oblasti v�t��ch kruh� v okol� MapCellu

							// zjist� po�et oblast� v�t��ch kruh� v okol� MapCellu
							if ( nContinuousAreaCount > 1 )
							{	// v okol� MapCellu je v�ce oblast� v�t��ch kruh� - jedn� se 
								//		o most
								// seznam bran mostu
								CBridgeGateList cBridgeGateList;
								// nech� naj�t br�ny mostu a ozna�� most
								GetBridgeGates ( cMapCellID, pMap, cBridgeGateList );
								ASSERT ( cBridgeGateList.GetSize () >= 2 );
								// p�id� ozna�en� most na z�sobn�k ozna�en�ch oblast�
								m_cMarkedAreaStack.Push ( SMarkedArea ( nPositionX, nPositionY ) );

								// ukazatel na oblast/most
								struct SAreaBridge *pAreaBridge;
								// pozice v seznamu oblast�/most�
								POSITION posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();
								// p��znak nalezen� bl�zk� oblasti/mostu
								BOOL bCloseAreaBridgeFound = FALSE;

								// projede oblasti/mosty "cMapCellID"
								while ( m_cAreaBridgeList.GetNext ( posAreaBridge, 
									pAreaBridge ) )
								{
									// zjist�, jedn�-li se o hledanou oblast/most
									if ( pAreaBridge->cMapCellID != cMapCellID )
									{	// nejedn� se o hledan� oblasti/mosty
										// ukon�� kontrolu oblast�/most� "cMapCellID"
										break;
									}
									// jedn� se o hledanou oblast/most

									// zjist�, je-li oblast/most bl�zko nov�ho mostu
									if ( ( GetDistance ( pAreaBridge->pointPosition.x, 
										nPositionX ) <= 2 ) && ( GetDistance ( 
										pAreaBridge->pointPosition.y, nPositionY ) <= 2 ) )
									{	// oblast/most je bl�zko nov�ho mostu
										// nastav� p��znak nalezen� bl�zk� oblasti/mostu
										bCloseAreaBridgeFound = TRUE;
										// ukon�� kontrolu oblast�/most� "cMapCellID"
										break;
									}
								}

								// zjist�, byla-li nalezena bl�zk� oblast/most
								if ( bCloseAreaBridgeFound )
								{	// byla nalezena bl�zk� oblast/most
									// zni�� br�ny mostu
									while ( !cBridgeGateList.IsEmpty () )
									{
										// vyjme br�nu mostu ze seznamu
										struct SBridgeGate *pBridgeGate = 
											cBridgeGateList.RemoveFirst ();
										ASSERT ( ( pBridgeGate->pArea != NULL ) && 
											( pBridgeGate->pBridge == NULL ) );
										// zni�� br�nu mostu
										m_cBridgeGatePool.Free ( pBridgeGate );
									}
									// br�ny mostu byly zni�eny
								}
								else
								{	// nebyla nalezena bl�zk� oblast/most
									// vytvo�� nov� most
									struct SAreaBridge *pBridge = new ( m_cAreaBridgePool ) 
										SAreaBridge ( CPoint ( nPositionX, nPositionY ), 
										cMapCellID );
									// p�id� most do seznamu oblast�/most�
									m_cAreaBridgeList.Add ( pBridge );
		
									// ukazatel na br�nu mostu
									struct SBridgeGate *pBridgeGate;
									// pozice v seznamu bran mostu
									POSITION posBridgeGate = cBridgeGateList.GetHeadPosition ();
									// projede br�ny mostu
									while ( cBridgeGateList.GetNext ( posBridgeGate, 
										pBridgeGate ) )
									{
										ASSERT ( ( pBridgeGate->pArea != NULL ) && 
											( pBridgeGate->pBridge == NULL ) );
										// inicializuje most br�ny mostu
										pBridgeGate->pBridge = pBridge;
										// p�id� most br�ny do sekund�rn�ch bran most� oblasti
										pBridgeGate->pArea->cSecondaryBridgeGateList.Add ( 
											pBridgeGate );
									}

									// inicializuje prim�rn� br�ny p�idan�ho mostu
									cBridgeGateList.MoveList ( pBridge->cPrimaryBridgeGateList );
								}
								// most byl zpracov�n
							}
							else
							{	// v okol� MapCellu je jedin� oblast v�t��ch kruh� - jedn� se 
								//		o potenci�ln� most d�lky 2
								ASSERT ( m_cTemporarilyMarkedMapCellStack.IsEmpty () );

								// ozna�� oblast v�t��ch kruh� v okol� a vzd�len�m okol� MapCellu
								for ( i = 8; i-- > 0; )
								{
									// ukazatel na MapCell v okol�
									signed char *pMapCell = pMap + m_aSurroundingMapCellOffset[i];

									// zjist�, jedn�-li se o v�t�� MapCell
									if ( *pMapCell > cMapCellID )
									{	// jedn� se o v�t�� MapCell
										// projede vzd�len� okoln� MapCelly
										for ( int j = 5; j-- > 0; )
										{
											// offset vzd�len�ho MapCellu
											int nFarContinuousMapCellOffset = 
												m_aFarContinuousMapCellOffset[i][j];
											// zjist�, jedn�-li se o platn� offset MapCellu
											if ( nFarContinuousMapCellOffset == 0 )
											{	// nejedn� se o platn� offset MapCellu
												continue;
											}

											// ukazatel na MapCell ze vzd�len�ho okol�
											signed char *pFarMapCell = pMap + 
												nFarContinuousMapCellOffset;

											// zjist�, jedn�-li se o v�t�� MapCell
											if ( *pFarMapCell > cMapCellID )
											{	// jedn� se o v�t�� MapCell
												// ozna�� vzd�len� MapCell
												*pFarMapCell = MARK_EXPLORED ( *pFarMapCell );
												m_cTemporarilyMarkedMapCellStack.Push ( pFarMapCell );

												// index vzd�len�ho MapCellu
												int nFarIndex = ( i + 7 ) * 2 + j;

												// ozna�� n�sleduj�c� sousedn� vzd�len� v�t�� MapCelly
												for ( int k = nFarIndex; ; )
												{
													k = ( k + 1 ) & 0x0f;	// k = ( k + 1 ) % 16
													// ukazatel na n�sleduj�c� sousedn� vzd�len� MapCell
													signed char *pNextFarMapCell = pMap + 
														m_aFarSurroundingMapCellOffset[k];
													// zjist�, jedn�-li se o v�t�� MapCell
													if ( *pNextFarMapCell <= cMapCellID )
													{	// nejedn� se o v�t�� MapCell
														// ukon�� ozna�ov�n� n�sleduj�c�ch v�t��ch 
														//		vzd�len�ch MapCell�
														break;
													}
													// ozna�� vzd�len� MapCell
													*pNextFarMapCell = MARK_EXPLORED ( 
														*pNextFarMapCell );
													m_cTemporarilyMarkedMapCellStack.Push ( 
														pNextFarMapCell );
												}
												// n�sleduj�c� sousedn� vzd�len� v�t�� MapCelly jsou 
												//		ozna�eny

												// ozna�� p�edchoz� sousedn� vzd�len� v�t�� MapCelly
												for ( k = nFarIndex; ; )
												{
													k = ( k + 15 ) & 0x0f;		// k = ( k - 1 ) % 16
													// ukazatel na p�edchoz� sousedn� vzd�len� MapCell
													signed char *pPreviousFarMapCell = pMap + 
														m_aFarSurroundingMapCellOffset[k];
													// zjist�, jedn�-li se o v�t�� MapCell
													if ( *pPreviousFarMapCell <= cMapCellID )
													{	// nejedn� se o v�t�� MapCell
														// ukon�� ozna�ov�n� p�edch�zej�c�ch v�t��ch 
														//		vzd�len�ch MapCell�
														break;
													}
													// ozna�� vzd�len� MapCell
													*pPreviousFarMapCell = MARK_EXPLORED ( 
														*pPreviousFarMapCell );
													m_cTemporarilyMarkedMapCellStack.Push ( 
														pPreviousFarMapCell );
												}
												// p�edchoz� sousedn� vzd�len� v�t�� MapCelly jsou 
												//		ozna�eny
											}
											// v�t�� vzd�len� MapCell byl zpracov�n
										}
										// vzd�len� okoln� MapCelly jsou projety
									}
									// okoln� MapCell byl zpracov�n
								}
								// okoln� a sousedn� vzd�len� okoln� v�t�� kruhy byly ozna�eny

								// seznam oblast� v okol� MapCellu = seznam bran mostu
								ASSERT ( m_cMapCellQueue.IsEmpty () );

								// najde dal�� oblasti ve vzd�len�m okol� MapCellu
								for ( i = 16; i-- > 0; )
								{
									// ukazatel na MapCell ze vzd�len�ho okol�
									signed char *pFarMapCell = pMap + 
										m_aFarSurroundingMapCellOffset[i];
									// zjist�, jedn�-li se o v�t�� MapCell
									if ( *pFarMapCell > cMapCellID )
									{	// jedn� se o v�t�� MapCell
										// ukazatel na n�sleduj�c� MapCell
										signed char *pNextFarMapCell = pMap + 
											m_aFarSurroundingMapCellOffset[( i + 1 ) & 0x0f];
												// ( i + 1 ) % 16
										// zjist�, jedn�-li se o konec souvisl� oblasti
										if ( *pNextFarMapCell <= cMapCellID )
										{	// jedn� se o konec souvisl� oblasti
											// zjist�, je-li oblast dostupn�
											for ( int j = i + 1; ; )
											{
												j = ( j + 15 ) & 0x0f;	// j = ( j - 1 ) % 16
												// zjist�, je-li p�edch�zej�c� MapCell sou��st� 
												//		oblasti
												if ( *(pMap + m_aFarSurroundingMapCellOffset[j]) <= 
													cMapCellID )
												{	// p�edch�zej�c� MapCell nen� sou��st� oblasti
													// ukon�� zji��ov�n� dostupnosti oblasti
													break;
												}
												// zjist�, je-li MapCell dostupn�
												if ( ( ( m_aSurroundingFarMapCellOffset[j][0] != 0 ) && 
													( *(pMap + m_aSurroundingFarMapCellOffset[j][0]) >= 
													cMapCellID ) ) || 
													( *(pMap + m_aSurroundingFarMapCellOffset[j][1]) >= 
													cMapCellID ) || 
													( ( m_aSurroundingFarMapCellOffset[j][2] != 0 ) && 
													( *(pMap + m_aSurroundingFarMapCellOffset[j][2]) >= 
													cMapCellID ) ) )
												{	// MapCell je dostupn�
													// p�id� MapCell do fronty bran mostu
													m_cMapCellQueue.Add ( pFarMapCell );
													// ukon�� zji��ov�n� dostupnosti oblasti
													break;
												}
												// MapCell nen� dostupn�
											}
											// zjistil dostupnost oblasti
										}
										// nejden� se o konec souvisl� oblasti
									}
									// nejedn� se o vzd�len� v�t�� MapCell
								}
								// oblasti ve vzd�len�m okol� byly nalezeny

								// ukazatel na ozna�en� MapCell
								signed char *pMarkedMapCell;
								// odzna�� do�asn� ozna�en� MapCelly
								while ( m_cTemporarilyMarkedMapCellStack.Pop ( pMarkedMapCell ) )
								{
									ASSERT ( *pMarkedMapCell == MARK_EXPLORED ( *pMarkedMapCell ) );
									// odzna�� ozna�en� MapCell
									*pMarkedMapCell = UNMARK_EXPLORED ( *pMarkedMapCell );
								}
								// do�asn� ozna�en� MapCelly byly odzna�eny

								// zjist�, jsou-li ve vzd�len�m okol� MapCellu dal�� oblasti = 
								//		br�ny mostu
								if ( !m_cMapCellQueue.IsEmpty () )
								{	// ve vzd�len�m okol� MapCellu jsou dal�� oblasti
									// ukazatel na oblast/most
									struct SAreaBridge *pAreaBridge;
									// pozice v seznamu oblast�/most�
									POSITION posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();
									// p��znak nalezen� vl�zk� oblasti/mostu
									BOOL bCloseAreaBridgeFound = FALSE;

									// projede oblasti/mosty "cMapCellID"
									while ( m_cAreaBridgeList.GetNext ( posAreaBridge, 
										pAreaBridge ) )
									{
										// zjist�, jedn�-li se o hledanou oblast/most
										if ( pAreaBridge->cMapCellID != cMapCellID )
										{	// nejedn� se o hledan� oblasti/mosty
											// ukon�� kontrolu oblast�/most� "cMapCellID"
											break;
										}
										// jedn� se o hledanou oblast/most

										// zjist�, je-li oblast/most bl�zko nov�ho mostu
										if ( ( GetDistance ( pAreaBridge->pointPosition.x, 
											nPositionX ) <= 2 ) && ( GetDistance ( 
											pAreaBridge->pointPosition.y, nPositionY ) <= 2 ) )
										{	// oblast/most je bl�zko nov�ho mostu
											// nastav� p��znak nalezen� bl�zk� oblasti/mostu
											bCloseAreaBridgeFound = TRUE;
											// ukon�� kontrolu oblast�/most� "cMapCellID"
											break;
										}
									}

									// zjist�, byla-li nalezena bl�zk� oblast/most
									if ( bCloseAreaBridgeFound )
									{	// byla nalezena bl�zk� oblast/most
										// zni�� frontu MapCell� bran mostu
										m_cMapCellQueue.RemoveAll ();
									}
									else
									{	// nebyla nalezena bl�zk� oblast/most
										// vytvo�� nov� most
										struct SAreaBridge *pBridge = new ( m_cAreaBridgePool ) 
											SAreaBridge ( CPoint ( nPositionX, nPositionY ), 
											cMapCellID );
										// p�id� most do seznamu oblast�/most�
										m_cAreaBridgeList.Add ( pBridge );

										// ukazatel na nejv�t�� MapCell v okol�
										signed char *pMaximumMapCell = pMap;

										// projede okoln� MapCelly
										for ( int i = 8; i-- > 0; )
										{
											// ukazatel na MapCell v okol�
											signed char *pMapCell = pMap + 
												m_aSurroundingMapCellOffset[i];

											// zjist�, jedn�-li se o v�t�� MapCell
											if ( *pMapCell > *pMaximumMapCell )
											{	// jedn� se o v�t�� MapCell
												// uschov� si ukazatel na v�t�� MapCell v okol�
												pMaximumMapCell = pMapCell;
											}
											// okoln� MapCell byl zpracov�n
										}
										// na�el nejv�t�� MapCell v okol�
										ASSERT ( *pMaximumMapCell > *pMap );

										// p�id� nejv�t�� MapCell v okol� do fronty bran most�
										m_cMapCellQueue.Add ( pMaximumMapCell );

										// ukazatel na MapCell br�ny mostu
										signed char *pMapCell;
										// vyrob� br�ny mostu
										while ( m_cMapCellQueue.RemoveFirst ( pMapCell ) )
										{
											// nech� naj�t oblast/most MapCellu
											struct SAreaBridge *pAreaBridge = FindAreaBridge ( 
												pMapCell );

											// vyrob� novou br�nu mostu
											struct SBridgeGate *pBridgeGate = 
												m_cBridgeGatePool.Allocate ();
											// inicializuje nov� vytvo�enou br�nu mostu
											pBridgeGate->pointPosition = GetMapCellPosition ( 
												pMapCell );
											pBridgeGate->pArea = pAreaBridge;
											pBridgeGate->pBridge = pBridge;

											// p�id� br�nu mostu do sekund�rn�ch bran most� oblasti
											pAreaBridge->cSecondaryBridgeGateList.Add ( pBridgeGate );
											// p�id� vytvo�enou br�nu mostu do seznamu bran mostu
											pBridge->cPrimaryBridgeGateList.Add ( pBridgeGate );
										}
										// br�ny mostu jsou vyrobeny
									}
									// bl�zk� oblast/most byla zpracov�na
								}
								// ve vzd�len�m okol� MapCellu nejsou dal�� oblasti
							}
							// nejedn� se o most
						}
						// MapCell s hledan�m kruhem byl zpracov�n
					}
					// MapCell byl zpracov�n
				}
			}
			// mapa je zpracov�na

		// zru�� ozna�en� ozna�en�ch oblast�

			// ozna�en� oblast
			struct SMarkedArea sMarkedArea;

			// projede v�echny ozna�en� oblasti
			while ( m_cMarkedAreaStack.Pop ( sMarkedArea ) )
			{
				// zjist�, je-li oblast st�le ozna�ena
				if ( *GetAt ( sMarkedArea.nX, sMarkedArea.nY ) != MARK_EXPLORED ( cMapCellID ) )
				{	// oblast ji� nen� ozna�ena
					// pokra�uje s dal�� ozna�enou oblast�
					continue;
				}
				// zru�� ozna�en� oblasti
				RepaintArea ( MARK_EXPLORED ( cMapCellID ), cMapCellID, sMarkedArea.nX, 
					sMarkedArea.nY );
				ASSERT ( *GetAt ( sMarkedArea.nX, sMarkedArea.nY ) == cMapCellID );
			}

		// ozna�� reprezentanty most� a fiktivn�ch oblast� "cMapCellID"

			// ukazatel na oblast/most
			struct SAreaBridge *pAreaBridge;
			// pozice v seznamu oblast�/most�
			POSITION posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();

			// projede v�echny oblasti/mosty "cMapCellID"
			while ( m_cAreaBridgeList.GetNext ( posAreaBridge, pAreaBridge ) )
			{
				// zjist�, jedn�-li se je�t� o oblast/most "cMapCellID"
				if ( pAreaBridge->cMapCellID != cMapCellID )
				{	// nejedn� se ji� o oblast/most "cMapCellID"
					break;
				}
				// jedn� se o oblast/most "cMapCellID"
				ASSERT ( *GetAt ( pAreaBridge->pointPosition ) == cMapCellID );
				// ozna�� MapCell za reprezentanta oblasti/mostu "cMapCellID"
				*GetAt ( pAreaBridge->pointPosition ) = GetMapCellAreaIDFromMapCellID ( 
					cMapCellID );
			}
		}
		// zpracoval v�echny zb�vaj�c� velikosti kruh�
	}
	// jsou nalezeny br�ny most� a fiktivn� oblasti

	// ukazatel na oblast/most
	struct SAreaBridge *pAreaBridge;
	// pozice v seznamu oblast�/most�
	POSITION posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();

	// zru�� ozna�en� reprezentant� oblast�/most�
	while ( m_cAreaBridgeList.GetNext ( posAreaBridge, pAreaBridge ) )
	{
		ASSERT ( pAreaBridge->cMapCellID != GetMapCellAreaIDFromMapCellID ( 
			pAreaBridge->cMapCellID ) );
		ASSERT ( GetMapCellAreaIDFromMapCellID ( pAreaBridge->cMapCellID ) == 
			*GetAt ( pAreaBridge->pointPosition ) );
		// zru�� ozna�en� reprezentanta oblasti/mostu
		*GetAt ( pAreaBridge->pointPosition ) = pAreaBridge->cMapCellID;
	}
	// zru�il ozna�en� reprezentanta oblasti/mostu

	// velikost tabulky oblast� (v po�tu DWORD�)
	DWORD dwAreaTableSize = 0;
	// po�et bran most�
	DWORD dwBridgeGateCount = 0;

#ifdef _DEBUG
	// pozice oblasti/mostu v seznamu
	posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();

	// znehodnot indexy bran most� oblasti
	while ( m_cAreaBridgeList.GetNext ( posAreaBridge, pAreaBridge ) )
	{
		// ukazatel na br�nu mostu
		struct SBridgeGate *pBridgeGate;
		// pozice prvn� prim�rn� br�ny mostu oblasti
		POSITION posBridgeGate = pAreaBridge->cPrimaryBridgeGateList.GetHeadPosition ();

		// znehodnot� indexy prim�rn�ch bran most�
		while ( CBridgeGateList::GetNext ( posBridgeGate, pBridgeGate ) )
		{
			// znehodnot� index br�ny mostu
			pBridgeGate->dwIndex = 0xffffffff;
		}
	}
#endif //_DEBUG

	// pozice oblasti/mostu v seznamu
	posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();

	// zjist� velikost tabulky oblast�, po�et bran a inicializuje indexy oblast� a bran 
	//		most�
	while ( m_cAreaBridgeList.GetNext ( posAreaBridge, pAreaBridge ) )
	{
		ASSERT ( ( pAreaBridge->cMapCellID != GetMapCellIDFromCircleSize ( 
			MAX_CIRCLE_SIZE ) ) || ( pAreaBridge->cPrimaryBridgeGateList.IsEmpty () ) );

		// po�et bran most� oblasti
		DWORD dwAreaBridgeGateCount = 0;

		// ukazatel na br�nu mostu
		struct SBridgeGate *pBridgeGate;
		// pozice prvn� prim�rn� br�ny mostu oblasti
		POSITION posBridgeGate = pAreaBridge->cPrimaryBridgeGateList.GetHeadPosition ();

		// inicializuje indexy prim�rn�ch bran most�
		while ( CBridgeGateList::GetNext ( posBridgeGate, pBridgeGate ) )
		{
			// aktualizuje po�et bran most� oblasti
			dwAreaBridgeGateCount++;

			// inicializuje index br�ny mostu
			ASSERT ( pBridgeGate->dwIndex == 0xffffffff );
			pBridgeGate->dwIndex = dwBridgeGateCount;
			// aktualizuje po�et bran most�
			dwBridgeGateCount++;

			ASSERT ( pBridgeGate->pBridge == pAreaBridge );
		}
		ASSERT ( dwAreaBridgeGateCount != 1 );

		// pozice prvn� sekund�rn� br�ny mostu oblasti
		posBridgeGate = pAreaBridge->cSecondaryBridgeGateList.GetHeadPosition ();

		// inicializuje indexy sekund�rn�ch bran most�
		while ( CBridgeGateList::GetNext ( posBridgeGate, pBridgeGate ) )
		{
			// aktualizuje po�et bran most� oblasti
			dwAreaBridgeGateCount++;

			ASSERT ( pBridgeGate->pArea == pAreaBridge );
			// najde dotyk br�ny mostu k oblasti
			FindBridgeGateTouch ( pBridgeGate, pBridgeGate->pArea );
		}

		// zjist�, jedn�-li se o samostatnou oblast
		if ( dwAreaBridgeGateCount == 0 )
		{	// jedn� se o samostatnou oblast
			continue;
		}
		// nejedn� se o samostatnou oblast

		// inicializuje index oblasti
		pAreaBridge->dwIndex = dwAreaTableSize;

		// aktualizuje velikost tabulky oblast�
		dwAreaTableSize += dwAreaBridgeGateCount * dwAreaBridgeGateCount + 2;
	}
	// zjistil velikost tabulky oblast�, po�et bran a inicializoval indexy oblast� a bran 
	//		most�

	// alokuje tabulku oblast�
	DWORD *aAreaTable = new DWORD[dwAreaTableSize];
	// alokuje tabulku bran most�
	DWORD *aBridgeGateTable = new DWORD[4 * dwBridgeGateCount];
  // Fill it with nonsense values
  memset ( aBridgeGateTable, 0xFF, sizeof ( DWORD ) * 4 * dwBridgeGateCount );

	// pozice prvn� oblasti/mostu v seznamu
	posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();

	// index do tabulky oblast�
	DWORD dwAreaIndex = 0;
	// index do tabulky bran most�
	DWORD dwBridgeGateIndex = 0;

	// vytvo�� pole vzd�lenost� MapCell�
	m_cMapCellDistanceArray.Create ( m_nMapSizeX, m_nMapSizeY, MAPSQUARE_WIDTH, 
		INFINITE_DISTANCE );

	// zpracuje seznam oblast�/most�
	while ( m_cAreaBridgeList.GetNext ( posAreaBridge, pAreaBridge ) )
	{
		// po�et bran most� oblasti
		DWORD dwAreaBridgeGateCount = pAreaBridge->cPrimaryBridgeGateList.GetSize () + 
			pAreaBridge->cSecondaryBridgeGateList.GetSize ();

		TRACE_FIND_PATH_CREATION ( "-------Area %d - (%d,%d) - %d, gates: %d\n", dwAreaIndex, 
			pAreaBridge->pointPosition.x, pAreaBridge->pointPosition.y, 
			pAreaBridge->cMapCellID, dwAreaBridgeGateCount );

		// zjist�, jedn�-li se o samostatnou oblast
		if ( dwAreaBridgeGateCount == 0 )
		{	// jedn� se o samostatnou oblast
			continue;
		}
		// nejedn� se o samostatnou oblast

		ASSERT ( pAreaBridge->dwIndex == dwAreaIndex );

		// zap�e do tabulky oblast� po�et bran most� oblasti a ���ku oblasti
		aAreaTable[dwAreaIndex++] = dwAreaBridgeGateCount;
		aAreaTable[dwAreaIndex++] = (DWORD)( pAreaBridge->cMapCellID / 2 );

		// ukazatel na br�nu mostu
		struct SBridgeGate *pBridgeGate;
		// pozice prvn� prim�rn� br�ny mostu oblasti
		POSITION posBridgeGate = pAreaBridge->cPrimaryBridgeGateList.GetHeadPosition ();

		// seznam bran most� oblasti
		CBridgeGateList cBridgeGateList;
		// pozice br�ny mostu v seznamu
		POSITION posNewBridgeGate;
		// br�na mostu v seznamu
		struct SBridgeGate *pNewBridgeGate;

		// zap�e do tabulky oblast� indexy prim�rn�ch bran most� oblasti a do tabulky bran 
		//		most� indexy oblast�
		while ( CBridgeGateList::GetNext ( posBridgeGate, pBridgeGate ) )
		{
			TRACE_FIND_PATH_CREATION ( "\tBridgeGate %d - (%d,%d)\n", pBridgeGate->dwIndex, 
				pBridgeGate->pointPosition.x, pBridgeGate->pointPosition.y );

			// zap�e do tabulky oblast� index prim�rn� br�ny mostu oblasti
			aAreaTable[dwAreaIndex++] = pBridgeGate->dwIndex;
			// zap�e do tabulky bran most� indexy oblast�
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

			// p�id� br�nu mostu na konec seznamu bran most�
			posNewBridgeGate = cBridgeGateList.GetHeadPosition ();
			while ( CBridgeGateList::GetNext ( posNewBridgeGate, pNewBridgeGate ) );
			cBridgeGateList.Insert ( posNewBridgeGate, pBridgeGate );
		}

		// pozice prvn� sekund�rn� br�ny mostu oblasti
		posBridgeGate = pAreaBridge->cSecondaryBridgeGateList.GetHeadPosition ();

		// zap�e do tabulky oblast� indexy sekund�rn�ch bran most� oblasti a do tabulky bran 
		//		most� indexy obalst�
		while ( CBridgeGateList::GetNext ( posBridgeGate, pBridgeGate ) )
		{
			TRACE_FIND_PATH_CREATION ( "\tBridgeGate %d - (%d,%d)\n", pBridgeGate->dwIndex, 
				pBridgeGate->pointPosition.x, pBridgeGate->pointPosition.y );

			// zap�e do tabulky oblast� index sekund�rn� br�ny mostu oblasti
			aAreaTable[dwAreaIndex++] = pBridgeGate->dwIndex;
			// zap�e do tabulky bran most� indexy oblast�
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

			// p�id� br�nu mostu na konec seznamu bran most�
			posNewBridgeGate = cBridgeGateList.GetHeadPosition ();
			while ( CBridgeGateList::GetNext ( posNewBridgeGate, pNewBridgeGate ) );
			cBridgeGateList.Insert ( posNewBridgeGate, pBridgeGate );
		}

		// po�et bran most� oblasti
		DWORD dwBridgeGateCount = cBridgeGateList.GetSize ();

		// zjist� vzd�lenosti bran most� oblasti
		while ( !cBridgeGateList.IsEmpty () )
		{
			// vyjme prvn� br�nu mostu oblasti
			pBridgeGate = cBridgeGateList.RemoveFirst ();

			// zjist�, jedn�-li se o posledn� br�nu mostu oblasti
			if ( cBridgeGateList.IsEmpty () )
			{	// jedn� se o posledn� br�nu mostu oblasti
				// ukon�� zji��ov�n� vzd�lenost� bran most� oblasti
				break;
			}
			// nejedn� se o posledn� br�nu mostu oblasti

			// aktualizuje po�et bran most� oblasti
			dwBridgeGateCount--;
			ASSERT ( dwBridgeGateCount > 0 );

			// zjist� vzd�lenosti bran most� oblasti
			GetBridgeGateDistances ( pAreaBridge, pBridgeGate, cBridgeGateList, 
				&aAreaTable[dwAreaIndex] );
			// aktualizuje index do tabulky oblast�
			dwAreaIndex += 2 * dwBridgeGateCount;
		}
	}
	// seznam oblast�/most� byl zpracov�n

	// zni�� pole vzd�lenost� MapCell� bran most�
	m_cMapCellDistanceArray.Delete ();

	// zni�� seznam oblast�/most� a ozna�� br�ny most� a p�elo�� jejich sou�adnice
	while ( !m_cAreaBridgeList.IsEmpty () )
	{
		// vyjme oblast/most ze seznamu
		pAreaBridge = m_cAreaBridgeList.RemoveFirst ();

		// zni�� seznam prim�rn�ch bran most� oblasti
		while ( !pAreaBridge->cPrimaryBridgeGateList.IsEmpty () )
		{
			// vyjme dal�� br�nu mostu ze seznamu
			struct SBridgeGate *pBridgeGate = 
				pAreaBridge->cPrimaryBridgeGateList.RemoveFirst ();

			// ukazatel na MapCell br�ny mostu
			signed char *pMapCell = GetAt ( pBridgeGate->pointPosition );
			// ozna�� br�nu mostu
			*pMapCell = GetMapCellAreaIDFromMapCellID ( *pMapCell );
			// p�elo�� sou�adnice br�ny mostu
			ASSERT ( aBridgeGateTable[4 * pBridgeGate->dwIndex] >= MAP_BORDER );
			ASSERT ( pBridgeGate->pointPosition.x == (int)aBridgeGateTable[4 * pBridgeGate->dwIndex] );
			aBridgeGateTable[4 * pBridgeGate->dwIndex] -= MAP_BORDER;
			ASSERT ( aBridgeGateTable[4 * pBridgeGate->dwIndex + 1] >= MAP_BORDER );
			ASSERT ( pBridgeGate->pointPosition.y == (int)aBridgeGateTable[4 * pBridgeGate->dwIndex + 1] );
			aBridgeGateTable[4 * pBridgeGate->dwIndex + 1] -= MAP_BORDER;

			// zni�� br�nu mostu
			m_cBridgeGatePool.Free ( pBridgeGate );
		}

		// zni�� seznam sekund�rn�ch bran most�
		pAreaBridge->cSecondaryBridgeGateList.RemoveAll ();

		// zni�� oblast/most
		SAreaBridge::operator delete ( pAreaBridge, m_cAreaBridgePool );
	}
	// seznam oblast�/most� byl zni�en

	// velikost tabulky bran most�
	DWORD dwBridgeGateTableSize = 4 * dwBridgeGateCount;

	// vytvo�� adres�� grafu pro hled�n� cesty
	cFindPathGraphsArchive.MakeDirectory ( m_strFindPathGraphName );
	// otev�e archiv grafu pro hled�n� cesty
	CDataArchive cFindPathGraphArchive = cFindPathGraphsArchive.CreateArchive ( 
		m_strFindPathGraphName );

	// vytvo�� hlavi�kov� soubor grafu pro hled�n� cesty
	{
		// hlavi�kov� soubor grafu pro hled�n� cesty
		CArchiveFile cHeaderFile = cFindPathGraphArchive.CreateFile ( 
			FIND_PATH_GRAPH_HEADER_FILE_NAME, CFile::modeWrite | CFile::modeCreate );

		// vypln� hlavi�ku grafu pro hled�n� cesty
		SMFindPathGraphHeader sFindPathGraphHeader;
		sFindPathGraphHeader.m_dwAreaTableSize = dwAreaTableSize;
		sFindPathGraphHeader.m_dwBridgeGateTableSize = dwBridgeGateTableSize;
		sFindPathGraphHeader.m_dwPathHintTableSize = m_dwPathHintTableSize;

		// ulo�� do hlavi�kov�ho souboru hlavi�ku grafu pro hled�n� cesty
		cHeaderFile.Write ( &sFindPathGraphHeader, sizeof ( sFindPathGraphHeader ) );
		// ulo�� do hlavi�kov�ho souboru tabulky grafu pro hled�n� cesty
		cHeaderFile.Write ( aAreaTable, dwAreaTableSize * sizeof ( DWORD ) );
		cHeaderFile.Write ( aBridgeGateTable, dwBridgeGateTableSize * sizeof ( DWORD ) );
		cHeaderFile.Write ( m_pPathHintTable, m_dwPathHintTableSize * sizeof ( DWORD ) );
	}

	// zni�� tabulku oblast�
	if ( aAreaTable != NULL )
	{
		delete [] aAreaTable;
	}
	// zni�� tabulku bran most�
	if ( aBridgeGateTable != NULL )
	{
		delete [] aBridgeGateTable;
	}
	// zni�� tabulku hint� cest
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

	// vytvo�� datov� soubor grafu pro hled�n� cesty
	{
		// datov� soubor grafu pro hled�n� cesty
		CArchiveFile cFile = cFindPathGraphArchive.CreateFile ( _T("beta.version.data"), 
			CFile::modeWrite | CFile::modeCreate );

		// ukazatel na ��dek mapy
		signed char *pLine = GetAt ( MAP_BORDER, MAP_BORDER );

		// ulo�� do datov�ho souboru kruhy mapy
		for ( int nLineIndex = m_nSizeY; nLineIndex-- > 0; pLine += m_nMapSizeX )
		{
			// ulo�� do datov�ho souboru dal�� ��dek kruh� mapy
			cFile.Write ( pLine, m_nSizeX );
		}
	}
}

// p�ebarv� oblast "cOldColor" za��naj�c� na sou�adnic�ch "nX" a "nY" barvou "cNewColor"
void CCFPGMap::RepaintArea ( signed char cOldColor, signed char cNewColor, int nX, 
	int nY ) 
{
	ASSERT ( nX >= MAP_BORDER );
	ASSERT ( nX < MAP_BORDER + m_nSizeX );
	ASSERT ( nY >= MAP_BORDER );
	ASSERT ( nY < MAP_BORDER + m_nSizeY );
	ASSERT ( cOldColor != cNewColor );

	// aktualizuje barvu p�ebarvovan� oblasti
	m_cRepaintAreaSearchLineAreaColor = cOldColor;

	struct SRepaintAreaBlock sBlock;

	signed char *pLine = GetAt ( nX, nY );
	ASSERT ( *pLine == cOldColor );

	// nech� prohledat ��dku v okol� bodu
	RepaintAreaSearchLine ( pLine, nX, nX, nY );

	// zpracuje z�sobn�k
	while ( m_cRepaintAreaBlockStack.Pop ( sBlock ) )
	{
		// zjist� ukazatel na za��tek ��dku
		pLine = GetAt ( sBlock.nMinX, sBlock.nY );
		// zjist�, je-li ��dek ji� zpracov�n
		if ( *pLine == cOldColor )
		{	// ��dek dosud nebyl zpracov�n
			// uschov� si za��tek ��dku
			signed char *pLineStart = pLine - 1;
			// ozna�� ��dek za objeven�
			for ( int i = sBlock.nMaxX - sBlock.nMinX + 1; i-- > 0; pLine++ )
			{
				ASSERT ( *pLine == cOldColor );
				*pLine = cNewColor;
			}
			// prohled� horn� ��dku (v�etn� p�echodu p�es roh)
			RepaintAreaSearchLine ( pLineStart - m_nMapSizeX, sBlock.nMinX - 1, 
				sBlock.nMaxX + 1, sBlock.nY - 1 );
			// prohled� doln� ��dku (v�etn� p�echodu p�es roh)
			RepaintAreaSearchLine ( pLineStart + m_nMapSizeX, sBlock.nMinX - 1, 
				sBlock.nMaxX + 1, sBlock.nY + 1 );
		}
		// ��dek byl zpacov�n
	}
	// zpracoval cel� z�sobn�k
}

// prohled� ��dek "nY" od "nMinX" ("pLine") do "nMaxX" pro p�ekreslov�n� oblasti
void CCFPGMap::RepaintAreaSearchLine ( signed char *pLine, int nMinX, int nMaxX, int nY ) 
{
	ASSERT ( pLine == GetAt ( nMinX, nY ) );

	// zjist� barvu p�ebarvovan� oblasti
	signed char cAreaColor = m_cRepaintAreaSearchLineAreaColor;
	// ukazatel na za��tek ��dky
	signed char *pLineStart;

	// index za��tku oblasti na ��dce
	int nStartX;

	// zjist�, za��n�me-li ji� na oblasti
	if ( *pLine != cAreaColor )
	{	// za��n�me mimo oblast
		// najde za��tek oblasti do "nMaxX"
		do
		{
			nMinX++;
			pLine++;
		} while ( ( nMinX <= nMaxX ) && ( *pLine != cAreaColor ) );

		// zjist�, na�el-li za��tek oblasti do "nMaxX"
		if ( nMinX > nMaxX )
		{	// nena�el za��tek oblasti do "nMaxX"
			// ukon�� prohled�v�n� ��dky
			return;
		}

		// nastav� za��tek oblasti ��dky ("nStartX", "pLine" a "pLineStart")
		nStartX = nMinX;
		pLineStart = ++pLine;
	}
	else
	{	// za��n�me na oblasti
		// najde za��tek oblasti na ��dce
		pLineStart = pLine;
		do
		{
			pLine--;
		} while ( *pLine == cAreaColor );
		pLine++;

		// nastav� za��tek oblasti ��dky ("nStartX", "pLine" a "pLineStart")
		nStartX = nMinX - ( pLineStart - pLine );
		pLine = ++pLineStart;
	}
	// "pLine" = "pLineStart" je za "nStartX" o 1 MapCell

	// index konce oblasti na ��dce
	int nStopX = nMinX + 1;
	// najde v�echny oblasti na ��dce
	do
	{
		// "pLine" = "pLineStart" je na "nStopX" >= "nStartX"

		// najde konec oblasti na ��dce
		while ( *pLine == cAreaColor )
		{
			pLine++;
		}
		nStopX += pLine - pLineStart - 1;
		// "pLine" je za "nStopX" o 1 MapCell

		// p�id� oblast ��dky do z�sobn�ku blok� pro objevov�n� oblast�
		m_cRepaintAreaBlockStack.Push ( SRepaintAreaBlock ( nStartX, nStopX, nY ) );

		// za�ne za nalezenou oblast� na ��dce
		nStartX = nStopX + 1;
		// "pLine" je na "nStartX"

		// najde za��tek dal�� oblasti do "nMaxX"
		do
		{
			pLine++;
			nStartX++;
		} while ( ( nStartX <= nMaxX ) && ( *pLine != cAreaColor ) );

		// nastav� za��tek oblasti ��dky ("nStartX", "pLine" a "pLineStart")
		nStopX = nStartX;
		pLineStart = pLine;
		// zjist�, jedn�-li se o oblast ��dky za "nMaxX"
	} while ( nStartX <= nMaxX );
	// na�el v�echny oblasti na ��dce od "nMinX" do "nMaxX" a p�idal je na z�sobn�k
}


// p�ebarv� oblast "cOldColor" za��naj�c� na sou�adnic�ch "nX" a "nY" barvou "cNewColor"
// z�rove� hled� reprezentanta ( mapcell, kter� m� hodnotu "cRepresentative" )
// d�l� to floodfillem
// vrac� uklazatel na reprezentanta ( nebo NULL )
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

	// aktualizuje barvu p�ebarvovan� oblasti
	m_cRepaintAreaSearchLineAreaColor = cOldColor;

	struct SRepaintAreaBlock sBlock;

	// v�sledek d� sem
	signed char *pRepresentative = NULL, *pRep;

	signed char *pLine = GetAt ( nX, nY );
	ASSERT ( *pLine == cOldColor );

	// nech� prohledat ��dku v okol� bodu
	if ( ( pRep = FindAreaRepresentativeSearchLine ( pLine, nX, nX, nY, cRepresentative ) ) != NULL )
	{
		// Super u� jsme ho na�li
		ASSERT ( pRepresentative == NULL );
		pRepresentative = pRep;
	}

	// zpracuje z�sobn�k
	while ( m_cRepaintAreaBlockStack.Pop ( sBlock ) )
	{
		// zjist� ukazatel na za��tek ��dku
		pLine = GetAt ( sBlock.nMinX, sBlock.nY );
		// zjist�, je-li ��dek ji� zpracov�n
		if ( ( *pLine == cOldColor ) ||  ( ( *pLine == cRepresentative ) && ( *( pLine + 1 ) != cNewColor ) ) )
		{	// ��dek dosud nebyl zpracov�n
			// uschov� si za��tek ��dku
			signed char *pLineStart = pLine - 1;
			// ozna�� ��dek za objeven�
			for ( int i = sBlock.nMaxX - sBlock.nMinX + 1; i-- > 0; pLine++ )
			{
				if ( *pLine == cRepresentative ) continue;

				ASSERT ( *pLine == cOldColor );
				*pLine = cNewColor;
			}
			// prohled� horn� ��dku (v�etn� p�echodu p�es roh)
			if ( ( pRep = FindAreaRepresentativeSearchLine ( pLineStart - m_nMapSizeX, sBlock.nMinX - 1, 
				sBlock.nMaxX + 1, sBlock.nY - 1, cRepresentative ) ) != NULL )
			{
//				ASSERT ( pRepresentative == NULL );
				pRepresentative = pRep;
			}
			// prohled� doln� ��dku (v�etn� p�echodu p�es roh)
			if ( ( pRep = FindAreaRepresentativeSearchLine ( pLineStart + m_nMapSizeX, sBlock.nMinX - 1, 
				sBlock.nMaxX + 1, sBlock.nY + 1, cRepresentative ) ) != NULL )
			{
//				ASSERT ( pRepresentative == NULL );
				pRepresentative = pRep;
			}
		}
		// ��dek byl zpracov�n
	}
	// zpracoval cel� z�sobn�k

	return pRepresentative;
}

// prohled� ��dek "nY" od "nMinX" ("pLine") do "nMaxX" pro p�ekreslov�n� oblasti
signed char * CCFPGMap::FindAreaRepresentativeSearchLine ( signed char *pLine, int nMinX, int nMaxX, int nY, signed char cRepresentative ) 
{
	ASSERT ( pLine == GetAt ( nMinX, nY ) );

	// zjist� barvu p�ebarvovan� oblasti
	signed char cAreaColor = m_cRepaintAreaSearchLineAreaColor;
	// ukazatel na za��tek ��dky
	signed char *pLineStart;

	// tohle je n�vratov� hodnota
	signed char *pRepresentative = NULL;

	// index za��tku oblasti na ��dce
	int nStartX;

	// zjist�, za��n�me-li ji� na oblasti
	if ( ( *pLine != cAreaColor ) && ( *pLine != cRepresentative )  )
	{	// za��n�me mimo oblast
		// najde za��tek oblasti do "nMaxX"
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

		// zjist�, na�el-li za��tek oblasti do "nMaxX"
		if ( nMinX > nMaxX )
		{	// nena�el za��tek oblasti do "nMaxX"
			// ukon�� prohled�v�n� ��dky
			return pRepresentative;
		}

		// nastav� za��tek oblasti ��dky ("nStartX", "pLine" a "pLineStart")
		nStartX = nMinX;
		pLineStart = ++pLine;
	}
	else
	{	// za��n�me na oblasti
		// najde za��tek oblasti na ��dce
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

		// nastav� za��tek oblasti ��dky ("nStartX", "pLine" a "pLineStart")
		nStartX = nMinX - ( pLineStart - pLine );
		pLine = ++pLineStart;
	}
	// "pLine" = "pLineStart" je za "nStartX" o 1 MapCell

	// index konce oblasti na ��dce
	int nStopX = nMinX + 1;
	// najde v�echny oblasti na ��dce
	do
	{
		// "pLine" = "pLineStart" je na "nStopX" >= "nStartX"

		// najde konec oblasti na ��dce
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

		// p�id� oblast ��dky do z�sobn�ku blok� pro objevov�n� oblast�
		m_cRepaintAreaBlockStack.Push ( SRepaintAreaBlock ( nStartX, nStopX, nY ) );

		// za�ne za nalezenou oblast� na ��dce
		nStartX = nStopX + 1;
		// "pLine" je na "nStartX"

		// najde za��tek dal�� oblasti do "nMaxX"
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

		// nastav� za��tek oblasti ��dky ("nStartX", "pLine" a "pLineStart")
		nStopX = nStartX;
		pLineStart = pLine;
		// zjist�, jedn�-li se o oblast ��dky za "nMaxX"
	} while ( nStartX <= nMaxX );
	// na�el v�echny oblasti na ��dce od "nMinX" do "nMaxX" a p�idal je na z�sobn�k

	return pRepresentative;
}


// najde br�ny mostu "cMapCellID" na "pPosition" a vlo�� je do "cBridgeGateList"
void CCFPGMap::GetBridgeGates ( signed char cMapCellID, signed char *pPosition, 
	CBridgeGateList &cBridgeGateList ) 
{
	ASSERT ( cBridgeGateList.IsEmpty () );
	ASSERT ( !IsMapCellAreaID ( cMapCellID ) );
	ASSERT ( *pPosition == cMapCellID );

	// ID ozna�en�ho MapCellu
	signed char cMarkedMapCellID = MARK_EXPLORED ( cMapCellID );
	ASSERT ( cMapCellID != cMarkedMapCellID );

	ASSERT ( m_cMapCellQueue.IsEmpty () );
	ASSERT ( m_cTemporarilyMarkedMapCellStack.IsEmpty () );

	// ozna�� MapCell mostu
	*pPosition = cMarkedMapCellID;

	// p�id� do fronty blok prvn�ho MapCellu
	m_cMapCellQueue.Add ( pPosition );

	// zpracuje frontu
	while ( m_cMapCellQueue.RemoveFirst ( pPosition ) )
	{
		ASSERT ( *pPosition == cMarkedMapCellID );

		// zjist�, je-li v okol� MapCellu v�t�� kruh
		if ( ( *(pPosition - 1) <= cMapCellID ) && 
			( *(pPosition + 1) <= cMapCellID ) && 
			( *(pPosition - m_nMapSizeX - 1) <= cMapCellID ) && 
			( *(pPosition - m_nMapSizeX) <= cMapCellID ) && 
			( *(pPosition - m_nMapSizeX + 1) <= cMapCellID ) && 
			( *(pPosition + m_nMapSizeX - 1) <= cMapCellID ) && 
			( *(pPosition + m_nMapSizeX) <= cMapCellID ) && 
			( *(pPosition + m_nMapSizeX + 1) <= cMapCellID ) )
		{	// v okol� MapCellu nen� v�t�� kruh
			// zpracuje okoln� MapCelly
			for ( int i = 8; i-- > 0; )
			{
				// ukazatel na sousedn� MapCell
				signed char *pMapCell = pPosition + m_aSurroundingMapCellOffset[i];
				// zjist�, jedn�-li se o hledan� MapCell
				if ( *pMapCell == cMapCellID )
				{	// jedn� se o hledan� MapCell
					// ozna�� MapCell
					*pMapCell = cMarkedMapCellID;
					// p�id� do fronty blok MapCellu
					m_cMapCellQueue.Add ( pMapCell );
				}
				// nejedn� se o hledan� MapCell
			}
			// okoln� MapCelly byly zpracov�ny
		}
		else
		{	// v okol� MapCellu je v�t�� kruh
			// do�asn� ozna�� sousedn� MapCelly a ozna�en� vyjme z fronty
			for ( int i = 0; i < 8; i++ )
			{
				// ukazatel na sousedn� MapCell
				signed char *pMapCell = pPosition + m_aSurroundingMapCellOffset[i];
				// zjist�, jedn�-li se o hledan� MapCell
				if ( *pMapCell == cMapCellID )
				{	// jedn� se o hledan� MapCell
					// ozna�� MapCell
					*pMapCell = cMarkedMapCellID;
					// p�id� MapCell do z�sobn�ku do�asn� ozna�en�ch MapCell�
					m_cTemporarilyMarkedMapCellStack.Push ( pMapCell );
				}
				else if ( *pMapCell == cMarkedMapCellID )
				{	// jedn� se o ozna�en� MapCell
					// odebere z fronty blok MapCellu
					(void)m_cMapCellQueue.RemoveElement ( pMapCell );
				}
				else if ( *pMapCell > cMapCellID )
				{	// jedn� se o MapCell v�t��ho kruhu
					ASSERT ( *pMapCell != GetMapCellAreaIDFromMapCellID ( cMapCellID ) );

					// do�asn� ozna�� okoln� MapCelly a ozna�en� vyjme z fronty
					for ( int j = 8; j-- > 0; )
					{
						// ukazatel na ozna�ovan� MapCell
						signed char *pMarkedMapCell = pMapCell + 
							m_aSurroundingMapCellOffset[j];

						// zjit�, jedn�-li se o hledan� MapCell
						if ( *pMarkedMapCell == cMapCellID )
						{	// jedn� se o hledan� MapCell
							// ozna�� MapCell
							*pMarkedMapCell = cMarkedMapCellID;
							// p�id� MapCell do z�sobn�ku do�asn� ozna�en�ch MapCell�
							m_cTemporarilyMarkedMapCellStack.Push ( pMarkedMapCell );
						}
						else if ( *pMarkedMapCell == cMarkedMapCellID )
						{	// jedn� se o ji� ozna�en� MapCell
							// odebere z fronty blok MapCellu
							(void)m_cMapCellQueue.RemoveElement ( pMarkedMapCell );
						}
						// MapCell byl zpracov�n
					}
					// okoln� MapCelly byly zpracov�ny

					// zjist�, jedn�-li se o pokra�ov�n� souvisl� oblasti v�t��ch kruh�
					if ( ( *(pPosition + m_aContinuousMapCellOffset[i][1]) <= cMapCellID ) && 
						( ( m_aContinuousMapCellOffset[i][0] == 0 ) || ( *(pPosition + 
						m_aContinuousMapCellOffset[i][0]) <= cMapCellID ) ) )
					{	// nejedn� se o pokra�ov�n� souvisl� oblasti v�t��ch kruh�
						// ukazatel na nejv�t�� kruh v okol� MapCellu "pMapCell"
						signed char *pMaximumMapCell = pMapCell;

						// najde v souvisl� oblasti okoln�ch MapCell� nejv�t�� kruh
						for ( j = i + 1; ; j++ )
						{
							// zjist� index MapCellu
							j &= 0x07;		// j %= 8
							ASSERT ( i != j );
							// zjist�, je-li MapCell pokra�ov�n�m souvisl� oblasti v�t��ch MapCell�
							if ( ( *(pPosition + m_aContinuousMapCellOffset[j][1]) <= 
								cMapCellID ) && ( ( m_aContinuousMapCellOffset[j][0] == 0 ) || 
								( *(pPosition + m_aContinuousMapCellOffset[j][0]) <= cMapCellID ) ) )
							{	// nejedn� se o pokra�ov�n� souvisl� oblasti v�t��ch kruh�
								// ukon�� hled�n� nejv�t��ho kruhu v souvisl� oblasti okoln�ch 
								//		MapCell�
								break;
							}

							// zjist�, jedn�-li se o nejv�t�� kruh
							if ( *pMaximumMapCell < *(pPosition + m_aSurroundingMapCellOffset[j]) )
							{	// jedn� se o nejv�t�� kruh
								// aktualizuje ukazatel na nejv�t�� kruh
								pMaximumMapCell = pPosition + m_aSurroundingMapCellOffset[j];
							}
						}
						// byl nalezen nejv�t�� kruh v okol� MapCellu

						// nech� naj�t oblast/most MapCellu
						struct SAreaBridge *pAreaBridge = FindAreaBridge ( pMaximumMapCell );

						// vyrob� novou br�nu mostu
						struct SBridgeGate *pBridgeGate = m_cBridgeGatePool.Allocate ();
						// inicializuje nov� vytvo�enou br�nu mostu
						pBridgeGate->pointPosition = GetMapCellPosition ( pMaximumMapCell );
						pBridgeGate->pArea = pAreaBridge;
						pBridgeGate->pBridge = NULL;

						// p�id� vytvo�enou br�nu mostu do seznamu bran mostu
						cBridgeGateList.Add ( pBridgeGate );
					}
					// jedn� se o pokra�ov�n� souvisl� oblasti v�t��ch kruh�
				}
				// jedn� se o MapCell men��ho kruhu
			}
			// okoln� MapCelly byly zpracov�ny
		}
		// okol� MapCellu bylo zpracov�no
	}
	// fronta je zpracov�na

	// odzna�� do�asn� ozna�en� MapCelly
	while ( m_cTemporarilyMarkedMapCellStack.Pop ( pPosition ) )
	{
		ASSERT ( *pPosition == cMarkedMapCellID );
		// odzna�� ozna�en� MapCell
		*pPosition = cMapCellID;
	}
}

// vr�t� oblast/most na pozici "pPosition"
struct CCFPGMap::SAreaBridge *CCFPGMap::FindAreaBridge ( signed char *pPosition ) 
{
	ASSERT ( *pPosition != MARK_EXPLORED ( *pPosition ) );

	ASSERT ( m_cFindAreaBridgeMapCellQueue.IsEmpty () );
	ASSERT ( m_cFindAreaBridgeTemporarilyMarkedMapCellStack.IsEmpty () );

	// zjist�, jedn�-li se o nejv�t�� oblast
	while ( *pPosition < GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ) )
	{	// nejedn� se dosud o nejv�t�� oblast
		// ID hledan�ho MapCellu
		signed char cMapCellID = *pPosition;
		ASSERT ( cMapCellID != MARK_EXPLORED ( cMapCellID ) );
		// ID MapCellu oblasti
		signed char cMapCellAreaID = GetMapCellAreaIDFromMapCellID ( cMapCellID );

		// zjist�, jedn�-li se o MapCell oblasti
		if ( cMapCellID != cMapCellAreaID )
		{	// nejedn� se o MapCell oblasti
			// ID ozna�en�ho MapCellu
			signed char cMarkedMapCellID = MARK_EXPLORED ( cMapCellID );
			// ukazatel na prvn� MapCell
			signed char *pStartPosition = pPosition;

			// ukazatel na v�t�� sousedn� oblast
			signed char *pNeighbourBiggerArea = NULL;
#ifdef _DEBUG
			// p��znak v�ce v�t��ch sousedn�ch oblast�
			BOOL bMultiNeighbourBiggerArea = FALSE;
#endif //_DEBUG

			// ozna�� prvn� MapCell
			*pPosition = cMarkedMapCellID;
			// p�id� do fronty blok prvn�ho MapCellu
			ASSERT ( m_cFindAreaBridgeMapCellQueue.IsEmpty () );
			m_cFindAreaBridgeMapCellQueue.Add ( pPosition );

			// zpracuje frontu
			while ( m_cFindAreaBridgeMapCellQueue.RemoveFirst ( pPosition ) )
			{
				ASSERT ( *pPosition == cMarkedMapCellID );

				// zjist�, je-li v okol� MapCellu v�t�� kruh
				if ( ( *(pPosition - 1) <= cMapCellID ) && 
					( *(pPosition + 1) <= cMapCellID ) && 
					( *(pPosition - m_nMapSizeX - 1) <= cMapCellID ) && 
					( *(pPosition - m_nMapSizeX) <= cMapCellID ) && 
					( *(pPosition - m_nMapSizeX + 1) <= cMapCellID ) && 
					( *(pPosition + m_nMapSizeX - 1) <= cMapCellID ) && 
					( *(pPosition + m_nMapSizeX) <= cMapCellID ) && 
					( *(pPosition + m_nMapSizeX + 1) <= cMapCellID ) )
				{	// v okol� MapCellu nen� v�t�� kruh
					// zpracuje okoln� MapCelly
					for ( int i = 8; i-- > 0; )
					{
						// ukazatel na sousedn� MapCell
						signed char *pMapCell = pPosition + m_aSurroundingMapCellOffset[i];
						// zjist�, jedn�-li se o hledan� MapCell
						if ( *pMapCell == cMapCellID )
						{	// jedn� se o hledan� MapCell
							// ozna�� MapCell
							*pMapCell = cMarkedMapCellID;
							// p�id� do fronty blok MapCellu
							m_cFindAreaBridgeMapCellQueue.Add ( pMapCell );
						}
						// nejedn� se o hledan� MapCell
					}
					// okoln� MapCelly byly zpracov�ny
				}
				else
				{	// v okol� MapCellu je v�t�� kruh
					// zpracuje okoln� MapCelly
					for ( int i = 0; i < 8; i++ )
					{
						// ukazatel na sousedn� MapCell
						signed char *pMapCell = pPosition + m_aSurroundingMapCellOffset[i];
						// zjist�, jedn�-li se o hledan� MapCell
						if ( *pMapCell == cMapCellID )
						{	// jedn� se o hledan� MapCell
							// ozna�� MapCell
							*pMapCell = cMarkedMapCellID;
							// p�id� MapCell do z�sobn�ku do�asn� ozna�en�ch MapCell�
							m_cFindAreaBridgeTemporarilyMarkedMapCellStack.Push ( pMapCell );
						}
						else if ( *pMapCell == cMarkedMapCellID )
						{	// jedn� se o ozna�en� MapCell
							// odebere z fronty blok MapCellu
							(void)m_cFindAreaBridgeMapCellQueue.RemoveElement ( pMapCell );
						}
						else if ( *pMapCell > cMapCellAreaID )
						{	// jedn� se o MapCell v�t��ho kruhu
							ASSERT ( *pMapCell != GetMapCellAreaIDFromMapCellID ( 
								cMapCellID ) );

							// projede okoln� MapCelly
							for ( int j = 8; j-- > 0; )
							{
								// ukazatel na ozna�ovan� MapCell
								signed char *pMarkedMapCell = pMapCell + 
									m_aSurroundingMapCellOffset[j];

								// zjist�, jedn�-li se o hledan� MapCell
								if ( *pMarkedMapCell == cMapCellID )
								{	// jedn� se o hledan� MapCell
									// ozna�� MapCell
									*pMarkedMapCell = cMarkedMapCellID;
									// p�id� MapCell do z�sobn�ku do�asn� ozna�en�ch MapCell�
									m_cFindAreaBridgeTemporarilyMarkedMapCellStack.Push ( 
										pMarkedMapCell );
								}
								else if ( *pMarkedMapCell == cMarkedMapCellID )
								{	// jedn� se o ji� ozna�en� MapCell
									// odebere z fronty blok MapCellu
									(void)m_cFindAreaBridgeMapCellQueue.RemoveElement ( 
										pMarkedMapCell );
								}
								// MapCell byl zpracov�n
							}
							// okoln� MapCelly byly zpracov�ny

							// zjist�, jedn�-li se o prvn� v�t�� sousedn� oblast
							if ( pNeighbourBiggerArea == NULL )
							{	// jedn� se o prvn� v�t�� sousedn� oblast
								// zjist�, jedn�-li se o pokra�ov�n� souvisl� oblasti v�t��ch kruh�
								if ( ( *(pPosition + m_aContinuousMapCellOffset[i][1]) <= 
									cMapCellID ) && ( ( m_aContinuousMapCellOffset[i][0] == 0 ) || 
									( *(pPosition + m_aContinuousMapCellOffset[i][0]) <= 
									cMapCellID ) ) )
								{	// nejedn� se o pokra�ov�n� souvisl� oblasti v�t��ch kruh�
									// ukazatel na nejv�t�� kruh v okol� MapCellu "pMapCell"
									signed char *pMaximumMapCell = pMapCell;

									// najde v souvisl� oblasti okoln�ch MapCell� nejv�t�� kruh
									for ( int j = i + 1; ; j++ )
									{
										// zjist� index MapCellu
										j &= 0x07;		// j %= 8
										ASSERT ( i != j );
										// zjist�, je-li MapCell pokra�ov�n�m souvisl� oblasti 
										//		v�t��ch MapCell�
										if ( ( *(pPosition + m_aContinuousMapCellOffset[j][1]) <= 
											cMapCellID ) && ( ( m_aContinuousMapCellOffset[j][0] == 0 ) 
											|| ( *(pPosition + m_aContinuousMapCellOffset[j][0]) <= 
											cMapCellID ) ) )
										{	// nejedn� se o pokra�ov�n� souvisl� oblasti v�t��ch kruh�
											// ukon�� hled�n� nejv�t��ho kruhu v souvisl� oblasti 
											//		okoln�ch MapCell�
											break;
										}

										// zjist�, jedn�-li se o nejv�t�� kruh
										if ( *pMaximumMapCell < *(pPosition + 
											m_aSurroundingMapCellOffset[j]) )
										{	// jedn� se o nejv�t�� kruh
											// aktualizuje ukazatel na nejv�t�� kruh
											pMaximumMapCell = pPosition + 
												m_aSurroundingMapCellOffset[j];
										}
									}
									// byl nalezen nejv�t�� kruh v okol� MapCellu

									// aktualizuje ukazatel na sousedn� v�t�� oblast
									pNeighbourBiggerArea = pMaximumMapCell;
								}
								// jedn� se o pokra�ov�n� souvisl� oblasti v�t��ch kruh�
							}
							else
							{	// nejedn� se o prvn� sousedn� v�t�� oblast
#ifdef _DEBUG
								// zjist�, jedn�-li se o pokra�ov�n� souvisl� oblasti v�t��ch kruh�
								if ( ( *(pPosition + m_aContinuousMapCellOffset[i][1]) <= 
									cMapCellID ) && ( ( m_aContinuousMapCellOffset[i][0] == 0 ) || 
									( *(pPosition + m_aContinuousMapCellOffset[i][0]) <= 
									cMapCellID ) ) )
								{	// nejedn� se o pokra�ov�n� souvisl� oblasti v�t��ch kruh�
									// p��znak v�ce v�t��ch sousedn�ch oblast�
									bMultiNeighbourBiggerArea = TRUE;
								}
#endif //_DEBUG
							}
							// sousedn� v�t�� oblast byla zpracov�na
						}
						else if ( *pMapCell == cMapCellAreaID )
						{	// jedn� se o MapCell oblasti
							// vypr�zdn� frontu MapCell�
							m_cFindAreaBridgeMapCellQueue.RemoveAll ();
							// aktualizuje ukazatel na MapCell
							pPosition = pMapCell;
							// ukon�� zpracov�v�n� okoln�ch MapCell�
							break;
						}
						// jedn� se o MapCell men��ho kruhu
					}
					// okoln� MapCelly byly zpracov�ny
				}
				// okol� MapCellu bylo zpracov�no
			}
			// fronta je zpracov�na

			// ukazatel na ozna�en� MapCell
			signed char *pMarkedMapCell;
			// odzna�� do�asn� ozna�en� MapCelly
			while ( m_cFindAreaBridgeTemporarilyMarkedMapCellStack.Pop ( pMarkedMapCell ) )
			{
				ASSERT ( *pMarkedMapCell == cMarkedMapCellID );
				// odzna�� ozna�en� MapCell
				*pMarkedMapCell = cMapCellID;
			}

			// odzna�� prvn� MapCell
			ASSERT ( *pStartPosition == cMarkedMapCellID );
			*pStartPosition = cMapCellID;
			// p�id� do fronty blok prvn�ho MapCellu
			m_cFindAreaBridgeMapCellQueue.Add ( pStartPosition );

			// zpracuje frontu odzna�ovan�ch MapCell�
			while ( m_cFindAreaBridgeMapCellQueue.RemoveFirst ( pStartPosition ) )
			{
				ASSERT ( *pStartPosition == cMapCellID );

				// zpracuje okoln� MapCelly
				for ( int i = 8; i-- > 0; )
				{
					// ukazatel na sousedn� MapCell
					signed char *pMapCell = pStartPosition + m_aSurroundingMapCellOffset[i];
					// zjist�, jedn�-li se o ozna�n� MapCell
					if ( *pMapCell == cMarkedMapCellID )
					{	// jedn� se o ozna�en� MapCell
						// odozna�� MapCell
						*pMapCell = cMapCellID;
						// p�id� MapCell do fronty
						m_cFindAreaBridgeMapCellQueue.Add ( pMapCell );
					}
					// nejedn� se o ozna�en� MapCell
				}
				// okoln� MapCelly byly zpracov�ny
			}
			// fronta odzna�ovan�ch MapCell� je zpracov�na

			// zjist�, jedn�-li se o MapCell oblasti
			if ( *pPosition == cMapCellAreaID )
			{	// jedn� se o MapCell oblasti
				// ukon�� hled�n� MapCellu oblasti
				break;
			}
			else
			{	// nejedn� se o MapCell oblasti
				ASSERT ( pNeighbourBiggerArea != NULL );
				ASSERT ( !bMultiNeighbourBiggerArea );
				// nech� naj�t MapCell oblasti v sousedn� v�t�� oblasti
				pPosition = pNeighbourBiggerArea;
			}
		}
		else
		{	// jedn� se o MapCell oblasti
			// ukon�� hled�n� MapCellu oblasti
			break;
		}
	}
	// jedn� se o MapCell oblasti nebo nejv�t��ho kruhu

	// zjist�, jedn�-li se o MapCell nejv�t��ho kruhu
	if ( *pPosition == GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE ) )
	{	// jedn� se o MapCell nejv�t��ho kruhu
		// ID MapCellu nejv�t��ho kruhu
		signed char cMaxMapCellID = GetMapCellIDFromCircleSize ( MAX_CIRCLE_SIZE );
		// zjist� ID MapCellu oblasti nejv�t��ch kruh�
		signed char cMapCellAreaID = GetMapCellAreaIDFromCircleSize ( MAX_CIRCLE_SIZE );

		// index vn�j��ho MapCellu
		int nOutsideMapCellIndex = 0;

		// najde vn�j�� MapCell
		for ( ; nOutsideMapCellIndex < 8; nOutsideMapCellIndex++ )
		{
			// zjist�, jedn�-li se o vn�j�� MapCell
			if ( *(pPosition + m_aSurroundingMapCellOffset[nOutsideMapCellIndex]) < 
				cMaxMapCellID )
			{	// jedn� se o vn�j�� MapCell
				// ukon�� hled�n� vn�j��ho MapCellu
				break;
			}
		}
		// byl nalezen vn�j�� MapCell
		ASSERT ( nOutsideMapCellIndex < 8 );

		ASSERT ( *pPosition == cMaxMapCellID );
		// do�asn� ozna�
		*pPosition = cMapCellAreaID;
		signed char *pStartPosition = pPosition;

		// najde MapCell oblasti nejv�t��ch kruh�
		do
		{
			ASSERT ( ( ( *pPosition == cMaxMapCellID ) && ( *(pPosition + 
				m_aSurroundingMapCellOffset[nOutsideMapCellIndex]) < cMaxMapCellID ) ) ||
				( *pPosition == cMapCellAreaID ) );

			// najde dal�� MapCell oblasti nejv�t��ch kruh�
			while ( *(pPosition + m_aSurroundingMapCellOffset[nOutsideMapCellIndex]) < 
				cMaxMapCellID )
			{
				// nOutsideMapCellIndex = ( nOutsideMapCellIndex + 1 ) % 8
				nOutsideMapCellIndex = ( nOutsideMapCellIndex + 1 ) & 0x07;
			}

			// p�esune se na dal�� MapCell oblasti nejv�t��ch kruh�
			pPosition += m_aSurroundingMapCellOffset[nOutsideMapCellIndex];
			// nOutsideMapCellIndex = ( ( ( nOutsideMapCellIndex + 1 ) / 2 ) * 2 + 5 ) % 8
			nOutsideMapCellIndex = ( ( ( nOutsideMapCellIndex + 1 ) & ~0x01 ) + 5 ) & 0x07;
		} while ( *pPosition != cMapCellAreaID );
		// byl nalezen MapCell oblasti nejv�t��ch kruh�

		// sma�e zna�ku
		*pStartPosition = cMaxMapCellID;

		if ( pPosition == pStartPosition )
		{
			signed char cMarkedMaxMapCellID = MARK_EXPLORED ( cMaxMapCellID );
			// pust� flood fill
			// ten najde representanta a oznackuje celou oblast
			CPoint ptPosition = GetMapCellPosition ( pStartPosition );
			pPosition = FindAreaRepresentative ( cMaxMapCellID, cMarkedMaxMapCellID, ptPosition.x,
				ptPosition.y, cMapCellAreaID );
			ASSERT ( pPosition != NULL );
			// Ted je�t� odozna�it celou oblast
			// Pust�m teda ten floodfill znova, akorat obracen� a zapomenu v�sledek ( m�l by to b� ten sam� )
			VERIFY ( FindAreaRepresentative ( cMarkedMaxMapCellID, cMaxMapCellID, ptPosition.x,
				ptPosition.y, cMapCellAreaID ) == pPosition );
		}
	}
	// byl nalezen MapCell oblasti/mostu

	ASSERT ( *pPosition > 0 );
	ASSERT ( *pPosition <= GetMapCellAreaIDFromCircleSize ( MAX_CIRCLE_SIZE ) );
	ASSERT ( *pPosition == GetMapCellAreaIDFromMapCellID ( *pPosition ) );

	// zjist� sou�adnice MapCellu oblasti/mostu
	int nX = GetMapCellPosition ( pPosition ).x;
	int nY = GetMapCellPosition ( pPosition ).y;

	// pozice v seznamu oblast�/most�
	POSITION posAreaBridge = m_cAreaBridgeList.GetHeadPosition ();
	// ukazatel na oblast/most
	struct SAreaBridge *pAreaBridge;

	// najde z�znam oblasti/mostu "pPosition"
	for ( ; ; )
	{
		VERIFY ( m_cAreaBridgeList.GetNext ( posAreaBridge, pAreaBridge ) );

		// zjist�, jedn�-li se o hledanou oblast/most
		if ( ( pAreaBridge->pointPosition.x == nX ) && 
			( pAreaBridge->pointPosition.y == nY ) )
		{	// jedn� se o hledanou oblast/most
			ASSERT ( pAreaBridge->cMapCellID == GetMapCellIDFromMapCellAreaID ( *pPosition ) );
			// vr�t� ukazatel na nalezenou oblast/most
			return pAreaBridge;
		}
	}
}

// vypln� vzd�lenosti od br�ny mostu "pBridgeGate" k bran�m most� "cBridgeGateList" 
//		oblasti "pAreaBridge" do tabulky vzd�lenost� bran most� "pBridgeGateDistances"
void CCFPGMap::GetBridgeGateDistances ( struct SAreaBridge *pAreaBridge,
	struct SBridgeGate *pBridgeGate, CBridgeGateList &cBridgeGateList, 
	DWORD *pBridgeGateDistances ) 
{
	ASSERT ( !cBridgeGateList.IsEmpty () );
	ASSERT ( m_cPriorityMapCellQueue.IsEmpty () );

	// pozice prvn�ho MapCellu br�ny mostu oblasti
	CPoint pointBridgeGatePosition;
	// vzd�lenost prvn�ho MapCellu br�ny mostu oblasti
	DWORD dwBridgeGateDistance;

	// zjist�, jedn�-li se o prim�rn� br�nu mostu oblasti
	if ( pAreaBridge->cPrimaryBridgeGateList.Find ( pBridgeGate ) != NULL )
	{	// jedn� se o prim�rn� br�nu mostu oblasti
		// inicializuje prvn� MapCell br�ny mostu na br�nu mostu
		pointBridgeGatePosition = pBridgeGate->pointPosition;
		dwBridgeGateDistance = 0;
	}
	else
	{	// jedn� se o sekund�rn� br�nu mostu oblasti
		ASSERT ( pAreaBridge->cSecondaryBridgeGateList.Find ( pBridgeGate ) != NULL );
		// inicializuje prvn� MapCell br�ny mostu na dotyk br�ny mostu
		pointBridgeGatePosition = pBridgeGate->pointTouchPosition;
		dwBridgeGateDistance = pBridgeGate->dwTouchDistance;
	}

	// ID MapCellu oblasti
	signed char cAreaID = pAreaBridge->cMapCellID;
	// ID ozna�en�ho MapCellu oblasti
	signed char cMarkedAreaID = MARK_EXPLORED ( cAreaID );
	ASSERT ( cAreaID != cMarkedAreaID );

	// vyma�e pole vzd�lenost�
	m_cMapCellDistanceArray.Clear ();

	// ukazatel na prvn� MapCell br�ny mostu oblasti
	signed char *pBridgeGateMapCell = GetAt ( pointBridgeGatePosition );
	// zjist�, jedn�-li se o MapCell oblasti
	if ( *pBridgeGateMapCell == cAreaID )
	{	// jedn� se o MapCell oblasti
		// ozna�� prvn� MapCell br�ny mostu oblasti
		*pBridgeGateMapCell = cMarkedAreaID;
	}

	// inicializuje vzd�lenost dotyku v�choz� br�ny mostu
	m_cMapCellDistanceArray.GetAt ( pointBridgeGatePosition ) = 0;
	// p�id� MapCell do prioritn� fronty
	m_cPriorityMapCellQueue.Add ( 0, pBridgeGateMapCell );

	// popis MapCellu v prioritn� front�
	DWORD dwMapCellPriorityQueuePriority;
	signed char *pMapCellPriorityQueueMapCell;

	// zpracuje prioritn� frontu
	while ( m_cPriorityMapCellQueue.RemoveFirst ( dwMapCellPriorityQueuePriority, 
		pMapCellPriorityQueueMapCell ) )
	{
		// projede okoln� MapCelly
		for ( int nIndex = 8; nIndex-- > 0; )
		{
			// ukazatel na sousedn� MapCell
			signed char *pMapCell = pMapCellPriorityQueueMapCell + 
				m_aSurroundingMapCellOffset[nIndex];
			// zjist�, jedn�-li se o MapCell oblasti
			if ( *pMapCell == cAreaID )
			{	// jedn� se o MapCell mostu
				// ozna�� MapCell
				*pMapCell = cMarkedAreaID;
				// zjist� vzd�lenost MapCellu
				DWORD dwDistance = dwMapCellPriorityQueuePriority + ( ( nIndex & 0x01 ) ? 10 : 
					14 );
				// p�id� MapCell do prioritn� fronty
				m_cPriorityMapCellQueue.Add ( dwDistance, pMapCell );
				// zjist� pozici MapCellu
				CPoint pointPosition = GetMapCellPosition ( pMapCell );
				// zap�e vzd�lenost MapCellu do pole
				m_cMapCellDistanceArray.GetAt ( pointPosition ) = dwDistance;
			}
			// nejedn� se o MapCell oblasti
		}
		// zpracoval okoln� MapCelly
	}
	// zpracoval prioritn� frontu

	ASSERT ( m_cTemporarilyMarkedMapCellStack.IsEmpty () );

	// ozna�en� MapCell
	signed char *pMapCell;
	// p��d� prvn� MapCell br�ny mostu do z�sobn�ku
	m_cTemporarilyMarkedMapCellStack.Push ( pBridgeGateMapCell );
	// zjist�, jedn�-li se o ozna�en� MapCell oblasti
	if ( *pBridgeGateMapCell == cMarkedAreaID )
	{	// jedn� se o MapCell oblasti
		// ozna�� prvn� MapCell br�ny mostu oblasti
		*pBridgeGateMapCell = cAreaID;
	}
	ASSERT ( *pBridgeGateMapCell == GetMapCellIDFromMapCellAreaID ( 
		*pBridgeGateMapCell ) );
	// odzna�� do�asn� ozna�en� MapCelly
	while ( m_cTemporarilyMarkedMapCellStack.Pop ( pMapCell ) )
	{
		// projede okoln� MapCelly
		for ( int nIndex = 8; nIndex-- > 0; )
		{
			// ukazatel na sousedn� MapCell
			signed char *pSurroundingMapCell = pMapCell + 
				m_aSurroundingMapCellOffset[nIndex];
			// zjist�, jedn�-li se o ozna�en� MapCell
			if ( *pSurroundingMapCell == cMarkedAreaID )
			{	// jedn� se o ozna�en� MapCell
				// odzna�� do�asn� ozna�en� MapCell
				*pSurroundingMapCell = cAreaID;
				// p�id� MapCell do z�sobn�ku
				m_cTemporarilyMarkedMapCellStack.Push ( pSurroundingMapCell );
			}
			// nejedn� se o ozna�en� MapCell
		}
		// zpracoval okoln� MapCelly
	}
	// odzna�il do�asn� ozna�en� MapCelly

	// pozice dal�� br�ny mostu v seznamu bran most� oblasti
	POSITION posNextBridgeGate = cBridgeGateList.GetHeadPosition ();
	// dal�� br�na mostu oblasti
	struct SBridgeGate *pNextBridgeGate;
	// aktualizuje vzd�lenosti bran most�
	while ( CBridgeGateList::GetNext ( posNextBridgeGate, pNextBridgeGate ) )
	{
		// pozice prvn�ho MapCellu br�ny mostu oblasti
		CPoint pointNextBridgeGatePosition;
		// vzd�lenost prvn�ho MapCellu br�ny mostu oblasti
		DWORD dwNextBridgeGateDistance;

		// zjist�, jedn�-li se o prim�rn� br�nu mostu oblasti
		if ( pAreaBridge->cPrimaryBridgeGateList.Find ( pNextBridgeGate ) != NULL )
		{	// jedn� se o prim�rn� br�nu mostu oblasti
			// inicializuje prvn� MapCell br�ny mostu na br�nu mostu
			pointNextBridgeGatePosition = pNextBridgeGate->pointPosition;
			dwNextBridgeGateDistance = 0;
		}
		else
		{	// jedn� se o sekund�rn� br�nu mostu oblasti
			ASSERT ( pAreaBridge->cSecondaryBridgeGateList.Find ( pNextBridgeGate ) != NULL );
			// inicializuje prvn� MapCell br�ny mostu na dotyk br�ny mostu
			pointNextBridgeGatePosition = pNextBridgeGate->pointTouchPosition;
			dwNextBridgeGateDistance = pNextBridgeGate->dwTouchDistance;
		}

		// ukazatel na prvn� MapCell br�ny mostu oblasti
		signed char *pNextBridgeGateMapCell = GetAt ( pointNextBridgeGatePosition );

		// vzd�lnost dotyku br�ny
		DWORD &rdwNextBridgeGateTouchDistance = m_cMapCellDistanceArray.GetAt ( 
			pointNextBridgeGatePosition );
		// zjist�, je-li dotyk br�ny mostu dosa�iteln� p��mo
		if ( rdwNextBridgeGateTouchDistance == INFINITE_DISTANCE )
		{	// dotyk br�ny nen� dosa�iteln� p��mo
			// zjist� vzd�lenost dotyku br�ny mostu
			for ( int nIndex = 8; nIndex--; )
			{
				// ukazatel na sousedn� MapCell
				signed char *pSurroundingMapCell = pNextBridgeGateMapCell + 
					m_aSurroundingMapCellOffset[nIndex];
				// spo��t� vzd�lenost p�es sousedn� MapCell
				DWORD dwSurroundingMapCellDistance = m_cMapCellDistanceArray.GetAt ( 
					GetMapCellPosition ( pSurroundingMapCell ) ) + ( ( nIndex & 0x01 ) ? 10 : 
					14 );
				// zjist�, jedn�-li se o krat�� vzd�lenost
				if ( dwSurroundingMapCellDistance < rdwNextBridgeGateTouchDistance )
				{	// jedn� se o krat�� vzd�lenost
					// aktualizuje vzd�lenost dotyku br�ny mostu
					rdwNextBridgeGateTouchDistance = dwSurroundingMapCellDistance;
				}
			}
			// zjistil vzd�lenost dotyku br�ny mostu
			ASSERT ( rdwNextBridgeGateTouchDistance < INFINITE_DISTANCE );
		}
		// dotyk br�ny je dosa�iteln� p��mo

		// aktualizuje vzd�lenost bran most�
		*(pBridgeGateDistances++) = ( rdwNextBridgeGateTouchDistance + 
			dwNextBridgeGateDistance + dwBridgeGateDistance + 5 ) / 10;
		ASSERT ( *(pBridgeGateDistances - 1) < INFINITE_DISTANCE );

		// koncov� zpracov�van� bod  cesty
		CPoint pointEnd = pointNextBridgeGatePosition;
		// posledn� zpracovan� bod cesty
		CPoint pointLastPosition = pointNextBridgeGatePosition;
		// vzd�lenost posledn�ho zpracovan�ho bodu cesty
		DWORD dwLastPositionDistance = m_cMapCellDistanceArray.GetAt ( 
			pointNextBridgeGatePosition );

		// fronta pozic kontroln�ch bod� hintu cesty
		static CSelfPooledList<CPoint> cCheckPointPositionList ( 100 );
		ASSERT ( cCheckPointPositionList.IsEmpty () );

		// p�evede cestu na kontroln� body
		while ( dwLastPositionDistance != 0 )
		{
			// dal�� bod cesty
			CPoint pointNextPosition;
			// vzd�lenost dal��ho bodu cesty
			DWORD dwNextPositionDistance;

			// najde dal�� bod cesty
			for ( int nIndex = 8; nIndex-- > 0; )
			{
				// zjist� sousedn� pozici na map�
				pointNextPosition = pointLastPosition + m_aSurroundingPositionOffset[nIndex];

				// vzd�lenost sousedn� pozice na map�
				dwNextPositionDistance = m_cMapCellDistanceArray.GetAt ( pointNextPosition );

				// zjist�, jedn�-li se o prozkoumanou pozici na map�
				if ( dwNextPositionDistance == INFINITE_DISTANCE )
				{	// jedn� se o neprozkoumanou pozici na map�
					// ukon�� zpracov�v�n� sousedn� pozice
					continue;
				}
				ASSERT ( dwNextPositionDistance < INFINITE_DISTANCE );

				ASSERT ( ( dwNextPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) >= 
					dwLastPositionDistance );

				// zjist�, jedn�-li se o n�sleduj�c� bod cesty
				if ( ( dwNextPositionDistance + ( ( nIndex & 0x01 ) ? 10 : 14 ) ) == 
					dwLastPositionDistance )
				{	// jedn� se o n�sleduj�c� bod cesty
					// nech� zpracovat bod cesty
					break;
				}
			}
			// byl nalezen dal�� bod cesty
			ASSERT ( nIndex >= 0 );

		// zjist�, existuje-li �se�ka mezi nelezen�m dal��m bodem a koncem cesty

			// vzd�lenost konc� �se�ky v jednotliv�ch os�ch
			CSize sizeDistance ( abs ( pointEnd.x - pointNextPosition.x ), 
				abs ( pointEnd.y - pointNextPosition.y ) );

			// zjist�, je-li �se�ka p��li� dlouh�
			if ( ( sizeDistance.cx * sizeDistance.cx + sizeDistance.cy * sizeDistance.cy ) >= 
				MAX_CHECK_POINT_DISTANCE * MAX_CHECK_POINT_DISTANCE )
			{	// �se�ka je p��li� dlouh�
				// p�id� kontroln� bod koncov�ho bodu cesty
				cCheckPointPositionList.Add ( pointLastPosition );
				// aktualizuje konec cesty
				pointEnd = pointLastPosition;
				// aktualizuje vzd�lenost bod� �se�ky v jednotliv�ch os�ch
				sizeDistance = CSize ( abs ( pointEnd.x - pointNextPosition.x ), 
				abs ( pointEnd.y - pointNextPosition.y ) );
			}

			// zjist�, ve kter�m rozm�ru je vzd�lenost konc� �se�ky v�t��
			if ( sizeDistance.cy < sizeDistance.cx )
			{	// vzd�lenost konc� �se�ky je v�t�� v ose X
				// pozice bodu �se�ky
				CPoint pointPosition;

				// pozice bodu �se�ky v ose X
				pointPosition.x = pointNextPosition.x;
				// p��r�stek v ose X
				int nXDifference = ( pointNextPosition.x < pointEnd.x ) ? 1 : -1;

				// p�esn� pozice bodu �se�ky v ose Y
				double dbYPosition = (double)pointNextPosition.y + 0.5;
				// p�esn� p��r�stek v ose Y
				double dbYDifference = ( (double)pointEnd.y - (double)pointNextPosition.y ) / 
					(double)sizeDistance.cx;

				ASSERT ( pointPosition.x != pointEnd.x );

				// "nulty" krok iterace, osa X je implicitne posunuta o "pul mapcellu",
				// na ose y pridame polovinu dbYDifference k prvni hranici mapcellu
				pointPosition.x += nXDifference;
				dbYPosition += dbYDifference / 2;
				// pozice bodu �se�ky v ose Y
				pointPosition.y = (int)dbYPosition;

				// zkontroluje MapCelly bod� �se�ky
				while ( pointPosition.x != pointEnd.x )
				{
				// zkontroluje prvn� okraj MapCellu �se�ky

					// vzd�lenost bodu �se�ky
					DWORD dwPositionDistance = m_cMapCellDistanceArray.GetAt ( pointPosition );

					// zjist�, je-li MapCell bodu �se�ky povolen�
					if ( dwPositionDistance == INFINITE_DISTANCE )
					{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
						// p�id� kontroln� bod koncov�ho bodu cesty
						cCheckPointPositionList.Add ( pointLastPosition );
						// aktualizuje konec cesty
						pointEnd = pointLastPosition;
						// ukon�� kontrolu MapCell� bod� �se�ky
						break;
					}
					// MapCell bodu �se�ky je povolen�
					ASSERT ( dwPositionDistance < INFINITE_DISTANCE );

				// posune pozici na druh� okraj MapCellu

					// posune pozici na druh� okraj MapCellu
					dbYPosition += dbYDifference;
					// pozice bodu �se�ky v ose Y
					pointPosition.y = (int)dbYPosition;

				// zkontroluje druh� okraj MapCellu �se�ky

					// aktualizuje vzd�lenost bodu �se�ky
					dwPositionDistance = m_cMapCellDistanceArray.GetAt ( pointPosition );

					// zjist�, je-li MapCell bodu �se�ky povolen�
					if ( dwPositionDistance == INFINITE_DISTANCE )
					{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
						// p�id� kontroln� bod koncov�ho bodu cesty
						cCheckPointPositionList.Add ( pointLastPosition );
						// aktualizuje konec cesty
						pointEnd = pointLastPosition;
						// ukon�� kontrolu MapCell� bod� �se�ky
						break;
					}
					// MapCell bodu �se�ky je povolen�
					ASSERT ( dwPositionDistance < INFINITE_DISTANCE );

				// posune se na dal�� bod �se�ky

					// posune se na dal�� bod �se�ky
					pointPosition.x += nXDifference;
				}
				// MapCelly bod� �se�ky jsou zkontrolov�ny
			}
			else if ( sizeDistance.cy > sizeDistance.cx )
			{	// vzd�lenost konc� �se�ky je v�t�� v ose Y
				// pozice bodu �se�ky
				CPoint pointPosition;

				// pozice bodu �se�ky v ose Y
				pointPosition.y = pointNextPosition.y;
				// p��r�stek v ose Y
				int nYDifference = ( pointNextPosition.y < pointEnd.y ) ? 1 : -1;

				// p�esn� pozice v ose X
				double dbXPosition = (double)pointNextPosition.x + 0.5;
				// p�esn� p��r�stek v ose X
				double dbXDifference = ( (double)pointEnd.x - (double)pointNextPosition.x ) / 
					(double)sizeDistance.cy;

				ASSERT ( pointPosition.y != pointEnd.y );

				// "nulty" krok iterace, osa Y je implicitne posunuta o "pul mapcellu",
				// na ose X pridame polovinu dbXDifference k prvni hranici mapcellu
				pointPosition.y += nYDifference;
				dbXPosition += dbXDifference / 2;
				// pozice bodu �se�ky v ose X
				pointPosition.x = (int)dbXPosition;

				// zkontroluje MapCelly bod� �se�ky
				while ( pointPosition.y != pointEnd.y )
				{
				// zkontroluje prvn� okraj MapCellu �se�ky

					// vzd�lenost bodu �se�ky
					DWORD dwPositionDistance = m_cMapCellDistanceArray.GetAt ( 
						pointPosition );

					// zjist�, je-li MapCell bodu �se�ky povolen�
					if ( dwPositionDistance == INFINITE_DISTANCE )
					{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
						// p�id� kontroln� bod koncov�ho bodu cesty
						cCheckPointPositionList.Add ( pointLastPosition );
						// aktualizuje konec cesty
						pointEnd = pointLastPosition;
						// ukon�� kontrolu MapCell� bod� �se�ky
						break;
					}
					// MapCell bodu �se�ky je povolen�
					ASSERT ( dwPositionDistance < INFINITE_DISTANCE );

				// posune pozici na druh� okraj MapCellu

					// posune pozici na druh� okraj MapCellu
					dbXPosition += dbXDifference;
					// pozice bodu �se�ky v ose X
					pointPosition.x = (int)dbXPosition;

				// zkontroluje druh� okraj MapCellu �se�ky
					// aktualizuje vzd�lenost bodu �se�ky
					dwPositionDistance = m_cMapCellDistanceArray.GetAt ( 
						pointPosition );

					// zjist�, je-li MapCell bodu �se�ky povolen�
					if ( dwPositionDistance == INFINITE_DISTANCE )
					{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
						// p�id� kontroln� bod koncov�ho bodu cesty
						cCheckPointPositionList.Add ( pointLastPosition );
						// aktualizuje konec cesty
						pointEnd = pointLastPosition;
						// ukon�� kontrolu MapCell� bod� �se�ky
						break;
					}
					// MapCell bodu �se�ky je povolen�
					ASSERT ( dwPositionDistance < INFINITE_DISTANCE );

				// posune se na dal�� bod �se�ky

					// posune se na dal�� bod �se�ky
					pointPosition.y += nYDifference;
				}
				// MapCelly bod� �se�ky jsou zkontrolov�ny
			}
			else
			{	// vzd�lenosti konc� �se�ky jsou stejn� v ose X i Y
				// pozice bodu �se�ky
				CPoint pointPosition = pointNextPosition;
				// p��r�stek v os�ch X a Y
				CPoint pointDifference ( ( pointNextPosition.x < pointEnd.x ) ? 1 : -1, 
					( pointNextPosition.y < pointEnd.y ) ? 1 : -1 );

				ASSERT ( pointNextPosition != pointEnd );

				// posune se na dal�� MapCell
				pointPosition += pointDifference;

				// zkontroluje MapCelly bod� �se�ky
				while ( pointPosition != pointEnd )
				{
					// vzd�lenost bodu �se�ky
					DWORD dwPositionDistance = m_cMapCellDistanceArray.GetAt ( pointPosition );

					// zjist�, je-li MapCell bodu �se�ky povolen�
					if ( dwPositionDistance == INFINITE_DISTANCE )
					{	// MapCell bodu �se�ky je zak�z�n� nebo dosud nenav�t�ven�
						// p�id� kontroln� bod koncov�ho bodu cesty
						cCheckPointPositionList.Add ( pointLastPosition );
						// aktualizuje konec cesty
						pointEnd = pointLastPosition;
						// ukon�� kontrolu MapCell� bod� �se�ky
						break;
					}
					// MapCell bodu �se�ky je povolen�
					ASSERT ( dwPositionDistance < INFINITE_DISTANCE );

					// posune se na dal�� MapCell
					pointPosition += pointDifference;
				}
				// MapCelly bod� �se�ky jsou zkontrolov�ny
			}
			// �se�ka je zpracov�na

			// aktualizuje posledn� zpracovan� bod cesty
			pointLastPosition = pointNextPosition;
			// aktualizuje vzd�lenost posledn�ho zpracovan�ho bodu cesty
			dwLastPositionDistance = dwNextPositionDistance;
		}
		// cesta byla p�evedena na kontroln� body

		// zjist� po�et kontroln�ch bod� hintu cesty
		DWORD dwCheckPointCount = cCheckPointPositionList.GetSize ();

		// zjist�, je-li seznam kontroln�ch bod� hintu cesty pr�zdn�
		if ( dwCheckPointCount > 0 )
		{	// seznam kontroln�ch bod� cesty nen� pr�zdn�
			// uschov� index nov�ho hintu cesty
			DWORD dwPathHintIndex = m_dwPathHintTableSize;

			// aktualizuje hinty cesty mezi branami most�
			*(pBridgeGateDistances++) = dwPathHintIndex;

			// zv�t�� pole hint� cest
			IncreasePathHintTable ( 2 * dwCheckPointCount + 1 );
			// vypln� po�et kontroln�ch bod� hintu cesty
			m_pPathHintTable[dwPathHintIndex++] = dwCheckPointCount;

			// kontroln� bod hintu cesty
			CPoint pointCheckPoint;

			// vypln� kontroln� body hintu cesty
			while ( cCheckPointPositionList.RemoveFirst ( pointCheckPoint ) )
			{
				// p�elo�� sou�adnice kotnroln�ho bodu hintu cesty
				ASSERT ( pointCheckPoint.x >= MAP_BORDER );
				pointCheckPoint.x -= MAP_BORDER;
				ASSERT ( pointCheckPoint.x < m_nSizeX );
				ASSERT ( pointCheckPoint.y >= MAP_BORDER );
				pointCheckPoint.y -= MAP_BORDER;
				ASSERT ( pointCheckPoint.y < m_nSizeY );
				// ulo�� kontroln� bod hintu cesty do tabulky hint� cesty
				m_pPathHintTable[dwPathHintIndex++] = (DWORD)pointCheckPoint.x;
				m_pPathHintTable[dwPathHintIndex++] = (DWORD)pointCheckPoint.y;
			}
			ASSERT ( dwPathHintIndex == m_dwPathHintTableSize );
		}
		else
		{	// seznam kontroln�ch bod� cesty je pr�zdn�
			// aktualizuje hinty cesty mezi branami most�
			*(pBridgeGateDistances++) = INVALID_PATH_HINT;
		}

		TRACE_FIND_PATH_CREATION ( "GetAreaDistance %d ... (%d,%d)[%d,%d]->[%d,%d](%d,%d)\n", 
			*(pBridgeGateDistances - 2), pBridgeGate->pointPosition.x, 
			pBridgeGate->pointPosition.y, pointBridgeGatePosition.x, 
			pointBridgeGatePosition.y, pointNextBridgeGatePosition.x, 
			pointNextBridgeGatePosition.y, pNextBridgeGate->pointPosition.x, 
			pNextBridgeGate->pointPosition.y );
	}

	// ukon�� zji��ov�n� vzd�lenost� bran most� oblasti
	return;
}

// najde dotyk br�ny mostu
void CCFPGMap::FindBridgeGateTouch ( struct SBridgeGate *pBridgeGate, 
	struct SAreaBridge *pAreaBridge ) 
{
	// ukazatel na prvn� MapCell br�ny mostu
	signed char *pPosition = GetAt ( pBridgeGate->pointPosition );
	ASSERT ( *pPosition != MARK_EXPLORED ( *pPosition ) );

	ASSERT ( m_cPriorityMapCellQueue.IsEmpty () );
	ASSERT ( m_cTemporarilyMarkedMapCellStack.IsEmpty () );

	// ID hledan�ho MapCellu
	signed char cMapCellID = *pPosition;
	ASSERT ( cMapCellID != MARK_EXPLORED ( cMapCellID ) );

	// ID MapCellu hledan� oblasti
	signed char cAreaID = pAreaBridge->cMapCellID;
	ASSERT ( cAreaID != MARK_EXPLORED ( cAreaID ) );
	ASSERT ( cAreaID != GetMapCellAreaIDFromMapCellID ( cAreaID ) );

	// zjist�, jedn�-li se o MapCell hledan� oblasti
	if ( cMapCellID < cAreaID )
	{	// nejedn� se o MapCell hledan� oblasti
		// ID ozna�en�ho MapCellu
		signed char cMarkedMapCellID = MARK_EXPLORED ( cMapCellID );

		// ozna�� prvn� MapCell
		*pPosition = cMarkedMapCellID;
		// p�id� prvn� MapCell do prioritn� fronty
		m_cPriorityMapCellQueue.Add ( 0, pPosition );

		// priorita MapCellu v prioritn� front�
		DWORD dwPriority;

		// vyjme z prioritn� fronty MapCell
		while ( m_cPriorityMapCellQueue.RemoveFirst ( dwPriority, pPosition ) )
		{
			ASSERT ( *pPosition == cMarkedMapCellID );

			// zjist�, je-li v okol� MapCellu v�t�� kruh
			if ( ( *(pPosition - 1) <= cMapCellID ) && 
				( *(pPosition + 1) <= cMapCellID ) && 
				( *(pPosition - m_nMapSizeX - 1) <= cMapCellID ) && 
				( *(pPosition - m_nMapSizeX) <= cMapCellID ) && 
				( *(pPosition - m_nMapSizeX + 1) <= cMapCellID ) && 
				( *(pPosition + m_nMapSizeX - 1) <= cMapCellID ) && 
				( *(pPosition + m_nMapSizeX) <= cMapCellID ) && 
				( *(pPosition + m_nMapSizeX + 1) <= cMapCellID ) )
			{	// v okol� MapCellu nen� v�t�� kruh
				// zpracuje okoln� MapCelly
				for ( int nIndex = 8; nIndex-- > 0; )
				{
					// ukazatel na sousedn� MapCell
					signed char *pSurroundingMapCell = pPosition + 
						m_aSurroundingMapCellOffset[nIndex];

					// zjist�, jedn�-li se o hledan� MapCell
					if ( *pSurroundingMapCell == cMapCellID )
					{	// jedn� se o hledan� MapCell
						// ozna�� MapCell
						*pSurroundingMapCell = cMarkedMapCellID;
						// p�id� MapCell do prioritn� fronty
						m_cPriorityMapCellQueue.Add ( dwPriority + 
							( ( nIndex & 0x01 ) ? 10 : 14 ), pSurroundingMapCell );
					}
					// nejedn� se o hledan� MapCell
				}
				// okoln� MapCelly byly zpracov�ny
			}
			else
			{	// v okol� MapCellu je v�t�� kruh
				// ukazatel na nejv�t�� kruh v okol� MapCellu
				signed char *pMaximumMapCell;
				// priorita nejv�t��ho kruhu v okol� MapCellu
				DWORD dwMaximumMapCellPriority;
#ifdef _DEBUG
				// p��znak nalezen� nejv�t��ho kruhu v okol� MapCellu
				BOOL bMaximumMapCellFound = FALSE;
#endif //_DEBUG

				// zpracuje okoln� MapCelly
				for ( int i = 0; i < 8; i++ )
				{
					// ukazatel na sousedn� MapCell
					signed char *pMapCell = pPosition + m_aSurroundingMapCellOffset[i];
					// zjist�, jedn�-li se o v�t�� MapCell
					if ( *pMapCell > cMapCellID )
					{	// jedn� se o MapCell v�t��ho kruhu
						// zjist�, jedn�-li se o pokra�ov�n� souvisl� oblasti v�t��ch kruh�
						if ( ( *(pPosition + m_aContinuousMapCellOffset[i][1]) <= 
							cMapCellID ) && ( ( m_aContinuousMapCellOffset[i][0] == 0 ) || 
							( *(pPosition + m_aContinuousMapCellOffset[i][0]) <= 
							cMapCellID ) ) )
						{	// nejedn� se o pokra�ov�n� souvisl� oblasti v�t��ch kruh�
							ASSERT ( !bMaximumMapCellFound );

							// inicializuje ukazatel na nejv�t�� kruh v okol� MapCellu "pMapCell"
							pMaximumMapCell = pMapCell;
							// inicializuje prioritu nejv�t��ho kruhu v okol� MapCellu "pMapCell"
							dwMaximumMapCellPriority = dwPriority;

							// najde v souvisl� oblasti okoln�ch MapCell� nejv�t�� kruh
							for ( int j = i + 1; ; j++ )
							{
								// zjist� index MapCellu
								j &= 0x07;		// j %= 8
								ASSERT ( i != j );
								// zjist�, je-li MapCell pokra�ov�n�m souvisl� oblasti 
								//		v�t��ch MapCell�
								if ( ( *(pPosition + m_aContinuousMapCellOffset[j][1]) <= 
									cMapCellID ) && ( ( m_aContinuousMapCellOffset[j][0] == 0 ) 
									|| ( *(pPosition + m_aContinuousMapCellOffset[j][0]) <= 
									cMapCellID ) ) )
								{	// nejedn� se o pokra�ov�n� souvisl� oblasti v�t��ch kruh�
									// ukon�� hled�n� nejv�t��ho kruhu v souvisl� oblasti 
									//		okoln�ch MapCell�
									break;
								}

								// zjist�, jedn�-li se o nejv�t�� kruh
								if ( *pMaximumMapCell < *(pPosition + 
									m_aSurroundingMapCellOffset[j]) )
								{	// jedn� se o nejv�t�� kruh
									// aktualizuje ukazatel na nejv�t�� kruh
									pMaximumMapCell = pPosition + 
										m_aSurroundingMapCellOffset[j];
									// aktualizuje prioritu nejv�t��ho kruhu
									dwMaximumMapCellPriority = dwPriority + ( ( j & 0x01 ) ? 10 : 
										14 );
								}
								else
								{	// nejedn� se o nejv�t�� kruh
									ASSERT ( ( *(pPosition + m_aSurroundingMapCellOffset[j]) == 
										MARK_EXPLORED ( *(pPosition + 
										m_aSurroundingMapCellOffset[j]) ) ) || !IsMapCellAreaID ( 
										*(pPosition + m_aSurroundingMapCellOffset[j]) ) );
								}
							}
							// byl nalezen nejv�t�� kruh v okol� MapCellu

#ifdef _DEBUG
							// nastav� p��znak nalezen� nejv�t��ho kruhu v okol� MapCellu
							bMaximumMapCellFound = TRUE;
#else //_DEBUG
							// ukon�� zpracov�v�n� okoln�ch MapCell�
							break;
#endif //!_DEBUG
						}
						// jedn� se o pokra�ov�n� souvisl� oblasti v�t��ch kruh�
					}
					// nejedn� se o v�t�� MapCell
				}
				// okoln� MapCelly byly zpracov�ny

				ASSERT ( bMaximumMapCellFound );

				// sma�e prioritn� frontu MapCell�
				m_cPriorityMapCellQueue.RemoveAll ();

				// zjist�, jedn�-li se o MapCell hledan� oblasti
				if ( *pMaximumMapCell >= cAreaID )
				{	// jedn� se o MapCell hledan� oblasti
					ASSERT ( GetMapCellIDFromMapCellAreaID ( *pMaximumMapCell ) == cAreaID );

					// aktualizuje dotyk br�ny mostu
					pBridgeGate->pointTouchPosition = GetMapCellPosition ( pMaximumMapCell );
					pBridgeGate->dwTouchDistance = dwMaximumMapCellPriority;

					// ukazatel na prvn� do�asn� ozna�en� MapCell
					pPosition = GetAt ( pBridgeGate->pointPosition );
					// odzna�� prvn� do�asn� ozna�en� MapCell
					ASSERT ( *pPosition == MARK_EXPLORED ( *pPosition ) );
					*pPosition = UNMARK_EXPLORED ( *pPosition );
					// p��d� prvn� MapCell br�ny mostu do z�sobn�ku
					m_cTemporarilyMarkedMapCellStack.Push ( pPosition );
					// odzna�� do�asn� ozna�en� MapCelly
					while ( m_cTemporarilyMarkedMapCellStack.Pop ( pPosition ) )
					{
						ASSERT ( *pPosition == UNMARK_EXPLORED ( *pPosition ) );
	
						// projede okoln� MapCelly
						for ( int nIndex = 8; nIndex-- > 0; )
						{
							// ukazatel na sousedn� MapCell
							signed char *pSurroundingMapCell = pPosition + 
								m_aSurroundingMapCellOffset[nIndex];
							// zjist�, jedn�-li se o ozna�en� MapCell
							if ( *pSurroundingMapCell == MARK_EXPLORED ( *pSurroundingMapCell ) )
							{	// jedn� se o ozna�en� MapCell
								// odzna�� do�asn� ozna�en� MapCell
								*pSurroundingMapCell = UNMARK_EXPLORED ( *pSurroundingMapCell );
								// p�id� MapCell do z�sobn�ku MapCell�
								m_cTemporarilyMarkedMapCellStack.Push ( pSurroundingMapCell );
							}
							// nejedn� se o ozna�en� MapCell
						}
						// zpracoval okoln� MapCelly
					}
					// odzna�il do�asn� ozna�en� MapCelly

					// ukon�� hled�n� dotyku br�ny mostu
					return;
				}
				// nejedn� se o MapCell hledan� oblasti

				// aktualizuje ID hledan�ho MapCellu
				cMapCellID = *pMaximumMapCell;
				ASSERT ( cMapCellID != MARK_EXPLORED ( cMapCellID ) );
				ASSERT ( cMapCellID == GetMapCellIDFromMapCellAreaID ( cMapCellID ) );

				// aktualizuje ID ozna�en�ho MapCellu
				cMarkedMapCellID = MARK_EXPLORED ( cMapCellID );

				// ozna�� nejv�t�� okoln� MapCell
				*pMaximumMapCell = cMarkedMapCellID;
				// p�id� nejv�t�� okoln� MapCell do prioritn� fronty
				m_cPriorityMapCellQueue.Add ( dwMaximumMapCellPriority, pMaximumMapCell );
			}
			// okol� MapCellu bylo zpracov�no
		}
		// fronta je zpracov�na - nemo�n� p��pad
		ASSERT ( FALSE );
	}
	else
	{	// jedn� se o MapCell hledan� oblasti
		ASSERT ( GetMapCellIDFromMapCellAreaID ( cMapCellID ) == cAreaID );

		// aktualizuje dotyk br�ny mostu
		pBridgeGate->pointTouchPosition = pBridgeGate->pointPosition;
		pBridgeGate->dwTouchDistance = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// Informace o map� t��dy CCFPGMap
//////////////////////////////////////////////////////////////////////

// vr�t� ukazatel na MapCell na sou�adnic�ch "nX", "nY" v sou�adnic�ch mapy s okraji
inline signed char *CCFPGMap::GetAt ( int nX, int nY ) 
{
	ASSERT ( ( nX >= 0 ) && ( nX < m_nMapSizeX ) );
	ASSERT ( ( nY >= 0 ) && ( nY < m_nMapSizeY ) );

	return &m_pMap[nY * m_nMapSizeX + nX];
}

// vr�t� pozici MapCellu "pMapCell" na map� v sou�adnic�ch mapy s okraji
inline CPoint CCFPGMap::GetMapCellPosition ( signed char *pMapCell ) 
{
	return CPoint ( ( pMapCell - m_pMap ) % m_nMapSizeX, ( pMapCell - m_pMap ) / 
		m_nMapSizeX );
}

//////////////////////////////////////////////////////////////////////
// Pomocn� operace
//////////////////////////////////////////////////////////////////////

// vr�t� ID MapCellu s kruhem velikosti "nCircleSize"
inline signed char CCFPGMap::GetMapCellIDFromCircleSize ( int nCircleSize ) 
{
	ASSERT ( ( nCircleSize <= MAX_CIRCLE_SIZE ) && ( nCircleSize >= 1 ) );

	// vr�t� ID MapCellu s kruhem velikosti "nCircleSize"
	return 2 * nCircleSize;
}

// vr�t� ID MapCellu MapCellu oblasti "cMapCellAreaID"
inline signed char CCFPGMap::GetMapCellIDFromMapCellAreaID ( signed char cMapCellAreaID ) 
{
	ASSERT ( ( cMapCellAreaID >= 0 ) && ( cMapCellAreaID <= ( 2 * MAX_CIRCLE_SIZE + 1 ) ) );

	// vr�t� ID MapCellu
	return ( cMapCellAreaID & ~0x01 );
}

// vr�t� ID MapCellu oblasti s kruhem velikosti "nCircleSize"
inline signed char CCFPGMap::GetMapCellAreaIDFromCircleSize ( int nCircleSize ) 
{
	ASSERT ( ( nCircleSize <= MAX_CIRCLE_SIZE ) && ( nCircleSize >= 2 ) );

	// vr�t� ID MapCellu oblasti s kruhem velikosti "nCircleSize"
	return 2 * nCircleSize + 1;
}

// vr�t� ID MapCellu oblasti MapCellu "cMapCellID"
inline signed char CCFPGMap::GetMapCellAreaIDFromMapCellID ( signed char cMapCellID ) 
{
	ASSERT ( ( cMapCellID >= 0 ) && ( cMapCellID <= ( 2 * MAX_CIRCLE_SIZE + 1 ) ) );

	// vr�t� ID MapCellu oblasti
	return ( cMapCellID | 0x01 );
}

// zjist�, je-li "cMapCellID" ID MapCellu oblasti
inline BOOL CCFPGMap::IsMapCellAreaID ( signed char cMapCellID ) 
{
	ASSERT ( ( cMapCellID >= 0 ) && ( cMapCellID <= ( 2 * MAX_CIRCLE_SIZE + 1 ) ) );

	// vr�t� p��znak ID MapCellu oblasti
	return ( cMapCellID & 0x01 );
}

// zv�t�� tabulku hint� cesty o velikost "dwSize"
inline void CCFPGMap::IncreasePathHintTable ( DWORD dwSize ) 
{
	ASSERT ( ( dwSize > 0 ) && ( m_dwPathHintTableSize + dwSize >= dwSize ) );

	ASSERT ( ( m_pPathHintTable != NULL ) && ( m_dwAllocatedPathHintTableSize > 0 ) && 
		( m_dwPathHintTableSize <= m_dwAllocatedPathHintTableSize ) );

	// zjist�, je-li tabulka hint� cest dostate�n� velik�
	if ( m_dwAllocatedPathHintTableSize < m_dwPathHintTableSize + dwSize )
	{	// tabulka hint� cest je mal�
		ASSERT ( m_dwAllocatedPathHintTableSize < 2 * m_dwAllocatedPathHintTableSize );
		// alokuje pot�ebnou velikost tabulky
		if ( ( m_pPathHintTable = (DWORD *)realloc ( m_pPathHintTable, 
			( m_dwAllocatedPathHintTableSize = max ( 2 * m_dwAllocatedPathHintTableSize, 
			m_dwPathHintTableSize + dwSize ) ) * sizeof ( DWORD ) ) ) == NULL )
		{	// nepoda�ilo se alokovat po�adovanou pam�
			// zne�kodn� data tabulky hint� cest
			m_dwPathHintTableSize = 0;
			m_dwAllocatedPathHintTableSize = 0;
			// obslou�� chybu pam�ti
			AfxThrowMemoryException ();
		}
	}
	// tabulka hint� cest je dostate�n� velk�

	// zv�t�� velikost tabulky hint�
	m_dwPathHintTableSize += dwSize;

	ASSERT ( ( m_dwPathHintTableSize > 0 ) && ( m_dwPathHintTableSize <= 
		m_dwAllocatedPathHintTableSize ) );
}

//////////////////////////////////////////////////////////////////////
// Glob�ln� funkce
//////////////////////////////////////////////////////////////////////

// vytvo�� grafy pro hled�n� cesty na map� "cMapArchive"
enum ECreateFindPathGraphError CreateFindPathGraphs ( CDataArchive cMapArchive ) 
{
	// n�vratov� hodnota
	enum ECreateFindPathGraphError eReturnValue = ECFPGE_OK;
	// mapa vytv��en� grafu pro hled�n� cesty
	class CCFPGMap cMap;

	// vytvo�� adres�� graf� pro hled�n� cesty
	cMapArchive.MakeDirectory ( FIND_PATH_GRAPHS_DIRECTORY );
	// otev�e archiv graf� pro hled�n� cesty
	CDataArchive cFindPathGraphsArchive = cMapArchive.CreateArchive ( 
		FIND_PATH_GRAPHS_DIRECTORY );

	// po�et graf� pro hled�n� cesty
	DWORD dwFindPathGraphCount;

	// zjist� po�et graf� pro hled�n� cesty
	try
	{
		// otev�e soubor mapy
		CArchiveFile cMapFile = cMapArchive.CreateFile ( MAP_FILE_NAME, CFile::modeRead | 
			CFile::shareDenyWrite );

	// na�te hlavi�ku verze souboru
		{
			SFileVersionHeader sFileVersionHeader;
			CHECK_MAP_FILE ( cMapFile.Read ( &sFileVersionHeader, sizeof ( sFileVersionHeader ) ) == sizeof ( sFileVersionHeader ) );

			// identifik�tor souboru mapy
			BYTE aMapFileID[16] = MAP_FILE_IDENTIFIER;
			ASSERT ( sizeof ( aMapFileID ) == sizeof ( sFileVersionHeader.m_aMapFileID ) );

			// zkontroluje identifik�tor souboru mapy
			for ( int nIndex = 16; nIndex-- > 0; )
			{
				CHECK_MAP_FILE ( aMapFileID[nIndex] == sFileVersionHeader.m_aMapFileID[nIndex] );
			}

			ASSERT ( CURRENT_MAP_FILE_VERSION >= COMPATIBLE_MAP_FILE_VERSION );
			// zkontroluje verze form�tu mapy
			CHECK_MAP_FILE ( sFileVersionHeader.m_dwFormatVersion >= 
				sFileVersionHeader.m_dwCompatibleFormatVersion );

			// zjist�, jedn�-li se o spr�vnou verzi form�tu mapy
			if ( sFileVersionHeader.m_dwFormatVersion != CURRENT_MAP_FILE_VERSION )
			{	// nejedn� se o spr�vnou verzi form�tu mapy
				// zjist�, jedn�-li se o starou verzi form�tu mapy
				if ( sFileVersionHeader.m_dwFormatVersion < CURRENT_MAP_FILE_VERSION )
				{	// jedn� se o starou verzi form�tu mapy
					// zkontroluje kompatabilitu verze form�tu mapy
					CHECK_MAP_FILE ( sFileVersionHeader.m_dwFormatVersion >= 
						COMPATIBLE_MAP_FILE_VERSION );
				}
				else
				{	// jedn� se o mlad�� verzi form�tu mapy
					// zkontroluje kompatabilitu verze form�tu mapy
					CHECK_MAP_FILE ( sFileVersionHeader.m_dwCompatibleFormatVersion <= 
						CURRENT_MAP_FILE_VERSION );
				}
			}
		}

	// na�te hlavi�ku mapy
		{
			SMapHeader sMapHeader;
			CHECK_MAP_FILE ( cMapFile.Read ( &sMapHeader, sizeof ( sMapHeader ) ) == sizeof ( sMapHeader ) );

			// zjist� po�et graf� pro hled�n� cesty
			dwFindPathGraphCount = sMapHeader.m_dwFindPathGraphCount;
		}
	}
	catch ( CCFPGMapFileException *pException )
	{
		// zni�� v�jimku
		pException->Delete ();
		// vr�t� p��znak chybn�ho souboru mapy
		return pException->m_eError;
	}
	catch ( CDataArchiveException *pException )
	{
		// zni�� v�jimku
		pException->Delete ();
		// vr�t� p��znak chybn�ho souboru mapy
		return ECFPGE_BadMapFile;
	}
	catch ( CMemoryException *pException )
	{
		// zni�� v�jimku
		pException->Delete ();
		// vr�t� p��znak nedostatku pam�ti
		return ECFPGE_NotEnoughMemory;
	}
	catch ( CException *pException )
	{
		// zni�� v�jimku
		pException->Delete ();
		// vr�t� p��znak nezn�m� chyby
		return ECFPGE_UnknownError;
	}

	// nech� vytvo�it grafy pro hled�n� cesty
	for ( DWORD dwFindPathGraphIndex = dwFindPathGraphCount; dwFindPathGraphIndex-- > 0; )
	{
		// vytvo�� mapu pro graf pro hled�n� cesty "dwFindPathGraphIndex"
		enum ECreateFindPathGraphError eError = cMap.Create ( dwFindPathGraphIndex, 
			cMapArchive, ( dwFindPathGraphIndex == 0 ) );
		// zjist�, poda�ilo-li se vytvo�it mapu
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

		// vytvo�� graf pro hled�n� cesty "dwFindPathGraphIndex"
		cMap.CreateFindPathGraph ( cFindPathGraphsArchive );

		// zni�� vytvo�enou mapu
		cMap.Delete ();
	}

	ASSERT ( ( eReturnValue == ECFPGE_OK ) || ( eReturnValue == 
		ECFPGE_IncompleteMapLand ) );
	// vr�t� p��znak vytvo�en� graf� pro hled�n� cesty
	return eReturnValue;
}
