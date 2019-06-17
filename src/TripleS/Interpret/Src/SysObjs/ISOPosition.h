// ISOPosition.h: interface for the CISOPosition class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISOPOSITION_H__D1BC12C4_2371_11D4_B050_004F49068BD6__INCLUDED_)
#define AFX_ISOPOSITION_H__D1BC12C4_2371_11D4_B050_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TripleS\Interpret\Src\SysObjs\IDuplicSystemObject.h"
#include "ISystemObject.h"
#include "GameServer\GameServer\GeneralClasses\GeneralClasses.h"
#include "GameServer\GameServer\SPosition.h"

class CISOPosition : public CIDuplicSystemObject  
{
public:
	CISOPosition();
	virtual ~CISOPosition();

public:
    // vytvoreni & zruseni
    virtual bool Create( CInterpret *pInterpret);
    virtual void Delete();
    
    // nastaveni
    void Set(CPointDW *pPoint, int Z);
    void Set(CSPosition *pPosition);
    void Set(int X, int Y, int Z);

    int GetXPosition(){ return m_nXYZ[0]; }
    int GetYPosition(){ return m_nXYZ[1]; }
    int GetZPosition(){ return m_nXYZ[2]; }

    CPointDW Get(){ return CPointDW(m_nXYZ[0], m_nXYZ[1]); }

    // load & store polozek objektu
    virtual bool LoadMember( CIDataStackItem& DSI, long nID);
    virtual bool StoreMember( CIDataStackItem* pDSI, long nID);

	// duplikace
    virtual bool CreateFrom( CInterpret *pInterpret, CIDuplicSystemObject *pSrc);

// METODY
protected:
    DECLARE_SYSCALL_INTERFACE_MAP(CISOPosition, 3);

    DECLARE_SYSCALL_METHOD1( M_GetDistancePos, SystemObject, CISystemObject*);
    DECLARE_SYSCALL_METHOD2( M_GetDistance, Int, int*, Int, int*);
    DECLARE_SYSCALL_METHOD0( M_IsOnMap);

// 0: 
    ESyscallResult M_GetDistancePos( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet,
                                    CISystemObject *pSystemObject);
// 1: 
    ESyscallResult M_GetDistance( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet,
                                int *pX, int *pY);
// 2:
    ESyscallResult M_IsOnMap(CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet);


/*
object Position
{
0: int x;
1: int y;
2: int z;
0: int GetDistancePos( Position pos);
1: int GetDistance( int x, int y);
2: bool IsOnMap();
}
*/

// polozky
public:
    int m_nXYZ[3];


public:
// 
// SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();

};

#endif // !defined(AFX_ISOPOSITION_H__D1BC12C4_2371_11D4_B050_004F49068BD6__INCLUDED_)
