// DataArchiveFileDirectoryInfo.cpp: implementation of the CDataArchiveFileDirectoryInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataArchiveFile.h"
#include "DataArchiveFileDirectoryInfo.h"
#include "DataArchiveFileMainHeader.h"
#include "DataArchiveException.h"
#include "DataArchiveFileDirectoryItem.h"
#include "DataArchiveFileDirectory.h"
#include "ArchiveFileCompressed.h"
#include "ArchiveFileInFile.h"
#include "Definitions.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDataArchiveFileDirectoryInfo::CDataArchiveFileDirectoryInfo()
{
	m_dwHeaderOffset=0; // Pokud je offset==0, znamena to, ze jeste nejsme otevreni
	m_nPointerCount=0;
	m_dwLastAccessTime=0;
	
	m_DirHeader.dwDataOffset=0;
	m_DirHeader.nFiles=0;
	m_bModified=TRUE;
}

CDataArchiveFileDirectoryInfo::~CDataArchiveFileDirectoryInfo()
{
	ASSERT(m_nPointerCount==0);

	if(m_dwHeaderOffset==0) return; /* Nejsme jeste otevreni */

	CString key;
	void *pValue;
	CDataArchiveFileDirectoryItem *pItem;

	Flush();

	for(POSITION pos=m_FileTable.GetStartPosition(); pos!=NULL;) {
		m_FileTable.GetNextAssoc(pos, key, pValue);
		
		pItem=(CDataArchiveFileDirectoryItem *)pValue;

		delete pItem;
	}

	// Vyprazdnime tabulku
	m_FileTable.RemoveAll();
}

void CDataArchiveFileDirectoryInfo::Open(CMappedFile &File, DWORD dwOffset) {	
	m_bModified=FALSE;

	// Nacteme si hlavicku
	File.Seek(dwOffset, CFile::begin);
	File.Read(&m_DirHeader, sizeof(m_DirHeader));
	
	// Test na invarianty
	// Kazdy adresar obsahuje alespon 2 soubory
	// Offset struktury nesmi byt v ramci headru archivu..
	// .. a samozrejme nesmi byt mimo soubor
	if(m_DirHeader.nFiles<2 ||
	   m_DirHeader.dwDataOffset<sizeof(SDataArchiveFileMainHeader) ||
	   m_DirHeader.dwDataOffset>File.Length()-sizeof(SDataArchiveFileDirectoryHeader)) {
		// archive corrupt
		throw new CDataArchiveException(File.Path(), CDataArchiveException::EDataArchiveExceptionTypes::archiveCorrupt,
												CDataArchiveException::EDataArchiveExceptionLocations::archiveItself);
	}

	// Nyni si nacteme uplny seznam souboru, jejich priznaku a offsetu jejich dat v souboru

	// Nejprve si otevreme data pro list souboru
	m_FileList.Open(File, m_DirHeader.dwDataOffset);

	// Nyni nacteme data o retezcich
	DWORD dwLength=m_FileList.GetLength();
	char *pDataBuffer, *pAct;
	pDataBuffer=new char[dwLength+1];
	pAct=pDataBuffer;

	try {
		// Nejdriv zkusime nacist vsechna data do bufferu
		m_FileList.Read(pDataBuffer, dwLength);

		// Pak dej na posledni misto bufferu 0 - zarazka
		pDataBuffer[dwLength]=0;
	}
	catch(CException *pError) {
		delete [] pDataBuffer; // Pri vyjimce smazeme buffer a posleme ji vys
		throw pError;
	}

	// Zpracujeme obsah bufferu
	for(int i=0; i<m_DirHeader.nFiles; i++) {
		CString strName;
		DWORD dwOffset, dwFlags;

		strName=pAct; // Priradime jmeno

		// Jmeno nesmi byt prazdne a nesmi byt delsi nez cely blok
		if(strName.GetLength()==0 || pAct+strName.GetLength()==pDataBuffer+dwLength) {
			// .. jinak je neco s archivem v neporadku
			delete [] pDataBuffer;
			throw new CDataArchiveException(File.Path(), CDataArchiveException::EDataArchiveExceptionTypes::archiveCorrupt,
				CDataArchiveException::EDataArchiveExceptionLocations::archiveItself);
		}
		pAct+=strName.GetLength()+1; // Posuneme se o jmeno souboru

		// Jestlize jsme na konci a uz nejde nacist offset a flagy...
		if(pAct+sizeof(dwOffset)+sizeof(dwFlags)>pDataBuffer+dwLength) {
			// .. archive corrupt
			delete [] pDataBuffer;
			throw new CDataArchiveException(File.Path(), CDataArchiveException::EDataArchiveExceptionTypes::archiveCorrupt,
				CDataArchiveException::EDataArchiveExceptionLocations::archiveItself);
		}

		// Nejdriv jsou ulozeny flagy
		dwFlags=*((DWORD *)pAct); pAct+=sizeof(dwFlags);

		// Pak offset
		dwOffset=*((DWORD *)pAct); pAct+=sizeof(dwOffset);

		// Pridame hodnotu do tabulky
		SetAt(strName, new CDataArchiveFileDirectoryItem(strName, dwFlags, dwOffset));
	}

	// Smazeme buffer
	delete [] pDataBuffer;

	// Uspesne otevreni
	m_dwHeaderOffset=dwOffset;
}

