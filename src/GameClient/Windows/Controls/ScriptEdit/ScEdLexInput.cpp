// ScEdLexInput.cpp: implementation of the CScEdLexInput class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScEdLexInput.h"

#include "ScriptEdit.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScEdLexInput::CScEdLexInput()
{

}

CScEdLexInput::~CScEdLexInput()
{

}

char CScEdLexInput::GetCharacter()
{
  CScriptEdit::SLine *pLine;

  // if we are to end it -> do it
  if(m_bEndIt)
    return (char)0x0FF;
  
  // if we are at the end of the edit -> return EOF
  if(m_nCurrentLine > (m_pEdit->m_aLines.GetSize() - 1))
    return (char)0x0FF;
  
  pLine = m_pEdit->m_aLines[m_nCurrentLine];

  // special case if we are at the end of line
  // we have to return some EOL characters, which aren't
  // in the text in the script edit
  if(m_nCurrentColumn == pLine->m_strText.GetLength()){
    m_nCurrentColumn++;
    return 0x0d;
  }
  if(m_nCurrentColumn >= (pLine->m_strText.GetLength() + 1)){
    m_nCurrentColumn = 0;
    m_nCurrentLine++;
    return 0x0a;
  }
  
  m_nCurrentColumn++;
  return pLine->m_strText[m_nCurrentColumn - 1];
}

BOOL CScEdLexInput::IsEOF()
{
  // if we are to end it -> do it
  if(m_bEndIt)
    return TRUE;
  
  // if we are at the end of the edit -> return EOF
  if(m_nCurrentLine > (m_pEdit->m_aLines.GetSize() - 1))
    return TRUE;

  return FALSE;
}

void CScEdLexInput::Create(CScriptEdit *pEdit)
{
  ASSERT_VALID(pEdit);

  m_pEdit = pEdit;
}

void CScEdLexInput::Delete()
{
  m_pEdit = NULL;
}

BOOL CScEdLexInput::Open()
{
  ASSERT_VALID(this);

  m_nCurrentLine = m_nStartLine;
  m_nCurrentColumn = m_nStartColumn;

  return TRUE;
}

void CScEdLexInput::Close()
{
  ASSERT_VALID(this);
}

void CScEdLexInput::SetRange(int nStartLine, int nStartColumn, int nEndLine, int nEndColumn)
{
  m_nStartLine = nStartLine; m_nStartColumn = nStartColumn;
  m_nCurrentLine = nStartLine; m_nCurrentColumn = nStartColumn;
  m_nEndLine = nEndLine; m_nEndColumn = nEndColumn;

  m_bEndIt = FALSE;
}

BOOL CScEdLexInput::IsOutOfRange(int nLine, int nColumn)
{
  // try if are out of givven range
  // if so -> end the input
  // elseway -> just go ahead
  if(m_bEndIt) return TRUE;
  if((nLine >= m_nEndLine) &&
    (nColumn >= m_nEndColumn)){
    m_bEndIt = TRUE;
    return TRUE;
  }
  return FALSE;
}
