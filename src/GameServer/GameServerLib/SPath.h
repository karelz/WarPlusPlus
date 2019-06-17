/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da cesty
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
// Dop�edn� deklarace t��d

// t��da mapy
class CSMap;
// t��da typu pohybov� skilly
class CSMoveSkillType;
// ******************************
class CSFindPathGraph;

//////////////////////////////////////////////////////////////////////
// T��da cesty na serveru hry.
class CSPath : public CPersistentObject 
{
	friend class CSMap;
	friend class CSMoveSkillType;
	// ******************************
	friend class CSFindPathGraph;

	DECLARE_DYNAMIC ( CSPath )

// Datov� typy
public:
	// stav cesty
	enum EPathState 
	{
		EPS_Valid,				// cesta je v po��dku
		EPS_Found,				// cesta byla nalezena
		EPS_NotExists,			// cesta neexistuje
		EPS_Blocked,			// cesta je zablokovan�
		EPS_Searching,			// cesta se hled�
		EPS_Uninitialized, 	// cesta nen� inicializov�na
	};

// Metody
private:
// Konstrukce a destrukce

	// konstruktor
	CSPath ();
	// destruktor (lze volat jen z FindPathLoop threadu)
	~CSPath ();

public:
// Inicializace a zni�en� dat objektu

	// zni�� cestu civilizace "dwCivilizationIndex"
	void Destroy ( DWORD dwCivilizationIndex );
	// zni�� cestu
	void Destroy ();

public:
// Ukl�d�n� dat objektu (CPersistentObject metody)

	// ukl�d�n� dat cesty
	void PersistentSave ( CPersistentStorage &storage );
	// nahr�v�n� pouze ulo�en�ch dat cesty (vrac� nahran� objekt cesty)
	static CSPath *PersistentLoadPath ( CPersistentStorage &storage );
private:
	// nahr�n� ulo�en�ch dat cesty
	void PersistentLoad ( CPersistentStorage &storage );
public:
	// p�eklad ukazatel� cesty
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahran�ho objektu cesty
	void PersistentInit ();

protected:
// Inicializace a zni�en� dat cest

	// vytvo�� data cest
	static void CreatePathes ( CDataArchive cMapArchive );
	// zni�� data cest
	static void DeletePathes ();

protected:
// Ukl�d�n� dat cest (obdoba CPersistentObject metod)

	// ukl�d�n� dat cest
	static void PersistentSavePathes ( CPersistentStorage &storage );
	// nahr�v�n� pouze ulo�en�ch dat cest s "dwCivilizationCount" civilizacemi
	static void PersistentLoadPathes ( CPersistentStorage &storage, 
		DWORD dwCivilizationCount );
	// p�eklad ukazatel� cest
	static void PersistentTranslatePointersPathes ( CPersistentStorage &storage );
	// inicializace nahran�ch objekt� cest
	static void PersistentInitPathes ();

public:
// Operace s cestou

	// nech� naj�t novou cestu ���ky "dwWidth" z m�sta "pointPosition" do m�sta 
	//		"pointDestination" v grafu "pFindPathGraph" (NULL=vzdu�n� cesta) pro civilizaci 
	//		"dwCivilizationIndex" 
	static CSPath *FindPath ( CPointDW pointPosition, CPointDW pointDestination, 
		CSFindPathGraph *pFindPathGraph, DWORD dwWidth, DWORD dwCivilizationIndex );
protected:
	// nech� naj�t lok�ln� cestu k prvn�mu kontroln�mu bodu cesty (je-li jednotka bl�zko 
	//		kontroln�ho bodu cesty, p�esko�� ho) (TRUE=kontroln� bod cesty byl p�esko�en)
	BOOL FindLocalPath ( CSUnit *pUnit );
	// vr�t� novou kopii cesty (lze volat jen z FindPathLoop threadu)
	CSPath *Duplicate ();

private:
// Pomocn� operace s cestou

