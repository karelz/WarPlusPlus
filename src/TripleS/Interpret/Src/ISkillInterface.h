// ISkillInterface.h: interface for the CISkillInterface class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISKILLINTERFACE_H__D1BC12C5_2371_11D4_B050_004F49068BD6__INCLUDED_)
#define AFX_ISKILLINTERFACE_H__D1BC12C5_2371_11D4_B050_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TripleS\Interpret\Src\SysObjs\ISystemObject.h"

class CSUnit;
class CInterpret;
class CSSkillType;
class CIBag;
class CISyscall;

class CISkillInterface : public CISystemObject  
{
    DECLARE_MULTITHREAD_POOL( CISkillInterface);

public:
    CISkillInterface();
	virtual ~CISkillInterface();
	
    virtual bool Create( CInterpret *pInterpret, CZUnit *pZUnit, CSSkillType *pSkillType);
    virtual void Delete();

    virtual ESyscallResult InvokeMethod( CIProcess *pCallingProcess, long nMethodID, CIBag *pBag,
									     CISyscall **ppSyscall);

    virtual bool LoadMember( CIDataStackItem &Reg, long nMemberID) { ASSERT(false); return false; }
    virtual bool StoreMember( CIDataStackItem *pReg, long nMemberID) { ASSERT(false); return false; }


protected:
    CZUnit *m_pZUnit;
    CSSkillType *m_pSkillType;

public:
// 
// SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();
};

#endif // !defined(AFX_ISKILLINTERFACE_H__D1BC12C5_2371_11D4_B050_004F49068BD6__INCLUDED_)
