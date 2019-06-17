// ISOMap.h: interface for the CISOMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISOMAP_H__CDDE99D4_524C_11D4_B09E_004F49068BD6__INCLUDED_)
#define AFX_ISOMAP_H__CDDE99D4_524C_11D4_B09E_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ISystemObject.h"

class CISOMap : public CISystemObject  
{
public:
	CISOMap();
	virtual ~CISOMap();

public:
    // vytvoreni & zruseni
    virtual bool Create( CInterpret *pInterpret) { return CISystemObject::Create( pInterpret); }
    virtual void Delete();

    // load & store polozek objektu
    // objekt Map nema zadne polozky
    virtual bool LoadMember( CIDataStackItem&, long) { return false; }
    virtual bool StoreMember( CIDataStackItem*, long) { return false; }

protected:
    DECLARE_SYSCALL_INTERFACE_MAP(CISOMap, 3);

    DECLARE_SYSCALL_METHOD3(M_GetMyUnitsInArea, Int, int*, Int, int*, Int, int*);
    DECLARE_SYSCALL_METHOD3(M_GetEnemyUnitsInArea, Int, int*, Int, int*, Int, int*);
    DECLARE_SYSCALL_METHOD3(M_GetAllUnitsInArea, Int, int*, Int, int*, Int, int*);

// 0: 
    ESyscallResult M_GetMyUnitsInArea( CIProcess*, CISyscall**, CIDataStackItem *pRet, 
                                       int *pX, int *pY, int *pRadius); 
// 1: 
    ESyscallResult M_GetEnemyUnitsInArea( CIProcess*, CISyscall**, CIDataStackItem *pRet, 
                                       int *pX, int *pY, int *pRadius); 
// 2: 
    ESyscallResult M_GetAllUnitsInArea( CIProcess*, CISyscall**, CIDataStackItem *pRet, 
                                       int *pX, int *pY, int *pRadius); 


/*
object Map
{
0: Unit{} GetMyUnitsInArea( int x, int y, int radius);
1: Unit{} GetEnemyUnitsInArea( int x, int y, int radius);
2: Unit{} GetAllUnitsInArea( int x, int y, int radius);
}
*/

//
//  SAVE & LOAD - nic se neuklada, takze se zavolaji predci
//

protected:
};

#endif // !defined(AFX_ISOMAP_H__CDDE99D4_524C_11D4_B09E_004F49068BD6__INCLUDED_)