void CDataArchiveFileDirectoryInfo::CreateRoot(CMappedFile &File, DWORD dwOffset) {
	m_bModified=TRUE;
	
	// Naplnime strukturu o adresari
	m_DirHeader.nFiles=2; // Dva soubory - . a .. (.. jsou totez jako .)
	m_DirHeader.dwDataOffset=dwOffset+sizeof(m_DirHeader); // Data zacinaji za touto hlavickou

	// Nastavime se na zadanou pozici
	File.Seek(dwOffset, CFile::begin);

	// Zapiseme hlavicku
	File.Write(&m_DirHeader, sizeof(m_DirHeader));

	// Nyni je treba vytvorit tabulku souboru a zapsat ji
	char *pDataBuffer, *pAct;
	pDataBuffer=new char[DATA_ARCHIVE_FILE_DIRECTORY_ROOT_INITIAL_DATA_SIZE];
	memset(pDataBuffer, 0xFF, DATA_ARCHIVE_FILE_DIRECTORY_ROOT_INITIAL_DATA_SIZE);
	pAct=pDataBuffer;
	
	// Nyni do bufferu dame spravna data
	
	// 1) adresar '.'

	// Jmeno
	*pAct='.'; pAct++; *pAct=0; pAct++; 
	// Je to adresar
	*((DWORD *)pAct)=CDataArchiveFile::directoryFlag; pAct+=sizeof(DWORD); 
	// A data o nem jsou na pozici dwOffset (je to on sam)	
	*((DWORD *)pAct)=dwOffset; pAct+=sizeof(DWORD);

	// 2) adresar '..' je stejny jako '.'

	*pAct='.'; pAct++; *pAct='.'; pAct++; *pAct=0; pAct++; 
	*((DWORD *)pAct)=CDataArchiveFile::directoryFlag; pAct+=sizeof(DWORD); 
	*((DWORD *)pAct)=dwOffset; pAct+=sizeof(DWORD);

	try {
		// Vytvorime si FileList
		m_FileList.Create(File, DATA_ARCHIVE_FILE_DIRECTORY_ROOT_INITIAL_DATA_SIZE);

		m_FileList.Seek(0, CFile::begin);
		// A zapiseme do nej data
		m_FileList.Write(pDataBuffer, DATA_ARCHIVE_FILE_DIRECTORY_ROOT_INITIAL_DATA_SIZE);
	} catch (...) { // Jestli se to nepovedlo, vycistime naalokovany buffer
		delete [] pDataBuffer;
		throw;
	}
	delete [] pDataBuffer;
	
	// Ted zbyva vytvorit tabulku
	SetAt(".", new CDataArchiveFileDirectoryItem(".", CDataArchiveFile::directoryFlag, dwOffset));
	SetAt("..", new CDataArchiveFileDirectoryItem("..", CDataArchiveFile::directoryFlag, dwOffset));

	// A je to!
	// Uspesne vytvoreni
	m_dwHeaderOffset=dwOffset;
}

void CDataArchiveFileDirectoryInfo::Close() {
	// Pri zavirani flushnout data
	Flush();
}

