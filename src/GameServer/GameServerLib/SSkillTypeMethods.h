#ifndef __SERVER_SKILL_TYPE_METHODS__HEADER_INCLUDED__
#define __SERVER_SKILL_TYPE_METHODS__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Makra pro volání ze skriptù
//////////////////////////////////////////////////////////////////////

#define DECLARE_SKILLCALL_INTERFACE_MAP(ClassName, MethodCount) \
	public: \
		virtual ESyscallResult InvokeMethod ( CSUnit *pUnit, CIProcess *pCallingProcess, long nMethodID, CIBag *pBag, CISyscall **ppSyscall, bool bCalledByEnemy ); \
	protected: \
		enum { _SCI_METHOD_COUNT = MethodCount }; \
		static ESyscallResult (ClassName::* _SCI_m_aMethods[_SCI_METHOD_COUNT]) ( CSUnit *p, CIProcess*, CIBag*, CISyscall**, bool );

#define DECLARE_SKILLCALL_INTERFACE_MAP_TEMPLATE(ClassName, MethodCount) \
	protected: \
		enum { _SCI_METHOD_COUNT = MethodCount }; \
		static ESyscallResult (ClassName::* _SCI_m_aMethods[_SCI_METHOD_COUNT]) ( CSUnit *p, CIProcess*, CIBag*, CISyscall**, bool );


//////////////////////////////////////////////////////////////////////
// pomocná makra zámkování

#define SKILLCALL_HELPER_LOCK_READ_SOFT() \
	VERIFY(pUnit->ReaderLock());

#define SKILLCALL_HELPER_LOCK_READ_HARD() \
	VERIFY(pUnit->ReaderLock()); \
	if(!IsEnabled(pUnit)){ pUnit->ReaderUnlock(); return SYSCALL_ERROR; }

#define SKILLCALL_HELPER_UNLOCK_READ() \
	pUnit->ReaderUnlock(); \
	return eResult;


#define SKILLCALL_HELPER_LOCK_WRITE_SOFT() \
	VERIFY(pUnit->WriterLock());

#define SKILLCALL_HELPER_LOCK_WRITE_HARD() \
	VERIFY(pUnit->WriterLock()); \
	if(!IsEnabled(pUnit)){ pUnit->WriterUnlock(); return SYSCALL_ERROR; }

#define SKILLCALL_HELPER_UNLOCK_WRITE() \
	pUnit->WriterUnlock(); \
	return eResult;


//////////////////////////////////////////////////////////////////////
// pomocná makra pro zákazy volání
#define SKILLCALL_HELPER_DISABLE_ENEMY_CALL() \
  if ( bCalledByEnemy ) return SYSCALL_ERROR;

#define SKILLCALL_HELPER_ENABLE_ENEMY_CALL() ;

#define SKILLCALL_HELPER_DEC_FLAGS() , pBag
#define SKILLCALL_HELPER_EEC_FLAGS() , pBag, bCalledByEnemy

//////////////////////////////////////////////////////////////////////
// pomocná makra pro deklaraci metod
// úplne obecné helpers

#define DECLARE_SKILLCALL_METHOD0_HELPER( FunctionName, LockCode, UnlockCode, EnableCode, Flags ) \
	ESyscallResult _SCI_M_##FunctionName( CSUnit *pUnit, CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall, bool bCalledByEnemy ) \
	{ \
    EnableCode \
		CIDataStackItem *pRet = pBag->GetReturnRegister(); \
		ASSERT(pRet != NULL && pBag->GetArgCount() == 0); \
		ESyscallResult eResult; \
		LockCode \
		eResult = FunctionName(pUnit, pProcess, ppSyscall Flags , pRet ); \
		UnlockCode \
	};

#define DECLARE_SKILLCALL_METHOD0V_HELPER( FunctionName, LockCode, UnlockCode, EnableCode, Flags ) \
	ESyscallResult _SCI_M_##FunctionName( CSUnit *pUnit, CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall, bool bCalledByEnemy ) \
	{ \
    EnableCode \
		ASSERT(pBag->GetArgCount() == 0); \
		ESyscallResult eResult; \
    LockCode \
		eResult = FunctionName(pUnit, pProcess, ppSyscall Flags); \
    UnlockCode \
  };

