// CompressDataSinkFile.h: interface for the CCompressDataSinkFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPRESSDATASINKFILE_H__904F4E13_95A7_11D3_BF6F_E914A8C58D07__INCLUDED_)
#define AFX_COMPRESSDATASINKFILE_H__904F4E13_95A7_11D3_BF6F_E914A8C58D07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CompressDataSink.h"

class CCompressDataSinkFile : public CCompressDataSink  
{
public:
	// Konstruktor
	CCompressDataSinkFile();

	// Destruktor
	virtual ~CCompressDataSinkFile();

	// Vrati cestu k souboru, kam sink pise nebo <unopened sink>, kdyz nikam nepise
	virtual CString GetID();

	// Otevre se na zadanem souboru
	void Open(LPCTSTR lpcszFileName, DWORD dwUncompressedSize=0);

	// Zavre zadany soubor
	void Close();

	// Ukladani dat
	virtual ECompressData StoreData(void *lpBuf, DWORD dwSize);

private:
	// Byl soubor otevren?
	BOOL m_bOpened;

	// Soubor, do ktereho zapisujeme
	CFile m_File;
};

#endif // !defined(AFX_COMPRESSDATASINKFILE_H__904F4E13_95A7_11D3_BF6F_E914A8C58D07__INCLUDED_)
