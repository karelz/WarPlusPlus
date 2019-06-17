// DataArchiveFileDirectoryCache.h: interface for the CDataArchiveFileDirectoryCache class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAARCHIVEFILEDIRECTORYCACHE_H__56FB0E04_8AD7_11D3_8964_00C04F513C03__INCLUDED_)
#define AFX_DATAARCHIVEFILEDIRECTORYCACHE_H__56FB0E04_8AD7_11D3_8964_00C04F513C03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDataArchiveFile;
class CDataArchiveFileDirectory;
class CMappedFile;

class CDataArchiveFileDirectoryCache : public CObject  
{
public:
	// Vrati CDataArchiveFileDirectory objekt, odpovidajici adresari v souboru
	// Tento objekt je automaticky pridan do cache, pokud v ni jeste nebyl
	// Pokud cache zabira moc mista, jsou nektere zaznamy zdeleteny
	CDataArchiveFileDirectory GetDirectory(CString strDirName);
	
	// Flushne cely obsah cache na disk a vyprazdni ji
	void Flush();

	// Oreze cache aby zabirala priblizne danou pamet
	// Ma zakazano smazat adresar pDirNoDelete
	void CutCache(DWORD dwSizeLimit, CDataArchiveFileDirectory DirNoDelete);

	// Konstruktor
	CDataArchiveFileDirectoryCache(CDataArchiveFileDirectory *pRootDir, CMappedFile *pMappedFile);

	// Destruktor
	virtual ~CDataArchiveFileDirectoryCache();

private:
	// Root adresar archivu
	CDataArchiveFileDirectory *m_pRootDir;

	// Tabulka 'jmeno adresare' -> 'ukazatel na CDataArchiveFileDirectory'
	CMapStringToPtr *m_pDirTable;

	// Aktualni 'cas'
	DWORD m_dwActualTime;

	// Soubor, ve kterem je archiv ulozeny
	CMappedFile *m_pMappedFile;

#ifdef _DEBUG
private:	
	// Maximalni velikost adresarove cache (pro profiling)
	DWORD m_dwMaxCacheSize;
#endif // _DEBUG
};

#endif // !defined(AFX_DATAARCHIVEFILEDIRECTORYCACHE_H__56FB0E04_8AD7_11D3_8964_00C04F513C03__INCLUDED_)
