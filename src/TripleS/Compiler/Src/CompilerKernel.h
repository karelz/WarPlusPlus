/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace prekladace ve tride CCompilerKernel,
 * 
 ***********************************************************/

#if !defined(AFX_COMPILERKERNEL_H__8A3A618A_314C_11D3_AF09_004F49068BD6__INCLUDED_)
#define AFX_COMPILERKERNEL_H__8A3A618A_314C_11D3_AF09_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\Compiler.h"

#include "PCodeStringTable.h"
#include "Type.h"
#include "PCode.h"
#include "PNamespace.h"
#include "PExpr.h"
#include "PPreview.h"
#include "PCurrentUnit.h"
#include "PCurrentGlobalFunction.h"
#include "PCurrentStruct.h"

#ifdef _DEBUG
#include "DebugFrame.h"
#endif

// jinam to vlozit nejde - jsou tu cyklicky reference
#include "TripleS\CodeManager\CodeManagerForCompiler.h"

// GLOBALNI PROMENNE PREKLADACE

class CCompilerKernel;
class CPLex2ndPass;
class CPLex1stPass;

// GLOBALNI MAKRA
#define IsRegister(r)	((r)>=0)
#define SizeOfInstruction(i)   (sizeof(INSTRUCTION)+sizeof(REG)*g_aInstrInfo[i].m_nParamCount)

// class CCompilerKernel
// =====================
// V teto tride je implementovan cely prekladac. V jednom case je mozna jen
// jedna jeho instance (kvuli globalnimu ukazateli na ni).
// Je vytvaren v metode Create objektu CCompiler, ktery ho obaluje.
// Vice viz Compiler.doc.
class CCompilerKernel : public CObject  
{
public:
    // konstruktor - chce pointer na manazera kodu (pres nej se bude ukladat prelozeny kod)
	CCompilerKernel(CCodeManagerForCompiler *pCodeManagerForCompiler);
					
    // destruktor
	virtual ~CCompilerKernel();

    // Prvni pruchod prekladace - viz Compiler.doc.
	bool FirstPass();
    // Druhy pruchod prekladace - viz Compiler.doc.
	bool SecondPass();

    // Vypsani chybove hlasky na StdErr.
	void ErrorMessage(LPCTSTR strErrorMsgFormat, ...);
    // Vypsani warning hlasky na StdErr.
	void WarningMessage(int nLevel, LPCTSTR strWarningMsgFormat, ...);

    // Resetovani prekladace - nastaveni vstupu a vystupu, nastaveni ....
    // (vola se pred kazdym prekladem)
	void Reset(CCompilerInput *pStdIn, // standardni vstup - musi byt otevren
  			   CCompilerErrorOutput *pStdErr, // standardni chybovy vytup - musi byt otevren);
               bool bUpdateCodeOnSuccess,
               bool bDebugOutput, // ma-li se vypisovat debug = prelozeny vysledek v citelne forme
			   CCompilerErrorOutput *pDbgOut,  // debugovaci vystup (pokud predchozi
                                               // parametr je true - musi byt otevren
               bool bDebugParser  // debug parseru - debugovaci hlasky od bisona 
                                  // do zvlastniho okna (CDebugFrame).
               );

    // Resetovani cisla radky vstupu. Viz CCompiler::ResetLineNo.
    void ResetLineNo();

protected:
    // priznak, zda jiz bylo systemove namespace nacteno z konfiguracniho
    // souboru (tedy ze m_Namespace.Create probehlo v poradku).
	bool m_bNamespaceCreated;

public:
    // lexikalni analyzer prvniho pruchodu
    CPLex1stPass *m_pLex1stPass;

    // lexikalni analyzer druheho pruchodu
    CPLex2ndPass *m_pLex2ndPass;

    // metoda pro hlaseni chyb z parseru - 2.pruchod (preklad chyb)
    void ParserError(char *errstr);


public:
    // cislo radky, ktera je aktualne prekladana
	//long m_nLineNo;
    // pocet chyb a warningu
    int m_nErrorCount, m_nWarningCount;
    // priznak, zda prerusit kompilaci
	bool m_bStopCompiling;
    // priznak, zda se ma generovat kod (resp. zda se ma prelozeny kod
    // ukladat na vystup). Nastavuje se na false pri jakekoliv chybe v prekladane
    // jednotce/globalni fci. Pro dalsi jednotku/globalni funkci preklad zacina 
    // znova na true.
	bool m_bGenerCode;

