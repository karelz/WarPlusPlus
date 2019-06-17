// ISystemObject.h: interface for the CISystemObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISYSTEMOBJECT_H__0FB42DD1_C420_11D3_AFB7_004F49068BD6__INCLUDED_)
#define AFX_ISYSTEMOBJECT_H__0FB42DD1_C420_11D3_AFB7_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _HASH_ #define

#define DECLARE_SYSCALL_INTERFACE_MAP(ClassName, MethodCount) \
	public: \
    virtual ESyscallResult InvokeMethod( CIProcess *pCallingProcess, long nMethodID, CIBag *pBag, CISyscall **ppSyscall); \
	protected: \
    enum {_SCI_METHOD_COUNT = MethodCount }; \
    static ESyscallResult (ClassName::* _SCI_m_aMethods[_SCI_METHOD_COUNT])( CIProcess*, CIBag*, CISyscall**);

#define DECLARE_SYSCALL_INTERFACE_MAP_TEMPLATE(ClassName, MethodCount) \
	protected: \
    enum {_SCI_METHOD_COUNT = MethodCount }; \
    static ESyscallResult (ClassName::* _SCI_m_aMethods[_SCI_METHOD_COUNT])( CIProcess*, CIBag*, CISyscall**);

#define DECLARE_SYSCALL_METHOD0(FunctionName) \
	ESyscallResult _SCI_M_##FunctionName( CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall) \
	{ \
		CIDataStackItem *pRet = pBag->GetReturnRegister(); \
		ASSERT(pRet != NULL && pBag->GetArgCount() == 0); \
		return FunctionName(pProcess, ppSyscall, pRet); \
	}; \

#define DECLARE_SYSCALL_METHOD0V(FunctionName) \
	ESyscallResult _SCI_M_##FunctionName( CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall) \
	{ \
		ASSERT(pBag->GetArgCount() == 0); \
		return FunctionName(pProcess, ppSyscall); \
	}; \

#define DECLARE_SYSCALL_METHOD1(FunctionName, Param1Type, Param1CType) \
	ESyscallResult _SCI_M_##FunctionName( CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall) \
	{ \
		CIDataStackItem *pRet = pBag->GetReturnRegister(); \
		void *pParam1 = pBag->Get##Param1Type(0); \
		ASSERT(pRet != NULL && pParam1 != NULL && pBag->GetArgCount() == 1);\
		return FunctionName(pProcess, ppSyscall, pRet, (Param1CType) pParam1); \
	}; \

#define DECLARE_SYSCALL_METHOD1V(FunctionName, Param1Type, Param1CType) \
	ESyscallResult _SCI_M_##FunctionName( CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall) \
	{ \
		void *pParam1 = pBag->Get##Param1Type(0); \
		ASSERT( pParam1 != NULL && pBag->GetArgCount() == 1); \
		return FunctionName(pProcess, ppSyscall, (Param1CType) pParam1); \
	}; \

#define DECLARE_SYSCALL_METHOD2(FunctionName, Param1Type, Param1CType, Param2Type, Param2CType) \
	ESyscallResult _SCI_M_##FunctionName( CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall) \
	{ \
		CIDataStackItem *pRet = pBag->GetReturnRegister(); \
		void *pParam1 = pBag->Get##Param1Type(0); \
		void *pParam2 = pBag->Get##Param2Type(1); \
		ASSERT( pRet != NULL && pParam1 != NULL && pParam2 != NULL && pBag->GetArgCount() == 2); \
		return FunctionName(pProcess, ppSyscall, pRet, (Param1CType) pParam1, (Param2CType) pParam2); \
	}; \

#define DECLARE_SYSCALL_METHOD2V(FunctionName, Param1Type, Param1CType, Param2Type, Param2CType) \
	ESyscallResult _SCI_M_##FunctionName( CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall) \
	{ \
		void *pParam1 = pBag->Get##Param1Type(0); \
		void *pParam2 = pBag->Get##Param2Type(1); \
		ASSERT( pParam1 != NULL && pParam2 != NULL && pBag->GetArgCount() == 2); \
		return FunctionName(pProcess, ppSyscall, (Param1CType) pParam1, (Param2CType) pParam2); \
	}; \

