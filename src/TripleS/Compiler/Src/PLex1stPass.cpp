// Lex1stPass.cpp: implementation of the CPLex1stPass class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CompilerKernel.h"
#include "PLex1stPass.h"

#include "TripleS\CompilerIO\CompilerIO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPLex1stPass::CPLex1stPass(CCompilerInput *pStdIn)
{
    m_pStdIn = pStdIn;
}

CPLex1stPass::~CPLex1stPass()
{

}

int CPLex1stPass::LexerInput( char* buf, int max_size )
{
    int c=m_pStdIn->GetChar();
    
    if (c==EOF) return 0;
    else 
    {
        buf[0]=(char)c;
        return 1;
    }
}

