// CodeManager.h: interface for the CCodeManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CODEMANAGER_H__1D9F4E33_9520_11D3_AF6E_004F49068BD6__INCLUDED_)
#define AFX_CODEMANAGER_H__1D9F4E33_9520_11D3_AF6E_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*
#ifndef __INTERPRET_TRACE_DEFINED__
#define __INTERPRET_TRACE_DEFINED__
#ifdef _DEBUG
	extern BOOL g_bTraceInterpret;
	extern BOOL g_bTraceInterpretDetails;

	#define TRACE_INTERPRET if ( g_bTraceInterpret ) TRACE_NEXT 
	#define TRACE_INTERPRET0(text) do { if ( g_bTraceInterpret ) { TRACE0_NEXT(text); } } while ( 0 )
	#define TRACE_INTERPRET1(text,p1) do { if ( g_bTraceInterpret ) { TRACE1_NEXT(text,p1); } } while ( 0 )
	#define TRACE_INTERPRET2(text,p1,p2) do { if ( g_bTraceInterpret ) { TRACE2_NEXT(text,p1,p2); } } while ( 0 )
	#define TRACE_INTERPRET3(text,p1,p2,p3) do { if ( g_bTraceInterpret ) { TRACE3_NEXT(text,p1,p2,p3); } } while ( 0 )

	#define TRACE_INTERPRET_DETAILS if ( g_bTraceInterpretDetails ) TRACE_NEXT 
	#define TRACE_INTERPRET_DETAILS0(text) do { if ( g_bTraceInterpretDetails ) { TRACE0_NEXT(text); } } while ( 0 )
	#define TRACE_INTERPRET_DETAILS1(text,p1) do { if ( g_bTraceInterpretDetails ) { TRACE1_NEXT(text,p1); } } while ( 0 )
	#define TRACE_INTERPRET_DETAILS2(text,p1,p2) do { if ( g_bTraceInterpretDetails ) { TRACE2_NEXT(text,p1,p2); } } while ( 0 )
	#define TRACE_INTERPRET_DETAILS3(text,p1,p2,p3) do { if ( g_bTraceInterpretDetails ) { TRACE3_NEXT(text,p1,p2,p3); } } while ( 0 )
	#define TRACE_INTERPRET_DETAILS4(text,p1,p2,p3,p4) do { if ( g_bTraceInterpretDetails ) { TRACE4_NEXT(text,p1,p2,p3,p4); } } while ( 0 )

#else //!_DEBUG
	#define TRACE_INTERPRET TRACE
	#define TRACE_INTERPRET0 TRACE0
	#define TRACE_INTERPRET1 TRACE1
	#define TRACE_INTERPRET2 TRACE2
	#define TRACE_INTERPRET3 TRACE3

	#define TRACE_INTERPRET_DETAILS TRACE
	#define TRACE_INTERPRET_DETAILS0 TRACE0
	#define TRACE_INTERPRET_DETAILS1 TRACE1
	#define TRACE_INTERPRET_DETAILS2 TRACE2
	#define TRACE_INTERPRET_DETAILS3 TRACE3
    #define TRACE_INTERPRET_DETAILS4 TRACE4
#endif //!_DEBUG
#endif //__INTERPRET_TRACE_DEFINED__
*/
class CLexDefaultUnit;
class CIUnitType;
class CIStructureType;
class CIGlobalFunction;
class CIMethod;

class CZCivilization;

class CCodeManager  :  public CNotifier
{
public:
    // class CUnitInheritanceInfo
    // ==========================
    // Trida pro uchovani stromu dedicnosti jednotek. 
    class CUnitInheritanceInfo
    {
    public:
        CUnitInheritanceInfo(CStringTableItem *stiUnitName, CUnitInheritanceInfo *pParent, 
            bool bDefined=true);
		~CUnitInheritanceInfo();

        // jmeno jednotky
        CStringTableItem *m_stiUnitName;
        // ukazatel na predka
        CUnitInheritanceInfo *m_pParent;
        // priznak, zda je jednotka definovana
        bool m_bDefined;
    };

    // class CScript
    // =============
    class CScript
    {
    public:
        CScript();
        ~CScript();
    public:
        CStringTableItem *m_stiName;
        EScriptType m_eScriptType;
        union
        {
            CIUnitType *m_pIUnitType;
            CIStructureType *m_pStructType;
            CIGlobalFunction *m_pGlobalFunc;
        };

