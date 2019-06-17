// ZMiniMapRequest.h: interface for the CZMiniMapRequest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZMINIMAP_H__CFB99743_6149_11D4_80B2_8EF14AD41A06__INCLUDED_)
#define AFX_ZMINIMAP_H__CFB99743_6149_11D4_80B2_8EF14AD41A06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Network/Network/VirtualConnection.h"
#include "Common/ServerClient/MiniMapStructures.h"

// Objekt, ktery reprezentuje klientuv pozadavek na minimapu na strane serveru
// Server pri sdelovani pozic jednotek komunikuje primo s nim

class CZMiniMapRequest
{
// Konstrukce a destrukce
public:
	// Konstruktor
	CZMiniMapRequest();

	// Destruktor
	virtual ~CZMiniMapRequest();

// Vytvareni a ruseni
public:
	// Vytvoreni
	void Create(CVirtualConnection VirtualConnection);

	// Zruseni
	void Delete();

// Debugovani
#ifdef _DEBUG
public:
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// Nastavovani dat
public:
	// Nastaveni pozadovaneho vyrezu minimapy
	void SetClip(int nLeft, int nTop, int nWidth, int nHeight, int nZoom);

	// Nastaveni pozadovaneho vyrezu minimapy
	void SetClip(SMiniMapClip Clip) {
		SetClip(Clip.nLeft, Clip.nTop, Clip.nWidth, Clip.nHeight, Clip.nZoom);
	}

// Ziskavani dat
public:
	// Zjisteni dat o vyrezu minimapy
	void GetClip(int &nLeft, int &nTop, int &nWidth, int &nHeight, int &nZoom);

	// Zjisteni dat o vyrezu minimapy
	void GetClip(SMiniMapClip &Clip) {
		GetClip(Clip.nLeft, Clip.nTop, Clip.nWidth, Clip.nHeight, Clip.nZoom);
	}

	// Zamknuti objektu behem doby, kdy se zpracovava minimapa
	// Zavolat pred volanim GetClip
	void CalculationStart();

	// Odemknuti objektu pote, co bylo zpracovavani dokonceno
	// Zavolat po volani SendMiniMap
	void CalculationStop();

// Zasilani dat
public:
	// Posle pres virtualni spojeni objektu spakovanou minimapu na klienta
	void SendMiniMap(int nLeft, int nTop, int nWidth, int nHeight, int nZoom, void *pData, DWORD dwSize, DWORD dwUnpackedSize) {
		SMiniMapClip Clip(nLeft, nTop, nWidth, nHeight, nZoom);		
		SendMiniMap(Clip, pData, dwSize, dwUnpackedSize);
	}

	// Posle pres virtualni spojeni objektu spakovanou minimapu na klienta
	void SendMiniMap(SMiniMapClip Clip, void *pData, DWORD dwSize, DWORD dwUnpackedSize);

// Data
private:
	// Sledovany obdelnik - levy okraj (v mapcellech)
	int m_nLeft;
	
	// Sledovany obdelnik - pravy okraj (v mapcellech)
	int m_nTop;

	// Sledovany obdelnik - sirka (v mapcellech)
	int m_nWidth;
	
	// Sledovany obdelnik - vyska (v mapcellech)
	int m_nHeight;

	// Pozadovane zmenseni. 
	// Povolene hodnoty 2, 4, 8, 16
	int m_nZoom;

// Synchronizace
private:
	// Zamek pro pristup k hodnotam
	CMutex m_lockDataAccess;

	// Priznak, ze je zrovna pozadavek vyrizovan
	BOOL m_bCalculationInProgress;

	// Signalizace, ze bylo dokonceno zpracovavani
	CEvent m_eventCalculationFinished;	

	// Virtualni spojeni na klienta
	CVirtualConnection m_VirtualConnection;
};

#endif // !defined(AFX_ZMINIMAP_H__CFB99743_6149_11D4_80B2_8EF14AD41A06__INCLUDED_)
