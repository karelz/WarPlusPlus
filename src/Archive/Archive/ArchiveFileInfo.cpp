// ArchiveFileInfo.cpp: implementation of the CArchiveFileInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ArchiveFileInfo.h"
#include "DataArchiveInfo.h"
#include "DataArchive.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CArchiveFileInfo::CArchiveFileInfo()
{
	Create();
}

CArchiveFileInfo::~CArchiveFileInfo()
{
	Delete();
}

CArchiveFileInfo::CArchiveFileInfo(CString strFileName) {
	Create(strFileName);
}

BOOL CArchiveFileInfo::Create() {
	// Na zacatku na mne nikdo neodkazuje
	m_nPointerCounter=0;
	m_pArchiveInfo=(CDataArchiveInfo*)NULL;
	return SUCCESS;
}

BOOL CArchiveFileInfo::Create(CString strFileName) {
	Create();
	m_strFileName=strFileName;
	return SUCCESS;
}

BOOL CArchiveFileInfo::Delete() {
	// Niceny objekt nesmi mit zadne ostatni odkazujici na nej	
	ASSERT(m_nPointerCounter==0);

	Close();

	// Rekneme svemu archivu, ze jsme niceni
	if(m_pArchiveInfo) {
		m_pArchiveInfo->FileDestructed(this);
	}
	return SUCCESS;
}

void CArchiveFileInfo::DetachFromArchiveFile(CArchiveFile * ArchiveFile)
{
	// Pokud se odpojujeme, museli jsme jiz byt pripojeni
	ASSERT(m_nPointerCounter>0);
	
	// Snizim pocet odkazu
	long nPC=InterlockedDecrement((LONG*)&m_nPointerCounter);

	// Koncim, pokud jiz na mne nikdo neodkazuje
	if(nPC==0) {
		delete this;
		return;
	}
}

void CArchiveFileInfo::AttachToArchiveFile(CArchiveFile *ArchiveFile)
{
	// Zvysim pocet odkazu
	InterlockedIncrement((LONG*)&m_nPointerCounter);
}

void CArchiveFileInfo::AttachToDataArchive(CDataArchiveInfo *pArchiveInfo)
{
	ASSERT(m_pArchiveInfo==NULL);
	ASSERT(pArchiveInfo);

	// Zapamatujeme si archiv
	m_pArchiveInfo=pArchiveInfo;
	pArchiveInfo->FileConstructed(this);
}

void CArchiveFileInfo::ChangeDataArchive(CDataArchiveInfo *pArchiveInfo, CString strNewFileName)
{
	ASSERT(m_pArchiveInfo);
	ASSERT(pArchiveInfo);

	pArchiveInfo->FileConstructed(this);
	m_pArchiveInfo->FileDestructed(this);
	m_pArchiveInfo=pArchiveInfo;

	m_strFileName=strNewFileName;
}

CString CArchiveFileInfo::GetFileName() const
{
	CString strFileName=m_strFileName;
	int nPos, nPos2;
	// Najdeme si lomitka
	nPos=strFileName.ReverseFind('\\');
	nPos2=strFileName.ReverseFind('/');
	// Vezmeme to opravdu posledni
	nPos=(nPos>nPos2)?nPos:nPos2;
	// A usekneme ho i s tim, co je pred nim
	if(nPos!=-1) {
		strFileName=strFileName.Mid(nPos+1);
	}
 
	return strFileName;
}	

CString CArchiveFileInfo::GetFilePath() const
{
	return m_strFileName;
}

CString CArchiveFileInfo::GetFileTitle() const
{
	// Vezmeme samotne jmeno
	CString strFileName=GetFileName();
	// Najdeme prvni tecku odzadu
	int nPos=strFileName.ReverseFind('.');
	// Urizneme priponu
	if(nPos!=-1) { strFileName=strFileName.Left(nPos); }
	return strFileName;
}

CString CArchiveFileInfo::GetFileDir() const
{
	CString strDirName=m_strFileName;
	int nPos, nPos2;
	// Najdeme si lomitka
	nPos=strDirName.ReverseFind('\\');
	nPos2=strDirName.ReverseFind('/');
	// Vezmeme to opravdu posledni
	nPos=(nPos>nPos2)?nPos:nPos2;
	// A usekneme s nim to, co je za nim
	if(nPos!=-1) {
		strDirName=strDirName.Left(nPos);
	} else {
		strDirName=".";
	}
 
	return strDirName;
}	

CDataArchive CArchiveFileInfo::GetDataArchive() const
{
	ASSERT(m_pArchiveInfo);
	CDataArchive Archive(m_pArchiveInfo);
	
	return Archive;
}	

BOOL CArchiveFileInfo::Lock(DWORD dwTimeOut /* = INFINITE */)
{
	return m_pArchiveInfo->Lock(dwTimeOut);
}

BOOL CArchiveFileInfo::Unlock()
{
	return m_pArchiveInfo->Unlock();
}

CArchiveFile CArchiveFileInfo::CreateFileClone() 
{
	return m_pArchiveInfo->CreateFile(m_strFileName, CArchiveFile::modeRead | CFile::shareDenyWrite);
}