/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CPNamespace, ktera obsahuje znama
 *          jmena objektu/promennych/glob.fci, hlavicky eventu atp.
 * 
 ***********************************************************/

#if !defined(AFX_NAMESPACE_H__405B2EC6_433E_11D3_A4B6_00A0C970CB8E__INCLUDED_)
#define AFX_NAMESPACE_H__405B2EC6_433E_11D3_A4B6_00A0C970CB8E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPLexNsp;
class CPPreview;

// class CPNamespace
// ================
// Trida udrzujici vsechny informace o globalni funkcich, jednotkach a objektech 
// nutne pro preklad (hlavicky, metody, member polozky, typy...), informace o
// systemovych objektech a globalnich funkcich, aktualni lokalni promenne,
// normanizovane hlavicky beznych zprav, globalni tabulku stringu pro preklad atp.
//
// Identifikatory a  hlavicky globalnich metod se nacitaji z konfiguracniho
// souboru, jehoz jmeno se definuje direktivou NAMESPACE_FILE (viz CICommon.h). 
// 
// Obsahuje podtridy CSysObjectMember, CSysObjectMethod, CSysObject, CUnitMember, 
// CUnitMethod, CUnit, CEventHeader, CVariable a CUnitObjsType.
class CPNamespace  :  public CObject
{
public:
    // class CSysObjectMember
    // ======================
    // Souzi pro uchovani informaci o polozce systemoveho objektu.
	class CSysObjectMember
	{
	public:
		CSysObjectMember(long nID, CStringTableItem *stiName, CType &DataType);
		virtual ~CSysObjectMember();

    public:
        // ID polozky - unikatni v ramci objektu
		long m_nID;
        // jmeno polozky
		CStringTableItem *m_stiName;
        // typ
		CType m_DataType;

        // namespace

	};

    // class CSysObjectMethod
    // ======================
    // Souzi pro uchovani informaci o metode systemoveho objektu.
	class CSysObjectMethod
	{
	public:
		CSysObjectMethod(long nID, CStringTableItem *stiName, CType &ReturnDataType);
		virtual ~CSysObjectMethod();
	public:
        // ID metody - unikatni v ramci objektu
		long m_nID;
        // jmeno metody
		CStringTableItem *m_stiName;
        // navratovy typ
		CType m_ReturnDataType;
        // typy argumentu
		CArray<CType, CType&> m_aArgs;
    public:
        // Pridava argument (na konec za vsechny ostatni - typ se urci dle 
        // predavaneho stringu pomoci CType::Set(strType), nemusi byt definovany).
		void AddArgument(CPNamespace *pNamespace, CString &strType, int nArrayLevel, CPPreview *pPreview);
	};

    // class CSysObject
    // ================
    // Souzi pro uchovani informaci o systemovem objektu.
	class CSysObject
	{
	public:
		CSysObject(CPNamespace *pNamespace, CStringTableItem *stiName, bool bCanCreateInstance);
		virtual ~CSysObject();	
	public:
        // jmeno objektu
		CStringTableItem *m_stiName;
        // metody 
		CTypedPtrMap<CMapStringToPtr, CString, CSysObjectMethod*> m_apMethods;
        // polozky
		CTypedPtrMap<CMapStringToPtr, CString, CSysObjectMember*> m_apMembers;
		// priznak, zda muze uzivatel vytvaret instance tohoto objektu
		bool m_bCanCreateInstance;

        // namespace
        CPNamespace *m_pNamespace; 

        // velikost hasovacich tabulek pro metody a polozky
		enum { MEMBERS_HASHTABLE_SIZE=13, METHODS_HASHTABLE_SIZE=21 };

        // ziskani metody objektu dle ID
		CSysObjectMethod* GetMethodByID(long nID);
        // ziskani polozky objektu dle ID
		CSysObjectMember* GetMemberByID(long nID);
        // ziskani metody objektu dle jmena
		CSysObjectMethod* GetMethodByName(CString &strName);
        // ziskani polozky objektu dle jmena
		CSysObjectMember* GetMemberByName(CString &strName);

