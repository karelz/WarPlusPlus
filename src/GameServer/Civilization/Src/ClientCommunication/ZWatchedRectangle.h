// ZWatchedRectangle.h: interface for the CZWatchedRectangle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WATCHEDRECTANGLE_H__6D9B2C09_0AC9_11D4_8030_0000B4A08F9A__INCLUDED_)
#define AFX_WATCHEDRECTANGLE_H__6D9B2C09_0AC9_11D4_8030_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CZClientUnitInfoSender;

class CZWatchedRectangle  
{
// Konstrukce a destrukce
public:
	// Konstruktor
	CZWatchedRectangle();

	// Jeste jeden konstruktor
	CZWatchedRectangle(CZClientUnitInfoSender *pInfoSender, DWORD dwID);

	// Vytvoreni (volano z konstruktoru)
	void Create(CZClientUnitInfoSender *pInfoSender, DWORD dwID);

	// Destruktor
	virtual ~CZWatchedRectangle();

// Notifikace mapy
public:
	// Zpravi mapu o zmene rozmeru
	void NotifyMap();

	// Nastavi obdelnik na nulovy, tim ho jakoby vypne
	void Empty() { SetDimensions(0, 0, 0, 0); }

// Nastavovani pozice a velikost
public:
	// Nastavi rozmery
	void SetDimensions(DWORD dwLeft, DWORD dwTop, DWORD dwWidth, DWORD dwHeight) {
		m_dwLeft=dwLeft; m_dwTop=dwTop; m_dwWidth=dwWidth; m_dwHeight=dwHeight;
	};

	// Nastavi pozici
	void SetPos(DWORD dwLeft, DWORD dwTop) {
		m_dwLeft=dwLeft; m_dwTop=dwTop; 
	};

	// Nastavi velikost
	void SetSize(DWORD dwWidth, DWORD dwHeight) {
		m_dwWidth=dwWidth; m_dwHeight=dwHeight;
	};

// Zjisteni pozice a velikosti
public:
	// Zjisteni leveho okraje
	DWORD Left() { return m_dwLeft; };

	// Zjisteni horniho okraje
	DWORD Top() { return m_dwTop; };

	// Zjisteni sirky
	DWORD Width() { return m_dwWidth; };

	// Zjisteni vysky
	DWORD Height() { return m_dwHeight; };

// Zapojovani do spojaku
public:
	// Pripojeni na konec seznamu
	void Append(CZWatchedRectangle *pList);

	// Pripojeni na zacatek seznamu
	void Insert(CZWatchedRectangle *pList);

	// Odpojeni se ze seznamu
	void Disconnect();

	// Dalsi obdelnik
	CZWatchedRectangle *Next() { return m_pNext; }

	// Nalezne dany obdelnik
	CZWatchedRectangle *FindRectangle(DWORD dwID);

// Data
private:
	// Identifikace
	DWORD m_dwID;

	// Info sender, kteremu patrime
	CZClientUnitInfoSender *m_pInfoSender;
	
// Aktualni rozmery
private:
	// Levy okraj
	DWORD m_dwLeft;
	// Vrsek
	DWORD m_dwTop;
	// Vyska
	DWORD m_dwWidth;
	// Sirka
	DWORD m_dwHeight;

// Naposled nabonzovane rozmery
private:
	// Levy okraj
	DWORD m_dwLastLeft;
	// Vrsek
	DWORD m_dwLastTop;
	// Vyska
	DWORD m_dwLastWidth;
	// Sirka
	DWORD m_dwLastHeight;

// Vazby do spojaku
private:
	// Predchozi obdelnik
	CZWatchedRectangle *m_pPrev;

	// Dalsi obdelnik
	CZWatchedRectangle *m_pNext;
};

#endif // !defined(AFX_WATCHEDRECTANGLE_H__6D9B2C09_0AC9_11D4_8030_0000B4A08F9A__INCLUDED_)
