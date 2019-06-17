// ArchiveFilePhysical.h: interface for the CArchiveFilePhysical class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARCHIVEFILEPHYSICAL_H__1FFC1CB4_2E06_11D3_890A_00C04F514356__INCLUDED_)
#define AFX_ARCHIVEFILEPHYSICAL_H__1FFC1CB4_2E06_11D3_890A_00C04F514356__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afx.h>

#include "ArchiveFileInfo.h"
#include "DataArchiveException.h"

class CArchiveFilePhysical : public CArchiveFileInfo  
{
public:
// Construction
// ------------

	// Konstruktor
	CArchiveFilePhysical();

	// Vytvori na danem miste. strPhysName urcuje skutecnou pozici na disku, strFileName je pozice v ramci archivu
	CArchiveFilePhysical(CString strFileName, CString strPhysName, DWORD dwOpenFlags);

	// Otevre soubor
	virtual BOOL Open();

	// Zavre soubor
	virtual void Close();

	// Destruktor
	virtual ~CArchiveFilePhysical();

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

	// Vrati TRUE, pokud je soubor pakovany
	virtual BOOL IsCompressed() const;

	// Vrati TRUE, pokud je soubor otevreny
	virtual BOOL IsOpened() const;

	// Vrati TRUE, pokud je soubor otevren v raw modu
	virtual BOOL IsRaw() const { return TRUE; }

	// Vrati TRUE, pokud je soubor read only
	BOOL IsReadOnly() const;

	// Nastavi, zdali je soubor read only. Vraci predchozi hodnotu
	BOOL SetReadOnly(BOOL bReadOnly);

private:
	// Fyzicke umisteni souboru a jmeno
	CString m_strPhysName;
	
	// Je soubor aktualne otevren?
	BOOL m_bWasOpened;

	// Mod, ve kterem se ma soubor otevirat
	DWORD m_dwMode;

	// Vlastni soubor
	CFile m_file;

	// Read only priznak (pokud bylo nastaveno SetReadOnly, automaticky se pri otevirani maskuje modeWrite a modeReadWrite)
	BOOL m_bReadOnly;
};

#endif // !defined(AFX_ARCHIVEFILEPHYSICAL_H__1FFC1CB4_2E06_11D3_890A_00C04F514356__INCLUDED_)
