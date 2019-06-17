/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: CCompilerInput - abstraktni trida pro vstup kompilatoru.
 *          Potomci CCompilerFileInput a CCompilerMemoryInput.
 * 
 ***********************************************************/

#if !defined(AFX_COMPILERINPUT_H__6B703563_EC43_11D2_AE99_004F49068BD6__INCLUDED_)
#define AFX_COMPILERINPUT_H__6B703563_EC43_11D2_AE99_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// class CCompilerInput
// ====================
// Akstraktni predek pro tridy implementujici vstup kompilatoru.
class CCompilerInput : public CObject
{
public:
	CCompilerInput();
	virtual ~CCompilerInput();

public:
    // nacteni jednoho znaku (-vola prekladac)
	virtual int GetChar()=0;
    // resetovani vstupu (=>bude se cist zase od zacatku)  (-vola prekladac)
    virtual void Reset()=0; 

    // jmeno vstupu
	void SetName(CString& strName)		{	m_strName=strName;	}
    
    // ziskani jmena vstupu  (-vola prekladac)
	CString GetName()					{	return m_strName;	}
    
    // znovuziskani posledniho nacteneho znaku  (-vola prekladac)
    char GetLastChar()                  {   return m_chLastChar;  }

    // dotaz na to, je-li otevren  (-vola prekladac)
	bool IsOpen()		{	return m_bOpen;	}

    // Tohle vola kompilator v pripade, ze preklad probehl v poradku.
    // U souboru by se mel nastavit read-only flag. Ten pak rozeznava
    // CodeManager. Soubory s read-only flagem == soubory, ktere byly uspesne
    // zkompilovany a jejich binarky byly ulozeny do spravneho adresare.
    virtual void OnInputCompiledAndUploadedSuccessfully() = 0;

protected:
    // priznak, je-li otevren
	bool m_bOpen;
    // jmeno vstupu (ktere bude vypisovani v chybovych hlaskach)
	CString m_strName; 
    // posledni nacteny znak
    char m_chLastChar;
};

#endif // !defined(AFX_COMPILERINPUT_H__6B703563_EC43_11D2_AE99_004F49068BD6__INCLUDED_)
