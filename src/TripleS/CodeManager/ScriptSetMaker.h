// ScriptSetMaker.h: interface for the CScriptSetMaker class.
//
//////////////////////////////////////////////////////////////////////

#include "CodeManager.h"
#include "Triples\CompilerIO\CompilerIO.h"
#include "TripleS\CompilerIO\Src\CompilerOutputArchive.h"

#if !defined(AFX_SCRIPTSETMAKER_H__99710EEB_3EFF_4ACB_9AA4_BF098F523C7E__INCLUDED_)
#define AFX_SCRIPTSETMAKER_H__99710EEB_3EFF_4ACB_9AA4_BF098F523C7E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CScriptSetMaker : public CCodeManagerForCompiler 
{
public:
	CScriptSetMaker();
	virtual ~CScriptSetMaker();

	// vytvori a znici objekt pro prekladani skriptsetu, ktery preklada zdrojaky
	// z InputDir a binarky uklada do OutputDir.
	bool Create( CString InputDir, CDataArchive OutputArchive);
	void Delete();

	// preklad zdrojaku
	bool Compile( CCompilerErrorOutput *pStdErr);

protected:
	// code manager - pro nacteni interface defaultni jednotky
	CCodeManager m_CodeManager;

	// compiler
	CCompiler m_Compiler;

	// current directory
	CString m_strCurrentDir;

	// input & output
	CString m_strInputDir;
	CDataArchive m_OutputArchive;

	// stdout for uploading binaries
	CCompilerOutputArchive m_CompilerOutput;

//
// CCodeManagerForCompiler interface
//

// UPLOADOVANI KODU
public:
    // ukladat se smi pouze jeden kod v jednom case, je to uzamkovane

	// zacatek ukladani (tohle vola kompilator)
	// throws CFileException
	virtual CCompilerOutput* UploadScriptCodeStart(EScriptType Type, CString &strName, CString strParentName = "");
	// konec ukladani - vyvola update beziciho kodu...
	virtual bool UploadScriptCodeEnd();
};

#endif // !defined(AFX_SCRIPTSETMAKER_H__99710EEB_3EFF_4ACB_9AA4_BF098F523C7E__INCLUDED_)
