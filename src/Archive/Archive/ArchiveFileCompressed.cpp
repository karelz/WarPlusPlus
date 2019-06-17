// ArchiveFileCompressed.cpp: implementation of the CArchiveFileCompressed class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ArchiveFile.h"
#include "ArchiveFileCompressed.h"
#include "DataArchive.h"
#include "Definitions.h"
#include "../Compression/CompressException.h"
#include "ArchiveUtils.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define TRY_CMP	\
	try {

#define CATCH_CMP_CMPRS	\
	} catch (CCompressException *pEx) { \
		pEx->Delete();	\
		throw new CDataArchiveException(GetFilePath(), CDataArchiveException::compressionFailed, CDataArchiveException::fileInArchive, m_pArchiveInfo);	\
	}

#define CATCH_CMP_DECMPRS	\
	} catch (CCompressException *pEx) { \
		pEx->Delete();	\
		throw new CDataArchiveException(GetFilePath(), CDataArchiveException::decompressionFailed, CDataArchiveException::fileInArchive, m_pArchiveInfo);	\
	}

// Construction
// ------------

CArchiveFileCompressed::CArchiveFileCompressed(CArchiveFile compressedFile)
{
	m_dwMode=0;
	m_bWasOpened=FALSE;
	m_bUncompressedCreated=FALSE;
	m_pFileCompressed=new CArchiveFile();
	*m_pFileCompressed=compressedFile;
	m_strFileName=m_pFileCompressed->GetFilePath();
	m_pFileUncompressed=NULL;
	m_nAccessMode=0;
	m_dwPos=0;
	Open();
}

CArchiveFileCompressed::~CArchiveFileCompressed()
{
	if(m_bWasOpened) Close();
	DWORD dwLength=GetLength();
	m_Engine.Reset();
	m_Sink.Close();
	m_Source.Close();
	if(m_bUncompressedCreated && !CArchiveUtils::FlagWrite(m_dwMode)) {
		// Jenom se cetlo, staci pomocny soubor smazat
		ASSERT(m_pFileCompressed);
		CString strFileNameTmp=m_pFileUncompressed->GetFilePath();
		m_pFileUncompressed->Close();
		CDataArchive::GetTempArchive()->RemoveFile(strFileNameTmp);
	}
	if(m_bUncompressedCreated && CArchiveUtils::FlagWrite(m_dwMode)) {
		// Psalo se, je treba pomocny soubor spakovat
		ASSERT(m_pFileUncompressed);
		ASSERT(m_pFileCompressed);

		m_pFileUncompressed->Seek(0, CFile::begin);

		m_pFileCompressed->Seek(0, CFile::begin);
		m_pFileCompressed->SetLength(0);

		char *buf=NULL;
		buf=new char[ARCHIVE_DIRECTORY_COPY_BUF_SIZE];

		try {		
			m_Sink.Open(*m_pFileCompressed, dwLength);
			m_Engine.InitCompression(m_Sink);

			int read;
			while(read=m_pFileUncompressed->Read(buf, ARCHIVE_DIRECTORY_COPY_BUF_SIZE)) {
				m_Engine.Write(buf, read);
			}

			m_Engine.DoneCompression();

			CString strFileName=m_pFileUncompressed->GetFilePath();
			m_pFileUncompressed->Close();

			CDataArchive::GetTempArchive()->RemoveFile(strFileName);

		} catch(CCompressException *pEx) {
			if(buf) delete [] buf;
			buf=NULL;
			pEx->Delete();			

			if(m_pFileUncompressed)
				delete m_pFileUncompressed;
			if(m_pFileCompressed)
				delete m_pFileCompressed;

			throw new CDataArchiveException(m_pFileCompressed->GetFilePath(), CDataArchiveException::decompressionFailed, CDataArchiveException::fileInArchive, m_pArchiveInfo);
		} catch(...) {
			if(buf) delete [] buf;
			buf = NULL;

			if(m_pFileUncompressed)
				delete m_pFileUncompressed;
			if(m_pFileCompressed)
				delete m_pFileCompressed;
			m_pFileUncompressed=NULL;
			m_pFileCompressed=NULL;

			throw;
		}
		if(buf) delete [] buf;		
  } else {
    if(m_nAccessMode == 2) {
      // Psalo se, je treba pridat informaci o celkove delce na zacatek
		  m_pFileCompressed->Seek(0, CFile::begin); // Zapiseme celkovou delku
		  m_pFileCompressed->Write(&dwLength, sizeof(dwLength));
    }
	}
	if(m_pFileUncompressed)
		delete m_pFileUncompressed;
	if(m_pFileCompressed)
		delete m_pFileCompressed;
	m_pFileUncompressed=NULL;
	m_pFileCompressed=NULL;

}

