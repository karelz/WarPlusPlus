/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace trid CPCode a CPCode::CInstruction, tridy pro 
 *          reprezentaci instrukci a kusu kodu
 * 
 ***********************************************************/

#if !defined(AFX_CODE_H__20CD7C06_4013_11D3_A4B3_00A0C970CB8E__INCLUDED_)
#define AFX_CODE_H__20CD7C06_4013_11D3_A4B3_00A0C970CB8E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// class CPCode
// ===========
// Trida uchovavajici prelozeny kod.
class CPCode  
{
public:
    // class CInstruction
    // ===================
    // Trida uchovavajici jednu instrukci a jeji parametry.
    // Instrukce si pocita reference, kolikratje na ni odkazovano,
    // kdyz se counter vynuluje, smaze se.
    // Kod je vlastne spojak trid CInstruction a kdyz se kopiruje, tak
    // si novy kod pouze zkopiruje pointer na zacatek (a na konec) a prida
    // prislusne reference.
	class CInstruction
	{
	public:
		CInstruction(EInstruction eInstr, REG nOp0=NO_REG, REG nOp1=NO_REG, REG nOp2=NO_REG);
		virtual ~CInstruction();
	public:
        // kod instrukce
		EInstruction m_eInstr;
        // operandy
		REG m_nOp[3];
		
        // ukazatel na dalsi instrukci ve spojaku
		CInstruction* m_pNext;

        // 'pripojeni' instrukce (zvyseni couteru referenci)
		CInstruction* Attach();
        // 'odpojeni' instrukce (snizeni couteru referenci)
		void Detach();
	
    protected:
        // couter referenci
		int m_nRefCount;
	};

// class CPCode
public:
	CPCode();
	virtual ~CPCode();
	operator=(CPCode &anotherOne);
public:
    // pridani instrukce k kodu
	void AddInstruction(EInstruction nInst, REG nOp0=NO_REG, REG nOp1=NO_REG, REG nOp2=NO_REG);
	void AddInstruction(EInstruction nInst, REG nOp[]);
    
    // pridani zadaneho kodu za svuj kod
	void AddCode(CPCode &Addin);
    
    // vytvoreni noveho kodu spojenim ze dvou zadanych casti
	void CreateFromConcatenation(CPCode &First, CPCode &Second);

    // 'vyprazdneni' kodu
	void Empty();
    // dotaz na neprazdnost kodu
	inline bool IsEmpty()	{ return m_nInstrCount==0; }

    // Vraci pocet instrukci v kodu.
    inline int GetInstructionCount()	{ return m_nInstrCount; }
    // Vraci delku kodu v bytech.
    long GetLengthInBytes();
	
    // PRUCHOD KODEM zvenku
    // Vraci prvni instrukci.
	inline CInstruction* GetFirstInstruction()	{ return m_pFirstInstr; }
    // Vraci instrukci nasledujici po zadane.
	inline CInstruction* GetNextInstruction(CInstruction* pCurrent) { ASSERT(pCurrent!=NULL); return pCurrent->m_pNext; }
    // Vraci posledni instrukci.
	inline CInstruction* GetLastInstruction() 	{ return m_pLastInstr; }

protected:
    // ukazatel na prvni a posledni prvek ve spojaku instrukci
	CInstruction *m_pFirstInstr, *m_pLastInstr;
    // pocet instrukci ve spojaku
	long m_nInstrCount;
    // delka kodu v bytech
    long m_nLengthInBytes;

public:
	// priznak zda dany blok kodu konci vzdy return;
	bool m_bReturns;

public:
    // ulozeni kodu na vystup
	void Save(CCompilerOutput *pOut);

    // debugovaci vypis kodu
	void DebugWrite(CCompilerErrorOutput *pOut);
};

#endif // !defined(AFX_CODE_H__20CD7C06_4013_11D3_A4B3_00A0C970CB8E__INCLUDED_)
