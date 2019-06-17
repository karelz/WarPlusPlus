// CompressDataSourceFile.h: interface for the CCompressDataSourceFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPRESSDATASOURCEFILE_H__780E06DB_95DC_11D3_BF6F_E914A8C58D07__INCLUDED_)
#define AFX_COMPRESSDATASOURCEFILE_H__780E06DB_95DC_11D3_BF6F_E914A8C58D07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CompressDataSource.h"

class CCompressDataSourceFile : public CCompressDataSource
{
public:
	// Konstruktor
	CCompressDataSourceFile();

	// Destruktor
	virtual ~CCompressDataSourceFile();

	// Otevre se na zadanem souboru
	void Open(LPCTSTR lpcszFileName);

	// Zavre zadany soubor
	void Close();

	// Vyprodukuje blok dat a vrati jeho velikost
	virtual ECompressData GiveData(void *&lpBuf, DWORD &dwSize);

	// Vrati cestu k souboru, odkud se cte nebo <unopened source>, kdyz se necte
	virtual CString GetID();

	// Vrati velikost nekomprimovaneho souboru
	DWORD GetUncompressedSize() { return m_dwUncompressedSize; }

private:
	// Byl soubor otevren?
	BOOL m_bOpened;

	// Soubor, do ktereho zapisujeme
	CFile m_File;

	// Buffer na cteni
	char *m_pBuffer;

	// Velikost bufferu na cteni
	DWORD m_dwBufSize;

	// Velikost souboru pred kompresi
	DWORD m_dwUncompressedSize;
};

#endif // !defined(AFX_COMPRESSDATASOURCEFILE_H__780E06DB_95DC_11D3_BF6F_E914A8C58D07__INCLUDED_)
