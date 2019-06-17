// DataArchiveFileDirectory.cpp: implementation of the CDataArchiveFileDirectory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataArchiveFileDirectory.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CDataArchiveFileDirectory::Open(CMappedFile &File, DWORD dwOffset) {
	ASSERT(m_pDirInfo==NULL);
	m_pDirInfo=new CDataArchiveFileDirectoryInfo();
	m_pDirInfo->AttachToLetter(this);
	m_pDirInfo->Open(File, dwOffset);
}

void CDataArchiveFileDirectory::CreateRoot(CMappedFile &File, DWORD dwOffset) {
	ASSERT(m_pDirInfo==NULL);
	m_pDirInfo=new CDataArchiveFileDirectoryInfo();
	m_pDirInfo->AttachToLetter(this);
	m_pDirInfo->CreateRoot(File, dwOffset);
}

CDataArchiveFileDirectory::CDataArchiveFileDirectory(CDataArchiveFileDirectory &CopyFrom) {
	m_pDirInfo=NULL;

	if(m_pDirInfo==CopyFrom.m_pDirInfo) { // Kopirujeme sami od sebe
		return;
	}
	
	if(m_pDirInfo!=NULL) { // Je potreba nejdrive odpojit stare DirInfo
		m_pDirInfo->DetachFromLetter(this);
		m_pDirInfo=NULL;
	}
	
	ASSERT(m_pDirInfo==NULL);

	m_pDirInfo=CopyFrom.m_pDirInfo;

	m_pDirInfo->AttachToLetter(this);
}

// Destruktor
CDataArchiveFileDirectory::~CDataArchiveFileDirectory() {
	if(m_pDirInfo!=NULL) { // Letter neni prazdny
		m_pDirInfo->DetachFromLetter(this);
		m_pDirInfo=NULL;
	}
}
