/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CCompilerInputFile - vstup kompilatoru ze souboru
 * 
 ***********************************************************/


#if !defined(AFX_COMPILERFILEINPUT_H__6B703568_EC43_11D2_AE99_004F49068BD6__INCLUDED_)
#define AFX_COMPILERFILEINPUT_H__6B703568_EC43_11D2_AE99_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CompilerInput.h"

// class CCompilerInputFile
// ========================
// Trida implementujici vstup kompilatoru ze souboru.
class CCompilerInputFile : public CCompilerInput  
{
public:
	CCompilerInputFile();
	virtual ~CCompilerInputFile();

public:
    // nacteni jednoho znaku (-vola prekladac)
	virtual int GetChar();
    // resetovani vstupu (=>bude se cist zase od zacatku) (-vola prekladac)
    virtual void Reset();

    // otevreni souboru
	bool Open(CString& strFilename, CFileException* pFileException=NULL);
    // zavreni souboru
	void Close();

    // Tohle vola kompilator v pripade, ze preklad probehl v poradku.
    // U souboru by se mel nastavit read-only flag. Ten pak rozeznava
    // CodeManager. Soubory s read-only flagem == soubory, ktere byly uspesne
    // zkompilovany a jejich binarky byly ulozeny do spravneho adresare.
    void OnInputCompiledAndUploadedSuccessfully();


protected:
    // velikost bufferu
	enum { BUFFER_SIZE=1024 };

    // vstupni soubor
    CFile fileIn;
    // buffer
	char m_pBuffer[BUFFER_SIZE];
    // pocet znaku v bufferu, pozice v bufferu
	int m_nRead, m_nPos;

    // priznak, zda se na pri zavreni soubory nastavit SuccessfullyCompiled flag 
    // (== read-only flag u souboru)
    bool m_bSetSuccessfullyCompiledFlag;

    // Jmeno souboru (i s cestou)
    CString m_strFilename;
};

#endif // !defined(AFX_COMPILERFILEINPUT_H__6B703568_EC43_11D2_AE99_004F49068BD6__INCLUDED_)
