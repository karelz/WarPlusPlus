// ArchiveFileCompressed.h: interface for the CArchiveFileCompressed class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARCHIVEFILECOMPRESSED_H__3AF848B3_9625_11D3_BF70_D9A290938907__INCLUDED_)
#define AFX_ARCHIVEFILECOMPRESSED_H__3AF848B3_9625_11D3_BF70_D9A290938907__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ArchiveFileInfo.h"

class CArchiveFile;

#include "../Compression/CompressEngine.h"
#include "CompressDataSinkArchiveFile.h"
#include "CompressDataSourceArchiveFile.h"

class CArchiveFileCompressed : public CArchiveFileInfo  
{
public:

// Construction
// ------------

	// Konstruktor
	CArchiveFileCompressed(CArchiveFile compressedFile);

	// Destruktor
	virtual ~CArchiveFileCompressed();

	// Otevre soubor
	virtual BOOL Open();

	// Zavre soubor
	virtual void Close();

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

	// Vraci TRUE, pokud je soubor otevreny
	BOOL IsOpened() const;

	// Vraci TRUE, pokud je soubor pakovany
	BOOL IsCompressed() const;

	// Vrati TRUE, pokud je soubor otevren v raw modu
	virtual BOOL IsRaw() const;

	// Vrati TRUE, pokud je soubor read only
	BOOL IsReadOnly() const;

	// Nastavi, zdali je soubor read only. Vraci predchozi hodnotu
	BOOL SetReadOnly(BOOL bReadOnly);

	virtual void ChangeDataArchive(CDataArchiveInfo *pArchiveInfo, CString strNewFileName);
private:
	// Rozpakuje soubor a prevede rizeni na nej
	void CreateUncompressed();

	// Zkomprimovany soubor
	CArchiveFile *m_pFileCompressed;

	// Rozbaleny soubor
	CArchiveFile *m_pFileUncompressed;

	// Byl vytvoren rozbaleny soubor?
	BOOL m_bUncompressedCreated;

	// Byl jiz otevren?
	BOOL m_bWasOpened;

	// Mod, ve kterem se ma soubor otevirat
	DWORD m_dwMode;

	// Aktualni druh pristupu
	// 0=nic, 1=read, 2=write
	int m_nAccessMode;

	// Komprimacni stroj
	CCompressEngine m_Engine;

	// Data sink
	CCompressDataSinkArchiveFile m_Sink;

	// Data source
	CCompressDataSourceArchiveFile m_Source; 

	// Aktualni pozice
	DWORD m_dwPos;
};

#endif // !defined(AFX_ARCHIVEFILECOMPRESSED_H__3AF848B3_9625_11D3_BF70_D9A290938907__INCLUDED_)
