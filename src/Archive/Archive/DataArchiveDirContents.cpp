// DataArchiveDirContents.cpp: implementation of the CDataArchiveDirContents class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataArchiveDirContents.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDataArchiveDirContents::CDataArchiveDirContents(int nItems, CDataArchiveFileDirectoryItem **pItems)
{
	m_nItems=nItems;
	m_pItems=pItems;
	m_nActual=0;
}

CDataArchiveDirContents::~CDataArchiveDirContents()
{
	for(int i=0; i<m_nItems; i++) {
		delete m_pItems[i];
	}
	if(m_pItems) {
		delete [] m_pItems;
	}
}

BOOL CDataArchiveDirContents::MoveFirst()
{
	m_nActual=0;
	if(m_nActual>=m_nItems) return FALSE; else return TRUE;
}

BOOL CDataArchiveDirContents::MoveNext()
{
	if(m_nActual+1>=m_nItems) return FALSE; else {
		m_nActual++;
		return TRUE;
	}	
}

CDataArchiveFileDirectoryItem *CDataArchiveDirContents::GetInfo() {
	if(m_nActual>=0 && m_nActual<m_nItems) 
		return m_pItems[m_nActual];
	else
		return NULL;
}
