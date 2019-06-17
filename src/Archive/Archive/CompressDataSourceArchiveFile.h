// CompressDataSourceArchiveFile.h: interface for the CCompressDataSourceArchiveFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPRESSDATASOURCEARCHIVEFILE_H__C3F4E3A7_962B_11D3_BF70_D9A290938907__INCLUDED_)
#define AFX_COMPRESSDATASOURCEARCHIVEFILE_H__C3F4E3A7_962B_11D3_BF70_D9A290938907__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Compression/CompressDataSource.h"

#include "ArchiveFile.h"

class CCompressDataSourceArchiveFile : public CCompressDataSource  
{
public:
	// Konstruktor
	CCompressDataSourceArchiveFile();

	// Destruktor
	virtual ~CCompressDataSourceArchiveFile();

	// Otevre se na zadanem souboru
	void Open(CArchiveFile file);

	// Zavre zadany soubor
	void Close();

	// Vyprodukuje blok dat a vrati jeho velikost
	virtual ECompressData GiveData(void *&lpBuf, DWORD &dwSize);

	// Vrati cestu k souboru, odkud se cte nebo <unopened source>, kdyz se necte
	virtual CString GetID();

	DWORD GetUncompressedSize() { return m_dwUncompressedSize; }

private:
	// Byl soubor otevren?
	BOOL m_bOpened;

	// Soubor, do ktereho zapisujeme
	CArchiveFile m_File;

	// Buffer na cteni
	char *m_pBuffer;

	// Velikost bufferu na cteni
	DWORD m_dwBufSize;

	// Velikost souboru pred kompresi
	DWORD m_dwUncompressedSize;

};

#endif // !defined(AFX_COMPRESSDATASOURCEARCHIVEFILE_H__C3F4E3A7_962B_11D3_BF70_D9A290938907__INCLUDED_)
