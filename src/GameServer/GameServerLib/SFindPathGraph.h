/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da grafu pro hled�n� cesty
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
// Dop�edn� deklarace t��d

// t��da mapy
class CSMap;
// t��da cesty
class CSPath;

//////////////////////////////////////////////////////////////////////
// T��da grafu pro hled�n� cesty na serveru hry.
class CSFindPathGraph : public CObject 
{
	friend class CSMap;
	friend class CSPath;

	DECLARE_DYNAMIC ( CSFindPathGraph )

// Datov� typy
public:
	// stav hled�n� lok�ln� cesty
	enum EFindLocalPathState 
	{
		EFLPS_Found,			// cesta byla nalezena
		EFLPS_UnitBlocked,	// jednotka je zablokov�na
		EFLPS_PathBlocked,	// cesta je zablokov�na
	};

private:
	// vzd�lenost br�ny mostu
	struct SBridgeGateDistance 
	{
		// vzd�lenost br�ny mostu
		DWORD dwDistance;
		// p�edchoz� br�na mostu
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

	// dop�edn� deklarace br�ny mostu
	struct SBridgeGate;
	// dop�edn� deklarace hintu cesty
	struct SPathHint;
public:
	// oblast
	struct SArea 
	{
		// po�et bran most� oblasti
		DWORD dwBridgeGateCount;
		// ���ka oblasti
		DWORD dwWidth;
		// n�sleduj� tato data:
		//		// pole ukazatel� na br�ny mostu oblasti
		//		struct SBridgeGate *aBridgeGates[dwBridgeGateCount];
		//		// pole cest mezi branami most� oblasti
		//		struct SBridgeGatePath[dwBridgeGateCount*(dwBridgeGateCount-1)/2]
		//			// 0,1; 0,2; 0,3; ...; 0,(dwBridgeGateCount-1); 1,2; 1,3; ...; 
		//			//		1,(dwBridgeGateCount-1); ...; (dwBridgeGateCount-2),(dwBridgeGateCount-1)
	};
private:
	// br�na mostu
	struct SBridgeGate 
	{
		// sou�adnice br�ny mostu
		DWORD dwX;
		DWORD dwY;
		// ukazatel na oblast bran br�ny mostu
		struct SArea *pBridgeArea;
		// ukazatel na oblast br�ny mostu
		struct SArea *pArea;
	};
	// cesta mezi branami most�
	struct SBridgeGatePath 
	{
		// vzd�lenost bran most�
		DWORD dwBridgeGateDistance;
		// ukazatel na hint cesty
		struct SPathHint *pPathHint;
	};
	// hint cesty
	struct SPathHint 
	{
		// po�et kontroln�ch bod� hintu cesty
		DWORD dwCheckPointCount;
		// n�sleduj� tato data:
		//		// pole kontroln�ch bod� hintu cesty
		//		struct SPathHintCheckPoint[dwCheckPointCount]

		// konstruktor
		SPathHint ( DWORD dwInitCheckPointCount ) 
			{ dwCheckPointCount = dwInitCheckPointCount; };
	};
	// kontroln� bod hintu cesty
	struct SPathHintCheckPoint 
	{
		// sou�adnice kontroln�ho bodu hintu cesty
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

// Inicializace a zni�en� dat objektu

	// vytvo�� graf pro hled�n� cesty s ID "dwID" z archivu "cArchive"
	void Create ( DWORD dwID, CDataArchive cArchive );
	// zni�� graf pro hled�n� cesty
	void Delete ();

protected:
// Inicializace a zni�en� graf� pro hled�n� cesty

	// za�ne vytv��en� "dwFindPathGraphCount" graf� pro hled�n� cesty
	static void PreCreateFindPathGraphs ( DWORD dwFindPathGraphCount );
	// vytvo�� graf pro hled�n� cesty s indexem "dwIndex", s ID "dwID" a se jm�nem 
	//		"szName" v archivu mapy "cMapArchive", vrac� ukazatel na vytvo�en� graf
	static CSFindPathGraph *CreateFindPathGraph ( DWORD dwIndex, DWORD dwID, 
		const char *szName, CDataArchive cMapArchive );
	// ukon�� vytv��en� graf� pro hled�n� cesty
	static void PostCreateFindPathGraphs ();
	// zni�� grafy pro hled�n� cesty
	static void DeleteFindPathGraphs ();

// Ukl�d�n� graf� pro hled�n� cesty (obdoba CPersistentObject metod)

	// ukl�d�n� dat graf� pro hled�n� cesty
	static void PersistentSaveFindPathGraphs ( CPersistentStorage &storage );
	// nahr�v�n� pouze ulo�en�ch graf� pro hled�n� cesty
	static void PersistentLoadFindPathGraphs ( CPersistentStorage &storage );
	// p�eklad ukazatel� graf� pro hled�n� cesty
	static void PersistentTranslatePointersFindPathGraphs ( CPersistentStorage &storage );
	// inicializace nahran�ch graf� pro hled�n� cesty
	static void PersistentInitFindPathGraphs ();

protected:
// Operace s grafy pro hled�n� cesty