#define DECLARE_SKILLCALL_METHOD1_HELPER( FunctionName, LockCode, UnlockCode, EnableCode, Flags, Param1Type, Param1CType ) \
	ESyscallResult _SCI_M_##FunctionName( CSUnit *pUnit, CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall, bool bCalledByEnemy) \
	{ \
    EnableCode \
		CIDataStackItem *pRet = pBag->GetReturnRegister(); \
		void *pParam1 = pBag->Get##Param1Type(0); \
		ASSERT(pRet != NULL && pParam1 != NULL && pBag->GetArgCount() == 1); \
		ESyscallResult eResult; \
		LockCode \
		eResult = FunctionName(pUnit, pProcess, ppSyscall Flags, pRet, (Param1CType) pParam1); \
		UnlockCode \
	};

#define DECLARE_SKILLCALL_METHOD1V_HELPER( FunctionName, LockCode, UnlockCode, EnableCode, Flags, Param1Type, Param1CType ) \
	ESyscallResult _SCI_M_##FunctionName( CSUnit *pUnit, CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall, bool bCalledByEnemy) \
	{ \
    EnableCode \
		void *pParam1 = pBag->Get##Param1Type(0); \
		ASSERT( pParam1 != NULL && pBag->GetArgCount() == 1); \
		ESyscallResult eResult; \
		LockCode \
		eResult = FunctionName(pUnit, pProcess, ppSyscall Flags, (Param1CType) pParam1); \
		UnlockCode \
	};


#define DECLARE_SKILLCALL_METHOD2_HELPER( FunctionName, LockCode, UnlockCode, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType ) \
	ESyscallResult _SCI_M_##FunctionName( CSUnit *pUnit, CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall, bool bCalledByEnemy) \
	{ \
    EnableCode \
		CIDataStackItem *pRet = pBag->GetReturnRegister(); \
		void *pParam1 = pBag->Get##Param1Type(0); \
		void *pParam2 = pBag->Get##Param2Type(1); \
		ASSERT( pRet != NULL && pParam1 != NULL && pParam2 != NULL && pBag->GetArgCount() == 2); \
		ESyscallResult eResult; \
		LockCode \
		eResult = FunctionName(pUnit, pProcess, ppSyscall Flags, pRet, (Param1CType) pParam1, (Param2CType) pParam2); \
		UnlockCode \
	};

#define DECLARE_SKILLCALL_METHOD2V_HELPER( FunctionName, LockCode, UnlockCode, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType) \
	ESyscallResult _SCI_M_##FunctionName( CSUnit *pUnit, CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall, bool bCalledByEnemy) \
	{ \
    EnableCode \
		void *pParam1 = pBag->Get##Param1Type(0); \
		void *pParam2 = pBag->Get##Param2Type(1); \
		ASSERT( pParam1 != NULL && pParam2 != NULL && pBag->GetArgCount() == 2); \
		ESyscallResult eResult; \
		LockCode \
		eResult = FunctionName(pUnit, pProcess, ppSyscall Flags, (Param1CType) pParam1, (Param2CType) pParam2); \
		UnlockCode \
	};

#define DECLARE_SKILLCALL_METHOD3_HELPER( FunctionName, LockCode, UnlockCode, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType) \
	ESyscallResult _SCI_M_##FunctionName( CSUnit *pUnit, CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall, bool bCalledByEnemy) \
	{ \
    EnableCode \
		CIDataStackItem *pRet = pBag->GetReturnRegister(); \
		void *pParam1 = pBag->Get##Param1Type(0); \
		void *pParam2 = pBag->Get##Param2Type(1); \
		void *pParam3 = pBag->Get##Param3Type(2); \
		ASSERT( pRet != NULL && pParam1 != NULL && pParam2 != NULL && pParam3 != NULL && pBag->GetArgCount() == 3); \
		ESyscallResult eResult; \
		LockCode \
		eResult = FunctionName(pUnit, pProcess, ppSyscall Flags, pRet, (Param1CType) pParam1, (Param2CType) pParam2, (Param3CType) pParam3); \
		UnlockCode \
	};

