#ifndef _ISOCLIENT_H_
#define _ISOCLIENT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ISystemObject.h"

class CZConnectedClient;
class CZCivilization;

class CISOClient : public CISystemObject
{
public:
  // constructor
  CISOClient();
  // destructor
  ~CISOClient();

// 
// SAVE & LOAD
//
public:
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();

public:
  // create and delete
  virtual bool Create( CInterpret *pInterpret);
  virtual void Delete();

  // nastaveni
  void Set(CZConnectedClient *pConnectedClient, CZCivilization *pCivilization);

  // load & store of the data members
  virtual bool LoadMember( CIDataStackItem& DSI, long nID);
  virtual bool StoreMember( CIDataStackItem* pDSI, long nID);

protected:
  DECLARE_SYSCALL_INTERFACE_MAP( CISOClient, 16 );

  // 0: String GetUserLogin()
  DECLARE_SYSCALL_METHOD0( M_GetUserLogin );
  // 1: bool IsLoggedOn();
  DECLARE_SYSCALL_METHOD0( M_IsLoggedOn );

  // 2: void MessageInfo(String strText);
  DECLARE_SYSCALL_METHOD1V( M_MessageInfo, SystemObject, CISystemObject * );
  // 3: void MessageInfoLocation(String strText, Position pos);
  DECLARE_SYSCALL_METHOD2V( M_MessageInfoLocation, SystemObject, CISystemObject *, SystemObject, CISystemObject * );
  // 4: void MessageInfoUnit(String strText, Unit unit);
  DECLARE_SYSCALL_METHOD2V( M_MessageInfoUnit, SystemObject, CISystemObject *, Unit, CZUnit * );

  // 5: void MessageWarning(String strText);
  DECLARE_SYSCALL_METHOD1V( M_MessageWarning, SystemObject, CISystemObject * );
  // 6: void MessageWarningLocation(String strText, Position pos);
  DECLARE_SYSCALL_METHOD2V( M_MessageWarningLocation, SystemObject, CISystemObject *, SystemObject, CISystemObject * );
  // 7: void MessageWarningUnit(String strText, Unit unit);
  DECLARE_SYSCALL_METHOD2V( M_MessageWarningUnit, SystemObject, CISystemObject *, Unit, CZUnit * );

  // 8: void MessageError(String strText);
  DECLARE_SYSCALL_METHOD1V( M_MessageError, SystemObject, CISystemObject * );
  // 9: void MessageErrorLocation(String strText, Position pos);
  DECLARE_SYSCALL_METHOD2V( M_MessageErrorLocation, SystemObject, CISystemObject *, SystemObject, CISystemObject * );
  // 10: void MessageErrorUnit(String strText, Unit unit);
  DECLARE_SYSCALL_METHOD2V( M_MessageErrorUnit, SystemObject, CISystemObject *, Unit, CZUnit * );

  // 11: void CloseUserInput()
  DECLARE_SYSCALL_METHOD0V( M_CloseUserInput );

  // 12: bool AskForPosition(String strQuestion, Position posResult);
  DECLARE_SYSCALL_METHOD2( M_AskForPosition, SystemObject, CISystemObject *, SystemObject, CISystemObject * );
  // 13: Unit AskForEnemyUnit(String strQuestion);
  DECLARE_SYSCALL_METHOD1( M_AskForEnemyUnit, SystemObject, CISystemObject * );
  // 14: Unit AskForMyUnit(String strQuestion);
  DECLARE_SYSCALL_METHOD1( M_AskForMyUnit, SystemObject, CISystemObject * );
  // 15: Unit AskForAnyUnit(String strQuestion);
  DECLARE_SYSCALL_METHOD1( M_AskForAnyUnit, SystemObject, CISystemObject * );


/*
object Client
{
0: String GetUserLogin();
1: bool IsLoggedOn();

2: void MessageInfo(String strText);
3: void MessageInfoLocation(String strText, Position pos);
4: void MessageInfoUnit(String strText, Unit unit);

5: void MessageWarning(String strText);
6: void MessageWarningLocation(String strText, Position pos);
7: void MessageWarningUnit(String strText, Unit unit);

8:  void MessageError(String strText);
9:  void MessageErrorLocation(String strText, Position pos);
10: void MessageErrorUnit(String strText, Unit unit);

11: void CloseUserInput();

12: bool AskForPosition(String strQuestion, Position posResult);
13: Unit AskForEnemyUnit(String strQuestion);
14: Unit AskForMyUnit(String strQuestion);
15: Unit AskForAnyUnit(String strQuestion);
}
*/