    // standardni vstup
    CCompilerInput *m_pStdIn; 
    // standardni chybovy vystup
	CCompilerErrorOutput *m_pStdErr;
	// vystup prelozeneho kodu pujde pres Code Managera
    // debugovaci vystup
    CCompilerErrorOutput *m_pDbgOut;
    // ukazatel na manazera kodu, pres nej se bude ukladat prelozeny kod
	CCodeManagerForCompiler *m_pCodeManagerForCompiler;

// ZKOMPILOVANE VECI - cekaji na konec prekladu, kdyz je uzpesny, tak se uploadnou
    CPCurrentUnit *m_pCompiledUnits;
    CPCurrentGlobalFunction *m_pCompiledGlobalFunctions;
    CPCurrentStruct *m_pCompiledStructs;

    // priznak zda se se ma delat pri uspesnem prekladu update kodu
    // (Zda se maji binarky uploadovat do CodeManagera).
    bool m_bUpdateCodeOnSuccess;

    // priznak zda vypisovat obsah prelozenych jednotek a glob.funkci
    // na debugovaci vystup
	bool m_bDebugOutput;

    // Namespace prekladace. Viz CPNamespace.
	CPNamespace m_Namespace;

// POMOCNE PROMENNE pri prekladu (vlastne inherited artibuty gramatiky)
    // uroven zanoreni bloku { }
	int m_nLevel;
    // Aktualni tabulka stringu (prave prekladane jednotky nebo globaln funkce).
    // Platny je-li prekladac uvnitr nejake jednotky/glob.fce.
	CPCodeStringTable *m_pCurrentStringTable;	
    // Aktualni tabulka realnych cisel (prave prekladane jednotky nebo globalni funkce)
    // Platny je-li prekladac uvnitr nejake jednotky/glob.fce.
	CFloatTable *m_pCurrentFloatTable;
    // ukazatel na typ nacteny pri deklaraci lok.promennych nebo polozek jednotky
    // Je platny jen do redukce pravidla, ve kterem byl nastaven.
	CType *m_pCurrentType;  
    // Ukazatel na normalizovanou hlavicku reakce na zpravu (kontroluje se hlavicka
    // v kodu).
    // Platny je -li prekladac uvnitr nejake reakce na zpravu.
	CPNamespace::CEventHeader *m_pCurrentEventHeader;
    // Pocet zpracovanych argumentu funkce/metody/eventy.
	int m_nFormalParamNo;

	// class CArgInfo
    // ==============
    // Pomocna trida pro udrzovani informaci o argumentech 
    // prave prekladanych volani funkci/metod/eventu.
	class CArgInfo
	{
	public:
		CArgInfo();
		virtual ~CArgInfo() {}
	public:
        // ukazatel na pole typu argumentu
		CArray<CType, CType&> *m_pArgArray;
        // pocet zpracovanych argumentu, celkovy pocet
		int m_nCurrentArgNo, m_nArgCount;
        // priznak, zda trida obsahuje platne informace
        // (napr. pro neznamou funkci se do seznamu prida neplatna instance)
		bool m_bValid;

        // Nastaveni ukazatele na pole.
		void Set(CArray<CType, CType&>* pArgArray, int nArgCount);
        // Ziskani dalsiho argumentu ke zpracovani (vraci 
        // m_pArgArray->GetAt(m_nCurrentArgNo) a inkrementuje
        // m_nCurrentArgNo. Pri chybe vraci NULL.
		bool GetNext(CType &Type);
	};

    // Spojak (zasobnik) informaci o argumentech prave prekladanych
    // volani metod/funkci/eventu.
	CList<CArgInfo*, CArgInfo*> m_lArgInfoList;
    // Uvolneni seznamu m_lArgInfoList.
	void FreeArgInfo();
	
public:
    // ukazatel na prave prekladanou jednotku
	CPCurrentUnit *m_pCurrentUnit;
    // ukazatel na prave prekladanou metodu
	CPCurrentUnit::CMethod *m_pCurrentMethod;
    // ukazatel na prave prekladanou reakci na zpravu
	CPCurrentUnit::CEvent *m_pCurrentEvent;
    // ukazatel na prave prekladanou globalni funkci
	CPCurrentGlobalFunction *m_pCurrentGlobalFunction;
    // Ukazatel na prave prekladanou strukturu
    CPCurrentStruct *m_pCurrentStruct;

