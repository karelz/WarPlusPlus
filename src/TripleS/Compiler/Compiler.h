/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CCompiler, trida obalujici samotny kompilator
 *          implementovany ve tride CCompilerKernel
 * 
 ***********************************************************/

#if !defined(AFX_COMPILER_H__8A3A6189_314C_11D3_AF09_004F49068BD6__INCLUDED_)
#define AFX_COMPILER_H__8A3A6189_314C_11D3_AF09_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCompilerKernel;
class CCodeManagerForCompiler;

#include "Src\CompilerKernel.h"

class CCompilerInput;

// class CCompiler
// ===============
// Trida CCompiler obaluje tridu CCompilerKernel, ve ktere je implementovan 
// cely kompilator.
class CCompiler : public CObject
{
public:
	CCompiler();
	virtual ~CCompiler();

public:
    // vytvoreni prekladace ( -> vytvoreni jadra m_pCompilerKrnl)
	bool Create(CCodeManagerForCompiler *pManager);  // pointer na manazera kodu (pres nej se bude ukladat
												     // prelozeny kod)
	
    // zruseni prekladace
	void Delete();
    
    // Preklad. Parametrem je priznak, zda se ma delat pri uspesnem prekladu update kodu
    // (Zda se maji binarky uploadovat do CodeManagera).
	bool Compile(CCompilerInput *pStdIn, // standardni vstup - musi byt otevren
				 CCompilerErrorOutput *pStdErr, // standardni chybovy vystup - musi byt otevren
                 bool bUpdateCodeOnSuccess,   // ma-li se provadet update pri uspesnem prekladu
				 bool bDebugOutput=false, // ma-li se vypisovat debug = prelozene
                                          // jednotky a globalnich funkce
				 CCompilerErrorOutput *pDbgOut=NULL, // debugovaci vystup (pokud predchozi
                                                     // parametr je true - musi byt otevren
				 bool bDebugParser=false // debug parseru - debugovaci hlasky od bisona 
                                         // do zvlastniho okna (CDebugFrame).
                 );

    // Resetovani cisla radky vstupu. 
    // Pouziva se pri prekladu vice souboru najednou, kdy do kompilatoru jde stream dat,
    // ktery odpovida konkatenaci vsech souboru, co se prekladaji. 
    // Metoda ResetLineNo se vola pro resetovani cisla radky na 1 pri prechodu do dalsiho souboru.
    // Dulezite pouze pro spravne vypisovani chybovych hlasek.
    void ResetLineNo();

    void ChangeInputNameAndLine(CString &strNewInputName, int nLineNo = 1);

protected:
    // objekt implementujici jadro prekladace
	CCompilerKernel* m_pCompilerKrnl;

    // 
    enum { NOT_COMPILING, FIRST_PASS, SECOND_PASS} m_eCompilerPhase;

#ifdef _DEBUG
public:
	virtual void AssertValid() const;
#endif
};

#endif // !defined(AFX_COMPILER_H__8A3A6189_314C_11D3_AF09_004F49068BD6__INCLUDED_)
