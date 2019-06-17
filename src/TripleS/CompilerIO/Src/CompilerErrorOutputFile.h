/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CCompilerErrorOutputFile - vystup kompilatoru do souboru
 * 
 ***********************************************************/

#if !defined(AFX_COMPILERFILEOUTPUT_H__6B703569_EC43_11D2_AE99_004F49068BD6__INCLUDED_)
#define AFX_COMPILERFILEOUTPUT_H__6B703569_EC43_11D2_AE99_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CompilerErrorOutput.h"

// class CCompilerErrorOutputFile
// ==============================
// Trida implementujici chybovy vystup kompilatoru do souboru.
class CCompilerErrorOutputFile : public CCompilerErrorOutput  
{
public:
	CCompilerErrorOutputFile();
	virtual ~CCompilerErrorOutputFile();

public:
    // Zapsani znaku na vystup. (-vola prekladac)
	virtual void PutChars(const char *pBuffer, int nCount);
    // Vyprazdneni bufferu. (-vola prekladac)
	virtual void Flush();

    // Otevreni vystupu.
	bool Open(CString& str1Filename, CFileException* pFileException=NULL);
    // Zavreni vystupu.
	void Close();

protected:
    // velikost bufferu
	enum { BUFFER_SIZE=1024 };
    // vystupni soubor
	CFile m_fileOut;
    // buffer
	char m_pBuffer[BUFFER_SIZE];
    // aktualni pozice v bufferu
	int m_nPos;
};

#endif // !defined(AFX_COMPILERFILEOUTPUT_H__6B703569_EC43_11D2_AE99_004F49068BD6__INCLUDED_)
