// MappedFile.h: interface for the CMappedFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAPPEDFILE_H__96DEC042_769D_11D3_A750_0040332A3CC0__INCLUDED_)
#define AFX_MAPPEDFILE_H__96DEC042_769D_11D3_A750_0040332A3CC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afx.h>
#include "Definitions.h"

class CMappedFileCommon;

// Memory-mapped file (zatim implementovano jako obycejny soubor)
class CMappedFile : public CObject
{
	// Nase pokracovani, spolecne vice CMappedFilum
	friend class CMappedFileCommon;
	
// Konfiguracni konstanty
public:
	enum {
		MAPPED_FILE_CACHE_SIZE=1024, // Tolik mame cache
		CACHE_TRIGGER_SIZE=100,	// Kdyz se nacita mene nebo rovno tomuhle cislu, 
								// nacte se rovnou cela cache. Jinak se cache nepouziva
	};

public:
	// Konstruktor
	CMappedFile();

	// Copy konstruktor
	CMappedFile(CMappedFile *pMappedFile);

	// Destruktor
	virtual ~CMappedFile();

	// Otevreni namapovaneho souboru
	void Open(LPCTSTR FileName, UINT nOpenFlags);

    // Pokud byl soubor jiz jednou otevren a uzavren, timto se znovu otevre
    void Reopen() {};

	// Uzavreni namapovaneho souboru
	void Close();

    // Zjisteni, jestli je soubor otevreny
	BOOL IsOpened() { return TRUE; }

	// Flushnuti dat
	void Flush();

	// Seek na danou pozici
	void Seek(DWORD dwOffset, UINT nFrom);

	// Cteni dat
	UINT Read(void *pData, UINT nCount);

	// Vynucene nacteni presne daneho poctu byty
	void ForceRead(void *pData, UINT nCount);

	// Zapis dat
	void Write(void *pData, UINT nCount);

	// Append na konec
	void Append(void *pData, UINT nCount);

	// Nastavi velikost souboru
	void SetLength(DWORD dwLength);

	// Velikost souboru
	DWORD Length();

	// Cesta k souboru (vcetne jmena)
	CString Path();

	// Pozice v souboru
	inline DWORD Position() { return m_File.GetPosition(); }

private:	
    // Pozice v souboru
    DWORD m_dwLastPosition;

	// Cache
	char m_pCache[MAPPED_FILE_CACHE_SIZE];

	// Pozice cache v ramci souboru
	DWORD m_dwCachePos;

	// Pocet bajtu cache, ktere jsou platne
	DWORD m_dwCacheUsed;

	// Pocet pristupu
	DWORD m_dwAccesses;

	// Soubor ktery se mapuje	
	CFile m_File;

    // Jmeno souboru
    CString m_strFileName;

    // Flagy pro otevirani
    UINT m_nOpenFlags;
};

#endif // !defined(AFX_MAPPEDFILE_H__96DEC042_769D_11D3_A750_0040332A3CC0__INCLUDED_)
