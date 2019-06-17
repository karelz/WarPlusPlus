// DataArchiveFileDataList.h: interface for the CDataArchiveFileDataList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAARCHIVEFILEDATALIST_H__96DEC043_769D_11D3_A750_0040332A3CC0__INCLUDED_)
#define AFX_DATAARCHIVEFILEDATALIST_H__96DEC043_769D_11D3_A750_0040332A3CC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afx.h>
#include "MappedFile.h"
#include "Definitions.h"

#pragma pack(1) // Zarovnani polozek struktury na bajty

// Blok dat s moznosti retezeni do seznamu
struct SDataArchiveFileDataItem {
	// Offset predchoziho bloku (pokud 0, predchozi blok neni)
	DWORD dwPrevOffset;
	// Offset dalsiho bloku (pokud 0, dalsi blok neni)
	DWORD dwNextOffset;

	// Kde v souboru zacinaji ma data
	DWORD dwDataOffset;
	// Kde v listu jsou ma data
	DWORD dwListOffset;

	// Kolik mista ma blok naalokovano
	DWORD dwDataSizeAllocated; 
};

// Seznam bloku
struct SDataArchiveFileDataList {
	// Pocet polozek seznamu
	int nItems;

	// Velikost dat v listu
	DWORD dwLength;

	// Skutecna velikost dat (po dekompresi)
	DWORD dwRealLength;
	
	// Pozice hlavy (prvni DataItem)
	DWORD dwHeadOffset;
	// Pozice ocasu (posledniDataItem)
	DWORD dwTailOffset;
};

#pragma pack() // Konec zarovnani polozek struktury na bajty

class CDataArchiveFileDataList : public CObject
{
public:
	// Konstruktor
	CDataArchiveFileDataList();

	// Destruktor
	virtual ~CDataArchiveFileDataList();

	// Otevre seznam ze souboru na dane pozici
	void Open(CMappedFile &File, DWORD dwOffset);

	// Zavre seznam
	void Close();

	// Vytvori novy seznam dane velikosti a vrati jeho offset
	// Pokud je bGonnaFill TRUE, znamena to, ze v zapeti tento seznam
	// BUDE zaplnen daty o velikosti dwSize, takze se mu rovnou velikost
	// techto dat zapise do hlavicky
	DWORD Create(CMappedFile &File, DWORD dwSize, BOOL bGonnaFill=FALSE);

	// Nacte data
	UINT Read(void *pData, UINT nCount);

	// Zapise data
	void Write(void *pData, UINT nCount);

	// Vyprazdni retezec
	void Empty();

	// Flushne data do souboru
	void Flush();

	// Seek na danou pozici v ramci seznamu
	void Seek(DWORD dwOffset, UINT nFrom);

	// Vraci pouzivanou delku retezce
	DWORD GetLength();

	// Nastavuje delku retezce
	void SetLength(DWORD dwLength);

	// Vraci realnou delku retezce (po dekompresi)
	DWORD GetRealLength();

	// Nastavuje realnou delku retezce
	void SetRealLength(DWORD dwLength);

	// Vraci aktualni pozici
	DWORD GetPosition();

	// Vrati soubor, nad kterym operuje
	CMappedFile *GetFile() { return m_pFile; }

	// Nastavi soubor, nad kterym operuje
	void SetFile(CMappedFile *pFile) { m_pFile=pFile; }

	// Nastavi granularitu (po kolika bajtech minimalne roste soubor pri psani)
	void SetGranularity(DWORD dwGranularity) {
		ASSERT(dwGranularity>0);
		m_dwGranularity=dwGranularity;
	}

private:
	// Presun na dalsi blok. Vraci true, pokud se zadarilo
	BOOL NextItem();

	// Presun na predchozi blok. Vraci true, pokud se zadarilo
	BOOL PrevItem();

	// Presun na prvni blok. Vraci true, pokud se zadarilo
	BOOL FirstItem();

private:
	// Ukazatel na CMappedFile
	CMappedFile *m_pFile;

	// Pozice hlavicky seznamu
	DWORD m_dwListOffset;

	// Data hlavicky seznamu
	SDataArchiveFileDataList m_DataList;

	// Pozice hlavicky bloku
	DWORD m_dwItemOffset;

	// Data polozky seznamu
	SDataArchiveFileDataItem m_DataItem;

	// Pozice v souboru
	DWORD m_dwPosition;

	// Po jakych blocich se pridavaji itemy na konec pri zapisu?
	DWORD m_dwGranularity;
};

#endif // !defined(AFX_DATAARCHIVEFILEDATALIST_H__96DEC043_769D_11D3_A750_0040332A3CC0__INCLUDED_)
