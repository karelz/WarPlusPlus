// IGlobalFunctions.cpp: implementation of the CISystemGlobalFunctions class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\..\Interpret.h"
#include "SystemObjects.h"

#include "..\Syscalls\ISyscallSome.h"

#include "GameServer\GameServer\SMap.h"
#include "GameServer\Civilization\ZCivilization.h"
#include "TripleS\CodeManager\CodeManager.h"

#include <math.h>

//////////////////////
// Metody
//////////////////////

ESyscallResult (CISystemGlobalFunctions::* CISystemGlobalFunctions::m_aFunctions[ 
                CISystemGlobalFunctions::FUNCTION_COUNT ])( CIProcess*, CIBag*, CISyscall**) = {
    CISystemGlobalFunctions::F_MessageBox,      //0
    CISystemGlobalFunctions::F_IntToString,     //1
    CISystemGlobalFunctions::F_FloatToString,   //2
    CISystemGlobalFunctions::F_BoolToString,    //3
    CISystemGlobalFunctions::F_WriteToLog,      //4 
    CISystemGlobalFunctions::F_DoStWithUnit,    //5
    CISystemGlobalFunctions::F_DoStWaiting,     //6
    CISystemGlobalFunctions::F_GetCivilizationCount,  //7
    CISystemGlobalFunctions::F_GetMap,          // 8
    CISystemGlobalFunctions::F_RandomFloat,     // 9
    CISystemGlobalFunctions::F_RandomInt,       // 10
    CISystemGlobalFunctions::F_GetGeneralCommander, // 11
	CISystemGlobalFunctions::F_Yield,			// 12
	CISystemGlobalFunctions::F_GetTimeSlice,    // 13
	CISystemGlobalFunctions::F_Sleep,           // 14
	CISystemGlobalFunctions::F_sin,				// 15
	CISystemGlobalFunctions::F_cos,				// 16
	CISystemGlobalFunctions::F_sqrt,			// 17

};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CISystemGlobalFunctions::CISystemGlobalFunctions()
{
}

//////////////////////////////////////////////////////////////////////

CISystemGlobalFunctions::~CISystemGlobalFunctions()
{
}

//////////////////////////////////////////////////////////////////////

ESyscallResult CISystemGlobalFunctions::InvokeFunction( CIProcess *pCallingProcess, long nFunctionID, 
													    CIBag *pBag, CISyscall **ppSyscall)
{
    if (nFunctionID < 0 || nFunctionID >= FUNCTION_COUNT || 
        m_aFunctions[nFunctionID] == NULL) 
    {
        TRACE_INTERPRET2("%3d: there is no global function with id %d!\n", pCallingProcess->m_nProcessID, 
            nFunctionID);
        ASSERT(false);
        return SYSCALL_ERROR;
    }
    else return (this->*m_aFunctions[nFunctionID])(pCallingProcess, pBag, ppSyscall);
}

