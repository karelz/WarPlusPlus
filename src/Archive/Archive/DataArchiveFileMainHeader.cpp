// DataArchiveFileMainHeader.cpp: implementation of the CDataArchiveFileMainHeader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataArchiveFileMainHeader.h"
#include "DataArchiveException.h"

//////////////////////////////////////////////////////////////////////
// Definice

// Nejnovejsi verze archivu
#define DATA_ARCHIVE_VERSION 1

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDataArchiveFileMainHeader::CDataArchiveFileMainHeader()
{
}

CDataArchiveFileMainHeader::~CDataArchiveFileMainHeader()
{
}

void CDataArchiveFileMainHeader::Init()
{
	// Identifikace
	m_HeaderData.strID[0]='W'; // War
	m_HeaderData.strID[1]='A'; // ARchive
	m_HeaderData.strID[2]='R';

	// Je vyuzito jen tolik bytu, kolik potrebuje nase struktur
	m_HeaderData.dwBytesUsed=sizeof(m_HeaderData);
	
	// Nevi se, kde zacina root directory
	m_HeaderData.dwRootOffset=0;

	// Verze je ta aktualni
	m_HeaderData.dwVersion=DATA_ARCHIVE_VERSION;
}

void CDataArchiveFileMainHeader::Read(CMappedFile &File) 
{
	try {
		File.Seek(0, CFile::begin);
		File.Read(&m_HeaderData, sizeof(m_HeaderData));
	} catch(CDataArchiveException *pError) {
		CDataArchiveException *pEx;
		pEx=new CDataArchiveException(File.Path(), 
									pError->GetErrorCode(), 
									CDataArchiveException::EDataArchiveExceptionLocations::archiveItself);
		pError->Delete();
		throw pEx;
	}
	
	// Test na identifikator archivu a verzi
	if(strncmp(m_HeaderData.strID, "WAR", 3)!=0 || m_HeaderData.dwVersion!=DATA_ARCHIVE_VERSION) {
		throw new CDataArchiveException(File.Path(),
									CDataArchiveException::EDataArchiveExceptionTypes::unknownArchiveFormat,
									CDataArchiveException::EDataArchiveExceptionLocations::archiveItself);
	}
}

void CDataArchiveFileMainHeader::Write(CMappedFile &File) {
	try {
		File.Seek(0, CFile::begin);
		File.Write(&m_HeaderData, sizeof(m_HeaderData));
	} catch(CDataArchiveException *pError) {
		CDataArchiveException *pEx;
		pEx=new CDataArchiveException(File.Path(), 
									pError->GetErrorCode(), 
									CDataArchiveException::EDataArchiveExceptionLocations::archiveItself);
		pError->Delete();
		throw pEx;
	}

}
