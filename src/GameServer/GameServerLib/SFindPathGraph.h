/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída grafu pro hledání cesty
 * 
 ***********************************************************/

#ifndef __SERVER_FIND_PATH_GRAPH__HEADER_INCLUDED__
#define __SERVER_FIND_PATH_GRAPH__HEADER_INCLUDED__

#include "AbstractDataClasses\256BitArray.h"

#include "Common\MemoryPool\Array2DOnDemand.h"
#include "Common\MemoryPool\SelfPooledList.h"
#include "Common\MemoryPool\SelfPooledQueue.h"
#include "Common\MemoryPool\SmartPriorityQueue.h"

#include "SCheckPointPositionList.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace tøíd

// tøída mapy
class CSMap;
// tøída cesty
class CSPath;

//////////////////////////////////////////////////////////////////////
// Tøída grafu pro hledání cesty na serveru hry.
class CSFindPathGraph : public CObject 
{
	friend class CSMap;
	friend class CSPath;

	DECLARE_DYNAMIC ( CSFindPathGraph )

// Datové typy
public:
	// stav hledání lokální cesty
	enum EFindLocalPathState 
	{
		EFLPS_Found,			// cesta byla nalezena
		EFLPS_UnitBlocked,	// jednotka je zablokována
		EFLPS_PathBlocked,	// cesta je zablokována
	};

private:
	// vzdálenost brány mostu
	struct SBridgeGateDistance 
	{
		// vzdálenost brány mostu
		DWORD dwDistance;
		// pøedchozí brána mostu
		DWORD dwPreviousBridgeGateIndex;

		// konstruktor
		SBridgeGateDistance () {};
		// konstruktor
		SBridgeGateDistance ( DWORD dwInitDistance, DWORD dwInitPreviousBridgeGateIndex ) 
			{ dwDistance = dwInitDistance; dwPreviousBridgeGateIndex = dwInitPreviousBridgeGateIndex; };
		// konstruktor
		SBridgeGateDistance ( SBridgeGateDistance &sBridgeGateDistance ) 
			{ dwDistance = sBridgeGateDistance.dwDistance; dwPreviousBridgeGateIndex = sBridgeGateDistance.dwPreviousBridgeGateIndex; };
	};

	// dopøedná deklarace brány mostu
	struct SBridgeGate;
	// dopøedná deklarace hintu cesty
	struct SPathHint;
public:
	// oblast
	struct SArea 
	{
		// poèet bran mostù oblasti
		DWORD dwBridgeGateCount;
		// šíøka oblasti
		DWORD dwWidth;
		// následují tato data:
		//		// pole ukazatelù na brány mostu oblasti
		//		struct SBridgeGate *aBridgeGates[dwBridgeGateCount];
		//		// pole cest mezi branami mostù oblasti
		//		struct SBridgeGatePath[dwBridgeGateCount*(dwBridgeGateCount-1)/2]
		//			// 0,1; 0,2; 0,3; ...; 0,(dwBridgeGateCount-1); 1,2; 1,3; ...; 
		//			//		1,(dwBridgeGateCount-1); ...; (dwBridgeGateCount-2),(dwBridgeGateCount-1)
	};
private:
	// brána mostu
	struct SBridgeGate 
	{
		// souøadnice brány mostu
		DWORD dwX;
		DWORD dwY;
		// ukazatel na oblast bran brány mostu
		struct SArea *pBridgeArea;
		// ukazatel na oblast brány mostu
		struct SArea *pArea;
	};
	// cesta mezi branami mostù
	struct SBridgeGatePath 
	{
		// vzdálenost bran mostù
		DWORD dwBridgeGateDistance;
		// ukazatel na hint cesty
		struct SPathHint *pPathHint;
	};
	// hint cesty
	struct SPathHint 
	{
		// poèet kontrolních bodù hintu cesty
		DWORD dwCheckPointCount;
		// následují tato data:
		//		// pole kontrolních bodù hintu cesty
		//		struct SPathHintCheckPoint[dwCheckPointCount]