    // ukazatel na prave prekladanou metodu/globalni funkci/eventu 
    // (spolecny predek - pro spolecne veci jako napr. pridavani 
    // lokalnich promennych a argumentu napr.)
	CPCurrentUnit::CMethodEvent *m_pCurrentMEG;  // current method, event or global function

    // ukazatel na informace ziskane pri prvnim pruchodu prekladace o 
    // prave prekladane jednotce
	CPNamespace::CUnit *m_pCurrentUnitPreview;

// PRACE S REGISTRY
protected:
    // prvni volny registr
	REG m_nFirstFreeRegister;
    // rezervovane registry
	CArray<bool, bool> m_aReservedRegisters;
public:
    // ziskani volneho registru
	REG GetReg();
    // rezervace registru (pouziva se pri 'switch', 'for' cyklu pro mnozinu ...)
	bool ReserveRegister(REG nReg);
    // zruseni rezervace registru
	void EndRegisterReservation(REG nReg);
    // Uvolenni registru (vsech krome tech, ktere byly rezervovany).
	void ResetRegisters()	{ m_nFirstFreeRegister=1; }
	
///////////
    // Pridani lokalni promenne do aktualni funkce/metody/eventy.
	bool AddLocalVariable(CPExpr &Var, CPNamespace::CVariable **ppVar=NULL);
    // Pridani argumentu do aktualni funkce/metody/eventy.
	bool AddArgument(CPExpr &Arg, CPNamespace::CVariable **ppVar=NULL);
    // Nalezeni spolecneho typu, na ktery jdou Type1 i Type2 predefinovat.
	bool FindResType(CType &Type1, CType &Type2, CType &ResType);

protected:
    // ulozeni informaci o prvnim pruchodu
	CPPreview m_Preview;
    
    // Prvni pruchod - rucne napsany parser.
	void DoFirstPass();

    // pridani globalni funkce
	CPCurrentGlobalFunction* AddGlobalFunction(CString strName, CType &ReturnDataType);

    // pridani struktury
    CPCurrentStruct* AddStruct(CString strName);


// SWITCH STATEMENT
    
    // SSwitchCase 
    // ===========
    // Struktura pro  udrzovani informace o jedne case vetvi switch statementu.
	struct tagSwitchCase
	{
	public:
        // konstanta pro kterou vetev plati
		CPLexVal m_LexVal;
        // cislo bloku, ktery vetvi nalezi
		int m_nBlockNo;
	};
	typedef struct tagSwitchCase SSwitchCase;

    // class CSwitchStatement
    // ======================
    // Trida udrzujici informace o jednom switch statementu
	class CSwitchStatement
	{
	public:
		CSwitchStatement(int nLevel);  // je-li vyraz ve switch (tento) error
		CSwitchStatement(int nLevel, REG nExprReg, CType &DataType, CPCode &ExprCode);
		~CSwitchStatement();
	public:
        // priznak, zda jsou informace platne (tedy zda nedoslo k chybe)
		bool m_bValid;

        // uroven zanoreni switch statementu v blocich {}
		int m_nLevel;
        // pocet vetvi
		int m_nCasesCount;
        // pocet bloku
		int m_nBlockCount;

        // Registr s vysledkem vyrazu, podle ktereho se rozskokuje.
        // Behem prekladu switch statementu je zarezervovany.
		REG m_nExprReg;
        // datovy typ vyrazu
		CType m_DataType;
        // kod, ktery vyraz spocita
		CPCode m_ExprCode;

        // switch statement ma vetev 'default' (to je posledni blok)
		bool m_bHasDefault;
        // vetve (cases)
		CArray<SSwitchCase, SSwitchCase&> m_aCases;
        // bloky
		CArray<CPCode, CPCode&> m_aBlocks;

        // Pridani vetve - prirazi se ji blok cislo m_nBlockCount,
        // tedy nasledujici blok, ktery bude pridan.
		void AddCase(CPLexVal &LexVal);
        // Pridani bloku.
		void AddBlock(CPCode &Code);
	};

