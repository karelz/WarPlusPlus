// MiniMapServer.h: interface for the CMiniMapServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MINIMAPSERVER_H__E5F3A6C6_61EE_11D4_80B3_DC3A991F1B06__INCLUDED_)
#define AFX_MINIMAPSERVER_H__E5F3A6C6_61EE_11D4_80B3_DC3A991F1B06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Archive/Compression/CompressEngine.h"
#include "../MiniMap/ZMiniMapRequest.h"

// Objekt, ktery zajistuje pocitani minimap pro jednotlive requesty

class CMiniMapServer : CCompressDataSink
{
// Konstrukce a destrukce
public:
	// Konstruktor
	CMiniMapServer();

	// Destruktor
	virtual ~CMiniMapServer();

// Vytvareni a ruseni
public:
	// Vytvoreni
	void Create();

	// Zruseni
	void Delete();

// Sypani dat
public:
	// Zacne zpracovavat zadany pozadavek na minimapu
	void StartRenderingUnits(CZMiniMapRequest *pRequest);

	// Nakresli do minimapy na dane misto jednotku patrici zadane civilizaci
	// (cislo civilizace musi byt mezi 0 a 254) Pokud je prujezdna velikost
	// jednotky vetsi nez jeden mapcell, je mozne ji nastavit
	void RenderUnit(DWORD dwX, DWORD dwY, int nCivilization, int nSize=1);

	// Signalizuje skonceni zpracovavani pozadavku na minimapu
	void StopRenderingUnits();

// Implemenace DataSinku
private:
	// Ulozi blok dat, ktery se mu podstrci
	virtual ECompressData StoreData(void *lpBuf, DWORD dwSize);

	// Vrati buffer, pomoci ktereho se bude ukladat
	virtual void GetStoreBuffer(void *&pBuffer, DWORD &dwSize);

	// Vrati nejaky retezec umoznujici identifikaci (napr. cestu k souboru, kam sink pise)
	virtual CString GetID() { return "MiniMap"; }

// Data
private:
	// Buffer pro data
	BYTE *m_pData;

	// Velikost bufferu
	DWORD m_dwDataSize;

	// Ukazatel na zacatek nepakovanych dat (v ramci bufferu m_pData)
	BYTE *m_pMapData;

	// Velikost nepakovanych dat
	DWORD m_dwMapDataSize;

	// Ukazatel na aktualni pozici v ramci dat z minimapy
	BYTE *m_pMapDataSource;

	// Ukazatel na aktualni pozici v ramci pakovanych dat (uvnitr bufferu m_pData)
	BYTE *m_pPackedData;

	// Aktualni velikost spakovanych dat
	DWORD m_dwPackedDataSize;

	// Aktualne zpracovavany pozadavek klienta
	CZMiniMapRequest *m_pRequest;

	// Aktualni obdelnik na zpracovani
	SMiniMapClip m_Clip;

// Debugovani
#ifdef _DEBUG
public:
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif
};

#endif // !defined(AFX_MINIMAPSERVER_H__E5F3A6C6_61EE_11D4_80B3_DC3A991F1B06__INCLUDED_)
