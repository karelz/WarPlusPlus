// DataArchiveFileCommon.h: interface for the CDataArchiveFileCommon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAARCHIVEFILECOMMON_H__7E6D1115_B6DF_11D4_813F_0000B48431EC__INCLUDED_)
#define AFX_DATAARCHIVEFILECOMMON_H__7E6D1115_B6DF_11D4_813F_0000B48431EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

class CDataArchiveFile;
class CMappedFileCommon;
class CDataArchiveFileMainHeader;

class CDataArchiveFileCommon  
{
friend class CDataArchiveFile; // Jsme soucasti teto tridy

public:
	// Konstruktor
	CDataArchiveFileCommon();

	// Destruktor
	virtual ~CDataArchiveFileCommon();

public:	
	// Prida odkaz
	void AddRef(CDataArchiveFile *pFile);

	// Odebere odkaz
	void Release(CDataArchiveFile *pFile);

// Data
private:
	// Hlavicka archivu
	CDataArchiveFileMainHeader *m_pMainHeader;

	// Byl dan pozadavek na rebuild
	BOOL m_bRebuildRequest;

	// Provadi se rebuild?
	BOOL m_bRebuildRunning;

	// Pocet odkazu
	int m_nRefCount;

	// Mapa ThreadID -> CDataArchiveFile
	CMap<DWORD,DWORD,CDataArchiveFile*,CDataArchiveFile*> m_ThreadMap;
};

#endif // !defined(AFX_DATAARCHIVEFILECOMMON_H__7E6D1115_B6DF_11D4_813F_0000B48431EC__INCLUDED_)