        inline bool IsStructure()      {  return m_eScriptType==SCRIPT_STRUCTURE; }
        inline bool IsUnit()           {  return m_eScriptType==SCRIPT_UNIT;      }
        inline bool IsGlobalFunction() {  return m_eScriptType==SCRIPT_FUNCTION;  }
    };

public:
    // konstruktor & destruktor
	CCodeManager();
	virtual ~CCodeManager();

    // vytvoreni managera - prohlidne adresare a vytvori si strom dedicnosti.
    // Pokud nastane nejaka chyba nebo nekonzistence stromu dedicnosti 
    // (napr. dedi se z jednotky, ktera neni definovana), hazi
    // vyjimku CStringException.
    void Create(CString &strUserScriptsPath, CString &strScriptSetPath, CZCivilization *pZCivilization);

    // zruseni managera
    void Delete();         
    // vyprazdenni (volano z Delete)
    void RemoveAll();

// ZISKANI SKRIPTU pro intepret
    // ziskani jednotky
    CIUnitType* GetUnitType(CStringTableItem *stiName);
    // ziskani struktury
    CIStructureType* GetStructureType(CStringTableItem *stiName);
    // ziskani globalni funkce
    CIGlobalFunction* GetGlobalFunction(CStringTableItem *stiName);

// PROFILE COUNTERS
    void GetCounters( CString &strResult);
	void ClearCounters( void (CIConstructor::*pClearMethod)());
    void ClearLocalCounters();
	void ClearGlobalAndLocalCounters();

// TOOLBARS
  CString GetToolbarsDirectory () { return m_strSourceUserPath + "Toolbars\\"; }

// METODY VOLANE Z CREATE
    // vytvoreni informace o dedicnosti jednotek
    void CreateUnitInheritanceInfo();
    // natazeni hlavicky defaultni funkce (ze souboru DEFAULTUNIT_FILE)
    void CreateDefaultUnit();

// LOADOVANI INTERFACE DEFAULT JEDNOTKY
    static void LoadDefaultUnit();
    // interface defaultni jednotky
    static CIUnitType *m_pDefaultUnitType;
	// refcount codemanageru
	static LONG m_nCodeManagersCount;

    // Pomocne promenne
    static CIUnitType *m_pHelperDefaultUnit;
    // aktualni metoda
    static CIMethod *m_pDefaultUnitMethod;
    // aktualni typu argumentu metody
    static CArray<CIType, CIType&> m_aDefaultUnitMethodArgs;
    // Lexikalni analyzator
    static CLexDefaultUnit *m_pLexDefaultUnit; 
    // Actions
    static void OnDefaultUnitBegin(CString &strName);
    static void OnDefaultUnitEnd();
    static void OnDefaultUnitMember(EScriptType ScType, CString &strType, CString &strName, int nArrayLevel);
    static void OnDefaultUnitMethodBegin(EScriptType ScType, CString &strRetType, int nArrayLevel, CString &strName);
    static void OnDefaultUnitMethodEnd();
    static void OnDefaultUnitArgument(EScriptType ScType, CString &strType, int nArrayLevel);

public:
// CIVILIZACE
    CZCivilization *m_pZCivilization;

// UCHOVAVANI KODU V PAMETI
protected:
    // metoda LookForScriptOnDisk:
    // nScriptType==SCRIPT_UNKNOWN:
    //      Vyhleda na disku soubor s nazvem stiName a podle pripony zjisti typ, co to je
    //      (jednotka, struktura...) a vrati se. 
    // nScriptType==SCRIPT_XXX:
    //      Dotaz zda existuje jednotka/struktura/fce daneho jmena.
    //      Vraci se SCRIPT_UNKNOWN (neexistuje-li) nebo hodnota parametru nScriptType (existuje).
    // !!! Neotvira soubor a nediva se dovnitr. 
    // Pokud pstrFoundFilename<>NULL, ve vysledku je jmeno souboru i s cestou, kde se
    // script nachazi (pokud ovsem nebylo vraceno SCRIPT_UNKNOWN).
    EScriptType LookForScriptOnDisk(CStringTableItem *stiName, CString *pstrFoundFilename=NULL, 
        EScriptType nScriptType = SCRIPT_UNKNOWN);

