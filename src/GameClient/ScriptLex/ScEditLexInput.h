// ScEditLexInput.h: interface for the CScEditLexInput class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCEDITLEXINPUT_H__964A7441_98D9_11D3_A881_00105ACA8325__INCLUDED_)
#define AFX_SCEDITLEXINPUT_H__964A7441_98D9_11D3_A881_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CScEditLexInput : public CObject  
{
public:
	virtual BOOL IsEOF();
	virtual char GetCharacter();
	CScEditLexInput();
	virtual ~CScEditLexInput();

};

#endif // !defined(AFX_SCEDITLEXINPUT_H__964A7441_98D9_11D3_A881_00105ACA8325__INCLUDED_)
