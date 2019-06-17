// ArchiveFileInFile.h: interface for the CArchiveFileInFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARCHIVEFILEINFILE_H__4A7AD8E6_928A_11D3_BF67_AE14368AF507__INCLUDED_)
#define AFX_ARCHIVEFILEINFILE_H__4A7AD8E6_928A_11D3_BF67_AE14368AF507__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ArchiveFileInfo.h"

class CDataArchiveFileDataList;
class CDataArchiveFileDirectoryItem;

class CArchiveFileInFile : public CArchiveFileInfo  
{
public:
// Construction
// ------------

	// Konstruktor
	CArchiveFileInFile();

	// Vytvori na danem miste. strFileName je pozice v ramci archivu
	CArchiveFileInFile(CString strFileName, CDataArchiveFileDataList *pDataList, DWORD dwOpenFlags);

	// Otevre soubor
	virtual BOOL Open();

	// Zavre soubor
	virtual void Close();

	// Destruktor
	virtual ~CArchiveFileInFile();

// Input/Output
// ------------

	// Nacteni bloku dat
	virtual UINT Read(void* lpBuf, UINT nCount);

	// Zapis bloku dat
	virtual void Write(const void* lpBuf, UINT nCount);

	// Flushne soubor
	virtual void Flush();

// Position
// --------

	// Seek
	virtual LONG Seek(LONG lOff, UINT nFrom);

	// Vraci delku souboru
	virtual DWORD GetLength() const;

	// Nastavuje delku souboru
	virtual void SetLength(DWORD dwNewLen);

// Locking
// -------

	// Zamyka kus souboru
	virtual void LockRange(DWORD dwPos, DWORD dwCount);

	// Odemyka kus souboru
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);

// Status
// ------

	// Vraci aktualni pozici v souboru
	virtual DWORD GetPosition() const;

	// Naplni status souboru
	virtual BOOL GetStatus(CFileStatus& rStatus) const;

	// Vrati TRUE, pokud je soubor otevreny
	virtual BOOL IsOpened() const;

	// Vrati TRUE, pokud je soubor pakovany
	virtual BOOL IsCompressed() const;

	// Vrati TRUE, pokud je soubor otevren jako raw
	virtual BOOL IsRaw() const;

	// Nastavi soubor na read-only
	virtual BOOL SetReadOnly(BOOL bReadOnly=TRUE);

	// Vrati TRUE, pokud je soubor read only
	virtual BOOL IsReadOnly() const;

	// Zmena archivu
	virtual void ChangeDataArchive(CDataArchiveInfo *pArchiveInfo, CString strNewFileName);

private:
	// Byl jiz otevren?
	BOOL m_bWasOpened;

	// Mod, ve kterem se ma soubor otevirat
	DWORD m_dwMode;

	// List dat souboru
	CDataArchiveFileDataList *m_pDataList;
};

#endif // !defined(AFX_ARCHIVEFILEINFILE_H__4A7AD8E6_928A_11D3_BF67_AE14368AF507__INCLUDED_)
