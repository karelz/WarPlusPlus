/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace interpretu skriptu ve tride CInterpret.
 * 
 ***********************************************************/

#include "stdafx.h"
#include "..\Interpret.h"

// systemove objekty
#include "SysObjs\SystemObjects.h"

// code manager
#include "TripleS\CodeManager\CodeManager.h"

// civilizace
#include "GameServer\Civilization\ZCivilization.h"

#include "ISendtoInformation.h"
#include "ISendtoWaitingAll.h"
#include "ISendtoWaiting1st.h"

#include "ISyscall.h"

// POOLY malych objektu, co nemaji vlastni cpp

DEFINE_MULTITHREAD_POOL(CIChangedMethodEventsList, 5);

                                         
//////////////////////////////////////////////////////////////////////
//
// Makra na pridavani a mazani procesu z front
//

#define AddProcessToListAtBegin( listname, process)    \
    {   \
        process->m_pNext = m_pFirst##listname;  \
        process->m_pPrevious = NULL;    \
        if (m_pFirst##listname != NULL) m_pFirst##listname->m_pPrevious = process;  \
        m_pFirst##listname = process;   \
    }

#define AddProcessToListAtBeginWithLastPointer( listname, process)    \
    {   \
        AddProcessToListAtBegin( listname, process)    \
        if (m_pLast##listname == NULL) m_pLast##listname = m_pFirst##listname;  \
    }

#define AddProcessToListAtEnd( listname, process)    \
    {   \
        if (m_pFirst##listname == NULL) \
        {   \
            m_pFirst##listname = m_pLast##listname = process;  \
            process->m_pPrevious = process->m_pNext = NULL;   \
        }   \
        else    \
        {   \
            ASSERT( m_pLast##listname != NULL); \
            process->m_pNext = NULL;   \
            process->m_pPrevious = m_pLast##listname;  \
            m_pLast##listname->m_pNext = process;  \
            m_pLast##listname = process;   \
        }   \
		ASSERT( m_pFirst##listname->m_nState != CIProcess::DESTROYED);	\
    }

#define RemoveProcessFromListWithLastPointer( listname, process)  \
    {   \
        RemoveProcessFromList( listname, process); \
        if (m_pLast##listname == process) m_pLast##listname = process->m_pPrevious;  \
		ASSERT( process->m_nState != CIProcess::DESTROYED);	\
    }

#define RemoveProcessFromList( listname, process)  \
    {   \
        if (process->m_pNext != NULL) process->m_pNext->m_pPrevious = process->m_pPrevious; \
        if (process->m_pPrevious != NULL) process->m_pPrevious->m_pNext = process->m_pNext; \
        if (m_pFirst##listname == process) m_pFirst##listname = process->m_pNext;   \
		ASSERT( process->m_nState != CIProcess::DESTROYED);	\
    }
    
#define RemoveFirstProcessFromList( listname, process)  \
    {   \
        process = m_pFirst##listname;   \
        m_pFirst##listname = m_pFirst##listname->m_pNext;    \
        if (m_pFirst##listname != NULL) m_pFirst##listname->m_pPrevious = NULL; \
        else m_pLast##listname = NULL;  \
		ASSERT( process->m_nState != CIProcess::DESTROYED);	\
    }

//////////////////////////////////////////////////////////////////////
//
// Tabulka textovych popisu statusu
//

char* c_EInterpretStatusString[INTERPRET_STATUS_COUNT] = 
{
	"STATUS_SUCCESS",
    "STATUS_ARGUMENT_TYPE_MISMATCH",
    "STATUS_INVALID_ARGUMENT_COUNT",
    "STATUS_METHOD_NOT_FOUND",
    "STATUS_FUNCTION_NOT_FOUND",
    "STATUS_STRUCTURE_NOT_FOUND",
    "STATUS_MEMBER_NOT_FOUND",
    "STATUS_NO_SUCH_SYSTEM_OBJECT",
    "STATUS_SYSCALL_ARGUMENT_ERROR",
    "STATUS_SYSCALL_ERROR",
    "STATUS_STRING_INDEX_OUT_OF_BOUNDS",  
    "STATUS_ARRAY_INDEX_OUT_OF_BOUNDS",   
    "STATUS_CANNOT_CREATE_SYSTEM_OBJECT", 
    "STATUS_DIVISION_BY_ZERO",
    "STATUS_MODULO_BY_ZERO",
    "STATUS_INVALID_REFERENCE",
    "STATUS_UNIT_CONVERSION_ERROR",
    "STATUS_NESTED_SET_FOR_CYCLE",
    "STATUS_UPDATE",
    "STATUS_INTERPRET_SHUTDOWN",
    "STATUS_COMMAND_NOTIFICATION_MISMATCH",
    "STATUS_COMMAND_DEFFERED",
    "STATUS_EMPTYING_COMMAND_QUEUE",
    "STATUS_PERMISSION_DENIED",
    "STATUS_UNIT_IS_DEAD",
    "STATUS_GAME_CLOSING",				
    "STATUS_SKILL_COLLISION",				
	"STATUS_INFINITE_CYCLE",
	"STATUS_FORBIDDEN_SEND_COMMAND",
    "STATUS_INFINITE_COMMAND_CYCLE",
    "STATUS_INFINITE_NOTIFICATION_CYCLE",

    "STATUS_UNKNOWN_ERROR",
};

CString GetStatusText( EInterpretStatus status)
{
	if (status < INTERPRET_STATUS_COUNT) return CString(c_EInterpretStatusString[status]);
	else return CString("unknown");
}

//////////////////////////////////////////////////////////////////////
// CInterpret message map
//////////////////////////////////////////////////////////////////////

BEGIN_OBSERVER_MAP(CInterpret, CObserver)
    BEGIN_NOTIFIER( CIVILIZATION_OR_INTERPRET_NOTIFIER_ID )   // civilizace
        EVENT( RUN_CONSTRUCTOR)
			TRACE_INTERPRET_DETAILS2( "-- %d: >> RUN_CONSTRUCTOR: %s\n", 
				m_pCivilization->GetCivilizationIndex(),
				(CString)*((CZUnit*)dwParam)->m_pIUnitType->m_stiName);
            OnRunConstructor((CZUnit*)dwParam);
            return true;

        EVENT( RUN_DESTRUCTOR)
			TRACE_INTERPRET_DETAILS2( "-- %d: >> RUN_DESTRUCTOR: %s\n", 
				m_pCivilization->GetCivilizationIndex(),
				(CString)*((CZUnit*)dwParam)->m_pIUnitType->m_stiName);
            OnRunDestructor((CZUnit*)dwParam);
            return true;

        EVENT( RUN_EVENT)
			TRACE_INTERPRET_DETAILS2( "-- %d: >> RUN_EVENT: %s\n", 
				m_pCivilization->GetCivilizationIndex(),
				(CString)*((CIRunEventData*)dwParam)->m_pNotifyOrCommand->m_stiEventName);
            OnRunEvent((CIRunEventData*)dwParam);
            return true;

        EVENT( RUN_GLOBAL_FUNCTION)
			TRACE_INTERPRET_DETAILS2( "-- %d: >> RUN_GLOBAL_FUNCTION: %s\n", 
				m_pCivilization->GetCivilizationIndex(),
				(CString)*((CIRunGlobalFunctionData*)dwParam)->m_stiName);
            OnRunGlobalFunction((CIRunGlobalFunctionData*)dwParam);
            return true;

		EVENT( KILL_PROCESS)
			TRACE_INTERPRET_DETAILS2( "-- %d: >> KILL_PROCESS: ID: %d\n", 
				m_pCivilization->GetCivilizationIndex(),
				((SIKillProcessData*)dwParam)->nProcessID);
			OnKillProcess((SIKillProcessData*)dwParam);
			return true;

        EVENT( PAUSE_INTERPRET)
			TRACE_INTERPRET_DETAILS1( "-- %d: >> PAUSE_INTERPRET\n",
				m_pCivilization->GetCivilizationIndex());
            OnPauseInterpret((SIPauseInterpret*)dwParam);
            return true;

        EVENT( CHANGE_UNIT_SCRIPT)
			TRACE_INTERPRET_DETAILS3( "-- %d: >> CHANGE_UNIT_SCRIPT: %s -> %s\n",
				m_pCivilization->GetCivilizationIndex(),
				(CString)*((SIChangeUnitScript*)dwParam)->m_pUnit->m_pIUnitType->m_stiName,
				(CString)*((SIChangeUnitScript*)dwParam)->m_strScriptName);
            OnChangeUnitScript((SIChangeUnitScript*)dwParam);
            return true;

    END_NOTIFIER()


    BEGIN_NOTIFIER( CODEMANAGER_NOTIFIER_ID)  // code manager

        EVENT( UPDATE_UNIT_CODE);
            OnUpdateUnitCode( (SIUpdateUnitCode*)dwParam);
            return true;

        EVENT( UPDATE_UNIT_ALL);
			OnUpdateUnitAll( (SIUpdateUnitAll*)dwParam);
            return true;

        EVENT( UPDATE_STRUCT);
            OnUpdateStruct( (SIUpdateStruct*)dwParam);
            return true;

        EVENT( UPDATE_GLOBAL_FUNCTION);
            OnUpdateGlobalFunction( (SIUpdateGlobalFunction*) dwParam);
            return true;

        END_NOTIFIER()

END_OBSERVER_MAP(CMyObserver, CObserver)

//////////////////////////////////////////////////////////////////////
// CInterpret::m_aInstructionImplementations
//////////////////////////////////////////////////////////////////////

void (CInterpret::* CInterpret::m_aInstructionImplementations[INSTRUCTION_COUNT])() = {
  CInterpret::Do_LOADUM,        CInterpret::Do_LOADOM,          CInterpret::Do_LOADSM,        
  CInterpret::Do_LOADIX,        CInterpret::Do_STOREUM,         CInterpret::Do_STOREOM,         
  CInterpret::Do_STORESM,       CInterpret::Do_STOREIX,         CInterpret::Do_MOV,           
  CInterpret::Do_MOV_CINT,      CInterpret::Do_MOV_CBOOL,       CInterpret::Do_MOV_CCHAR, 
  CInterpret::Do_MOV_CSTR, 	    CInterpret::Do_MOV_CFLT, 	    CInterpret::Do_CRTBAG, 		
  CInterpret::Do_DELBAG, 		CInterpret::Do_ADDBAG_REG,      CInterpret::Do_ADDBAG_CCHAR,
  CInterpret::Do_ADDBAG_CINT,   CInterpret::Do_ADDBAG_CFLT,     CInterpret::Do_ADDBAG_CBOOL,
  CInterpret::Do_CALLS, 		CInterpret::Do_CALLU, 			CInterpret::Do_CALLG, 			
  CInterpret::Do_CALLP, 	   
  CInterpret::Do_CNV_CHAR2STR,  CInterpret::Do_CNV_INT2CHAR,    CInterpret::Do_CNV_INT2FLT, 	
  CInterpret::Do_CNV_FLT2INT,   CInterpret::Do_CNV_U2U,         CInterpret::Do_DUPL,        
  CInterpret::Do_INC_CHAR,      CInterpret::Do_INC_INT, 		CInterpret::Do_INC_FLT, 		
  CInterpret::Do_DEC_CHAR,      CInterpret::Do_DEC_INT, 	    CInterpret::Do_DEC_FLT,       
  CInterpret::Do_INV_INT, 		
  CInterpret::Do_INV_FLT, 	    CInterpret::Do_NEG, 		    CInterpret::Do_ADD_CHAR, 	
  CInterpret::Do_ADD_CCHAR, 	CInterpret::Do_ADD_INT, 	    CInterpret::Do_ADD_CINT, 		
  CInterpret::Do_ADD_FLT, 	    CInterpret::Do_ADD_CFLT, 	    CInterpret::Do_SUB_CHAR, 	
  CInterpret::Do_SUB_CCHARL, 	CInterpret::Do_SUB_CCHARR, 	    CInterpret::Do_SUB_INT, 		
  CInterpret::Do_SUB_CINTL,     CInterpret::Do_SUB_CINTR, 	    CInterpret::Do_SUB_FLT, 	
  CInterpret::Do_SUB_CFLTL, 	CInterpret::Do_SUB_CFLTR, 	    CInterpret::Do_MUL_INT, 		
  CInterpret::Do_MUL_CINT, 	    CInterpret::Do_MUL_FLT, 	    CInterpret::Do_MUL_CFLT, 	
  CInterpret::Do_DIV_INT, 	    CInterpret::Do_DIV_CINTL, 	    CInterpret::Do_DIV_CINTR, 		
  CInterpret::Do_DIV_FLT, 	    CInterpret::Do_DIV_CFLTL, 	    CInterpret::Do_DIV_CFLTR, 	
  CInterpret::Do_MOD_INT, 	    CInterpret::Do_MOD_CINTL, 		CInterpret::Do_MOD_CINTR, 		
  CInterpret::Do_LT_CHAR, 	    CInterpret::Do_LT_CCHAR, 	    CInterpret::Do_LT_INT, 			
  CInterpret::Do_LT_CINT, 	    CInterpret::Do_LT_FLT, 		    CInterpret::Do_LT_CFLT, 	
  CInterpret::Do_LT_STR, 		CInterpret::Do_LT_CSTR, 		CInterpret::Do_LE_CHAR, 		
  CInterpret::Do_LE_CCHAR, 	    CInterpret::Do_LE_INT, 		    CInterpret::Do_LE_CINT, 	
  CInterpret::Do_LE_FLT, 		CInterpret::Do_LE_CFLT, 		CInterpret::Do_LE_STR, 			
  CInterpret::Do_LE_CSTR,       CInterpret::Do_GT_CHAR, 	    CInterpret::Do_GT_CCHAR, 	
  CInterpret::Do_GT_INT, 		CInterpret::Do_GT_CINT, 	    CInterpret::Do_GT_FLT, 			
  CInterpret::Do_GT_CFLT, 	    CInterpret::Do_GT_STR, 		    CInterpret::Do_GT_CSTR, 	
  CInterpret::Do_GE_CHAR, 	    CInterpret::Do_GE_CCHAR, 		CInterpret::Do_GE_INT, 			
  CInterpret::Do_GE_CINT, 	    CInterpret::Do_GE_FLT, 		    CInterpret::Do_GE_CFLT, 	
  CInterpret::Do_GE_STR, 		CInterpret::Do_GE_CSTR, 		CInterpret::Do_EQ_CHAR, 		
  CInterpret::Do_EQ_CCHAR, 	    CInterpret::Do_EQ_INT, 		    CInterpret::Do_EQ_CINT, 	
  CInterpret::Do_EQ_FLT, 		CInterpret::Do_EQ_CFLT, 		CInterpret::Do_EQ_BOOL,         
  CInterpret::Do_EQ_CBOOL,      CInterpret::Do_EQ_STR, 		    CInterpret::Do_EQ_CSTR, 	
  CInterpret::Do_NEQ_CHAR, 	    CInterpret::Do_NEQ_CCHAR, 	    CInterpret::Do_NEQ_INT, 		
  CInterpret::Do_NEQ_CINT, 	    CInterpret::Do_NEQ_FLT, 	    CInterpret::Do_NEQ_CFLT, 	
  CInterpret::Do_NEQ_BOOL,      CInterpret::Do_NEQ_CBOOL,       CInterpret::Do_NEQ_STR, 		
  CInterpret::Do_NEQ_CSTR, 	    CInterpret::Do_INSTANCEOF, 	    CInterpret::Do_LOGOR, 		
  CInterpret::Do_LOGAND, 		CInterpret::Do_STRADD, 		    CInterpret::Do_STRADDCL, 		
  CInterpret::Do_STRADDCR, 	    CInterpret::Do_JMPF, 		    CInterpret::Do_JMPT, 		
  CInterpret::Do_JMP, 		    CInterpret::Do_RET, 		    CInterpret::Do_RETV, 			
  CInterpret::Do_RETV_CCHAR,    CInterpret::Do_RETV_CINT, 	    CInterpret::Do_RETV_CFLT,   
  CInterpret::Do_RETV_CBOOL,    CInterpret::Do_RETV_CSTR, 		CInterpret::Do_SENDTO_BEGIN, 	
  CInterpret::Do_SENDTO_END,    CInterpret::Do_SENDTO_WAIT,     CInterpret::Do_SENDTO_WAIT1ST,
  CInterpret::Do_SENDTO_NOTIFICATION, 
  CInterpret::Do_SENDTO_ADDITEM,  CInterpret::Do_SENDTO_SEND, 	            
  CInterpret::Do_SEND,          CInterpret::Do_SET_UN,          CInterpret::Do_SET_IS,          
  CInterpret::Do_SET_DIF,       CInterpret::Do_SET_ELEM,        CInterpret::Do_SET_ADD,
  CInterpret::Do_SET_REM,       CInterpret::Do_SETFOR_FIRST,    CInterpret::Do_SETFOR_NEXT,   
  CInterpret::Do_SETFOR_END,    CInterpret::Do_ISNULL,          CInterpret::Do_ISNOTNULL,
  CInterpret::Do_ADDBAG_NULL,   CInterpret::Do_SETNULL,			CInterpret::Do_SETNULLM,
};

//////////////////////////////////////////////////////////////////////
// CInterpret - implementace instrukci
//////////////////////////////////////////////////////////////////////

void CInterpret::Do_LOADUM() 
{ 
    CZUnit *pUnit = Reg(Param(2)).GetUnit();
    if ( pUnit == NULL) 
    {
        KillCurrentProcess(STATUS_INVALID_REFERENCE);
        return;
    }

    if (pUnit->m_pConstructorFinished != NULL && 
        !(m_pCurProcess->m_nProcessType == CIProcess::PROCESS_CONSTRUCTOR  && 
         m_pCurProcess->m_pUnit == pUnit)) 
    {
        UnitAccessedAndConstructorNoFinished( pUnit, m_pCurProcess);
        return;
    }

    if (!pUnit->LoadMember( Reg(Param(1)), 
        (*m_pCurCodeStack->m_pCodeStringTable)[Param(3)],
        m_pCivilization != pUnit->m_pCivilization))
    {
        KillCurrentProcess(STATUS_MEMBER_NOT_FOUND);
		return;
    }

    MoveToNextInstruction(LOADUM);
}

//////////////////////////////////////////////////////////////////////

void CInterpret::Do_LOADOM() 
{ 
	CISystemObject *pSystemObject = Reg(Param(2)).GetSystemObject();

	if (pSystemObject == NULL)
	{
		KillCurrentProcess( STATUS_INVALID_REFERENCE);
		return;
	}

	if (!(pSystemObject->LoadMember( Reg(Param(1)), Param(3))))
	{
		KillCurrentProcess(STATUS_MEMBER_NOT_FOUND);
		return;
	}

	MoveToNextInstruction(LOADOM);
}	

//////////////////////////////////////////////////////////////////////

void CInterpret::Do_LOADSM() 
{ 
	CIStructure *pStruct = Reg(Param(2)).GetStruct();

	if (pStruct == NULL)
	{
		 KillCurrentProcess( STATUS_INVALID_REFERENCE);
		 return;
	}

	if (!Reg(Param(2)).GetStruct()->LoadMember( Reg(Param(1)), 
						(*m_pCurCodeStack->m_pCodeStringTable)[Param(3)]))
	{
		KillCurrentProcess(STATUS_MEMBER_NOT_FOUND);
		return;
	}
	
	MoveToNextInstruction(LOADSM);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LOADIX() 
{ 
	CISystemObject *pSystemObject = Reg(Param(2)).GetSystemObject();

	if (pSystemObject == NULL)
	{
		KillCurrentProcess( STATUS_INVALID_REFERENCE);
		return;
	}

	ASSERT(Reg(Param(2)).IsArray() || Reg(Param(2)).IsString());

	// Array
	if (Reg(Param(2)).IsArray())
	{
		if (((CISOArrayParent*)Reg(Param(2)).GetSystemObject())->GetAt( Reg(Param(3)).GetInt(), 
																	Reg(Param(1))))
		{
			MoveToNextInstruction(LOADIX);
		}
		else KillCurrentProcess(STATUS_ARRAY_INDEX_OUT_OF_BOUNDS);
	}
	// String
	else 
	{
		char c;
		if (((CISOString*)Reg(Param(2)).GetSystemObject())->GetAt( Reg(Param(3)).GetInt(), c))
		{
			Reg(Param(1)).Set(c);
			MoveToNextInstruction(LOADIX);
		}
		else KillCurrentProcess(STATUS_STRING_INDEX_OUT_OF_BOUNDS);
	}
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_STOREUM() 
{ 
    CZUnit *pUnit = Reg(Param(2)).GetUnit();

    if (pUnit == NULL) 
    {
        KillCurrentProcess(STATUS_INVALID_REFERENCE);
        return;
    }

    if (pUnit->m_pConstructorFinished != NULL &&  
        !(m_pCurProcess->m_nProcessType == CIProcess::PROCESS_CONSTRUCTOR  && 
         m_pCurProcess->m_pUnit == pUnit)) 
    {
        UnitAccessedAndConstructorNoFinished( pUnit, m_pCurProcess);
        return;
    }

    if (!Reg(Param(2)).GetUnit()->StoreMember( &Reg(Param(1)), 
                        (*m_pCurCodeStack->m_pCodeStringTable)[Param(3)],
                        m_pCivilization != pUnit->m_pCivilization))
    {
        KillCurrentProcess(STATUS_MEMBER_NOT_FOUND);
    }
    else MoveToNextInstruction(STOREUM);
}
 
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_STOREOM() 
{ 
	 CISystemObject *pSystemObject = Reg(Param(2)).GetSystemObject();

	 if ( pSystemObject == NULL)
	 {
		 KillCurrentProcess( STATUS_INVALID_REFERENCE);
		 return;
	 }

	 if ( ! Reg(Param(2)).GetSystemObject()->StoreMember( &Reg(Param(1)), Param(3)))
	 {
		  KillCurrentProcess(STATUS_MEMBER_NOT_FOUND);
		  return;
	 }
	 
	 MoveToNextInstruction(STOREOM);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_STORESM() 
{                        
	CIStructure *pStruct = Reg(Param(2)).GetStruct();

	if (pStruct == NULL)
	{
		 KillCurrentProcess( STATUS_INVALID_REFERENCE);
		 return;
	}

	if (!pStruct->StoreMember( &Reg(Param(1)), 
              (*m_pCurCodeStack->m_pCodeStringTable)[Param(3)]))
	{
		KillCurrentProcess(STATUS_MEMBER_NOT_FOUND);
		return;
	}
	
	MoveToNextInstruction(STORESM);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_STOREIX()    
{ 
	CISystemObject *pSystemObject = Reg(Param(2)).GetSystemObject();

	if (pSystemObject == NULL)
	{
		KillCurrentProcess( STATUS_INVALID_REFERENCE);
		return;
	}

	ASSERT(Reg(Param(2)).IsArray() || Reg(Param(2)).IsString());

	// Array
	if (Reg(Param(2)).IsArray())
	{
		if (((CISOArrayParent*)pSystemObject)->SetAt( Reg(Param(3)).GetInt(), 
													  &Reg(Param(1))))
		{
			MoveToNextInstruction(LOADIX);
		}
		else KillCurrentProcess(STATUS_ARRAY_INDEX_OUT_OF_BOUNDS);
	}
	// String
	else 
	{
		if (((CISOString*)pSystemObject)->SetAt( Reg(Param(3)).GetInt(), 
			Reg(Param(1)).GetChar()))
		{
			MoveToNextInstruction(STOREIX);
		}
		else KillCurrentProcess(STATUS_STRING_INDEX_OUT_OF_BOUNDS);
	}
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_MOV() 
{ 
	ASSERT( Param(1) != Param(2));
	Reg(Param(1)) = Reg(Param(2));

    MoveToNextInstruction(MOV);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_MOV_CINT()	
{ 
	Reg(Param(1)).Set( (int)Param(2) );

    MoveToNextInstruction(MOV_CINT);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_MOV_CBOOL()		
{ 
	Reg(Param(1)).Set( Param(2)!=0 );

    MoveToNextInstruction(MOV_CBOOL);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_MOV_CCHAR()	
{ 
	Reg(Param(1)).Set( (char)Param(2) );

    MoveToNextInstruction(MOV_CCHAR);
}
	
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_MOV_CSTR()
{ 
    CISOString *pString;

    // konstantni string
    CStringTableItem* stiString = (*m_pCurCodeStack->m_pCodeStringTable)[Param(2)];
    ASSERT(stiString != NULL);

    // vytvoreni systemoveho objektu string
    pString = (CISOString*)CreateSystemObject(SO_STRING);
    ASSERT(pString != NULL);

    pString->Set( (CString)*stiString );

    Reg(Param(1)).Set( pString);

    pString->Release();

    MoveToNextInstruction(MOV_CSTR);
}
	
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_MOV_CFLT()	
{ 
	Reg(Param(1)).Set( (*m_pCurCodeStack->m_pFloatTable)[ Param(2) ] );

    MoveToNextInstruction(MOV_CFLT);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_CRTBAG()	
{ 
    CIBag *pBag = new CIBag;
    pBag->Create();
    pBag->m_pPreviousBag = m_pCurCodeStack->m_pBag;
    m_pCurCodeStack->m_pBag = pBag;

    MoveToNextInstruction(CRTBAG);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_DELBAG()	
{ 
    CIBag *pBag = m_pCurCodeStack->m_pBag;
    ASSERT(pBag != NULL);

    m_pCurCodeStack->m_pBag = pBag->m_pPreviousBag;

    pBag->Delete();
    delete pBag;

    MoveToNextInstruction(DELBAG);
}

//////////////////////////////////////////////////////////////////////

void CInterpret::Do_ADDBAG_REG()	
{ 
	ASSERT(m_pCurCodeStack->m_pBag != NULL);

    m_pCurCodeStack->m_pBag->AddRegister(Reg(Param(1)));
    MoveToNextInstruction(ADDBAG_REG);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_ADDBAG_CCHAR()	
{ 
	ASSERT(m_pCurCodeStack->m_pBag != NULL);

    m_pCurCodeStack->m_pBag->AddConstantChar((char)Param(1));
    MoveToNextInstruction(ADDBAG_CCHAR);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_ADDBAG_CINT() 	
{ 
	ASSERT(m_pCurCodeStack->m_pBag != NULL);

    m_pCurCodeStack->m_pBag->AddConstantInt((int)Param(1));
    MoveToNextInstruction(ADDBAG_CINT);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_ADDBAG_CFLT()	
{ 
	ASSERT(m_pCurCodeStack->m_pBag != NULL);

    m_pCurCodeStack->m_pBag->AddConstantFloat( (*m_pCurCodeStack->m_pFloatTable)[Param(1)] );
    MoveToNextInstruction(ADDBAG_CFLT);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_ADDBAG_CBOOL()		
{ 
	ASSERT(m_pCurCodeStack->m_pBag != NULL);

    m_pCurCodeStack->m_pBag->AddConstantBool(Param(1)==0 ? false : true);
    MoveToNextInstruction(ADDBAG_CBOOL);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_CALLS()	
{ 
	ASSERT(m_pCurCodeStack->m_pBag != NULL);

    // navratovy registr
    if (Param(1) != 0) m_pCurCodeStack->m_pBag->SetReturnRegister(&Reg(Param(1)));

    // GLOBALNI FUNKCE
    if (Param(2) == 0)
    {
		CISyscall *pSyscall = NULL;
		int nID = Param(3);

        MoveToNextInstruction(CALLS);

        switch ( m_SystemGlobalFunctions.InvokeFunction( m_pCurProcess, nID, m_pCurCodeStack->m_pBag,
														 &pSyscall))
        {
        case SYSCALL_SUCCESS:
            break;

        case SYSCALL_SUCCESS_SLEEP:
			ASSERT( pSyscall != NULL);
            SleepCurrentProcess( pSyscall);
			pSyscall->Release();
            break;

        case SYSCALL_ARGUMENT_ERROR:
            KillCurrentProcess(STATUS_SYSCALL_ARGUMENT_ERROR);
            break;

        case SYSCALL_ERROR:
            KillCurrentProcess(STATUS_SYSCALL_ERROR);
			break;

		default:
			ASSERT(false);
        }
    }
    else
    // METODA SYSTEMOVEHO OBJEKTU
    {
		  CISyscall *pSyscall = NULL;
		  CISystemObject *pSystemObject = Reg(Param(2)).GetSystemObject();

		  if (pSystemObject == NULL)
		  {
			  KillCurrentProcess( STATUS_INVALID_REFERENCE);
		  }
		  else
		  {
			  switch (pSystemObject->InvokeMethod( m_pCurProcess, Param(3), m_pCurCodeStack->m_pBag, &pSyscall))
			  {
			  case SYSCALL_SUCCESS:
					MoveToNextInstruction(CALLS);
					break;

			  case SYSCALL_SUCCESS_SLEEP:
					MoveToNextInstruction(CALLS);

					ASSERT( pSyscall != NULL);
					SleepCurrentProcess( pSyscall);
					pSyscall->Release();
					break;

			  case SYSCALL_ARGUMENT_ERROR:
					KillCurrentProcess(STATUS_SYSCALL_ARGUMENT_ERROR);
					break;

			  case SYSCALL_ERROR:
					KillCurrentProcess(STATUS_SYSCALL_ERROR);
					break;

			  default:
				   ASSERT(false);

			  }
		  }
    }
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_CALLU()	
{                                 
    // jednotka
    CZUnit *pUnit = Reg(Param(2)).GetUnit();
    CIUnitType *pIUnitType;
    CIMethod *pMethod;
    EInterpretStatus st;

    if (pUnit == NULL) 
    {
        KillCurrentProcess(STATUS_INVALID_REFERENCE);
        return;
    }

	pIUnitType = pUnit->m_pIUnitType;

    // bylo na jednotce uz dokonceno volani konstruktoru?
    if (pUnit->m_pConstructorFinished != NULL &&  
        !(m_pCurProcess->m_nProcessType == CIProcess::PROCESS_CONSTRUCTOR  && 
         m_pCurProcess->m_pUnit == pUnit)) 
    {
        UnitAccessedAndConstructorNoFinished( pUnit, m_pCurProcess);
        return;
    }

    // navratovy registr
    if (Param(1) != 0) m_pCurCodeStack->m_pBag->SetReturnRegister(&Reg(Param(1)));

    // metoda
    CStringTableItem *stiName = (*m_pCurCodeStack->m_pCodeStringTable)[Param(3)];
    
    switch (pIUnitType->GetMethod(stiName, pMethod))
    {
    case IMPL_NONE:
        ReportError( "Unit \"%s\" has no method called %s.", (CString)*pIUnitType->m_stiName, (CString)*stiName);
        KillCurrentProcess(STATUS_METHOD_NOT_FOUND);
        break;

    case IMPL_SCRIPT:                     
        MoveToNextInstruction(CALLU);

        if ((st = m_pCurProcess->CallMethod(pUnit, pMethod)) != STATUS_SUCCESS)
        {
            KillCurrentProcess(st);
        }
        else 
        {
            TRACE_INTERPRET_DETAILS("-- %d: %3d: Now in method %s::%s.\n", m_pCivilization->GetCivilizationIndex(),
                m_pCurProcess->m_nProcessID, (CString)*pIUnitType->m_stiName, (CString)*stiName);
            SetRunningProcess(m_pCurProcess, false);
        }
        break;

    case IMPL_CPLUSPLUS:

		MoveToNextInstruction(CALLU);

		if ( ( st = pUnit->InvokeMethod(m_pCurProcess, stiName, m_pCurCodeStack->m_pBag)) != STATUS_SUCCESS)
		{
			KillCurrentProcess(st);
		}
    }
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_CALLG()	
{ 
    CIGlobalFunction *pGlobalFunc = NULL;
    EInterpretStatus st;
    
    // jmeno funkce
    CStringTableItem *stiName = (*m_pCurCodeStack->m_pCodeStringTable)[Param(2)];

    // navratovy registr
    if (Param(1) != 0) m_pCurCodeStack->m_pBag->SetReturnRegister(&Reg(Param(1)));

    MoveToNextInstruction(CALLG);

    if ((pGlobalFunc = m_pCodeManager->GetGlobalFunction(stiName)) != NULL)
    {
        if (( st = m_pCurProcess->CallFunction(pGlobalFunc)) != STATUS_SUCCESS)
        {
            KillCurrentProcess( st);
        }
        else
        {
            TRACE_INTERPRET_DETAILS3("-- %d: %3d: Now in global function %s.\n", m_pCivilization->GetCivilizationIndex(),
                m_pCurProcess->m_nProcessID, (CString)*stiName);
            SetRunningProcess(m_pCurProcess, false);
        }
    }
    else
    {
        ReportError( "Can't find global function called %s.", (CString)*stiName);
        KillCurrentProcess(STATUS_FUNCTION_NOT_FOUND);
    }
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_CALLP()	
{ 
    EInterpretStatus st;

    ASSERT( Reg(0).IsUnit());
    ASSERT( Reg(0).GetUnit() != NULL);
    ASSERT( Reg(0).GetUnitType() != NULL);

    // KONSTRUKTOR nebo DESTRUKTOR
    if (Param(2) == -1)
    {
        MoveToNextInstruction(CALLP);
        if ((st = m_pCurProcess->CallParentConstructorOrDestructor(true)) == STATUS_SUCCESS)
        {
#ifdef _DEBUG
            if (Reg(0).GetUnitType()->m_pParent->m_stiName != m_stiDefaultUnit)
            {
                TRACE_INTERPRET_DETAILS3("-- %d: %3d: Now in %s constructor.\n", m_pCivilization->GetCivilizationIndex(),
                    m_pCurProcess->m_nProcessID, (CString)*Reg(0).GetUnitType()->m_pParent->m_stiName);
            }
#endif //_DEBUG
            SetRunningProcess(m_pCurProcess, false);
        }
        else 
        {
            ReportError( "Can't call constructor of unit \"%s\".",  (CString)*Reg(0).GetUnitType()->m_pParent->m_stiName);
            KillCurrentProcess(st);
        }
    }
    else if (Param(2) == -2 )
    {
        MoveToNextInstruction(CALLP);
        if ( (st = m_pCurProcess->CallParentConstructorOrDestructor(false)) == STATUS_SUCCESS)
        {
            TRACE_INTERPRET_DETAILS3("-- %d: %3d: Now in %s destructor.", m_pCivilization->GetCivilizationIndex(),
                m_pCurProcess->m_nProcessID, (CString)*Reg(0).GetUnit()->m_pIUnitType->m_pParent->m_stiName);
            SetRunningProcess(m_pCurProcess, false);
        }
        else 
        {
            ReportError( "Can't call destructor of unit \"%s\".\n", 
                (CString)*Reg(0).GetUnitType()->m_pParent->m_stiName);
            KillCurrentProcess(st);
        }
    }
    // NECO JINEHO
    else
    {   
        CZUnit *pUnit;
        CIUnitType *pParent;
        CIMethod *pMethod;

        ASSERT(Reg(0).IsUnit());
        pUnit = Reg(0).GetUnit();

        ASSERT(pUnit != NULL);

        pParent = pUnit->m_pIUnitType->m_pParent;
        // nesmi byt defaultni jednotka, ta nema predka
        ASSERT(pParent != NULL);

        // navratovy registr
        if (Param(1) != 0) m_pCurCodeStack->m_pBag->SetReturnRegister(&Reg(Param(1)));

        // metoda
        CStringTableItem *stiName = (*m_pCurCodeStack->m_pCodeStringTable)[Param(2)];

        switch (pParent->GetMethod( stiName, pMethod))
        {
        case IMPL_NONE:
            ReportError( "Unit \"%s\" has no method called %s.", (CString)*pParent->m_stiName, (CString)Param(2));
            KillCurrentProcess(STATUS_METHOD_NOT_FOUND);
            break;

        case IMPL_SCRIPT:
            MoveToNextInstruction(CALLP);

            if (( st = m_pCurProcess->CallMethod(pUnit, pMethod)) != STATUS_SUCCESS)
            {
                KillCurrentProcess(st);
            }
            else 
            {
                TRACE_INTERPRET_DETAILS("-- %d: %3d: Now in method %s::%s.\n", m_pCivilization->GetCivilizationIndex(),
                    m_pCurProcess->m_nProcessID, (CString)*pParent->m_stiName, (CString)*stiName);
                SetRunningProcess(m_pCurProcess, false);
            }
            break;

        case IMPL_CPLUSPLUS:
			MoveToNextInstruction(CALLU);

			if ( ( st = pUnit->InvokeMethod(m_pCurProcess, stiName, m_pCurCodeStack->m_pBag)) != STATUS_SUCCESS)
			{
				KillCurrentProcess(st);
			}
		}
    }
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_CNV_CHAR2STR()	
{ 
	ASSERT( Reg(Param(2)).IsChar());

    CISOString *pString;
    
    if ((pString = (CISOString*)CreateSystemObject(SO_STRING)) == NULL) 
	{
		KillCurrentProcess(STATUS_CANNOT_CREATE_SYSTEM_OBJECT);
	}                                                          
	else
	{
		pString->Set( (char)Reg(Param(2)).GetChar() );
		Reg(Param(1)).Set( pString);
		pString->Release();

	    MoveToNextInstruction(CNV_CHAR2STR);
	}
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_CNV_INT2CHAR()	
{ 
	Reg(Param(1)).Set( (char) Reg(Param(2)).GetInt());
    MoveToNextInstruction(CNV_INT2CHAR);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_CNV_INT2FLT()	
{ 
	Reg(Param(1)).Set( (double) Reg(Param(2)).GetInt());
    MoveToNextInstruction(CNV_INT2FLT);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_CNV_FLT2INT()	
{ 
	Reg(Param(1)).Set( (int) Reg(Param(2)).GetFloat());
    MoveToNextInstruction(CNV_FLT2INT);
}

//////////////////////////////////////////////////////////////////////

void CInterpret::Do_CNV_U2U()
{ 
    CStringTableItem *stiDestUnitName = (*m_pCurCodeStack->m_pCodeStringTable)[Param(3)];
    CZUnit *pSrcUnit = Reg(Param(2)).GetUnit();

    if (pSrcUnit == NULL)
	{
		Reg(Param(1)).Set( NULL, NULL);
		MoveToNextInstruction(CNV_U2U);
		return;
	}
		
	CIUnitType *pUnitType = pSrcUnit->m_pIUnitType;

    while (pUnitType != NULL  &&  stiDestUnitName != pUnitType->m_stiName)
    {
        pUnitType = pUnitType->m_pParent;
    }

    if (pUnitType == NULL)
    {
        KillCurrentProcess(STATUS_UNIT_CONVERSION_ERROR);
    }
    else
    {
        Reg(Param(1)).Set( pSrcUnit, pUnitType);
    
        MoveToNextInstruction(CNV_U2U);
    }
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_DUPL()	
{ 
	ASSERT( Reg(Param(2)).IsSystemObject() || Reg(Param(2)).IsStruct() );

	// STRUCT
	if ( Reg(Param(2)).IsStruct())
	{
		CIStructure *pStruct = Reg( Param(2)).GetStruct();
		CIStructure *pDestStruct;

		if (pStruct == NULL)
		{
			KillCurrentProcess( STATUS_INVALID_REFERENCE);
			return;
		}

	    pDestStruct = CreateStructure( pStruct->m_pStructType, false);
		pDestStruct->CopyDataFrom( pStruct);

		Reg(Param(1)).Set( pDestStruct);
		pDestStruct->Release();

		MoveToNextInstruction( DUPL);
	}
	// OBJECT
	else
	{
		CISystemObject *pSrc = Reg(Param(2)).GetSystemObject();
		CIDuplicSystemObject *pDest;

		if (pSrc == NULL)
		{
			KillCurrentProcess( STATUS_INVALID_REFERENCE);
			return;
		}

		ASSERT( pSrc->CanBeDuplicated());

		switch (pSrc->GetObjectType())
		{
		case SO_ARRAY:
			pDest = (CIDuplicSystemObject*)CreateArray( &((CISOArrayParent*)pSrc)->m_DataType, 1, false);
			break;
		case SO_SET:
			pDest = (CIDuplicSystemObject*)CreateSet( &((CISOArrayParent*)pSrc)->m_DataType, false);
			break;
		default:
			pDest = (CIDuplicSystemObject*)CreateSystemObject( pSrc->GetObjectType(), false);
			break;
		}

		if (pDest == NULL)
		{
			KillCurrentProcess(STATUS_CANNOT_CREATE_SYSTEM_OBJECT);
		}
		else
		{
            if (!pDest->CreateFrom( this, (CIDuplicSystemObject*)pSrc))
			{
				pDest->Release();
				KillCurrentProcess(STATUS_CANNOT_CREATE_SYSTEM_OBJECT);
			}
			else
			{
				Reg(Param(1)).Set(pDest);

				pDest->Release();

				MoveToNextInstruction(DUPL);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_INC_CHAR()	
{
	Reg(Param(1)).IncreaseChar();
    MoveToNextInstruction(INC_CHAR);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_INC_INT()
{ 
	Reg(Param(1)).IncreaseInt();
    MoveToNextInstruction(INC_INT);
}
	
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_INC_FLT()
{ 
	Reg(Param(1)).IncreaseFloat();
    MoveToNextInstruction(INC_FLT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_DEC_CHAR()	
{ 
	Reg(Param(1)).DecreaseChar();
    MoveToNextInstruction(DEC_CHAR);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_DEC_INT()
{ 
	Reg(Param(1)).DecreaseInt();
    MoveToNextInstruction(DEC_INT);
}
	
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_DEC_FLT()
{ 
	Reg(Param(1)).DecreaseFloat();
    MoveToNextInstruction(DEC_FLT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_INV_INT()	
{ 
	Reg(Param(1)).InvertInt();
    MoveToNextInstruction(INV_INT);
}

//////////////////////////////////////////////////////////////////////

void CInterpret::Do_INV_FLT()	
{ 
	Reg(Param(1)).InvertFloat();
    MoveToNextInstruction(INV_FLT);
}
	
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_NEG()	
{ 
	Reg(Param(1)).NegateBool();
    MoveToNextInstruction(NEG);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_ADD_CHAR()	
{ 
    Reg(Param(1)).Set( (char)(Reg(Param(2)).GetChar() + Reg(Param(3)).GetChar()));
    MoveToNextInstruction(ADD_CHAR);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_ADD_CCHAR()	
{ 
    Reg(Param(1)).Set( (char)(Reg(Param(2)).GetChar() + (char)Param(3)));
    MoveToNextInstruction(ADD_CCHAR);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_ADD_INT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetInt() + Reg(Param(3)).GetInt());
    MoveToNextInstruction(ADD_INT);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_ADD_CINT()			
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetInt() + (char)Param(3) );
    MoveToNextInstruction(ADD_CINT);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_ADD_FLT()		
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() + Reg(Param(3)).GetFloat());
    MoveToNextInstruction(ADD_FLT);
}
	
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_ADD_CFLT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() + 0 );
    MoveToNextInstruction(ADD_CFLT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SUB_CHAR()	
{ 
    Reg(Param(1)).Set( (char)((Reg(Param(2)).GetChar() - Reg(Param(3)).GetChar())) );
    MoveToNextInstruction(SUB_CHAR);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SUB_CCHARL()	
{ 
    Reg(Param(1)).Set( (char)((char)Param(3) - Reg(Param(2)).GetChar()) );
    MoveToNextInstruction(SUB_CCHARL);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SUB_CCHARR()	
{ 
    Reg(Param(1)).Set( (char)(Reg(Param(2)).GetChar() - (char)Param(3)) );
    MoveToNextInstruction(SUB_CCHARR);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SUB_INT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetInt() - Reg(Param(3)).GetInt() );
    MoveToNextInstruction(SUB_INT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SUB_CINTL()	
{ 
    Reg(Param(1)).Set( (int)Param(3) - Reg(Param(2)).GetInt() );
    MoveToNextInstruction(SUB_CINTL);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SUB_CINTR()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetInt() - (int)Param(3) );
    MoveToNextInstruction(SUB_CINTR);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SUB_FLT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() - Reg(Param(3)).GetFloat() );
    MoveToNextInstruction(SUB_FLT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SUB_CFLTL()		
{ 
    Reg(Param(1)).Set( (*m_pCurCodeStack->m_pFloatTable)[ Param(3) ] - Reg(Param(2)).GetFloat() );
    MoveToNextInstruction(SUB_CFLTL);
}
	
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SUB_CFLTR()		
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() - (*m_pCurCodeStack->m_pFloatTable)[ Param(3)] );
    MoveToNextInstruction(SUB_CFLTR);
}
	
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_MUL_INT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetInt() * Reg(Param(3)).GetInt());
    MoveToNextInstruction(MUL_INT);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_MUL_CINT()			
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetInt() * (char)Param(3));
    MoveToNextInstruction(MUL_CINT);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_MUL_FLT()		
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() * Reg(Param(3)).GetFloat());
    MoveToNextInstruction(MUL_FLT);
}
	
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_MUL_CFLT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() * (*m_pCurCodeStack->m_pFloatTable)[ Param(3)] );
    MoveToNextInstruction(MUL_CFLT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_DIV_INT()	
{ 
    int i = Reg(Param(3)).GetInt();

    if (i == 0) KillCurrentProcess(STATUS_DIVISION_BY_ZERO);
    else
    {
        Reg(Param(1)).Set( Reg(Param(2)).GetInt() / i );
        MoveToNextInstruction(DIV_INT);
    }
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_DIV_CINTL()	
{ 
    int i = Reg(Param(2)).GetInt();

    if (i == 0) KillCurrentProcess(STATUS_DIVISION_BY_ZERO);
    else
    {
        Reg(Param(1)).Set( (int)Param(3) / i );
        MoveToNextInstruction(DIV_CINTL);
    }
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_DIV_CINTR()	
{ 
    if ( (int)Param(3) == 0) KillCurrentProcess(STATUS_DIVISION_BY_ZERO);
    else
    {
        Reg(Param(1)).Set( Reg(Param(2)).GetInt() / (int)Param(3) );
        MoveToNextInstruction(DIV_CINTR);
    }
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_DIV_FLT()	
{ 
    double f = Reg(Param(3)).GetFloat();

    if (f == 0) KillCurrentProcess(STATUS_DIVISION_BY_ZERO);
    else
    {
        Reg(Param(1)).Set( Reg(Param(2)).GetFloat() / f );
        MoveToNextInstruction(DIV_FLT);
    }
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_DIV_CFLTL()		
{ 
    double f = Reg(Param(2)).GetFloat();

    if (f == 0) KillCurrentProcess(STATUS_DIVISION_BY_ZERO);
    else
    {
        Reg(Param(1)).Set( (*m_pCurCodeStack->m_pFloatTable)[ Param(3) ] / f );
        MoveToNextInstruction(DIV_CFLTL);
    }
}
	
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_DIV_CFLTR()		
{ 
    double f = (*m_pCurCodeStack->m_pFloatTable)[ Param(3)];

    if (f == 0) KillCurrentProcess(STATUS_DIVISION_BY_ZERO);
    else
    {
        Reg(Param(1)).Set( Reg(Param(2)).GetFloat() / f );
        MoveToNextInstruction(DIV_CFLTR);
    }
}
	
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_MOD_INT()	
{ 
    int i = Reg(Param(3)).GetInt();

    if (i == 0) KillCurrentProcess(STATUS_MODULO_BY_ZERO);
    else
    {
        Reg(Param(1)).Set( Reg(Param(2)).GetInt() % i );
        MoveToNextInstruction(MOD_INT);
    }
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_MOD_CINTL()	
{ 
    int i = Reg(Param(2)).GetInt();

    if (i == 0) KillCurrentProcess(STATUS_MODULO_BY_ZERO);
    else
    {
        Reg(Param(1)).Set( (int)Param(3) % i );
        MoveToNextInstruction(MOD_CINTL);
    }
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_MOD_CINTR()	
{ 
    if ( (int)Param(3) == 0) KillCurrentProcess(STATUS_MODULO_BY_ZERO);
    else
    {
        Reg(Param(1)).Set( Reg(Param(2)).GetInt() % (int)Param(3) );
        MoveToNextInstruction(MOD_CINTR);
    }
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LT_CHAR()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetChar() < Reg(Param(3)).GetChar() );
    MoveToNextInstruction(LT_CHAR);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LT_CCHAR()	
{
    Reg(Param(1)).Set( Reg(Param(2)).GetChar() < (char)Param(3) );
    MoveToNextInstruction(LT_CCHAR);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LT_INT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetInt() < Reg(Param(3)).GetInt() );
    MoveToNextInstruction(LT_INT);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LT_CINT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetInt() < (int)Param(3) );
    MoveToNextInstruction(LT_CINT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LT_FLT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() < Reg(Param(3)).GetFloat() );
    MoveToNextInstruction(LT_FLT);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LT_CFLT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() < (*m_pCurCodeStack->m_pFloatTable)[ Param(3)] );
    MoveToNextInstruction(LT_CFLT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LT_STR()	
{ 
    ASSERT( Reg(Param(2)).IsString());

    CISOString *pStr1, *pStr2;

    pStr1 = (CISOString*)(Reg(Param(2)).GetSystemObject());
    pStr2 = (CISOString*)Reg(Param(3)).GetSystemObject();

    if (pStr1 != NULL && pStr2 != NULL) Reg(Param(1)).Set( pStr1->m_strString < pStr2->m_strString);
    else Reg(Param(1)).Set(false);

    MoveToNextInstruction(LT_STR);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LT_CSTR()	
{ 
    CISOString *pStr1;

    pStr1 = (CISOString*)Reg(Param(2)).GetSystemObject();

    if (pStr1 != NULL) Reg(Param(1)).Set( pStr1->m_strString < (CString)*(*m_pCurCodeStack->m_pCodeStringTable)[ Param(3)] );
    else Reg(Param(1)).Set(false);

    MoveToNextInstruction(LT_CSTR);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LE_CHAR()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetChar() <= Reg(Param(3)).GetChar() );
    MoveToNextInstruction(LE_CHAR);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LE_CCHAR()	
{
    Reg(Param(1)).Set( Reg(Param(2)).GetChar() <= (char)Param(3) );
    MoveToNextInstruction(LE_CCHAR);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LE_INT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetInt() <= Reg(Param(3)).GetInt() );
    MoveToNextInstruction(LE_INT);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LE_CINT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetInt() <= (int)Param(3) );
    MoveToNextInstruction(LE_CINT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LE_FLT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() <= Reg(Param(3)).GetFloat() );
    MoveToNextInstruction(LE_FLT);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LE_CFLT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() <= (*m_pCurCodeStack->m_pFloatTable)[ Param(3)] );
    MoveToNextInstruction(LE_CFLT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LE_STR()	
{ 
    ASSERT( Reg(Param(2)).IsString());

    CISOString *pStr1, *pStr2;

    pStr1 = (CISOString*)Reg(Param(2)).GetSystemObject();
    pStr2 = (CISOString*)Reg(Param(3)).GetSystemObject();

    if (pStr1 != NULL && pStr2 != NULL) Reg(Param(1)).Set( pStr1->m_strString <= pStr2->m_strString);
    else Reg(Param(1)).Set(false);

    MoveToNextInstruction(LE_STR);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LE_CSTR()	
{ 
    CISOString *pStr1;

    pStr1 = (CISOString*)Reg(Param(2)).GetSystemObject();

    if (pStr1 != NULL) Reg(Param(1)).Set( pStr1->m_strString <= (CString)*(*m_pCurCodeStack->m_pCodeStringTable)[ Param(3)] );
    else Reg(Param(1)).Set(false);

    MoveToNextInstruction(LE_CSTR);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_GT_CHAR()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetChar() > Reg(Param(3)).GetChar() );
    MoveToNextInstruction(GT_CHAR);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_GT_CCHAR()	
{
    Reg(Param(1)).Set( Reg(Param(2)).GetChar() > (char)Param(3) );
    MoveToNextInstruction(GT_CCHAR);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_GT_INT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetInt() > Reg(Param(3)).GetInt() );
    MoveToNextInstruction(GT_INT);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_GT_CINT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetInt() > (int)Param(3) );
    MoveToNextInstruction(GT_CINT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_GT_FLT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() > Reg(Param(3)).GetFloat() );
    MoveToNextInstruction(GT_FLT);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_GT_CFLT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() > (*m_pCurCodeStack->m_pFloatTable)[ Param(3)] );
    MoveToNextInstruction(GT_CFLT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_GT_STR()	
{ 
    ASSERT( Reg(Param(2)).IsString());

    CISOString *pStr1, *pStr2;

    pStr1 = (CISOString*)Reg(Param(2)).GetSystemObject();
    pStr2 = (CISOString*)Reg(Param(3)).GetSystemObject();

    if (pStr1 != NULL && pStr2 != NULL) Reg(Param(1)).Set( pStr1->m_strString > pStr2->m_strString);
    else Reg(Param(1)).Set(false);

    MoveToNextInstruction(GT_STR);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_GT_CSTR()	
{ 
    CISOString *pStr1;

    pStr1 = (CISOString*)Reg(Param(2)).GetSystemObject();

    if (pStr1 != NULL) Reg(Param(1)).Set( pStr1->m_strString > (CString)*(*m_pCurCodeStack->m_pCodeStringTable)[ Param(3)] );
    else Reg(Param(1)).Set(false);

    MoveToNextInstruction(GT_CSTR);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_GE_CHAR()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetChar() >= Reg(Param(3)).GetChar() );
    MoveToNextInstruction(GE_CHAR);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_GE_CCHAR()	
{
    Reg(Param(1)).Set( Reg(Param(2)).GetChar() >= (char)Param(3) );
    MoveToNextInstruction(GE_CCHAR);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_GE_INT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetInt() >= Reg(Param(3)).GetInt() );
    MoveToNextInstruction(GE_INT);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_GE_CINT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetInt() >= (int)Param(3) );
    MoveToNextInstruction(GE_CINT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_GE_FLT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() >= Reg(Param(3)).GetFloat() );
    MoveToNextInstruction(GE_FLT);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_GE_CFLT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() >= (*m_pCurCodeStack->m_pFloatTable)[ Param(3)] );
    MoveToNextInstruction(GE_CFLT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_GE_STR()	
{ 
    ASSERT( Reg(Param(2)).IsString());

    CISOString *pStr1, *pStr2;

    pStr1 = (CISOString*)Reg(Param(2)).GetSystemObject();
    pStr2 = (CISOString*)Reg(Param(3)).GetSystemObject();

    if (pStr1 != NULL && pStr2 != NULL) Reg(Param(1)).Set( pStr1->m_strString >= pStr2->m_strString);
    else Reg(Param(1)).Set(false);

    MoveToNextInstruction(GE_STR);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_GE_CSTR()	
{ 
    CISOString *pStr1;

    pStr1 = (CISOString*)Reg(Param(2)).GetSystemObject();

    if (pStr1 != NULL) Reg(Param(1)).Set( pStr1->m_strString >= (CString)*(*m_pCurCodeStack->m_pCodeStringTable)[ Param(3)] );
    else Reg(Param(1)).Set(false);

    MoveToNextInstruction(GE_CSTR);
}
		
//////////////////////////////////////////////////////////////////////

void CInterpret::Do_EQ_CHAR()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetChar() == Reg(Param(3)).GetChar() );
    MoveToNextInstruction(EQ_CHAR);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_EQ_CCHAR()	
{
    Reg(Param(1)).Set( Reg(Param(2)).GetChar() == (char)Param(3) );
    MoveToNextInstruction(EQ_CCHAR);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_EQ_INT()	
{ 
    CIDataStackItem &Op1 = Reg(Param(2)); 
    CIDataStackItem &Op2 = Reg(Param(3));

    if (Op1.IsInt())
    {
        Reg(Param(1)).Set( Op1.GetInt() == Op2.GetInt() );
    }
    else
    {
        ASSERT(Op1.IsObjectUnitStruct());
        ASSERT( Op2.IsObjectUnitStruct());

        switch ( Op1.GetDataType())
        {
        case T_OBJECT:
            ASSERT( !Op1.IsString());
            Reg(Param(1)).Set( Op1.GetSystemObject() == Op2.GetSystemObject());
            break;
        case T_UNIT:
            Reg(Param(1)).Set( Op1.GetUnit() == Op2.GetUnit());
            break;
        case T_STRUCT:
            Reg(Param(1)).Set( Op1.GetStruct() == Op2.GetStruct());
            break;
        default:
            ASSERT(false);
        }
    }

    MoveToNextInstruction(EQ_INT);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_EQ_CINT()	
{ 
    CIDataStackItem &Op1 = Reg(Param(2)); 

    if (Op1.IsInt())
    {
        Reg(Param(1)).Set( Op1.GetInt() == (int)Param(3) );
    }
    else
    {
        ASSERT( Op1.IsObjectUnitStruct());
        ASSERT( (int)Param(3) == 0 );

        switch ( Op1.GetDataType())
        {
        case T_OBJECT:
            Reg(Param(1)).Set( Op1.GetSystemObject() == NULL);
            break;
        case T_UNIT:
            Reg(Param(1)).Set( Op1.GetUnit() == NULL);
            break;
        case T_STRUCT:
            Reg(Param(1)).Set( Op1.GetStruct() == NULL);
            break;
        default:
            ASSERT(false);
        }
    }

    MoveToNextInstruction(EQ_INT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_EQ_FLT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() == Reg(Param(3)).GetFloat() );
    MoveToNextInstruction(EQ_FLT);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_EQ_CFLT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() == (*m_pCurCodeStack->m_pFloatTable)[ Param(3)] );
    MoveToNextInstruction(EQ_CFLT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_EQ_BOOL()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetBool() == Reg(Param(3)).GetBool() );
    MoveToNextInstruction(EQ_BOOL);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_EQ_CBOOL()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetBool() == (Param(3) != 0) );
    MoveToNextInstruction(EQ_CBOOL);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_EQ_STR()	
{ 
    ASSERT( Reg(Param(2)).IsString());

    CISOString *pStr1, *pStr2;

    pStr1 = (CISOString*)Reg(Param(2)).GetSystemObject();
    pStr2 = (CISOString*)Reg(Param(3)).GetSystemObject();

    if (pStr1 != NULL && pStr2 != NULL) Reg(Param(1)).Set( pStr1->m_strString == pStr2->m_strString);
    else Reg(Param(1)).Set(false);

    MoveToNextInstruction(EQ_STR);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_EQ_CSTR()	
{ 
    CISOString *pStr1;

    pStr1 = (CISOString*)Reg(Param(2)).GetSystemObject();

    if (pStr1 != NULL) Reg(Param(1)).Set( pStr1->m_strString == (CString)*(*m_pCurCodeStack->m_pCodeStringTable)[ Param(3)] );
    else Reg(Param(1)).Set(false);

    MoveToNextInstruction(EQ_CSTR);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_NEQ_CHAR()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetChar() != Reg(Param(3)).GetChar() );
    MoveToNextInstruction(NEQ_CHAR);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_NEQ_CCHAR()	
{
    Reg(Param(1)).Set( Reg(Param(2)).GetChar() != (char)Param(3) );
    MoveToNextInstruction(NEQ_CCHAR);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_NEQ_INT()	
{ 
    CIDataStackItem &Op1 = Reg(Param(2)); 
    CIDataStackItem &Op2 = Reg(Param(3));

    if (Op1.IsInt())
    {
        Reg(Param(1)).Set( Op1.GetInt() != Op2.GetInt() );
    }
    else
    {
        ASSERT( Op1.IsObjectUnitStruct());
        ASSERT( Op2.IsObjectUnitStruct());

        switch ( Op1.GetDataType())
        {
        case T_OBJECT:
            ASSERT( !Op1.IsString());
            Reg(Param(1)).Set( Op1.GetSystemObject() != Op2.GetSystemObject());
            break;
        case T_UNIT:
            Reg(Param(1)).Set( Op1.GetUnit() != Op2.GetUnit());
            break;
        case T_STRUCT:
            Reg(Param(1)).Set( Op1.GetStruct() != Op2.GetStruct());
            break;
        default:
            ASSERT(false);
        }
    }

    MoveToNextInstruction(NEQ_INT);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_NEQ_CINT()	
{ 
    CIDataStackItem &Op1 = Reg(Param(2)); 

    if (Op1.IsInt())
    {
        Reg(Param(1)).Set( Op1.GetInt() != (int)Param(3) );
    }
    else
    {
        ASSERT( Op1.IsObjectUnitStruct());
        ASSERT( (int)Param(3) == 0 );

        switch ( Op1.GetDataType())
        {
        case T_OBJECT:
            Reg(Param(1)).Set( Op1.GetSystemObject() != NULL);
            break;
        case T_UNIT:
            Reg(Param(1)).Set( Op1.GetUnit() != NULL);
            break;
        case T_STRUCT:
            Reg(Param(1)).Set( Op1.GetStruct() != NULL);
            break;
        default:
            ASSERT(false);
        }
    }

    MoveToNextInstruction(NEQ_INT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_NEQ_FLT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() != Reg(Param(3)).GetFloat() );
    MoveToNextInstruction(NEQ_FLT);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_NEQ_CFLT()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetFloat() != (*m_pCurCodeStack->m_pFloatTable)[ Param(3)] );
    MoveToNextInstruction(NEQ_CFLT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_NEQ_BOOL()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetBool() != Reg(Param(3)).GetBool() );
    MoveToNextInstruction(NEQ_BOOL);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_NEQ_CBOOL()	
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetBool() != (Param(3) != 0) );
    MoveToNextInstruction(NEQ_CBOOL);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_NEQ_STR()	
{ 
    ASSERT( Reg(Param(2)).IsString());

    CISOString *pStr1, *pStr2;

    pStr1 = (CISOString*)Reg(Param(2)).GetSystemObject();
    pStr2 = (CISOString*)Reg(Param(3)).GetSystemObject();

    if (pStr1 != NULL && pStr2 != NULL) Reg(Param(1)).Set( pStr1->m_strString != pStr2->m_strString);
    else Reg(Param(1)).Set(false);

    MoveToNextInstruction(NEQ_STR);
}

//////////////////////////////////////////////////////////////////////

void CInterpret::Do_NEQ_CSTR()	
{ 
    ASSERT( Reg(Param(2)).IsString());

    CISOString *pStr1;

    pStr1 = (CISOString*)Reg(Param(2)).GetSystemObject();

    if (pStr1 != NULL) Reg(Param(1)).Set( pStr1->m_strString != (CString)*(*m_pCurCodeStack->m_pCodeStringTable)[ Param(3)] );
    else Reg(Param(1)).Set(false);

    MoveToNextInstruction(NEQ_STR);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_INSTANCEOF()
{ 
    CZUnit *pUnit = Reg(Param(2)).GetUnit();

    if (pUnit == NULL) 
    {
        KillCurrentProcess(STATUS_INVALID_REFERENCE);
        return;
    }

    Reg(Param(1)).Set( pUnit->m_pIUnitType->m_stiName ==  (*m_pCurCodeStack->m_pCodeStringTable)[Param(3)] );
    MoveToNextInstruction(INSTANCEOF);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LOGOR()			
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetBool() || Reg(Param(3)).GetBool() );
    MoveToNextInstruction(LOGOR);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_LOGAND()			
{ 
    Reg(Param(1)).Set( Reg(Param(2)).GetBool() && Reg(Param(3)).GetBool() );
    MoveToNextInstruction(LOGAND);
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_STRADD()
{ 
	ASSERT(Reg(Param(2)).IsString());
	ASSERT(Reg(Param(3)).IsString());

	CISOString *pString;
    CISOString *pStr1, *pStr2;

    pStr1 = (CISOString*)Reg(Param(2)).GetSystemObject();
    pStr2 = (CISOString*)Reg(Param(3)).GetSystemObject();

    if (pStr1 == NULL && pStr2 == NULL)
    {
        KillCurrentProcess(STATUS_INVALID_REFERENCE);
    }
    else
    {
        if ((pString = (CISOString*)CreateSystemObject(SO_STRING)) == NULL)
    	{
	    	KillCurrentProcess(STATUS_CANNOT_CREATE_SYSTEM_OBJECT);
	    }
        else
	    {
            pString->Set( pStr1->m_strString + pStr2->m_strString);
		    
		    Reg(Param(1)).Set(pString);
		    pString->Release();

	        MoveToNextInstruction(STRADD);
        }
	}
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_STRADDCL()		
{ 
	ASSERT(Reg(Param(3)).IsString());

	CISOString *pString, *pStr2;

    pStr2 = (CISOString*)Reg(Param(3)).GetSystemObject();
    if (pStr2 == NULL)
    {
        KillCurrentProcess( STATUS_INVALID_REFERENCE);
    }
    else
    {
    	if ((pString = (CISOString*)CreateSystemObject(SO_STRING)) == NULL)
	    {
		    KillCurrentProcess(STATUS_CANNOT_CREATE_SYSTEM_OBJECT);
	    }
	    else
	    {
    		pString->Set( (CString)*(*m_pCurCodeStack->m_pCodeStringTable)[Param(2)]  +
	    				  pStr2->m_strString );
    
	    	Reg(Param(1)).Set(pString);
		    pString->Release();

	        MoveToNextInstruction(STRADDCL);
        }
	}
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_STRADDCR()
{ 
	ASSERT(Reg(Param(2)).IsString());

	CISOString *pString, *pStr1;

    pStr1 = (CISOString*)Reg(Param(2)).GetSystemObject();
    if (pStr1 == NULL)
    {
        KillCurrentProcess( STATUS_INVALID_REFERENCE);
    }
    else
    {
	    if ((pString = (CISOString*)CreateSystemObject(SO_STRING)) == NULL)
	    {
		    KillCurrentProcess(STATUS_CANNOT_CREATE_SYSTEM_OBJECT);
	    }
	    else
	    {
		    pString->Set( pStr1->m_strString +
					      (CString)*(*m_pCurCodeStack->m_pCodeStringTable)[Param(3)] );

		    Reg(Param(1)).Set(pString);
		    pString->Release();

	        MoveToNextInstruction(STRADDCR);
	    }
    }
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_JMPF()
{ 
    if (!Reg(Param(1)).GetBool())
    {
        * m_ppCurCode += (Param(2) >>2);
    }
    else MoveToNextInstruction(JMPF);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_JMPT()	
{ 
    if (Reg(Param(1)).GetBool())
    {
        *m_ppCurCode += (Param(2) >>2);
    }
    else MoveToNextInstruction(JMPT);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_JMP()
{ 
    *m_ppCurCode += (Param(1) >>2);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_RET()
{ 
    if (m_pCurCodeStack->m_pPrevious == NULL)
    // ukonceni procesu
    {
        FinishCurrentProcess();
    }
    else
    {
        TRACE_INTERPRET_DETAILS2("-- %d: %3d: Now returning.\n", m_pCivilization->GetCivilizationIndex(), m_pCurProcess->m_nProcessID);
        m_pCurProcess->Return();
        SetRunningProcess(m_pCurProcess, false);
    }
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_RETV()
{ 
    // neni ukonceni procesu, to nic nevraci
    ASSERT(m_pCurCodeStack->m_pPrevious != NULL);

    TRACE_INTERPRET_DETAILS2("-- %d: %3d: Now returning.\n", m_pCivilization->GetCivilizationIndex(), m_pCurProcess->m_nProcessID);

    *m_pCurCodeStack->m_pReturnRegister = Reg(Param(1));

    m_pCurProcess->Return();
    SetRunningProcess(m_pCurProcess, false);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_RETV_CCHAR()
{ 
    // neni ukonceni procesu, to nic nevraci
    ASSERT(m_pCurCodeStack->m_pPrevious != NULL);

    TRACE_INTERPRET_DETAILS2("-- %d: %3d: Now returning.\n", m_pCivilization->GetCivilizationIndex(), m_pCurProcess->m_nProcessID);

    ASSERT(m_pCurCodeStack->m_pReturnRegister != NULL);
    m_pCurCodeStack->m_pReturnRegister->Set((char)Param(1));

    m_pCurProcess->Return();
    SetRunningProcess(m_pCurProcess, false);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_RETV_CINT()
{ 
    // neni ukonceni procesu, to nic nevraci
    ASSERT(m_pCurCodeStack->m_pPrevious != NULL);

    TRACE_INTERPRET_DETAILS2("-- %d: %3d: Now returning.\n", m_pCivilization->GetCivilizationIndex(), m_pCurProcess->m_nProcessID);

    ASSERT(m_pCurCodeStack->m_pReturnRegister != NULL);
    m_pCurCodeStack->m_pReturnRegister->Set((int)Param(1));

    m_pCurProcess->Return();
    SetRunningProcess(m_pCurProcess, false);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_RETV_CFLT()
{ 
    // neni ukonceni procesu, to nic nevraci
    ASSERT(m_pCurCodeStack->m_pPrevious != NULL);

    TRACE_INTERPRET_DETAILS2("-- %d: %3d: Now returning.\n", m_pCivilization->GetCivilizationIndex(), m_pCurProcess->m_nProcessID);

    ASSERT(m_pCurCodeStack->m_pReturnRegister != NULL);
    m_pCurCodeStack->m_pReturnRegister->Set( (*m_pCurCodeStack->m_pFloatTable)[ Param(1) ] );

    m_pCurProcess->Return();
    SetRunningProcess(m_pCurProcess, false);

}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_RETV_CBOOL()
{ 
    // neni ukonceni procesu, to nic nevraci
    ASSERT(m_pCurCodeStack->m_pPrevious != NULL);

    TRACE_INTERPRET_DETAILS2("-- %d: %3d: Now returning.\n", m_pCivilization->GetCivilizationIndex(), m_pCurProcess->m_nProcessID);

    ASSERT(m_pCurCodeStack->m_pReturnRegister != NULL);
    m_pCurCodeStack->m_pReturnRegister->Set(Param(1)==0 ? false : true);

    m_pCurProcess->Return();
    SetRunningProcess(m_pCurProcess, false);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_RETV_CSTR()
{ 
    // neni ukonceni procesu, to nic nevraci
    ASSERT(m_pCurCodeStack->m_pPrevious != NULL);
    TRACE_INTERPRET_DETAILS2("-- %d: %3d: Now returning.\n", m_pCivilization->GetCivilizationIndex(), m_pCurProcess->m_nProcessID);
    
    ASSERT(m_pCurCodeStack->m_pReturnRegister != NULL);
    
    CISOString *pString;
    
    if ((pString = (CISOString*)CreateSystemObject(SO_STRING)) == NULL) 
	{
		KillCurrentProcess(STATUS_CANNOT_CREATE_SYSTEM_OBJECT);
	}
    else
    {
		pString->Set( (CString)*(*m_pCurCodeStack->m_pCodeStringTable)[ Param(1)]);

		m_pCurCodeStack->m_pReturnRegister->Set( pString);
		pString->Release();

        m_pCurProcess->Return();
        SetRunningProcess(m_pCurProcess, false);
    }
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SENDTO_BEGIN()
{ 
    ASSERT( m_pCurCodeStack->m_pSendtoInfo == NULL);

    m_pCurCodeStack->m_pSendtoInfo = new CISendtoInformation( m_pCurProcess->FindSenderUnit());

    MoveToNextInstruction(SENDTO_BEGIN);
}
	
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SENDTO_END()
{ 
    ASSERT( m_pCurCodeStack->m_pSendtoInfo != NULL);

    EInterpretStatus eStatus = m_pCurCodeStack->m_pSendtoInfo->Send( this, m_pCurProcess);
    
    delete m_pCurCodeStack->m_pSendtoInfo;
    m_pCurCodeStack->m_pSendtoInfo = NULL;

	if (eStatus != STATUS_SUCCESS) 
	{
		KillCurrentProcess( eStatus);
		return;
	}

    MoveToNextInstruction(SENDTO_END);
}
		
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SENDTO_WAIT()
{ 
    ASSERT( m_pCurCodeStack->m_pSendtoInfo != NULL);

	CISendtoWaitingAll *pWaiting = new CISendtoWaitingAll();
	pWaiting->Create();
    m_pCurCodeStack->m_pSendtoInfo->SetWaiting( pWaiting, CISendtoInformation::WAIT_ALL);
	pWaiting->Release();

    MoveToNextInstruction(SENDTO_WAIT);
}
	
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SENDTO_WAIT1ST()
{ 
    ASSERT( m_pCurCodeStack->m_pSendtoInfo != NULL);

	CISendtoWaiting1st *pWaiting = new CISendtoWaiting1st();
	pWaiting->Create();
    m_pCurCodeStack->m_pSendtoInfo->SetWaiting( pWaiting, CISendtoInformation::WAIT_1ST);
	pWaiting->Release();

    MoveToNextInstruction(SENDTO_WAIT1ST);
}
	
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SENDTO_NOTIFICATION()
{ 
    ASSERT( m_pCurCodeStack->m_pSendtoInfo != NULL);

    m_pCurCodeStack->m_pSendtoInfo->SetNotification();

    MoveToNextInstruction(SENDTO_NOTIFICATION);
}
	
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SENDTO_ADDITEM()
{ 
    ASSERT( m_pCurCodeStack->m_pSendtoInfo != NULL);

    CIDataStackItem &DSI = Reg(Param(1));

    // jednotka
    if (DSI.IsUnit())
    {
        if (DSI.GetUnit() == NULL) 
        {
            KillCurrentProcess( STATUS_INVALID_REFERENCE);
            return;
        }
        else m_pCurCodeStack->m_pSendtoInfo->AddUnit( DSI.GetUnit());
    }
    // mnozina jednotek
    else
    {
        ASSERT( DSI.IsSet());

        CISOSet<CZUnit*> *pSet = (CISOSet<CZUnit*> *) DSI.GetSystemObject();

        ASSERT( pSet->m_DataType.IsUnit());

        CISOSet<CZUnit*>::SSetItem *pSetItem = pSet->m_pItems;

        while (pSetItem != NULL)
        {
        		if (pSetItem->Item != NULL)
			      {
              m_pCurCodeStack->m_pSendtoInfo->AddUnit( pSetItem->Item);
      			}

            pSetItem = pSetItem->pNext;
        }
    }    

    MoveToNextInstruction(SENDTO_ADDITEM);
}
 
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SENDTO_SEND()
{ 
    ASSERT( m_pCurCodeStack->m_pSendtoInfo != NULL);

    ASSERT( (*m_pCurCodeStack->m_pCodeStringTable)[ Param(1)] != NULL);

    m_pCurCodeStack->m_pSendtoInfo->AddEvent( (*m_pCurCodeStack->m_pCodeStringTable)[ Param(1)], 
                                               m_pCurCodeStack->m_pBag);

    MoveToNextInstruction(SENDTO_SEND);
}
	
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SEND()
{ 
    ASSERT( Reg(0).IsUnit());
    ASSERT( Reg(Param(1)).IsUnit());
    ASSERT( (*m_pCurCodeStack->m_pCodeStringTable)[ Param(2)] != NULL);

    CZUnit *pUnit = Reg(Param(1)).GetUnit();

    if (pUnit == NULL)
    {
        KillCurrentProcess( STATUS_INVALID_REFERENCE);
    }
    else
    {
		EInterpretStatus eStatus;
		
		eStatus = pUnit->AppendEventArrow( this, (*m_pCurCodeStack->m_pCodeStringTable)[ Param(2)], 
										   m_pCurCodeStack->m_pBag);
		if ( eStatus != STATUS_SUCCESS) 
		{
			KillCurrentProcess( eStatus);
			return;
		}

        MoveToNextInstruction(SEND);
    }
}

//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SET_UN()
{ 
    CISOSetParent *pSet1 = (CISOSetParent*)Reg(Param(2)).GetSystemObject();
    CISOSetParent *pSet2 = (CISOSetParent*)Reg(Param(3)).GetSystemObject();

	if (pSet1 == NULL || pSet2 == NULL)
	{
		KillCurrentProcess( STATUS_INVALID_REFERENCE);
		return;
	}

    if ( Param(1) == Param(2))
    {
        pSet1->UnionWith( pSet2);
    }
    else
    {
        CISOSetParent *pNewSet;

        if ( (pNewSet = (CISOSetParent*)CreateSet( &pSet1->m_DataType, false)) == NULL)
        {
            KillCurrentProcess(STATUS_CANNOT_CREATE_SYSTEM_OBJECT);
            return;
        }

        pNewSet->CreateFromUnion( this, pSet1, pSet2);
        Reg(Param(1)).Set(pNewSet);
		pNewSet->Release();
    }

    MoveToNextInstruction(SET_UN);
}
         
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SET_IS()
{ 
    CISOSetParent *pSet1 = (CISOSetParent*)Reg(Param(2)).GetSystemObject();
    CISOSetParent *pSet2 = (CISOSetParent*)Reg(Param(3)).GetSystemObject();

	if (pSet1 == NULL || pSet2 == NULL)
	{
		KillCurrentProcess( STATUS_INVALID_REFERENCE);
		return;
	}

    if ( Param(1) == Param(2))
    {
        pSet1->IntersectWith( pSet2);
    }
    else
    {
        CISOSetParent *pNewSet;

        if ( (pNewSet = (CISOSetParent*) CreateSet( &pSet1->m_DataType, false)) == NULL)
        {
            KillCurrentProcess(STATUS_CANNOT_CREATE_SYSTEM_OBJECT);
            return;
        }

        pNewSet->CreateFromIntersection( this, pSet1, pSet2);
        Reg(Param(1)).Set(pNewSet);
		pNewSet->Release();
    }

    MoveToNextInstruction(SET_IS);
}
         
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SET_DIF() 
{ 
    CISOSetParent *pSet1 = (CISOSetParent*)Reg(Param(2)).GetSystemObject();
    CISOSetParent *pSet2 = (CISOSetParent*)Reg(Param(3)).GetSystemObject();

	if (pSet1 == NULL || pSet2 == NULL)
	{
		KillCurrentProcess( STATUS_INVALID_REFERENCE);
		return;
	}

    if ( Param(1) == Param(2))
    {
        pSet1->DifferWith( pSet2);
    }
    else
    {
        CISOSetParent *pNewSet;

        if ( (pNewSet = (CISOSetParent*) CreateSet( &pSet1->m_DataType, false)) == NULL)
        {
            KillCurrentProcess(STATUS_CANNOT_CREATE_SYSTEM_OBJECT);
            return;
        }

        pNewSet->CreateFromDifference( this, pSet1, pSet2);
        Reg(Param(1)).Set(pNewSet);
		pNewSet->Release();
    }

    MoveToNextInstruction(SET_DIF);
}
       
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SET_ELEM() 
{ 
	ASSERT( Reg(Param(3)).IsSet());

	bool bResult;
	EInterpretStatus eStatus;
	CISOSetParent *pSet = (CISOSetParent*)Reg(Param(3)).GetSystemObject();
	
	if ( pSet == NULL)
	{
		KillCurrentProcess( STATUS_INVALID_REFERENCE);
		return;
	}

	eStatus = pSet->Contains( Reg(Param(2)), bResult);
	if (eStatus != STATUS_SUCCESS) KillCurrentProcess( eStatus);
	else
	{
	    Reg(Param(1)).Set( bResult);
		MoveToNextInstruction(SET_ELEM);
	}
}
      
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SET_ADD() 
{ 
    CISOSetParent *pSet = (CISOSetParent*)Reg(Param(2)).GetSystemObject();

	if ( pSet == NULL)
	{
		KillCurrentProcess( STATUS_INVALID_REFERENCE);
		return;
	}

    if ( Param(1) == Param(2))
    {
        EInterpretStatus eStatus = pSet->AddItemToSet( Reg(Param(3)));
		if ( eStatus != STATUS_SUCCESS) KillCurrentProcess( eStatus);
		else MoveToNextInstruction(SET_ADD);
    }
    else
    {
        CISOSetParent *pNewSet;

        if ( (pNewSet = (CISOSetParent*) CreateSet( &pSet->m_DataType, false)) == NULL)
        {
            KillCurrentProcess(STATUS_CANNOT_CREATE_SYSTEM_OBJECT);
            return;
        }
        pNewSet->CreateFrom( this, pSet);
        EInterpretStatus eStatus = pNewSet->AddItemToSet( Reg(Param(3)));

		if ( eStatus != STATUS_SUCCESS) 
		{
			pNewSet->Release();
			KillCurrentProcess( eStatus);
			return;
		}
        
        Reg(Param(1)).Set(pNewSet);
		pNewSet->Release();

		MoveToNextInstruction(SET_ADD);
    }
}
      
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SET_REM() 
{ 
    CISOSetParent *pSet = (CISOSetParent*)Reg(Param(2)).GetSystemObject();
	EInterpretStatus eStatus;

	if ( pSet == NULL)
	{
		KillCurrentProcess( STATUS_INVALID_REFERENCE);
		return;
	}

    if ( Param(1) == Param(2))
    {
        eStatus = pSet->RemoveItemFromSet( Reg(Param(3)));
		if (eStatus != STATUS_SUCCESS)
		{
			KillCurrentProcess( eStatus);
			return;
		}
    }
    else
    {
        CISOSetParent *pNewSet;

        if ( (pNewSet = (CISOSetParent*) CreateSet( &pSet->m_DataType, false)) == NULL)
        {
            KillCurrentProcess(STATUS_CANNOT_CREATE_SYSTEM_OBJECT);
            return;
        }
        pNewSet->CreateFrom( this, pSet);
        eStatus = pNewSet->RemoveItemFromSet( Reg(Param(3)));
		if ( eStatus != STATUS_SUCCESS)
		{
			pNewSet->Release();
			KillCurrentProcess( eStatus);
			return;
		}
        
        Reg(Param(1)).Set(pNewSet);
		pNewSet->Release();
    }

    MoveToNextInstruction(SET_REM);
}
      
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SETFOR_FIRST()
{ 
    ASSERT(Reg(Param(3)).IsSet() );

	CISOSetParent *pSet = (CISOSetParent*)Reg(Param(3)).GetSystemObject();
	if ( pSet == NULL)
	{
		KillCurrentProcess( STATUS_INVALID_REFERENCE);
		return;
	}

	if (!pSet->SetForFirst( Reg(Param(2)), Reg(Param(1)) ) )
	{
		KillCurrentProcess(STATUS_NESTED_SET_FOR_CYCLE);
	}
	else MoveToNextInstruction(SETFOR_FIRST);
}
   
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SETFOR_NEXT()
{ 
    ASSERT(Reg(Param(3)).IsSet() );

	CISOSetParent *pSet = (CISOSetParent*)Reg(Param(3)).GetSystemObject();
	if ( pSet == NULL)
	{
		KillCurrentProcess( STATUS_INVALID_REFERENCE);
		return;
	}

    pSet->SetForNext( Reg(Param(2)), Reg(Param(1)) );

    MoveToNextInstruction(SETFOR_NEXT);
}
    
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_SETFOR_END() 
{ 
    ASSERT(Reg(Param(1)).IsSet() );

	CISOSetParent *pSet = (CISOSetParent*)Reg(Param(1)).GetSystemObject();
	if ( pSet == NULL)
	{
		KillCurrentProcess( STATUS_INVALID_REFERENCE);
		return;
	}

    pSet->SetForEnd() ;

    MoveToNextInstruction(SETFOR_END);
}
    
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_ISNULL()
{ 
    // OBSOLETE !!! Prekompiluj skriptset!!!
    ASSERT( false);

    CIDataStackItem &DSI = Reg(Param(2));
    
    if (DSI.IsUnit()) Reg(Param(1)).Set( DSI.GetUnit() == 0 );
    else if (DSI.IsStruct()) Reg(Param(1)).Set( DSI.GetStruct() == 0 );
    else if (DSI.IsSystemObject()) Reg(Param(1)).Set( DSI.GetSystemObject() == 0 );
    else ASSERT(false);
    
    MoveToNextInstruction(OBSOLETE_ISNULL);
}
			
//////////////////////////////////////////////////////////////////////
 
void CInterpret::Do_ISNOTNULL() 
{ 
    // OBSOLETE !!! Prekompiluj skriptset!!!
    ASSERT( false);

    CIDataStackItem &DSI = Reg(Param(2));
    
    if (DSI.IsUnit()) Reg(Param(1)).Set( DSI.GetUnit() != 0 );
    else if (DSI.IsStruct()) Reg(Param(1)).Set( DSI.GetStruct() != 0 );
    else if (DSI.IsSystemObject()) Reg(Param(1)).Set( DSI.GetSystemObject() != 0 );
    else ASSERT(false);
    
    MoveToNextInstruction(OBSOLETE_ISNOTNULL);
}

//////////////////////////////////////////////////////////////////////

void CInterpret::Do_ADDBAG_NULL()
{
	ASSERT(m_pCurCodeStack->m_pBag != NULL);

    m_pCurCodeStack->m_pBag->AddNull();
    MoveToNextInstruction(ADDBAG_NULL);
}


//////////////////////////////////////////////////////////////////////

void CInterpret::Do_SETNULL()
{
    CIDataStackItem &DSI = Reg(Param(1));

    if (DSI.IsUnit()) DSI.Set( (CZUnit*) NULL );
    else if (DSI.IsStruct()) DSI.Set( (CIStructure*) NULL );
    else if (DSI.IsSystemObject()) DSI.Set( (CISystemObject*) NULL );
    else ASSERT(false);
    
    MoveToNextInstruction(SETNULL);
}


//////////////////////////////////////////////////////////////////////

void CInterpret::Do_SETNULLM()
{
	// unit
	if ( Reg(Param(1)).IsUnit())
	{
		CZUnit *pUnit = Reg(Param(1)).GetUnit();

		if (pUnit != NULL) 
		{
			if (pUnit->m_pConstructorFinished != NULL &&
                !(m_pCurProcess->m_nProcessType == CIProcess::PROCESS_CONSTRUCTOR  && 
                 m_pCurProcess->m_pUnit == pUnit)) 
			{
				UnitAccessedAndConstructorNoFinished( pUnit, m_pCurProcess);
				return;
			}

			if (!pUnit->StoreMember( NULL, 
								(*m_pCurCodeStack->m_pCodeStringTable)[Param(2)],
								m_pCivilization != pUnit->m_pCivilization))
			{
				KillCurrentProcess(STATUS_MEMBER_NOT_FOUND);
                return;
			}
		}
	}
	// system object
	else if ( Reg(Param(1)).IsSystemObject())
	{
		 CISystemObject *pSystemObject = Reg(Param(1)).GetSystemObject();

		 if ( pSystemObject != NULL)
		 {
			 // array
			 if ( pSystemObject->GetObjectType() == SO_ARRAY)
			 {
				 ASSERT( Reg(Param(2)).IsInt());

				 ((CISOArrayParent*)pSystemObject)->SetAt( Reg(Param(2)).GetInt(), NULL);
			 }
			 // other
			 else
			 {
				 if ( !pSystemObject->StoreMember( NULL, Param(2)))
				 {
					  KillCurrentProcess(STATUS_MEMBER_NOT_FOUND);
                      return;
				 }
			 }
		 }
	}
	// structure
	else if (Reg(Param(1)).IsStruct())
	{
		CIStructure *pStruct = Reg(Param(1)).GetStruct();

		if (pStruct != NULL)
		{
			if (!pStruct->StoreMember( NULL, (*m_pCurCodeStack->m_pCodeStringTable)[Param(2)]))
			{
				KillCurrentProcess(STATUS_MEMBER_NOT_FOUND);
                return;
			}
		}
	}
	else ASSERT( false);

    MoveToNextInstruction(SETNULLM);
}

//////////////////////////////////////////////////////////////////////
// CInterpret
//////////////////////////////////////////////////////////////////////

CInterpret::CInterpret()
{
    ASSERT( sizeof(INSTRUCTION) == sizeof(REG));
    m_pCodeManager = NULL;

	m_tpSystemObjects.InitHashTable( SYSTEM_OBJECTS_HASH_TABLE_SIZE);
	m_tpStructures.InitHashTable( STRUCTURES_HASH_TABLE_SIZE);

	m_nSystemObjectCount = 0;
	m_nStructureCount = 0;
	m_ppPersistentLoadProcesses = NULL;

    m_pCivilization = NULL;

    m_ppCurCode = NULL;
    m_nCurBorder = 0;
    m_pCurCodeStack = NULL;
    m_pCurDataStack = NULL;

    m_nProcessCount = 0;

	m_liGlobalProfileCounter = 0;
	m_liLocalProfileCounter = 0;
    m_liLastLocalProfileCounter = 0;
}

//////////////////////////////////////////////////////////////////////

CInterpret::~CInterpret()
{
    // bylo zavolano Delete
    ASSERT (m_pCodeManager == NULL);
}

//////////////////////////////////////////////////////////////////////
 
bool CInterpret::Create(CZCivilization *pCivilization, CEventManager *pEventManager)
{
    if (!CNotifier::Create()) return false;

    ASSERT( pCivilization != NULL);
    m_pCivilization = pCivilization;
    m_pCodeManager = pCivilization->GetCodeManager();

    ASSERT(pEventManager!=NULL);
    m_pEventManager = pEventManager;

    m_nState = IDLE;

    m_pCurProcess = NULL;
    m_ppCurCode = NULL;
    m_pCurDataStack = NULL;
    m_nCurBorder = 0;

    m_nQuantum = 0;

    // inicializace front
    m_pFirstReadyToRun = m_pLastReadyToRun = m_pFirstSleeping = NULL;

    // inicializace nazvu def. jednotky
    m_stiDefaultUnit = g_StringTable.AddItem(DEFAULT_UNIT_PARENT);

    // process IDs
    m_nFirstFreeProcessID = 1;

    // pripojit se sam na sebe
    Connect( this, CIVILIZATION_OR_INTERPRET_NOTIFIER_ID);

	// systemove objekty a struktury
	m_nSystemObjectCount = 0;
	m_nStructureCount = 0;

    m_nProcessCount = 0;

    return true;
}

//////////////////////////////////////////////////////////////////////

void CInterpret::Delete()
{
    // uklid po padu pro loadovani hry
    if ( g_bGameLoading)
    {
        // system objects
        POSITION pos = m_tpSystemObjects.GetStartPosition();
        while ( pos != NULL)
        {
            CISystemObject *pTmp, *pSystemObject;
            m_tpSystemObjects.GetNextAssoc( pos, pTmp, pSystemObject);
            pSystemObject->Delete();
            delete pSystemObject;
        }
        m_tpSystemObjects.RemoveAll();
        m_nSystemObjectCount = 0;

        // structures
        pos = m_tpStructures.GetStartPosition();
        while ( pos != NULL)
        {
            CIStructure *pTmp, *pStructure;
            m_tpStructures.GetNextAssoc( pos, pTmp, pStructure);
            pStructure->Delete();
            delete pStructure;
        }
        m_tpStructures.RemoveAll();
        m_nStructureCount = 0;

        // loaded processes
        if ( m_ppPersistentLoadProcesses != NULL)
        {
            int i;
            for (i=0; i<m_nProcessCount; i++)
            {
                m_ppPersistentLoadProcesses[i]->Delete();
                delete m_ppPersistentLoadProcesses[i];
            }

            delete [] m_ppPersistentLoadProcesses;
            m_ppPersistentLoadProcesses = NULL;
        }
        return;
    }

    CIProcess *pProcess;

    // uvolnit bezici process
    if (m_pCurProcess != NULL) 
    {
        m_pCurProcess->Delete(STATUS_GAME_CLOSING);
        delete m_pCurProcess;
		m_pCurProcess = NULL;
    }

    // uvolnit vsechny ready-to-run procesy
	while ( m_pFirstReadyToRun != NULL)
	{
		ASSERT(m_pFirstReadyToRun->m_nState == CIProcess::RUNNING);
		pProcess = m_pFirstReadyToRun;

		RemoveProcessFromListWithLastPointer( ReadyToRun, pProcess);

        pProcess->Delete( STATUS_GAME_CLOSING);
		delete pProcess;
	}

    // uvolnit vsechny ready-to-run procesy
	while ( m_pFirstSleeping != NULL)
	{
		ASSERT(m_pFirstSleeping->m_nState == CIProcess::SLEEPING);
		pProcess = m_pFirstSleeping;

		RemoveProcessFromList( Sleeping, pProcess);
        pProcess->Delete( STATUS_GAME_CLOSING);
		delete pProcess;
	}

	ASSERT( m_pFirstReadyToRun == NULL);
	ASSERT( m_pFirstSleeping == NULL);

    // uvolneni nazvu defaultni jednotky
    if (m_stiDefaultUnit != NULL) m_stiDefaultUnit = m_stiDefaultUnit->Release();

    m_pCodeManager = NULL;

    CNotifier::Delete();
}

////////////////////////////////////////////////////////////////

void CInterpret::Run()
{
    int nRet = 0;
    DWORD dwQuitParam;
    bool bRun = true;

    // inicializace
    ASSERT(m_pEventManager != NULL);

#ifdef _DEBUG
    if (m_nState == IDLE) TRACE_INTERPRET1("- %d: Interpret IDLING\n", m_pCivilization->GetCivilizationIndex());
#endif

    while (bRun)
    {
		ASSERT( m_pFirstReadyToRun == NULL || m_pFirstReadyToRun->m_nState != CIProcess::DESTROYED);
        // ZPRACOVANI JEDNE INSTRUKCE
        if (m_nState == IDLE) ::Sleep(100);
        else
        {
			// increment profile counters
			
            ::InterlockedIncrement( (LONG*)&m_liLocalProfileCounter);
			m_pCurCodeStack->IncrementProfileCounter();

#ifdef _DEBUG
			if (!(**m_ppCurCode >= 0 && **m_ppCurCode < INSTRUCTION_COUNT)) __asm { int 3 };
#endif
            // do one instruction
            TRACE_INTERPRET_DETAILS3("-- %d: %3d: >> %s\n",  m_pCivilization->GetCivilizationIndex(), 
                m_pCurProcess->m_nProcessID, g_aInstrInfo[**m_ppCurCode].m_lpszName);
//            TRACE_INTERPRET_DETAILS3("-- %d: %3d: Quantum: %d\n", m_pCivilization->GetCivilizationIndex(), 
//                m_pCurProcess->m_nProcessID, m_nQuantum);
            
            (this->*m_aInstructionImplementations[**m_ppCurCode])();
            m_nQuantum--;
        
			// infinite cycle?
            if (m_nQuantum <= 0) 
			{
				KillCurrentProcess( STATUS_INFINITE_CYCLE);
			}
        }

        // ZPRACOVANI ZPRAV (na konci kvuli testovani podminky nRun hned ve while(bRun) )
        while ((nRet = m_pEventManager->DoEvent(&dwQuitParam))!=0)
        {
            // ukonceni interpretace  - quit event
            if (nRet == -1) 
            {
                TRACE_INTERPRET1("- %d: Interpret ENDING\n", m_pCivilization->GetCivilizationIndex());
 
                bRun = false;
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////

void CInterpret::ReadyToRun( CIProcess *pProcess, bool bHighPriority)
{
    TRACE_INTERPRET3("- %d: Process %d (%s) READY TO RUN\n",  m_pCivilization->GetCivilizationIndex(), 
        pProcess->m_nProcessID, (CString)*pProcess->m_stiName);

    pProcess->m_nState = CIProcess::RUNNING;

    if (bHighPriority) 
    {
        AddProcessToListAtBeginWithLastPointer( ReadyToRun, pProcess);
    }
    else 
    {
        AddProcessToListAtEnd( ReadyToRun, pProcess);
    }

    if (m_nState == IDLE) RunNextProcess(false);
}

//////////////////////////////////////////////////////////////////////

void CInterpret::RunNextProcess( bool bCanBeCurrent)
{
    CIProcess *pProcess;

    if ( m_pFirstReadyToRun == NULL) 
    {
        if (bCanBeCurrent)
        {
            TRACE_INTERPRET3("- %d: Process %d (%s) RUNNING\n",  m_pCivilization->GetCivilizationIndex(), 
                m_pCurProcess->m_nProcessID, (CString)*m_pCurProcess->m_stiName);
            m_nQuantum = MAX_QUANTUM;
        }
        else
        {   
            TRACE_INTERPRET1("- %d: Interpret IDLING\n", m_pCivilization->GetCivilizationIndex());
            m_nState = IDLE;
            m_pCurProcess = NULL;
        }
    }
    else 
    {
        RemoveFirstProcessFromList( ReadyToRun, pProcess);

		ASSERT( pProcess->m_nState != CIProcess::DESTROYED);

        //if (bCanBeCurrent) ReadyToRun( m_pCurProcess);

        TRACE_INTERPRET3("- %d: Process %d (%s) RUNNING\n",  m_pCivilization->GetCivilizationIndex(), 
            pProcess->m_nProcessID, (CString)*pProcess->m_stiName);
        SetRunningProcess( pProcess, true);

        m_nState = RUNNING;
    }
}

//////////////////////////////////////////////////////////////////////

void CInterpret::Sleep(CIProcess *pProcess, CIWaitingInterface *pWaitingInterface)
{
    ASSERT( pProcess != NULL);
	ASSERT( pWaitingInterface != NULL);

    TRACE_INTERPRET3("- %d, Process %d (%s) SLEEPING\n",  m_pCivilization->GetCivilizationIndex(), 
        pProcess->m_nProcessID, (CString)*pProcess->m_stiName);

    if ( pProcess != m_pCurProcess ) 
    {
        if (pProcess->m_nState == RUNNING)
        {
            RemoveProcessFromListWithLastPointer( ReadyToRun, pProcess); 
        }
        else if (pProcess->m_nState != CIProcess::CREATED)
        {
            ASSERT( pProcess->m_nState == CIProcess::SLEEPING);

            // Pokus o uspani uz uspaneho procesu.
            ASSERT( false);
            return;
        }
    }

    pProcess->m_nState = CIProcess::SLEEPING;
	pProcess->m_pWaitingInterface = pWaitingInterface->AddRef();

    // zarazeni do seznamu spicich procesu
    AddProcessToListAtBegin( Sleeping, pProcess);

    // spusteni dalsiho procesu
    if ( pProcess == m_pCurProcess ) RunNextProcess(false);
}

//////////////////////////////////////////////////////////////////////

void CInterpret::SleepCurrentProcess( CIWaitingInterface *pWaitingInterface)
{
    Sleep( m_pCurProcess, pWaitingInterface);
}

/////////////////////////////////////////////////////////////////////

void CInterpret::Wakeup(CIProcess *pProcess, EInterpretStatus eStatus)
{
    ASSERT( pProcess->m_nState == CIProcess::SLEEPING);

    TRACE_INTERPRET3("- %d: Process %d (%s) WAKEUP\n",  m_pCivilization->GetCivilizationIndex(), 
        pProcess->m_nProcessID, (CString)*pProcess->m_stiName);

    // vyparat z fronty spicich procesu
    RemoveProcessFromList( Sleeping, pProcess);

	ASSERT( pProcess->m_pWaitingInterface != NULL);
	pProcess->m_pWaitingInterface->Release();
	pProcess->m_pWaitingInterface = NULL;
	ReadyToRun( pProcess);

	if (eStatus != STATUS_SUCCESS) KillProcess( pProcess, eStatus);
}

//////////////////////////////////////////////////////////////////////

void CInterpret::FinishCurrentProcess()
{
    TRACE_INTERPRET3("- %d: Process %d (%s) FINISHED\n",  m_pCivilization->GetCivilizationIndex(), 
        m_pCurProcess->m_nProcessID, (CString)*m_pCurProcess->m_stiName);

    KillCurrentProcess( STATUS_SUCCESS);
}

//////////////////////////////////////////////////////////////////////

void CInterpret::KillProcess( CIProcess *pProcess, EInterpretStatus eStatus)
{
    if (eStatus != STATUS_SUCCESS)
    {
        // oznameni zabiti
        CString str;
    
        str.Format("Killing process %s (%s::%s), status: %s", (CString)*pProcess->m_stiName, 
            (CString)*pProcess->m_pCodeStack->m_stiUnitName, (CString)*pProcess->m_pCodeStack->m_stiFunctionName, 
			GetStatusText( eStatus));
        ReportError( str);

        TRACE_INTERPRET("- %d: Process %d (%s) KILLED, status %s\n",  m_pCivilization->GetCivilizationIndex(), 
            pProcess->m_nProcessID, (CString)*pProcess->m_stiName, GetStatusText( eStatus));
    }

    if (pProcess->m_nState == CIProcess::RUNNING)
    {
        // vyhaknuti z ReadyToRun fronty
        if (pProcess != m_pCurProcess) 
        {
            RemoveProcessFromListWithLastPointer( ReadyToRun, pProcess);

            pProcess->Delete( eStatus);
        }
        // nebo spusteni dalsiho procesu
        else 
        {
            pProcess->Delete( eStatus);
            RunNextProcess(false);
        }
    }
    else if (pProcess->m_nState == CIProcess::SLEEPING)
    {
        // vyhaknuti ze Sleeping fronty
        RemoveProcessFromList( Sleeping, pProcess);

		// ukonceni procesu
        pProcess->Delete( eStatus);
    }

    else ASSERT(false);

	delete pProcess;
}

////////////////////
// REAKCE NA ZPRAVY
////////////////////

void CInterpret::OnRunConstructor(CZUnit *pUnit)
{
    ASSERT(pUnit != NULL);
    ASSERT(pUnit->m_pIUnitType != NULL);

    if ( pUnit->m_pIUnitType->m_stiName != m_stiDefaultUnit)
    {
        TRACE_INTERPRET_DETAILS2("-- %d: Trying to run %s constructor...\n",  m_pCivilization->GetCivilizationIndex(), 
            (CString)*pUnit->m_pIUnitType->m_stiName);

        CIProcess *pProcess = new CIProcess;
        pProcess->CreateConstructorDestructorCall(this, pUnit, true);

        TRACE_INTERPRET3("- %d: Process %d (%s) JUST CREATED\n",  m_pCivilization->GetCivilizationIndex(), 
            pProcess->m_nProcessID, (CString)*pProcess->m_stiName);
    
        ReadyToRun(pProcess);
    }
    else
    // defaultni jednotka (Unit)
    {
        TRACE_INTERPRET_DETAILS0("- Default unit constructor called.\n");
        pUnit->m_pConstructorFinished->FinishWaiting( STATUS_SUCCESS);
        pUnit->m_pConstructorFinished->Release();  
        pUnit->m_pConstructorFinished = NULL;
    }
}

//////////////////////////////////////////////////////////////////////

void CInterpret::OnRunDestructor(CZUnit *pUnit)
{
    ASSERT(pUnit != NULL);
    ASSERT(pUnit->m_pIUnitType != NULL);
    
    TRACE_INTERPRET_DETAILS2("-- %d: Trying to run %s destructor...\n",  m_pCivilization->GetCivilizationIndex(), 
        (CString)*pUnit->m_pIUnitType->m_stiName);
    
    CIProcess *pProcess = new CIProcess;
    pProcess->CreateConstructorDestructorCall(this, pUnit, false);
    
    TRACE_INTERPRET3("- %d: Process %d (%s) JUST CREATED\n",  m_pCivilization->GetCivilizationIndex(), 
        pProcess->m_nProcessID, (CString)*pProcess->m_stiName);
    
    if (pUnit->m_pConstructorFinished != NULL) UnitAccessedAndConstructorNoFinished( pUnit, pProcess);
    else ReadyToRun(pProcess);
}

//////////////////////////////////////////////////////////////////////

void CInterpret::OnRunEvent(CIRunEventData *pRunEventData)
{
    EInterpretStatus status;
    
    ASSERT(pRunEventData != NULL);
    
    ASSERT(pRunEventData->m_pUnit != NULL);
    ASSERT(pRunEventData->m_eEventType == EVENT_NOTIFICATION || 
        pRunEventData->m_eEventType == EVENT_COMMAND);
    ASSERT(pRunEventData->m_pNotifyOrCommand != NULL);
    ASSERT(pRunEventData->m_pNotifyOrCommand->m_stiEventName != NULL);
    
    // zamceni prace nad eventama
    m_pCivilization->LockEvents();
    
    CIEvent *pEvent;
    CZUnit *pUnit = pRunEventData->m_pUnit;
    CZUnitNotifyQueueItem *pNotifyOrCommand = pRunEventData->m_pNotifyOrCommand;
    
#ifdef _DEBUG
    if (pRunEventData->m_eEventType == EVENT_COMMAND)
    {
        TRACE_INTERPRET_DETAILS2("-- %d: Trying to run command %s...\n",  m_pCivilization->GetCivilizationIndex(),
            (CString)*pNotifyOrCommand->m_stiEventName);
    }
    else
    {
        TRACE_INTERPRET_DETAILS2("-- %d: Trying to run notification %s...\n",  m_pCivilization->GetCivilizationIndex(), 
            (CString)*pNotifyOrCommand->m_stiEventName);
    }
#endif
    
    switch (pUnit->m_pIUnitType->GetEvent(pNotifyOrCommand->m_stiEventName, pEvent))
    {
    case IMPL_NONE:
        // neni nadefinovana reakce na zpravu
        ReportError( "Unit \"%s\" has no reaction on event %s", 
            (CString)*pUnit->m_pIUnitType->m_stiName, (CString)*pNotifyOrCommand->m_stiEventName);
        pUnit->OnEventFinished( NULL, pRunEventData->m_eEventType, STATUS_UNKNOWN_ERROR /* status zde ted neni podstatny*/, this);
        
        goto Konec;
        break;
        
    case IMPL_SCRIPT:
        // EVENT_NOTIFICATION
        if (pRunEventData->m_eEventType == EVENT_NOTIFICATION)
        {
            if ( pEvent->m_bIsCommand)
            {
                ReportError( "Attempting to execute the command '%s' as notification, skipping execution.", 
                    (CString)*pNotifyOrCommand->m_stiEventName);
                pUnit->OnEventFinished( NULL, EVENT_NOTIFICATION, STATUS_COMMAND_NOTIFICATION_MISMATCH, this);
                goto Konec;
            }
            
            CIProcess *pProcess;
            pProcess = new CIProcess;
            
            if ( (status = pProcess->CreateEventCall( this, pUnit, pEvent, &pNotifyOrCommand->m_Bag, 
                NULL, CIProcess::PROCESS_EVENT_NOTIFICATION)) == STATUS_SUCCESS)
            {
                TRACE_INTERPRET3("- %d: Process %d (%s) JUST CREATED (notification)\n",  m_pCivilization->GetCivilizationIndex(), 
                    pProcess->m_nProcessID, (CString)*pProcess->m_stiName);
                pUnit->m_nNotifyProcessID = pProcess->m_nProcessID;
                
                if (pRunEventData->m_pUnit->m_pConstructorFinished != NULL)
                {
                    UnitAccessedAndConstructorNoFinished( pRunEventData->m_pUnit, pProcess);
                }
                else ReadyToRun(pProcess);
            }
            else
            {
                ReportError( "Cannot execute notification %s::%s : %s.", (CString)*pUnit->m_pIUnitType->m_stiName,
                    (CString)*pEvent->m_stiName, GetStatusText(status));
                pUnit->OnEventFinished( NULL, EVENT_NOTIFICATION, status, this);
                delete pProcess;
            }
        }
        else // EVENT_COMMAND
        {
            if (!pEvent->m_bIsCommand)
            {
                ReportError( "Attempting to execute the notification '%s'as command, skipping execution.", 
                    (CString)*pNotifyOrCommand->m_stiEventName);
                pUnit->OnEventFinished( NULL, EVENT_COMMAND, STATUS_COMMAND_NOTIFICATION_MISMATCH, this);
                goto Konec;
            }
            
            // current command
            if (pUnit->m_pCurrentCommandQueueFirst == pNotifyOrCommand) 
            {
                CIProcess *pProcess;
                pProcess = new CIProcess;
                
                if ( (status = pProcess->CreateEventCall(this, pUnit, pEvent, &pNotifyOrCommand->m_Bag, 
                    ((CZUnitCommandQueueItem*)pNotifyOrCommand)->m_pWaitingInterface,
                    CIProcess::PROCESS_EVENT_CURRENT_COMMAND)) == STATUS_SUCCESS)
                {
                    TRACE_INTERPRET3("- %d: Process %d (%s) JUST CREATED (current command)\n",  m_pCivilization->GetCivilizationIndex(), 
                        pProcess->m_nProcessID, (CString)*pProcess->m_stiName);
                    pUnit->m_nCommandProcessID = pProcess->m_nProcessID;
                    
                    if (pRunEventData->m_pUnit->m_pConstructorFinished != NULL)
                    {
                        UnitAccessedAndConstructorNoFinished( pRunEventData->m_pUnit, pProcess);
                    }
                    else ReadyToRun(pProcess);
                }
                else
                {
BadStatus:
                ReportError( "Cannot execute command %s::%s : %s.", 
                    (CString)*pUnit->m_pIUnitType->m_stiName, (CString)*pEvent->m_stiName, GetStatusText(status));
                pUnit->OnEventFinished( NULL, EVENT_COMMAND, status, this);
                delete pProcess;
                }
            }
            // mandatory command
            else if (pUnit->m_pMandatoryCommandQueueFirst == pNotifyOrCommand)
            {
                CIProcess *pProcess;
                pProcess = new CIProcess;
                
                if ( (status = pProcess->CreateEventCall( this, pUnit, pEvent, &pNotifyOrCommand->m_Bag, 
                    ((CZUnitCommandQueueItem*)pNotifyOrCommand)->m_pWaitingInterface,
                    CIProcess::PROCESS_EVENT_MANDATORY_COMMAND)) == STATUS_SUCCESS)
                {
                    TRACE_INTERPRET3("- %d: Process %d (%s) JUST CREATED (mandatory command)\n",  m_pCivilization->GetCivilizationIndex(), 
                        pProcess->m_nProcessID, (CString)*pProcess->m_stiName);
                    pUnit->m_nCommandProcessID = pProcess->m_nProcessID;
                    
                    if (pRunEventData->m_pUnit->m_pConstructorFinished != NULL)
                    {
                        UnitAccessedAndConstructorNoFinished( pRunEventData->m_pUnit, pProcess);
                    }
                    else ReadyToRun(pProcess);
                }
                else goto BadStatus;
            }
            // pokus o spusteni prikazu, ktery byl mezitim, co byl ve fronte Eventu zrusen
            else
            {
                TRACE_INTERPRET_DETAILS3("-- %d: Skipping execution of event %s::%s (current command)\n",  m_pCivilization->GetCivilizationIndex(), 
                    (CString)*pUnit->m_pIUnitType->m_stiName, (CString)*pEvent->m_stiName);
                
                // musim vzbutit ty, co by na to mohli cekat
                if ( ((CZUnitCommandQueueItem*)pNotifyOrCommand)->m_pWaitingInterface != NULL)
                {
                    ((CZUnitCommandQueueItem*)pNotifyOrCommand)->m_pWaitingInterface->FinishWaiting(STATUS_UNKNOWN_ERROR);
                }
                // konec
                goto Konec;
            }
        }
        
        break;
        
    case IMPL_CPLUSPLUS:
        // v C++ nejsou implementovany zadne udalosti
        ASSERT(false);
    }
    
Konec:
    // odemceni prace nad eventama
    m_pCivilization->UnlockEvents();
    
    delete pRunEventData;
}

///////////////////////////////////////////////////////////////////

void CInterpret::OnRunGlobalFunction(CIRunGlobalFunctionData *pRunGlobalFunctionData)
{
    ASSERT(pRunGlobalFunctionData != NULL);

    ASSERT(pRunGlobalFunctionData->m_stiName != NULL);

    CIGlobalFunction *pGlobalFunction;
    CIProcess *pProcess = NULL;
	EInterpretStatus status;
    
    TRACE_INTERPRET_DETAILS2("-- %d: Trying to run global function %s...\n",  m_pCivilization->GetCivilizationIndex(), 
        (CString)*pRunGlobalFunctionData->m_stiName);

    if ((pGlobalFunction = m_pCodeManager->GetGlobalFunction( pRunGlobalFunctionData->m_stiName)) == NULL)
    {
		ReportError( "Can't find global function '%s'.", (CString)*pRunGlobalFunctionData->m_stiName);
		
		pRunGlobalFunctionData->m_nProcessID = 0;
    }
	else
	{
		pProcess = new CIProcess;

		if ( ( status = pProcess->CreateGlobalFunctionCall(this, pGlobalFunction, &pRunGlobalFunctionData->m_Bag)) == STATUS_SUCCESS)
		{
			TRACE_INTERPRET3("- %d: Process %d (%s) JUST CREATED\n",  m_pCivilization->GetCivilizationIndex(), 
				pProcess->m_nProcessID, (CString)*pProcess->m_stiName);
        
			ReadyToRun(pProcess, true);

			pRunGlobalFunctionData->m_nProcessID = pProcess->m_nProcessID;
		}
		else
		{
			ReportError( "Cannot execute global function %s : %s.", 
				(CString)*pRunGlobalFunctionData->m_stiName, GetStatusText(status));
			delete pProcess;

			pRunGlobalFunctionData->m_nProcessID = 0;
		}
	}

	pRunGlobalFunctionData->m_ProcessEvent.SetEvent();
}

///////////////////////////////////////////////////////////////////

void CInterpret::OnKillProcess( SIKillProcessData *pKillProcessData)
{
	ASSERT( pKillProcessData != NULL);

	if ((m_pCurProcess != NULL) && (pKillProcessData->nProcessID == m_pCurProcess->m_nProcessID))
	{
		KillCurrentProcess( pKillProcessData->Status);
	}
	else
	{
		int i;
		CIProcess *pPom;

		// najit a zabit process
		for (i=0; i<2; i++)
		{
			pPom = (i==0) ? m_pFirstReadyToRun : m_pFirstSleeping;
			while ( pPom != NULL)
			{
				if (pPom->m_nProcessID == pKillProcessData->nProcessID) 
				{
					KillProcess( pPom, pKillProcessData->Status);
					goto Konec;
				}
				pPom = pPom->m_pNext;
			}
		}
	}

Konec:
	pKillProcessData->ProcessKilled.SetEvent();
	if (pKillProcessData->m_bDelete) delete pKillProcessData;
}

///////////////////////////////////////////////////////////////////

void CInterpret::OnPauseInterpret( SIPauseInterpret *pPauseInterpret)
{
    // pause interpret, predpoklada se, ze eventa je non-signaled
    TRACE_INTERPRET1("- %d: Interpret PAUSED\n", m_pCivilization->GetCivilizationIndex());
    pPauseInterpret->m_pIntepretPaused->SetEvent();
    pPauseInterpret->m_pFinishPause->Lock();
    TRACE_INTERPRET1("- %d: Interpret UNPAUSED\n", m_pCivilization->GetCivilizationIndex());

    delete pPauseInterpret;
}

///////////////////////////////////////////////////////////////////

void CInterpret::OnChangeUnitScript( SIChangeUnitScript *pChangeUnitScript)
{
	ASSERT( pChangeUnitScript != NULL);

    CStringTableItem *stiUnitName = g_StringTable.AddItem( pChangeUnitScript->m_strScriptName);
	CZUnit *pUnit = pChangeUnitScript->m_pUnit;
	CIUnitType *pNewUnitType;

	ASSERT( pUnit != NULL);

    if (( pNewUnitType = m_pCivilization->m_pCodeManager->GetUnitType( stiUnitName)) == NULL)
    {
        m_pCivilization->ReportError("Cannot found unit '%s'!", pChangeUnitScript->m_strScriptName);
		pChangeUnitScript->m_bSuccess = false;
		pChangeUnitScript->m_Done.SetEvent();
        return;
    }

    // vyprazdnit jednotce vsechny fronty
    pUnit->EmptyMandatoryCommandQueue( false );
    pUnit->EmptyCurrentCommandQueue( false );

    // zabit vsechny procesy, ktere bezi na jednotce
    KillAllProcessesOfUnit( pUnit);

    // uvolnit stare datove polozky
    if (pUnit->m_pIUnitType->m_nHeapSize != 0) 
    {
		CIUnitType *pIUnitType;
		POSITION pos;
		CIMember *pMember;
		CStringTableItem *stiName;

		pIUnitType = pUnit->m_pIUnitType;
		while (pIUnitType != CCodeManager::m_pDefaultUnitType)
		{
			pos = pIUnitType->m_tpMembers.GetStartPosition();
			while (pos != NULL)
			{
				pIUnitType->m_tpMembers.GetNextAssoc( pos, stiName, pMember);
				
				if (pMember->m_DataType.IsObjectUnitStruct() &&
                    ((CIObjUnitStruct*)pUnit->m_pData[ pMember->m_nHeapOffset]) != NULL)
                {
                    ((CIObjUnitStruct*)pUnit->m_pData[ pMember->m_nHeapOffset])->Release();
                }
            }

			pIUnitType = pIUnitType->m_pParent;
        }
    }
    delete [] pUnit->m_pData;
    pUnit->m_pData = NULL;

    // prevlect typ jednotky
    pUnit->m_pIUnitType = pNewUnitType;

    // vytvorit nove datove polozky    
    if (pUnit->m_pIUnitType->m_nHeapSize != 0) 
    {
        pUnit->m_pData = new int[ pUnit->m_pIUnitType->m_nHeapSize];
        ::ZeroMemory( pUnit->m_pData, sizeof(int)* pUnit->m_pIUnitType->m_nHeapSize);

		CIUnitType *pIUnitType;
		POSITION pos;
		CIMember *pMember;
		CStringTableItem *stiName;

		pIUnitType = pUnit->m_pIUnitType;
		while (pIUnitType->m_stiName != m_pCivilization->m_pInterpret->m_stiDefaultUnit)
		{
			pos = pIUnitType->m_tpMembers.GetStartPosition();
			while (pos != NULL)
			{
				pIUnitType->m_tpMembers.GetNextAssoc( pos, stiName, pMember);
				
				if (pMember->m_DataType.IsObject())
				{
					pUnit->m_pData[ pMember->m_nHeapOffset] = (int)m_pCivilization->m_pInterpret->CreateSystemObject(pMember->m_DataType);
					if ( pUnit->m_pData[ pMember->m_nHeapOffset] == NULL) 
					{
						m_pCivilization->ReportError("Error while creating unit '%s': cannot create member '%s', no such system object.",
							(CString)*pUnit->m_pIUnitType->m_stiName, (CString)*pMember->m_stiName);
					}
				}
				else if (pMember->m_DataType.IsStruct())
				{
					ASSERT(pMember->m_DataType.m_stiName != NULL);

					CIStructureType *pStructType;
					if (( pStructType = m_pCivilization->m_pCodeManager->GetStructureType( pMember->m_DataType.m_stiName)) == NULL)
					{
						m_pCivilization->ReportError("Error while creating unit '%s': cannot create member '%s %s', no such structure.",
							(CString)*pUnit->m_pIUnitType->m_stiName, (CString)*pMember->m_DataType.m_stiName, (CString)*pMember->m_stiName);
					}
					else
					{
						pUnit->m_pData[ pMember->m_nHeapOffset] = (int)m_pCivilization->m_pInterpret->CreateStructure( pStructType, true);
					}
				}
			}

			pIUnitType = pIUnitType->m_pParent;
		}
    }

    // zavolat konstruktor
    if (pUnit->m_pConstructorFinished == NULL) 
    {
        pUnit->m_pConstructorFinished = new CIConstructorFinishedWaiting();
        pUnit->m_pConstructorFinished->Create();
    }
    InlayEvent( RUN_CONSTRUCTOR, (DWORD) pUnit);

    stiUnitName->Release();

	pChangeUnitScript->m_bSuccess = true;
	pChangeUnitScript->m_Done.SetEvent();
}

///////////////////////////////////////////////////////////////////

void CInterpret::OnUpdateUnitCode( SIUpdateUnitCode *psUpdateUnitCode)
{
    CIChangedMethodEventsList *pMethodEvent;
    CIProcess *pProcess, *pPom;
    int i;

    // zaradit current process na zacatek ReadyToRun fronty
    if (m_nState != IDLE)
    {
        AddProcessToListAtBeginWithLastPointer( ReadyToRun, m_pCurProcess);
        m_pCurProcess = NULL;
    }

// ZABIJENI
    // zabit vsechny procesy, ktere pouzivaji neco z menenych metod a eventu
    for (i=0; i<2; i++)
    {
        // ReadyToRun fronta
        if (i==0) pProcess = m_pFirstReadyToRun;
        else pProcess = m_pFirstSleeping;

Continue:
        while (pProcess != NULL)
        {
			// konstruktor
			if ( psUpdateUnitCode->m_bConstructorChanged && 
                 pProcess->m_nProcessType == CIProcess::PROCESS_CONSTRUCTOR &&
				 pProcess->ContainsUnitOfType( psUpdateUnitCode->m_pIUnitType))
			{
                pPom = pProcess;
                pProcess = pProcess->m_pNext;

                // a zaridit znovaspusteni
                InlayEvent( RUN_CONSTRUCTOR, (DWORD)pPom->m_pUnit);

                // zabit starou verzi
                KillProcess( pPom, STATUS_UPDATE);

                // pokracujeme
                goto Continue;
			}

            // destruktor
			if ( psUpdateUnitCode->m_bDestructorChanged &&
				 pProcess->m_nProcessType == CIProcess::PROCESS_DESTRUCTOR &&
				 pProcess->ContainsUnitOfType( psUpdateUnitCode->m_pIUnitType))
            {
                pPom = pProcess;
                pProcess = pProcess->m_pNext;

                // zabit starou verzi
                KillProcess( pPom, STATUS_UPDATE);

                // pokracujeme
                goto Continue;
            }

			// ostatni
            pMethodEvent = psUpdateUnitCode->m_pChangedMethodEvents;
            while (pMethodEvent != NULL)
            {
                if (pProcess->ContainsMethodOrEventOfType( psUpdateUnitCode->m_pIUnitType->m_stiName, pMethodEvent->m_stiName))
                {
                    pPom = pProcess;
                    pProcess = pProcess->m_pNext;

                    // zabit starou verzi
                    KillProcess( pPom, STATUS_UPDATE);

                    // pokracujeme
                    goto Continue;
                }
                pMethodEvent = pMethodEvent->m_pNext;
            }

            pProcess = pProcess->m_pNext;
        }
    }

    // spustit zastaveny process
    RunNextProcess(false);

	// vzbudit codemanagera
	psUpdateUnitCode->m_pJobDone->SetEvent();

	// pockat az to dodela
	psUpdateUnitCode->m_pDoJob->Lock();

	// vzbudit codemanagera, aby mohl zrusit eventy
	psUpdateUnitCode->m_pJobDone->SetEvent();
}

///////////////////////////////////////////////////////////////////

void CInterpret::OnUpdateUnitAll( SIUpdateUnitAll* psUpdateUnitAll)
{
    int nUnitsUpdated = 0; // pocitadlo

// Zjistim, co vsechno (jake polozky) updatovane jednotky se budou kopirovat (krome polozek
// predku a potomku). V poli si zapamatuju vychozi a cilovy heap-offset.
    CArray<int, int> aSameMemberOldOffset, aSameMemberNewOffset;
    CArray<bool, bool> aSameMemberIsObjUnitStruct;
    int nSameMemberArraySize = 0;
    POSITION pos;
    CStringTableItem *stiName;
    CIMember *pMember1, *pMember2;

    pos = psUpdateUnitAll->m_pOldUnitType->m_tpMembers.GetStartPosition();
    while (pos != NULL)
    {
        psUpdateUnitAll->m_pOldUnitType->m_tpMembers.GetNextAssoc( pos, stiName, pMember1);
        if ( ( psUpdateUnitAll->m_pNewUnitType->GetMember( stiName, pMember2)) == IMPL_SCRIPT &&
              pMember1->m_DataType == pMember2->m_DataType)
        {
            aSameMemberIsObjUnitStruct.SetAtGrow( nSameMemberArraySize, pMember1->m_DataType.IsObjectUnitStruct());
            aSameMemberOldOffset.SetAtGrow( nSameMemberArraySize, pMember1->m_nHeapOffset);
            aSameMemberNewOffset.SetAtGrow( nSameMemberArraySize, pMember2->m_nHeapOffset);
            nSameMemberArraySize++;
            if (pMember1->m_DataType.IsFloat())  // float zabira dve bunky v poli polozek (int[])
            {
                aSameMemberIsObjUnitStruct.SetAtGrow( nSameMemberArraySize, false);
                aSameMemberOldOffset.SetAtGrow( nSameMemberArraySize, pMember1->m_nHeapOffset+1);
                aSameMemberNewOffset.SetAtGrow( nSameMemberArraySize, pMember2->m_nHeapOffset+1);
                nSameMemberArraySize++;
            }
        }
    }

// ZABIJENI - Zabit vsechny procesy, ktere pouzivaji updatovanou jednotku nebo jejiho potomka
    // zaradit current process na zacatek ReadyToRun fronty
    if (m_nState != IDLE)
    {
        AddProcessToListAtBeginWithLastPointer( ReadyToRun, m_pCurProcess);
        m_pCurProcess = NULL;
    }

    int i;
    CIProcess *pProcess, *pPomProcess;

    for (i=0; i<2; i++)
    {
        // ReadyToRun fronta
        if (i==0) pProcess = m_pFirstReadyToRun;
        // Sleeping fronta
        else pProcess = m_pFirstSleeping;

Continue:
        while (pProcess != NULL)
        {
            if ( OnUpdateUnitAll_ProcessUsesOneOfUnits( pProcess, 
                                                       psUpdateUnitAll->m_pOldUnitType,
                                                       psUpdateUnitAll->m_pChildren))
            {
                // konstruktor
                if (pProcess->m_nProcessType == CIProcess::PROCESS_CONSTRUCTOR)
                {
                    // a zaridit znovaspusteni
                    InlayEvent( RUN_CONSTRUCTOR, (DWORD)pProcess->m_pUnit);
                }

                pPomProcess = pProcess;
                pProcess = pProcess->m_pNext;

                // zabit starou verzi
                KillProcess( pPomProcess, STATUS_UPDATE);

                // pokracujeme
                goto Continue;
            }

            pProcess = pProcess->m_pNext;
        }
    }

// update vsech instanci daneho typu + zkopirovani stejnych polozek
    CZUnitListItem *pInstances = m_pCivilization->GetAllUnitsOfTypeAndChildren( psUpdateUnitAll->m_pOldUnitType);
    CZUnitListItem *pPom = pInstances;
    CIUnitType *pUnitType;
	CIBag Bag;
    int *pData;

	Bag.Create();

    while (pPom != NULL)
    {
        pUnitType = pPom->m_pUnit->m_pIUnitType;

        // naalokovat misto na nove polozky
        pData = new int[ pUnitType->m_nHeapSize +
                         psUpdateUnitAll->m_nHeapOffsetDelta];
        ::ZeroMemory( pData, pUnitType->m_nHeapSize + psUpdateUnitAll->m_nHeapOffsetDelta);

		/* // TODO: ODLADIT!!!!!!
        // zkopirovat polozky od potomka updatovane jednotky
        if (pUnitType->m_nHeapSize - psUpdateUnitAll->m_pOldUnitType->m_nHeapSize != 0)
        {
            memcpy( &pData[ psUpdateUnitAll->m_pNewUnitType->m_nHeapSize],
                    &pPom->m_pUnit->m_pData[ psUpdateUnitAll->m_pOldUnitType->m_nHeapSize],
                    pUnitType->m_nHeapSize - psUpdateUnitAll->m_pOldUnitType->m_nHeapSize);
        }

        // zkopirovat polozky od predku
        if ( psUpdateUnitAll->m_pOldUnitType->m_pParent->m_nHeapSize != 0)
        {
            memcpy (&pData, pPom->m_pUnit->m_pData, 
                    psUpdateUnitAll->m_pOldUnitType->m_pParent->m_nHeapSize);
        }

        // zkopirovat polozky od aktualni jednotky (ty, co zbyly)
        for (i=0; i<nSameMemberArraySize; i++)
        {
            // presunume tam starou hodnotu
            pData[ aSameMemberNewOffset[i]] = pPom->m_pUnit->m_pData[ aSameMemberOldOffset[i]];
            pPom->m_pUnit->m_pData[ aSameMemberOldOffset[i]] = 0;
        }
		*/

        // vymenit prostory datovych polozek a starou verzi uvolnit
        delete pPom->m_pUnit->m_pData;
        pPom->m_pUnit->m_pData = pData;

		pPom->m_pUnit->SendNotification( CZCivilization::m_stiUNIT_UPDATE, &Bag);

		pPom = pPom->m_pNext;

        nUnitsUpdated++;
    }

	Bag.Delete();

	// znovy spustit zastaveny process
	RunNextProcess(false);


	// vzbudit codemanagera
	psUpdateUnitAll->m_pJobDone->SetEvent();

	// pockat az to dodela
	psUpdateUnitAll->m_pDoJob->Lock();

	// vzbudit codemanagera, aby mohl zrusit eventy
	psUpdateUnitAll->m_pJobDone->SetEvent();


    TRACE_INTERPRET1("- Update units: %d units updated\n", nUnitsUpdated);
}

///////////////////////////////////////////////////////////////////

bool CInterpret::OnUpdateUnitAll_ProcessUsesOneOfUnits( CIProcess *pProcess, 
                                                       CIUnitType *pIUnitType, 
                                                       CIUnitTypeList *pChildren)
{
    if ( pProcess->ContainsUnitOfType( pIUnitType)) return true;

    CIUnitTypeList *pPom = pChildren;
    while (pPom != NULL)
    {
        if ( pProcess->ContainsUnitOfType( pPom->m_pIUnitType)) return true;
        pPom = pPom->m_pNext;
    }

    return false;
}

///////////////////////////////////////////////////////////////////

void CInterpret::OnUpdateStruct( SIUpdateStruct* psUpdateStruct)
{
    // Update vsech instanci dane struktury
    int nStructuresUpdated = 0; // pocitadlo

    // Nejdriv si zjistime, co vsechno (jake polozky) se budou kopirovat.
    // V poli si zapamatujeme vychozi a cilovy heap-offset.
    CArray<int, int> aSameMemberOldOffset, aSameMemberNewOffset;
    CArray<bool, bool> aSameMemberIsObjUnitStruct;
    int nSameMemberArraySize = 0;
    POSITION pos;
    CStringTableItem *stiName;
    CIMember *pMember1, *pMember2;

    pos = psUpdateStruct->m_pOldStructType->m_tpMembers.GetStartPosition();
    while (pos != NULL)
    {
        psUpdateStruct->m_pOldStructType->m_tpMembers.GetNextAssoc( pos, stiName, pMember1);
        if (psUpdateStruct->m_pNewStructType->m_tpMembers.Lookup( stiName, pMember2) &&
            *pMember1 == *pMember2)
        {
            aSameMemberIsObjUnitStruct.SetAtGrow( nSameMemberArraySize, pMember1->m_DataType.IsObjectUnitStruct());
            aSameMemberOldOffset.SetAtGrow( nSameMemberArraySize, pMember1->m_nHeapOffset);
            aSameMemberNewOffset.SetAtGrow( nSameMemberArraySize, pMember2->m_nHeapOffset);
            nSameMemberArraySize++;
            if (pMember1->m_DataType.IsFloat())  // float zabira dve bunky v poli polozek (int[])
            {
                aSameMemberIsObjUnitStruct.SetAtGrow( nSameMemberArraySize, aSameMemberIsObjUnitStruct[ nSameMemberArraySize-1]);
                aSameMemberOldOffset.SetAtGrow( nSameMemberArraySize, pMember1->m_nHeapOffset+1);
                aSameMemberNewOffset.SetAtGrow( nSameMemberArraySize, pMember2->m_nHeapOffset+1);
                nSameMemberArraySize++;
            }
        }
    }

    // update vsech instanci daneho typu + zkopirovani stejnych polozek
    m_mutexStructures.Lock();

    CIStructure *pStruct, *pNewStruct;
    int *pPom;   
    int i;

    // cyklus pres vsechny struktury
    pos = m_tpStructures.GetStartPosition();
    while (pos != NULL)
    {
        m_tpStructures.GetNextAssoc( pos, pStruct, pStruct);

        // nasli jsme strukturu daneho typu
        if (pStruct->m_pStructType == psUpdateStruct->m_pOldStructType)
        {
            // vytvorime novou instanci struktury (s jiz spravnym updatovanym typem)
            pNewStruct = CreateStructure( psUpdateStruct->m_pNewStructType, true);

            // kopie hodnot stejnych polozek do noveho prostoru
            for (i=0; i<nSameMemberArraySize; i++)
            {
                // zrusime prave vytvorenou novou polozku (kdyz reference)
                if (aSameMemberIsObjUnitStruct[i] && pNewStruct->m_pData[  aSameMemberNewOffset[i]] != NULL) 
                {
                    ((CIObjUnitStruct*)pNewStruct->m_pData[  aSameMemberNewOffset[i]])->Release();
                }

                // presunume tam starou hodnotu
                pNewStruct->m_pData[ aSameMemberNewOffset[i]] = pStruct->m_pData[ aSameMemberOldOffset[i]];

                // starou referenci vynulujeme
                pStruct->m_pData[ aSameMemberOldOffset[i]] = 0;
            }

            // vymenime polozky
            pPom = pStruct->m_pData;
            pStruct->m_pData = pNewStruct->m_pData;
            pNewStruct->m_pData = pPom;
            
            // zrusime novou strukturu
            pNewStruct->Release();

            nStructuresUpdated++;
        }
    }
    m_mutexStructures.Unlock();

    // hotovo, vzbudit codemanagera, aby updatoval kod
    psUpdateStruct->m_pJobDone->SetEvent();

    // pockat, az to dokonci
    psUpdateStruct->m_pDoJob->Lock();

    // ted codemanager ceka, az se vzbudim, aby mohl smazat ty eventy,
    // tak ho vzbudim
    psUpdateStruct->m_pJobDone->SetEvent();

    TRACE_INTERPRET1("- Update structure: %d structures updated\n", nStructuresUpdated);
}

///////////////////////////////////////////////////////////////////

void CInterpret::OnUpdateGlobalFunction( SIUpdateGlobalFunction* psUpdateGlobalFunction)
{
    CIProcess *pProcess, *pPom;
    CStringTableItem *stiGFUnitName = g_StringTable.AddItem(GLOBAL_FUNCTIONS_CLASS_NAME);
    int i;

    // zaradit current process na zacatek ReadyToRun fronty
    if (m_nState != IDLE)
    {
        AddProcessToListAtBeginWithLastPointer( ReadyToRun, m_pCurProcess);
        m_pCurProcess = NULL;
    }

// ZABIJENI
    // zabit vsechny procesy, ktere pouzivaji menenou globalni funkci
    for (i=0; i<2; i++)
    {
        // ReadyToRun fronta
        if (i==0) pProcess = m_pFirstReadyToRun;
        else pProcess = m_pFirstSleeping;

Continue:
        while (pProcess != NULL)
        {
            if (pProcess->ContainsMethodOrEventOfType( stiGFUnitName, psUpdateGlobalFunction->m_stiGlobalFunctionName))
            {
                // konstruktor
                if (pProcess->m_nProcessType == CIProcess::PROCESS_CONSTRUCTOR)
                {
                    pPom = pProcess;
                    pProcess = pProcess->m_pNext;

                    // a zaridit znovaspusteni
                    InlayEvent( RUN_CONSTRUCTOR, (DWORD)pPom->m_pUnit);

                    // zabit starou verzi
                    KillProcess( pPom, STATUS_UPDATE);

                    // pokracujeme
                    goto Continue;
                }
                // cokoli jineho
                else 
                {
                    pPom = pProcess;
                    pProcess = pProcess->m_pNext;

                    // zabit starou verzi
                    KillProcess( pPom, STATUS_UPDATE);

                    // pokracujeme
                    goto Continue;
                }
            }

            pProcess = pProcess->m_pNext;
        }
    }

    // spustit zastaveny process
    RunNextProcess(false);


    // hotovo, vzbudit codemanagera, aby updatoval kod
    psUpdateGlobalFunction->m_pJobDone->SetEvent();

    // pockat, az to dokonci
    psUpdateGlobalFunction->m_pDoJob->Lock();

    // ted codemanager ceka, az se vzbudim, aby mohl smazat ty eventy,
    // tak ho vzbudim
    psUpdateGlobalFunction->m_pJobDone->SetEvent();

    stiGFUnitName->Release();
}

///////////////////////////////////////////////////////////////////
// hlaseni chyb

void CInterpret::ReportError(LPCTSTR strErrorMsgFormat, ...)
{
    ASSERT( m_pCivilization != NULL);

    va_list args;
    va_start(args, strErrorMsgFormat);

    m_pCivilization->ReportError( strErrorMsgFormat, args);
}

///////////////////////////////////////////////////////////////////
// vytvareni systemovych objektu

CISystemObject* CInterpret::CreateSystemObject(ESystemObjectType nSOType, bool bCallCreate /*= true*/)
{
    CISystemObject* pObj;

    switch (nSOType)
    {
    case SO_ARRAY:
    case SO_SET:
        // pole a mnoziny takto nelze vytvaret, musi se volat CreateArray nebo CreateSet
        ASSERT(false);

    case SO_STRING:     pObj = new CISOString();    break;
    case SO_POSITION:   pObj = new CISOPosition();  break;
    case SO_MAP:        pObj = new CISOMap();       break;
    case SO_CLIENT:     pObj = new CISOClient();    break;
    case SO_PATH:       pObj = new CISOPath();      break;
    case SO_RESOURCES:  pObj = new CISOResources(); break;
	/*SYSOB*/

    // Skills and non-instantiable objects
    case SO_SKILLINTERFACE:
    case SO_EXAMPLESKILL:
    case SO_MOVESKILL:
    case SO_MAKESKILL:
    case SO_BULLETATTACKSKILL:
    case SO_BULLETDEFENSESKILL:
        ASSERT(false);
        break;

    case SO_UNKNOWN:
    case SO_NONE:
    default:
        {
            TRACE_INTERPRET_DETAILS1("-- Don't know how to create object with SOType '%d'\n", nSOType);
            ASSERT(false);
            return NULL;
        }
    }
    
	if ( pObj == NULL) return NULL;
	
    if ( bCallCreate && !pObj->Create( this)) 
	{
		delete pObj;
		return NULL;
	}
	pObj->AddRef();
	RegisterSystemObject( pObj);

	return pObj;
}

CISystemObject* CInterpret::CreateSystemObject(CIType &IType, bool bCallCreate /*= true*/)
{
    if (!IType.IsObject())
    {
        ASSERT(false);
        return NULL;
    }

    switch (IType.m_nSOType)
    {
    case SO_ARRAY:
        return CreateArray(IType.m_pOfType, IType.m_nArraySize, bCallCreate);
        break;
    case SO_SET:
        return CreateSet(IType.m_pOfType, bCallCreate);
        break;
    default:
        return CreateSystemObject(IType.m_nSOType, bCallCreate);
    }
}

/////////////////////
// POMOCNE METODY
/////////////////////

CISOArrayParent* CInterpret::CreateArray(CType *pOfType, int nArraySize, bool bCallCreate /*= true*/)
{
    CISOArrayParent *pObj;

    ASSERT( nArraySize > 0);
    ASSERT( pOfType != NULL);

    switch (pOfType->m_nType)
    {
    case T_CHAR:    pObj = new CISOArray<char>;             break;
    case T_INT:     pObj = new CISOArray<int>;              break;
    case T_FLOAT:   pObj = new CISOArray<double>;           break;
    case T_BOOL:    pObj = new CISOArray<bool>;             break;
    case T_OBJECT:  pObj = new CISOArray<CISystemObject*>;  break;
    case T_UNIT:    pObj = new CISOArray<CZUnit*>;          break;
    case T_STRUCT:  pObj = new CISOArray<CIStructure*>;     break;
    default:  ASSERT(false);
    }

	if ( pObj == NULL) return NULL;

    if ( bCallCreate && !pObj->Create( this, pOfType, nArraySize)) 
	{
		delete pObj;
		return NULL;
	}
	pObj->AddRef();
	RegisterSystemObject( pObj);

    return pObj;
}

CISystemObject* CInterpret::CreateSet(CType *pOfType, bool bCallCreateEmptyRoutineAndAddRef)
{
    CISOSetParent *pObj;

    ASSERT( pOfType != NULL);
	ASSERT( !pOfType->IsSet());
	ASSERT( !pOfType->IsArray());

    switch (pOfType->m_nType)
    {
    case T_CHAR:    pObj = new CISOSet<char>;             break;
    case T_INT:     pObj = new CISOSet<int>;              break;
    case T_FLOAT:   pObj = new CISOSet<double>;           break;
    case T_BOOL:    pObj = new CISOSet<bool>;             break;
    case T_OBJECT:  pObj = new CISOSet<CISystemObject*>;  break;
    case T_UNIT:    pObj = new CISOSet<CZUnit*>;          break;
    case T_STRUCT:  pObj = new CISOSet<CIStructure*>;     break;
    default:  ASSERT(false);
    }

	if ( pObj == NULL) return NULL;

	if ( bCallCreateEmptyRoutineAndAddRef && !pObj->CreateEmpty( this, pOfType)) 
	{
		delete pObj;
		return NULL;
	}
	pObj->AddRef();
	RegisterSystemObject( pObj);

    return pObj;
}

CISkillInterface* CInterpret::CreateSkillInterface( CZUnit* pZUnit, CSSkillType *pSkillType)
{
    CISkillInterface *pSkillInterface = new CISkillInterface;

    if ( !pSkillInterface->Create( this, pZUnit, pSkillType))
    {
        delete pSkillInterface;
        return NULL;
    }

    pSkillInterface->AddRef();

    RegisterSystemObject( pSkillInterface);

    return pSkillInterface;
}

//////////////////////////////////////////////////////////////////////////////////////
// vytvareni struktur

CIStructure* CInterpret::CreateStructure( CIStructureType *pStructType, bool bCreateMembers)
{
	CIStructure *pStruct;

    pStruct = new CIStructure();
    pStruct->Create(this, pStructType, bCreateMembers);
    pStruct->AddRef();
	RegisterStructure( pStruct);
	return pStruct;
}

//////////////////////////////////////////////////////////////////////////////////////
// Registrace system.objektu a struktur
//////////////////////////////////////////////////////////////////////////////////////

void CInterpret::RegisterSystemObject( CISystemObject *pSystemObject)
{
	m_mutexSystemObjects.Lock();
	m_tpSystemObjects.SetAt( pSystemObject, pSystemObject);
	m_nSystemObjectCount++;
	m_mutexSystemObjects.Unlock();
}

void CInterpret::UnregisterSystemObject( CISystemObject *pSystemObject)
{
	m_mutexSystemObjects.Lock();
	m_tpSystemObjects.RemoveKey( pSystemObject);
	m_nSystemObjectCount--;
	m_mutexSystemObjects.Unlock();
	return;
}

void CInterpret::RegisterStructure( CIStructure *pStructure)
{
	m_mutexStructures.Lock();
	m_tpStructures.SetAt( pStructure, pStructure);
	m_nStructureCount++;
	m_mutexStructures.Unlock();
}

void CInterpret::UnregisterStructure( CIStructure *pStructure)
{
	m_mutexStructures.Lock();
	m_tpStructures.RemoveKey( pStructure);
	m_nStructureCount--;

	m_mutexStructures.Unlock();
	return;
}

//////////////////////////////////////////////////////////////////////////////////////
//	SAVE & LOAD
//////////////////////////////////////////////////////////////////////////////////////

void CInterpret::PersistentSave( CPersistentStorage &storage)
{
// Vyprazdnit frontu zprav 
    BRACE_BLOCK(storage);
    
    int nRet = 0;
    DWORD dwQuitParam;
    while ((nRet = m_pEventManager->DoEvent(&dwQuitParam))!=0)
    {
        // ukonceni interpretace  - quit event -> chyba, tohle by se tu nemelo objevit
        if (nRet == -1) 
        {
            ASSERT(false);
            return;
        }
    }
    

// Pridat current process na zacatek ready-to-run fronty
    if (m_nState != IDLE) AddProcessToListAtBeginWithLastPointer( ReadyToRun, m_pCurProcess);
    m_pCurProcess = NULL;

// Ulozit se
	// pointer na sebe
	storage << (DWORD)this;

	// notifier
	CNotifier::PersistentSave( storage);

    // civilizace
    storage << (DWORD)m_pCivilization;

    // event manager
    storage << (DWORD)m_pEventManager;

	// code manager
    storage << (DWORD)m_pCodeManager;

	// stav interpretu
    storage << (DWORD)m_nState;

    // kvantum prave interpretovaneho procesu.
    // Prave interpretovany process == prvni ulozeny process
    if (m_nState == RUNNING) 
	{
	    storage << m_nQuantum;
	}

    // prvni volny process ID
    storage << m_nFirstFreeProcessID;

// ULOZIT VSECHNY OBJEKTY A STRUKTURY

	POSITION pos;
	CISystemObject *pSystemObject;
	CIStructure *pStructure;

	storage << m_nSystemObjectCount;

	// ulozeni objektu
	pos = m_tpSystemObjects.GetStartPosition();
	while (pos != NULL)
	{
		m_tpSystemObjects.GetNextAssoc( pos, pSystemObject, pSystemObject);
		PersistentSaveSystemObject( storage, pSystemObject);
	}

	// spocitame, kolik je struktur 
	storage << m_nStructureCount;

	// ulozeni struktur
	pos = m_tpStructures.GetStartPosition();
	while (pos != NULL)
	{
		m_tpStructures.GetNextAssoc( pos, pStructure, pStructure);
		pStructure->PersistentSave( storage);
	}

// ULOZIT PROCESY - obe fronty za sebou
	CIProcess *pProcess;

	// pocet procesu
#ifdef _DEBUG
    int nCount;

	for ( nCount=0, pProcess = m_pFirstReadyToRun; pProcess != NULL; pProcess = pProcess->m_pNext) nCount++;
    for (           pProcess = m_pFirstSleeping; pProcess != NULL; pProcess = pProcess->m_pNext) nCount++;

    ASSERT( nCount == m_nProcessCount);
#endif

	storage << m_nProcessCount;

	// fronta ReadyToRun procesu
	for ( pProcess = m_pFirstReadyToRun; pProcess != NULL; pProcess = pProcess->m_pNext) 
	{
		pProcess->PersistentSave( storage);
	}
	
	// fronta Sleeping
	for ( pProcess = m_pFirstSleeping; pProcess != NULL; pProcess = pProcess->m_pNext) 
	{
		pProcess->PersistentSave( storage);
	}

	// zacatky a konce front
	storage << (DWORD)m_pFirstReadyToRun << (DWORD)m_pLastReadyToRun << (DWORD)m_pFirstSleeping;

	// jmeno defautni jednotky
    storage << (DWORD)m_stiDefaultUnit;

// Znova rozbehnout current process
    RunNextProcess( false);
}

//////////////////////////////////////////////////////////////////////////////////////

void CInterpret::PersistentLoad( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    // pointer na sebe
	CInterpret *pThis;
	storage >> (DWORD&)pThis;
	storage.RegisterPointer( pThis, this);

	// notifier
	CNotifier::PersistentLoad( storage);

    // civilizace
    storage >> (DWORD&)m_pCivilization;

    // event manager
    storage >> (DWORD&)m_pEventManager;

	// code manager
    storage >> (DWORD&)m_pCodeManager;

	// stav interpretu
    storage >> (DWORD&)m_nState;

    // kvantum prave interpretovaneho procesu
    if (m_nState == RUNNING) storage >> m_nQuantum;
	else m_nQuantum = 0;

	m_pCurProcess = NULL;

    // prvni volny process ID
    storage >> m_nFirstFreeProcessID;

// NACIST VSECHNY OBJEKTY A STRUKTURY
	int i;

	// objekty
    ASSERT( m_nSystemObjectCount == 0);

    long nSystemObjectCount;
	storage >> nSystemObjectCount;

	if (nSystemObjectCount != 0)
	{
        CISystemObject *pSystemObject;

		// pri loadovani se i automaticky zaregistruje
		for (i=0; i<nSystemObjectCount; i++) 
        {
            // v PersistentLoadSystemObject se objekty i registruji
            pSystemObject = PersistentLoadSystemObject( storage);
        }
	}

    ASSERT( nSystemObjectCount == m_nSystemObjectCount);

	// struktury
    ASSERT( m_nStructureCount == 0);

    long nStructureCount;
	storage >> nStructureCount;

	if (nStructureCount != 0)
	{
		for (i=0; i<nStructureCount; i++) 
		{
			CIStructure *pStructure = new CIStructure;
			pStructure->PersistentLoad( storage);
            RegisterStructure( pStructure);
		}
	}

    ASSERT( m_nStructureCount == nStructureCount);


// NACIST PROCESY
	storage >> m_nProcessCount;

	if (m_nProcessCount == 0)
	{
		m_ppPersistentLoadProcesses = NULL;
	}
	else
	{
		m_ppPersistentLoadProcesses = (CIProcess**)new char [sizeof(CIProcess*) * m_nProcessCount];
		ZeroMemory( m_ppPersistentLoadProcesses, sizeof(CIProcess*) * m_nProcessCount);

		for (i=0; i<m_nProcessCount; i++)
		{
			m_ppPersistentLoadProcesses[i] = new CIProcess;
			m_ppPersistentLoadProcesses[i]->PersistentLoad( storage);
		}
	}

	// zacatky a konce front
	storage >> (DWORD&)m_pFirstReadyToRun >> (DWORD&)m_pLastReadyToRun >> (DWORD&)m_pFirstSleeping;

	// jmeno defautni jednotky
    storage >> (DWORD&)m_stiDefaultUnit;
}

//////////////////////////////////////////////////////////////////////////////////////

void CInterpret::PersistentTranslatePointers( CPersistentStorage &storage)
{
	// notifier
	CNotifier::PersistentTranslatePointers( storage);

    // civilizace
    m_pCivilization = (CZCivilization*)storage.TranslatePointer(m_pCivilization);

    // event manager
    m_pEventManager = (CEventManager*) storage.TranslatePointer( m_pEventManager);

	// code manager
    m_pCodeManager = (CCodeManager*) storage.TranslatePointer( m_pCodeManager);

// OBJEKTY A STRUKTURY
	int i;

	// objekty
	POSITION pos = m_tpSystemObjects.GetStartPosition();
    while (pos != NULL)
    {
        CISystemObject *pTmp, *pSystemObject;
        m_tpSystemObjects.GetNextAssoc( pos, pTmp, pSystemObject);
        ASSERT( pTmp == pSystemObject);
        PersistentTranslatePointersSystemObject( storage, pSystemObject);
    }
			
	// struktury
    pos = m_tpStructures.GetStartPosition();
    while (pos != NULL)
    {
        CIStructure *pTmp, *pStructure;
        m_tpStructures.GetNextAssoc( pos, pTmp, pStructure);
        ASSERT( pTmp == pStructure);
		pStructure->PersistentTranslatePointers( storage);
    }

// PRELOZIT PROCESY
	for (i=0; i<m_nProcessCount; i++)
	{
		m_ppPersistentLoadProcesses[i]->PersistentTranslatePointers( storage);
	}

	// zacatky a konce front
	m_pFirstReadyToRun = (CIProcess*) storage.TranslatePointer( m_pFirstReadyToRun);
	m_pLastReadyToRun = (CIProcess*) storage.TranslatePointer( m_pLastReadyToRun);
	m_pFirstSleeping= (CIProcess*) storage.TranslatePointer( m_pFirstSleeping);

	// jmeno defautni jednotky
    m_stiDefaultUnit = (CStringTableItem*)storage.TranslatePointer( m_stiDefaultUnit);
}

//////////////////////////////////////////////////////////////////////////////////////

void CInterpret::PersistentInit()
{
	// notifier
	CNotifier::PersistentInit();


// OBJEKTY A STRUKTURY
	int i;

	// objekty
    POSITION pos = m_tpSystemObjects.GetStartPosition();
    while (pos != NULL)
    {
        CISystemObject *pTmp, *pSystemObject;
        m_tpSystemObjects.GetNextAssoc( pos, pTmp, pSystemObject);
		PersistentInitSystemObject( pSystemObject);
    }
			
	// struktury
    pos = m_tpStructures.GetStartPosition();
    while (pos != NULL)
    {
        CIStructure *pTmp, *pStructure;
        m_tpStructures.GetNextAssoc( pos, pTmp, pStructure);
		pStructure->PersistentInit();
    }

// PRELOZIT PROCESY
	if (m_ppPersistentLoadProcesses != NULL)
	{
		for (i=0; i<m_nProcessCount; i++)
		{
			m_ppPersistentLoadProcesses[i]->PersistentInit();
		}

		delete [] m_ppPersistentLoadProcesses;
		m_ppPersistentLoadProcesses = NULL;
	}

	// nastavit bezici process
	if (m_nState != IDLE) RunNextProcess(false);
}

//////////////////////////////////////////////////////////////////////////////////////
	
#define _CallRoutineOnArray( pobject, routine, args)	\
		switch (((CISOArrayParent*)pobject)->m_DataType.m_nType)	\
		{	\
		case T_CHAR:    ((CISOArray<char>*)pobject)->routine args ;			  break;	\
		case T_INT:     ((CISOArray<int>*)pobject)->routine args ;              break;	\
		case T_FLOAT:   ((CISOArray<double>*)pobject)->routine args ;           break;	\
		case T_BOOL:    ((CISOArray<bool>*)pobject)->routine args ;             break;	\
		case T_OBJECT:  ((CISOArray<CISystemObject*>*)pobject)->routine args ;  break;	\
		case T_UNIT:    ((CISOArray<CZUnit*>*)pobject)->routine args ;          break;	\
		case T_STRUCT:  ((CISOArray<CIStructure*>*)pobject)->routine args ;     break;	\
		default:  ASSERT(false);	\
		}	\
		break;	

#define _CallRoutineOnSet( pobject, routine, args)	\
    	switch (((CISOSetParent*)pobject)->m_DataType.m_nType)	\
		{	\
		case T_CHAR:    ((CISOSet<char>*)pobject)->routine args ;             break;	\
		case T_INT:     ((CISOSet<int>*)pobject)->routine args ;              break;	\
		case T_FLOAT:   ((CISOSet<double>*)pobject)->routine args ;           break;	\
		case T_BOOL:    ((CISOSet<bool>*)pobject)->routine args ;             break;	\
		case T_OBJECT:  ((CISOSet<CISystemObject*>*)pobject)->routine args ;  break;	\
		case T_UNIT:    ((CISOSet<CZUnit*>*)pobject)->routine args ;          break;	\
		case T_STRUCT:  ((CISOSet<CIStructure*>*)pobject)->routine args ;     break;	\
		default:  ASSERT(false);	\
		}	\
		break;

#define _SwitchAndCallRoutineOnOtherSystemObjects( pobject, routine, args)	\
    case SO_UNKNOWN:    \
    case SO_NONE:       \
        ASSERT(false); /* uklada se objekt, ktery ma spatne nastaveny typ ve sve polozce m_nSOType */   \
        break;  \
    case SO_STRING:	\
		((CISOString*)pobject)->routine args ;	break;	\
    case SO_POSITION:   \
        ((CISOPosition*)pobject)->routine args; break;  \
    case SO_MAP:    \
        ((CISOMap*)pobject)->routine args; break;    \
    case SO_PATH:    \
        ((CISOPath*)pobject)->routine args; break;    \
    case SO_CLIENT:   \
        ((CISOClient *)pobject)->routine args; break; /*can't be called - not exists in this phase*/ \
    case SO_RESOURCES: \
        ((CISOResources *)pobject)->routine args; break; \
	/*SYSOB*/	\
	\
    



void CInterpret::PersistentSaveSystemObject( CPersistentStorage &storage, CISystemObject *pSystemObject)
{
    BRACE_BLOCK(storage);

    ESystemObjectType eSOType = pSystemObject->GetObjectType();
    
	storage << (DWORD)eSOType;

	switch ( eSOType)
	{
    
    case SO_ARRAY:
		storage << (DWORD)((CISOArrayParent*)pSystemObject)->m_DataType.m_nType;

		_CallRoutineOnArray( pSystemObject, PersistentSave, (storage));
 
		break;

    case SO_SET:
		storage << (DWORD)((CISOSetParent*)pSystemObject)->m_DataType.m_nType;

		_CallRoutineOnSet( pSystemObject, PersistentSave, (storage));

		break;

	case SO_SKILLINTERFACE:
		((CISkillInterface*)pSystemObject)->PersistentSave( storage);
		break;


	_SwitchAndCallRoutineOnOtherSystemObjects( pSystemObject, PersistentSave, (storage));

	default:
		// systemovy objekt musi mit v konstruktoru vyplnenou polozku m_nSOType;
		ASSERT(false);
	}
}

//////////////////////////////////////////////////////////////////////////////////////

CISystemObject* CInterpret::PersistentLoadSystemObject( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    CISystemObject *pSystemObject = NULL;
	ESystemObjectType eSOType;
	EDataType eDataType;

	storage >> (DWORD&)eSOType;

	if (eSOType != SO_ARRAY && eSOType != SO_SET && eSOType != SO_SKILLINTERFACE)
	{ 
		pSystemObject = CreateSystemObject(eSOType);
	}

	switch ( eSOType)
	{
    case SO_ARRAY:
		storage >> (DWORD&)eDataType;

		switch (eDataType)
		{
		case T_CHAR:    pSystemObject = new CISOArray<char>;
						((CISOArray<char>*)pSystemObject)->PersistentLoad(storage);				break;
		case T_INT:     pSystemObject = new CISOArray<int>;
						((CISOArray<int>*)pSystemObject)->PersistentLoad(storage);              break;
		case T_FLOAT:   pSystemObject = new CISOArray<double>;
						((CISOArray<double>*)pSystemObject)->PersistentLoad(storage);           break;
		case T_BOOL:    pSystemObject = new CISOArray<bool>;
						((CISOArray<bool>*)pSystemObject)->PersistentLoad(storage);             break;
		case T_OBJECT:  pSystemObject = new CISOArray<CISystemObject*>;
						((CISOArray<CISystemObject*>*)pSystemObject)->PersistentLoad(storage);  break;
		case T_UNIT:    pSystemObject = new CISOArray<CZUnit*>;
						((CISOArray<CZUnit*>*)pSystemObject)->PersistentLoad(storage);          break;
		case T_STRUCT:  pSystemObject = new CISOArray<CIStructure*>;
						((CISOArray<CIStructure*>*)pSystemObject)->PersistentLoad(storage);     break;
		default:  ASSERT(false);
		}

		RegisterSystemObject( pSystemObject);

		break;

    case SO_SET:
		storage >> (DWORD&)eDataType;

		switch (eDataType)
		{
		case T_CHAR:    pSystemObject = new CISOSet<char>;
						((CISOSet<char>*)pSystemObject)->PersistentLoad(storage);             break;
		case T_INT:     pSystemObject = new CISOSet<int>;
						((CISOSet<int>*)pSystemObject)->PersistentLoad(storage);              break;
		case T_FLOAT:   pSystemObject = new CISOSet<double>;
						((CISOSet<double>*)pSystemObject)->PersistentLoad(storage);           break;
		case T_BOOL:    pSystemObject = new CISOSet<bool>;
						((CISOSet<bool>*)pSystemObject)->PersistentLoad(storage);             break;
		case T_OBJECT:  pSystemObject = new CISOSet<CISystemObject*>;
						((CISOSet<CISystemObject*>*)pSystemObject)->PersistentLoad(storage);  break;
		case T_UNIT:    pSystemObject = new CISOSet<CZUnit*>;
						((CISOSet<CZUnit*>*)pSystemObject)->PersistentLoad(storage);          break;
		case T_STRUCT:  pSystemObject = new CISOSet<CIStructure*>;
						((CISOSet<CIStructure*>*)pSystemObject)->PersistentLoad(storage);     break;
		default:  ASSERT(false);
		}

		RegisterSystemObject( pSystemObject);

		break;

	case SO_SKILLINTERFACE:
		pSystemObject = new CISkillInterface();
		((CISkillInterface*)pSystemObject)->PersistentLoad( storage);
		
		RegisterSystemObject( pSystemObject);

		break;

	_SwitchAndCallRoutineOnOtherSystemObjects( pSystemObject, PersistentLoad, (storage));

    default:
		ASSERT(false);
	}

	return pSystemObject;
}

//////////////////////////////////////////////////////////////////////////////////////
	
void CInterpret::PersistentTranslatePointersSystemObject( CPersistentStorage &storage, CISystemObject *pSystemObject)
{
	switch ( pSystemObject->GetObjectType())
	{
    case SO_ARRAY:
		_CallRoutineOnArray( pSystemObject, PersistentTranslatePointers, (storage));
		break;

    case SO_SET:
		_CallRoutineOnSet( pSystemObject, PersistentTranslatePointers, (storage));
		break;

	case SO_SKILLINTERFACE:
		((CISkillInterface*)pSystemObject)->PersistentTranslatePointers( storage);
		break;

	_SwitchAndCallRoutineOnOtherSystemObjects( pSystemObject, PersistentTranslatePointers, (storage));

	default:
		// systemovy objekt musi mit v konstruktoru vyplnenou polozku m_nSOType;
		ASSERT(false);
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void CInterpret::PersistentInitSystemObject(CISystemObject *pSystemObject)
{
	switch ( pSystemObject->GetObjectType())
	{
    case SO_ARRAY:
		_CallRoutineOnArray( pSystemObject, PersistentInit, ());
		break;

    case SO_SET:
		_CallRoutineOnSet( pSystemObject, PersistentInit, ());
		break;

	case SO_SKILLINTERFACE:
		((CISkillInterface*)pSystemObject)->PersistentInit( );
		break;

	_SwitchAndCallRoutineOnOtherSystemObjects( pSystemObject, PersistentInit, ());

	default:
		// systemovy objekt musi mit v konstruktoru vyplnenou polozku m_nSOType;
		ASSERT(false);
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void CInterpret::UnitAccessedAndConstructorNoFinished(CZUnit *pUnit, CIProcess *pProcess)
{
	// moje jednotka
	if ( pUnit->m_pCivilization == m_pCivilization)
	{

		ASSERT( pUnit->m_pConstructorFinished != NULL);

		// uspi se a nech se vzbudit, az bude konstruktor dokoncen
		pUnit->m_pConstructorFinished->AddWaitingProcess( pProcess);
		Sleep( pProcess, pUnit->m_pConstructorFinished);
	}
	// nepratelska jednotka
	else
	{
		ReadyToRun( m_pCurProcess);
		RunNextProcess( true);
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void CInterpret::KillAllProcessesOfUnit( CZUnit *pUnit)
{
    ASSERT( ::AfxGetThread()->m_nThreadID == m_dwInterpretThreadID);

    // zaradit current process na zacatek fronty
    if (m_nState != IDLE)
    {
        AddProcessToListAtBeginWithLastPointer( ReadyToRun, m_pCurProcess);
        m_pCurProcess = NULL;
    }

    int i;
    CIProcess *pProcess, *pPomProcess;

    for (i=0; i<2; i++)
    {
        // ReadyToRun fronta
        if (i==0) pProcess = m_pFirstReadyToRun;
        // Sleeping fronta
        else pProcess = m_pFirstSleeping;

Continue:
        while (pProcess != NULL)
        {
            if ( pProcess->m_nProcessType == CIProcess::PROCESS_GLOBALFUNCTION)
            {
                pProcess = pProcess->m_pNext;
                continue;
            }

            ASSERT( pProcess->m_nProcessType != CIProcess::PROCESS_NONE);

			if (pProcess->ContainsInstanceOfUnit(pUnit))
            {
                pPomProcess = pProcess;
                pProcess = pProcess->m_pNext;

                // zabit starou verzi
                KillProcess( pPomProcess, STATUS_UPDATE);

                // pokracujeme
                goto Continue;
            }

            pProcess = pProcess->m_pNext;
        }
    }

	// znova spustit bezici process
	RunNextProcess( false);
}

//////////////////////////////////////////////////////////////////////////////////////