//////////////////////////////////////////////////////////////////////
// 0: void MessageBox(String);
ESyscallResult CISystemGlobalFunctions::F_MessageBox( CIProcess*, CIBag *pBag, CISyscall **ppSyscall)
{
    // parametry
    CISOString *pString = pBag->GetString(0);

    ASSERT (pBag->GetArgCount() == 1 && pString != NULL);

    ::AfxMessageBox(pString->m_strString);

    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 1: String IntToString(int i);
ESyscallResult CISystemGlobalFunctions::F_IntToString( CIProcess* pProcess, CIBag *pBag, CISyscall **ppSyscall)
{
    // parametry
    int *pInt = pBag->GetInt(0);
    CIDataStackItem *pRet = pBag->GetReturnRegister();

    ASSERT (pBag->GetArgCount() == 1 && pRet != NULL && pInt != NULL);

    // vytvoreni navratoveho stringu
    CISOString *pString;
    
    if ((pString = (CISOString*)pProcess->m_pInterpret->CreateSystemObject(SO_STRING)) == NULL) return SYSCALL_ERROR;

    // naplneni navratove hodnoty
    pString->m_strString.Format("%d", *pInt);

    pRet->Set( pString);

    pString->Release();
    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 2: String FloatToString(int i);
ESyscallResult CISystemGlobalFunctions::F_FloatToString( CIProcess* pProcess, CIBag *pBag, CISyscall **ppSyscall)
{
    // parametry
    double *pFloat = pBag->GetFloat(0);
    int *pPrecision = pBag->GetInt(1);
    CIDataStackItem *pRet = pBag->GetReturnRegister();

    ASSERT (pBag->GetArgCount() == 2 && pRet != NULL &&
            pFloat != NULL && pPrecision != NULL);
       
    if (*pPrecision < -2) return SYSCALL_ARGUMENT_ERROR;

    // vytvoreni navratoveho stringu
    CISOString *pString;
    
    if ((pString = (CISOString*)pProcess->m_pInterpret->CreateSystemObject(SO_STRING)) == NULL) return SYSCALL_ERROR;

    // naplneni navratove hodnoty
    switch (*pPrecision)
    {
    case -1:
        // 1E10
        pString->m_strString.Format("%E", *pFloat);
        break;
    case -2:
        // 1e10
        pString->m_strString.Format("%e", *pFloat);
        break;
    default:
        CString pom;
        pom.Format("%%.%df", *pPrecision);
        pString->m_strString.Format(pom, *pFloat);
        break;
    }

    pRet->Set( pString);

    pString->Release();
    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 3: String BoolToString(int i);
ESyscallResult CISystemGlobalFunctions::F_BoolToString( CIProcess* pProcess, CIBag *pBag, CISyscall **ppSyscall)
{
    // parametry
    int *pBool = pBag->GetBool(0);
    CIDataStackItem *pRet = pBag->GetReturnRegister();

    ASSERT (pBag->GetArgCount() == 1 && pRet != NULL && pBool != NULL );

    // vytvoreni navratoveho stringu
    CISOString *pString;
    
    if ((pString = (CISOString*)pProcess->m_pInterpret->CreateSystemObject(SO_STRING)) == NULL) return SYSCALL_ERROR;

    pString->m_strString = (*pBool != 0) ? "true" : "false";

    pRet->Set( pString);

    pString->Release();
    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 4: void WriteToLog(String);
ESyscallResult CISystemGlobalFunctions::F_WriteToLog( CIProcess* pProcess, CIBag *pBag, CISyscall **ppSyscall)
{
    // parametry
    CISOString *pString = pBag->GetString(0);

    ASSERT (pBag->GetArgCount() == 1);

	if ( pString == NULL) return SYSCALL_ERROR;

    pProcess->m_pInterpret->ReportError(pString->m_strString);

    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 5: void DoStWithUnit( Unit u);
ESyscallResult CISystemGlobalFunctions::F_DoStWithUnit( CIProcess*, CIBag *pBag, CISyscall **ppSyscall)
{
    // parametry
    CZUnit *pUnit = pBag->GetUnit(0);

    ASSERT( pBag->GetArgCount() == 1);

	if (pUnit == NULL) return SYSCALL_ERROR;

    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 6: int DoStWaiting();
ESyscallResult CISystemGlobalFunctions::F_DoStWaiting( CIProcess* pProcess, CIBag *pBag, CISyscall **ppSyscall)
{
    CISyscallSome *pSyscallSome = new CISyscallSome();

    if (!pSyscallSome->Create( pProcess, pBag)) return SYSCALL_ERROR;
    else 
	{
		*ppSyscall = pSyscallSome;
		return SYSCALL_SUCCESS_SLEEP;
	}
}

//////////////////////////////////////////////////////////////////////
// 7: int GetCivilizationCount();
ESyscallResult CISystemGlobalFunctions::F_GetCivilizationCount( CIProcess* pProcess, CIBag *pBag, CISyscall **ppSyscall)
{
    pBag->ReturnInt( g_cMap.GetCivilizationCount());
    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 8: int GetMap();
ESyscallResult CISystemGlobalFunctions::F_GetMap( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall)
{
    CISOMap *pMap = (CISOMap *)pProcess->m_pInterpret->CreateSystemObject(SO_MAP);
    if (pMap == NULL) return SYSCALL_ERROR;
    pBag->ReturnSystemObject( pMap );
    pMap->Release();

    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 9: float RandomFloat();
ESyscallResult CISystemGlobalFunctions::F_RandomFloat( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall)
{
    ASSERT (pBag->GetArgCount() == 0);

    pBag->ReturnFloat( ((double)rand())/(double)RAND_MAX );

    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 10: int RandomInt(int range);
ESyscallResult CISystemGlobalFunctions::F_RandomInt( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall)
{
    // parametry
    int *pRange=pBag->GetInt(0);
    
    ASSERT (pBag->GetArgCount() == 1 && pRange != NULL);
    
    int nRange = *pRange;
    if(nRange<0) nRange=0;
    if(nRange>RAND_MAX) nRange=RAND_MAX;


    if(nRange<=1)
        pBag->ReturnInt( 0 );
    else
        pBag->ReturnInt( rand() % nRange );

    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 11: Unit GetGeneralCommander();
ESyscallResult CISystemGlobalFunctions::F_GetGeneralCommander( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall)
{    
    ASSERT(pBag!=NULL); 
    ASSERT(pBag->GetArgCount() == 0);
    
    ASSERT(pProcess!=NULL);
    ASSERT(pProcess->m_pInterpret!=NULL);
    ASSERT(pProcess->m_pInterpret->m_pCivilization!=NULL);

    CZCivilization *pCivilization=pProcess->m_pInterpret->m_pCivilization;
    CZUnit *pUnit=pCivilization->GetGeneralCommander();
    
    ASSERT(pUnit!=NULL);

    // zamknout vojenskou hierarchii
    pCivilization->LockHierarchyAndEvents();

    pBag->ReturnUnit(pUnit, CCodeManager::m_pDefaultUnitType);

    // odemnknout vojenskou hierarchii
    pCivilization->UnlockHierarchyAndEvents();

    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 12: void Yield();
ESyscallResult CISystemGlobalFunctions::F_Yield( CIProcess *pProcess, CIBag*, CISyscall**)
{
	ASSERT( pProcess != NULL);
	pProcess->m_pInterpret->RunNextProcess( true);
	return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 13: int GetTimeSlice();
ESyscallResult CISystemGlobalFunctions::F_GetTimeSlice ( CIProcess *, CIBag *pBag, CISyscall **)
{
  ASSERT ( pBag->GetArgCount() == 0 );

  pBag->ReturnInt ( (int)g_cMap.GetTimeSlice () );

  return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 14: int GetTimeSlice();
ESyscallResult CISystemGlobalFunctions::F_Sleep ( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall )
{
	ASSERT ( pProcess != NULL );
	ASSERT ( pBag->GetArgCount () == 1 );

	// Get the timeslice count from the bag
	int *pTimeslices = pBag->GetInt ( 0 );
	ASSERT ( pTimeslices != NULL );

	// If the number is below zero -> error
	if ( *pTimeslices < 0 )
	{
		return SYSCALL_ARGUMENT_ERROR;
	}

	// If it was Sleep ( 0 ) -> just Yield
	if ( *pTimeslices == 0 )
	{
		pProcess->m_pInterpret->RunNextProcess ( true );
		return SYSCALL_SUCCESS;
	}

	// OK, we have to create a syscall
	CISyscall * pNewSyscall = new CISyscall ();
	if ( !pNewSyscall->Create ( pProcess ) )
	{
		return SYSCALL_ERROR;
	}
	*pSyscall = pNewSyscall;

	// Set the timer
  pNewSyscall->AddRef ();
	CZCivilization::SetTimer ( pNewSyscall, g_cMap.GetTimeSlice () + *pTimeslices );

	return SYSCALL_SUCCESS_SLEEP;
}

//////////////////////////////////////////////////////////////////////
// 15: float sin(float angle);
ESyscallResult CISystemGlobalFunctions::F_sin( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall)
{
    // parametry
    double *pAngle=pBag->GetFloat(0);
    
    ASSERT (pBag->GetArgCount() == 1 && pAngle != NULL);
    
    double fAngle = *pAngle;

	pBag->ReturnFloat(sin(fAngle));

    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 16: float cos(float angle);
ESyscallResult CISystemGlobalFunctions::F_cos( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall)
{
    // parametry
    double *pAngle=pBag->GetFloat(0);
    
    ASSERT (pBag->GetArgCount() == 1 && pAngle != NULL);
    
    double fAngle = *pAngle;

	pBag->ReturnFloat(cos(fAngle));

    return SYSCALL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// 17: float sqrt(float f);
ESyscallResult CISystemGlobalFunctions::F_sqrt( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall)
{
    // parametry
    double *pF=pBag->GetFloat(0);
    
    ASSERT (pBag->GetArgCount() == 1 && pF != NULL);
    
    double f= *pF;

	if(f>=0) {
		pBag->ReturnFloat(sqrt(f));
		return SYSCALL_SUCCESS;
	} else {
		pProcess->m_pInterpret->ReportError("Cannot calculate square root of negative number.");
		return SYSCALL_ERROR;
	}    
}


//////////////////////////////////////////////////////////////////////
