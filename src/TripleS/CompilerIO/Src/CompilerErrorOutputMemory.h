/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CCompilerErrorOutputMemory - vystup kompilatoru
 *          do pametovych bufferu
 * 
 ***********************************************************/

#if !defined(AFX_COMPILERMEMORYOUTPUT_H__6B70356A_EC43_11D2_AE99_004F49068BD6__INCLUDED_)
#define AFX_COMPILERMEMORYOUTPUT_H__6B70356A_EC43_11D2_AE99_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CompilerErrorOutput.h"

class CCompilerErrorOutputMemory;

// Definice callbackove funkce pouzite ve tride CCompilerInputMemory.
// Paramentry: 
// - ukazatel na objekt, ktery ji vola
// - pocet platnych bytu v bufferu
// - ukazatel na uzivatelska data
typedef void (*COMPILERMEMORYOUTPUT_BUFFERFULL_CALLBACK)(CCompilerErrorOutputMemory*, int, void*);

// class CCompilerErrorOutputMemory
// ================================
// Trida implementujici chybovy vystup do bufferu pameti.
// Pri otevreni vystupu se zada buffer, do ktereho se ma psat,
// jeho delka a callbackova funkce, ktera se ma volat, jestlize je 
// buffer plny. Není callbacková funkce zadána, nebo neni-li
// v ni zadan novy buffer, do ktereho se ma psat (metodou SetBuffer), je
// prepisovan posledni znak v bufferu.
// Callbackova funkce je take volana pri skonceni zapisu (metoda Flush()),
// kde jednim z parametru je pocet platnych bytu v bufferu.
class CCompilerErrorOutputMemory : public CCompilerErrorOutput  
{
public:
	CCompilerErrorOutputMemory();
	virtual ~CCompilerErrorOutputMemory();

public:
    // 'otevreni' vystupu - zadani prvniho bufferu, callbackove funkce a 
    // uzivatelskych dat
	void Open(char *pBuffer, long nBufferLength, 
		COMPILERMEMORYOUTPUT_BUFFERFULL_CALLBACK pBufferFullCallback, void *pUserData=NULL);
	// Zavreni vystupu.
    void Close();  

    // Ulozeni znaku na vystup. Je-li na konci bufferu, vola se callbackova funkce
    // pro nastaveni noveho bufferu ((*m_pOnBufferFullCallback)(this,m_nPos,m_pUserData))
    // (-vola prekladac)
	virtual void PutChars(const char *pBuffer, int nCount);
    // zavolani (*m_pOnBufferFullCallback)(this, m_nPos, m_pUserData);
    // (-vola prekladac)
	virtual void Flush();

    // nastaveni bufferu (predpoklada se, se ze provede ve 
    // volani (*m_pOnBufferFullCallback)(this,m_nPos,m_pUserData)
	void SetBuffer(char *pBuffer, long nBufferLength);
    // nastaveni callbackove funkce a pointeru na uzivatelska data
	void SetBufferFullCallback(COMPILERMEMORYOUTPUT_BUFFERFULL_CALLBACK pBufferFullCallback, 
		void *pUserData=NULL);
	
protected:
    // delka bufferu, aktualni pozice
	long m_nBufferLength, m_nPos;
    // buffer
	char *m_pBuffer;
    // uzivatelska data
	void *m_pUserData;
    // callbackova funkce (-vola prekladac)
	COMPILERMEMORYOUTPUT_BUFFERFULL_CALLBACK m_pOnBufferFullCallback;
};

#endif // !defined(AFX_COMPILERMEMORYOUTPUT_H__6B70356A_EC43_11D2_AE99_004F49068BD6__INCLUDED_)
