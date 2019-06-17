// Lex1stPass.h: interface for the CPLex1stPass class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LEX1STPASS_H__154C5E43_9FF9_11D3_AF82_004F49068BD6__INCLUDED_)
#define AFX_LEX1STPASS_H__154C5E43_9FF9_11D3_AF82_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef yyFlexLexer
#define yyFlexLexer yy1stFlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer
#endif

#define FIRSTPASS_IDENTIFIER			1
#define FIRSTPASS_UNIT_IDENTIFIER		2
#define FIRSTPASS_EXTENDS				3
#define FIRSTPASS_UNIT_KEYWORD		    4
#define FIRSTPASS_STRUCT				5

class CCompilerInput;

class CPLex1stPass : public yy1stFlexLexer  
{
public:
	CPLex1stPass(CCompilerInput *pStdIn);
	virtual ~CPLex1stPass();

    // tohle vola lex, kdyz chce vstup
    virtual int LexerInput( char* buf, int max_size );

    // tohle volam, kdyz chci token
    virtual int yylex();

    inline int GetLineNo() { return yylineno; }

public:
    int m_nArrayLevel;
    bool m_bSearchSets;

protected:
    CCompilerInput *m_pStdIn;
};

#endif // !defined(AFX_LEX1STPASS_H__154C5E43_9FF9_11D3_AF82_004F49068BD6__INCLUDED_)
