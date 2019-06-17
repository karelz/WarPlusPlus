// CompilerOutputArchive.h: interface for the CCompilerOutputArchive class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPILEROUTPUTARCHIVE_H__488BCBEE_4C78_45EA_8E06_25BFD13A42CC__INCLUDED_)
#define AFX_COMPILEROUTPUTARCHIVE_H__488BCBEE_4C78_45EA_8E06_25BFD13A42CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CompilerOutput.h"
#include "Archive\Archive\Archive.h"

class CCompilerOutputArchive : public CCompilerOutput  
{
public:
	CCompilerOutputArchive();
	virtual ~CCompilerOutputArchive();

    // Otevreni vystupu - bude se ukladat do souboru strFilename, ktere vytvori
	// CodeManager
    // (-vola manazer kodu) 
	// throws CFileException	
	virtual bool Open(CDataArchive Archive, CString &strFilename);
    // zavreni vystupu  (-vola manager kodu)
	virtual void Close();

    // zapsani znaku na vystup   (-vola prekladac)
	virtual void PutChars(const char *pBuffer, int nCount);
    // vyprazdneni cache (bufferu)  (-vola prekladac)
	virtual void Flush();

protected:
    // vystupni soubor
	CArchiveFile m_fileOut;
};

#endif // !defined(AFX_COMPILEROUTPUTARCHIVE_H__488BCBEE_4C78_45EA_8E06_25BFD13A42CC__INCLUDED_)
