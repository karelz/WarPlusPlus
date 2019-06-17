// ISOPath.h: interface for the CISOPath class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISOPATH_H__E4A96293_778E_11D4_80CD_0000B4A08F9A__INCLUDED_)
#define AFX_ISOPATH_H__E4A96293_778E_11D4_80CD_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TripleS\Interpret\Src\SysObjs\IDuplicSystemObject.h"

class CSPath;

class CISOPath : public CIDuplicSystemObject  
{
public:
	CISOPath();
	virtual ~CISOPath();

    // vytvoreni & zruseni
    virtual bool Create( CInterpret *pInterpret);
    virtual void Delete();
    
    // nastaveni
    void Set(CSPath *pPath);

    // load & store polozek objektu
    virtual bool LoadMember( CIDataStackItem& DSI, long nID);
    virtual bool StoreMember( CIDataStackItem *pDSI, long nID);

	// duplikace
    virtual bool CreateFrom( CInterpret *pInterpret, CIDuplicSystemObject *pSrc);

// METODY
protected:
    DECLARE_SYSCALL_INTERFACE_MAP(CISOPath, 11);

    DECLARE_SYSCALL_METHOD0( M_GetStartPoint);
    DECLARE_SYSCALL_METHOD0( M_GetEndPoint);
    
    DECLARE_SYSCALL_METHOD0V( M_SwapDirection);

    DECLARE_SYSCALL_METHOD0( M_GetDistance);
    DECLARE_SYSCALL_METHOD0( M_GetBeeLineDistance);

    DECLARE_SYSCALL_METHOD0( M_GetMinWidth);
    DECLARE_SYSCALL_METHOD1( M_GetMinDistance, SystemObject, CISystemObject*);
    DECLARE_SYSCALL_METHOD1( M_GetMaxDistance, SystemObject, CISystemObject*);

    DECLARE_SYSCALL_METHOD1V( M_Truncate, Int, int*);
    DECLARE_SYSCALL_METHOD1V( M_TruncateBeeLine, Int, int*);

    DECLARE_SYSCALL_METHOD1V( M_AppendPath, SystemObject, CISystemObject*);

// 0: 
    ESyscallResult M_GetStartPoint( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet);
// 1:
    ESyscallResult M_GetEndPoint( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet);
// 2:
    ESyscallResult M_SwapDirection( CIProcess*, CISyscall **pSyscall);
// 3:
    ESyscallResult M_GetDistance( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet);
// 4:
    ESyscallResult M_GetBeeLineDistance( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet);
// 5:
    ESyscallResult M_GetMinWidth( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet);
// 6:
    ESyscallResult M_GetMinDistance( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet, CISystemObject *pPos);
// 7:
    ESyscallResult M_GetMaxDistance( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet, CISystemObject *pPos);
// 8:
    ESyscallResult M_Truncate( CIProcess*, CISyscall **pSyscall, int *pDistance);
// 9:
    ESyscallResult M_TruncateBeeLine( CIProcess*, CISyscall **pSyscall, int *pDistance);
// 10:
    ESyscallResult M_AppendPath( CIProcess*, CISyscall **pSyscall, CISystemObject *pPos);

/*
object Path
{
0: Position GetStartPoint();
1: Position GetEndPoint();
2: void SwapDirection();			// Swaps direction of the path
3: int GetDistance();			// Total distance travelled from startpoint to endpoint
4: int GetBeeLineDistance();		// BeeLine distance from startpoint to endpoint
5: int GetMinWidth();			// Minimal width of the path (bottleneck)
6: int GetMinDistance(Position pos); // Minimal bee line distance from the path to specified position
7: int GetMaxDistance(Position pos); // Maximal bee line distance from the path to specified position
8: void Truncate(int distance);	// Shorten the path by given distance from its endpoint
9: void TruncateBeeLine(int distance); // Shorten the path so it ends at given bee line distance from position
10: void AppendPath(Position pos);	// Append path from the endpoint to pos
}
*/

// polozky
public:

// 
// SAVE & LOAD
//
public:
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();

// Vlastni data
private:
    // Cesta    
    CSPath *m_pPath;
};

#endif // !defined(AFX_ISOPATH_H__E4A96293_778E_11D4_80CD_0000B4A08F9A__INCLUDED_)
