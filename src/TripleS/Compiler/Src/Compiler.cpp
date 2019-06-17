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

#include "stdafx.h"
#include "CompilerKernel.h"

#include "TripleS\CompilerIO\CompilerIO.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// debug bisona

extern int yydebug;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompiler::CCompiler()
{
	m_pCompilerKrnl=NULL;
    m_eCompilerPhase = NOT_COMPILING;
}

CCompiler::~CCompiler()
{
	this->Delete();
}

bool CCompiler::Create( CCodeManagerForCompiler *pCodeManagerForCompiler)
{
	ASSERT(m_pCompilerKrnl==NULL);

	ASSERT(pCodeManagerForCompiler!=NULL);

	m_pCompilerKrnl=new CCompilerKernel( pCodeManagerForCompiler);
    m_eCompilerPhase = NOT_COMPILING;

	return true;
}

void CCompiler::Delete()
{
	if (m_pCompilerKrnl != NULL) 
	{
		delete m_pCompilerKrnl;
		m_pCompilerKrnl=NULL;
	}
}

bool CCompiler::Compile(CCompilerInput *pStdIn, 
				        CCompilerErrorOutput *pStdErr,
                        bool bUpdateCodeOnSuccess,
                        bool bDebugOutput/*=false*/,
				        CCompilerErrorOutput *pDbgOut/*=NULL*/,
				        bool bDebugParser/*=false*/)
{
	ASSERT_VALID(this);

	ASSERT(m_pCompilerKrnl!=NULL); // assert zda bylo zavolano Create

    CString str;
    bool returnValue=false;

    // reset compiler
	m_pCompilerKrnl->Reset(pStdIn, pStdErr, bUpdateCodeOnSuccess, bDebugOutput, pDbgOut, bDebugParser);

    pStdErr->PutString("Compiling...\r\n" );
    m_eCompilerPhase = FIRST_PASS;

    // prvni pruchod
    if (m_pCompilerKrnl->FirstPass())
    {
        pStdIn->Reset();

        // druhy pruchod
        m_eCompilerPhase = SECOND_PASS;
        pStdErr->Format(" %s\n", pStdIn->GetName());
        m_pCompilerKrnl->SecondPass();
    }

    m_eCompilerPhase = NOT_COMPILING;

    // vysledky
    str.Format("\r\nDone - %d error(s), %d warning(s)\r\n",  
        m_pCompilerKrnl->m_nErrorCount, m_pCompilerKrnl->m_nWarningCount);
    pStdErr->PutString(str);

	pStdErr->Flush();

    return (m_pCompilerKrnl->m_nErrorCount + m_pCompilerKrnl->m_nWarningCount) == 0;
}

void CCompiler::ResetLineNo()
{
    ASSERT(m_pCompilerKrnl != NULL); 
    m_pCompilerKrnl->ResetLineNo();
}

#ifdef _DEBUG

void CCompiler::AssertValid() const
{
    CObject::AssertValid();
	ASSERT(m_pCompilerKrnl!=NULL);
	m_pCompilerKrnl->AssertValid();
}	

#endif