	// najde novou cestu ���ky "dwWidth" z m�sta "pointStart" do m�sta "pointEnd", vrac� 
	//		seznam kontroln�ch bod� cesty "pCheckPointPositionList"
	BOOL FindPath ( CPointDW pointStart, CPointDW pointEnd, DWORD dwWidth, 
		CSCheckPointPositionList *pCheckPointPositionList );
	// *****************************, POSITION posCheckPointPosition, CSPath *pPath
	// najde lok�ln� cestu ���ky "dwWidth" z m�sta "pointStart" do m�sta "pointEnd" 
	//		pro jednotku "pUnit", vrac� p�idan� a upraven� kontroln� body na pozici 
	//		"posCheckPointPosition"
	enum EFindLocalPathState FindLocalPath ( CSUnit *pUnit, CPointDW pointStart, 
		CPointDW pointEnd, DWORD dwWidth, CSPath *pPath );
	// *****************************, POSITION posCheckPointPosition, CSPath *pPath
	// najde vzdu�nou lok�ln� cestu z m�sta "pointStart" do m�sta "pointEnd" pro jednotku 
	//		"pUnit", vrac� p�idan� a upraven� kontroln� body na pozici 
	//		"posCheckPointPosition"
	static enum EFindLocalPathState FindAirLocalPath ( CSUnit *pUnit, CPointDW pointStart, 
		CPointDW pointEnd, CSPath *pPath );

private:
// Pomocn� operace pro hled�n� cesty

	// vlo�� na pozici "posCheckPointPosition" seznam kontroln�ch bod� kr�tk� cesty 
	//		pro ���ku MapCellu "cMapCellWidth" z m�sta "pointStart" do m�sta "pointEnd" 
	//		(cesta ur�it� existuje)
	void FindShortPath ( CPointDW pointStart, CPointDW pointEnd, 
		signed char cMapCellWidth, POSITION posCheckPointPosition );
	// *******************************************bFoundNearStart, CPointDW &
	// vr�t� index nejbli��� br�ny mostu z pozice "pointStart" pro ���ku MapCellu 
	//		"cMapCellWidth" (DWORD_MAX=neexistuje br�na mostu, DWORD_MAX-1="pointEnd" je 
	//		bli���)
	DWORD GetNearestBridgeGateIndex ( CPointDW &pointStart, signed char cMapCellWidth, 
		CPointDW pointEnd, BOOL bFoundNearStart = FALSE );
	// *****************************, CPointDW &pointSecondBridgeGate, CPointDW &pointPreLastBridgeGate
	// najde nejkrat�� cestu od br�ny mostu "dwStartBridgeGateIndex" k br�n� mostu 
	//		"dwEndBridgeGateIndex" ���ky "dwWidth", vypln� seznam kontroln�ch bod� cesty 
	//		"pCheckPointPositionList", vrac� po�et bran most� cesty
	DWORD FindBridgeGatePath ( DWORD dwStartBridgeGateIndex, DWORD dwEndBridgeGateIndex, 
		DWORD dwWidth, CSCheckPointPositionList *pCheckPointPositionList, 
		CPointDW &pointSecondBridgeGate, CPointDW &pointPreLastBridgeGate );
	// vynuluje vzd�lenosti bran most� (pomocn� metoda metody "FindBridgeGatePath")
	static void ClearBridgeGatesDistances ();
	// aktualizuje vzd�lenosti bran most� oblasti "pArea" od br�ny mostu 
	//		"dwBridgeGateIndex" (pomocn� metoda metody "FindBridgeGatePath")
	void UpdateBridgeGateAreaDistances ( DWORD dwBridgeGateIndex, struct SArea *pArea );

	// vr�t� hodnotu MapCellu na pozici "pointMapCellPosition"
	signed char GetMapCellAt ( CPointDW pointMapCellPosition );
	// zjist�, je-li "cMapCell" MapCell br�ny mostu
	BOOL IsBridgeGateMapCell ( signed char cMapCell ) { return ( cMapCell & 0x01 ); };

// Data
private:
// Informace o grafu pro hled�n� cesty

	// ID grafu pro hled�n� cesty
	DWORD m_dwFindPathGraphID;

	// tabulka oblast�
	DWORD *m_aAreaTable;
	// tabulka bran most�
	struct SBridgeGate *m_aBridgeGateTable;
	// tabulka hint� cest
	DWORD *m_aPathHintTable;

	// mapa grafu pro hled�n� cesty
	signed char *m_pMap;

// Grafy pro hled�n� cesty

	// po�et graf� pro hled�n� cesty
	static DWORD m_dwFindPathGraphCount;
	// ukazatel na pole graf� pro hled�n� cesty
	static CSFindPathGraph *m_aFindPathGraph;
	// pr�zdn� hint cesty
	static struct SPathHint m_sEmptyPathHint;

// Data pro hled�n� cesty

	// pole vzd�lenost� MapCell�
	static CArray2DOnDemand<DWORD> m_cMapCellDistance;
	// prioritn� fronta pozic na map�
	static CSmartPriorityQueue<CPointDW> m_cPositionPriorityQueue;
	// pole offset� pozic okoln�ch pozic na map�
	static CPointDW m_aSurroundingPositionOffset[8];

// Data pro hled�n� cesty mezi branami

	// nejv�t�� po�et bran most� grafu pro hled�n� cesty
	static DWORD m_dwMaxBridgeGateCount;
	// pole vzd�lenost� bran most�
	static struct SBridgeGateDistance *m_aBridgeGateDistance;
	// seznam bran most�
	static CSelfPooledList<DWORD> m_cBridgeGateList ;
};

#endif //__SERVER_FIND_PATH_GRAPH__HEADER_INCLUDED__