void CDataArchiveFileDirectoryInfo::Flush() {
	// Flushnuti zmen
	
	// Flushuje se jen tehdy, doslo-li od posledniho flushu ke zmene
	if(!m_bModified) return;

	ASSERT(m_dwHeaderOffset);

	char *pDataBuffer, *pAct;
	int bufSize;
	POSITION pos;
	void *pValue;
	CString key;
	CDataArchiveFileDirectoryItem *pItem;

	// Nejprve zjistime presnou velikost, kterou potrebuji nase data
	bufSize=GetStorageSize();

	// Tuto velikost zaokrouhlime nahoru podle velikosti bloku
	bufSize/=DATA_ARCHIVE_FILE_DIRECTORY_DATA_GRANULARITY;
	bufSize=(bufSize+1)*DATA_ARCHIVE_FILE_DIRECTORY_DATA_GRANULARITY;

	// Ted muzeme naalokovat buffer a naplnit ho daty
	pDataBuffer=new char[bufSize];
	pAct=pDataBuffer;

	// Pracujeme v try-catch, abychom mohli dealokovat pDataBuffer pri zavolani vyjimky
	try {
		// VyFFujeme buffer, at je to v souboru prehledne...
		memset(pDataBuffer, 0xFF, bufSize);

		for(pos=m_FileTable.GetStartPosition(); pos!=NULL;) {
			m_FileTable.GetNextAssoc(pos, key, pValue);

			pItem=(CDataArchiveFileDirectoryItem *)pValue;

			// Ulozime do bufferu
			pAct=pItem->Store(pAct);
		}

		// Nyni je v bufferu veskera informace o adresarich -> sup s ni do souboru
		m_FileList.Seek(0, CFile::begin);
		m_FileList.Write(pDataBuffer, bufSize);
		// A opravdu data flushneme, at je to bezpecnejsi...
		m_FileList.Flush();
	} catch(...) {
		delete pDataBuffer;
		throw;
	}
	delete pDataBuffer;

	// A nyni jeste flushnout hlavicku
	m_FileList.GetFile()->Seek(m_dwHeaderOffset, CFile::begin);
	m_FileList.GetFile()->Write(&m_DirHeader, sizeof(m_DirHeader));

	// Modifikacni priznak shodit...
	m_bModified=FALSE;
}

CDataArchiveFileDirectoryItem *CDataArchiveFileDirectoryInfo::FileInfo(CString strFileName) {
	ASSERT(m_dwHeaderOffset);
	
	strFileName.MakeLower();
	CDataArchiveFileDirectoryItem *pItem;
	ASSERT(m_dwHeaderOffset>0);
	if(!m_FileTable.Lookup(strFileName, (void *&)pItem)) { // Nenalezeno
		return (CDataArchiveFileDirectoryItem*)NULL;
	} else {
		return pItem;
	}
}

