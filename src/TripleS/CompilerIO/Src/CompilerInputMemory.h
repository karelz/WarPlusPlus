/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CCompilerInputMemory - vstup kompilatoru
 *          z pametovych bufferu
 * 
 ***********************************************************/

#if !defined(AFX_COMPILERMEMORYINPUT_H__7C68B3C4_EC50_11D2_AE99_004F49068BD6__INCLUDED_)
#define AFX_COMPILERMEMORYINPUT_H__7C68B3C4_EC50_11D2_AE99_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CompilerInput.h"

class CCompilerInputMemory;

// Definice callbackove funkce pouzite ve tride CCompilerInputMemory.
// Paramentry: 
// - ukazatel na objekt, ktery ji vola
// - priznak, zda se chce, aby se vstup cetl cely jeste jednou (true) (prekladac 
//   zavolal Reset), nebo se jen doslo na konec bufferu (false)
// - ukazatel na uzivatelska data
typedef void (*COMPILERMEMORYINPUT_ENDOFBUFFER_CALLBACK)(CCompilerInputMemory*, bool, void*);

// class CCompilerInputMemory
// ==========================
// Trida implementujici vstup kompilatoru z bufferu pameti.
// Pri otevreni vstupu se zada buffer, ze ktereho se ma cist,
// jeho delka a callbackova funkce, ktera se ma volat, jestlize je 
// buffer cely precteny. Není callbacková funkce zadána, nebo neni-li
// v ni zadan novy buffer, ze ktereho se ma cist (metodou SetBuffer), je
// cteni ze vstupu ukonceno (jako by byl precten znak EOF).
// Kompilator je dvoupruchodovy, tj. je treba vstup cist dvakrat.
// Resetovani vstupu se provadi metodou Reset (vola kompilator),
// a vypada tak, ze se zavola callbackova funkce s priznakem,
// ze se bude cist zase od zacatku. Neni-li callbackova funkce zadana,
// zadany buffer se sam cte od zacatku.
class CCompilerInputMemory : public  CCompilerInput  
{
public:
	CCompilerInputMemory();
	virtual ~CCompilerInputMemory();

public:
    // Nacteni jednoho znaku. Je-li na konci bufferu, vola se callbackova funkce
    // pro nastaveni noveho bufferu ((*m_pOnEndOfBufferCallback)(this,false,m_pUserData))
    // (-vola prekladac)
	virtual int GetChar();
    // Resetovani vstupu (=>bude se cist zase od zacatku) - volani callbackove
    // funkce (*m_pOnEndOfBufferCallback)(this,true,m_pUserData);
    // (-vola prekladac)
    virtual void Reset();

    // 'otevreni' vstupu - nastaveni prvniho bufferu, callbackove funkce a uzivatelskych
    // dat predavanych funkci
	void Open(char *pBuffer, long nBufferLength, 
		 COMPILERMEMORYINPUT_ENDOFBUFFER_CALLBACK pEndOfBufferCallback, void *pUserData=NULL);
    // 'zavreni' vstupu
	void Close(); 

    // Tohle vola kompilator v pripade, ze preklad probehl v poradku.
    // U souboru by se mel nastavit read-only flag. Ten pak rozeznava
    // CodeManager. Soubory s read-only flagem == soubory, ktere byly uspesne
    // zkompilovany a jejich binarky byly ulozeny do spravneho adresare.
    void OnInputCompiledAndUploadedSuccessfully() {}

    // nastaveni bufferu (predpoklada se, se ze provede ve 
    // volani (*m_pOnEndOfBufferCallback)(this,false,m_pUserData)
	void SetBuffer(char *pBuffer, long nBufferLength);
    // nastaveni callbackove funkce a pointeru na uzivatelska data
	void SetEndOfBufferCallback(COMPILERMEMORYINPUT_ENDOFBUFFER_CALLBACK pEndOfBufferCallback, 
		void *pUserData=NULL);

protected:
    // delka bufferu, aktualni pozice
	long m_nBufferLength, m_nPos;
    // buffer
	char *m_pBuffer;
    // uzivatelska data predavana callbackove funkci
	void *m_pUserData;
    // callbackova funkce (-vola prekladac)
	COMPILERMEMORYINPUT_ENDOFBUFFER_CALLBACK m_pOnEndOfBufferCallback;
};

#endif // !defined(AFX_COMPILERMEMORYINPUT_H__7C68B3C4_EC50_11D2_AE99_004F49068BD6__INCLUDED_)
