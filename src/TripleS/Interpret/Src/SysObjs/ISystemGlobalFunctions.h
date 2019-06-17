// IGlobalFunctions.h: interface for the CISystemGlobalFunctions class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IGLOBALFUNCTIONS_H__A7DAF2C6_D9AB_11D3_AFD5_004F49068BD6__INCLUDED_)
#define AFX_IGLOBALFUNCTIONS_H__A7DAF2C6_D9AB_11D3_AFD5_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIProcess;
class CISyscall;

class CISystemGlobalFunctions  
{
public:
    CISystemGlobalFunctions();
    virtual ~CISystemGlobalFunctions();

    // volani funkce
    virtual ESyscallResult InvokeFunction( CIProcess *pCallingProcess, long nFunctionID, CIBag *pBag,
										   CISyscall **ppSyscall);

protected:
    enum {FUNCTION_COUNT = 18 };

    static ESyscallResult (CISystemGlobalFunctions::* m_aFunctions[FUNCTION_COUNT])( CIProcess*, CIBag*, CISyscall**);

// 0: void MessageBox(String);
    ESyscallResult F_MessageBox( CIProcess*, CIBag *pBag, CISyscall **ppSyscall);
// 1: String IntToString(int i);
    ESyscallResult F_IntToString( CIProcess*, CIBag *pBag, CISyscall **ppSyscall);
// 2: String FloatToString(int i);
    ESyscallResult F_FloatToString( CIProcess*, CIBag *pBag, CISyscall **ppSyscall);
// 3: String BoolToString(bool b);
    ESyscallResult F_BoolToString( CIProcess*, CIBag *pBag, CISyscall **ppSyscall);
// 4: void WriteToLog(String);
    ESyscallResult F_WriteToLog( CIProcess*, CIBag *pBag, CISyscall **ppSyscall);
// 5: void DoStWithUnit( Unit u);
    ESyscallResult F_DoStWithUnit( CIProcess*, CIBag *pBag, CISyscall **ppSyscall);
// 6: int DoStWaiting();
    ESyscallResult F_DoStWaiting( CIProcess* pProcess, CIBag *pBag, CISyscall **ppSyscall);
// 7: int GetCivilizationCount();
    ESyscallResult F_GetCivilizationCount( CIProcess* pProcess, CIBag *pBag, CISyscall **ppSyscall);
// 8: Map GetMap();
    ESyscallResult F_GetMap( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall);
// 9: float RandomFloat();
    ESyscallResult F_RandomFloat( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall);
// 10: int RandomInt(int range);
    ESyscallResult F_RandomInt( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall);
// 11: Unit GetGeneralCommander();
    ESyscallResult F_GetGeneralCommander( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall);
// 12: void Yield();
	ESyscallResult F_Yield( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall);
// 13: int GetTimeSlice();
	ESyscallResult F_GetTimeSlice( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall);
// 14: void Sleep ( int nTimeslices );
	ESyscallResult F_Sleep ( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall );
// 15: float sin ( float angle );
	ESyscallResult F_sin ( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall );
// 16: float cos ( float angle );
	ESyscallResult F_cos ( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall );
// 17: float sqrt ( float f );
	ESyscallResult F_sqrt( CIProcess *pProcess, CIBag *pBag, CISyscall **pSyscall );

/*
functions
{
0: void MessageBox(String);
1: String IntToString(int i);
2: String FloatToString(float i, int precision);  // -1 .. 1E10, -2 .. 1e10
3: String BoolToString(bool b);
4: void WriteToLog(String);
5: void DoStWithUnit( Unit u);
6: int GetCivilizationCount();
7: int GetCivilizationCount();
8: Map GetMap();
9: float RandomFloat();
10: int RandomInt(int range);
11: Unit GetGeneralCommander();
12: void Yield();
13: int GetTimeSlice();
14: void Sleep ( int nTimeslices );
15: float sin ( float angle );
16: float cos ( float angle );
17: float sqrt ( float f );

}
*/

};

#endif // !defined(AFX_IGLOBALFUNCTIONS_H__A7DAF2C6_D9AB_11D3_AFD5_004F49068BD6__INCLUDED_)
