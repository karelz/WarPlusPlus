// CWatchedRectangle.h: interface for the CCWatchedRectangle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CWATCHEDRECTANGLE_H__43A8F8C1_106D_11D4_8044_0000B48431EC__INCLUDED_)
#define AFX_CWATCHEDRECTANGLE_H__43A8F8C1_106D_11D4_8044_0000B48431EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCServerUnitInfoReceiver;

class CCWatchedRectangle  
{
// Konstrukce a destrukce
public:
	// Konstruktor
	CCWatchedRectangle();

	// Jeste jeden konstruktor
	CCWatchedRectangle(CCServerUnitInfoReceiver *pInfoReceiver);

	// Vytvoreni (volano z konstruktoru)
	void Create(CCServerUnitInfoReceiver *pInfoReceiver);

	// Zruseni (volano z destruktoru)
	void Delete();

	// Destruktor
	virtual ~CCWatchedRectangle();

public:
	// Updatne data na serveru (pokud uzna za vhodne)
	void UpdateServerData();

	// Nastavi obdelnik na nulovy, tim ho jakoby vypne
	void Empty() { SetDimensions(0, 0, 0, 0); }

	// Odregistruje obdelni
	void Unregister();

// Nastavovani pozice a velikosti
public:
	// Nastavi rozmery
	void SetDimensions(long lLeft, long lTop, long lWidth, long lHeight);

	// Nastavi pozici
	void SetPos(long lLeft, long lTop);

	// Nastavi velikost
	void SetSize(long lWidth, long lHeight);

// Zjisteni informaci
public:
	// ID
	DWORD ID() { return m_dwID; }

// Rozmery v mapselech
public:
	// Zjisteni leveho okraje
	long Left() { return m_lLeft; };

	// Zjisteni horniho okraje
	long Top() { return m_lTop; };

	// Zjisteni sirky
	long Width() { return m_lWidth; };

	// Zjisteni vysky
	long Height() { return m_lHeight; };

// Rozmery v mapsquarech
public:
	// Zjisteni leveho okraje
	long LeftMapSquare() { return m_lLeftMapSquare; }

	// Zjisteni horniho okraje
	long TopMapSquare() { return m_lTopMapSquare; }

	// Zjisteni sirky
	long WidthMapSquare() { return m_lWidthMapSquare; }

	// Zjisteni vysky
	long HeightMapSquare() { return m_lHeightMapSquare; }

// Zapojovani do spojaku
public:
	// Pripojeni na konec seznamu
	void Append(CCWatchedRectangle *pList);

	// Pripojeni na zacatek seznamu
	void Insert(CCWatchedRectangle *pList);

	// Odpojeni se ze seznamu
	void Disconnect();

	// Dalsi obdelnik
	CCWatchedRectangle *Next() { return m_pNext; }

	// Nalezne dany obdelnik
	CCWatchedRectangle *FindRectangle(DWORD dwID);

	// Nastavi flag, ze uz je rectangle sledovany
	void Watched(BOOL bWatched=TRUE) { m_bWatched=bWatched; }

// Data
private:
	// Identifikace
	DWORD m_dwID;

	// Nove ID pro vznikajici rectangle
	static DWORD m_dwNewFreeID;

	// Zamek pro pridelovani ID
	static CMutex m_lockNewFreeID;

	// Info receiver, kteremu patrime
	CCServerUnitInfoReceiver *m_pInfoReceiver;
	
// Aktualni rozmery (v mapcellech)
private:
	// Levy okraj
	long m_lLeft;
	// Vrsek
	long m_lTop;
	// Vyska
	long m_lWidth;
	// Sirka
	long m_lHeight;

// Aktualni rozmery s napocitanymi okraji (v mapsquarech)
private:
	// Levy okraj
	long m_lLeftMapSquare;
	// Vrsek
	long m_lTopMapSquare;
	// Vyska
	long m_lWidthMapSquare;
	// Sirka
	long m_lHeightMapSquare;

// Predchozi rozmery s napocitanymi okraji (v mapsquarech)
private:
	// Levy okraj
	long m_lLastLeftMapSquare;
	// Vrsek
	long m_lLastTopMapSquare;
	// Vyska
	long m_lLastWidthMapSquare;
	// Sirka
	long m_lLastHeightMapSquare;

// Naposled nabonzovane rozmery (v mapsquarech)
private:
	// Levy okraj
	long m_lLastLeft;
	// Vrsek
	long m_lLastTop;
	// Vyska
	long m_lLastWidth;
	// Sirka
	long m_lLastHeight;

// Aktualni vektor pohybu
private:
	// O kolik se posunul viewport ve smeru osy X (v mapcellech)
	int m_nXMoveVertex;
	// O kolik se posunul viewport ve smeru osy Y (v mapcellech)
	int m_nYMoveVertex;

// Vazby do spojaku
// Spojak je obousmerny a zacykleny dokolecka!
private:
	// Predchozi obdelnik
	CCWatchedRectangle *m_pPrev;

	// Dalsi obdelnik
	CCWatchedRectangle *m_pNext;

// Je sledovany?
	BOOL m_bWatched;

// Pomocna funkce
private:
  // Oreze rozmery obdelniku, aby se vesel na mapu
  void ClipDimensions();

};

#endif // !defined(AFX_CWATCHEDRECTANGLE_H__43A8F8C1_106D_11D4_8044_0000B48431EC__INCLUDED_)
