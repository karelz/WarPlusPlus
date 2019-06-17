// DataArchiveFileCommon.cpp: implementation of the CDataArchiveFileCommon class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataArchiveFileCommon.h"
#include "DataArchiveFile.h"
#include "DataArchiveFileMainHeader.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDataArchiveFileCommon::CDataArchiveFileCommon()
{
	m_pMainHeader=NULL;
	m_bRebuildRequest=FALSE;
	m_bRebuildRunning=FALSE;
	m_nRefCount=0;
}

CDataArchiveFileCommon::~CDataArchiveFileCommon()
{
	if(m_pMainHeader!=NULL) {
		delete m_pMainHeader;
		m_pMainHeader=NULL;
	}
}

void CDataArchiveFileCommon::AddRef(CDataArchiveFile *pFile)
{
	ASSERT(m_nRefCount>=0);
	m_nRefCount++;
}

void CDataArchiveFileCommon::Release(CDataArchiveFile *pFile)
{
	ASSERT(m_nRefCount>0);
	m_nRefCount--;
	// Zapomeneme mapovani
	// m_ThreadMap.RemoveKey(pFile->m_dwThreadID);
	if(m_nRefCount==0) {
		// ASSERT(m_ThreadMap.IsEmpty());
		delete this;
	}
}
