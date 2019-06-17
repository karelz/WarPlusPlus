/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CPExpr reprezentujici 
 *          semantickou hodnotu kazdeho terminalu a neterminalu
 * 
 ***********************************************************/


#if !defined(AFX_EXPR_H__62DCE781_3F92_11D3_A4B2_00A0C970CB8E__INCLUDED_)
#define AFX_EXPR_H__62DCE781_3F92_11D3_A4B2_00A0C970CB8E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// class CPLexVal
// =============
// Tato trida slouzi pro uchovavani konstant nactenych pri prekladu
// zdrojoveho souboru (lexikalni hodnota tokenu). Je polozkou tridy 
// CPExpr (semanticka hodnota tokenu).
class CPLexVal
{
public:
	CPLexVal();	
	CPLexVal(CPLexVal &anotherOne);
	~CPLexVal();	
	operator=(CPLexVal &anotherOne);

    // "typy" konstant
	enum tagLexValType { LEXVAL_ERROR, LEXVAL_NONE, LEXVAL_INT, LEXVAL_FLOAT, 
        LEXVAL_BOOL, LEXVAL_CHAR, LEXVAL_STRING };
	typedef enum tagLexValType ELexValType;

    // typ konstanty
	ELexValType m_nType;

    // hodnota
	union
	{
		long m_lInt;
		float m_fFloat;
		bool m_bBool;
		char m_chChar;
		char *m_lpszString;
	};

    // Tato metoda vraci nazev typu, ktereho je konstanta (tedy napr. int, string..)
	CString GetString();

    // nastaveni hodnoty konstanty
	void SetInt(int lInt);
	void SetFloat(float fFloat);
	void SetBool(bool bBool);
	void SetChar(char chChar);
	void SetString(LPCTSTR lpszString);	
	void SetError();
	void SetNone();

    // dotaz na typ konstanty
	bool IsError()	{	return m_nType==LEXVAL_ERROR;	}
	bool IsInt()	{	return m_nType==LEXVAL_INT;		}
	bool IsFloat()	{	return m_nType==LEXVAL_FLOAT;	}
	bool IsBool()	{	return m_nType==LEXVAL_BOOL;	}
	bool IsChar()	{	return m_nType==LEXVAL_CHAR;	}
	bool IsString()	{	return m_nType==LEXVAL_STRING;	}

    // dotaz je-li konstanta daneho (skriptovaciho) typu
	bool IsType(CType&Type);
};

// class CPExpr
// ===========
// Tato trida slouzi k uchovani semanticke hodnoty kazdeho tokenu.
// Semantickou hodnotou muze byt konstanta, nejaky identifikator, 
// typ, ale i kus kodu, pripadne 'chyba'.
class CPExpr : public CObject
{
public:
	CPExpr();
	CPExpr(CPExpr &anotherOne);
	virtual ~CPExpr();
	operator=(CPExpr &anotherOne);

    // "typy" semantickych hodnot
	enum tagExprType { E_NONE=0, E_ERROR=1, E_CODE=2, E_CONSTANT=3, E_TYPE=4, E_DECLAR=5};
	typedef enum tagExprType EExprType;
	
    // typ semanticke hodnoty
	EExprType m_nType;

    // lexikalni hodnota - platne pro m_nType==E_CONSTANT
	CPLexVal m_LexVal;	
    // identifikator - platne pro m_nType==E_DECLAR
	CString m_strIdentifier;
    // datovy typ - platne pro m_nType==E_CODE, E_TYPE, E_DECLAR, E_CONSTANT
	CType m_DataType;	
    // registr s vysledkem kodu - platne pro m_nType==E_CODE
	REG m_nReg;			
    // kod - platne pro m_nType==E_CODE
	CPCode m_Code;		
    // typ l-value - platne pro m_nType==E_CODE
	bool m_bLValue;		
	
    // "typy" l-value - platne pro m_nType==E_CODE
	enum tagLValueType
		{  LVALUE_PRIMARY,	// Lokalni promenna nebo argument
							// m_nReg je cislo registru, m_Code prazdny

		   LVALUE_UMEMBER,  // 1) Member aktualni prekladane unity
							//	  nReg=0, m_lNum=jmeno (odkaz do stringtable unity)
							// 2) Member jine unity
							//	  nReg=registr, v nemz je reference na tu unitu
							//    m_lNum=jmeno (odkaz do stringtable _aktualni_ unity)