#define DECLARE_SKILLCALL_METHOD3V_HELPER( FunctionName, LockCode, UnlockCode, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType) \
	ESyscallResult _SCI_M_##FunctionName( CSUnit *pUnit, CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall, bool bCalledByEnemy) \
	{ \
    EnableCode \
		void *pParam1 = pBag->Get##Param1Type(0); \
		void *pParam2 = pBag->Get##Param2Type(1); \
		void *pParam3 = pBag->Get##Param3Type(2); \
		ASSERT( pParam1 != NULL && pParam2 != NULL && pParam3 != NULL && pBag->GetArgCount() == 3); \
		ESyscallResult eResult; \
		LockCode \
		eResult = FunctionName(pUnit, pProcess, ppSyscall Flags, (Param1CType) pParam1, (Param2CType) pParam2, (Param3CType) pParam3); \
		UnlockCode \
	};

#define DECLARE_SKILLCALL_METHOD4_HELPER( FunctionName, LockCode, UnlockCode, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType) \
	ESyscallResult _SCI_M_##FunctionName( CSUnit *pUnit, CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall, bool bCalledByEnemy) \
	{ \
    EnableCode \
		CIDataStackItem *pRet = pBag->GetReturnRegister(); \
		void *pParam1 = pBag->Get##Param1Type(0); \
		void *pParam2 = pBag->Get##Param2Type(1); \
		void *pParam3 = pBag->Get##Param3Type(2); \
		void *pParam4 = pBag->Get##Param4Type(3); \
		ASSERT( pRet != NULL && pParam1 != NULL && pParam2 != NULL && pParam3 != NULL && pParam4 != NULL && pBag->GetArgCount() == 4); \
		ESyscallResult eResult; \
		LockCode \
		eResult = FunctionName(pUnit, pProcess, ppSyscall Flags, pRet, (Param1CType) pParam1, (Param2CType) pParam2, (Param3CType) pParam3, (Param4CType) pParam4); \
		UnlockCode \
	};

#define DECLARE_SKILLCALL_METHOD4V_HELPER( FunctionName, LockCode, UnlockCode, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType) \
	ESyscallResult _SCI_M_##FunctionName( CSUnit *pUnit, CIProcess *pProcess, CIBag *pBag, CISyscall **ppSyscall, bool bCalledByEnemy) \
	{ \
    EnableCode \
		void *pParam1 = pBag->Get##Param1Type(0); \
		void *pParam2 = pBag->Get##Param2Type(1); \
		void *pParam3 = pBag->Get##Param3Type(2); \
		void *pParam4 = pBag->Get##Param4Type(3); \
		ASSERT( pParam1 != NULL && pParam2 != NULL && pParam3 != NULL && pParam4 != NULL && pBag->GetArgCount() == 4); \
		ESyscallResult eResult; \
		LockCode \
		eResult = FunctionName(pUnit, pProcess, ppSyscall Flags, (Param1CType) pParam1, (Param2CType) pParam2, (Param3CType) pParam3, (Param4CType) pParam4); \
		UnlockCode \
	};

//////////////////////////////////////////////////////////////////////
// pomocná makra pro deklaraci metod
// zámkované helpers

#define DECLARE_SKILLCALL_METHOD0_READ_SOFT_HELPER( FunctionName, EnableCode, Flags ) \
  DECLARE_SKILLCALL_METHOD0_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_SOFT(), SKILLCALL_HELPER_UNLOCK_READ(), EnableCode, Flags )
#define DECLARE_SKILLCALL_METHOD0_WRITE_SOFT_HELPER( FunctionName, EnableCode, Flags ) \
  DECLARE_SKILLCALL_METHOD0_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_SOFT (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags )
#define DECLARE_SKILLCALL_METHOD0_READ_HARD_HELPER( FunctionName, EnableCode, Flags ) \
  DECLARE_SKILLCALL_METHOD0_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_HARD (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags )
#define DECLARE_SKILLCALL_METHOD0_WRITE_HARD_HELPER( FunctionName, EnableCode, Flags ) \
  DECLARE_SKILLCALL_METHOD0_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_HARD (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags )

