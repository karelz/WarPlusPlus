/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace interpretu skriptu ve tride CInterpret.
 * 
 ***********************************************************/

#if !defined(AFX_INTERPRET_H__574E728C_986E_11D3_AF75_004F49068BD6__INCLUDED_)
#define AFX_INTERPRET_H__574E728C_986E_11D3_AF75_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __INTERPRET_TRACE_DEFINED__
#define __INTERPRET_TRACE_DEFINED__
#ifdef _DEBUG
	extern BOOL g_bTraceInterpret;
	extern BOOL g_bTraceInterpretDetails;

	#define TRACE_INTERPRET if ( g_bTraceInterpret ) TRACE_NEXT 
	#define TRACE_INTERPRET0(text) do { if ( g_bTraceInterpret ) { TRACE0_NEXT(text); } } while ( 0 )
	#define TRACE_INTERPRET1(text,p1) do { if ( g_bTraceInterpret ) { TRACE1_NEXT(text,p1); } } while ( 0 )
	#define TRACE_INTERPRET2(text,p1,p2) do { if ( g_bTraceInterpret ) { TRACE2_NEXT(text,p1,p2); } } while ( 0 )
	#define TRACE_INTERPRET3(text,p1,p2,p3) do { if ( g_bTraceInterpret ) { TRACE3_NEXT(text,p1,p2,p3); } } while ( 0 )

	#define TRACE_INTERPRET_DETAILS if ( g_bTraceInterpretDetails ) TRACE_NEXT 
	#define TRACE_INTERPRET_DETAILS0(text) do { if ( g_bTraceInterpretDetails ) { TRACE0_NEXT(text); } } while ( 0 )
	#define TRACE_INTERPRET_DETAILS1(text,p1) do { if ( g_bTraceInterpretDetails ) { TRACE1_NEXT(text,p1); } } while ( 0 )
	#define TRACE_INTERPRET_DETAILS2(text,p1,p2) do { if ( g_bTraceInterpretDetails ) { TRACE2_NEXT(text,p1,p2); } } while ( 0 )
	#define TRACE_INTERPRET_DETAILS3(text,p1,p2,p3) do { if ( g_bTraceInterpretDetails ) { TRACE3_NEXT(text,p1,p2,p3); } } while ( 0 )
    #define TRACE_INTERPRET_DETAILS4(text,p1,p2,p3,p4) do { if ( g_bTraceInterpretDetails ) { TRACE4_NEXT(text,p1,p2,p3,p4); } } while ( 0 )

#else //!_DEBUG
	#define TRACE_INTERPRET TRACE
	#define TRACE_INTERPRET0 TRACE0
	#define TRACE_INTERPRET1 TRACE1
	#define TRACE_INTERPRET2 TRACE2
	#define TRACE_INTERPRET3 TRACE3

	#define TRACE_INTERPRET_DETAILS TRACE
	#define TRACE_INTERPRET_DETAILS0 TRACE0
	#define TRACE_INTERPRET_DETAILS1 TRACE1
	#define TRACE_INTERPRET_DETAILS2 TRACE2
	#define TRACE_INTERPRET_DETAILS3 TRACE3
    #define TRACE_INTERPRET_DETAILS4 TRACE4
#endif //!_DEBUG
#endif //__INTERPRET_TRACE_DEFINED__

#include "Src\InterpretEnums.h"

#include "Src\InterpretEvents.h"

#include "Src\ICodeStringTable.h"
#include "Src\IType.h"
#include "Src\ICode.h"
#include "Src\IMember.h"
#include "Src\IStructureType.h"
#include "Src\IConstructor.h"
#include "Src\IEvent.h"
#include "Src\IMethod.h"
#include "Src\IUnitType.h"
#include "Src\IGlobalFunction.h"

#include "Src\IObjUnitStruct.h"
#include "Src\IStructure.h"

#include "Src\ICodeStackItem.h"
#include "Src\IDataStack.h"
#include "Src\IBag.h"

#include "Src\SysObjs\ISystemObject.h"
#include "Src\SysObjs\ISystemGlobalFunctions.h"
// systemove objekty (SysObjs\SystemObjects.h) se inkluduji jen tam, kde je to potreba

#include "Src\IProcess.h"

