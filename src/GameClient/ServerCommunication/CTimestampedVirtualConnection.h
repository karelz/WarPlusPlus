// CTimestampedVirtualConnection.h: interface for the CCTimestampedVirtualConnection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CTIMESTAMPEDVIRTUALCONNECTION_H__43A8F8C4_106D_11D4_8044_0000B48431EC__INCLUDED_)
#define AFX_CTIMESTAMPEDVIRTUALCONNECTION_H__43A8F8C4_106D_11D4_8044_0000B48431EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Common/ServerClient/VirtualConnectionIDs.h"
#include "Common/ServerClient/CommunicationInfoStructures.h"

class CCServerUnitInfoReceiver;

class CCUnit;
// Tohle je obalka na CVirtualConnection
class CCTimestampedVirtualConnection : public CObserver {
	// Vyhlasime pozorovateli mapu
	DECLARE_OBSERVER_MAP(CCTimestampedVirtualConnection);

// Konstanty
public:
	enum ECTimestampedVirtualConnectionConstants {
		// Velikost bufferu na prijimani zprav
		ReceiveBufSize=20480,

		// Identifikace spojeni
		ID_TimestampedVirtualConnection=0x8100
	};
	
	// Konstrukce a destrukce
public:
	// Konstruktor
	CCTimestampedVirtualConnection();

	// a jeste jeden Konstruktor
    CCTimestampedVirtualConnection(CVirtualConnection VirtualConnection, EVirtualConnectionID dwID, CCServerUnitInfoReceiver *pReceiver);

	// Destruktor
	virtual ~CCTimestampedVirtualConnection();

	// Vytvoreni
	void Create(CVirtualConnection VirtualConnection, EVirtualConnectionID dwID, CCServerUnitInfoReceiver *pReceiver);

  // Niceni
  virtual void Delete();

// Metody
public:
	// Zjisteni aktualniho casu
	DWORD GetTime();

	// Nastavi odkaz na spojeni
	void SetConnection(CVirtualConnection Connection) { m_VirtualConnection=Connection; }

	// Vrati odkaz na spojeni
	CVirtualConnection GetConnection() { return m_VirtualConnection; }

	// Prisel paket na nejakem spojeni
	void OnPacketAvailable(WORD wConnectionNumber);
   
// Private metody
private:
	
	// Nasledujici metody vzdy z bloku prectou JEDNU informaci
	// a vrati true, pokud se ma pokracovat
	BOOL ParseControl(DWORD dwTime);
	BOOL ParseBriefInfo(DWORD dwTime);
	BOOL ParseCheckPointInfo(DWORD dwTime);
	BOOL ParseFullInfo(DWORD dwTime);
	BOOL ParseEnemyFullInfo(DWORD dwTime);

	// Metody volane z ParseControl
	void ParseStartInfo(SControlConnectionStartInfo &StartInfo, DWORD dwTime);
	void ParseStopInfo(SControlConnectionStopInfo &StopInfo, DWORD dwTime);
	void ParseStartFullInfo(SUnitStartFullInfo &StartFullInfo, BYTE *pAdditionalInfo, DWORD dwAdditionalInfoSize, DWORD dwTime);
	void ParseStopFullInfo(SControlConnectionStopFullInfo &StopFullInfo, DWORD dwTime);
	void ParseStartEnemyFullInfo(SUnitStartEnemyFullInfo &StartEnemyFullInfo, BYTE *pAdditionalInfo, DWORD dwAdditionalInfoSize, DWORD dwTime);
	void ParseStopEnemyFullInfo(SControlConnectionStopEnemyFullInfo &StopEnemyFullInfo, DWORD dwTime);

  // You must call Release on returned unit
  CCUnit * CreateNewUnit(SUnitStartInfo &StartInfo, DWORD dwTime);
  // Applies breif info to the unit
  void ApplyBriefInfo(CCUnit *pUnit, SUnitBriefInfo &BriefInfo, DWORD dwTime);
  // Applies checkpoint info on the unit
  void ApplyCheckPointInfo(CCUnit *pUnit, SUnitCheckPointInfo &CheckPointInfo, DWORD dwTime);

private:
	// Identifikacni cislo pro spojeni
	DWORD m_dwID;

	// Ukazatel na spojeni
	CVirtualConnection m_VirtualConnection;
  BOOL m_bVirtualConnectionCreated;

	// Ukazatel na info receivera
	CCServerUnitInfoReceiver *m_pReceiver;

	// Aktualni cas
	DWORD m_dwActualTime;
};

#endif // !defined(AFX_CTIMESTAMPEDVIRTUALCONNECTION_H__43A8F8C4_106D_11D4_8044_0000B48431EC__INCLUDED_)