		// konstruktor
		SPathHint ( DWORD dwInitCheckPointCount ) 
			{ dwCheckPointCount = dwInitCheckPointCount; };
	};
	// kontrolní bod hintu cesty
	struct SPathHintCheckPoint 
	{
		// souøadnice kontrolního bodu hintu cesty
		DWORD dwX;
		DWORD dwY;
	};

// Metody
private:
// Konstrukce a destrukce

	// konstruktor
	CSFindPathGraph ();
	// destruktor
	~CSFindPathGraph ();

// Inicializace a znièení dat objektu

	// vytvoøí graf pro hledání cesty s ID "dwID" z archivu "cArchive"
	void Create ( DWORD dwID, CDataArchive cArchive );
	// znièí graf pro hledání cesty
	void Delete ();

protected:
// Inicializace a znièení grafù pro hledání cesty

	// zaène vytváøení "dwFindPathGraphCount" grafù pro hledání cesty
	static void PreCreateFindPathGraphs ( DWORD dwFindPathGraphCount );
	// vytvoøí graf pro hledání cesty s indexem "dwIndex", s ID "dwID" a se jménem 
	//		"szName" v archivu mapy "cMapArchive", vrací ukazatel na vytvoøený graf
	static CSFindPathGraph *CreateFindPathGraph ( DWORD dwIndex, DWORD dwID, 
		const char *szName, CDataArchive cMapArchive );
	// ukonèí vytváøení grafù pro hledání cesty
	static void PostCreateFindPathGraphs ();
	// znièí grafy pro hledání cesty
	static void DeleteFindPathGraphs ();

// Ukládání grafù pro hledání cesty (obdoba CPersistentObject metod)

	// ukládání dat grafù pro hledání cesty
	static void PersistentSaveFindPathGraphs ( CPersistentStorage &storage );
	// nahrávání pouze uložených grafù pro hledání cesty
	static void PersistentLoadFindPathGraphs ( CPersistentStorage &storage );
	// pøeklad ukazatelù grafù pro hledání cesty
	static void PersistentTranslatePointersFindPathGraphs ( CPersistentStorage &storage );
	// inicializace nahraných grafù pro hledání cesty
	static void PersistentInitFindPathGraphs ();

protected:
// Operace s grafy pro hledání cesty

	// najde novou cestu šíøky "dwWidth" z místa "pointStart" do místa "pointEnd", vrací 
	//		seznam kontrolních bodù cesty "pCheckPointPositionList"
	BOOL FindPath ( CPointDW pointStart, CPointDW pointEnd, DWORD dwWidth, 
		CSCheckPointPositionList *pCheckPointPositionList );
	// *****************************, POSITION posCheckPointPosition, CSPath *pPath
	// najde lokální cestu šíøky "dwWidth" z místa "pointStart" do místa "pointEnd" 
	//		pro jednotku "pUnit", vrací pøidané a upravené kontrolní body na pozici 
	//		"posCheckPointPosition"
	enum EFindLocalPathState FindLocalPath ( CSUnit *pUnit, CPointDW pointStart, 
		CPointDW pointEnd, DWORD dwWidth, CSPath *pPath );
	// *****************************, POSITION posCheckPointPosition, CSPath *pPath
	// najde vzdušnou lokální cestu z místa "pointStart" do místa "pointEnd" pro jednotku 
	//		"pUnit", vrací pøidané a upravené kontrolní body na pozici 
	//		"posCheckPointPosition"
	static enum EFindLocalPathState FindAirLocalPath ( CSUnit *pUnit, CPointDW pointStart, 
		CPointDW pointEnd, CSPath *pPath );

private:
// Pomocné operace pro hledání cesty

