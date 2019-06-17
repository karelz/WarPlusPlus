// DataArchiveFileMainHeader.h: interface for the CDataArchiveFileMainHeader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAARCHIVEFILEMAINHEADER_H__6BB46C22_7694_11D3_A750_0040332A3CC0__INCLUDED_)
#define AFX_DATAARCHIVEFILEMAINHEADER_H__6BB46C22_7694_11D3_A750_0040332A3CC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afx.h>
#include "MappedFile.h"
#include "Definitions.h"

#pragma pack(1)	// Zarovnani dat struktury na jeden bajt

// Data pro hlavicku souboru
struct SDataArchiveFileMainHeader {
	char strID[3]; // Tri pismena pro identifikaci, ze se jedna o archiv
	DWORD dwVersion; // Verze archivu
	DWORD dwBytesUsed; // Pocet bytu, ktere jsou opravdu vyuzity
	
	DWORD dwRootOffset; // Offset korenoveho adresare
};

#pragma pack() // Konec zarovnani dat struktury

class CDataArchiveFileMainHeader : public CObject
{
public:
	// Konstruktor
	CDataArchiveFileMainHeader();

	// Destruktor
	virtual ~CDataArchiveFileMainHeader();

	// Inicializace
	void Init();

	// Nacteni ze souboru
	void Read(CMappedFile &File);

	// Zapis do souboru
	void Write(CMappedFile &File);

	// Zjisteni poctu pouzitych bajtu
	DWORD BytesUsed() { return m_HeaderData.dwBytesUsed; }

	// Nastaveni poctu pouzitych bajtu
	void BytesUsed(DWORD dwBytesUsed) { m_HeaderData.dwBytesUsed=dwBytesUsed; }

	// Zjisteni offsetu korenoveho adresare
	DWORD RootOffset() { return m_HeaderData.dwRootOffset; }

	// Nastaveni offsetu korenoveho adresare
	void RootOffset(DWORD dwRootOffset) { m_HeaderData.dwRootOffset=dwRootOffset; }

private:
	// Data o hlavicce
	SDataArchiveFileMainHeader m_HeaderData;
};

#endif // !defined(AFX_DATAARCHIVEFILEMAINHEADER_H__6BB46C22_7694_11D3_A750_0040332A3CC0__INCLUDED_)
