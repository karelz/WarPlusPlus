/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: CCompilerErrorOutput - abstraktni trida pro vystup kompilatoru.
 *          Potomci CCompilerFileOutput a CCompilerMemoryOutput.
 * 
 ***********************************************************/

#if !defined(AFX_COMPILEROUTPUT_H__6B703564_EC43_11D2_AE99_004F49068BD6__INCLUDED_)
#define AFX_COMPILEROUTPUT_H__6B703564_EC43_11D2_AE99_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// class CCompilerErrorOutput
// ==========================
// Akstraktni predek pro tridy implementujici jakykoli vystup kompilatoru.
class CCompilerErrorOutput : public CObject
{
public:
	CCompilerErrorOutput();
	virtual ~CCompilerErrorOutput();

public:
    // Zapsani znaku na vystup (-vola prekladac)
	virtual void PutChars(const char *pBuffer, int nCount)=0;
    // Vyprazdneni cache (jsou-li) (-vola prekladac pred skoncenim)
	virtual void Flush()=0;

    // Zapsani retezce na vystup (-vola prekladac)
    virtual void PutString(LPCTSTR pstrOut);	
    // Zapsani formatovaneho retezce na vystup (-vola prekladac)
    virtual void Format(LPCTSTR pstrFormat, ...); 

    // Dotaz je-li vystup otevren. (-vola prekladac)
	bool IsOpen()	{	return m_bOpen;	}

protected:
    // Priznak, je-li otevren.
	bool m_bOpen;
};

#endif // !defined(AFX_COMPILEROUTPUT_H__6B703564_EC43_11D2_AE99_004F49068BD6__INCLUDED_)
