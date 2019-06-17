// CompilerInputMultipleFile.h: interface for the CCompilerInputMultipleFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPILERINPUTMULTIPLEFILE_H__68D40805_1843_11D4_B03C_004F49068BD6__INCLUDED_)
#define AFX_COMPILERINPUTMULTIPLEFILE_H__68D40805_1843_11D4_B03C_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CompilerInput.h"

class CCompiler;

class CCompilerInputMultipleFile : public CCompilerInput  
{
public:
	CCompilerInputMultipleFile();
	virtual ~CCompilerInputMultipleFile();

public:
    // nacteni jednoho znaku (-vola prekladac)
	virtual int GetChar();
    // resetovani vstupu (=>bude se cist zase od zacatku) (-vola prekladac)
    virtual void Reset();

    // Otevreni vsech souboru.
    // V parametru strFilenames jsou jmena vsech souboru oddelena znakem '\n', v strPath je cesta k adresari, kde jsou.
    // Vraci false kdyz se nektery ze souboru nepovedlo otevrit. Pak je v pFileException duvod proc.
	bool Open(CString &strPath, CString &strFilenames, CCompiler *pCompiler, 
              CFileException* pFileException=NULL);
    // zavreni vsech souboru
	void Close();

    // Tohle vola kompilator v pripade, ze preklad probehl v poradku.
    // Nastavuje read-only flag u vsech souboru (az po jejich zavreni).
    void OnInputCompiledAndUploadedSuccessfully();

protected:
    // velikost bufferu
	enum { BUFFER_SIZE=1024 };

    // vstupni soubory
    CArray<CFile,CFile> m_aInputFiles;
    // pocet souboru
    int m_nFilesCount;
    // aktualni soubor
    int m_nActualFile;

    // buffer
	char m_pBuffer[BUFFER_SIZE];
    // pocet znaku v bufferu, pozice v bufferu
	int m_nRead, m_nPos;

    // priznak, zda se na pri zavreni souboru nastavit SuccessfullyCompiled flag 
    // (== read-only flag u souboru)
    bool m_bSetSuccessfullyCompiledFlag;

    // compiler
    CCompiler *m_pCompiler;

    // Cesta k souborum
    CString m_strPath;
};

#endif // !defined(AFX_COMPILERINPUTMULTIPLEFILE_H__68D40805_1843_11D4_B03C_004F49068BD6__INCLUDED_)