	// najde novou cestu ���ky "dwWidth" z m�sta "pointPosition" do m�sta 
	//		"pointDestination" v grafu "pFindPathGraph" (NULL=vzdu�n� cesta) (lze volat jen 
	//		z FindPathLoop threadu)
	void FindPath ( CPointDW pointPosition, CPointDW pointDestination, 
		CSFindPathGraph *pFindPathGraph, DWORD dwWidth );
	// najde lok�ln� cestu ���ky "dwWidth" z m�sta "pointPosition" do m�sta 
	//		"pointDestination" v grafu "pFindPathGraph" pro jednotku "pUnit" (lze volat jen 
	//		z FindPathLoop threadu)
	void FindLocalPath ( CPointDW pointPosition, CPointDW pointDestination, 
		CSFindPathGraph *pFindPathGraph, DWORD dwWidth, CSUnit *pUnit );
	// najde vzdu�nou lok�ln� cestu z m�sta "pointPosition" do m�sta "pointDestination" 
	//		pro jednotku "pUnit" (lze volat jen z FindPathLoop threadu)
	void FindAirLocalPath ( CPointDW pointPosition, CPointDW pointDestination, 
		CSUnit *pUnit );

public:
// Informace o cest�

	// vr�t� stav cesty
	enum EPathState GetState () { return m_eState; };
protected:
	// p�ijme nalezenou cestu (cesta bude d�le v po��dku)
	void Confirm () 
		{ ASSERT ( ( m_eState == EPS_Found ) || ( m_eState == EPS_NotExists ) ); m_eState = EPS_Valid; };

// Operace s kontroln�mi body cesty pro MoveSkillu

	// vr�t� pozici prvn�ho kontroln�ho bodu cesty (NULL=konec cesty) (tuto metodu lze 
	//		volat v�hradn� z MoveSkilly na cestu, kterou MoveSkilla vlastn� jako jedin�)
	CPointDW *GetCheckPointPosition ();
	// vr�t� pozici druh�ho kontroln�ho bodu cesty (p��padn� prvn�ho, neexistuje-li 
	//		druh�) (tuto metodu lze volat v�hradn� z MoveSkilly na cestu, kterou 
	//		MoveSkilla vlastn� jako jedin�)
	CPointDW *GetNextCheckPointPosition ();
	// odstran� prvn� kontroln� bod cesty (tuto metodu lze volat v�hradn� z MoveSkilly 
	//		na cestu, kterou MoveSkilla vlastn� jako jedin�)
	void RemoveCheckPoint ();

private:
// Operace s kontroln�mi body cesty

	// sma�e seznam pozic kontroln�ch bod� cesty (z�mek seznam� pozic kontroln�ch bod� 
	//		cesty mus� b�t zam�en pro z�pis)
	void DeleteDeletedCheckPointPositionList ();

protected:
// Vl�kna v�po�tu serveru hry

	// smy�ka hled�n� cesty
	static UINT FindPathLoop ( LPVOID pMap );

// Data
private:
// Informace o cest�

	// p��znaky stavu cesty
	enum EPathState m_eState;

// Informace o kontroln�ch bodech cesty

	// seznam pozic kontroln�ch bod� cesty
	CSCheckPointPositionList m_cCheckPointPositionList;
	// seznam pozic smazan�ch kontroln�ch bod� cesty
	CSCheckPointPositionList m_cDeletedCheckPointPositionList;

// Informace o cest�ch

	// z�mek front po�adavk� na hled�n� cesty
	static CMutex m_mutexFindPathRequestQueueLock;
	// fronty po�adavk� na hled�n� cesty
	static CSFindPathRequestQueue m_aFindPathRequestQueue[CIVILIZATION_COUNT_MAX];
	// ud�lost po�adavku na hled�n� cesty
	static CEvent m_eventFindPathRequest;
	// z�mek seznam� pozic kontroln�ch bod� cesty
//	static CMutex m_mutexCheckPointPositionListLock;
	// seznam smazan�ch cest
	static CSelfPooledList<CSPath *> m_cDeletedPathList;
	// z�mek seznamu smazan�ch cest
	static CMutex m_mutexDeletedPathListLock;

// Informace o vl�knech v�po�tu serveru hry

	// ud�lost zapausov�n� FindPathLoop threadu
	static CEvent m_eventFindPathLoopPaused;
};

#endif //__SERVER_PATH__HEADER_INCLUDED__