  // Returns user login name
  ESyscallResult M_GetUserLogin( CIProcess *pProcess, CISyscall **ppSyscall, CIDataStackItem *pRet );
  // Returns if the user is logged on (also returns FALSE if the object is not valid)
  ESyscallResult M_IsLoggedOn( CIProcess *pProcess, CISyscall **ppSyscall, CIDataStackItem *pRet );

  // Displayes info message on the client
  ESyscallResult M_MessageInfo( CIProcess *pProcess, CISyscall **ppSyscall, CISystemObject *pText );
  // Displayes info message on the client with location
  ESyscallResult M_MessageInfoLocation( CIProcess *pProcess, CISyscall **ppSyscall, CISystemObject *pText, CISystemObject *pLocation );
  // Displayes info message on the client with unit
  ESyscallResult M_MessageInfoUnit( CIProcess *pProcess, CISyscall **ppSyscall, CISystemObject *pText, CZUnit *pUnit );

  // Displayes warning message on the client
  ESyscallResult M_MessageWarning( CIProcess *pProcess, CISyscall **ppSyscall, CISystemObject *pText );
  // Displayes warning message on the client with location
  ESyscallResult M_MessageWarningLocation( CIProcess *pProcess, CISyscall **ppSyscall, CISystemObject *pText, CISystemObject *pLocation );
  // Displayes warning message on the client with unit
  ESyscallResult M_MessageWarningUnit( CIProcess *pProcess, CISyscall **ppSyscall, CISystemObject *pText, CZUnit *pUnit );

  // Displayes error message on the client
  ESyscallResult M_MessageError( CIProcess *pProcess, CISyscall **ppSyscall, CISystemObject *pText );
  // Displayes error message on the client with location
  ESyscallResult M_MessageErrorLocation( CIProcess *pProcess, CISyscall **ppSyscall, CISystemObject *pText, CISystemObject *pLocation );
  // Displayes error message on the client with unit
  ESyscallResult M_MessageErrorUnit( CIProcess *pProcess, CISyscall **ppSyscall, CISystemObject *pText, CZUnit *pUnit );

  // Stops user input
  // This also makes the object invalid
  ESyscallResult M_CloseUserInput ( CIProcess *pProcess, CISyscall **ppSyscall );

  // Asks the user for position
  ESyscallResult M_AskForPosition ( CIProcess *pProcess, CISyscall **ppSyscall, CIDataStackItem *pRet, CISystemObject *pQuestion, CISystemObject *pPosition );
  // Asks the user for enemy unit
  ESyscallResult M_AskForEnemyUnit ( CIProcess *pProcess, CISyscall **ppSyscall, CIDataStackItem *pRet, CISystemObject *pQuestion );
  // Asks the user for my unit
  ESyscallResult M_AskForMyUnit ( CIProcess *pProcess, CISyscall **ppSyscall, CIDataStackItem *pRet, CISystemObject *pQuestion );
  // Asks the user for any unit
  ESyscallResult M_AskForAnyUnit ( CIProcess *pProcess, CISyscall **ppSyscall, CIDataStackItem *pRet, CISystemObject *pQuestion );

private:
  // The connected client object
  CZConnectedClient *m_pConnectedClient;
  // This pointer is allways valid
  CZCivilization *m_pCivilization;

  // Next client object in the list of existing client objects
  // on the CZConnectedClient
  CISOClient *m_pNext;

  // ID of the process we were started on
  // Just a helper variable for the CZConnectedClient to kill our process
  DWORD m_dwProcessID;

  // TRUE if the object is in active state (can use hard methods)
  BOOL m_bActive;

  CCriticalSection m_lockLock;

  friend class CZConnectedClient;
};

#endif //_ISOCLIENT_H_