// PLexNsp.cpp: implementation of the CPLexNsp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CompilerKernel.h"
#include "PLexNsp.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPLexNsp::CPLexNsp(CArchiveFile *pfileInput)
{
    m_pfileInput = pfileInput;
}

CPLexNsp::~CPLexNsp()
{
}

int CPLexNsp::LexerInput( char* buf, int max_size )
{
    if (m_pfileInput->Read(&buf[0], 1)==0)
    {   
        m_cLastChar=EOF;
        return 0;
    }
    else
    {
        m_cLastChar=buf[0];
        return 1;
    }
}

// tohle vola bison, kdyz chce token
int CPLexNsp::YyLex(SNspTokenData *pToken)
{
    m_pToken = pToken;
    return yylex();
}