CArchiveFile CDataArchiveFileDirectoryInfo::CreateFile(CString strFileName, CString strFullName, CDataArchiveInfo *pDataArchiveInfo, DWORD dwFlags, DWORD dwInitialSize, BOOL bGonnaFill /* = FALSE */) {
	ASSERT(m_dwHeaderOffset);

	// Predpokladame, ze soubor udelat lze (to je na volajicim)
	CDataArchiveFileDirectoryItem *pItem;
	CDataArchiveFileDataList *pList;
	CMappedFile *pFile;
	DWORD listOffset;

	pItem=FileInfo(strFileName);

	pFile=m_FileList.GetFile();
	pFile->Seek(0, CFile::end);

	BOOL newItem;
	newItem=FALSE; // Nastavime na true, pokud VYTVARIME novou polozku
	BOOL create;
	create = dwFlags & CArchiveFile::modeCreate;

	DWORD cFlag; // Flag pro komprimovani
	cFlag=dwFlags & CArchiveFile::modeUncompressed?0:CDataArchiveFile::EDataArchiveFileFlags::compressedFlag;

	pList=new CDataArchiveFileDataList();
	
	try {

		if(!pItem) { // Jeste takovy soubor NENI
			// Jestlize po nas neni pozadovano vytvoreni, je to chyba
			if(!create) {
				throw new CDataArchiveException(strFileName, CDataArchiveException::fileNotFound, CDataArchiveException::archiveItself);
			}

			// Vytvorime item
			newItem=TRUE;
			pItem=new CDataArchiveFileDirectoryItem(strFileName, CDataArchiveFile::fileFlag | cFlag, pFile->Position());
			try {
				// Pridame do tabulky
				SetAt(strFileName, pItem);

				// Vytvorime list dat souboru, v tomto jedinem pripade se pouziva bGonnaFill optimalizace
				listOffset=pList->Create(*pFile, dwInitialSize, bGonnaFill);
			} catch(...) {
				delete pList;
				delete pItem;
				throw;
			}
		} else if(pItem->IsDeleted()) { // Neco takoveho jiz je, musi to byt smazane
			// Jestlize po nas neni pozadovano vytvoreni, je to chyba
			if(!create) {
				throw new CDataArchiveException(strFileName, CDataArchiveException::fileNotFound, CDataArchiveException::archiveItself);
			}

			if(pItem->IsDirectory()) { // Je to smazany adresar
				// Data adresare se proste ZAZDI

				// Vytvorime list dat souboru
				listOffset=pList->Create(*pFile, dwInitialSize);

			} else { // Je to smazany soubor.. muzeme pouzit jeho bloky dat
				// Nacteme list dat
				pList->Open(*pFile, pItem->GetOffset());
				listOffset=pItem->GetOffset();
				
				// Nastavime jeho delku na 0
				pList->Empty();
			}
			// Nastavime flagy na soubor
			pItem->SetFlags(CDataArchiveFile::fileFlag | cFlag);
			// Doslo k modifikaci
			m_bModified=TRUE;
		} else if(!pItem->IsDirectory()) { // Takovy soubor jiz existuje
			// Nacteme list dat
			pList->Open(*pFile, pItem->GetOffset());
			listOffset=pItem->GetOffset();

			// Jestlize po nas je pozadovano vytvoreni, je treba ho vynulovat			
			// .. pokud to ovsem user nezakazal
			if(create && !(dwFlags & CArchiveFile::modeNoTruncate)) {
				pList->Empty();
			}
			
		} else {
			TRACE("Volajici RUCI za to, ze soubor udelat lze!\n");
			ASSERT(FALSE);
		}
		
		// Pribylo nam itemu?
		if(newItem) {
			m_DirHeader.nFiles++;
			m_bModified=TRUE;
		}		

		// A nyni si pItem dame do sve tabulky
		SetAt(strFileName, pItem);

		// A flushneme se
		if(create) {
		  // Protoze jsme vytvareli novou polozku, je treba flushnout informace o adresari
//			  Flush();
		}
		
		if(pItem->IsCompressed()) { // Je pakovany, nulujeme uncompressed flag
			dwFlags&= ~CArchiveFile::modeUncompressed;
		} else {
			dwFlags|=CArchiveFile::modeUncompressed; // Jinak nastavujeme
		}
		CArchiveFileInFile *pFileInFile=new CArchiveFileInFile(strFullName, pList, dwFlags);
		try {
			if(!pItem->IsCompressed() || dwFlags & CArchiveFile::modeRaw)  // nepakovany soubor
			{ 
				CArchiveFile ArchiveFile(pFileInFile, pDataArchiveInfo);
				return ArchiveFile;
			} else {
				CArchiveFile wrap(pFileInFile, pDataArchiveInfo); // Zabalime fileInFile;
				CArchiveFileCompressed *pcafc=new CArchiveFileCompressed(wrap); // Vytvorime pakovany soubor
				CArchiveFile retFile(pcafc, pDataArchiveInfo); // Zabalime ho
				return retFile;
			}
		} catch(...) {
			delete pFileInFile;
			throw;
		}
	} catch(...) {
		delete pList;
		throw;
	}
}

