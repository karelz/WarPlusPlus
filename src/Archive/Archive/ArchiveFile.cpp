// ArchiveFile.cpp: implementation of the CArchiveFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ArchiveFile.h"
#include "DataArchive.h"

/*BOOL CArchiveFile::Open(LPCTSTR lpcszFileName, DWORD dwOpenFlags) { 	
	ASSERT(m_pFileInfo); return m_pFileInfo->Open(lpcszFileName, dwOpenFlags); 
}*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CArchiveFile::CArchiveFile()
{
	// Zadne informace o souboru
	m_pFileInfo=(CArchiveFileInfo*)NULL;	
}

// Konstruktor obalovac
CArchiveFile::CArchiveFile(CArchiveFileInfo *pArchiveFileInfo, CDataArchiveInfo *pDataArchiveInfo)
{
	ASSERT(pArchiveFileInfo);
	ASSERT(pDataArchiveInfo);
	m_pFileInfo=pArchiveFileInfo;
	m_pFileInfo->AttachToArchiveFile(this);
	m_pFileInfo->AttachToDataArchive(pDataArchiveInfo);
}

CArchiveFile::~CArchiveFile()
{	
	// Jestlize jsme byli navazani
	// Odpojime se
	if(m_pFileInfo) {
		m_pFileInfo->DetachFromArchiveFile(this);
		m_pFileInfo=(CArchiveFileInfo*)NULL;
	}
}

// Copy konstruktor
CArchiveFile::CArchiveFile(const CArchiveFile& ArchiveFile)
{
	// Zadne informace o souboru
	m_pFileInfo=(CArchiveFileInfo*)NULL;

	// Pouzijeme operator =
	(*this)=ArchiveFile; 
}

// Operator =
CArchiveFile& CArchiveFile::operator =(const CArchiveFile & ArchiveFile)
{
	// Prirazuji sam do sebe
	if(((CArchiveFile*)&ArchiveFile)==this) return *this;

	// Byli jsme pripojeni k objektu CArchiveFileInfo.
	if(m_pFileInfo) {
		m_pFileInfo->DetachFromArchiveFile(this); // Odpojime se
		m_pFileInfo=(CArchiveFileInfo*)NULL; // A vynulujeme odkaz
	}

	ASSERT(!m_pFileInfo);
	
	// Zkopiruji se informace o souboru
	m_pFileInfo=ArchiveFile.m_pFileInfo;

	// FileInfo musi byt vyplnene, aby se s nim neco delalo
	// Jestli neni - patologicky pripad, kopiruji se dva nevyplnene soubory (to muze delat jen CDataArchive)
	if(m_pFileInfo) {
		m_pFileInfo->AttachToArchiveFile(this);
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
// Diagnostic support
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG // Diagnostic support

void CArchiveFile::AssertValid(void) const
{
	// Assert pro otce
	CFile::AssertValid();

	// Vlastni asserty
	ASSERT(m_pFileInfo);
}

void CArchiveFile::Dump(CDumpContext& dc) const
{
	// Dumpnout otce
	CFile::Dump(dc);

	dc << "Dump souboru\n";
}

#endif // Diagnostic support

// Abort
void CArchiveFile::Abort()
{
	try {
		Close();
	} catch (CDataArchiveException *pError) {
		// Jestli nastala chyba, ignorujeme ji.
		pError->Delete();
	}
}

// Duplikuje obalku na soubor
CFile* CArchiveFile::Duplicate() const {
	// Vytvorime novou obalku pomoci copy konstruktoru
	CArchiveFile *pArchiveFile=new CArchiveFile(*this);

	// Vratime na ni ukazatel
	return (CFile*)pArchiveFile;
}

CDataArchive CArchiveFile::GetDataArchive() const { 
	ASSERT(m_pFileInfo); 
	return m_pFileInfo->GetDataArchive(); 
}