        // pridani polozky
		CSysObjectMember* AddMember(long nID, CString &strName, CString &strType, int nArrayLevel, CPPreview *pPreview);
        // pridani metody
		CSysObjectMethod* AddMethod(long nID, CString &strName, CString &strReturnType, int nArrayLevel, CPPreview *pPreview);
	};

    // class CUnitMember
    // =================
    // Souzi pro uchovani informaci o polozce jednotky.
	class CUnitMember
	{
	public:
		CUnitMember(CStringTableItem *stiName, CType &DataType);
		virtual ~CUnitMember();
	public:
        // jmeno
		CStringTableItem *m_stiName;
        // datovy typ
		CType m_DataType;
	};

	// class CUnitMethod
    // =================
    // Souzi pro uchovani informaci o metode jednotky.
	class CUnitMethod
	{
	public:
		CUnitMethod(CStringTableItem *stiName, CType &ReturnDataType);
		virtual ~CUnitMethod();
    protected:
        CUnitMethod();
	public:
        // jmeno
		CStringTableItem *m_stiName;
        // navratovy datovy typ
		CType m_ReturnDataType;
        // typy argumentu
		CArray<CType, CType&> m_aArgs;

        // pridani argumentu - dle datovaho typu
		void AddArgument(CPNamespace *pNamespace, CString &strType, int nArrayLevel, CPPreview *pPreview);
        // pridani argumentu - dle stringu (CType::Set(CString))
		void AddArgument(CType &DataType);
	};

	// class CUnit
    // =================
    // Souzi pro uchovani informaci o jednotce.
	class CUnit
	{
	public:
        // normalni konstruktor pro prekladane jednotky
		CUnit(CPNamespace *pNamespace, CStringTableItem *stiName, CStringTableItem *stiParent);
        // konstruktor bez parametru se pouziva, kdyz bude jednotka natazena
        // od code managera
        CUnit(CPNamespace *pNamespace);
		virtual ~CUnit();	
	public:
        // jmeno jednotky
		CStringTableItem *m_stiName;
        // jmeno predka 
		CStringTableItem *m_stiParent;
        // metody
		CTypedPtrMap<CMapStringToPtr, CString, CUnitMethod*> m_apMethods;
        // polozky
		CTypedPtrMap<CMapStringToPtr, CString, CUnitMember*> m_apMembers;
        // velikost hasovacich tabulek pro polozky a metody
		enum { MEMBERS_HASHTABLE_SIZE=13, METHODS_HASHTABLE_SIZE=21 };

        // namespace
        CPNamespace *m_pNamespace;

        // ziskani metody - dle jmena
		CUnitMethod* GetMethod(CString &strName, CPPreview *pPreview);
        // ziskani metody implementovane pouze na danem typu jednotky (ne na predkovi) - dle jmena
		CUnitMethod* GetMethodOnSelf(CString &strName);
        // ziskani polozky - dle jmena
		CUnitMember* GetMember(CString &strName, CPPreview *pPreview);

        // pridani polozky - typ ze stringu
		CUnitMember* AddMember(CString &strName, CString &strType, int nArrayLevel, CPPreview *pPreview);
        // pridani polozky 
		CUnitMember* AddMember(CString &strName, CType &DataType);
		CUnitMember* AddMember(CStringTableItem *stiName, CType &DataType);
        // pridani metody - navratovy typ ze stringu
		CUnitMethod* AddMethod(CString &strName, CString &strReturnType, int nArrayLevel, CPPreview *pPreview);
        // pridani metody
		CUnitMethod* AddMethod(CString &strName, CType &ReturnDataType);
		CUnitMethod* AddMethod(CStringTableItem *stiName, CType &ReturnDataType);

        // dotazovaci metoda - zda je jednotka potomkem nejake jine jednotky
		bool IsChildOf(CUnit *pAnotherUnit, CPPreview *pPreview);

