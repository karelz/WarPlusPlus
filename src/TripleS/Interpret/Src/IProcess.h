/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CIProcess reprezentujici jeden process 
 *          interpretovany interpretem.
 *          
 ***********************************************************/

#if !defined(AFX_IPROCESS_H__12901456_A24A_11D3_AF86_004F49068BD6__INCLUDED_)
#define AFX_IPROCESS_H__12901456_A24A_11D3_AF86_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// IDENTIFIKACE NOTIFIERU 

// CIWaitingInterface (jeho potomci jako jsou CISyscall, CIConstructorFinished atp.)
#define WAITING_INTERFACE_NOTIFIER_ID   1

// A ZPRAVY ZASILANE PROCESU

// Cekani skonceno, probud se. 
// dwParam = status (EInterpretStatus)
// Je-li status==STATUS_SUCCESS, pak se vzbud,
// jinak se zabij (s prislusnym statusem).
#define WAITING_FINISHED    1

// prozatimni!!!

class CIWaitingInterface;

class CIProcess : public CObserver
{
    DECLARE_OBSERVER_MAP(CObserver)
    
    DECLARE_MULTITHREAD_POOL( CIProcess);

public:
// TYP PROCESU - konstruktor, destruktor, eventa, globalni funkce
    typedef enum { PROCESS_NONE,
                   PROCESS_CONSTRUCTOR, 
                   PROCESS_DESTRUCTOR, 
                   PROCESS_EVENT_NOTIFICATION, 
                   PROCESS_EVENT_MANDATORY_COMMAND,
                   PROCESS_EVENT_CURRENT_COMMAND,
                   PROCESS_GLOBALFUNCTION,
    } EProcessType;

public:
	CIProcess();
	virtual ~CIProcess();

    // vytvoreni procesu - pro konstruktor a destruktor
    EInterpretStatus CreateConstructorDestructorCall(CInterpret *pInterpret, CZUnit *pUnit, bool bConstructor);

    // vytvoreni procesu - pro eventu
    EInterpretStatus CreateEventCall( CInterpret *pInterpret, CZUnit *pUnit, CIEvent *pEvent, CIBag *pBag, 
									  CIWaitingInterface *pEventFinished, EProcessType eEventType);

    // vytvoreni procesu - pro globalni funkci
    EInterpretStatus CreateGlobalFunctionCall( CInterpret *pInterpret, CIGlobalFunction *pGlobalFunction,
                                               CIBag *pBag);

    // "smazani" objektu
    void Delete( EInterpretStatus eStatus = STATUS_SUCCESS);

// INTERPRETACE - volani procedur (pripravi zasobniky a nastavi kod)

    // volani konstruktoru nebo destruktoru (vetsinou rodice)
    EInterpretStatus CallParentConstructorOrDestructor( bool bConstructor);
    // volani metody
    EInterpretStatus CallMethod( CZUnit *pUnit, CIMethod *pMethod);
    // volani globalni funkce
    EInterpretStatus CallFunction( CIGlobalFunction *pGlobalFunc);

    // navrat z volani metody nebo funkce
    EInterpretStatus Return();

    // nalezeni odesilatele zpravy = jednotky, ktere je to metoda/ktera volala lgobalni funkci ...
    CZUnit* FindSenderUnit();

// UPDATE KODU  - dotazy na to, zda process obsahuje danou instanci

    // dotaz, zda obsahuje instanci metody nebo eventy jednotky daneho typu
    bool ContainsMethodOrEventOfType( CStringTableItem *stiUnitName, CStringTableItem *stiMethodEventName);

    // dotaz, zda obsahuje instanci jednotky daneho typu nebo jejiho potomka
    bool ContainsUnitOfType( CIUnitType *pIUnitType);

	// dotaz, zda obsahuje instanci dané jednotky (zda vola nekterou jeji metodu)
	bool ContainsInstanceOfUnit( CZUnit *pUnit);

// DATOVE POLOZKY
public:
    // interpret
    CInterpret *m_pInterpret;

    // kodovy zasobnik
    CICodeStackItem *m_pCodeStack;
    
    // datovy zasobnik
    CIDataStack m_DataStack;
    
    // jmeno procesu (napr. MyUnit::MOVE)
    CStringTableItem *m_stiName;

    // stav procesu
    typedef enum { CREATED, RUNNING, SLEEPING, DESTROYED } EProcessState;
    EProcessState m_nState;

    // retezeni ve fronte cekajicich nebo spicich procesu (obousmerne spojaky)
    CIProcess *m_pNext, *m_pPrevious;

    // process ID
    unsigned long m_nProcessID;

	// typ procesu
    EProcessType m_nProcessType;

    // jednotka, na ktere se to vola
    // platne pro PROCESS_CONSTRUCTOR, PROCESS_DESTRUCTOR, PROCESS_EVENT_XXX
    CZUnit *m_pUnit;

    // nekdo ceka na dokonceni eventy
    // - platne pro PROCESS_EVENT_XXX_COMMAND
	// - po dokonceni se na tomto vola WaitingFinished
    CIWaitingInterface *m_pEventFinished;

	// Na co process ceka (platne kdyz m_nState == SLEEPING);
	// - pri zabiti ve spanku se na tomto vola Cancel (ukonci cinnost, na kterou jsem cekal)
	CIWaitingInterface *m_pWaitingInterface;

protected:
    // priznak, zda byl process spravne inicializovan
    bool m_bCreated;

public:
//
// SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();
};

#endif // !defined(AFX_IPROCESS_H__12901456_A24A_11D3_AF86_004F49068BD6__INCLUDED_)