#define DECLARE_SKILLCALL_METHOD0V_READ_SOFT_HELPER( FunctionName, EnableCode, Flags ) \
  DECLARE_SKILLCALL_METHOD0V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_SOFT (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags )
#define DECLARE_SKILLCALL_METHOD0V_WRITE_SOFT_HELPER( FunctionName, EnableCode, Flags ) \
  DECLARE_SKILLCALL_METHOD0V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_SOFT (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags )
#define DECLARE_SKILLCALL_METHOD0V_READ_HARD_HELPER( FunctionName, EnableCode, Flags ) \
  DECLARE_SKILLCALL_METHOD0V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_HARD (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags )
#define DECLARE_SKILLCALL_METHOD0V_WRITE_HARD_HELPER( FunctionName, EnableCode, Flags ) \
  DECLARE_SKILLCALL_METHOD0V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_HARD (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags )

#define DECLARE_SKILLCALL_METHOD1_READ_SOFT_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_SOFT (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags, Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1_WRITE_SOFT_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_SOFT (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags, Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1_READ_HARD_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_HARD (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags, Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1_WRITE_HARD_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_HARD (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags, Param1Type, Param1CType )

#define DECLARE_SKILLCALL_METHOD1V_READ_SOFT_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType  ) \
  DECLARE_SKILLCALL_METHOD1V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_SOFT (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags, Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1V_WRITE_SOFT_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_SOFT (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags, Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1V_READ_HARD_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_HARD (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags, Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1V_WRITE_HARD_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_HARD (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags, Param1Type, Param1CType )

#define DECLARE_SKILLCALL_METHOD2_READ_SOFT_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_SOFT (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2_WRITE_SOFT_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_SOFT (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2_READ_HARD_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_HARD (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2_WRITE_HARD_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_HARD (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType )

#define DECLARE_SKILLCALL_METHOD2V_READ_SOFT_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_SOFT (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2V_WRITE_SOFT_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_SOFT (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2V_READ_HARD_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_HARD (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2V_WRITE_HARD_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_HARD (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType )

#define DECLARE_SKILLCALL_METHOD3_READ_SOFT_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_SOFT (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3_WRITE_SOFT_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_SOFT (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3_READ_HARD_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_HARD (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3_WRITE_HARD_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_HARD (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )

#define DECLARE_SKILLCALL_METHOD3V_READ_SOFT_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_SOFT (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3V_WRITE_SOFT_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_SOFT (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3V_READ_HARD_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_HARD (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3V_WRITE_HARD_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_HARD (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )

#define DECLARE_SKILLCALL_METHOD4_READ_SOFT_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_SOFT (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4_WRITE_SOFT_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_SOFT (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4_READ_HARD_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_HARD (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4_WRITE_HARD_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_HARD (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )

#define DECLARE_SKILLCALL_METHOD4V_READ_SOFT_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_SOFT (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4V_WRITE_SOFT_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_SOFT (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4V_READ_HARD_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_READ_HARD (), SKILLCALL_HELPER_UNLOCK_READ (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4V_WRITE_HARD_HELPER( FunctionName, EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4V_HELPER( FunctionName, SKILLCALL_HELPER_LOCK_WRITE_HARD (), SKILLCALL_HELPER_UNLOCK_WRITE (), EnableCode, Flags, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )


//////////////////////////////////////////////////////////////////////
// makra pro deklaraci metod
// METHODX - metoda s X parametry, ktera vraci nejakou hodnotu
// METHODXV - metoda s X parametry, ktera vraci void
// READ_SOFT - soft metoda, zamknout jednotku na read
// READ_HARD - hard metoda, zamknout jednotku na read
// WRITE_SOFT - soft metoda, zamknout jednotku na write
// WRITE_HARD - hard metoda, zamknout jednotku na write
// sufix EEC - metodu muze volat i nepritel

#define DECLARE_SKILLCALL_METHOD0_READ_SOFT( FunctionName ) \
  DECLARE_SKILLCALL_METHOD0_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS() )
#define DECLARE_SKILLCALL_METHOD0_READ_SOFT_EEC( FunctionName ) \
  DECLARE_SKILLCALL_METHOD0_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS() )
#define DECLARE_SKILLCALL_METHOD0_WRITE_SOFT( FunctionName ) \
  DECLARE_SKILLCALL_METHOD0_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS() )