	// vloží na pozici "posCheckPointPosition" seznam kontrolních bodù krátké cesty 
	//		pro šíøku MapCellu "cMapCellWidth" z místa "pointStart" do místa "pointEnd" 
	//		(cesta urèitì existuje)
	void FindShortPath ( CPointDW pointStart, CPointDW pointEnd, 
		signed char cMapCellWidth, POSITION posCheckPointPosition );
	// *******************************************bFoundNearStart, CPointDW &
	// vrátí index nejbližší brány mostu z pozice "pointStart" pro šíøku MapCellu 
	//		"cMapCellWidth" (DWORD_MAX=neexistuje brána mostu, DWORD_MAX-1="pointEnd" je 
	//		bližší)
	DWORD GetNearestBridgeGateIndex ( CPointDW &pointStart, signed char cMapCellWidth, 
		CPointDW pointEnd, BOOL bFoundNearStart = FALSE );
	// *****************************, CPointDW &pointSecondBridgeGate, CPointDW &pointPreLastBridgeGate
	// najde nejkratší cestu od brány mostu "dwStartBridgeGateIndex" k bránì mostu 
	//		"dwEndBridgeGateIndex" šíøky "dwWidth", vyplní seznam kontrolních bodù cesty 
	//		"pCheckPointPositionList", vrací poèet bran mostù cesty
	DWORD FindBridgeGatePath ( DWORD dwStartBridgeGateIndex, DWORD dwEndBridgeGateIndex, 
		DWORD dwWidth, CSCheckPointPositionList *pCheckPointPositionList, 
		CPointDW &pointSecondBridgeGate, CPointDW &pointPreLastBridgeGate );
	// vynuluje vzdálenosti bran mostù (pomocná metoda metody "FindBridgeGatePath")
	static void ClearBridgeGatesDistances ();
	// aktualizuje vzdálenosti bran mostù oblasti "pArea" od brány mostu 
	//		"dwBridgeGateIndex" (pomocná metoda metody "FindBridgeGatePath")
	void UpdateBridgeGateAreaDistances ( DWORD dwBridgeGateIndex, struct SArea *pArea );

	// vrátí hodnotu MapCellu na pozici "pointMapCellPosition"
	signed char GetMapCellAt ( CPointDW pointMapCellPosition );
	// zjistí, je-li "cMapCell" MapCell brány mostu
	BOOL IsBridgeGateMapCell ( signed char cMapCell ) { return ( cMapCell & 0x01 ); };

// Data
private:
// Informace o grafu pro hledání cesty

	// ID grafu pro hledání cesty
	DWORD m_dwFindPathGraphID;

	// tabulka oblastí
	DWORD *m_aAreaTable;
	// tabulka bran mostù
	struct SBridgeGate *m_aBridgeGateTable;
	// tabulka hintù cest
	DWORD *m_aPathHintTable;

	// mapa grafu pro hledání cesty
	signed char *m_pMap;

// Grafy pro hledání cesty

	// poèet grafù pro hledání cesty
	static DWORD m_dwFindPathGraphCount;
	// ukazatel na pole grafù pro hledání cesty
	static CSFindPathGraph *m_aFindPathGraph;
	// prázdný hint cesty
	static struct SPathHint m_sEmptyPathHint;

// Data pro hledání cesty

	// pole vzdáleností MapCellù
	static CArray2DOnDemand<DWORD> m_cMapCellDistance;
	// prioritní fronta pozic na mapì
	static CSmartPriorityQueue<CPointDW> m_cPositionPriorityQueue;
	// pole offsetù pozic okolních pozic na mapì
	static CPointDW m_aSurroundingPositionOffset[8];

// Data pro hledání cesty mezi branami

	// nejvìtší poèet bran mostù grafu pro hledání cesty
	static DWORD m_dwMaxBridgeGateCount;
	// pole vzdáleností bran mostù
	static struct SBridgeGateDistance *m_aBridgeGateDistance;
	// seznam bran mostù
	static CSelfPooledList<DWORD> m_cBridgeGateList ;
};

#endif //__SERVER_FIND_PATH_GRAPH__HEADER_INCLUDED__