void CDataArchiveFileDirectoryInfo::CreateDirectory(CString strDirName) {
	ASSERT(m_dwHeaderOffset);

	m_bModified=TRUE;

	// Predpokladame, ze adresar udelat lze (to je na volajicim)
	CDataArchiveFileDirectoryItem *pItem;
	CDataArchiveFileDataList list;
	SDataArchiveFileDirectoryHeader header;
	CMappedFile *pFile;
	DWORD listOffset;

	pItem=FileInfo(strDirName);

	pFile=m_FileList.GetFile();
	pFile->Seek(0, CFile::end);

	BOOL newItem;
	BOOL createList;
	newItem=FALSE; // Nastavime na true, pokud VYTVARIME novou polozku
	createList=FALSE; // Nastavime na true, pokud VYTVARIME novy list dat

	if(!pItem) { // Jeste takovy adresar NENI
		// Vytvorime item
		newItem=TRUE;
		createList=TRUE;
		pItem=new CDataArchiveFileDirectoryItem(strDirName, CDataArchiveFile::directoryFlag, pFile->Position());
		try {
			// Pridame do tabulky
			SetAt(strDirName, pItem);

			// Novy adresar ma 2 soubory
			header.nFiles=2;
			// A jeho data zacinaji na hned za hlavickou
			header.dwDataOffset=pItem->GetOffset()+sizeof(header);
			// Zapiseme hlavicku do souboru
			pFile->Write(&header, sizeof(header));
			pFile->SetLength(header.dwDataOffset);

			// Vytvorime seznam adresaru
			listOffset=list.Create(*pFile, DATA_ARCHIVE_FILE_DIRECTORY_INITIAL_DATA_SIZE);
			ASSERT(listOffset==header.dwDataOffset);
		} catch(...) {
			delete pItem;
			throw;
		}
	} else if(pItem->IsDeleted()) { // Neco takoveho jiz je, musi to byt smazane
		if(pItem->IsDirectory()) { // Je to smazany adresar
			// Nacteme info o jeho headru
			pFile->Seek(pItem->GetOffset(), CFile::begin);
			pFile->ForceRead(&header, sizeof(header));
			// Otevreme seznam souboru
			list.Open(*pFile, header.dwDataOffset);
			listOffset=header.dwDataOffset;
		} else { // Je to smazane neco jineho - stara data jsou nepouzitelna
			// Novy adresar ma 2 soubory
			header.nFiles=2;
			// A jeho data zacinaji na hned za hlavickou
			header.dwDataOffset=pItem->GetOffset()+sizeof(header);
			// Zapiseme hlavicku do souboru
			pFile->Write(&header, sizeof(header));

			// Vytvorime seznam adresaru
			listOffset=list.Create(*pFile, DATA_ARCHIVE_FILE_DIRECTORY_INITIAL_DATA_SIZE);
			createList=TRUE;
			ASSERT(listOffset==header.dwDataOffset);
		}
		// Nastavime flagy na adresar
		pItem->SetFlags(CDataArchiveFile::directoryFlag);
	} else {
		TRACE("Volajici RUCI za to, ze adresar udelat lze!\n");
		ASSERT(FALSE);
	}
	
	// Jestlize se tabulka musi vytvaret (neslo o obnovu smazaneho adresare)
	if(createList) {
		// Nyni je treba vytvorit tabulku souboru a zapsat ji
		char *pDataBuffer, *pAct;
		pDataBuffer=new char[DATA_ARCHIVE_FILE_DIRECTORY_INITIAL_DATA_SIZE];
		memset(pDataBuffer, 0xFF, DATA_ARCHIVE_FILE_DIRECTORY_INITIAL_DATA_SIZE);
		pAct=pDataBuffer;
		
		// Nyni do bufferu dame spravna data
		
		// 1) adresar '.'

		// Jmeno
		*pAct='.'; pAct++; *pAct=0; pAct++; 
		// Je to adresar
		*((DWORD *)pAct)=CDataArchiveFile::directoryFlag; pAct+=sizeof(DWORD); 
		// A data o nem jsou na pozici dwOffset (je to on sam)	
		*((DWORD *)pAct)=pItem->GetOffset(); pAct+=sizeof(DWORD);

		// 2) adresar '..' je stejny jako '.', pouze ukazuje o jeden vys

		*pAct='.'; pAct++; *pAct='.'; pAct++; *pAct=0; pAct++; 
		*((DWORD *)pAct)=CDataArchiveFile::directoryFlag; pAct+=sizeof(DWORD); 
		*((DWORD *)pAct)=m_dwHeaderOffset; pAct+=sizeof(DWORD);

		try {
			list.Seek(0, CFile::begin);
			// A zapiseme do nej data
			list.Write(pDataBuffer, DATA_ARCHIVE_FILE_DIRECTORY_INITIAL_DATA_SIZE);
		} catch (...) { // Jestli se to nepovedlo, vycistime naalokovany buffer
			delete [] pDataBuffer;
			throw;
		}
		delete [] pDataBuffer;
	}

	// Pribylo nam itemu?
	if(newItem) {
		m_DirHeader.nFiles++;
	}

	// A nyni si pItem dame do sve tabulky
	SetAt(strDirName, pItem);

	// A flushneme se
	// Flush();
}