// cekani
#include "Src\IWaitingInterface.h"
#include "Src\IConstructorFinishedWaiting.h"
#include "Src\ISendtoWaiting1st.h"
#include "Src\ISendtoWaitingAll.h"
#include "Src\PersistentSaveLoadWaiting.h"

#include "Src\ISkillInterface.h"

extern bool g_bGameLoading;

//
// Spojak ukazatelu na jednotky
//
typedef struct tagCZUnitList
{
    CZUnit *pUnit;
    struct tagCZUnitList *pNext;

} SCZUnitList;

// makra
#ifdef _DEBUG
#define MoveToNextInstruction(instr)    { \
                                            *m_ppCurCode += 1 + g_aInstrInfo[instr].m_nParamCount; \
                                            /* m_pCurProcess->m_pCodeStack->m_nRemainLength -= ((g_aInstrInfo[instr].m_nParamCount + 1) << 2) ; */\
                                        }
#else
#define MoveToNextInstruction(instr)    { \
                                            *m_ppCurCode += 1 + g_aInstrInfo[instr].m_nParamCount; \
                                        }
#endif

#define Param(i)  *(*m_ppCurCode + i)
#define Reg(i) (*m_pCurDataStack)[ m_nCurBorder + i]

//////////////////////////////////////////////////////////

typedef void(CInterpret::*INSTRUCTION_IMPLEMENATION)(void);

class CISOArrayParent;
class CZCivilization;

class CInterpret :  public CNotifier
{
    friend CIProcess;
    friend CZUnit;
	friend CISystemGlobalFunctions;

    DECLARE_OBSERVER_MAP(CInterpret)

// METODY VOLANE ZVENKU
public:
	CInterpret();
	virtual ~CInterpret();

    bool Create(CZCivilization *pCivilization, CEventManager *pEventManager);
    void Delete();

    void Run();

////////////////////
// REAKCE NA ZPRAVY
////////////////////
protected:
    
    // RUN_CONSTRUCTOR
    void OnRunConstructor( CZUnit *pUnit);
    
    // RUN_DESTRUCTOR
    void OnRunDestructor( CZUnit *pUnit);

    // RUN_EVENT
    void OnRunEvent( CIRunEventData *pRunEventData);

    // RUN_GLOBAL_FUNCTION
    void OnRunGlobalFunction(CIRunGlobalFunctionData *pRunGlobalFunctionData);

	// KILL_PROCESS
	void OnKillProcess( SIKillProcessData *pKillProcessData);

    // PAUSE_INTERPRET
public:  // public kvuli volani z RunInterpret v ZCivilization.cpp, vytvari se zapauzovany
    void OnPauseInterpret( SIPauseInterpret *pWaitEvent);

protected:

	// CHANGE_UNIT_SCRIPT
	void OnChangeUnitScript( SIChangeUnitScript *pChangeUnitScript);


    // UPDATE_UNIT_CODE
    void OnUpdateUnitCode( SIUpdateUnitCode *psUpdateUnitCode);

    // UPDATE_UNIT_ALL
    void OnUpdateUnitAll( SIUpdateUnitAll* psUpdateUnitAll);
    bool OnUpdateUnitAll_ProcessUsesOneOfUnits( CIProcess *pProcess, 
                                               CIUnitType *pIUnitType, 
                                               CIUnitTypeList *pChildren);

    // UPDATE_STRUCT
    void OnUpdateStruct( SIUpdateStruct* psUpdateStruct);

    // UPDATE_GLOBAL_FUNCTION
    void OnUpdateGlobalFunction( SIUpdateGlobalFunction* psUpdateGlobalFunction);

/////////////////
// INTERPRETACE
/////////////////

#ifdef _DEBUG
public:
    // interpret thread - pro asserty
    DWORD m_dwInterpretThreadID;
#endif

protected:
    // event manager
    CEventManager *m_pEventManager;

public:
    // code manager
    CCodeManager *m_pCodeManager;

    // civilization
    CZCivilization *m_pCivilization;

protected:
    // stav intepretu
    typedef enum { IDLE, RUNNING } EInterpretState;
    EInterpretState m_nState;