        // dotaz na existenci polozky
        bool HasMember(CString &strName, CPPreview *pPreview);
        // dotaz na typ polozky
        bool GetMemberType(CString &strName, CType &MemberType, CPPreview *pPreview);
	};

    // class CStruct
    // =============
    // Trida pro uchovani informaci o strukture
    class CStruct
    {
    public:
        // normalni konstruktor po prekladu struktury
        CStruct(CPNamespace *pNamespace, CStringTableItem *m_stiName);
        // konstruktor bez parametru se pouziva, kdyz bude jednotka natazena
        // od code managera
        CStruct();
        ~CStruct();
    public:
        // jmeno struktury
		CStringTableItem *m_stiName;

        // polozky
		CArray<CType, CType&> m_aMemberTypes;
        CTypedPtrArray<CPtrArray, CStringTableItem*> m_aMemberNames;
        int m_nMemberCount;

        // namespace
        CPNamespace *m_pNamespace;

        // pridani polozky - typ ze stringu
		void AddMember(CString &strName, CString &strType, int nArrayLevel, CPPreview *pPreview);
        // pridani polozky 
		void AddMember(CString &strName, CType &DataType);

        // dotaz na existenci polozky
        bool HasMember(CString &strName);
        // dotaz na typ polozky
        bool GetMemberType(CString &strName, CType &MemberType);
    };

    // class CGlobalFunction
    // ======================
    // Trida pro uchovavani informaci o globalni funkci.
    class CGlobalFunction : public CUnitMethod 
    {
    public:
        CGlobalFunction(CStringTableItem *stiName, CType &ReturnDataType) 
                : CUnitMethod(stiName, ReturnDataType) {}
        CGlobalFunction() : CUnitMethod() {}
    };

    // class CSysGlobalFunction
    // ======================
    // Trida pro uchovavani informaci o systemove globalni funkci.
	typedef CSysObjectMethod CSysGlobalFunction;

    // class CEventHeader
    // ==================
    // Slouzi k uchovanani informaci o normalizovane hlavicce (bezne) zpravy.
	class CEventHeader
	{
	public:
		CEventHeader(CStringTableItem *stiName);
        virtual ~CEventHeader();
	public:
        // jmeno zpravy
		CStringTableItem *m_stiName;
        // typy argumentu
		CArray<CType, CType&> m_aArgs;

        // pridani argumentu
		void AddArgument(CPNamespace *pNamespace, CString &strType, int nArrayLevel, CPPreview *pPreview);
        // formulace informaci zde obsazenych do jednoho retezce 
        // (pro chybova hlaseni napr.)
		CString GetHeaderString();
	};

    // class CVariable
    // ===============
    // Trida pro uchovavani informaci o aktualnich lokalnich promennych
	class CVariable
	{
    public:
   		CVariable(CStringTableItem *stiName, CType &DataType, REG nReg, int nLevel, bool bArgument=false);
		virtual ~CVariable();
    public:
        // jmeno promenne
		CStringTableItem *m_stiName;
        // datovy typ
		CType m_DataType;
        // uroven bloku, ve kterem byla definovana
		int m_nLevel;
        // priznak, je-li argumentem aktualni funkce/metody (argument 
        // je vlastne take lokalni promenna)
		bool m_bArgument;
        // registr, ve kterem je ulozena
		REG m_nReg;	
        // priznak, byla-li inicializovana
		bool m_nInitialized;
		
        // pomocny union pro uchovavani typu promenne - je-li datovym typem
        // promenne jednotka nebo systemovy objekt, odkazuje na typ tohoto
        // objektu
		union
		{
			CUnit *m_pTypeUnit;
			CSysObject *m_pTypeSysObj;
		};
	};