void CDataArchiveFileDirectoryInfo::AppendFile(CString strFileName) {
	m_bModified=TRUE;	
	ASSERT(m_dwHeaderOffset);
}

void CDataArchiveFileDirectoryInfo::RemoveFile(CString strFileName) {
	m_bModified=TRUE;
	ASSERT(m_dwHeaderOffset);

	// Predpokladame, ze soubor daneho jmena EXISTUJE
	CDataArchiveFileDirectoryItem *pItem;
	pItem=FileInfo(strFileName);
	
	ASSERT(pItem);
	ASSERT(!pItem->IsDirectory());
	ASSERT(!pItem->IsDeleted());

	// Nastavime flag, ze je soubor smazan
	pItem->SetDeleted();
}

void CDataArchiveFileDirectoryInfo::RemoveDirectory(CString strDirName) {
	m_bModified=TRUE;
	ASSERT(m_dwHeaderOffset);

	// Predpokladame, ze adresar daneho jmena EXISTUJE a je prazdny
	CDataArchiveFileDirectoryItem *pItem;
	pItem=FileInfo(strDirName);
	
	ASSERT(pItem);
	ASSERT(pItem->IsDirectory());
	ASSERT(!pItem->IsDeleted());

	// Nastavime flag, ze je adresar smazan
	pItem->SetDeleted();
}

DWORD CDataArchiveFileDirectoryInfo::GetStorageSize() {
	ASSERT(m_dwHeaderOffset);	

	int bufSize;
	POSITION pos;
	void *pValue;
	CString key;
	CDataArchiveFileDirectoryItem *pItem;

	bufSize=0;

	// Projedeme tabulku
	for(pos=m_FileTable.GetStartPosition(); pos!=NULL;) {
		m_FileTable.GetNextAssoc(pos, key, pValue);

		pItem=(CDataArchiveFileDirectoryItem *)pValue;

		bufSize+=pItem->GetStorageSize();
	}

	return bufSize;
}

int CDataArchiveFileDirectoryInfo::GetNumFiles() {
	ASSERT(m_dwHeaderOffset);

	POSITION pos;
	CString key;
	void *pValue;
	CDataArchiveFileDirectoryItem *pItem;
	int nFiles;

	nFiles=0;

	for(pos=m_FileTable.GetStartPosition(); pos!=NULL;) {
		m_FileTable.GetNextAssoc(pos, key, pValue);

		pItem=(CDataArchiveFileDirectoryItem *)pValue;

		if(!pItem->IsDeleted()) nFiles++;
	}

	return nFiles;
}

CDataArchiveDirContents *CDataArchiveFileDirectoryInfo::GetDirContents() {
	ASSERT(m_dwHeaderOffset);
	
	CDataArchiveFileDirectoryItem **m_pItems;
	POSITION pos;
	CString key;
	void *pValue;
	CDataArchiveFileDirectoryItem *pItem;	
	int nFiles=GetNumFiles()-2;

	m_pItems=new CDataArchiveFileDirectoryItem *[nFiles];

	int i=0;
	for(pos=m_FileTable.GetStartPosition(); pos!=NULL;) {
		m_FileTable.GetNextAssoc(pos, key, pValue);

		pItem=(CDataArchiveFileDirectoryItem *)pValue;

		if(!pItem->IsDeleted() && pItem->GetName()!="." && pItem->GetName()!="..") {
			m_pItems[i]=new CDataArchiveFileDirectoryItem(pItem->GetName(), pItem->GetFlags(), pItem->GetOffset());
			i++;
		}
	}
	ASSERT(i==nFiles);
	return new CDataArchiveDirContents(nFiles, m_pItems);
}

void CDataArchiveFileDirectoryInfo::DetachFromLetter(CDataArchiveFileDirectory *pLetter) {
	ASSERT(m_nPointerCount>0);
	m_nPointerCount--;
	if(m_nPointerCount==0) {
		delete this;
		return;
	}
}

void CDataArchiveFileDirectoryInfo::AttachToLetter(CDataArchiveFileDirectory *pLetter) {
	ASSERT(m_nPointerCount>=0);
	m_nPointerCount++;
}

void CDataArchiveFileDirectoryInfo::SetAt(CString strName, CDataArchiveFileDirectoryItem *pItem)
{
	strName.MakeLower();
	m_FileTable.SetAt(strName, pItem);
}