    // prave interpretovany proces 
    CIProcess *m_pCurProcess;
    // prave interpretovany kod procesu
    INSTRUCTION **m_ppCurCode;
    // aktualni border ("tlusta cara")
    int m_nCurBorder;
    // aktualni code stack
    CICodeStackItem *m_pCurCodeStack;
    // aktualni datat stack
    CIDataStack *m_pCurDataStack;

    // pocet instrukci, ktere musi byt interpretovany, aby byl proces preplanovan
    long m_nQuantum;

    // prvni volne process ID
    unsigned long m_nFirstFreeProcessID;

    // fronta cekajicich procesu - obousmerne zretezeny seznam
    CIProcess *m_pFirstReadyToRun;
    CIProcess *m_pLastReadyToRun;

    // uspane procesy - obousmerne zretezeny seznam
    CIProcess *m_pFirstSleeping;

    // pocet procesu
    int m_nProcessCount;

// PROFILE COUNTERS
protected:
	__int64 m_liGlobalProfileCounter;
	__int64 m_liLocalProfileCounter;
    __int64 m_liLastLocalProfileCounter;

public:
    // volano z Code Manageru
    inline __int64 GetLocalProfileCounter()     { return m_liLastLocalProfileCounter; }
    inline __int64 GetGlobalProfileCounter()    { return m_liGlobalProfileCounter; }
    inline void ClearLocalProfileCounter()      { m_liGlobalProfileCounter += m_liLocalProfileCounter; 
                                                  m_liLastLocalProfileCounter = m_liLocalProfileCounter;
                                                  m_liLocalProfileCounter = 0; }
    inline void ClearGlobalAndLocalProfileCounter()     
                                                { m_liGlobalProfileCounter = 0; 
                                                  m_liLastLocalProfileCounter = 0;
                                                  m_liLocalProfileCounter = 0; }
protected:
// PRACE S PROCESY
    void ReadyToRun(CIProcess* pProcess, bool bHighPriority = false);
    void RunNextProcess(bool bCanBeCurrent);

    void Sleep(CIProcess *pProcess, CIWaitingInterface *pWaitingInterface);
    void SleepCurrentProcess( CIWaitingInterface *pWaitingInterface);
    void Wakeup(CIProcess *pProcess, EInterpretStatus eStatus);
    void KillProcess( CIProcess *pProcess, EInterpretStatus eStatus);
    inline void KillCurrentProcess(EInterpretStatus eStatus) { KillProcess( m_pCurProcess, eStatus); }
    void FinishCurrentProcess();

    inline void SetRunningProcess(CIProcess *pProcess, bool bSetQuantum)
    {
        ASSERT(pProcess != NULL);
        m_pCurProcess = pProcess;

        m_ppCurCode = &pProcess->m_pCodeStack->m_pCode;
        m_nCurBorder = pProcess->m_pCodeStack->m_nBorder;
        m_pCurDataStack = &pProcess->m_DataStack;
        m_pCurCodeStack = pProcess->m_pCodeStack;

        if (bSetQuantum) m_nQuantum = MAX_QUANTUM;
    }

    // Tato metoda se vola v pripade, kdyz se pristupuje na jednotku, jejiz
    // konstruktor jeste nedobehl nebo nebyl jeste spusten
    void UnitAccessedAndConstructorNoFinished(CZUnit *pUnit, CIProcess *pProcess);

    // Zabiti vsech procesu, ktere pouzivaji danou instanci jednotky
    void KillAllProcessesOfUnit( CZUnit *pUnit);

// PRACE SE SYSTEMOVYMI OBJEKTY
public:
    CISystemObject* CreateSystemObject(CIType& Type, bool bCallCreate = true);
    CISystemObject* CreateSystemObject(ESystemObjectType, bool bCallCreate = true); // SO_STRING, SO_ARRAY, ...

    CISOArrayParent* CreateArray(CType *pOfType, int nArraySize, bool bCallCreate = true);
    CISystemObject*  CreateSet(CType *pOfType, bool bCallCreateEmptyRoutineAndAddRef);
    CISkillInterface* CreateSkillInterface( CZUnit* pZUnit, CSSkillType *pSkillType);

	// hasovaci tabulka, kde jsou ulozeny vsechny systemove objekty
	CTypedPtrMap<CMapPtrToPtr, CISystemObject*, CISystemObject*> m_tpSystemObjects;
	long m_nSystemObjectCount;
	CCriticalSection m_mutexSystemObjects;
	enum { SYSTEM_OBJECTS_HASH_TABLE_SIZE = 511 };