// Otevre soubor
BOOL CArchiveFileCompressed::Open() {
	m_bWasOpened=TRUE;
	return SUCCESS;
}

// Zavre soubor
void CArchiveFileCompressed::Close() {
    if(m_bWasOpened) {
		Flush();
		m_bWasOpened=FALSE;
	}
}

// Input/Output
// ------------

// Nacteni bloku dat
UINT CArchiveFileCompressed::Read(void* lpBuf, UINT nCount) {
	DWORD dwNumRead;
  if(m_bUncompressedCreated) {
		ASSERT(m_pFileUncompressed);
    dwNumRead=m_pFileUncompressed->Read(lpBuf, nCount);
    m_dwPos+=dwNumRead;
    return dwNumRead;
	} else {
		switch(m_nAccessMode) {
		case 0: // Zaciname cist
			m_nAccessMode=1;
			ASSERT(m_pFileCompressed);
			m_Source.Open(*m_pFileCompressed);
			TRY_CMP
				m_Engine.InitDecompression(m_Source);
				dwNumRead=m_Engine.Read(lpBuf, nCount);
				m_dwPos+=dwNumRead;
				return dwNumRead;
			CATCH_CMP_DECMPRS
			break;
		case 1: // Uz se cte
			TRY_CMP
				dwNumRead=m_Engine.Read(lpBuf, nCount);
				m_dwPos+=dwNumRead;
				return dwNumRead;

			CATCH_CMP_DECMPRS
			break;
		case 2: // Zaciname psat
			CreateUncompressed();			
			dwNumRead=m_pFileUncompressed->Read(lpBuf, nCount);
			m_dwPos+=dwNumRead;
			return dwNumRead;
			
		default:
			ASSERT(FALSE);
			return 0;
		}
	}
}

// Zapis bloku dat
void CArchiveFileCompressed::Write(const void* lpBuf, UINT nCount) {
	if(m_bUncompressedCreated) {
		ASSERT(m_pFileUncompressed);
		m_pFileUncompressed->Write(lpBuf, nCount);
    m_dwPos+=nCount;
	} else {
		switch(m_nAccessMode) {
		case 0: // Zaciname psat
			m_nAccessMode=2;
			if(GetLength()==0) {
				ASSERT(m_pFileCompressed);
				TRY_CMP
					m_pFileCompressed->Seek(0, CFile::begin);
					m_pFileCompressed->SetLength(0);
					m_Sink.Open(*m_pFileCompressed);
					m_Engine.InitCompression(m_Sink);			
					m_Engine.Write(const_cast <void *>(lpBuf), nCount);
          m_dwPos+=nCount;
				CATCH_CMP_CMPRS
			} else {
				TRY_CMP
					CreateUncompressed();
					m_pFileUncompressed->Write(lpBuf, nCount);
          m_dwPos+=nCount;
				CATCH_CMP_CMPRS
			}
			break;
		case 1: // Zaciname cist
			CreateUncompressed();
			m_pFileUncompressed->Write(lpBuf, nCount);
      m_dwPos+=nCount;
			break;
		case 2: // Uz se pise
			TRY_CMP
				m_Engine.Write(const_cast <void *>(lpBuf), nCount);
        m_dwPos+=nCount;
			CATCH_CMP_CMPRS
		}
	}
}

// Flushne soubor
void CArchiveFileCompressed::Flush() {
	if(CArchiveUtils::FlagWrite(m_dwMode)) {
		if(m_bUncompressedCreated) {
			ASSERT(m_pFileUncompressed);
			m_pFileUncompressed->Flush();
		} else {
			ASSERT(m_pFileCompressed);
			m_pFileCompressed->Flush();
		}
	}
}