    // Spojak (zasobnik) switch statementu (mohou byt vnorene)
	CList<CSwitchStatement*, CSwitchStatement*> m_lSwitches;
    // uvolneni obsahu predchoziho zasobniku
	void FreeSwitches();

// SENDTO statement
    // priznak, zda se preklada 'sendto' statement (tedy nekontroluji se hlavicky
    // volani)
	bool m_bSendToStatement;
	
// ACTIONS
// Metody volane z kodu vygenerovaneho bisonem - reakce na redukce pravidel atp.
// Kde se metody volaji viz komentare pred jejich implementaci v CCompilerKernel.cpp.
public:
    void OnThis(CPExpr &Res);
	void OnSetForEnd(CPExpr &Res, CPExpr &Var, CPExpr &Set, CPExpr &Block);
	void OnSetForBegin(CPExpr &Set);
	void OnReferenceType(CPExpr &Res, CPExpr &Ident);
	void OnIn(CPExpr &Res, CPExpr &Var, CPExpr &Set);
	void OnLocalVarDeclConcat(CPExpr &Res, CPExpr &Var1, CPExpr &Var2);
	void OnExceptionStmt(CPExpr &Res, CPExpr &TryBlock, CPExpr &CatchBlock);
	void OnEventCallEndsError(CPExpr &Res);
	void OnEventCallEnds(CPExpr &Res, CPExpr &Ident, CPExpr &Primary, CPExpr &Begin, CPExpr &Args);
	void OnEventCallBegins(CPExpr &Res, CPExpr &Primary, CPExpr &Ident);
	void OnSendToEventList(CPExpr &Res, CPExpr &List, CPExpr &SentToEvent);
	void OnSendToEventEndsError(CPExpr &Res);
	void OnSendToEventEnds(CPExpr &Res, CPExpr &Ident, CPExpr &Begin, CPExpr &Args);
	void OnSendToEventBegins(CPExpr &Res, CPExpr &Ident);
	void OnSendToPrimaryExprList(CPExpr &Res, CPExpr &List, CPExpr &Expr);
	void OnSendToPrimaryExpr(CPExpr &Res, CPExpr &Expr);
	void OnSendToEnds(CPExpr &Res, CPExpr &PrimaryExprList, CPExpr &Options, CPExpr &EventList);
	void OnSendToBegins();
	void OnSwitchDefault();
	void OnSwitchCase(CPExpr &Expr);
	void OnSwitchBlockStmtS(CPExpr &BlockStmtS);
	void OnSwitchEnds(CPExpr &Res);
	void OnSwitchBegins(CPExpr &Expr);
	void OnForStatement(CPExpr &Res, CPExpr &Init, CPExpr &BoolExpr, CPExpr &Update, CPExpr &Stat);
	void OnExprList(CPExpr &Res, CPExpr &List, CPExpr &Expr);
	void OnContinueStatement(CPExpr &Res);
	void OnBreakStatement(CPExpr &Res);
	void OnReturnStatementError(CPExpr &Res);
	void OnReturnStatementVoid(CPExpr &Res);
	void OnReturnStatementValue(CPExpr &Res, CPExpr &Expr);
	void OnBoolExpr(CPExpr &Res, CPExpr &Expr);
	void OnWhileStatement(CPExpr &Res, CPExpr &Expr, CPExpr &Stat);
	void OnDoStatement(CPExpr &Res, CPExpr &Stat, CPExpr &Expr);
	void OnIfStatement(CPExpr &Res, CPExpr &Expr, CPExpr &StatThen, CPExpr &StatElse);
	void OnIfStatement(CPExpr &Res, CPExpr &Expr, CPExpr &Stat);
	void OnAndOrExpr(CPExpr &Res, CPExpr &Expr1, CPExpr &Expr2, char cWhich);
	void OnInstanceOf(CPExpr &Res, CPExpr &Expr, CPExpr &Type);
	void OnRelatEqualExpr(CPExpr &Res, CPExpr &Expr1, CPExpr &Expr2, char cWhich);
	void OnBinaryOperation(CPExpr &Res, CPExpr &Expr1, CPExpr &Expr2, char cWhich);
	void OnAssign(CPExpr &Res, CPExpr &Primary, CPExpr &Oper, CPExpr &Expr);
	void OnUnaryExpr(CPExpr &Res, CPExpr &Expr, char cWhich);
	void OnPostfixExpr(CPExpr &Res, CPExpr &Expr, char cWhich);
	void OnBlockStatementConcat(CPExpr &Res, CPExpr &BlockStmtS, CPExpr &BlockStmt);
	void OnMethodInvocSuperEnds(CPExpr &Res, CPExpr &Ident, CPExpr &Begin, CPExpr &Args);
	void OnMethodInvocIdentEnds(CPExpr &Res,  CPExpr &Ident, CPExpr &Begin, CPExpr &Args);
	void OnMethodInvocPrimaryIdentEnds(CPExpr &Res, CPExpr &Primary, CPExpr &Ident, CPExpr &Begin, CPExpr &Args);
	void OnMethodInvocEndsError(CPExpr &Res);
	void OnArgument(CPExpr &Res, CPExpr &Expr);
	void OnArgumentNext(CPExpr &Res, CPExpr &Args, CPExpr& Expr);
	void OnArgumentFirst(CPExpr &Res, CPExpr& Expr);
	void OnEmptySuper(CPExpr &Res);
	void OnMethodInvocSuperBegins(CPExpr &Res, CPExpr &Ident);
	void OnMethodInvocPrimaryIdentBegins(CPExpr &Res, CPExpr &Primary, CPExpr &Identifier);
	void OnMethodInvocIdentBegins(CPExpr &Res, CPExpr &Ident);
	void OnEndOfStatement();
	void OnArrayAccessError(CPExpr &Res, CPExpr &Primary);
	void OnArrayAccess(CPExpr &Res, CPExpr &Primary, CPExpr &Expr);
	void OnFieldAccess(CPExpr &Res, CPExpr &Primary, CPExpr &Ident);
	void OnLocalVarOrMember(CPExpr &Res, CPExpr &Ident);
	void OnGlobalFunctionEnds(CPExpr &Block);
	bool OnGlobalFunctionBegins(CPExpr& RetType, CPExpr &Name);
	void OnVarDeclInit(CPExpr &Res, CPExpr &Expr);
	void OnVarDeclInitDone(CPExpr &Res, CPExpr &Variable, CPExpr &Expr);
	void OnArrayVarDecl(CPExpr &Res, CPExpr &Var);
	void OnArrayVarDeclMore(CPExpr &Res, CPExpr &Var, CPExpr &Size);
	void OnArrayVarDeclIdent(CPExpr &Res, CPExpr &Ident, CPExpr &Size);
	void OnSetVarDecl(CPExpr &Res, CPExpr &Ident);
    void OnSimpleVarDecl(CPExpr &Res, CPExpr &Ident);
	void OnFieldVarDecl(CPExpr &Member);
	void OnFieldVarDeclIdentifier(CPExpr &Res, CPExpr &Ident);
	void OnFieldVarDeclArrayIdent(CPExpr &Res, CPExpr &Ident, CPExpr &Size);
	void OnFieldVarDeclArrayMore(CPExpr &Res, CPExpr &FieldVarDecl, CPExpr &Size);
	void OnFieldVarDeclSet(CPExpr &Res, CPExpr &Ident);
	void OnBlockEnds();
	void OnBlockBegins();
	void OnParamDeclIdentifier(CPExpr &Res, CPExpr &Ident);
	void OnParamDeclArray(CPExpr &Res, CPExpr &ParamDecl);
    void OnParamDeclSet(CPExpr &Res, CPExpr &Ident);
	bool OnFormalParam(CPExpr &Name);
	void OnMethodEnds(CPExpr &Block);
	bool OnMethodBegins(CPExpr &Name);
	void OnEventEnds(CPExpr &Block);
	bool OnEventBegins(CPExpr &Name, bool bIsCommand);
	void OnDestructorBegins();
	void OnDestructorEnds(CPExpr &Body);
	void OnConstructorBegins();
	void OnConstructorEnds(CPExpr &Body);
	void OnConstantExpr(CPExpr &Res, CPExpr& Expr);
	bool OnUnitBegin(CPExpr& Name, CPExpr& Extends);
	void OnUnitEnd();

	// pomocne funkce
	bool LoadDataType( CType &Type);

    // DEBUGOVACI VYPIS bisona  do CDebugFrame
public:
    // vypsani debugovaci hlasky
    void DebugOut(LPCTSTR strFormat, ...);
#ifdef _DEBUG
    // pauzovani prekladu (pri vypisu do debug. okna CDebugFrame,
    // ceka na ENTER nebo ESC)
    void Pause();
#endif

protected:
#ifdef _DEBUG
    // debugovaci okno bisona - frame
    CDebugFrame *m_pwndDebugFrame;
    // fonto tohoto okna
    CFont m_DebugFont;
#endif
public:    
	void OnDupExpr(CPExpr &Res, CPExpr &Ob);
	void OnStructEnd();
	bool OnStructBegin(CPExpr &Ident);
};

#endif // !defined(AFX_COMPILERKERNEL_H__8A3A618A_314C_11D3_AF09_004F49068BD6__INCLUDED_)