	void RegisterSystemObject( CISystemObject *pSystemObject);
	void UnregisterSystemObject( CISystemObject *pSystemObject);
	

// PRACE SE STRUKTURAMI
public:

	CIStructure* CreateStructure( CIStructureType *pStructType, bool bCreateMembers);

	// hasovaci tabulka, kde jsou ulozeny vsechny struktury
	CTypedPtrMap<CMapPtrToPtr, CIStructure*, CIStructure*> m_tpStructures;
	long m_nStructureCount;
	CCriticalSection m_mutexStructures;
	enum { STRUCTURES_HASH_TABLE_SIZE = 311 };

	void RegisterStructure( CIStructure *pStructure);
	void UnregisterStructure( CIStructure *pStructure);

public:
    long GetProcessID() { if (m_nFirstFreeProcessID == 0) m_nFirstFreeProcessID++;
						  return m_nFirstFreeProcessID++; }

// hlaseni chyb
    void ReportError(LPCTSTR strErrorMsgFormat, ...);

////////////////
// SYSCALLS
////////////////

	CISystemGlobalFunctions m_SystemGlobalFunctions;

///////////////////////////////////////////////////
// JMENO DEFAULTNI JEDNOTKY 
///////////////////////////////////////////////////

    CStringTableItem *m_stiDefaultUnit;

///////////////////////////
//	SAVE & LOAD
///////////////////////////

	// interpret
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();

	// systemove objekty - PersistentXXX nejsou virtualni -> musi byt
	void PersistentSaveSystemObject( CPersistentStorage &storage, CISystemObject *pSystemObject);
	CISystemObject* PersistentLoadSystemObject( CPersistentStorage &storage);
	void PersistentTranslatePointersSystemObject( CPersistentStorage &storage, CISystemObject *pSystemObject);
	void PersistentInitSystemObject(CISystemObject *pSystemObject);

	// pomocne veci pri loadovani objektu a procesu
	CIProcess **m_ppPersistentLoadProcesses;

///////////////////////////
// IMPLEMENTACE INSTRUKCI
///////////////////////////

    static void (CInterpret::* m_aInstructionImplementations[INSTRUCTION_COUNT])();
                               