// Position
// --------

// Seek
LONG CArchiveFileCompressed::Seek(LONG lOff, UINT nFrom) {	
	LONG lEfOff, lPos;
	if(!m_bUncompressedCreated) {
		lPos=GetPosition();
		switch(nFrom) {
		case CFile::begin:
			lEfOff=lOff;
			break;
		case CFile::current:
			lEfOff=lPos+lOff;
			break;
		case CFile::end:
			lEfOff=m_pFileCompressed->GetLength()+lOff;
			break;
		}
		
		// Seek na miste
		if(lEfOff==lPos)
			return lPos;

		if(lEfOff<0 || (DWORD)lEfOff>GetLength()) {
			throw new CDataArchiveException(GetFilePath(), CDataArchiveException::badSeek, CDataArchiveException::fileInArchive, m_pArchiveInfo);
		}

		if(lEfOff>lPos && lEfOff-lPos<MAX_FORWARD_SEEK_IN_PLACE_LENGTH) {
			// Seek dopredu, kratky
			char buf[FORWARD_SEEK_BUFFER_SIZE];
			// Nacteme si do bufferu data a pak je zahodime
			while(lPos<lEfOff) {
				lPos+=Read(buf, min(lEfOff-lPos, FORWARD_SEEK_BUFFER_SIZE));
			}
			m_dwPos=lPos;
			return lPos;
		} else {
			// Seek dozadu, nebo hodne dlouhy dopredu
			CreateUncompressed();
		}
	}	
	ASSERT(m_pFileUncompressed);
	return m_dwPos=m_pFileUncompressed->Seek(lOff, nFrom);
}

// Vraci delku souboru
DWORD CArchiveFileCompressed::GetLength() const {
	if(m_bUncompressedCreated) {
		ASSERT(m_pFileUncompressed);
		return m_pFileUncompressed->GetLength();
	} else {
		switch(m_nAccessMode) {
		case 0: // Jeste se ani necetlo, ani nepsalo
			DWORD dwSize;
			try {
				// Jestlize se jeste nic nedelalo, musime byt v souboru na zacatku
				ASSERT(m_pFileCompressed->GetPosition()==0);
				if(m_pFileCompressed->Read(&dwSize, sizeof(dwSize))!=sizeof(dwSize)) {
					dwSize=0;
				}
				m_pFileCompressed->Seek(0, CFile::begin);
			} catch(...) {
				return 0;
			}
			return dwSize;
			break;
		case 1: // Uz se zacalo cist
			return m_Engine.GetUncompressedSize();
			break;
		case 2: // Uz se zacalo psat
			return m_Engine.GetUncompressedSize();
			break;
		default:
			ASSERT(FALSE);
			return 0;			
		}
	}
}

// Nastavuje delku souboru
void CArchiveFileCompressed::SetLength(DWORD dwNewLen) {
	if(!m_bUncompressedCreated) {
		CreateUncompressed();
	}	
	ASSERT(m_pFileUncompressed);
	m_pFileUncompressed->SetLength(dwNewLen);
}

// Locking
// -------

// Zamyka kus souboru
void CArchiveFileCompressed::LockRange(DWORD dwPos, DWORD dwCount) {
	TRACE("Funkce LockRange neni implementovana pro komprimovane soubory (zatim)\n");
	ASSERT(FALSE);
}

// Odemyka kus souboru
void CArchiveFileCompressed::UnlockRange(DWORD dwPos, DWORD dwCount) {
	TRACE("Funkce UnlockRange neni implementovana pro komprimovane soubory (zatim)\n");
	ASSERT(FALSE);
}

// Status
// ------

// Vraci aktualni pozici v souboru
DWORD CArchiveFileCompressed::GetPosition() const {
	if(m_bUncompressedCreated) {
		ASSERT(m_pFileUncompressed);
		return m_pFileUncompressed->GetPosition();
	} else {
		ASSERT(m_pFileCompressed);
		switch(m_nAccessMode) {
		case 0: // Jeste se ani necetlo, ani nepsalo
			return 0;
			break;
		case 1: // Uz se zacalo cist 
		case 2: // nebo psat
			return m_dwPos;
			break;
		default:
			ASSERT(FALSE);
			return 0;			
		}
	}
}

