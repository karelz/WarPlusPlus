// MappedFileCommon.h: interface for the CMappedFileCommon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAPPEDFILECOMMON_H__7E6D1114_B6DF_11D4_813F_0000B48431EC__INCLUDED_)
#define AFX_MAPPEDFILECOMMON_H__7E6D1114_B6DF_11D4_813F_0000B48431EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxmt.h>

class CMappedFile;

class CMappedFileCommon  
{
	// Rozsirujici trida, jejiz jsme nedilnou soucasti
	friend class CMappedFile;

// Konstrukce, destrukce
public:
	// Konstruktor
	CMappedFileCommon();

	// Destruktor
	virtual ~CMappedFileCommon();

// Odkazy
public:
	// Pridani odkazu z CMappedFile
	void AddRef();

	// Odebrani odkazu z CMappedFile
	void Release();

// Pomocne funkce
private:
	// Zamkne soubor
	void Lock() { VERIFY(m_Mutex.Lock()); }

	// Odemkne soubor
	void Unlock() { VERIFY(m_Mutex.Unlock()); }

	// Nacte data z dane pozice, se zamykanim
	UINT SeekRead(DWORD dwPos, void *pData, UINT nCount);

	// Nacte vsechna data z dane pozice, se zamykanim
	void SeekForceRead(DWORD dwPos, void *pData, UINT nCount);

	// Zapise data na danou pozici, se zamykani
	void SeekWrite(DWORD dwPos, void *pData, UINT nCount);

// Data
private:
	// Pocet odkazu z CMappedFile
	int m_nRefCount;

	// Soubor ktery se mapuje	
	CFile m_File;

    // Jmeno souboru
    CString m_strFileName;

    // Flagy pro otevirani
    UINT m_nOpenFlags;

	// Zamek na soubor
	CMutex m_Mutex;
};

#endif // !defined(AFX_MAPPEDFILECOMMON_H__7E6D1114_B6DF_11D4_813F_0000B48431EC__INCLUDED_)
