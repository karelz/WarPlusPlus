/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída cesty
 * 
 ***********************************************************/

#ifndef __SERVER_PATH__HEADER_INCLUDED__
#define __SERVER_PATH__HEADER_INCLUDED__

#include "GameServerCompileSettings.h"

#include "Common\MemoryPool\SelfPooledList.h"
#include "Common\ReadWriteLock\ReadWriteLock.h"

#include "AbstractDataClasses\256BitArray.h"

#include "SFindPathRequestQueue.h"
#include "SCheckPointPositionList.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace tøíd

// tøída mapy
class CSMap;
// tøída typu pohybové skilly
class CSMoveSkillType;
// ******************************
class CSFindPathGraph;

//////////////////////////////////////////////////////////////////////
// Tøída cesty na serveru hry.
class CSPath : public CPersistentObject 
{
	friend class CSMap;
	friend class CSMoveSkillType;
	// ******************************
	friend class CSFindPathGraph;

	DECLARE_DYNAMIC ( CSPath )

// Datové typy
public:
	// stav cesty
	enum EPathState 
	{
		EPS_Valid,				// cesta je v poøádku
		EPS_Found,				// cesta byla nalezena
		EPS_NotExists,			// cesta neexistuje
		EPS_Blocked,			// cesta je zablokovaná
		EPS_Searching,			// cesta se hledá
		EPS_Uninitialized, 	// cesta není inicializována
	};

// Metody
private:
// Konstrukce a destrukce

	// konstruktor
	CSPath ();
	// destruktor (lze volat jen z FindPathLoop threadu)
	~CSPath ();

public:
// Inicializace a znièení dat objektu

	// znièí cestu civilizace "dwCivilizationIndex"
	void Destroy ( DWORD dwCivilizationIndex );
	// znièí cestu
	void Destroy ();

public:
// Ukládání dat objektu (CPersistentObject metody)

	// ukládání dat cesty
	void PersistentSave ( CPersistentStorage &storage );
	// nahrávání pouze uložených dat cesty (vrací nahraný objekt cesty)
	static CSPath *PersistentLoadPath ( CPersistentStorage &storage );
private:
	// nahrání uložených dat cesty
	void PersistentLoad ( CPersistentStorage &storage );
public:
	// pøeklad ukazatelù cesty
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahraného objektu cesty
	void PersistentInit ();

protected:
// Inicializace a znièení dat cest

	// vytvoøí data cest
	static void CreatePathes ( CDataArchive cMapArchive );
	// znièí data cest
	static void DeletePathes ();

protected:
// Ukládání dat cest (obdoba CPersistentObject metod)

	// ukládání dat cest
	static void PersistentSavePathes ( CPersistentStorage &storage );
	// nahrávání pouze uložených dat cest s "dwCivilizationCount" civilizacemi
	static void PersistentLoadPathes ( CPersistentStorage &storage, 
		DWORD dwCivilizationCount );
	// pøeklad ukazatelù cest
	static void PersistentTranslatePointersPathes ( CPersistentStorage &storage );
	// inicializace nahraných objektù cest
	static void PersistentInitPathes ();

public:
// Operace s cestou

	// nechá najít novou cestu šíøky "dwWidth" z místa "pointPosition" do místa 
	//		"pointDestination" v grafu "pFindPathGraph" (NULL=vzdušná cesta) pro civilizaci 
	//		"dwCivilizationIndex" 
	static CSPath *FindPath ( CPointDW pointPosition, CPointDW pointDestination, 
		CSFindPathGraph *pFindPathGraph, DWORD dwWidth, DWORD dwCivilizationIndex );
protected:
	// nechá najít lokální cestu k prvnímu kontrolnímu bodu cesty (je-li jednotka blízko 
	//		kontrolního bodu cesty, pøeskoèí ho) (TRUE=kontrolní bod cesty byl pøeskoèen)
	BOOL FindLocalPath ( CSUnit *pUnit );
	// vrátí novou kopii cesty (lze volat jen z FindPathLoop threadu)
	CSPath *Duplicate ();

private:
// Pomocné operace s cestou