           LVALUE_SMEMBER,  //    Member struktury
							//	  nReg=registr, v nemz je reference na strukturu
							//    m_lNum=member jmeno (odkaz do stringtable _aktualni_ unity)

		   LVALUE_OMEMBER,  // Member systemoveho objektu
							//	  nReg=registr, v nemz je reference na ten objekt
							//    m_lNum=ID polozky
							
		   LVALUE_INDEX };  // Indexovany pristup
							// m_nReg=reference na objekt typu String nebo Array 
							// m_lNum je index v poli/stringu
	typedef enum tagLValueType ELValueType;

    // typ l-value
	ELValueType m_nLValueType;
    // pomozna polozka pro dodacelne informace k l-value
	long m_lNum;

    // prekladac, ke kteremu patri
    CCompilerKernel *m_pCompilerKrnl;

public:
    // Presun konstanty do registru (E_CONSTANT->E_CODE s instrukci MOV_CXXX)
	void MoveConstantToRegister(REG nReg);	
    // Presune slozitou l-value do registru (LVALUE_??->LVALUE_PRIMARY).
	void Load(); 
    // Ulozi na zadanou l-value hodnotu ze sveho registru.
	void Store(ELValueType nLValueType, REG nReg, long lNum);
	// Ulozi na zadanou l-value null.
	void StoreNull();

	// Prida instrukci a zmeni registr m_nReg na to, co je v operandu nOp1.
	void AddInstructionChangeRegister(EInstruction eInstr, REG nOp1=NO_REG, 
		REG nOp2=NO_REG, REG nOp3=NO_REG); 

    // Konvertuje semantickou na zadany typ.
	bool ConvertToType(CType &DestType);
    // Konverze znaku na string (dela se ve specialnich pripadech, neni implicitni).
	void ConvertCharToString();

    // Konverze l-value na r-value (-->Load).
	void ConvertToRValue();

    // Nastavi defaultni neskodne hodnoty (vola se v konstruktorech).
	void SetDefaults();
	void SetNone()		{	SetDefaults();  }

    // Nastavi semantickou hodnotu na 'novy', tj. prazdny kod (pokud tam nejaky byl, maze ho)
	void SetNewCode()	{	SetDefaults();  m_nType=E_CODE;	}  
    // Nemaze kod, pouze nastavuje registr, datovy typ a l-value
	void SetCode(REG nReg, CType&DataType, bool bLValue=true,  
		ELValueType nLValueType=LVALUE_PRIMARY, long lNum=0);

    // Nastaveni semanticke hodnoty na E_CONSTANT dle daneho typu.
	void SetConstant(char chChar, CCompilerKernel *pCompiler);
	void SetConstant(int nInt, CCompilerKernel *pCompiler);
	void SetConstant(float fFloat, CCompilerKernel *pCompiler);
	void SetConstant(bool bBool, CCompilerKernel *pCompiler);
	void SetConstant(LPCTSTR lpszString, CCompilerKernel *pCompiler);

    // Nastaveni chybove konstanty.
	void SetErrorConstant()	{ SetDefaults(); m_LexVal.SetError(); m_DataType.Set(m_LexVal); }

    // Nastaveni semanticke hodnoty na E_TYPE.
	void SetType(CCompilerKernel *pCompiler)		{ SetDefaults(); m_nType=E_TYPE; m_pCompilerKrnl = pCompiler; }
    void SetType(EDataType nType, CCompilerKernel *pCompiler);

    // Nastaveni semanticke hodnoty na E_DECLAR
	void SetDeclar(CString &strIdentifier, CCompilerKernel *pCompiler);

    // Nastaveni semanticke hodnoty na E_ERROR
	void SetError()		{   SetDefaults();  m_nType=E_ERROR;		}

    // Dotaz na typ semanticke hodnoty
	bool IsNone()		{	return m_nType==E_NONE;		}
	bool IsCode()		{	return m_nType==E_CODE;		}
	bool IsConstant()	{	return m_nType==E_CONSTANT;	}
	bool IsType()		{	return m_nType==E_TYPE;		}
	bool IsDeclar()		{	return m_nType==E_DECLAR;		}
	bool IsError()		{	return m_nType==E_ERROR;		}

    bool IsNull()       {   return IsConstant() && m_LexVal.IsInt() && m_LexVal.m_lInt == 0; }
};

#endif // !defined(AFX_EXPR_H__62DCE781_3F92_11D3_A4B2_00A0C970CB8E__INCLUDED_)