    // naloadovani skriptu - dle jmena souboru
    // throws CFileException, CStringException
	// musi byt zamceno m_mutexScripts
    CScript* LoadUnitByFilename(CString &strFilename, CIUnitType *pParentUnit, bool bAddToLoadedScripts);

	// tu uz to neni zamceno
    CScript* LoadGlobalFunctionByFilename(CString &strFilename, bool bAddToLoadedScripts);
    CScript* LoadStructureByFilename(CString &strFilename, bool bAddToLoadedScripts);
    
    // naloadovani skriptu - dle jmena unity/glob.fce/struktury
    // throws CFileException, CStringException
    CScript* LoadUnit(CStringTableItem *stiName);
    CScript* LoadGlobalFunction(CStringTableItem *stiName);
    CScript* LoadStructure(CStringTableItem *stiName);

    // ulozeni naloadovanych skriptu v pameti
    CTypedPtrMap<CMapPtrToPtr, CStringTableItem*, CScript*> m_tpScripts;
    // zamek pro m_tpScripts
    CCriticalSection m_mutexScripts;
    // velikost hashovaci tabulky
    enum { SCRIPTS_HASHTABLE_SIZE=503 };

public:
// UPLOADOVANI KODU
    // ukladat se smi pouze jeden kod v jednom case, je to uzamkovane

	// zacatek ukladani (tohle vola kompilator)
	// throws CFileException
	CCompilerOutput* UploadScriptCodeStart(EScriptType Type, CString &strName, CString strParentName="");
	// konec ukladani - vyvola update beziciho kodu...
	bool UploadScriptCodeEnd();
protected:
	// pro ukladani prelozeneho kodu - vraci metoda UnploadScriptCodeStart
	CCompilerOutput m_CodeStdOut;
    // jmeno a typ ukladaneho kodu
	CStringTableItem *m_stiCodeUpdateName;
    CString m_strCodeUpdateFilename;
	EScriptType m_CodeUpdateType;
	// zamek
	CCriticalSection m_mutexCodeUpload;

public:
    // UPLOADOVANI ZDROJAKU
    // ukladat se smi pouze jeden zdrojak v jednom case, je to uzamkovane

	// zacatek ukladani
	// throws CFileException
	CCompilerOutput* UploadScriptSourceStart(CString &strFilename);
	// konec ukladani - vyvola update beziciho kodu...
	bool UploadScriptSourceEnd();

protected:
    // pro ukladani zdrojaku - vraci metoda UnploadScriptSourceStart
	CCompilerOutput m_SourceStdOut;
    // jmeno a typ ukladaneho kodu
	CString m_strSourceUpdateName, m_strSourceUpdateFilename;
	EScriptType m_SourceUpdateType;
	// zamek
	CCriticalSection m_mutexSourceUpload;

public:
// VYLISTOVANI ZDROJAKU
    typedef enum { USER_SOURCE, SYSTEM_SOURCE} ESourceType;
    void ListSourceFiles(ESourceType eSourceType, CString &strResult);

public:
    // DOWNLOADOVANI ZDROJAKU
    // download zdrojaku dle adresare a jmena
    // throws CFileException 
    CCompilerInputFile* DownloadScriptSourceStart(ESourceType eSourceType, CString &strFileName);
    // konec downloadu
    void DownloadScriptEnd();

    // ZISKANI SEZNAMU ZDROJAKU, ktere byly zmeneny (tj. ty, co nemaji read-only flag)
    // Vraci jmena souboru z adresare Source\User oddelene znakem '\n'
    void GetChangedScripts( CString &strChangedScriptsFilenames, CString &strPath);

protected:
    // pro download zdrojaku
    CCompilerInputFile m_SourceStdIn;
    // zamek
    CCriticalSection m_mutexSourceDownload;

// POMOCNE FUNKCE pro soubory, pracuji s read-only flagem
    void MoveFile( CString &strSrcFileName, CString &strDestFileName);
    void DeleteFile( CString &strFileName);

// ZISKANI HLAVICKY PRO PREKLADAC
public:
    // ziskani typu (jednotka, glob.fce, struktura) dle jmena
    EScriptType GetScriptType(CStringTableItem *stiName);
    // ziskani hlavicky jednotky
    bool GetUnitTypeForCompiler(CStringTableItem *stiName, CPNamespace::CUnit *pCompilerUnit);
    // ziskani hlavicky globalni funkce
    bool GetGlobalFunctionTypeForCompiler(CStringTableItem *stiName, CPNamespace::CGlobalFunction *pCompilerGlobFunc);
    // ziskani hlavicky struktury 
    bool GetStructureTypeForCompiler(CStringTableItem *stiName, CPNamespace::CStruct *pCompilerStruct);

protected:
    // cesty ke zkompilovanemu kodu
    CString m_strCodeUserPath, m_strCodeSystemPath;
    // cesty ke zdrojakum
    CString m_strSourceUserPath, m_strSourceSystemPath;

