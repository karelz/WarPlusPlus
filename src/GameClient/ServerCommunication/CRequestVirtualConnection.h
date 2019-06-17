// CRequestVirtualConnection.h: interface for the CCRequestVirtualConnection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CREQUESTVIRTUALCONNECTION_H__3B3AB1F3_1098_11D4_8048_0000B48431EC__INCLUDED_)
#define AFX_CREQUESTVIRTUALCONNECTION_H__3B3AB1F3_1098_11D4_8048_0000B48431EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Network/Network/VirtualConnection.h"

struct SMiniMapClip;
class CCWatchedRectangle;
class CCUnit;

class CCRequestVirtualConnection : public CObserver
{
// Vyhlasime pozorovateli mapu
		DECLARE_OBSERVER_MAP(CCRequestVirtualConnection);

// Konstanty
public:
	enum ERequestVirtualConnectionConstants {
		// ID pro sit
		ID_RequestVirtualConnection=0x8100
	};

// Konstrukce a destrukce
public:
	// Konstruktor
	CCRequestVirtualConnection();

	// Jeste jeden konstruktor
	CCRequestVirtualConnection(CVirtualConnection Connection);

	// Destruktor
	virtual ~CCRequestVirtualConnection();

// Metody
public:
	// Vytvoreni spojeni
	void Create(CVirtualConnection Connection);

	// Niceni
	void Delete();

	// Posle zpravu, ze jsme se zacali divat na obdelnik
	void SendStartWatchingRectangle(CCWatchedRectangle *pRect);

	// Posle zpravu, ze jsme se prestali divat na obdelnik
	void SendStopWatchingRectangle(CCWatchedRectangle *pRect);

	// Posle zpravu, ze se obdelnik pohnul
	void SendRectangleMoved(CCWatchedRectangle *pRect);

	// Posle zpravu, ze chceme full info dane jednotky
	void SendStartFullInfo(CCUnit *pUnit);

	// Posle zpravu, ze nechceme full info dane jednotky
	void SendStopFullInfo(CCUnit *pUnit);

	// Posle zpravu, ze koncime
	void SendCloseComms();

	// Posle zadost o vyrez minimapy
	void SendMiniMapClip(SMiniMapClip Clip);

// Data
private:
	// Vlastni spojeni
	CVirtualConnection m_Connection;

	// Flag, zda je virtualni spojeni funkcni
	BOOL m_bVirtualConnectionCreated;

    // Zamek na spojeni
    CMutex m_lockConnection;
};

#endif // !defined(AFX_CREQUESTVIRTUALCONNECTION_H__3B3AB1F3_1098_11D4_8048_0000B48431EC__INCLUDED_)
