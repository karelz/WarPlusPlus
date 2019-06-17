// ISOResources.h: interface for the CISOResources class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISORESOURCES_H__0BEFEB53_7D76_11D4_B0F4_004F49068BD6__INCLUDED_)
#define AFX_ISORESOURCES_H__0BEFEB53_7D76_11D4_B0F4_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TripleS/Interpret/Src/SysObjs/IDuplicSystemObject.h"
#include "GameServer/GameServer/SResources.h"

class CISOResources : public CIDuplicSystemObject  
{
public:
	CISOResources();
	virtual ~CISOResources();

public:
    // vytvoreni & zruseni
    virtual bool Create( CInterpret *pInterpret);
    virtual void Delete();
    
    // nastaveni
    void Set(TSResources &Resources);

    // load & store polozek objektu
    virtual bool LoadMember( CIDataStackItem& DSI, long nID);
    virtual bool StoreMember( CIDataStackItem *pDSI, long nID);

	// duplikace
    virtual bool CreateFrom( CInterpret *pInterpret, CIDuplicSystemObject *pSrc);

// METODY
protected:
    DECLARE_SYSCALL_INTERFACE_MAP(CISOResources, 10);

    DECLARE_SYSCALL_METHOD0V( M_Empty);
    DECLARE_SYSCALL_METHOD1( M_LessThan, SystemObject, CISystemObject*);
    DECLARE_SYSCALL_METHOD1( M_LessEqual, SystemObject, CISystemObject*);
    DECLARE_SYSCALL_METHOD1( M_MoreThan, SystemObject, CISystemObject*);
    DECLARE_SYSCALL_METHOD1( M_MoreEqual, SystemObject, CISystemObject*);
    DECLARE_SYSCALL_METHOD1( M_Equal, SystemObject, CISystemObject*);
    DECLARE_SYSCALL_METHOD1( M_GetResource, Int, int*);
    DECLARE_SYSCALL_METHOD2V( M_SetResource, Int, int*, Int, int*);
    DECLARE_SYSCALL_METHOD0( M_GetNumResources);
    DECLARE_SYSCALL_METHOD0( M_IsEmpty);

// 0: 
    ESyscallResult M_Empty(CIProcess*, CISyscall **pSyscall);
    
// 1: 
    ESyscallResult M_LessThan( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet,
                                    CISystemObject *pSystemObject);
// 2: 
    ESyscallResult M_LessEqual( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet,
                                    CISystemObject *pSystemObject);
// 3: 
    ESyscallResult M_MoreThan( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet,
                                    CISystemObject *pSystemObject);
// 4: 
    ESyscallResult M_MoreEqual( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet,
                                    CISystemObject *pSystemObject);
// 5: 
    ESyscallResult M_Equal( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet,
                                    CISystemObject *pSystemObject);
// 6: 
    ESyscallResult M_GetResource( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet,
                                    int *pID);
// 7: 
    ESyscallResult M_SetResource( CIProcess*, CISyscall **pSyscall,
                                    int *pID, int *pValue);
// 8: 
    ESyscallResult M_GetNumResources( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet);

// 9: 
    ESyscallResult M_IsEmpty( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet);


/*
object Resources
{
0: void Empty();  // Sets all resources in the vector to zero
1: bool LessThan(Resources res);
2: bool LessEqual(Resources res);
3: bool MoreThan(Resources res);
4: bool MoreEqual(Resources res);
5: bool Equal(Resources res);
6: int GetResource(int id);
7: void SetResource(int id, int value);
8: int GetNumResources();
9: bool IsEmpty();
}*/

// polozky
public:
  TSResources m_Resources;

public:
// 
// SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();


};

#endif // !defined(AFX_ISORESOURCES_H__0BEFEB53_7D76_11D4_B0F4_004F49068BD6__INCLUDED_)
