/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CZUnit -> bude CZUnit!!!
 *          
 ***********************************************************/

#if !defined(AFX_IUNIT_H__644304B6_A7CA_11D3_AF92_004F49068BD6__INCLUDED_)
#define AFX_IUNIT_H__644304B6_A7CA_11D3_AF92_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TripleS\Interpret\Src\InterpretEnums.h"
#include "TripleS\Interpret\Src\IObjUnitStruct.h"

#include "ZUnitNotifyQueueItem.h"
#include "ZUnitCommandQueueItem.h"

#define MAX_COMMANDS_COUNT          300
#define MAX_NOTIFICATIONS_COUNT     1000

class CIDataStackItem;
class CIConstructorFinishedWaiting;
class CInterpret;
class CIProcess;
class CZCivilization;
class CSUnit;
class CISkillInterface;

class CZUnit : public CIObjUnitStruct
{
	friend CISkillInterface;

    DECLARE_DYNAMIC(CZUnit);

    friend CZCivilization;
    friend CInterpret;

public:
	CZUnit();
	virtual ~CZUnit();

	// inicializace a uvolneni statickych polozek
	static void CreateStatic();
	static void DeleteStatic();
	static bool m_bStaticInitialized;
//
// -> COMMON
//

	// vytvori jednotku a prida ji nadrizenemu pCommander.
	virtual void Create(CZCivilization *pCivilization, CSUnit *pSUnit, 
						CZUnit *pCommander, CIUnitType *pIUnitType, bool bPhysicalUnit, DWORD dwID);
	virtual void Delete();
	void PreDelete();

	// pokud refcount je 0, zavola Detele a uvolni jednotku a vrati true, Jinak vrati false.
    // !!Tohle se vola jen ze serveru, nevolat primo!!!
	bool TryDelete();

	// je mrtva?
	bool IsDead() { return m_bIsDead; }

  // vrati serverovou jednotku - pokud to je fyzicka jednotka, jinak NULL
  CSUnit *GetSUnit(){ return m_pSUnit; }

  // returns the civilization
  CZCivilization *GetCivilization(){ return m_pCivilization; }

  // returns the commander for this unit - unit hierarchy must be locked on civilization
  CZUnit *GetCommander(){ return m_pCommander; }
  // returns first inferior unit - unit hierarchy must be locked
  CZUnit *GetFirstInferior(){ return m_pFirstInferior; }
  // returns next sibling unit - unit hierarchy must be locked
  CZUnit *GetNextSibling(){ return m_pNextSibbling; }
  // returns previous sibling unit - unit hierarchy must be locked
  CZUnit *GetPreviousSibling(){ return m_pPreviousSibbling; }

  // vrati jmeno jednotky
  CString GetName(){ return m_strName; }
  // nastavi jmeno jednotky
  void SetName(CString &strName){ m_strName = strName; }

  // vrati jmeno jednotky
  CString GetStatus(){ return m_strStatus; }
  // nastavi jmeno jednotky
  void SetStatus(CString &strStatus){ m_strStatus = strStatus; }

  // returns the ZUnit ID
  DWORD GetID(){ return m_dwID; }

protected:
    // Civilizace, do ktere jednotka patri.
    CZCivilization *m_pCivilization;

    // link na jednotku na serveru - pokud je to fyzicka jednotka
    CSUnit *m_pSUnit;

    // Jmeno jednotky
    CString m_strName;

	// Status jednotky (pise se do fullinfa)
	CString m_strStatus;

    // ID of the ZUnit object
    DWORD m_dwID;

    // priznak, zda je jednotka mrtva. Long kvuli tomu, aby slo volat InterlockedIncrement
    bool m_bIsDead;

  	// Priznak, zda se jedna o realnou (fyzickou) jednotku. Pokud ano, tak nemuze mit podrizene.
  	bool m_bPhysicalUnit;

	// priznad zda bylo zavolano PreDelete
	bool m_bPreDeleted;

  // If false-> do not send IDLE on next empty queue
  bool m_bSendIdle;

// VOJENSKA HIERARCHIE
    // Velitel jednotky
    CZUnit *m_pCommander;

    // Sousedni jednotky na stejne urovni se stejnym velitelem.
    CZUnit *m_pPreviousSibbling, *m_pNextSibbling;

