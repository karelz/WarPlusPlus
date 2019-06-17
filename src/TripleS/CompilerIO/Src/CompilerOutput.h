/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: CCompilerOutput - abstraktni trida pro vystup kompilatoru.
 *          Potomci jsou CCompilerFileOutput a CCompilerMemoryOutput 
 *          (v jinem souboru)
 * 
 ***********************************************************/


#if !defined(AFX_COMPILEROUTPUT_H__BF10FD64_5947_11D3_A533_00A0C970CB8E__INCLUDED_)
#define AFX_COMPILEROUTPUT_H__BF10FD64_5947_11D3_A533_00A0C970CB8E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CompilerErrorOutput.h"

// class CCompilerOutput
// =====================
// Trida implementujici standartni vystup prekladace, ktery je vzdy do souboru.
class CCompilerOutput : public CCompilerErrorOutput 
{
public:
	CCompilerOutput();
	virtual ~CCompilerOutput();

public:
    
    // Otevreni vystupu - bude se ukladat do souboru strFilename, ktere vytvori
	// CodeManager
    // (-vola manazer kodu) 
	// throws CFileException	
	virtual bool Open(CString &strFilename);
    // zavreni vystupu  (-vola namager kodu)
	virtual void Close();

    // zapsani znaku na vystup   (-vola prekladac)
	virtual void PutChars(const char *pBuffer, int nCount);
    // vyprazdneni cache (bufferu)  (-vola prekladac)
	virtual void Flush();

    // ulozeni typu short  (-vola prekladac)
	inline void PutShort(short n) { PutChars((char*)&n, sizeof(short)); }
    // ulozeni typu long (-vola prekladac)
	inline void PutLong(long l)   { PutChars((char*)&l, sizeof(long));      }
    // ulozeni typu float (-vola prekladac)
	inline void PutDouble(double f) { PutChars((char*)&f, sizeof(double));     }

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

#endif // !defined(AFX_COMPILEROUTPUT_H__BF10FD64_5947_11D3_A533_00A0C970CB8E__INCLUDED_)
