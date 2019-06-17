// ILexDefaultUnit.cpp: implementation of the CLexDefaultUnit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../CodeManager.h"
#include "LexDefaultUnit.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLexDefaultUnit::CLexDefaultUnit(CArchiveFile *pfileInput)
{
    m_pfileInput = pfileInput;
}

CLexDefaultUnit::~CLexDefaultUnit()
{
}

int CLexDefaultUnit::LexerInput( char* buf, int max_size )
{
    return m_pfileInput->Read( buf, max_size);
}

int CLexDefaultUnit::YyLex(SLDefaultUnitTokenData *yylval)
{   
    m_pToken = yylval;
    return yylex();
}

int CLexDefaultUnit::YyError(char *lpszError)
{
    throw new CStringException(DEFAULTUNIT_FILE"(%d): %s", yylineno, lpszError);
}