	// najde novou cestu šíøky "dwWidth" z místa "pointPosition" do místa 
	//		"pointDestination" v grafu "pFindPathGraph" (NULL=vzdušná cesta) (lze volat jen 
	//		z FindPathLoop threadu)
	void FindPath ( CPointDW pointPosition, CPointDW pointDestination, 
		CSFindPathGraph *pFindPathGraph, DWORD dwWidth );
	// najde lokální cestu šíøky "dwWidth" z místa "pointPosition" do místa 
	//		"pointDestination" v grafu "pFindPathGraph" pro jednotku "pUnit" (lze volat jen 
	//		z FindPathLoop threadu)
	void FindLocalPath ( CPointDW pointPosition, CPointDW pointDestination, 
		CSFindPathGraph *pFindPathGraph, DWORD dwWidth, CSUnit *pUnit );
	// najde vzdušnou lokální cestu z místa "pointPosition" do místa "pointDestination" 
	//		pro jednotku "pUnit" (lze volat jen z FindPathLoop threadu)
	void FindAirLocalPath ( CPointDW pointPosition, CPointDW pointDestination, 
		CSUnit *pUnit );

public:
// Informace o cestì

	// vrátí stav cesty
	enum EPathState GetState () { return m_eState; };
protected:
	// pøijme nalezenou cestu (cesta bude dále v poøádku)
	void Confirm () 
		{ ASSERT ( ( m_eState == EPS_Found ) || ( m_eState == EPS_NotExists ) ); m_eState = EPS_Valid; };

// Operace s kontrolními body cesty pro MoveSkillu

	// vrátí pozici prvního kontrolního bodu cesty (NULL=konec cesty) (tuto metodu lze 
	//		volat výhradnì z MoveSkilly na cestu, kterou MoveSkilla vlastní jako jediná)
	CPointDW *GetCheckPointPosition ();
	// vrátí pozici druhého kontrolního bodu cesty (pøípadnì prvního, neexistuje-li 
	//		druhý) (tuto metodu lze volat výhradnì z MoveSkilly na cestu, kterou 
	//		MoveSkilla vlastní jako jediná)
	CPointDW *GetNextCheckPointPosition ();
	// odstraní první kontrolní bod cesty (tuto metodu lze volat výhradnì z MoveSkilly 
	//		na cestu, kterou MoveSkilla vlastní jako jediná)
	void RemoveCheckPoint ();

private:
// Operace s kontrolními body cesty

	// smaže seznam pozic kontrolních bodù cesty (zámek seznamù pozic kontrolních bodù 
	//		cesty musí být zamèen pro zápis)
	void DeleteDeletedCheckPointPositionList ();

protected:
// Vlákna výpoètu serveru hry

	// smyèka hledání cesty
	static UINT FindPathLoop ( LPVOID pMap );

// Data
private:
// Informace o cestì

	// pøíznaky stavu cesty
	enum EPathState m_eState;

// Informace o kontrolních bodech cesty

	// seznam pozic kontrolních bodù cesty
	CSCheckPointPositionList m_cCheckPointPositionList;
	// seznam pozic smazaných kontrolních bodù cesty
	CSCheckPointPositionList m_cDeletedCheckPointPositionList;

// Informace o cestách

	// zámek front požadavkù na hledání cesty
	static CMutex m_mutexFindPathRequestQueueLock;
	// fronty požadavkù na hledání cesty
	static CSFindPathRequestQueue m_aFindPathRequestQueue[CIVILIZATION_COUNT_MAX];
	// událost požadavku na hledání cesty
	static CEvent m_eventFindPathRequest;
	// zámek seznamù pozic kontrolních bodù cesty
//	static CMutex m_mutexCheckPointPositionListLock;
	// seznam smazaných cest
	static CSelfPooledList<CSPath *> m_cDeletedPathList;
	// zámek seznamu smazaných cest
	static CMutex m_mutexDeletedPathListLock;

// Informace o vláknech výpoètu serveru hry

	// událost zapausování FindPathLoop threadu
	static CEvent m_eventFindPathLoopPaused;
};

#endif //__SERVER_PATH__HEADER_INCLUDED__