#define DECLARE_SKILLCALL_METHOD0_WRITE_SOFT_EEC( FunctionName ) \
  DECLARE_SKILLCALL_METHOD0_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS() )
#define DECLARE_SKILLCALL_METHOD0_READ_HARD( FunctionName ) \
  DECLARE_SKILLCALL_METHOD0_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS() )
#define DECLARE_SKILLCALL_METHOD0_READ_HARD_EEC( FunctionName ) \
  DECLARE_SKILLCALL_METHOD0_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS() )
#define DECLARE_SKILLCALL_METHOD0_WRITE_HARD( FunctionName ) \
  DECLARE_SKILLCALL_METHOD0_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS() )
#define DECLARE_SKILLCALL_METHOD0_WRITE_HARD_EEC( FunctionName ) \
  DECLARE_SKILLCALL_METHOD0_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS() )

#define DECLARE_SKILLCALL_METHOD0V_READ_SOFT( FunctionName ) \
  DECLARE_SKILLCALL_METHOD0V_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS() )
#define DECLARE_SKILLCALL_METHOD0V_READ_SOFT_EEC( FunctionName ) \
  DECLARE_SKILLCALL_METHOD0V_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS() )
#define DECLARE_SKILLCALL_METHOD0V_WRITE_SOFT( FunctionName ) \
  DECLARE_SKILLCALL_METHOD0V_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS() )
#define DECLARE_SKILLCALL_METHOD0V_WRITE_SOFT_EEC( FunctionName ) \
  DECLARE_SKILLCALL_METHOD0V_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS() )
#define DECLARE_SKILLCALL_METHOD0V_READ_HARD( FunctionName ) \
  DECLARE_SKILLCALL_METHOD0V_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS() )
#define DECLARE_SKILLCALL_METHOD0V_READ_HARD_EEC( FunctionName ) \
  DECLARE_SKILLCALL_METHOD0V_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS() )
#define DECLARE_SKILLCALL_METHOD0V_WRITE_HARD( FunctionName ) \
  DECLARE_SKILLCALL_METHOD0V_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS() )
#define DECLARE_SKILLCALL_METHOD0V_WRITE_HARD_EEC( FunctionName ) \
  DECLARE_SKILLCALL_METHOD0V_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS() )

#define DECLARE_SKILLCALL_METHOD1_READ_SOFT( FunctionName, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1_READ_SOFT_EEC( FunctionName, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1_WRITE_SOFT( FunctionName, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1_WRITE_SOFT_EEC( FunctionName, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1_READ_HARD( FunctionName, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1_READ_HARD_EEC( FunctionName, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1_WRITE_HARD( FunctionName, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1_WRITE_HARD_EEC( FunctionName, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType )

#define DECLARE_SKILLCALL_METHOD1V_READ_SOFT( FunctionName, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1V_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1V_READ_SOFT_EEC( FunctionName, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1V_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1V_WRITE_SOFT( FunctionName, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1V_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1V_WRITE_SOFT_EEC( FunctionName, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1V_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1V_READ_HARD( FunctionName, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1V_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1V_READ_HARD_EEC( FunctionName, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1V_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1V_WRITE_HARD( FunctionName, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1V_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType )
#define DECLARE_SKILLCALL_METHOD1V_WRITE_HARD_EEC( FunctionName, Param1Type, Param1CType ) \
  DECLARE_SKILLCALL_METHOD1V_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType )

#define DECLARE_SKILLCALL_METHOD2_READ_SOFT( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2_READ_SOFT_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2_WRITE_SOFT( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2_WRITE_SOFT_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2_READ_HARD( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2_READ_HARD_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2_WRITE_HARD( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2_WRITE_HARD_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType )

#define DECLARE_SKILLCALL_METHOD2V_READ_SOFT( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2V_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2V_READ_SOFT_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2V_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2V_WRITE_SOFT( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2V_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2V_WRITE_SOFT_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2V_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2V_READ_HARD( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2V_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2V_READ_HARD_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2V_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2V_WRITE_HARD( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2V_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType )
#define DECLARE_SKILLCALL_METHOD2V_WRITE_HARD_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType ) \
  DECLARE_SKILLCALL_METHOD2V_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType )

