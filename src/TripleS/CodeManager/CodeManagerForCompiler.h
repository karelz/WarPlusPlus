// CodeManagerForCompiler.h: interface for the CCodeManagerForCompiler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CODEMANAGERFORCOMPILER_H__DE75D203_E953_11D3_AFED_004F49068BD6__INCLUDED_)
#define AFX_CODEMANAGERFORCOMPILER_H__DE75D203_E953_11D3_AFED_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCodeManager;

class CCodeManagerForCompiler : public CObject  
{
public:
	CCodeManagerForCompiler( );
    virtual ~CCodeManagerForCompiler() {}

    void Init( CCodeManager *pCodeManager) { m_pCodeManager = pCodeManager; }

// ZISKANI HLAVICKY PRO PREKLADAC
public:
    // ziskani typu (jednotka, glob.fce, struktura) dle jmena
    virtual EScriptType GetScriptType(CStringTableItem *stiName);

    // ziskani hlavicky jednotky
    virtual bool GetUnitTypeForCompiler(CStringTableItem *stiName, CPNamespace::CUnit *pCompilerUnit);

    // ziskani hlavicky globalni funkce
    virtual bool GetGlobalFunctionTypeForCompiler(CStringTableItem *stiName, CPNamespace::CGlobalFunction *pCompilerGlobFunc);

    // ziskani hlavicky struktury 
    virtual bool GetStructureTypeForCompiler(CStringTableItem *stiName, CPNamespace::CStruct *pCompilerStruct);

// UPLOADOVANI KODU
public:
    // ukladat se smi pouze jeden kod v jednom case, je to uzamkovane

	// zacatek ukladani (tohle vola kompilator)
	// throws CFileException
	virtual CCompilerOutput* UploadScriptCodeStart(EScriptType Type, CString &strName, CString strParentName = "");
	// konec ukladani - vyvola update beziciho kodu...
	virtual bool UploadScriptCodeEnd();


protected:
    CCodeManager *m_pCodeManager;

};

#endif // !defined(AFX_CODEMANAGERFORCOMPILER_H__DE75D203_E953_11D3_AFED_004F49068BD6__INCLUDED_)
