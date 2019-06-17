/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac, Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CFloatTable reprezentujici 
 *          tabulku floatu pouzitych prekladanou jednotkou/glob.fci
 * 
 ***********************************************************/

#if !defined(AFX_FLOATTABLE_H__4632EE37_4F3A_11D3_A506_00A0C970CB8E__INCLUDED_)
#define AFX_FLOATTABLE_H__4632EE37_4F3A_11D3_A506_00A0C970CB8E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCompilerErrorOutput;
class CCompilerOutput;

// class CFloatTable
// =================
// Trida pro uchovavani konstantnich realnych cisel, ktere prekladana 
// jednotka/globalni funkce pouziva (konstantni realne cislo se nevejde
// do registru, takze se (podobne jako stringy) ukladaji do tabulek.
//
// Typ v jazyce se sice jmenuje float, ale interne je to double (8 bytu)
//
class CFloatTable  
{
public:
	CFloatTable();
	virtual ~CFloatTable();

public:
    // pridani realniho cisla
	int Add(double fFloat);
    
    // vyprazdneni tabulky
	void Clean();

    // vytvoreni tabulky dle jine (kopie)
    void UpdateFrom( CFloatTable *pAnotherOne);

    // operator []
    double operator[] (int nIndex);

    // operator ==
    bool operator== (CFloatTable &anotherOne);

    // Ulozeni tabulky na vystup.
	void Save(CCompilerOutput* pOut);
    // Nacteni tabulky.
	void Load(CFile &file);

    // Debugovaci vypis obsahu tabulky.
	void DebugWrite(CCompilerErrorOutput* pOut);

protected:
    // samotna tabulka (ID realneho cisla = index do tohoto pole)
	CArray<double, double> m_aFloats;
    // pocet polozek v tabulce
	int m_nCount;
};

#endif // !defined(AFX_FLOATTABLE_H__4632EE37_4F3A_11D3_A506_00A0C970CB8E__INCLUDED_)