#define DECLARE_SKILLCALL_METHOD3_READ_SOFT( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3_READ_SOFT_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3_WRITE_SOFT( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3_WRITE_SOFT_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3_READ_HARD( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3_READ_HARD_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3_WRITE_HARD( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3_WRITE_HARD_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )

#define DECLARE_SKILLCALL_METHOD3V_READ_SOFT( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3V_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3V_READ_SOFT_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3V_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3V_WRITE_SOFT( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3V_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3V_WRITE_SOFT_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3V_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3V_READ_HARD( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3V_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3V_READ_HARD_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3V_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3V_WRITE_HARD( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3V_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )
#define DECLARE_SKILLCALL_METHOD3V_WRITE_HARD_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType ) \
  DECLARE_SKILLCALL_METHOD3V_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType )

#define DECLARE_SKILLCALL_METHOD4_READ_SOFT( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4_READ_SOFT_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4_WRITE_SOFT( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4_WRITE_SOFT_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4_READ_HARD( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4_READ_HARD_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4_WRITE_HARD( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4_WRITE_HARD_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )

#define DECLARE_SKILLCALL_METHOD4V_READ_SOFT( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4V_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4V_READ_SOFT_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4V_READ_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4V_WRITE_SOFT( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4V_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4V_WRITE_SOFT_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4V_WRITE_SOFT_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4V_READ_HARD( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4V_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4V_READ_HARD_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4V_READ_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4V_WRITE_HARD( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4V_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_DISABLE_ENEMY_CALL(), SKILLCALL_HELPER_DEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )
#define DECLARE_SKILLCALL_METHOD4V_WRITE_HARD_EEC( FunctionName, Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType ) \
  DECLARE_SKILLCALL_METHOD4V_WRITE_HARD_HELPER( FunctionName, SKILLCALL_HELPER_ENABLE_ENEMY_CALL(), SKILLCALL_HELPER_EEC_FLAGS(), Param1Type, Param1CType, Param2Type, Param2CType, Param3Type, Param3CType, Param4Type, Param4CType )


//////////////////////////////////////////////////////////////////////
// pomocná makra

#define SKILLCALL_EMPTY			(NULL)
#define SKILLCALL_BASECLASS	(_SCI_M_SkillCallBaseClass)


#define BEGIN_SKILLCALL_INTERFACE_MAP(ClassName, BaseClass) \
	ESyscallResult ClassName::InvokeMethod( CSUnit *pUnit, CIProcess *pCallingProcess, long nMethodID, CIBag *pBag, CISyscall **ppSyscall, bool bCalledByEnemy) \
	{ \
		if (nMethodID < 0 || nMethodID >= _SCI_METHOD_COUNT || \
			_SCI_m_aMethods[nMethodID] == SKILLCALL_EMPTY) \
		{ \
			TRACE2("%3d: object " #ClassName " doesn't have any method with id %d!\n", pCallingProcess->m_nProcessID, nMethodID); \
			ASSERT(false); \
			return SYSCALL_ERROR; \
		} \
		else if (_SCI_m_aMethods[nMethodID] == SKILLCALL_BASECLASS) \
		{ \
			return (this->BaseClass::InvokeMethod(pUnit, pCallingProcess, nMethodID, pBag, ppSyscall, bCalledByEnemy)); \
		} \
		else return (this->*_SCI_m_aMethods[nMethodID])(pUnit, pCallingProcess, pBag, ppSyscall, bCalledByEnemy); \
	} \
ESyscallResult (ClassName::* ClassName::_SCI_m_aMethods[ ClassName::_SCI_METHOD_COUNT ])( CSUnit *, CIProcess*, CIBag*, CISyscall**, bool bCalledByEnemy) = { 
	
#define SKILLCALL_METHOD(FunctionName) _SCI_M_##FunctionName

#define END_SKILLCALL_INTERFACE_MAP() };

#endif // __SERVER_SKILL_TYPE_METHODS__HEADER_INCLUDED__
