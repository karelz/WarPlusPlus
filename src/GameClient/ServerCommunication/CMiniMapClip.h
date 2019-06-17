// CMiniMapClip.h: interface for the CCMiniMapClip class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CMINIMAPOBSERVER_H__F7C02933_62CE_11D4_80B5_FD78B9BD1E06__INCLUDED_)
#define AFX_CMINIMAPOBSERVER_H__F7C02933_62CE_11D4_80B5_FD78B9BD1E06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Common/ServerClient/MiniMapStructures.h"
#include "Archive/Compression/CompressEngine.h"

class CCRequestVirtualConnection;
class CCMiniMapClip;

// Uzce svazana trida - zdroj dat pro dekompresi
class CCMiniMapClipDataSource : public CCompressDataSource
{
// Konstrukce a destrukce
public:
	// Konstruktor
	CCMiniMapClipDataSource() { m_pClip=NULL; }

	// Destruktor
	virtual ~CCMiniMapClipDataSource() { m_pClip=NULL; }

// Vytvoreni a zruseni
public:
	// Vytvoreni
	void Create(CCMiniMapClip *pClip) { ASSERT(m_pClip==NULL); m_pClip=pClip; }

	// Zruseni
	void Delete() { m_pClip=NULL; }

// Dekomprese
public:
	// Vyprodukuje blok dat a vrati jeho velikost
	virtual ECompressData GiveData(void *&lpBuf, DWORD &dwSize);

	// Vrati nejaky retezec umoznujici identifikaci (napr. cestu k souboru, odkud source cte)
	virtual CString GetID() { return "MiniMap"; }

	// Vrati velikost nekomprimovaneho souboru
	virtual DWORD GetUncompressedSize();

// Data
private:
	CCMiniMapClip *m_pClip;
};

class CCMiniMapClip : public CNotifier
{
friend class CCMiniMapClipDataSource;

// Vyhlasime pozorovateli mapu
DECLARE_OBSERVER_MAP(CCMiniMapClip);

// Konstrukce a destrukce
public:
	// Konstruktor
	CCMiniMapClip();

	// Destruktor
	virtual ~CCMiniMapClip();

// Vytvareni a ruseni
public:
	// Vytvoreni
	void Create(CVirtualConnection *pMiniMapConnection, CCRequestVirtualConnection *pRequestConnection);

	// Zruseni
	void Delete();

// Pozadavky od uzivatele
public:
	// Pozadavek na vyrez
	void RequestClip(int nLeft, int nTop, int nWidth, int nHeight, int nZoom) {
		SMiniMapClip Clip(nLeft, nTop, nWidth, nHeight, nZoom);
		RequestClip(Clip);
	}

	// Pozadavek na vyrez
	void RequestClip(SMiniMapClip Clip);

// Notifikace
public:
	// Tato funkce je vyvolana, kdyz prijdou nova data
	// To neznamena, ze nez stihne uzivatel zareagovat, ze tam ty data musi byt
	void NotifyDataArrived(SMiniMapClip Clip);

  enum{ E_ClipChanged = 1, };

// Cteni dat minimapy
public:
	// Zamkne data na cteni. Kdyz prijdou nova, ignoruji se a drzi tam tyhle
	void LockClip();

	// Odemkne data na cteni, takze mohou chodit dalsi
	void UnlockClip();

	// Vrati rozmery aktualnich dat
	SMiniMapClip GetClip();

	// Vrati, zdali jsou prisla data stejne zoomnuta, jako pozadovana
	BOOL IsCorrectZoom();

	// Vrati velikost jedne radky dat v bajtech
	DWORD GetScanLineSize();

	// Nastartuje cteni dat
	void StartDataReading();

	// Predcasne zrusi cteni dat
	void CancelDataReading();

	// Nacte do dane pameti dalsi kus dat, vraci FALSE, pokud jsme na konci
	BOOL ReadData(BYTE *pData, DWORD &dwSize);

	// Vrati true, pokud jsme na konci dat pro cteni
	BOOL IsEOF();
	
	// Vrati cislo sloupce, ze ktereho se budou cist data
	// (cislovane od 0 pro prvni sloupec vyrezu)
	int GetX();

	// Vrati cislo radky, ze ktereho se budou cist data
	// (cislovane od 0 pro prvni radku vyrezu)
	int GetY();

	// Vrati x-ovou souradnici mapcellu, ze ktereho se budou cist data
	int GetMapX();

	// Vrati y-ovou souradnici mapcellu, ze ktereho se budou cist data
	int GetMapY();

// Prijimani dat po siti
public:
	// Prisel paket na nejakem spojeni
	void OnPacketAvailable(WORD wConnectionNumber);

// Konstanty
public:
	enum EMiniMapConnectionConstants {
		// Pocatecni velikost bufferu pro prijimani pakovanych dat
		cPackedMiniMapSize=20480,

		// Identifikace spojeni
		ID_MiniMapVirtualConnection=0x8200
	};

// Data
private:
	// Buffer na spakovana data
	BYTE *m_pPackedData;

	// Velikost bufferu
	DWORD m_dwMaxPackedDataSize;
	
	// Aktualni velikost spakovanych dat
	DWORD m_dwPackedDataSize;

	// Aktualni pozice, ze ktere se ctou spakovana data
	BYTE *m_pPackedDataPosition;

	// Poradi bajtu, ze ktereho se ctou spakovana data
	DWORD m_dwPackedDataPosition;

	// Velikost dat po rozpakovani
	DWORD m_dwUncompressedDataSize;

	// Aktualni vyrez, ktery prisel ze serveru
	SMiniMapClip m_DownloadedClip;

	// Vyrez pozadovany uzivatelem
	SMiniMapClip m_RequestedClip;

	// Spojeni na zasilani requestu
	CCRequestVirtualConnection *m_pRequestConnection;

	// Spojeni na nacitani dat
	CVirtualConnection *m_pMiniMapConnection;

	// Priznak, ze jsou data zamcena
	BOOL m_bDataLocked;

    // Zamek na kterem se ceka pri prichodu dat
    CMutex m_lockDataLocked;

	// Zamek na pakovana data, zamezuje nekolikanasobnemu pristupu
	CMutex m_lockPackedData;

	// Zdroj dat pro rozpakovani
	CCMiniMapClipDataSource m_DataSource;

	// Pakostroj
	CCompressEngine m_Engine;

    // Priznak, ze muzeme request poslat na server
    BOOL m_bClearToSendRequest;

    // Priznak, ze jsme chteli posilat na server request, ale neslo to
    BOOL m_bRequestNotSent;

// Debugovani
#ifdef _DEBUG
public:
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext &dc) const;
#endif

};

#endif // !defined(AFX_CMINIMAPOBSERVER_H__F7C02933_62CE_11D4_80B5_FD78B9BD1E06__INCLUDED_)
