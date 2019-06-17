/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Editace skriptu - syntax highlight
 *   Autor: Helena Kupková
 *  
 *   Popis: Objekt CScEditLex implementujici rozhrani k lexem 
 *          vygenerovanemu automatu
 * 
 ***********************************************************/

#include "stdafx.h"

#include "ScEditLex.h"
#include "ScEditLexGlobals.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// globalni ukazatel na instanci objektu ScEditLex, plni se v konstuktoru

CScEditLex *gpScEditLex=NULL;

// globalni promenne pro lexer

DWORD g_nRow;
WORD g_nColumn;

//////////////////////////////////////////////////////////////////////
// class CScEditLexToken
//////////////////////////////////////////////////////////////////////

CScEditLexToken::CScEditLexToken()
{
    m_nToken=SLTError;
    m_nStartRow=-1;
    m_nEndRow=-1;
    m_nStartColumn=-1;
    m_nEndColumn=-1;
}

void CScEditLexToken::SetStart(CScEditLexToken::EScEditToken nToken, DWORD nStartRow, WORD nStartColumn)
{
    m_nToken=nToken;
    m_nStartRow=nStartRow;
    m_nStartColumn=nStartColumn;
}

void CScEditLexToken::SetEnd(DWORD nEndRow, WORD nEndColumn)
{
    m_nEndRow=nEndRow;
    m_nEndColumn=nEndColumn;
}

//////////////////////////////////////////////////////////////////////
// class CScEditLex
//////////////////////////////////////////////////////////////////////
                                              
CScEditLex::CScEditLex()
{
    m_pInput=NULL;
    gpScEditLex=this;
}

CScEditLex::~CScEditLex()
{
    FreeScEditLexBuffer();
}

bool CScEditLex::Create(CScEditLexInput *pInput)
{
    Reset(pInput, 0, 0);

    return true;
}

void CScEditLex::Delete()
{
}

void CScEditLex::Reset(CScEditLexInput *pInput, DWORD nRow, WORD nColumn )
{
    m_pInput = pInput;

    g_nRow = nRow; g_nColumn = nColumn;

    sceditrestart(NULL);
}

void CScEditLex::GetToken(CScEditLexToken &Token)
{
    ASSERT (m_pInput!=NULL);

    m_pToken=&Token;

    sceditlex();
}