// Naplni status souboru
BOOL CArchiveFileCompressed::GetStatus(CFileStatus& rStatus) const {
	ASSERT(m_pFileCompressed);
	return m_pFileCompressed->GetStatus(rStatus);
}

BOOL CArchiveFileCompressed::IsCompressed() const {
	return TRUE;
}

BOOL CArchiveFileCompressed::IsOpened() const {
	return m_bWasOpened;
}

BOOL CArchiveFileCompressed::IsRaw() const {
	return m_dwMode & CArchiveFile::EArchiveFileFlags::modeRaw;
}

void CArchiveFileCompressed::CreateUncompressed() {
	ASSERT(m_pFileUncompressed==NULL);
	ASSERT(!m_bUncompressedCreated);

	m_pFileUncompressed=new CArchiveFile();
	*m_pFileUncompressed=CDataArchive::GetTempArchive()->CreateFile(CDataArchive::GetNewTemporaryName(), CArchiveFile::modeReadWrite | CArchiveFile::modeCreate | CArchiveFile::modeUncompressed);

	DWORD dwPos=GetPosition();

	char *buf=NULL;
	buf=new char[ARCHIVE_DIRECTORY_COPY_BUF_SIZE];

	try {
		m_Engine.Reset();

		m_pFileCompressed->Seek(0, CFile::begin);
		m_pFileCompressed->Flush();
		
		if(m_nAccessMode!=0) {
			m_Source.Close();
			m_Sink.Close();
		}

		m_Source.Open(*m_pFileCompressed);
		m_Engine.InitDecompression(m_Source);

		int read;
		
		while(read=m_Engine.Read(buf, ARCHIVE_DIRECTORY_COPY_BUF_SIZE)) {
			m_pFileUncompressed->Write(buf, read);
		}

		m_Engine.DoneDecompression();		

		m_pFileUncompressed->Seek(dwPos, CFile::begin);
	} catch(CCompressException *pEx) {
		if(buf) delete [] buf;
		buf=NULL;
		pEx->Delete();
		ASSERT(m_pFileUncompressed);
		delete m_pFileUncompressed;
		m_pFileUncompressed=NULL;
		throw new CDataArchiveException(m_pFileCompressed->GetFilePath(), CDataArchiveException::decompressionFailed, CDataArchiveException::fileInArchive, m_pArchiveInfo);
	} catch(CFileException *pFileEx) {
		if(buf) delete [] buf;
		buf = NULL;
		ASSERT(m_pFileUncompressed);
		delete m_pFileUncompressed;
		m_pFileUncompressed=NULL;
        throw new CDataArchiveException(pFileEx, m_pArchiveInfo);
    } catch(CException *pException) {
		if(buf) delete [] buf;
		buf = NULL;
		ASSERT(m_pFileUncompressed);
		delete m_pFileUncompressed;
		m_pFileUncompressed=NULL;
        throw pException;
    }
    if(buf) { delete [] buf; buf=NULL; }
	m_bUncompressedCreated=TRUE;
}

BOOL CArchiveFileCompressed::IsReadOnly() const {
	ASSERT(m_pFileCompressed);
	return m_pFileCompressed->IsReadOnly();
}

BOOL CArchiveFileCompressed::SetReadOnly(BOOL bReadOnly) {
	ASSERT(m_pFileCompressed);
	return m_pFileCompressed->SetReadOnly(bReadOnly);
}

void CArchiveFileCompressed::ChangeDataArchive(CDataArchiveInfo *pArchiveInfo, CString strNewFileName)
{
	ASSERT(m_pArchiveInfo);
	ASSERT(pArchiveInfo);

	m_pFileCompressed->m_pFileInfo->ChangeDataArchive(pArchiveInfo, m_pFileCompressed->GetFilePath());

	pArchiveInfo->FileConstructed(this);
	m_pArchiveInfo->FileDestructed(this);
	m_pArchiveInfo=pArchiveInfo;

	m_strFileName=strNewFileName;
}

