// ISOMap.cpp: implementation of the CISOMap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\..\Interpret.h"
#include "ISOMap.h"
#include "ISOSet.h"
#include "GameServer\GameServer\SMap.h"
#include "GameServer\Skills\SystemObjects\Map\SMapGetUnitsInAreaDPC.h"
#include "GameServer\Skills\SysCalls\IDPCSysCall.h"
#include "GameServer\Civilization\ZCivilization.h"

//////////////////////////////////////////////////////////////////////

BEGIN_SYSCALL_INTERFACE_MAP(CISOMap)
	SYSCALL_METHOD(M_GetMyUnitsInArea),
    SYSCALL_METHOD(M_GetEnemyUnitsInArea),
    SYSCALL_METHOD(M_GetAllUnitsInArea)
END_SYSCALL_INTERFACE_MAP()

//////////////////////////////////////////////////////////////////////

CISOMap::CISOMap()
{
    m_nSOType = SO_MAP;
}

//////////////////////////////////////////////////////////////////////

CISOMap::~CISOMap()
{
}

//////////////////////////////////////////////////////////////////////

void CISOMap::Delete()
{
}

//////////////////////////////////////////////////////////////////////

ESyscallResult CISOMap::M_GetMyUnitsInArea( CIProcess *pProcess, CISyscall **ppSysCall, CIDataStackItem *pRet, 
                                       int *pX, int *pY, int *pRadius)
{
    if(*pRadius > MAX_GET_UNITS_IN_AREA_RADIUS) {
        pProcess->m_pInterpret->ReportError("GetMyUnitsInArea: Area too large.");
        *pRadius=MAX_GET_UNITS_IN_AREA_RADIUS;
    }
    else if(*pRadius < 0) {
        pProcess->m_pInterpret->ReportError("GetMyUnitsInArea: Area radius negative.");
        *pRadius=0;
    }

    CISOSet<CZUnit*> *pSet;
    CType Type;

    Type.SetUnit( g_StringTable.AddItem(DEFAULT_UNIT_PARENT, false));

    pSet = (CISOSet<CZUnit*> *)pProcess->m_pInterpret->CreateSet(&Type, TRUE);

    if (pSet == NULL) {
        return SYSCALL_ERROR;
    }

    pRet->Set(pSet);
    pSet->Release(); 

    CSMapGetUnitsInAreaDPC *pDPC=new CSMapGetUnitsInAreaDPC(pProcess, pRet, 
        CPointDW(*pX, *pY), *pRadius, CSMapGetUnitsInAreaDPC::MyUnitsInArea);
    CIDPCSysCall *pSysCall=pDPC->GetDPCSysCall();
    g_cMap.RegisterDPC(pDPC, 
		 pProcess->m_pInterpret->m_pCivilization->GetCivilizationIndex());
    *ppSysCall=pSysCall;

    return SYSCALL_SUCCESS_SLEEP;
}

//////////////////////////////////////////////////////////////////////

ESyscallResult CISOMap::M_GetEnemyUnitsInArea( CIProcess *pProcess, CISyscall **ppSysCall, CIDataStackItem *pRet, 
                                       int *pX, int *pY, int *pRadius)
{
    if(*pRadius > MAX_GET_UNITS_IN_AREA_RADIUS) {
        pProcess->m_pInterpret->ReportError("GetEnemyUnitsInArea: Area too large.");
        *pRadius=MAX_GET_UNITS_IN_AREA_RADIUS;
    }
    else if(*pRadius < 0) {
        pProcess->m_pInterpret->ReportError("GetEnemyUnitsInArea: Area radius negative.");
        *pRadius=0;
    }

    CISOSet<CZUnit*> *pSet;
    CType Type;

    Type.SetUnit( g_StringTable.AddItem(DEFAULT_UNIT_PARENT, false));

    pSet = (CISOSet<CZUnit*> *)pProcess->m_pInterpret->CreateSet(&Type, TRUE);

    if (pSet == NULL) {
        return SYSCALL_ERROR;
    }

    pRet->Set(pSet);
    pSet->Release(); 

    CSMapGetUnitsInAreaDPC *pDPC=new CSMapGetUnitsInAreaDPC(pProcess, pRet, 
        CPointDW(*pX, *pY), *pRadius, CSMapGetUnitsInAreaDPC::EnemyUnitsInArea);
    CIDPCSysCall *pSysCall=pDPC->GetDPCSysCall();
    g_cMap.RegisterDPC(pDPC, 
		 pProcess->m_pInterpret->m_pCivilization->GetCivilizationIndex());
    *ppSysCall=pSysCall;

    return SYSCALL_SUCCESS_SLEEP;
}

//////////////////////////////////////////////////////////////////////

ESyscallResult CISOMap::M_GetAllUnitsInArea( CIProcess *pProcess, CISyscall **ppSysCall, CIDataStackItem *pRet, 
                                   int *pX, int *pY, int *pRadius)
{
    if(*pRadius > MAX_GET_UNITS_IN_AREA_RADIUS) {
        pProcess->m_pInterpret->ReportError("GetAllUnitsInArea: Area too large.");
        *pRadius=MAX_GET_UNITS_IN_AREA_RADIUS;
    }
    else if(*pRadius < 0) {
        pProcess->m_pInterpret->ReportError("GetAllUnitsInArea: Area radius negative.");
        *pRadius=0;
    }

    CISOSet<CZUnit*> *pSet;
    CType Type;

    Type.SetUnit( g_StringTable.AddItem(DEFAULT_UNIT_PARENT, false));

    pSet = (CISOSet<CZUnit*> *)pProcess->m_pInterpret->CreateSet(&Type, TRUE);

    if (pSet == NULL) {
        return SYSCALL_ERROR;
    }

    pRet->Set(pSet);
    pSet->Release(); 

    CSMapGetUnitsInAreaDPC *pDPC=new CSMapGetUnitsInAreaDPC(pProcess, pRet, 
        CPointDW(*pX, *pY), *pRadius, CSMapGetUnitsInAreaDPC::AllUnitsInArea);
    CIDPCSysCall *pSysCall=pDPC->GetDPCSysCall();
    g_cMap.RegisterDPC(pDPC, 
		 pProcess->m_pInterpret->m_pCivilization->GetCivilizationIndex());
    *ppSysCall=pSysCall;

    return SYSCALL_SUCCESS_SLEEP;

}

//////////////////////////////////////////////////////////////////////