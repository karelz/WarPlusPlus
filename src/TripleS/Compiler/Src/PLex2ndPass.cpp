// PLex2ndPass.cpp: implementation of the CPLex2ndPass class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CompilerKernel.h"
#include "PLex2ndPass.h"

#include "TripleS\CompilerIO\CompilerIO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPLex2ndPass::CPLex2ndPass(CCompilerInput *pStdIn, CCompilerKernel *pCompilerKrnl)
{
    m_pStdIn = pStdIn;
    m_pCompilerKrnl = pCompilerKrnl;
}

CPLex2ndPass::~CPLex2ndPass()
{
}

int CPLex2ndPass::LexerInput( char* buf, int max_size )
{
    int c=m_pStdIn->GetChar();
    
    if (c==EOF) return 0;
    else 
    {
        buf[0]=(char)c;
        return 1;
    }
}

int CPLex2ndPass::YyLex(CPExpr *pExpr)
{
    m_pExpr = pExpr;
    return yylex();
}