    void Do_LOADUM();
    void Do_LOADOM();	
    void Do_LOADSM(); 
    void Do_LOADIX();	
	void Do_STOREUM();
	void Do_STOREOM();
    void Do_STORESM();
	void Do_STOREIX();
	void Do_MOV();		
	void Do_MOV_CINT();	
	void Do_MOV_CBOOL();	
	void Do_MOV_CCHAR();	
	void Do_MOV_CSTR();	
	void Do_MOV_CFLT();	
	void Do_CRTBAG();		
	void Do_DELBAG();		
	void Do_ADDBAG_REG();
	void Do_ADDBAG_CCHAR();
	void Do_ADDBAG_CINT();
	void Do_ADDBAG_CFLT();
	void Do_ADDBAG_CBOOL();	
	void Do_CALLS();			
	void Do_CALLU();			
	void Do_CALLG();			
	void Do_CALLP();			
	void Do_CNV_CHAR2STR();	
	void Do_CNV_INT2CHAR();	
	void Do_CNV_INT2FLT();	
	void Do_CNV_FLT2INT();	
	void Do_CNV_U2U();		
    void Do_DUPL();           
	void Do_INC_CHAR();		
	void Do_INC_INT();		
	void Do_INC_FLT();		
	void Do_DEC_CHAR();		
	void Do_DEC_INT();		
	void Do_DEC_FLT();		
	void Do_INV_INT();		
	void Do_INV_FLT();		
	void Do_NEG();			
	void Do_ADD_CHAR();		
	void Do_ADD_CCHAR();		
	void Do_ADD_INT();		
	void Do_ADD_CINT();		
	void Do_ADD_FLT();		
	void Do_ADD_CFLT();		
	void Do_SUB_CHAR();		
	void Do_SUB_CCHARL();		
	void Do_SUB_CCHARR();		
	void Do_SUB_INT();		
	void Do_SUB_CINTL();		
	void Do_SUB_CINTR();		
	void Do_SUB_FLT();		
	void Do_SUB_CFLTL();		
	void Do_SUB_CFLTR();		
	void Do_MUL_INT();		
	void Do_MUL_CINT();		
	void Do_MUL_FLT();		
	void Do_MUL_CFLT();		
	void Do_DIV_INT();		
	void Do_DIV_CINTL();		
	void Do_DIV_CINTR();		
	void Do_DIV_FLT();		
	void Do_DIV_CFLTL();		
	void Do_DIV_CFLTR();		
	void Do_MOD_INT();		
	void Do_MOD_CINTL();		
	void Do_MOD_CINTR();		
	void Do_LT_CHAR();		
	void Do_LT_CCHAR();		
	void Do_LT_INT();			
	void Do_LT_CINT();		
	void Do_LT_FLT();			
	void Do_LT_CFLT();		
	void Do_LT_STR();			
	void Do_LT_CSTR();		
	void Do_LE_CHAR();		
	void Do_LE_CCHAR();		
	void Do_LE_INT();			
	void Do_LE_CINT();		
	void Do_LE_FLT();			
	void Do_LE_CFLT();		
	void Do_LE_STR();			
	void Do_LE_CSTR();		
	void Do_GT_CHAR();		
	void Do_GT_CCHAR();		
	void Do_GT_INT();			
	void Do_GT_CINT();		
	void Do_GT_FLT();			
	void Do_GT_CFLT();		
	void Do_GT_STR();			
	void Do_GT_CSTR();		
	void Do_GE_CHAR();		
	void Do_GE_CCHAR();		
	void Do_GE_INT();			
	void Do_GE_CINT();		
	void Do_GE_FLT();			
	void Do_GE_CFLT();		
	void Do_GE_STR();			
	void Do_GE_CSTR();		
	void Do_EQ_CHAR();		
	void Do_EQ_CCHAR();		
	void Do_EQ_INT();			
	void Do_EQ_CINT();		
	void Do_EQ_FLT();			
	void Do_EQ_CFLT();		
    void Do_EQ_BOOL();        
    void Do_EQ_CBOOL();       
	void Do_EQ_STR();			
	void Do_EQ_CSTR();		
	void Do_NEQ_CHAR();		
	void Do_NEQ_CCHAR();		
	void Do_NEQ_INT();		
	void Do_NEQ_CINT();		
	void Do_NEQ_FLT();		
	void Do_NEQ_CFLT();		
    void Do_NEQ_BOOL();       
    void Do_NEQ_CBOOL();      
	void Do_NEQ_STR();		
	void Do_NEQ_CSTR();		
	void Do_INSTANCEOF();		
	void Do_LOGOR();			
	void Do_LOGAND();			
	void Do_STRADD();			
	void Do_STRADDCL();		
	void Do_STRADDCR();		
	void Do_JMPF();			
	void Do_JMPT();			
	void Do_JMP();			
	void Do_RET();			
	void Do_RETV();			
	void Do_RETV_CCHAR();		
	void Do_RETV_CINT();		
	void Do_RETV_CFLT();		
	void Do_RETV_CBOOL();		
	void Do_RETV_CSTR();		
	void Do_SENDTO_BEGIN();	
	void Do_SENDTO_END();		
	void Do_SENDTO_WAIT();	
	void Do_SENDTO_WAIT1ST();	
	void Do_SENDTO_NOTIFICATION();	
	void Do_SENDTO_ADDITEM(); 
	void Do_SENDTO_SEND();	
	void Do_SEND();			
    void Do_SET_UN();         
    void Do_SET_IS();         
    void Do_SET_DIF();        
    void Do_SET_ELEM();    
    void Do_SET_ADD();
    void Do_SET_REM();
    void Do_SETFOR_FIRST();   
    void Do_SETFOR_NEXT();    
    void Do_SETFOR_END();     
	void Do_ISNULL();			
	void Do_ISNOTNULL();	
	void Do_ADDBAG_NULL();
	void Do_SETNULL();
	void Do_SETNULLM();

};

#endif // !defined(AFX_INTERPRET_H__574E728C_986E_11D3_AF75_004F49068BD6__INCLUDED_)
