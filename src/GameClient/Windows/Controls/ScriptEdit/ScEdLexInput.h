// ScEdLexInput.h: interface for the CScEdLexInput class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCEDLEXINPUT_H__9F891D59_99ED_11D3_A0DD_B66FF94DE631__INCLUDED_)
#define AFX_SCEDLEXINPUT_H__9F891D59_99ED_11D3_A0DD_B66FF94DE631__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\..\ScriptLex\ScEditLexInput.h"

class CScriptEdit;

class CScEdLexInput : public CScEditLexInput  
{
public:
	BOOL IsOutOfRange(int nLine, int nColumn);
	void SetRange(int nStartLine, int nStartColumn, int nEndLine, int nEndColumn);
	void Close();
	BOOL Open();
	virtual void Delete();
	void Create(CScriptEdit *pEdit);
	virtual BOOL IsEOF();
	virtual char GetCharacter();
	CScEdLexInput();
	virtual ~CScEdLexInput();

private:
  // pointer to the editor object
  CScriptEdit *m_pEdit;

  // range to compute the lex on
  int m_nStartLine;
  int m_nStartColumn;
  int m_nEndLine;
  int m_nEndColumn;

  // current position
  int m_nCurrentLine;
  int m_nCurrentColumn;

  BOOL m_bEndIt;
};

#endif // !defined(AFX_SCEDLEXINPUT_H__9F891D59_99ED_11D3_A0DD_B66FF94DE631__INCLUDED_)
