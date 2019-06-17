// CompressDataSinkArchiveFile.h: interface for the CCompressDataSinkArchiveFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPRESSDATASINKARCHIVEFILE_H__C3F4E3A6_962B_11D3_BF70_D9A290938907__INCLUDED_)
#define AFX_COMPRESSDATASINKARCHIVEFILE_H__C3F4E3A6_962B_11D3_BF70_D9A290938907__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ArchiveFile.h"
#include "../Compression/CompressDataSink.h"

class CCompressDataSinkArchiveFile : public CCompressDataSink
{
public:
	// Konstruktor
	CCompressDataSinkArchiveFile();

	// Destruktor
	virtual ~CCompressDataSinkArchiveFile();

	// Vrati cestu k souboru, kam sink pise nebo <unopened sink>, kdyz nikam nepise
	virtual CString GetID();

	// Otevre se na zadanem souboru
	void Open(CArchiveFile file, DWORD dwUncompressedSize=0);

	// Zavre zadany soubor
	void Close();

	// Ukladani dat
	virtual ECompressData StoreData(void *lpBuf, DWORD dwSize);

private:
	// Byl soubor otevren?
	BOOL m_bOpened;

	// Soubor, do ktereho zapisujeme
	CArchiveFile m_File;
};

#endif // !defined(AFX_COMPRESSDATASINKARCHIVEFILE_H__C3F4E3A6_962B_11D3_BF70_D9A290938907__INCLUDED_)
