// CodeManagerForCompiler.cpp: implementation of the CCodeManagerForCompiler class.
//
// Interface mezi kompilatorem a code managerem.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\CodeManager.h"
#include "..\CodeManagerForCompiler.h"

//////////////////////////////////////////////////////////////////////

CCodeManagerForCompiler::CCodeManagerForCompiler( )
{
    m_pCodeManager = NULL;
}

//////////////////////////////////////////////////////////////////////

// ziskani typu (jednotka, glob.fce, struktura) dle jmena
EScriptType CCodeManagerForCompiler::GetScriptType(CStringTableItem *stiName)
{
    ASSERT( m_pCodeManager != NULL);
    return m_pCodeManager->GetScriptType(stiName);
}

//////////////////////////////////////////////////////////////////////

// ziskani hlavicky jednotky
bool CCodeManagerForCompiler::GetUnitTypeForCompiler(CStringTableItem *stiName, 
                                                     CPNamespace::CUnit *pCompilerUnit)
{
    ASSERT( m_pCodeManager != NULL);
    return m_pCodeManager->GetUnitTypeForCompiler( stiName, pCompilerUnit);
}

//////////////////////////////////////////////////////////////////////

// ziskani hlavicky globalni funkce
bool CCodeManagerForCompiler::GetGlobalFunctionTypeForCompiler(CStringTableItem *stiName, 
                                              CPNamespace::CGlobalFunction *pCompilerGlobFunc)
{
    ASSERT( m_pCodeManager != NULL);
    return m_pCodeManager->GetGlobalFunctionTypeForCompiler( stiName, pCompilerGlobFunc);
}

//////////////////////////////////////////////////////////////////////

// ziskani hlavicky struktury 
bool CCodeManagerForCompiler::GetStructureTypeForCompiler(CStringTableItem *stiName, 
                                                          CPNamespace::CStruct *pCompilerStruct)
{
    ASSERT( m_pCodeManager != NULL);
    return m_pCodeManager->GetStructureTypeForCompiler( stiName, pCompilerStruct);
}

//////////////////////////////////////////////////////////////////////

// zacatek ukladani (tohle vola kompilator)
// throws CFileException
CCompilerOutput* CCodeManagerForCompiler::UploadScriptCodeStart(EScriptType Type, CString &strName, CString strParentName)
{
    ASSERT( m_pCodeManager != NULL);
    return m_pCodeManager->UploadScriptCodeStart( Type, strName, strParentName);
}

//////////////////////////////////////////////////////////////////////

// konec ukladani - vyvola update beziciho kodu...
bool CCodeManagerForCompiler::UploadScriptCodeEnd()
{
    ASSERT( m_pCodeManager != NULL);
    return m_pCodeManager->UploadScriptCodeEnd();
}

//////////////////////////////////////////////////////////////////////