#ifndef __INTERPRETEVENTS_H
#define __INTERPRETEVENTS_H

// NOTIFIERS IDs

#define CIVILIZATION_OR_INTERPRET_NOTIFIER_ID       1 
#define CODEMANAGER_NOTIFIER_ID                     2
#define SKILL_NOTIFIER_ID                           3

// ZPRAVY ZASILANE INTERPRETU OD CIVILIZACE NEBO OD INTERPRETA

// Spusti konstruktor na dane jednotce.
// dwParam == CZUnit*
#define RUN_CONSTRUCTOR 1

// Spusti destruktor na dane jednotce.
// dwParam == CZUnit*
#define RUN_DESTRUCTOR  2

// Spusti se reakce na zpravu na dane jednotce.
// dwParam == CSIRunEventData* dynamicky naalokovana.
// Interpret tuto strukturu uvolnuje.
#define RUN_EVENT       3

// Spusti se globalni funkce.
// dwParam == CIRunGlobalFunctionData* dynamicky alokovana.
// Interpret tuto strukturu uvolnuje.
#define RUN_GLOBAL_FUNCTION 4

// Interpret se zastavi 
// Parametrem je SIPauseIntepret*, ktera obsahuje pointery na eventy.
// Tuto strukturu uvolnuje ten, kdo ji naalokoval.
#define PAUSE_INTERPRET 5

// Zabiti procesu
// Parametrem je SIKillProcessData*.
// Pokud polozka struktury m_bDelete == true, tak tuto strukturu uvolnuje 
// interpret, jinak ten, kdo ji naalokoval.
#define KILL_PROCESS  6

// Zmena skriptu jednotky.
// Parametrem je SIChangeUnitScript*.
// Tuto strukturu uvolnuje ten, kdo ji naalokoval.
#define CHANGE_UNIT_SCRIPT  8

// ZPRAVY ZASILANE INTERPRETU OD CODEMANAGERA

// Update casti kodu jednotky (nekterych metod a 
// eventu). dwParam == SIUpdateUnitCode*.
// Zabije vsechny procesy pouzivaci dane metody
// a eventy, nahodi semafor a bude na nej cekat,
// CodeManager mezitim updatuje kod a nahodi
// semafor, interpret se rozbehne dal.
// Strukturu uvolnuje CodeManager az je vse hotovo.
#define UPDATE_UNIT_CODE    100

// Kompletni update jednotky.
// dwParam == SIUpdateUnitAll*.
// Zabije vsechny procesy pouzivajici danou jednotku,
// pocka na evente, az codemanager udela update
// a jede dal.
// Strukturu uvolnije CodeManager, az je vse hotovo.
#define UPDATE_UNIT_ALL     101

// Update struktury. dwParam == SIUpdateStruct*.
// Zastavi interpret a updatuje datove polozky vsech struktur daneho typu
// v interpretu.
// Strukturu SIUpdateStruct uvolnuje CodeManager az je vse hotovo.
#define UPDATE_STRUCT       102

// Update globalni funkce. dwParam == SIUpdateGlobalFunction*.
// Zasatvi interpret, postrili vsechno, co to pouziva, a necha 
// code manager updatovat kod a zase se rozbehne.
#define UPDATE_GLOBAL_FUNCTION  103

// pro zpravu RUN_EVENT
#include "IRunEventData.h"

// pro zpravu RUN_GLOBAL_FUNCTION

#include "IRunGlobalFunctionData.h"

// pro zpravu UPDATE_UNIT_CODE

// Spojak zmenenych metod a eventu
class CIChangedMethodEventsList
{
    DECLARE_MULTITHREAD_POOL( CIChangedMethodEventsList);
public:
    CStringTableItem *m_stiName; 
    enum { METHOD_CHANGED, METHOD_NEW, METHOD_DELETED,
           EVENT_CHANGED, EVENT_NEW, EVENT_DELETED} m_eAction;
    CIChangedMethodEventsList *m_pNext;

};

// parametr zpravy UPDATE_UNIT_CODE
typedef struct tagIUpdateUnitCode
{
    CIUnitType *m_pIUnitType;
    CIChangedMethodEventsList *m_pChangedMethodEvents;
    bool m_bDestructorChanged;
    bool m_bConstructorChanged;
    CEvent *m_pDoJob;
    CEvent *m_pJobDone;
} SIUpdateUnitCode;

class CIProcess;

// parametr zpravy KILL_PROCESS
typedef struct tagIKillProcessData
{
    unsigned long nProcessID;
    EInterpretStatus Status;
	bool m_bDelete;
	CEvent ProcessKilled;
} SIKillProcessData;


class CIUnitTypeList;

// parametr zpravy UPDATE_UNIT_All
typedef struct tagIUpdateUnitAll
{
    CIUnitType *m_pOldUnitType, *m_pNewUnitType;
    CIUnitTypeList *m_pChildren;
    int m_nHeapOffsetDelta;
    CEvent *m_pDoJob;
    CEvent *m_pJobDone;
} SIUpdateUnitAll;

// parametr zpravy UPDATE_STRUCT
class CIStructureType;

typedef struct tagIUpdateStruct
{
    CIStructureType *m_pOldStructType, *m_pNewStructType;
    CEvent *m_pDoJob;
    CEvent *m_pJobDone;
} SIUpdateStruct;

// parametr zpravy UPDATE_GLOBAL_FUNCTION
typedef struct tagIUpdateGlobalFunction
{
    CStringTableItem *m_stiGlobalFunctionName;
    CEvent *m_pDoJob;
    CEvent *m_pJobDone;
} SIUpdateGlobalFunction;

// parametr zpravy PAUSE_INTERPRET
typedef struct tagIPauseInterpret
{
    CEvent *m_pIntepretPaused;  // interpret ji nastavi, jakmile je zapauzovan
    CEvent *m_pFinishPause;     // civilizace ji nastavi, jakmile se ma interpret zase rozjet
} SIPauseInterpret;

// parametr zpravy CHANGE_UNIT_SCRIPT
typedef struct tagIChangeUnitScript
{
	CZUnit *m_pUnit; // input
	CString m_strScriptName;
	bool m_bSuccess; // output
	CEvent m_Done;
} SIChangeUnitScript;


#endif //__INTERPRETEVENTS_H