    // class CUnitObjsType
    // ===================
    // Slouzi k tomu, aby informace o typech jednotek a objektu mohly
    // byt uchovavany v jedne tabulce -> obal nad tridy CUnit a CSysObject
	class CUnitObjsType
	{
	public:
		CUnitObjsType(EScriptType nType, void *pData);
		virtual ~CUnitObjsType();
	public:
        // skriptovy typ - pouzivame jednotka, objekt nebo struktura
		EScriptType m_nType;

        // ukazatel na prislusny typ
		union
		{
			CUnit *m_pUnit;
			CSysObject *m_pSysObj;
            CStruct *m_pStruct;
		};

	};

// class CPNamespace
public:
	CPNamespace();
	virtual ~CPNamespace();

    CCompilerKernel *m_pCompilerKrnl;

    // Vytvoreni namespace - nacita informace o systemych jmenech ze souboru
    // NAMESPACE_FILE (viz CICommon.h) (systemove objekty, sys.glob.fce, hlavicky 
    // zprav). Konfiguracni soubor parsuje flexem a bisonem (LexNsp.*, Nsp.*),
    // vola pritom funkce CPNamespace::OnXXX uvedene nize. Jestlize se mu soubor 
    // nepovede sparsovat, vypise chybovou hlasku (na StdErr) a vrati false,
    // jinak true.
	bool Create(CCompilerKernel *pCompilerKrnl);
    // Zruseni namespace - uvolneni obsahu vsech tabulek (vola RemoveAll()).
	void Delete();

    // Uvolneni obsahu vsech tabulek
	void RemoveAll();
    // Uvolneni vsech promennych s m_nLevel>nLevel (napr. na konci bloku, funkce).
	void RemoveVariablesToLevel(int nLevel);  
    // Odstraneni typu z namespace.
	void RemoveType(CString &strName);
    // Osdtraneni globalni funkce.
	void RemoveGlobalFunction(CString &strName);

    // Ziskani promenne dle jmena.
	CVariable* GetVariable(CString &strName);
    // Pridani nove promenne.
	CVariable* AddVariable(CStringTableItem *stiName, CType &DataType, REG nReg,
		int nLevel, bool bArgument=false);

    // Ziskani "objektoveho" typu - jednotka, struktura, objekt
	CUnitObjsType* GetType(CString &strName);
    // Pridani "objektoveho" typu - jednotka, struktura, objekt
	CUnitObjsType* AddType(EScriptType nType, void *pData);
	
    // Pridani typu jednotky.
	CUnit* AddUnit(CString &strName, CString &strParent);
    // Ziskani typu jednotky dle jmena.
	CUnit* GetUnit(CString &strName);
    
    // Pridani systemoveho objektu
    CSysObject* AddObject(CString &strName, bool bCanCreateInstance);
    // Ziskani typu systemoveho objektu
	CSysObject* GetObject(ESystemObjectType nSOType);
    CSysObject* GetObject(CString &strName);

    // Ziskani hlavicky zpravy dle jmena.
	CEventHeader* GetEventHeader(CString &strName);

    // Pridani globalni funkce.
	CGlobalFunction* AddGlobalFunction(CString &strName, CType &ReturnDataType);
    // Ziskani globalni funkce dle jmena.
	CGlobalFunction* GetGlobalFunction(CString &strName);

    // Pridani systemove globalni funkce.
	CSysGlobalFunction* AddSysGlobalFunction(long nID, CString &strName, CString &strReturnDataType,
         int nArrayLevel, CPPreview *pPreview);
    // Ziskani systemove globalni funkce dle jmena.
	CSysGlobalFunction* GetSysGlobalFunctionByName(CString &strName);
    // Ziskani systemove globalni funkce dle ID.
	CSysGlobalFunction* GetSysGlobalFunctionByID(long nID);

    // Pridani struktury
    CStruct* AddStruct(CString &strName);
    // Ziskani struktury
    CStruct* GetStruct(CString &strName);