#define DECLARE_SYSCALL_METHOD3(FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType) \
	ESyscallResult _SCI_M_##FunctionName( CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall) \
	{ \
		CIDataStackItem *pRet = pBag->GetReturnRegister(); \
		void *pParam1 = pBag->Get##Param1Type(0); \
		void *pParam2 = pBag->Get##Param2Type(1); \
        void *pParam3 = pBag->Get##Param2Type(2); \
		ASSERT( pRet != NULL && pParam1 != NULL && pParam2 != NULL && pParam3 != NULL && pBag->GetArgCount() == 3); \
		return FunctionName(pProcess, ppSyscall, pRet, (Param1CType) pParam1, (Param2CType) pParam2, (Param3CType) pParam3); \
	}; \

#define DECLARE_SYSCALL_METHOD3V(FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType) \
	ESyscallResult _SCI_M_##FunctionName( CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall) \
	{ \
		void *pParam1 = pBag->Get##Param1Type(0); \
		void *pParam2 = pBag->Get##Param2Type(1); \
        void *pParam3 = pBag->Get##Param2Type(2); \
		ASSERT( pParam1 != NULL && pParam2 != NULL && pParam3 != NULL && pBag->GetArgCount() == 3); \
		return FunctionName(pProcess, ppSyscall, pRet, (Param1CType) pParam1, (Param2CType) pParam2, (Param3CType) pParam3); \
	}; \


#define BEGIN_SYSCALL_INTERFACE_MAP(ClassName) \
	ESyscallResult ClassName::InvokeMethod( CIProcess *pCallingProcess, long nMethodID, CIBag *pBag, CISyscall **ppSyscall) \
	{ \
		if (nMethodID < 0 || nMethodID >= _SCI_METHOD_COUNT || \
			_SCI_m_aMethods[nMethodID] == NULL) \
		{ \
			TRACE_INTERPRET3("%3d: object %s doesn't have any method with id %d!\n", pCallingProcess->m_nProcessID, GetStringFromSOType(m_nSOType), nMethodID); \
			ASSERT(false); \
			return SYSCALL_ERROR; \
		} \
		else return (this->*_SCI_m_aMethods[nMethodID])(pCallingProcess, pBag, ppSyscall); \
	} \
	ESyscallResult (ClassName::* ClassName::_SCI_m_aMethods[ ClassName::_SCI_METHOD_COUNT ])( CIProcess*, CIBag*, CISyscall**) = { \

#define SYSCALL_METHOD(FunctionName) \
	    _SCI_M_##FunctionName

#define END_SYSCALL_INTERFACE_MAP() \
	};



class CIProcess;
class CISyscall;

class CISystemObject : public CIObjUnitStruct
{
public:
	CISystemObject() : CIObjUnitStruct() { m_bCanBeDuplicated = false; }
	virtual ~CISystemObject() {}

public:
    // vytvoreni & zruseni
    virtual bool Create( CInterpret *pInterpret)	{ m_pInterpret = pInterpret; return true; }
    virtual void Delete()   { }

    virtual CIObjUnitStruct* AddRef();
    virtual CIObjUnitStruct* Release();

    // load & store polozek objektu
    virtual bool LoadMember( CIDataStackItem &Reg, long nMemberID) = 0;
    virtual bool StoreMember( CIDataStackItem *pReg, long nMemberID) = 0;

    // volani metod
    virtual ESyscallResult InvokeMethod( CIProcess *pCallingProcess, long nMethodID, CIBag *pBag,
									     CISyscall **ppSyscall) = 0;


protected:
	CInterpret *m_pInterpret; // nutne pro odregistrovani objektu pri ruseni (kdyz m_nRefCount klesne na 0);
    ESystemObjectType m_nSOType; // typ objektu
	bool m_bCanBeDuplicated;	 // zda je mozno vytvaret kopie

public:
	inline ESystemObjectType GetObjectType()	{ return m_nSOType; }
	inline CanBeDuplicated()					{ return m_bCanBeDuplicated; }

public:
// 
// SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();
};

#endif // !defined(AFX_ISYSTEMOBJECT_H__0FB42DD1_C420_11D3_AFB7_004F49068BD6__INCLUDED_)