    // strom dedicnosti (tridy CUnitInheritanceInfo ulozene v hasovaci
    // tabulce pro rychlejsi vyhledavani dle jmena, mezi sebou jsou
    // pointery provazany do stromu dedicnosti (kazda jednotka ukazuje na
    // sveho predka).
    CTypedPtrMap<CMapPtrToPtr, CStringTableItem*, CUnitInheritanceInfo*> m_tpUnitInheritanceInfo;
    enum { UNITINHERITANCE_TABLE_SIZE = 199 };

    // zamek nad m_tpUnitInheritanceInfo
    CCriticalSection m_mutexUnitInheritanceInfo;

    // priznak, zda byl code manager inicializovan
    bool m_bCreated;

// INTERFACE PRO PREKLADAC
protected:
    CCodeManagerForCompiler m_CodeManagerForCompiler;
public:
    CCodeManagerForCompiler* GetCodeManagerForCompiler() { return &m_CodeManagerForCompiler; }

// UPDATE KODU
protected:
    typedef enum {
        NO_DIFFERENCES = 0x00,
        DIFFERENT_ALL = 0x01,
        DIFFERENT_CODE = 0x02,
        DIFFERENT_STRINGTABLE = 0x04,
        DIFFERENT_FLOATTABLE = 0x08,
        DIFFERENT_CONSTRUCTOR = 0x10,
        DIFFERENT_DESTRUCTOR = 0x20
    } ECompareResult;

public:
    void DoUpdateUnit();
    void DoUpdateStruct();
    void DoUpdateFunction();

protected:
    CEvent m_DoUpdate;
    CEvent m_UpdateDone;
    void DoUpdateUnitCode( CIUnitType *pOldType, CIUnitType *pNewType, 
                                     CIChangedMethodEventsList *pChangedMethodEvents,
                                     ECompareResult CompRes);
    void DoUpdateUnitAll( CIUnitType *pOldType, CIUnitType *pNewType);

    ECompareResult CompareUnits( CIUnitType *pOldType, CIUnitType *pNewType,
                             CIChangedMethodEventsList* &pMethodEvents);
    ECompareResult CompareStructures( CIStructureType *pOldType, CIStructureType *pNewType);
    ECompareResult CompareFunctions( CIGlobalFunction *pOldFunc, CIGlobalFunction *pNewFunc);

    CIUnitTypeList* GetUnitChildren( CIUnitType *pSearchedUnitType);

#ifdef _DEBUG
public:
	virtual void AssertValid() const;
//	virtual void Dump(CDumpContext& dc) const;
#endif

public:
//
//	SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage,
                         CString &strUserScriptsPath, 
                         CString &strScriptSetPath,
                         CString &strCivilizationName);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit( CInterpret *pInterpret);
	void PersistentInit();

protected:
    CTypedPtrMap<CMapPtrToPtr, CStringTableItem*, CScript*> *m_ptpPersistentLoadScripts;

    void CopyFileToArchive( CString &strSrcPath, CString &strDestArchivePath, 
                            LPCTSTR lpszFilename, CDataArchive *pDataArchive,
                            CFileException *pFE);
    void CopyFileFromArchive( CString &strSrcArchivePath, CString &strDestPath, 
                            LPCTSTR lpszFilename, CDataArchive *pDataArchive,
                            CFileException *pFE);

};

#endif // !defined(AFX_CODEMANAGER_H__1D9F4E33_9520_11D3_AF6E_004F49068BD6__INCLUDED_)
