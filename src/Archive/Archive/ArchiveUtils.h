// ArchiveUtils.h: interface for the CArchiveUtils class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARCHIVEUTILS_H__49583A24_9782_11D3_BF72_FC0837D98807__INCLUDED_)
#define AFX_ARCHIVEUTILS_H__49583A24_9782_11D3_BF72_FC0837D98807__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CArchiveUtils  
{
public:
	CArchiveUtils();
	virtual ~CArchiveUtils();

	// Pomocna funkce vracejici true, pokud soubor daneho jmena existuje jako pakovany
	static BOOL IsCompressed(CString strFileName);

	// Pomocna funkce vracejici true, pokud dane jmeno konci na '.wz'
	static BOOL HasExtension(CString strFileName);

	// Pomocna funkce - vraci true, pokud soubor daneho jmena existuje a je to soubor
	static BOOL IsFile(LPCTSTR lpcszFileName);
	
	// Pomocna funkce - vraci true, pokud soubor daneho jmena existuje a je to adresar
	static BOOL IsDirectory(LPCTSTR lpcszDirectoryName);

	// Pomocna funkce - pokud soubor neexistuje, ale s priponou '.wz' uz ano, vrati jeho jmeno rozsirene o tuto priponu
	static CString AddExtension(LPCTSTR lpcszFileName);

	// Pomocna funkce - pokud retezec ma priponu '.wz', je mu uriznuta
	static void ClipExtension(CString &strName);

	// Pomocna funkce - slepi dve cesty za sebe
	static CString ConcatenatePaths(CString strPath1, CString strPath2);

	// Pomocna funkce - vraci true, pokud jsou dane flagy writeable
	static BOOL FlagWrite(DWORD dwFlag) { return ((dwFlag & CFile::modeWrite) || (dwFlag & CFile::modeReadWrite)); }
};

#endif // !defined(AFX_ARCHIVEUTILS_H__49583A24_9782_11D3_BF72_FC0837D98807__INCLUDED_)
