// ZRequestVirtualConnection.h: interface for the CZRequestVirtualConnection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REQUESTVIRTUALCONNECTION_H__6D9B2C08_0AC9_11D4_8030_0000B4A08F9A__INCLUDED_)
#define AFX_REQUESTVIRTUALCONNECTION_H__6D9B2C08_0AC9_11D4_8030_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CZClientUnitInfoSender;

// Spojeni zajistujici obsluhu pozadavku uzivatele
class CZRequestVirtualConnection : public CObserver  
{
	// Vyhlasime pozorovateli mapu
	DECLARE_OBSERVER_MAP(CZRequestVirtualConnection);

// Konstanty
private:
	enum ERequestVirtualConnectionConstants {
		// Velikost bufferu na prijimani zprav
		ReceiveBufSize=10000
	};

// Konstrukce a destrukce
public:
	// Konstruktor
	CZRequestVirtualConnection();

	// a jeste jeden Konstruktor
    CZRequestVirtualConnection(CVirtualConnection VirtualConnection);

	// Destruktor
	virtual ~CZRequestVirtualConnection();

// Vytvareni a ruseni
public:
    // Vytvoreni
    void Create();

    // Zruseni
    void Delete();

// Metody
public:
	// Napojeni na CZClientUnitInfoSendera
	void SetClientUnitInfoSender(CZClientUnitInfoSender *pSender);

	// Inicializace virtualniho spojeni
	void SetConnection(CVirtualConnection VirtualConnection);

	// Prisel paket na nejakem spojeni
	void OnPacketAvailable(WORD wConnectionNumber);

private:
	// Vlastni spojeni
	CVirtualConnection m_VirtualConnection;

	// Ukazatel na CZClientUnitInfoSendera
	CZClientUnitInfoSender *m_pSender;
};

#endif // !defined(AFX_REQUESTVIRTUALCONNECTION_H__6D9B2C08_0AC9_11D4_8030_0000B4A08F9A__INCLUDED_)