    // Prvni a posledni podrizeni jednotky.
    CZUnit *m_pFirstInferior /*, *m_pLastInferior*/;

//
// -> PRO INTERPRET
//
private:
    // Staticka tabulka a polozky pro pristup k systemovym metodam a polozkam jednotky.
    // Vse je inicializovane v prvnim konstruktoru.

    // Metody
    // Tabulka dvojic <jmeno metody, ID metody>.
    static CMap<CStringTableItem*, CStringTableItem*, int, int> m_tpMethodIDs;
    static CMap<CStringTableItem*, CStringTableItem*, int, int> * m_ptpLoadMethodIDs;
	
	/*UNITMETHOD*/ 
    static enum { UNIT_METHOD_COUNT = 19 }; 

	// Pole   m_apMethods[ ID metody] == metoda
    static EInterpretStatus (CZUnit::* m_apMethods[ UNIT_METHOD_COUNT])( CIProcess*, CIBag*, bool);

    // Polozky
    static CStringTableItem *m_stiMemberName;
	static CStringTableItem *m_stiMemberStatus;
	/*UNITMEMBER*/

    // Pocitadlo vsech jednotek. Kdyz klesne na 0, staticke polozky se uvolni.
    static long m_nUnitInstanceCount;
    
public:
    virtual CIObjUnitStruct* AddRef();
    virtual CIObjUnitStruct* Release();

    // Pristup k skriptovym polozkam jednotky, vraci false, kdyz member nenalezen.
    bool LoadMember( CIDataStackItem &Reg, CStringTableItem *stiName, bool bCalledFromEnemy);
    bool StoreMember( CIDataStackItem *pReg, CStringTableItem *stiName, bool bCalledFromEnemy);

    // Pristup k C++ polozkam jednotky,  vraci false, kdyz nastala nejaka chyba.
    bool LoadSystemMember(  CIDataStackItem &Reg, CStringTableItem *stiName, bool bCalledFromEnemy);
    bool StoreSystemMember( CIDataStackItem *pReg, CStringTableItem *stiName, bool bCalledFromEnemy);

// Metody jednotky Unit dosazitelne ze skriptu
    
    // obecna metoda na vyvolavani metod.
    EInterpretStatus InvokeMethod(CIProcess *pProcess, CStringTableItem *stiName, CIBag *pBag);  

