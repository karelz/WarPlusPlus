// ScEditLexInput.cpp: implementation of the CScEditLexInput class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScEditLexInput.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScEditLexInput::CScEditLexInput()
{

}

CScEditLexInput::~CScEditLexInput()
{

}

char CScEditLexInput::GetCharacter()
{
  return (char)0xFF;
}

BOOL CScEditLexInput::IsEOF()
{
  return TRUE;
}