    // Zjisteni, zda je jmeno nekde pouzito
    EScriptType GetScriptType(CString &strName);

protected:
	// ULOZENI NAMESPACE
    // default unit - defaultni predek (metody implementovane v C++)
    CUnit *m_pDefaultUnit;
    // vsechny aktualni lokalni promenne & argumenty
	CTypedPtrMap<CMapStringToPtr, CString, CVariable*> m_apVariables;	
    // nactene typy systemovych objektu a jednotek
	CTypedPtrMap<CMapStringToPtr, CString, CUnitObjsType*> m_apUnitObjsTypes;  
	// globalni funkce (ve skriptech)
    CTypedPtrMap<CMapStringToPtr, CString, CGlobalFunction*> m_apGlobalFunctions;  
    // systemove globalni funkce
	CTypedPtrMap<CMapStringToPtr, CString, CSysGlobalFunction*> m_apSysGlobalFunctions;  
	// hlavicky zprav
    CTypedPtrMap<CMapStringToPtr, CString, CEventHeader*> m_apEventHeaders;   
    // velikosti hashovacich tabulek
	enum { VAR_HASHTABLE_SIZE=1023, TYPES_HASHTABLE_SIZE=1021, 
		EVENTS_HASHTABLE_SIZE=101, GLOBALFUNCTIONS_HASHTABLE_SIZE=103, 
		SYSTEMGLOBALFUNCTIONS_HASHTABLE_SIZE=137};
	
public:
// NATAHOVANI TYPU OD CODE MANAGERA
    // Natazeni jednotky nebo struktury (pri nalezeni neznameho jmena typu),
    // vraci true, pokud takovy typ nalezen a natazen.
    bool LoadUnitOrStructType(CType &Type);
    // Natazeni jednotky (pri nalezeni neznameho jmena typu u 'extends'),
    // vraci true, pokud takovy typ nalezen a natazen.
    bool LoadUnitType(CString &strName);
    // Natazeni hlavicky globalni funkce (pri nalezeni jmena nezname funkce)
    bool LoadGlobalFunctionType(CString &strName);

public:
// NACITANI NAMESPACE ze souboru NAMESPACE_FILE (viz CICommon.h)
    // hlavni funkce pro nacteni systemoveho namespace - otevreni souboru,
    // spusteni parseru (viz CPNamespace::Create()).
	bool ReadNamespace();

protected:
    // aktualni parsovany objekt
	CSysObject *m_pCurrentObject;
    // aktualni parsovana metoda
	CSysObjectMethod  *m_pCurrentObjectMethod; 
    // aktualni parsovana hlavicka zpravy
	CEventHeader *m_pCurrentEventHeader;
    // aktualni parsovana globalni funkce
	CSysGlobalFunction *m_pCurrentFunction;
	
public:
// NACTENI NAMESPACE z konfigu
    // lexikalni parser
    CPLexNsp *m_pLexNsp;

    // hlaseni chyb
    void ParserError(char *);
	        
	// debugovaci vypis toho, co nacetl z NAMESPACE_FILE
	void DebugWrite(CCompilerErrorOutput *pOut);

	// akce pri nacitani z NAMESPACE_FILE
	void OnArgument(CString &strType, long nArrayLevel);
	bool OnFunctionBegin(long nID, CString &strRetType, int nArrayLevel, CString &strName);
	void OnFunctionEnd();
	bool OnObjectMethodBegin(long nID, CString &strReturnType, int nArrayLevel, CString &strName);
	void OnObjectMethodEnd();
	bool OnObjectMember(long nID, CString &strType, int nTypeArrayLevel, CString &strName, int nArrayLevel);
	bool OnObjectBegin(CString &strName);
	void OnObjectEnd();
	bool OnEventBegin(CString &strName);
	void OnEventEnd();
	void OnNoinstancesObjectEnd();
	bool OnNoinstancesObjectBegin(CString &strName);
};

#endif // !defined(AFX_NAMESPACE_H__405B2EC6_433E_11D3_A4B6_00A0C970CB8E__INCLUDED_)