    // konkretni metody - zadna neni uspavaci
    // !! pri pridavani nezapomenou opravit inicializaci m_tpMethods v CZUnit kontruktoru
    EInterpretStatus M_GetLives         (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
    EInterpretStatus M_GetLivesMax      (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
    EInterpretStatus M_GetPosition      (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
    EInterpretStatus M_GetScriptName    (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
    EInterpretStatus M_GetViewRadius    (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
    EInterpretStatus M_GetCommander     (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
    EInterpretStatus M_GetInferiorUnits (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
    EInterpretStatus M_IsCommander      (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
    EInterpretStatus M_EmptyCommandQueue (CIProcess *pProcess, CIBag *pBag, bool bCalledByEnemy);
    EInterpretStatus M_ChangeScript     (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
    EInterpretStatus M_IsDead           (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
    EInterpretStatus M_CreateCommander  (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
    EInterpretStatus M_KillCommander    (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
    EInterpretStatus M_IsMyUnit         (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
  	EInterpretStatus M_ChangeCommander  (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
    EInterpretStatus M_GetUnitTypeName  (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
    EInterpretStatus M_SetTimer         (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
    EInterpretStatus M_IsEnemyUnit      (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
    EInterpretStatus M_IsCommandQueueEmpty
                                        (CIProcess*, CIBag *pBag, bool bCalledByEnemy);
    
	/*UNITMETHOD*/

	bool IsCommanderOfUnit( CZUnit *pUnit);
public:
    // typ jednotky
    CIUnitType *m_pIUnitType;

    // Priznak, zda jiz bylo dokonceno volani konstruktoru.
    // m_pConstructorFinished == NULL   =>  volani dokonceno
    // m_pConstructorFinished != NULL   =>  volani probiha, obsahuje pointer na objekt, pomoci
    //                                      ktereho muze jiny process cekat, az konstruktor dobehne
    CIConstructorFinishedWaiting *m_pConstructorFinished;

protected:
    // datove polozky    
    int *m_pData;

//
//  -> FRONTY ZPRAV JEDNOTKY
//

protected:
// nadrizena (horni) fronta prikazu
    CZUnitCommandQueueItem *m_pMandatoryCommandQueueFirst, *m_pMandatoryCommandQueueLast;
// aktualni (dolni) fronta prikazu
    CZUnitCommandQueueItem  *m_pCurrentCommandQueueFirst, *m_pCurrentCommandQueueLast;

    ULONG m_nMandatoryCommandsCount, m_nCurrentCommandsCount;

    // priznak, zda se vyrizuje prikaz z mandatory queue, nebo z current queue
    bool m_bProcesingMandatoryQueueCommand;
    // process vyrizujici prvni prikaz ( kdyz je == NULL, tak to jeste lezi ve fronte interpreta,
    // nebo neni co vyrizovat).
    DWORD m_nCommandProcessID;

// fronta notifikaci
    CZUnitNotifyQueueItem *m_pNotifyQueueFirst, *m_pNotifyQueueLast;
    ULONG m_nNotificationsCount;

    // process, ve kterem se vyrizuje (kdyz je == NULL, tak to lezi jeste ve fronte interpreta,
    // nebo neni co vyrizovat).
    DWORD m_nNotifyProcessID;

// metody pro praci s frontama

protected:
    typedef enum { UNKNOWN, NOTIFICATION, CURRENT, MANDATORY } ETargetEventQueue;

protected:
    ETargetEventQueue FindTargetQueue( CZUnit *pSender, EEventType eEventType);

public:
    // Zaradi zpravu do spravne fronty, pripadne ji spusti.
    EInterpretStatus AppendEvent( CZUnit *pSender, CInterpret *pInterpret, EEventType eEventType, 
                                  CStringTableItem *stiEventName, CIBag *pBag, CIWaitingInterface *pWaitingInterface,
                                  bool bFromScript);

	// zkratka s kontrolovanim prikazu pro posilani pomoci sipky "<-"
	// muze volat jen interpret
	EInterpretStatus AppendEventArrow( CInterpret *pInterpret, CStringTableItem *stiEventName, CIBag *pBag);

protected:
    EInterpretStatus AppendCommand( CZUnit *pSender, CInterpret *pInterpret, CZUnitCommandQueueItem *pCommand, ETargetEventQueue eEventQueue, bool bFromScript);
    EInterpretStatus AppendNotification( CZUnit *pSender, CInterpret *pInterpret, CZUnitNotifyQueueItem *pNotification, bool bFromScript);

public:
	bool CanSendCommandToMyselfFromProcess( CIProcess *pProcess);

    // vyprazdnovani front
    void EmptyMandatoryCommandQueue ( bool bSendIdle );
    void EmptyCurrentCommandQueue ( bool bSendIdle );
    void EmptyNotificationQueue();

public:
    // Tohle vola interpret, kdyz na jednotce neco dojede. Jednotka ma moznost
    // spustit vykonani dalsiho prikazu/notifikace.
    void OnEventFinished( CIProcess *pProcess, EEventType eEventType, EInterpretStatus eStatus, 
						  CInterpret *pInterpret);

//
// -> PRO SERVER
//
public:
    // Poslani notifikace jednotce. Vraci false, kdyz nebylo mozno poslat, napr. proto, ze 
	// jednotka je mrtva.
	EInterpretStatus SendNotification( CStringTableItem *stiNotificationName, CIBag *pBag);

    // Pridani noveho podrizeneho. Pokud vrati false, tak jednotka nemuze mit podrizene, nebo kdyz je mrtva.
    bool AddInferior( CZUnit *pInferior);

    // Zmena velitele
    bool ChangeCommander( CZUnit *pNewCommander);


//
//	SAVE & LOAD
//
    static void PersistentSaveStatic( CPersistentStorage &storage);
	static void PersistentLoadStatic( CPersistentStorage &storage);
	static void PersistentTranslatePointersStatic( CPersistentStorage &storage);
	static void PersistentInitStatic();

	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();
};


class CZUnitListItem
{
    DECLARE_MULTITHREAD_POOL( CZUnitListItem);
public:
	CZUnitListItem() {} // pro load&save
    CZUnitListItem( CZUnit *pUnit, CZUnitListItem *pNext = NULL) { m_pUnit = (CZUnit*)pUnit->AddRef(); 
                                                                   m_pNext = pNext; }
    ~CZUnitListItem()  { if (m_pUnit != NULL) m_pUnit->Release(); }

    CZUnit *m_pUnit;
    CZUnitListItem *m_pNext;
};



#endif // !defined(AFX_IUNIT_H__644304B6_A7CA_11D3_AF92_004F49068BD6__INCLUDED_)
